/******************************************************************************
 *
 * Copyright(c) 2007 - 2017  Realtek Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * The full GNU General Public License is included in this distribution in the
 * file called LICENSE.
 *
 * Contact Information:
 * wlanfae <wlanfae@realtek.com>
 * Realtek Corporation, No. 2, Innovation Road II, Hsinchu Science Park,
 * Hsinchu 300, Taiwan.
 *
 * Larry Finger <Larry.Finger@lwfinger.net>
 *
 *****************************************************************************/

/* ************************************************************
 * include files
 * ************************************************************ */

#include "mp_precomp.h"
#include "phydm_precomp.h"

/*
 * ODM IO Relative API.
 *   */

u8 odm_read_1byte(
	struct dm_struct		*dm,
	u32			reg_addr
)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP))
	struct rtl8192cd_priv	*priv	= dm->priv;
	return	RTL_R8(reg_addr);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE) && defined(DM_ODM_CE_MAC80211)
	struct rtl_priv *rtlpriv = (struct rtl_priv *)dm->adapter;

	return rtl_read_byte(rtlpriv, reg_addr);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
	void		*adapter = dm->adapter;
	return rtw_read8(adapter, reg_addr);
#elif (DM_ODM_SUPPORT_TYPE & ODM_WIN)
	void		*adapter = dm->adapter;
	return	PlatformEFIORead1Byte(adapter, reg_addr);
#endif

}


u16 odm_read_2byte(
	struct dm_struct		*dm,
	u32			reg_addr
)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP))
	struct rtl8192cd_priv	*priv	= dm->priv;
	return	RTL_R16(reg_addr);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE) && defined(DM_ODM_CE_MAC80211)
	struct rtl_priv *rtlpriv = (struct rtl_priv *)dm->adapter;

	return rtl_read_word(rtlpriv, reg_addr);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
	void		*adapter = dm->adapter;
	return rtw_read16(adapter, reg_addr);
#elif (DM_ODM_SUPPORT_TYPE & ODM_WIN)
	void		*adapter = dm->adapter;
	return	PlatformEFIORead2Byte(adapter, reg_addr);
#endif

}


u32 odm_read_4byte(
	struct dm_struct		*dm,
	u32			reg_addr
)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP))
	struct rtl8192cd_priv	*priv	= dm->priv;
	return	RTL_R32(reg_addr);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE) && defined(DM_ODM_CE_MAC80211)
	struct rtl_priv *rtlpriv = (struct rtl_priv *)dm->adapter;

	return rtl_read_dword(rtlpriv, reg_addr);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
	void		*adapter = dm->adapter;
	return rtw_read32(adapter, reg_addr);
#elif (DM_ODM_SUPPORT_TYPE & ODM_WIN)
	void		*adapter = dm->adapter;
	return	PlatformEFIORead4Byte(adapter, reg_addr);
#endif

}


void odm_write_1byte(
	struct dm_struct		*dm,
	u32			reg_addr,
	u8			data
)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP))
	struct rtl8192cd_priv	*priv	= dm->priv;
	RTL_W8(reg_addr, data);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE) && defined(DM_ODM_CE_MAC80211)
	struct rtl_priv *rtlpriv = (struct rtl_priv *)dm->adapter;

	rtl_write_byte(rtlpriv, reg_addr, data);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
	void		*adapter = dm->adapter;
	rtw_write8(adapter, reg_addr, data);
#elif (DM_ODM_SUPPORT_TYPE & ODM_WIN)
	void		*adapter = dm->adapter;
	PlatformEFIOWrite1Byte(adapter, reg_addr, data);
#endif

}


void odm_write_2byte(
	struct dm_struct		*dm,
	u32			reg_addr,
	u16			data
)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP))
	struct rtl8192cd_priv	*priv	= dm->priv;
	RTL_W16(reg_addr, data);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE) && defined(DM_ODM_CE_MAC80211)
	struct rtl_priv *rtlpriv = (struct rtl_priv *)dm->adapter;

	rtl_write_word(rtlpriv, reg_addr, data);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
	void		*adapter = dm->adapter;
	rtw_write16(adapter, reg_addr, data);
#elif (DM_ODM_SUPPORT_TYPE & ODM_WIN)
	void		*adapter = dm->adapter;
	PlatformEFIOWrite2Byte(adapter, reg_addr, data);
#endif

}


void odm_write_4byte(
	struct dm_struct		*dm,
	u32			reg_addr,
	u32			data
)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP))
	struct rtl8192cd_priv	*priv	= dm->priv;
	RTL_W32(reg_addr, data);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE) && defined(DM_ODM_CE_MAC80211)
	struct rtl_priv *rtlpriv = (struct rtl_priv *)dm->adapter;

	rtl_write_dword(rtlpriv, reg_addr, data);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
	void		*adapter = dm->adapter;
	rtw_write32(adapter, reg_addr, data);
#elif (DM_ODM_SUPPORT_TYPE & ODM_WIN)
	void		*adapter = dm->adapter;
	PlatformEFIOWrite4Byte(adapter, reg_addr, data);
#endif

}


void odm_set_mac_reg(
	struct dm_struct	*dm,
	u32		reg_addr,
	u32		bit_mask,
	u32		data
)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP))
	phy_set_bb_reg(dm->priv, reg_addr, bit_mask, data);
#elif (DM_ODM_SUPPORT_TYPE & ODM_WIN)
	PHY_SetBBReg((PADAPTER)dm->adapter, reg_addr, bit_mask, data);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE) && defined(DM_ODM_CE_MAC80211)
	struct rtl_priv *rtlpriv = (struct rtl_priv *)dm->adapter;

	rtl_set_bbreg(rtlpriv->hw, reg_addr, bit_mask, data);
