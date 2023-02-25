/*
    Copyright 2000-2011 Broadcom Corporation

    <:label-BRCM:2011:DUAL/GPL:standard
    
    Unless you and Broadcom execute a separate written software license
    agreement governing use of this software, this software is licensed
    to you under the terms of the GNU General Public License version 2
    (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
    with the following added to such license:
    
       As a special exception, the copyright holders of this software give
       you permission to link this software with independent modules, and
       to copy and distribute the resulting executable under terms of your
       choice, provided that you also meet, for each linked independent
       module, the terms and conditions of the license of that module.
       An independent module is a module which is not derived from this
       software.  The special exception does not apply to any modifications
       of the software.
    
    Not withstanding the above, under no circumstances may you combine
    this software in any way with any other Broadcom software provided
    under a license other than the GPL, without Broadcom's express prior
    written consent.
    
    :>
*/

/***********************************************************************/
/*                                                                     */
/*   MODULE:  bcmnet.h                                                 */
/*   DATE:    05/16/02                                                 */
/*   PURPOSE: network interface ioctl definition                       */
/*                                                                     */
/***********************************************************************/
#ifndef _IF_NET_H_
#define _IF_NET_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <linux/sockios.h>
#if defined(CONFIG_COMPAT)
#include <linux/compat.h>
#endif
//#include "skb_defines.h"
//#include "bcmPktDma_defines.h"
#include "bcmtypes.h"

#define LINKSTATE_DOWN      0
#define LINKSTATE_UP        1

#ifndef IFNAMSIZ
#define IFNAMSIZ  16
#endif

/*---------------------------------------------------------------------*/
/* Ethernet Switch Type                                                */
/*---------------------------------------------------------------------*/
#define ESW_TYPE_UNDEFINED                  0
#define ESW_TYPE_BCM5325M                   1
#define ESW_TYPE_BCM5325E                   2
#define ESW_TYPE_BCM5325F                   3
#define ESW_TYPE_BCM53101                   4

#define ETHERNET_ROOT_DEVICE_NAME     "bcmsw"

/*
 * Ioctl definitions.
 */
/* Note1: The maximum device private ioctls allowed are 16 */
/* Note2: SIOCDEVPRIVATE is reserved */
enum {
    SIOCGLINKSTATE = SIOCDEVPRIVATE + 1,
    SIOCSCLEARMIBCNTR,
    SIOCMIBINFO,
    SIOCGENABLEVLAN,
    SIOCGDISABLEVLAN,
    SIOCGQUERYNUMVLANPORTS,
    SIOCGQUERYNUMPORTS,
    SIOCPORTMIRROR,
    SIOCSWANPORT,
    SIOCGWANPORT,
    SIOCETHCTLOPS,
    SIOCGPONIF,
    SIOCETHSWCTLOPS,
    SIOCGSWITCHPORT,
    SIOCIFREQ_EXT,
    SIOCLAST,
};

/* Various operations through the SIOCETHCTLOPS */
enum {
    ETHGETNUMTXDMACHANNELS = 0,
    ETHSETNUMTXDMACHANNELS,
    ETHGETNUMRXDMACHANNELS,
    ETHSETNUMRXDMACHANNELS,   
    ETHGETSOFTWARESTATS,
    ETHGETMIIREG,
    ETHSETMIIREG,
    ETHSETLINKSTATE,
    ETHGETCOREID,
    ETHGETPHYEEE,
    ETHSETPHYEEEON,
    ETHSETPHYEEEOFF,
    ETHGETPHYEEERESOLUTION,
    ETHSETPHYWOLSLEEP,
    ETHGETPHYAPD,
    ETHSETPHYAPDON,
    ETHSETPHYAPDOFF,
    ETHGETPHYPWR,
    ETHSETPHYPWRON,
    ETHSETPHYPWROFF,
    ETHMOVESUBPORT,
    ETHPHYMAP,
    ETHGETPHYID,
    ETHG9991CARRIERON,
    ETHG9991CARRIEROFF,
    ETHCDGET,
    ETHCDSET,
    ETHCDRUN,
    ETHGETSFPINFO,
    ETHPHYMACSEC,
    ETHWIRESPEEDGET,
    ETHWIRESPEEDSET,
};

