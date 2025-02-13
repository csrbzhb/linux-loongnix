// SPDX-License-Identifier: GPL-2.0+
#include <linux/types.h>
#include <linux/gpio.h>
#include <linux/pwm.h>
#include "loongson_drv.h"
#include "loongson_vbios.h"
#include "loongson_legacy_vbios.h"
#include "loongson_backlight.h"
#include "bridge/bridge_phy.h"

static bool loongson_backlight_get_hw_status(struct loongson_backlight *ls_bl)
{
	return (gpio_get_value(LOONGSON_GPIO_LCD_VDD)
			&& gpio_get_value(LOONGSON_GPIO_LCD_EN)
			&& pwm_is_enabled(ls_bl->pwm));
}

static void loongson_backlight_enable(struct loongson_backlight *ls_bl)
{
	struct loongson_device *ldev;
	struct bridge_phy *phy;

	ldev = (struct loongson_device *)ls_bl->driver_private;
	phy = ldev->mode_info[ls_bl->display_pipe_index].bridge_phy;
	if (IS_ERR(ls_bl->pwm))
		return;

	if (phy && phy->cfg_funcs && phy->cfg_funcs->backlight_ctrl)
		phy->cfg_funcs->backlight_ctrl(phy, DRM_MODE_DPMS_ON);
	else {
		BACKLIGHT_DEFAULT_METHOD_OPEN(ls_bl);
		DRM_INFO("%s set backlight enable.\n",
				phy ? phy->res->chip_name : "");
	}

	ls_bl->device->props.power = FB_BLANK_UNBLANK;
}

static void loongson_backlight_disable(struct loongson_backlight *ls_bl)
{
	struct loongson_device *ldev;
	struct bridge_phy *phy;

	ldev = (struct loongson_device *)ls_bl->driver_private;
	phy = ldev->mode_info[ls_bl->display_pipe_index].bridge_phy;
	if (IS_ERR(ls_bl->pwm))
		return;

	if (phy && phy->cfg_funcs && phy->cfg_funcs->backlight_ctrl)
		phy->cfg_funcs->backlight_ctrl(phy, DRM_MODE_DPMS_OFF);
	else {
		BACKLIGHT_DEFAULT_METHOD_CLOSE(ls_bl);
		DRM_INFO("%s set backlight disable.\n",
				phy ? phy->res->chip_name : "");
	}
	ls_bl->device->props.power = !FB_BLANK_UNBLANK;
}

static void loongson_backlight_power(struct loongson_backlight *ls_bl,
		bool enable)
{
	struct loongson_device *ldev;
	struct bridge_phy *phy;

	ldev = (struct loongson_device *)ls_bl->driver_private;
	phy = ldev->mode_info[ls_bl->display_pipe_index].bridge_phy;
	DRM_DEBUG("%s Request backlight power: %s->%s.\n",
			phy ? phy->res->chip_name : "", ls_bl->hw_enabled ? "open"
			: "close", enable ? "open" : "close");

	if (enable && !ls_bl->hw_enabled) {
		if (ls_bl->enable)
			ls_bl->enable(ls_bl);
	} else if (!enable && ls_bl->hw_enabled) {
		if (ls_bl->disable)
			ls_bl->disable(ls_bl);
	}
}

static int loongson_backlight_update(struct backlight_device *bd)
{
	struct loongson_device *ldev;
	struct bridge_phy *phy;
	struct loongson_backlight *ls_bl = bl_get_data(bd);

	ldev = (struct loongson_device *)ls_bl->driver_private;
	phy = ldev->mode_info[ls_bl->display_pipe_index].bridge_phy;
	DRM_DEBUG("%s Request bl update: %s->%s, level:%d->%d.\n",
			phy ? phy->res->chip_name : "",
			ls_bl->hw_enabled ? "open" : "close",
			bd->props.power == FB_BLANK_UNBLANK ? "open" : "close",
			ls_bl->level, bd->props.brightness);

	if (ls_bl->hw_enabled != (bd->props.power == FB_BLANK_UNBLANK))
		ls_bl->power(ls_bl, bd->props.power == FB_BLANK_UNBLANK);

	if (ls_bl->level != bd->props.brightness) {
		ls_bl->level = bd->props.brightness;
		ls_bl->set_brightness(ls_bl, ls_bl->level);
	}

	return 0;
}

static int loongson_backlight_get_brightness(struct backlight_device *bd)
{
	struct loongson_backlight *ls_bl = bl_get_data(bd);

	if (ls_bl->get_brightness)
		return ls_bl->get_brightness(ls_bl);

	return -ENOEXEC;
}