#else
	phy_set_bb_reg(dm->adapter, reg_addr, bit_mask, data);
#endif
}


u32 odm_get_mac_reg(
	struct dm_struct	*dm,
	u32		reg_addr,
	u32		bit_mask
)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP))
	return phy_query_bb_reg(dm->priv, reg_addr, bit_mask);
#elif (DM_ODM_SUPPORT_TYPE & ODM_WIN)
	return PHY_QueryMacReg((PADAPTER)dm->adapter, reg_addr, bit_mask);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE) && defined(DM_ODM_CE_MAC80211)
	struct rtl_priv *rtlpriv = (struct rtl_priv *)dm->adapter;

	return rtl_get_bbreg(rtlpriv->hw, reg_addr, bit_mask);
#else
	return phy_query_mac_reg(dm->adapter, reg_addr, bit_mask);
#endif
}


void odm_set_bb_reg(
	struct dm_struct	*dm,
	u32		reg_addr,
	u32		bit_mask,
	u32		data
)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP))
	phy_set_bb_reg(dm->priv, reg_addr, bit_mask, data);
#elif (DM_ODM_SUPPORT_TYPE & ODM_WIN)
	PHY_SetBBReg((PADAPTER)dm->adapter, reg_addr, bit_mask, data);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE) && defined(DM_ODM_CE_MAC80211)
	struct rtl_priv *rtlpriv = (struct rtl_priv *)dm->adapter;

	rtl_set_bbreg(rtlpriv->hw, reg_addr, bit_mask, data);
#else
	phy_set_bb_reg(dm->adapter, reg_addr, bit_mask, data);
#endif
}


u32 odm_get_bb_reg(
	struct dm_struct	*dm,
	u32		reg_addr,
	u32		bit_mask
)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP))
	return phy_query_bb_reg(dm->priv, reg_addr, bit_mask);
#elif (DM_ODM_SUPPORT_TYPE & ODM_WIN)
	return PHY_QueryBBReg((PADAPTER)dm->adapter, reg_addr, bit_mask);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE) && defined(DM_ODM_CE_MAC80211)
	struct rtl_priv *rtlpriv = (struct rtl_priv *)dm->adapter;

	return rtl_get_bbreg(rtlpriv->hw, reg_addr, bit_mask);
#else
	return phy_query_bb_reg(dm->adapter, reg_addr, bit_mask);
#endif
}


void odm_set_rf_reg(
	struct dm_struct			*dm,
	u8			e_rf_path,
	u32				reg_addr,
	u32				bit_mask,
	u32				data
)
{
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
	phy_set_rf_reg(dm->priv, e_rf_path, reg_addr, bit_mask, data);
#elif (DM_ODM_SUPPORT_TYPE & ODM_WIN)
	PHY_SetRFReg((PADAPTER)dm->adapter, e_rf_path, reg_addr, bit_mask, data);
	ODM_delay_us(2);

#elif (DM_ODM_SUPPORT_TYPE & ODM_CE) && defined(DM_ODM_CE_MAC80211)
	struct rtl_priv *rtlpriv = (struct rtl_priv *)dm->adapter;

	rtl_set_rfreg(rtlpriv->hw, e_rf_path, reg_addr, bit_mask, data);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
	phy_set_rf_reg(dm->adapter, e_rf_path, reg_addr, bit_mask, data);
#endif
}

u32 odm_get_rf_reg(
	struct dm_struct			*dm,
	u8			e_rf_path,
	u32				reg_addr,
	u32				bit_mask
)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP))
	return phy_query_rf_reg(dm->priv, e_rf_path, reg_addr, bit_mask, 1);
#elif (DM_ODM_SUPPORT_TYPE & ODM_WIN)
	return PHY_QueryRFReg((PADAPTER)dm->adapter, e_rf_path, reg_addr,
			      bit_mask);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE) && defined(DM_ODM_CE_MAC80211)
	struct rtl_priv *rtlpriv = (struct rtl_priv *)dm->adapter;

	return rtl_get_rfreg(rtlpriv->hw, e_rf_path, reg_addr, bit_mask);
#else
	return phy_query_rf_reg(dm->adapter, e_rf_path, reg_addr, bit_mask);
#endif
}

enum hal_status phydm_set_reg_by_fw(
	struct dm_struct			*dm,
	enum phydm_halmac_param	config_type,
	u32	offset,
	u32	data,
	u32	mask,
	enum rf_path	e_rf_path,
	u32 delay_time
)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_WIN))
	return HAL_MAC_Config_PHY_WriteNByte(dm,
					     config_type,
					     offset,
					     data,
					     mask,
					     e_rf_path,
					     delay_time);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
#if (DM_ODM_SUPPORT_TYPE & ODM_CE) && defined(DM_ODM_CE_MAC80211)
	PHYDM_DBG(dm, ODM_COMP_COMMON, "Not support for CE MAC80211 driver!\n");
#else
	return rtw_phydm_cfg_phy_para(dm,
				      config_type,
				      offset,
				      data,
				      mask,
				      e_rf_path,
				      delay_time);
#endif
#endif

}


/*
 * ODM Memory relative API.
 *   */
void odm_allocate_memory(
	struct dm_struct	*dm,
	void **ptr,
	u32		length
)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP))
	*ptr = kmalloc(length, GFP_ATOMIC);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE) && defined(DM_ODM_CE_MAC80211)
	*ptr = kmalloc(length, GFP_ATOMIC);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
	*ptr = rtw_zvmalloc(length);
#elif (DM_ODM_SUPPORT_TYPE & ODM_WIN)
	void		*adapter = dm->adapter;
	PlatformAllocateMemory(adapter, ptr, length);
#endif
}