struct ethctl_data {
    /* ethctl ioctl operation */
    int op;
    /* number of DMA channels */
    int num_channels;
    /* return value for get operations */
    int ret_val;
    /* value for set operations */
    int val;
    int sub_port;
    unsigned int phy_addr;
    unsigned int phy_reg;
    /* flags to indicate to ioctl functions */
    unsigned int flags;
    union {
        char ifname[IFNAMSIZ];
#define ETHCTL_FLAG_MPMAC_SET    (1<<0)
        char mpmac[6];
    };
    BCM_IOC_PTR(char *, buf);
    int buf_size;
    int pair_len[4];
};

typedef enum {
    SFP_TYPE_XPON,
    SFP_TYPE_ETHERNET,
    SFP_TYPE_UNKNOWN,
    SFP_TYPE_NOT_ETHERNET,  /* Active Ethernet Port not defined in board paramters */
    SFP_TYPE_NO_MODULE,
} sfp_type_t;

enum {
    CD_INVALID,
    CD_OK,
    CD_OPEN,
    CD_INTRA_SHORT,
    CD_INTER_SHORT,
    CD_ENABLED,
    CD_DISABLED,
    CD_NOT_SUPPORTED,
    CD_ALL_PAIR_OK=0x1111,
    CD_ALL_PAIR_OPEN=0x2222,
};
#define CD_CODE_PAIR_GET(v, p)          (((v)>>((p)*4))&0xf)
#define CD_CODE_PAIR_SET(v, p)          (((v)&0xf)<<((p)*4))

/* Flags for Cable Diagnostic */
#define INTERFACE_NEXT    (1<<0)
#define CD_LINK_UP           (1<<1)
/* ethctl ret_val definitions */
enum {
    ETHCTL_RET_OK   = 0,
    ETHMOVESUBPORT_RET_INVALID_EP,
    ETHMOVESUBPORT_RET_SRCDEV_UP,
    ETHMOVESUBPORT_RET_DSTDEV_UP,
    ETHMOVESUBPORT_RET_MAC2MAC,
    ETHMOVESUBPORT_RET_NOT_MOVEABLE,
};

/* PHY type */
enum {
    ETHCTL_FLAG_ACCESS_INT_PHY              = 0,
    ETHCTL_FLAG_ACCESS_EXT_PHY              = (1<<0),
    ETHCTL_FLAG_ACCESS_EXTSW_PHY            = (1<<1),
    ETHCTL_FLAG_ACCESS_I2C_PHY              = (1<<2),
    ETHCTL_FLAG_ACCESS_SERDES               = (1<<3),
    ETHCTL_FLAG_ACCESS_SERDES_POWER_MODE    = (1<<4),
    ETHCTL_FLAG_ACCESS_32BIT                = (1<<5),
    ETHCTL_FLAG_ACCESS_10GSERDES            = (1<<6),
    ETHCTL_FLAG_ACCESS_10GPCS               = (1<<7),
    ETHCTL_FLAG_ACCESS_SILENT_START         = (1<<8),
};
#define ETHCTL_FLAG_ANY_SERDES  (ETHCTL_FLAG_ACCESS_SERDES|ETHCTL_FLAG_ACCESS_10GSERDES)

enum ethctl_error {
    ETHCTL_ERROR_POWER_SAVING_DOWN = -100,
    ETHCTL_ERROR_POWER_ADMIN_DOWN = -101,
};

enum {
    SERDES_NO_POWER_SAVING, 
    SERDES_BASIC_POWER_SAVING, 
    SERDES_ADVANCED_POWER_SAVING, 
    SERDES_FORCE_OFF,
    SERDES_POWER_MODE_MAX};

/* Various operations through the SIOCGPONIF */
enum {
    GETFREEGEMIDMAP = 0,
    SETGEMIDMAP,
    GETGEMIDMAP,
    CREATEGPONVPORT,
    DELETEGPONVPORT,
    DELETEALLGPONVPORTS,
    SETMCASTGEMID,
};

struct interface_data{
    char ifname[IFNAMSIZ];
    int switch_port_id;
};

/* Definition for IFREQ extension structure to
   support more IFREQs than kernel allocated 16 types */
