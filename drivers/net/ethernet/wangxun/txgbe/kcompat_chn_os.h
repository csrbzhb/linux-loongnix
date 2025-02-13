/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 1999 - 2022  Beijing WangXun Technology Co., Ltd. */

#ifndef _KCOMPAT_CHN_OS_H_
#define _KCOMPAT_CHN_OS_H_

/* This is the SUSE Linux Enterprise distribution specific definitions file.
 * It defines what features need backports for a given version of the SUSE
 * Linux Enterprise kernel.
 *
 * It checks a combination of the LINUX_VERSION code and the
 * SLE_LOCALVERSION_CODE to determine what support the kernel has.
 *
 * It assumes that kcompat_std_defs.h has already been processed, and will
 * #define or #undef any flags that have changed based on backports done by
 * SUSE.
 */

#ifndef LINUX_VERSION_CODE
#error "LINUX_VERSION_CODE is undefined"
#endif

#ifndef KERNEL_VERSION
#error "KERNEL_VERSION is undefined"
#endif

#ifdef CONFIG_UOS_KERNEL
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0))
#define HAVE_ETHTOOL_COALESCE_EXTACK
#if (KERNEL_REVISON > 4)
#define HAVE_ETHTOOL_EXTENDED_RINGPARAMS
#endif
#endif
#endif

#ifdef CONFIG_EULER_KERNEL
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0))
#if (KERNEL_REVISON > 4)
#define HAVE_ETHTOOL_COALESCE_EXTACK
#define HAVE_ETHTOOL_EXTENDED_RINGPARAMS
#endif
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,208))
#define HAVE_DEFINE_SKB_FRAG_OFF
#endif
#endif

#ifdef CONFIG_KYLIN_KERNEL

#if !KERNEL_REVISON
//#error "SLE_KERNEL_REVISION is 0 or undefined"
#endif

#define TXGBE_SUPPORT_KYLIN_FT
#define TXGBE_SUPPORT_KYLIN_LX
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,19,90))
#if (KERNEL_REVISON > 21)
#define HAVE_NETDEV_XMIT_MORE
#endif
#if (KERNEL_REVISON >= 52)
#define HAVE_DEFINE_PTP_SYSTEM
#define HAVE_DEFINE_FLOW_CORRELATION
#define HAVE_DEFINE_SKB_FRAG_OFF
#define HAVE_DEFINE_SKB_FRAG_ADD
#define HAVE_ETH_EXTENDED_HEADLEN
#endif
#endif
#endif

#endif /* _KCOMPAT_SLES_DEFS_H_ */