/* length could be ignored, used to detect memory leakage. */
void odm_free_memory(
	struct dm_struct	*dm,
	void		*ptr,
	u32		length
)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP))
	kfree(ptr);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE) && defined(DM_ODM_CE_MAC80211)
	kfree(ptr);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
	rtw_vmfree(ptr, length);
#elif (DM_ODM_SUPPORT_TYPE & ODM_WIN)
	/* struct void*    adapter = dm->adapter; */
	PlatformFreeMemory(ptr, length);
#endif
}

void odm_move_memory(
	struct dm_struct	*dm,
	void		*dest,
	void		*src,
	u32		length
)
{
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
	memcpy(dest, src, length);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE) && defined(DM_ODM_CE_MAC80211)
	memcpy(dest, src, length);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
	_rtw_memcpy(dest, src, length);
#elif (DM_ODM_SUPPORT_TYPE & ODM_WIN)
	PlatformMoveMemory(dest, src, length);
#endif
}

void odm_memory_set(
	struct dm_struct	*dm,
	void		*pbuf,
	s8		value,
	u32		length
)
{
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
	memset(pbuf, value, length);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE) && defined(DM_ODM_CE_MAC80211)
	memset(pbuf, value, length);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
	_rtw_memset(pbuf, value, length);
#elif (DM_ODM_SUPPORT_TYPE & ODM_WIN)
	PlatformFillMemory(pbuf, length, value);
#endif
}
s32 odm_compare_memory(
	struct dm_struct		*dm,
	void           *buf1,
	void           *buf2,
	u32          length
)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP))
	return memcmp(buf1, buf2, length);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE) && defined(DM_ODM_CE_MAC80211)
	return memcmp(buf1, buf2, length);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
	return _rtw_memcmp(buf1, buf2, length);
#elif (DM_ODM_SUPPORT_TYPE & ODM_WIN)
	return PlatformCompareMemory(buf1, buf2, length);
#endif
}



/*
 * ODM MISC relative API.
 *   */
void odm_acquire_spin_lock(
	struct dm_struct			*dm,
	enum rt_spinlock_type	type
) {
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP))

#elif (DM_ODM_SUPPORT_TYPE & ODM_CE) && defined(DM_ODM_CE_MAC80211)
	struct rtl_priv *rtlpriv = (struct rtl_priv *)dm->adapter;
	rtl_odm_acquirespinlock(rtlpriv, type);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
	void *adapter = dm->adapter;
	rtw_odm_acquirespinlock(adapter, type);
#elif (DM_ODM_SUPPORT_TYPE & ODM_WIN)
	void		*adapter = dm->adapter;
	PlatformAcquireSpinLock(adapter, type);
#endif
}
void odm_release_spin_lock(
	struct dm_struct			*dm,
	enum rt_spinlock_type	type
) {
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP))

#elif (DM_ODM_SUPPORT_TYPE & ODM_CE) && defined(DM_ODM_CE_MAC80211)
	struct rtl_priv *rtlpriv = (struct rtl_priv *)dm->adapter;
	rtl_odm_releasespinlock(rtlpriv, type);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
	void *adapter = dm->adapter;
	rtw_odm_releasespinlock(adapter, type);
#elif (DM_ODM_SUPPORT_TYPE & ODM_WIN)
	void		*adapter = dm->adapter;
	PlatformReleaseSpinLock(adapter, type);
#endif
}

#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
/*
 * Work item relative API. FOr MP driver only~!
 *   */
void odm_initialize_work_item(
	struct dm_struct					*dm,
	PRT_WORK_ITEM				p_rt_work_item,
	RT_WORKITEM_CALL_BACK		rt_work_item_callback,
	void						*context,
	const char					*sz_id
)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP))

#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)

#elif (DM_ODM_SUPPORT_TYPE & ODM_WIN)
	void		*adapter = dm->adapter;
	PlatformInitializeWorkItem(adapter, p_rt_work_item, rt_work_item_callback,
				   context, sz_id);
#endif
}


void odm_start_work_item(
	PRT_WORK_ITEM	p_rt_work_item
)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP))

#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)

#elif (DM_ODM_SUPPORT_TYPE & ODM_WIN)
	PlatformStartWorkItem(p_rt_work_item);
#endif
}


void odm_stop_work_item(
	PRT_WORK_ITEM	p_rt_work_item
)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP))

#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)

#elif (DM_ODM_SUPPORT_TYPE & ODM_WIN)
	PlatformStopWorkItem(p_rt_work_item);
#endif
}


void odm_free_work_item(
	PRT_WORK_ITEM	p_rt_work_item
)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP))

#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)

#elif (DM_ODM_SUPPORT_TYPE & ODM_WIN)
	PlatformFreeWorkItem(p_rt_work_item);
#endif
}


void odm_schedule_work_item(
	PRT_WORK_ITEM	p_rt_work_item
)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP))

#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)

#elif (DM_ODM_SUPPORT_TYPE & ODM_WIN)
	PlatformScheduleWorkItem(p_rt_work_item);
#endif
}


boolean odm_is_work_item_scheduled(
	PRT_WORK_ITEM	p_rt_work_item
)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP))

#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)

#elif (DM_ODM_SUPPORT_TYPE & ODM_WIN)
	return PlatformIsWorkItemScheduled(p_rt_work_item);
#endif
}
#endif


/*
 * ODM Timer relative API.
 *   */

void ODM_delay_ms(u32	ms)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP))
	delay_ms(ms);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE) && defined(DM_ODM_CE_MAC80211)
	mdelay(ms);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
	rtw_mdelay_os(ms);
#elif (DM_ODM_SUPPORT_TYPE & ODM_WIN)
	delay_ms(ms);