struct ifreq_ext {
    int opcode;
    /* add union struct for different opcode's data below if needed */
    union 
    {
        struct {
            BCM_IOC_PTR(char *, stringBuf);
            int bufLen;
        };
    };
};
typedef struct ifreq_ext ifreq_ext_t;

/* Definition for opcode */
enum
{
    SIOCGPORTWANONLY,
    SIOCGPORTWANPREFERRED,
    SIOCGPORTLANONLY,
};

/* The enet driver subdivides queue field (mark[4:0]) in the skb->mark into
   priority and channel */
/* priority = queue[2:0] (=>mark[2:0]) */
#define SKBMARK_Q_PRIO_S        (SKBMARK_Q_S)
#define SKBMARK_Q_PRIO_M        (0x07 << SKBMARK_Q_PRIO_S)
#define SKBMARK_GET_Q_PRIO(MARK) \
    ((MARK & SKBMARK_Q_PRIO_M) >> SKBMARK_Q_PRIO_S)
#define SKBMARK_SET_Q_PRIO(MARK, Q) \
    ((MARK & ~SKBMARK_Q_PRIO_M) | (Q << SKBMARK_Q_PRIO_S))
/* channel = queue[4:3] (=>mark[4:3]) */
#define SKBMARK_Q_CH_S          (SKBMARK_Q_S + 3)
#define SKBMARK_Q_CH_M          (0x03 << SKBMARK_Q_CH_S)
#define SKBMARK_GET_Q_CHANNEL(MARK) ((MARK & SKBMARK_Q_CH_M) >> SKBMARK_Q_CH_S)
#define SKBMARK_SET_Q_CHANNEL(MARK, CH) \
    ((MARK & ~SKBMARK_Q_CH_M) | (CH << SKBMARK_Q_CH_S))

#define SPEED_10MBIT        10000000
#define SPEED_100MBIT       100000000
#define SPEED_200MBIT       200000000
#define SPEED_1000MBIT      1000000000
#define SPEED_2500MBIT      2500000000u
#define SPEED_5000MBIT      5000000000ull
#define SPEED_10000MBIT     10000000000ull
#define SPEED_DOWN          0

#define BCMNET_DUPLEX_HALF         0
#define BCMNET_DUPLEX_FULL         1

// Use for Auto negotiation capability
#define AN_10M_HALF           0x0001
#define AN_10M_FULL           0x0002
#define AN_100M_HALF          0x0004
#define AN_100M_FULL          0x0008
#define AN_1000M_HALF         0x0010
#define AN_1000M_FULL         0x0020
#define AN_2500               0x0040
#define AN_5000               0x0080
#define AN_10000              0x0100
#define AN_AUTONEG            0x0200
#define AN_FLOW_CONTROL       0x0400
#define AN_FLOW_CONTROL_ASYM  0x0800
#define AN_REPEATER           0x1000

#define AUTONEG_CTRL_MASK 0x01
#define AUTONEG_RESTART_MASK 0x02


typedef struct IoctlMibInfo
{
    uint32 ulIfLastChange;
    uint64 ulIfSpeed;
    uint32 ulIfDuplex;
} IOCTL_MIB_INFO, *PIOCTL_MIB_INFO;


#define MIRROR_INTF_SIZE    32
#define MIRROR_DIR_IN       0
#define MIRROR_DIR_OUT      1
#define MIRROR_DISABLED     0
#define MIRROR_ENABLED      1

typedef struct _MirrorCfg
{
    char szMonitorInterface[MIRROR_INTF_SIZE];
    char szMirrorInterface[MIRROR_INTF_SIZE];
    int nDirection;
    int nStatus;
#if defined(DMP_X_ITU_ORG_GPON_1) && defined(CONFIG_BCM_MAX_GEM_PORTS)
    /* +1 is when CONFIG_BCM_MAX_GEM_PORTS is not a multiple of 8 */
    unsigned char nGemPortMaskArray[(CONFIG_BCM_MAX_GEM_PORTS/8)+1]; 
#endif
} MirrorCfg ;

int sfp_i2c_phy_read( int reg, int *data);
int sfp_i2c_phy_write( int reg, int data);

int bcmeapi_init_wan(void);

/* VLAN TPIDs that need to be checked
   ETH_P_8021Q  0x8100
   ETH_P_8021AD 0x88A8
   ETH_P_QINQ1  0x9100
   ETH_P_QINQ2  0x9200
 */