static const struct backlight_ops loongson_backlight_ops = {
	.update_status  = loongson_backlight_update,
	.get_brightness = loongson_backlight_get_brightness,
};

static unsigned int loongson_backlight_get(struct loongson_backlight *ls_bl)
{
	u16 duty_ns, period_ns;
	u32 level;

	if (IS_ERR(ls_bl->pwm))
		return 0;

	period_ns = ls_bl->pwm_period;
	duty_ns = pwm_get_duty_cycle(ls_bl->pwm);

	level = DIV_ROUND_UP((duty_ns * ls_bl->max), period_ns);
	level = clamp(level, ls_bl->min, ls_bl->max);

	return level;
}

static void loongson_backlight_set(struct loongson_backlight *ls_bl,
		unsigned int level)
{
	unsigned int period_ns;
	unsigned int duty_ns;
	struct bridge_phy *phy;
	struct loongson_device *ldev;

	ldev = (struct loongson_device *)ls_bl->driver_private;
	phy = ldev->mode_info[ls_bl->display_pipe_index].bridge_phy;
	if (IS_ERR(ls_bl->pwm))
		return;

	level = clamp(level, ls_bl->min, ls_bl->max);
	period_ns = ls_bl->pwm_period;
	duty_ns = DIV_ROUND_UP((level * period_ns), ls_bl->max);

	DRM_DEBUG("%s Set backlight: level=%d, 0x%x/0x%x ns.\n",
			phy ? phy->res->chip_name : "", level, duty_ns, period_ns);
	pwm_config(ls_bl->pwm, duty_ns, period_ns);
}

static int loongson_backlight_hw_request_init(struct loongson_backlight *ls_bl)
{
	int ret = 0;
	bool pwm_enable_default;

	ls_bl->pwm = pwm_request(ls_bl->pwm_id, "Loongson_bl");
	if (IS_ERR(ls_bl->pwm)) {
		DRM_ERROR("Failed to get the pwm chip\n");
		ls_bl->pwm = NULL;
		goto ERROR_PWM;
	}

	pwm_enable_default = pwm_is_enabled(ls_bl->pwm);
	/* pwm init.*/
	pwm_disable(ls_bl->pwm);
	pwm_set_polarity(ls_bl->pwm, ls_bl->pwm_polarity);
	loongson_backlight_set(ls_bl, ls_bl->level);
	if (pwm_enable_default)
		pwm_enable(ls_bl->pwm);

	ret = gpio_request(LOONGSON_GPIO_LCD_VDD, "GPIO_VDD");
	if (ret) {
		DRM_ERROR("EN request error!\n");
		goto ERROR_VDD;
	}

	ret = gpio_request(LOONGSON_GPIO_LCD_EN, "GPIO_EN");
	if (ret) {
		DRM_ERROR("VDD request error!\n");
		goto ERROR_EN;
	}
	/* gpio init */
	gpio_direction_output(LOONGSON_GPIO_LCD_VDD, 1);
	gpio_direction_output(LOONGSON_GPIO_LCD_EN, 1);
	return ret;

ERROR_EN:
	gpio_free(LOONGSON_GPIO_LCD_VDD);
ERROR_VDD:
	pwm_free(ls_bl->pwm);
ERROR_PWM:
	return -ENODEV;
}

static struct loongson_backlight *loongson_backlight_init(
		struct loongson_device *ldev, int index)
{
	int ret = 0;
	unsigned char *name;
	struct bridge_phy *phy = NULL;
	struct loongson_backlight *ls_bl = NULL;
	struct backlight_properties props;

	phy = ldev->mode_info[index].bridge_phy;
	name = phy ? phy->res->chip_name : "";

	ls_bl = kzalloc(sizeof(struct loongson_backlight), GFP_KERNEL);
	if (IS_ERR(ls_bl)) {
		DRM_ERROR("%s Failed to alloc backlight.\n", name);
		return NULL;
	}

	ls_bl->min = LOONGSON_BL_MIN_LEVEL;
	ls_bl->max = LOONGSON_BL_MAX_LEVEL;
	ls_bl->level = LOONGSON_BL_DEF_LEVEL;
	ls_bl->driver_private = ldev;
	ls_bl->display_pipe_index = index;
	ls_bl->get_brightness = loongson_backlight_get;
	ls_bl->set_brightness = loongson_backlight_set;
	ls_bl->enable = loongson_backlight_enable;
	ls_bl->disable = loongson_backlight_disable;
	ls_bl->power = loongson_backlight_power;
	ls_bl->pwm_id = get_vbios_pwm(ldev, index, VBIOS_PWM_ID);
	/* 0:low start, 1:high start */
	ls_bl->pwm_polarity = get_vbios_pwm(ldev, index, VBIOS_PWM_POLARITY);
	ls_bl->pwm_period = get_vbios_pwm(ldev, index, VBIOS_PWM_PERIOD);