#endif
}

void ODM_delay_us(u32	us)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP))
	delay_us(us);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE) && defined(DM_ODM_CE_MAC80211)
	udelay(us);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
	rtw_udelay_os(us);
#elif (DM_ODM_SUPPORT_TYPE & ODM_WIN)
	PlatformStallExecution(us);
#endif
}

void ODM_sleep_ms(u32	ms)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP))
	delay_ms(ms);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE) && defined(DM_ODM_CE_MAC80211)
	msleep(ms);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
	rtw_msleep_os(ms);
#elif (DM_ODM_SUPPORT_TYPE & ODM_WIN)
	delay_ms(ms);
#endif
}

void ODM_sleep_us(u32	us)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP))
	delay_us(us);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE) && defined(DM_ODM_CE_MAC80211)
	usleep_range(us, us + 1);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
	rtw_usleep_os(us);
#elif (DM_ODM_SUPPORT_TYPE & ODM_WIN)
	PlatformStallExecution(us);
#endif
}

void odm_set_timer(
	struct dm_struct		*dm,
	struct phydm_timer_list		*timer,
	u32			ms_delay
)
{
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
	mod_timer(timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(ms_delay));
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE) && defined(DM_ODM_CE_MAC80211)
	mod_timer(timer, jiffies + msecs_to_jiffies(ms_delay));
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
	_set_timer(timer, ms_delay); /* ms */
#elif (DM_ODM_SUPPORT_TYPE & ODM_WIN)
	void		*adapter = dm->adapter;
	PlatformSetTimer(adapter, timer, ms_delay);
#endif

}

void odm_initialize_timer(
	struct dm_struct			*dm,
	struct phydm_timer_list			*timer,
	void	*call_back_func,
	void				*context,
	const char			*sz_id
)
{
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
	init_timer(timer);
	timer->function = call_back_func;
	timer->data = (unsigned long)dm;
	/*mod_timer(timer, jiffies+RTL_MILISECONDS_TO_JIFFIES(10));	*/
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE) && defined(DM_ODM_CE_MAC80211)
	init_timer(timer);
	timer->function = call_back_func;
	timer->data = (unsigned long)dm;
	/*mod_timer(timer, jiffies+RTL_MILISECONDS_TO_JIFFIES(10));	*/
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
	struct _ADAPTER *adapter = dm->adapter;

	_init_timer(timer, adapter->pnetdev, call_back_func, dm);
#elif (DM_ODM_SUPPORT_TYPE & ODM_WIN)
	void *adapter = dm->adapter;

	PlatformInitializeTimer(adapter, timer, (RT_TIMER_CALL_BACK)call_back_func,
				context, sz_id);
#endif
}


void odm_cancel_timer(
	struct dm_struct		*dm,
	struct phydm_timer_list		*timer
)
{
#if (DM_ODM_SUPPORT_TYPE & ODM_AP)
	del_timer(timer);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE) && defined(DM_ODM_CE_MAC80211)
	del_timer(timer);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
	_cancel_timer_ex(timer);
#elif (DM_ODM_SUPPORT_TYPE & ODM_WIN)
	void *adapter = dm->adapter;
	PlatformCancelTimer(adapter, timer);
#endif
}


void odm_release_timer(
	struct dm_struct		*dm,
	struct phydm_timer_list		*timer
)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP))

#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)

#elif (DM_ODM_SUPPORT_TYPE & ODM_WIN)

	void *adapter = dm->adapter;

	/* <20120301, Kordan> If the initilization fails, InitializeAdapterXxx will return regardless of InitHalDm.
	 * Hence, uninitialized timers cause BSOD when the driver releases resources since the init fail. */
	if (timer == 0) {
		PHYDM_DBG(dm, ODM_COMP_INIT,
			  "=====>odm_release_timer(), The timer is NULL! Please check it!\n");
		return;
	}

	PlatformReleaseTimer(adapter, timer);
#endif
}