#define BCM_VLAN_TPID_CHECK(x) ( (x) == htons(ETH_P_8021Q) \
                                || (x) == htons(ETH_P_8021AD)  \
                             /* || (x) == htons(ETH_P_QINQ1) */\
                             /* || (x) == htons(ETH_P_QINQ2) */)

#define check_arp_lcp_pkt(pkt_p, ret_val)   {                                                                       \
            unsigned char l3_offset = sizeof(struct ethhdr);                                                        \
            struct vlan_hdr    *vlanhdr = (struct vlan_hdr *)(pkt_p + l3_offset - sizeof(struct vlan_hdr));         \
            ret_val = 0;                                                                                            \
            /* Skip over all the VLAN Tags */                                                                       \
            while ( BCM_VLAN_TPID_CHECK(vlanhdr->h_vlan_encapsulated_proto) )                                       \
            {                                                                                                       \
                vlanhdr = (struct vlan_hdr *)(pkt_p + l3_offset);                                                   \
                l3_offset +=  sizeof(struct vlan_hdr);                                                              \
            }                                                                                                       \
            if (vlanhdr->h_vlan_encapsulated_proto == htons(ETH_P_ARP))                                             \
            {                                                                                                       \
                ret_val = 1;                                                                                        \
            }                                                                                                       \
            else if ( vlanhdr->h_vlan_encapsulated_proto == htons(ETH_P_PPP_DISC) )                                 \
            {                                                                                                       \
                ret_val = 1;                                                                                        \
            }                                                                                                       \
            else if ( vlanhdr->h_vlan_encapsulated_proto == htons(ETH_P_PPP_SES) )                                  \
            {                                                                                                       \
                struct pppoe_hdr *pppoe = (struct pppoe_hdr *)(pkt_p + l3_offset);                                  \
                                                                                                                    \
                if ( ! (pppoe->tag[0].tag_type  == htons(PPP_IP) || pppoe->tag[0].tag_type  == htons(PPP_IPV6)) )   \
                {                                                                                                   \
                    ret_val = 1;                                                                                    \
                }                                                                                                   \
            }                                                                                                       \
    }

enum Bcm63xxEnetStats {
    ET_TX_BYTES = 0,
    ET_TX_PACKETS,
    ET_TX_ERRORS,
    ET_TX_CAPACITY,
    ET_RX_BYTES,
    ET_RX_PACKETS, 
    ET_RX_ERRORS,
    ET_MAX
};

typedef struct mac_limit_arg{
    uint32 cmd;
    uint32 val;
    union {
        void *mac_limit;
        char rsvd[8];
    };
}mac_limit_arg_t;

enum mac_limit_cmd{
    MAC_LIMIT_IOCTL_GET = 0,
    MAC_LIMIT_IOCTL_SET,
    MAC_LIMIT_IOCTL_CLR,
    MAC_LIMIT_IOCTL_EN
};

enum mac_limit_set_op{
    MAC_LIMIT_SET_MAX = 0,
    MAC_LIMIT_SET_MIN,
};

/*------------------------------------------------------------------------*/
/* BCM net character device for ioctl to get/set netdev BRCM private info */
/*------------------------------------------------------------------------*/
#define BCMNET_DRV_MAJOR            377
#define BCMNET_DRV_NAME             "bcmnet"
#define BCMNET_DRV_DEVICE_NAME      "/dev/" BCMNET_DRV_NAME

typedef enum bcmnet_ioctl_cmd
{
    BCMNET_IOCTL_GET_EXT_FLAGS,
    BCMNET_IOCTL_GET_LAST_CHANGE,
    BCMNET_IOCTL_CLR_STATS,
    BCMNET_IOCTL_ADD_NETDEV_PATH,
    BCMNET_IOCTL_MAC_LIMIT,
    BCMNET_IOCTL_MAX
} bcmnet_ioctl_cmd_t;

typedef struct {
    unsigned int unused : 25;
    unsigned int is_bcm_dev : 1;
    unsigned int is_wlan : 1;
    unsigned int is_hw_switch : 1;
    unsigned int is_hw_fdb : 1;
    unsigned int is_ppp : 1;
    unsigned int is_vlan : 1;
    unsigned int is_wan : 1;
} bcmnet_extflags;