	DRM_INFO("%s pwm: id=%d, period=%dns, polarity=%d.\n",
			name, ls_bl->pwm_id,
			ls_bl->pwm_period, ls_bl->pwm_polarity);

	ret = loongson_backlight_hw_request_init(ls_bl);
	if (ret)
		goto ERROR_HW;

	/* register */
	memset(&props, 0, sizeof(props));
	props.type = BACKLIGHT_RAW;
	props.power = FB_BLANK_UNBLANK; /* Power on */
	props.max_brightness = ls_bl->max;
	props.brightness = ls_bl->level;
	if (ldev->mode_info[index].connector_is_legacy)
		ls_bl->device = backlight_device_register("loongson-gpu",
				ldev->mode_info[index].connector->base.kdev,
				ls_bl, &loongson_backlight_ops, &props);
	else
		ls_bl->device = backlight_device_register("loongson-gpu",
				phy->connector.kdev, ls_bl, &loongson_backlight_ops, &props);
	if (IS_ERR(ls_bl->device)) {
		DRM_ERROR("%s Failed to register backlight.\n", name);
		goto ERROR_REG;
	}

	DRM_INFO("%s register loongson backlight_%d completed.\n", name, index);
	ls_bl->present = true;
	ldev->mode_info[index].backlight = ls_bl;
	return ls_bl;

ERROR_HW:
ERROR_REG:
	kfree(ls_bl);

	return NULL;
}


int loongson_backlight_register(struct drm_connector *connector)
{
	int ret = 0;
	bool ls_bl_status = false;
	bool connector_is_legacy, initialized;
	struct loongson_backlight *ls_bl;
	struct loongson_device *ldev = connector->dev->dev_private;
	struct bridge_phy *phy = NULL;
	struct loongson_connector *ls_connector = NULL;

	initialized = ldev->mode_info[connector->index].mode_config_initialized;
	connector_is_legacy = ldev->mode_info[connector->index].connector_is_legacy;

	if (!initialized) {
		DRM_ERROR("mode config not initialized!\n");
		return -EAGAIN;
	}

	if (connector_is_legacy) {
		ls_connector = ldev->mode_info[connector->index].connector;
		DRM_INFO("legacy connector_type:%s.\n",
				connector_type_to_str(ls_connector->type));

		if (!(ls_connector->type == DRM_MODE_CONNECTOR_LVDS
				|| ls_connector->type == DRM_MODE_CONNECTOR_eDP))
			return ret;
	} else {
		phy = ldev->mode_info[connector->index].bridge_phy;
		DRM_INFO("%s connector_type:%s. backlight_type:%s.\n",
				phy->res->chip_name,
				connector_type_to_str(phy->connector_type),
				bl_type_to_str(phy->res->bl_type));

		if (!(phy->res->bl_type == bl_pwm ||
				phy->connector_type == DRM_MODE_CONNECTOR_eDP))
			return ret;
	}

	ls_bl = loongson_backlight_init(ldev, connector->index);
	if (!ls_bl)
		return -ENXIO;

	/*
	 * force sync dpms status.
	 */
	ls_bl_status = loongson_backlight_get_hw_status(ls_bl);
	if (ls_bl_status)
		ls_bl->hw_enabled = true;
	else
		ls_bl->hw_enabled = false;

	if (!connector_is_legacy && (get_encoder_chip(ldev, connector->index)
			== ENCODER_OBJECT_ID_EDP_NCS8805)) {
		/*
		 * FIX: If it is NCS8805 encoder, it should be turned on
		 * again even if it is on.
		 */
		ls_bl->hw_enabled = false;
		ls_bl->power(ls_bl, true);
	} else {
		if (!ls_bl->hw_enabled
				&& connector->dpms == DRM_MODE_DPMS_ON)
			ls_bl->power(ls_bl, true);
		else if (ls_bl->hw_enabled
				&& connector->dpms != DRM_MODE_DPMS_ON)
			ls_bl->power(ls_bl, false);
	}

	DRM_INFO("%s backlight power status: %s->%s.\n",
			phy ? phy->res->chip_name : "", ls_bl_status ? "on" : "off",
			ls_bl->hw_enabled ? "on" : "off");

	return ret;
}