u8 phydm_trans_h2c_id(
	struct dm_struct	*dm,
	u8		phydm_h2c_id
)
{
	u8 platform_h2c_id = phydm_h2c_id;

	switch (phydm_h2c_id) {
	/* 1 [0] */
	case ODM_H2C_RSSI_REPORT:

#if (DM_ODM_SUPPORT_TYPE & ODM_WIN)
		if (dm->support_ic_type == ODM_RTL8188E)
			platform_h2c_id = H2C_88E_RSSI_REPORT;
		else if (dm->support_ic_type == ODM_RTL8814A)
			platform_h2c_id = H2C_8814A_RSSI_REPORT;
		else
			platform_h2c_id = H2C_RSSI_REPORT;

#elif (DM_ODM_SUPPORT_TYPE & ODM_CE) && defined(DM_ODM_CE_MAC80211)
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
		platform_h2c_id = H2C_RSSI_SETTING;

#elif (DM_ODM_SUPPORT_TYPE & ODM_AP)
#if ((RTL8881A_SUPPORT == 1) || (RTL8192E_SUPPORT == 1) || (RTL8814A_SUPPORT == 1) || (RTL8822B_SUPPORT == 1) || (RTL8197F_SUPPORT == 1))
		if (dm->support_ic_type == ODM_RTL8881A
		    || dm->support_ic_type == ODM_RTL8192E
		    || dm->support_ic_type & PHYDM_IC_3081_SERIES)
			platform_h2c_id = H2C_88XX_RSSI_REPORT;
		else
#endif
#if (RTL8812A_SUPPORT == 1)
			if (dm->support_ic_type == ODM_RTL8812)
				platform_h2c_id = H2C_8812_RSSI_REPORT;
			else
#endif
			{}
#endif

		break;

	/* 1 [3] */
	case ODM_H2C_WIFI_CALIBRATION:
#if (DM_ODM_SUPPORT_TYPE & ODM_WIN)
		platform_h2c_id = H2C_WIFI_CALIBRATION;

#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
#if (RTL8723B_SUPPORT == 1)
		platform_h2c_id = H2C_8723B_BT_WLAN_CALIBRATION;
#endif

#elif (DM_ODM_SUPPORT_TYPE & ODM_AP)
#endif
		break;


	/* 1 [4] */
	case ODM_H2C_IQ_CALIBRATION:
#if (DM_ODM_SUPPORT_TYPE & ODM_WIN)
		platform_h2c_id = H2C_IQ_CALIBRATION;

#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
#if ((RTL8812A_SUPPORT == 1) || (RTL8821A_SUPPORT == 1))
		platform_h2c_id = H2C_8812_IQ_CALIBRATION;
#endif
#elif (DM_ODM_SUPPORT_TYPE & ODM_AP)
#endif

		break;
	/* 1 [5] */
	case ODM_H2C_RA_PARA_ADJUST:

#if (DM_ODM_SUPPORT_TYPE & ODM_WIN)
		if (dm->support_ic_type & (ODM_RTL8814A | ODM_RTL8822B))
			platform_h2c_id = H2C_8814A_RA_PARA_ADJUST;
		else
			platform_h2c_id = H2C_RA_PARA_ADJUST;
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE) && defined(DM_ODM_CE_MAC80211)
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
#if ((RTL8812A_SUPPORT == 1) || (RTL8821A_SUPPORT == 1))
		platform_h2c_id = H2C_8812_RA_PARA_ADJUST;
#elif ((RTL8814A_SUPPORT == 1) || (RTL8822B_SUPPORT == 1))
		platform_h2c_id = H2C_RA_PARA_ADJUST;
#elif (RTL8192E_SUPPORT == 1)
		platform_h2c_id = H2C_8192E_RA_PARA_ADJUST;
#elif (RTL8723B_SUPPORT == 1)
		platform_h2c_id = H2C_8723B_RA_PARA_ADJUST;
#endif

#elif (DM_ODM_SUPPORT_TYPE & ODM_AP)
#if ((RTL8881A_SUPPORT == 1) || (RTL8192E_SUPPORT == 1) || (RTL8814A_SUPPORT == 1) || (RTL8822B_SUPPORT == 1) || (RTL8197F_SUPPORT == 1))
		if (dm->support_ic_type == ODM_RTL8881A
		    || dm->support_ic_type == ODM_RTL8192E
		    || dm->support_ic_type & PHYDM_IC_3081_SERIES)
			platform_h2c_id = H2C_88XX_RA_PARA_ADJUST;
		else
#endif
#if (RTL8812A_SUPPORT == 1)
			if (dm->support_ic_type == ODM_RTL8812)
				platform_h2c_id = H2C_8812_RA_PARA_ADJUST;
			else
#endif
			{}
#endif

		break;


	/* 1 [6] */
	case PHYDM_H2C_DYNAMIC_TX_PATH:

#if (DM_ODM_SUPPORT_TYPE & ODM_WIN)
		if (dm->support_ic_type == ODM_RTL8814A)
			platform_h2c_id = H2C_8814A_DYNAMIC_TX_PATH;
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
#if (RTL8814A_SUPPORT == 1)
		if (dm->support_ic_type == ODM_RTL8814A)
			platform_h2c_id = H2C_DYNAMIC_TX_PATH;
#endif
#elif (DM_ODM_SUPPORT_TYPE & ODM_AP)
#if (RTL8814A_SUPPORT == 1)
		if (dm->support_ic_type == ODM_RTL8814A)
			platform_h2c_id = H2C_88XX_DYNAMIC_TX_PATH;
#endif

#endif

		break;

	/* [7]*/
	case PHYDM_H2C_FW_TRACE_EN:

#if (DM_ODM_SUPPORT_TYPE & ODM_WIN)
		if (dm->support_ic_type & (ODM_RTL8814A | ODM_RTL8822B))
			platform_h2c_id = H2C_8814A_FW_TRACE_EN;
		else
			platform_h2c_id = H2C_FW_TRACE_EN;

#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)

		platform_h2c_id = 0x49;

#elif (DM_ODM_SUPPORT_TYPE & ODM_AP)
#if ((RTL8881A_SUPPORT == 1) || (RTL8192E_SUPPORT == 1) || (RTL8814A_SUPPORT == 1) || (RTL8822B_SUPPORT == 1) || (RTL8197F_SUPPORT == 1))
		if (dm->support_ic_type == ODM_RTL8881A
		    || dm->support_ic_type == ODM_RTL8192E
		    || dm->support_ic_type & PHYDM_IC_3081_SERIES)
			platform_h2c_id  = H2C_88XX_FW_TRACE_EN;
		else
#endif
#if (RTL8812A_SUPPORT == 1)
			if (dm->support_ic_type == ODM_RTL8812)
				platform_h2c_id = H2C_8812_FW_TRACE_EN;
			else
#endif
			{}

#endif

		break;

	case PHYDM_H2C_TXBF:
#if ((RTL8192E_SUPPORT == 1) || (RTL8812A_SUPPORT == 1))
		platform_h2c_id  = 0x41;	/*H2C_TxBF*/
#endif
		break;

	case PHYDM_H2C_MU:
#if (RTL8822B_SUPPORT == 1)
		platform_h2c_id  = 0x4a;	/*H2C_MU*/
#endif
		break;

	default:
		platform_h2c_id = phydm_h2c_id;
		break;
	}

	return platform_h2c_id;

}

/*ODM FW relative API.*/