typedef struct {
    char if_name[IFNAMSIZ];
    union {
        struct {
            bcmnet_extflags ret_val;
        }st_get_ext_flags;
        struct {
            unsigned long last_change;
        }st_get_last_change;
        struct {
            char next_if_name[IFNAMSIZ];
        }st_add_netdev_path;
        mac_limit_arg_t st_mac_limit;
    };
}bcmnet_info_t;

#if defined(CONFIG_COMPAT)
typedef struct mac_limit_compat_arg{
    uint32 cmd;
    uint32 val;
    union {
        compat_uptr_t mac_limit;
        char rsvd[8];
    };
}compat_mac_limit_arg_t;

typedef struct {
    char if_name[IFNAMSIZ];
    union {
        struct {
            bcmnet_extflags ret_val;
        }st_get_ext_flags;
        struct {
            unsigned long last_change;
        }st_get_last_change;
        struct {
            char next_if_name[IFNAMSIZ];
        }st_add_netdev_path;
        compat_mac_limit_arg_t st_mac_limit;
    };
}compat_bcmnet_info_t;
#endif

#if !defined(__KERNEL__)
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>


// return 0 if successful, -1 - error
static inline int bcmnet_ioctl_get_ext_flags(char const* if_name, bcmnet_extflags* flags)
{
    bcmnet_info_t info;
    int fd, err;

    fd = open(BCMNET_DRV_DEVICE_NAME, O_RDWR);
    if (fd < 0)
        return -1;

    strncpy(info.if_name, if_name, IFNAMSIZ);
    err = ioctl(fd, BCMNET_IOCTL_GET_EXT_FLAGS, &info);
    close(fd);
    if (err == -1)
        return err;
    *flags = info.st_get_ext_flags.ret_val;
    return 0;
}

// return 1 - dev is WAN, 0 - dev is not WAN, -1 - error
static inline int bcmnet_ioctl_iswandev(char const* if_name)
{
    bcmnet_extflags flags;
    int ret;

    ret = bcmnet_ioctl_get_ext_flags(if_name, &flags);
    if (ret < 0)
        return -1;

    return flags.is_wan;
}

// return 1 - dev is VLAN, 0 - dev is not VLAN, -1 - error
static inline int bcmnet_ioctl_isvlandev(char const* if_name)
{
    bcmnet_extflags flags;
    int ret;

    ret = bcmnet_ioctl_get_ext_flags(if_name, &flags);
    if (ret < 0)
        return -1;

    return flags.is_vlan;
}

// return last transction jiffies, 0 if there is error
static inline unsigned long bcmnet_ioctl_get_last_change(char* if_name)
{
    bcmnet_info_t info;
    int fd, err;

    fd = open(BCMNET_DRV_DEVICE_NAME, O_RDWR);
    if (fd < 0)
        return 0;

    strncpy(info.if_name, if_name, IFNAMSIZ);
    err = ioctl(fd, BCMNET_IOCTL_GET_LAST_CHANGE, &info);
    close(fd);
    if (err == -1)
        return 0;
    return info.st_get_last_change.last_change;
}

static inline void bcmnet_ioctl_clr_stat(char* if_name)
{
    bcmnet_info_t info;
    int fd;

    fd = open(BCMNET_DRV_DEVICE_NAME, O_RDWR);
    if (fd < 0)
        return;

    strncpy(info.if_name, if_name, IFNAMSIZ);
    ioctl(fd, BCMNET_IOCTL_CLR_STATS, &info);
    close(fd);
}

static inline int bcmnet_ioctl_add_netdev_path(char const* dev_name, char const* next_dev_name)
{
    bcmnet_info_t info;
    int fd, err;

    fd = open(BCMNET_DRV_DEVICE_NAME, O_RDWR);
    if (fd < 0)
        return -1;

    strncpy(info.if_name, dev_name, IFNAMSIZ);
    strncpy(info.st_add_netdev_path.next_if_name, next_dev_name, IFNAMSIZ);
    err = ioctl(fd, BCMNET_IOCTL_ADD_NETDEV_PATH, &info);
    close(fd);
    return err;
}

#endif //!__KERNEL__

#ifdef __cplusplus
}
#endif

#endif /* _IF_NET_H_ */