void odm_fill_h2c_cmd(
	struct dm_struct		*dm,
	u8			phydm_h2c_id,
	u32			cmd_len,
	u8			*cmd_buffer
)
{
#if (DM_ODM_SUPPORT_TYPE & ODM_CE) && defined(DM_ODM_CE_MAC80211)
	struct rtl_priv *rtlpriv = (struct rtl_priv *)dm->adapter;
#else
	PADAPTER	adapter = (PADAPTER)dm->adapter;
#endif
	u8		h2c_id = phydm_trans_h2c_id(dm, phydm_h2c_id);

	PHYDM_DBG(dm, DBG_RA, "[H2C]  h2c_id=((0x%x))\n", h2c_id);

#if (DM_ODM_SUPPORT_TYPE & ODM_WIN)
	if (dm->support_ic_type == ODM_RTL8188E)	{
		if (!dm->ra_support88e)
			FillH2CCmd88E(adapter, h2c_id, cmd_len, cmd_buffer);
	} else if (dm->support_ic_type == ODM_RTL8814A)
		FillH2CCmd8814A(adapter, h2c_id, cmd_len, cmd_buffer);
	else if (dm->support_ic_type == ODM_RTL8822B)
		FillH2CCmd8822B(adapter, h2c_id, cmd_len, cmd_buffer);
	else
		FillH2CCmd(adapter, h2c_id, cmd_len, cmd_buffer);

#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)

#ifdef DM_ODM_CE_MAC80211
	rtlpriv->cfg->ops->fill_h2c_cmd(rtlpriv->hw, h2c_id, cmd_len, cmd_buffer);
#else
	rtw_hal_fill_h2c_cmd(adapter, h2c_id, cmd_len, cmd_buffer);
#endif

#elif (DM_ODM_SUPPORT_TYPE & ODM_AP)

#if (RTL8812A_SUPPORT == 1)
	if (dm->support_ic_type == ODM_RTL8812) {
		fill_h2c_cmd8812(dm->priv, h2c_id, cmd_len, cmd_buffer);
	} else
#endif
	{
		GET_HAL_INTERFACE(dm->priv)->fill_h2c_cmd_handler(dm->priv, h2c_id,
				cmd_len, cmd_buffer);
	}
#endif
}

u8 phydm_c2H_content_parsing(
	void			*dm_void,
	u8			c2h_cmd_id,
	u8			c2h_cmd_len,
	u8			*tmp_buf
)
{
	struct dm_struct		*dm = (struct dm_struct *)dm_void;
#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	void	*adapter = dm->adapter;
#endif
	u8	extend_c2h_sub_id = 0;
	u8	find_c2h_cmd = true;

	if ((c2h_cmd_len > 12) || (c2h_cmd_len == 0)) {
		pr_debug("[Warning] Error C2H ID=%d, len=%d\n", c2h_cmd_id, c2h_cmd_len);

		find_c2h_cmd = false;
		return find_c2h_cmd;
	}

	switch (c2h_cmd_id) {
	case PHYDM_C2H_DBG:
		phydm_fw_trace_handler(dm, tmp_buf, c2h_cmd_len);
		break;

	case PHYDM_C2H_RA_RPT:
		phydm_c2h_ra_report_handler(dm, tmp_buf, c2h_cmd_len);
		break;

	case PHYDM_C2H_RA_PARA_RPT:
		odm_c2h_ra_para_report_handler(dm, tmp_buf, c2h_cmd_len);
		break;

	case PHYDM_C2H_DYNAMIC_TX_PATH_RPT:
		if (dm->support_ic_type & (ODM_RTL8814A))
			phydm_c2h_dtp_handler(dm, tmp_buf, c2h_cmd_len);
		break;

	case PHYDM_C2H_IQK_FINISH:
#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)

		if (dm->support_ic_type & (ODM_RTL8812 | ODM_RTL8821)) {
			RT_TRACE(COMP_MP, DBG_LOUD, ("== FW IQK Finish ==\n"));
			odm_acquire_spin_lock(dm, RT_IQK_SPINLOCK);
			dm->rf_calibrate_info.is_iqk_in_progress = false;
			odm_release_spin_lock(dm, RT_IQK_SPINLOCK);
			dm->rf_calibrate_info.iqk_progressing_time = 0;
			dm->rf_calibrate_info.iqk_progressing_time = odm_get_progressing_time(dm,
					dm->rf_calibrate_info.iqk_start_time);
		}

#endif
		break;

	case PHYDM_C2H_CLM_MONITOR:
		phydm_clm_c2h_report_handler(dm, tmp_buf, c2h_cmd_len);
		break;

	case PHYDM_C2H_DBG_CODE:
		phydm_fw_trace_handler_code(dm, tmp_buf, c2h_cmd_len);
		break;

	case PHYDM_C2H_EXTEND:
		extend_c2h_sub_id = tmp_buf[0];
		if (extend_c2h_sub_id == PHYDM_EXTEND_C2H_DBG_PRINT)
			phydm_fw_trace_handler_8051(dm, tmp_buf, c2h_cmd_len);

		break;

	default:
		find_c2h_cmd = false;
		break;
	}

	return find_c2h_cmd;

}

u64 odm_get_current_time(
	struct dm_struct		*dm
)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP))
	return (u64)rtw_get_current_time();
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE) && defined(DM_ODM_CE_MAC80211)
	return jiffies;
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
	return rtw_get_current_time();
#elif (DM_ODM_SUPPORT_TYPE & ODM_WIN)
	return  PlatformGetCurrentTime();
#endif
}

u64 odm_get_progressing_time(
	struct dm_struct		*dm,
	u64			start_time
)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP))
	return rtw_get_passing_time_ms((u32)start_time);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE) && defined(DM_ODM_CE_MAC80211)
	return jiffies_to_msecs(jiffies - start_time);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
	return rtw_get_passing_time_ms((systime)start_time);
#elif (DM_ODM_SUPPORT_TYPE & ODM_WIN)
	return ((PlatformGetCurrentTime() - start_time) >> 10);
#endif
}

#if (DM_ODM_SUPPORT_TYPE & (ODM_WIN|ODM_CE)) && !defined(DM_ODM_CE_MAC80211)

void phydm_set_hw_reg_handler_interface(
	struct dm_struct		*dm,
	u8				RegName,
	u8				*val
)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_WIN|ODM_CE))
	struct _ADAPTER *adapter = (PADAPTER)dm->adapter;

#if (DM_ODM_SUPPORT_TYPE & ODM_WIN)
	((PADAPTER)adapter)->HalFunc.SetHwRegHandler(adapter, RegName, val);
#else
	adapter->hal_func.set_hw_reg_handler(adapter, RegName, val);
#endif

#endif

}

void phydm_get_hal_def_var_handler_interface(
	struct dm_struct		*dm,
	enum _HAL_DEF_VARIABLE		e_variable,
	void						*value
)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_WIN|ODM_CE))
	struct _ADAPTER *adapter = (PADAPTER)dm->adapter;

#if (DM_ODM_SUPPORT_TYPE & ODM_WIN)
	adapter->HalFunc.GetHalDefVarHandler(adapter, e_variable, value);
#else
	adapter->hal_func.get_hal_def_var_handler(adapter, e_variable, value);
#endif

#endif
}

#endif

void odm_set_tx_power_index_by_rate_section(
	struct dm_struct	*dm,
	enum rf_path		path,
	u8				channel,
	u8				rate_section
)
{
#if (DM_ODM_SUPPORT_TYPE & ODM_WIN)
	PHY_SetTxPowerIndexByRateSection((PADAPTER)dm->adapter, path, channel,
					 rate_section);
#elif (DM_ODM_SUPPORT_TYPE == ODM_CE) && defined(DM_ODM_CE_MAC80211)
	phy_set_tx_power_index_by_rs((PADAPTER)dm->adapter, channel, path,
				     rate_section);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
	phy_set_tx_power_index_by_rate_section(dm->adapter, path, channel,
					       rate_section);
#endif
}


u8 odm_get_tx_power_index(
	struct dm_struct	*dm,
	enum rf_path		path,
	u8				tx_rate,
	u8				band_width,
	u8				channel
)
{
#if (DM_ODM_SUPPORT_TYPE & ODM_WIN)
	return PHY_GetTxPowerIndex((PADAPTER)dm->adapter, path, tx_rate,
				   (CHANNEL_WIDTH)band_width, channel);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE) && defined(DM_ODM_CE_MAC80211)
	void		*adapter = dm->adapter;

	return phy_get_tx_power_index(adapter, (enum rf_path)path, tx_rate,
				      band_width, channel);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
	return phy_get_tx_power_index(dm->adapter, path, tx_rate, band_width,
				      channel);
#endif
}



u8 odm_efuse_one_byte_read(
	struct dm_struct	*dm,
	u16			addr,
	u8			*data,
	boolean		b_pseu_do_test
)
{
#if (DM_ODM_SUPPORT_TYPE & ODM_WIN)
	return (u8)EFUSE_OneByteRead((PADAPTER)dm->adapter, addr, data,
				     b_pseu_do_test);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE) && defined(DM_ODM_CE_MAC80211)
	void		*adapter = dm->adapter;

	return rtl_efuse_onebyte_read(adapter, addr, data, b_pseu_do_test);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
	return efuse_onebyte_read(dm->adapter, addr, data, b_pseu_do_test);
#elif (DM_ODM_SUPPORT_TYPE & ODM_AP)
	/*ReadEFuseByte(dm->priv, addr, data);*/
	/*return true;*/
#endif
}



void odm_efuse_logical_map_read(
	struct	dm_struct	*dm,
	u8	type,
	u16	offset,
	u32	*data
)
{
#if (DM_ODM_SUPPORT_TYPE & ODM_WIN)
	EFUSE_ShadowRead((PADAPTER)dm->adapter, type, offset, data);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE) && defined(DM_ODM_CE_MAC80211)
	void		*adapter = dm->adapter;

	rtl_efuse_logical_map_read(adapter, type, offset, data);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
	efuse_logical_map_read(dm->adapter, type, offset, data);
#endif
}

enum hal_status odm_iq_calibrate_by_fw(
	struct dm_struct	*dm,
	u8 clear,
	u8 segment
)
{
	enum hal_status iqk_result = HAL_STATUS_FAILURE;

#if (DM_ODM_SUPPORT_TYPE & ODM_WIN)
	struct _ADAPTER *adapter = (PADAPTER)dm->adapter;

	if (HAL_MAC_FWIQK_Trigger(&GET_HAL_MAC_INFO(adapter), clear, segment) == 0)
		iqk_result = HAL_STATUS_SUCCESS;
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
#if (DM_ODM_SUPPORT_TYPE & ODM_CE) && defined(DM_ODM_CE_MAC80211)
	void		*adapter = dm->adapter;
	iqk_result = rtl_phydm_fw_iqk(adapter, clear, segment);
#else
	iqk_result = rtw_phydm_fw_iqk(dm, clear, segment);
#endif
#endif
	return iqk_result;
}

void odm_cmn_info_ptr_array_hook(
	struct dm_struct		*dm,
	enum odm_cmninfo	cmn_info,
	u16			index,
	void			*value
)
{
	switch	(cmn_info) {
	/*Dynamic call by reference pointer.	*/
	case	ODM_CMNINFO_STA_STATUS:
		dm->odm_sta_info[index] = (struct sta_info *)value;
		break;
	/* To remove the compiler warning, must add an empty default statement to handle the other values. */
	default:
		/* do nothing */
		break;
	}

}

void phydm_cmn_sta_info_hook(
	struct dm_struct		*dm,
	u8			mac_id,
	struct cmn_sta_info *pcmn_sta_info
)
{
	dm->phydm_sta_info[mac_id] = pcmn_sta_info;

	if (is_sta_active(pcmn_sta_info))
		dm->phydm_macid_table[pcmn_sta_info->mac_id] = mac_id;
}

void phydm_macid2sta_idx_table(
	struct dm_struct	*dm,
	u8			entry_idx,
	struct cmn_sta_info	*pcmn_sta_info
)
{
	if (is_sta_active(pcmn_sta_info))
		dm->phydm_macid_table[pcmn_sta_info->mac_id] = entry_idx;
}

void phydm_add_interrupt_mask_handler(
	struct dm_struct		*dm,
	u8							interrupt_type
)
{
#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
#elif (DM_ODM_SUPPORT_TYPE == ODM_AP)

	struct rtl8192cd_priv	*priv = dm->priv;

#if IS_EXIST_PCI || IS_EXIST_EMBEDDED
	GET_HAL_INTERFACE(priv)->AddInterruptMaskHandler(priv, interrupt_type);
#endif

#elif (DM_ODM_SUPPORT_TYPE == ODM_CE)
#endif
}

void phydm_enable_rx_related_interrupt_handler(
	struct dm_struct		*dm
)
{
#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
#elif (DM_ODM_SUPPORT_TYPE == ODM_AP)

	struct rtl8192cd_priv	*priv = dm->priv;

#if IS_EXIST_PCI || IS_EXIST_EMBEDDED
	GET_HAL_INTERFACE(priv)->EnableRxRelatedInterruptHandler(priv);
#endif

#elif (DM_ODM_SUPPORT_TYPE == ODM_CE)
#endif
}

#if 0
boolean phydm_get_txbf_en(
	struct dm_struct		*dm,
	u16							mac_id,
	u8							i
)
{
	boolean txbf_en = false;

#if (DM_ODM_SUPPORT_TYPE & ODM_WIN)
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE) && !defined(DM_ODM_CE_MAC80211)

#ifdef CONFIG_BEAMFORMING
	enum beamforming_cap beamform_cap;
	void *adapter = dm->adapter;
#if (BEAMFORMING_SUPPORT == 1)
	beamform_cap =
		phydm_beamforming_get_entry_beam_cap_by_mac_id(dm, mac_id);
#else/*for drv beamforming*/
	beamform_cap =
		beamforming_get_entry_beam_cap_by_mac_id(&adapter->mlmepriv, mac_id);
#endif
	if (beamform_cap & (BEAMFORMER_CAP_HT_EXPLICIT | BEAMFORMER_CAP_VHT_SU))
		txbf_en = true;
	else
		txbf_en = false;
#endif /*#ifdef CONFIG_BEAMFORMING*/

#elif (DM_ODM_SUPPORT_TYPE & ODM_AP)

#if (BEAMFORMING_SUPPORT == 1)
	u8 idx = 0xff;
	boolean act_bfer = false;
	BEAMFORMING_CAP beamform_cap = BEAMFORMING_CAP_NONE;
	PRT_BEAMFORMING_ENTRY	entry = NULL;
	struct rtl8192cd_priv *priv			= dm->priv;
#if (defined(CONFIG_PHYDM_ANTENNA_DIVERSITY))
	struct _BF_DIV_COEX_	*dm_bdc_table = &dm->dm_bdc_table;

	dm_bdc_table->num_txbfee_client = 0;
	dm_bdc_table->num_txbfer_client = 0;
#endif
#endif

#if (BEAMFORMING_SUPPORT == 1)
	beamform_cap = Beamforming_GetEntryBeamCapByMacId(priv, mac_id);
	entry = Beamforming_GetEntryByMacId(priv, mac_id, &idx);
	if (beamform_cap & (BEAMFORMER_CAP_HT_EXPLICIT | BEAMFORMER_CAP_VHT_SU)) {
		if (entry->Sounding_En)
			txbf_en = true;
		else
			txbf_en = false;
		act_bfer = true;
	}
#if (defined(CONFIG_PHYDM_ANTENNA_DIVERSITY)) /*BDC*/
	if (act_bfer == true) {
		dm_bdc_table->w_bfee_client[i] = true; /* AP act as BFer */
		dm_bdc_table->num_txbfee_client++;
	} else
		dm_bdc_table->w_bfee_client[i] = false; /* AP act as BFer */

	if (beamform_cap & (BEAMFORMEE_CAP_HT_EXPLICIT | BEAMFORMEE_CAP_VHT_SU)) {
		dm_bdc_table->w_bfer_client[i] = true; /* AP act as BFee */
		dm_bdc_table->num_txbfer_client++;
	} else
		dm_bdc_table->w_bfer_client[i] = false; /* AP act as BFer */

#endif
#endif

#endif
	return txbf_en;

}
#endif

void phydm_iqk_wait(
	struct dm_struct		*dm,
	u32		timeout
)
{
#if (DM_ODM_SUPPORT_TYPE == ODM_CE)
#if (DM_ODM_SUPPORT_TYPE & ODM_CE) && defined(DM_ODM_CE_MAC80211)
	PHYDM_DBG(dm, ODM_COMP_COMMON, "Not support for CE MAC80211 driver!\n");
#else
	void		*adapter = dm->adapter;

	rtl8812_iqk_wait(adapter, timeout);
#endif
#endif
}
