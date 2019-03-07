/***********************  I n c l u d e  -  F i l e  ************************/
/*!
 *        \file  m77_drv.h
 *
 *      \author  christophe.hannoyer@men.de
 *        $Date: 2009/06/24 16:18:15 $
 *    $Revision: 1.1 $
 *
 *       \brief  Header file for xxx
 *
 *     Switches: -
 */
/*-------------------------------[ History ]---------------------------------
 *
 * $Log: m77_drv.h,v $
 * Revision 1.1  2009/06/24 16:18:15  channoyer
 * Initial Revision
 *
 *---------------------------------------------------------------------------
 * (c) Copyright 2009 by MEN Mikro Elektronik GmbH, Nuremberg, Germany
 ****************************************************************************/

#ifndef _M77_DRV_H_
#define _M77_DRV_H_

#ifdef __cplusplus
    extern "C" {
#endif

/*--------------------------------------+
|   DEFINES                             |
+--------------------------------------*/

    /* offsets for main register */
#define M77_HOLD_DLL_REG        0x00
#define M77_IER_DLM_REG         0x01
#define M77_ISR_FCR_REG         0x02
#define M77_LCR_REG             0x03
#define M77_MCR_REG             0x04
#define M77_LSR_REG             0x05
#define M77_MSR_REG             0x06
#define M77_SPR_REG             0x07
#define M77_REG_TOTAL           8

    /* offset for 650 compatible registers */
#define M77_EFR_OFFSET          0x02    /* enhanced features register */
#define M77_XON1_OFFSET         0x04    /* XON1 flow control character */
#define M77_XON2_OFFSET         0x05    /* XON2 flow control character */
#define M77_XOFF1_OFFSET        0x06    /* XOFF1 flow control character */
#define M77_XOFF2_OFFSET        0x07    /* XOFF2 flow control character */
#define M77_LCR_650_ACCESS_KEY  0xbf    /* access key for
                                           650 compatible registers */

    /* equates for extended feature register */
#define M77_EFR_FLOWCTL_CTS     0x80    /* automatic CTS Flow control enable */
#define M77_EFR_FLOWCTL_RTS     0x40    /* automatic RTS Flow control enable */
#define M77_EFR_SPEC_CHAR_DET   0x20    /* special character detect enable */
#define M77_EFR_ENHANCED_MODE   0x10    /* Enhanced mode enable */

    /* offsets for 950 specific registers */
#define M77_ASR_OFFSET          0x01    /* additional status register */
#define M77_RFL_OFFSET          0x03    /* receiver FIFO fill level */
#define M77_TFL_OFFSET          0x04    /* transmitter FIFO fill level */
#define M77_ICR_OFFSET          0x05    /* indexed control register */
#define M77_ACR_OFFSET          0x00    /* index of ARC register */
#define M77_CPR_OFFSET          0x01    /* clock prescaler register */
#define M77_TCR_OFFSET          0x02    /* times clock register */
#define M77_CKS_OFFSET          0x03    /* clock select register */
#define M77_TTL_OFFSET          0x04    /* transmitter int trigger level */
#define M77_RTL_OFFSET          0x05    /* receiver int trigger level */
#define M77_FCL_OFFSET          0x06    /* automatic flow control
                                           lower trigger level */
#define M77_FCH_OFFSET          0x07    /* automatic flow control
                                           higher trigger level */
#define M77_CSR_INDEX           0x0c    /* index of CSR register */
#define M77_ID1_OFFSET          0x08    /* hardwired ID byte 1 */
#define M77_ID2_OFFSET          0x09    /* hardwired ID byte 2 */
#define M77_ID3_OFFSET          0x0a    /* hardwired ID byte 3 */
#define M77_REV_OFFSET          0x0b    /* hardwired revision byte */

    /* equates for additional control register */
#define M77_ACR_RX_DIS          0x01    /* enable receiver */
#define M77_ACR_TX_DIS          0x02    /* enable transmitter */
#define M77_ACR_AUTO_DSR_EN     0x04    /* enable auto DSR flow control */
#define M77_ACR_DTR_MASK        0x18    /* mask bits setting function
                                           of DTR pin */
#define M77_ACR_DTR_NORM        0x00    /* DTR: compatible to 16C450, C550,
                                           C650, C750 */
#define M77_ACR_DTR_RX_CTL      0x08    /* DTR: automatic flow control */
#define M77_ACR_DTR_HD_L_DRIVER_CTL 0x10  /* DTR: activate line driver control
                                             in HD-mode, low active EN pin */
#define M77_ACR_DTR_HD_H_DRIVER_CTL 0x18  /* DTR: activate line driver control
                                             in HD-mode, high active EN pin */
#define M77_ACR_950_TRIG_EN     0x20    /* enable 950 interrupt trigger level */
#define M77_ACR_ICR_READ_EN     0x40    /* enable read of ICR register */
#define M77_ACR_950_READ_EN     0x80    /* enable read of 950 registers */



    /* equates for FIFO Control Register */
#define FCR_EN                  0x01    /* enable xmit and rcvr */
#define FIFO_ENABLE             FCR_EN
#define FCR_RXCLR               0x02    /* clears rcvr fifo */
#define RxCLEAR                 FCR_RXCLR
#define FCR_TXCLR               0x04    /* clears xmit fifo */
#define TxCLEAR                 FCR_TXCLR
#define FCR_DMA                 0x08    /* dma */
#define FCR_RXTRIG_L            0x40    /* rcvr fifo trigger lvl low */
#define FCR_RXTRIG_H            0x80    /* rcvr fifo trigger lvl high */
#define FCR_DMA_MODE            0x08    /* Set to use 550 Tx Trigger Levels */

        /* receiver FIFO trigger level */
#define FCR_RX_TRIGGER_16       0x00    /*  16 Byte */
#define FCR_RX_TRIGGER_32       0x40    /*  32 Byte */
#define FCR_RX_TRIGGER_112      0x80    /* 112 Byte */
#define FCR_RX_TRIGGER_120      0xC0    /* 120 Byte */
        /* transmitter FIFO trigger level */
#define FCR_TX_TRIGGER_16       0x00    /*  16 Byte */
#define FCR_TX_TRIGGER_32       0x10    /*  32 Byte */
#define FCR_TX_TRIGGER_64       0x20    /*  64 Byte */
#define FCR_TX_TRIGGER_112      0x30    /* 112 Byte */

#define M77_FIFO_ENABLE         0x01
#define M77_FIFO_DISABLE        0x00


    /* equates for Line Control Register */
#define M77_LCR_CS5             0x00    /* 5 bits data size */
#define M77_LCR_CS6             0x01    /* 6 bits data size */
#define M77_LCR_CS7             0x02    /* 7 bits data size */
#define M77_LCR_CS8             0x03    /* 8 bits data size */
#define M77_LCR_1_STB           0x00
#define M77_LCR_2_STB           0x04
#define M77_LCR_PEN             0x08    /* parity enable */
#define M77_LCR_PDIS            0x00    /* parity disable */
#define M77_LCR_EPS             0x10    /* even parity slect */
#define M77_LCR_SP              0x20    /* stick parity select */
#define M77_LCR_SBRK            0x40    /* break control bit */
#define M77_LCR_DLAB            0x80    /* divisor latch access enable */
#define M77_LCR_DL_ACCESS_KEY   0x80

    /* equates for interrupt enable register */
#define M77_IER_RXRDY           0x01    /* receiver data ready */
#define M77_IER_TBE             0x02    /* transmit bit enable */
#define M77_IER_LSI             0x04    /* line status interrupts */
#define M77_IER_MSI             0x08    /* modem status interrupts */

    /* equates for interrupt status register */
#define M77_ISR_MASK            0x3f    /* ISR mask to cut of INT level 6 */
#define M77_ISR_NO_INT_PEND     0x01    /* ISR value if no int is pending */
#define M77_ISR_RX_ERROR        0x06    /* ISR value for int lavel 1 RX Error */
#define M77_ISR_RX_DATA_AVAIL   0x04    /* ISR value for int level 2a RX data avail */
#define M77_ISR_RX_TIMEOUT      0x0c    /* ISR value for int level 2b RX timeout */
#define M77_ISR_THR_EMPTY       0x02    /* ISR value for int level 3 TX queue empty */
#define M77_ISR_MODEM_STATE     0x00    /* ISR value for int level 4 Modem status change */
#define M77_ISR_INBAND_XOFF     0x10    /* ISR value for int level 5 In-ban flow control XOFF */
#define M77_ISR_RTSCTS_STATE    0x20    /* ISR value for int level 6 RTS or CTS change of state */

    /* equates for line status register */
#define M77_LSR_RXRDY           0x01    /* bit mask for RxRDY in LSR reg */
#define M77_LSR_OE              0x02
#define M77_LSR_PE              0x04
#define M77_LSR_FE              0x08
#define M77_LSR_BI              0x10
#define M77_LSR_TXRDY           0x20
#define M77_LSR_TSRE            0x40
#define M77_LSR_RCV_FIFO        0x80

    /* equates for modem control register */
#define M77_MCR_DTR             0x01    /* mask to access DTR bit */
#define M77_MCR_RTS             0x02    /* mask to access RTS bit */
#define M77_MCR_INT_EN          0x08    /* external interrupt enable */
#define M77_MCR_LOOP            0x10    /* internal loopback enable */
#define M77_MCR_NORM_RTS_CTS_EN 0x20    /* normal mode: enable RTS and CTS flow control */
#define M77_MCR_ENH_XONALL_EN   0x20    /* enhanced mode: enable XON-Any flow control */

    /* equates for modem status register */
#define M77_MSR_DCTS            0x01    /* CTS changed */
#define M77_MSR_DDSR            0x02    /* DSR changed */
#define M77_MSR_TRI             0x04    /* trailing Ring Indicator edge */
#define M77_MSR_DDCD            0x08    /* DCD changed */
#define M77_MSR_CTS             0x10    /* CTS status */
#define M77_MSR_DSR             0x20    /* DSR status */
#define M77_MSR_RI              0x40    /* Ring Indicator status */
#define M77_MSR_DCD             0x80    /* DCD status */


    /* Size of complete register block (offset of second controller on M45N) */
#define M45N_CTRL_REG_BLOCK_SIZE 0x80

    /* offset for M77 HW line driver configuration register & ISR register */
#define M77_DCR_REG_BASE        0x40    /* offset of Driver Configuration Register */
#define M77_IRQ_REG             0x48
#define M45N_TCR_REG            0x40    /* offset of Tristate Contol Register 1*/
#define M45N_TCR1_REG           0x40    /* offset of Tristate Contol Register 1*/
#define M45N_TCR2_REG           (0x40 + M45N_CTRL_REG_BLOCK_SIZE)

    /* equates for M77 configuration/interrupt register */
#define M77_IRQ_CLEAR           0x01
#define M77_IRQ_EN              0x02
#define M77_TX_EN               0x04

    /* equates for M45N tristate configuration register */
#define M45N_TCR1_CHAN0         0x01    /* channel 0 is tristate */
#define M45N_TCR1_CHAN1         0x02    /* channel 1 is tristate */
#define M45N_TCR1_CHAN23        0x04    /* channels 2 and 3 are tristate */
#define M45N_TCR2_CHAN45        0x01    /* channels 4 and 5 are tristate */
#define M45N_TCR2_CHAN67        0x02    /* channels 6 and 7 are tristate */

/* ioctl function */

/*  M77 special ioctl functions for echo Modes */
#define M77_ECHO_SUPPRESS       __DIOT(_CMD_IOCTL_TTY, 240, int)

/*  M77 special ioctl function for physical Modes */
#define M77_PHYS_INT_SET        __DIOT(_CMD_IOCTL_TTY, 241, int)

/*  M45N special ioctl for Tristate Modes */
#define M45_TIO_TRI_MODE        __DIOT(_CMD_IOCTL_TTY, 242, int)

/* ioctl arguments */
#define M77_RS422_HD            0x01    /* M77_PHYS_INT_SET arg for RS422 half duplex */
#define M77_RS422_FD            0x02    /* M77_PHYS_INT_SET arg for RS422 full duplex */
#define M77_RS485_HD            0x03    /* M77_PHYS_INT_SET arg for RS485 half duplex */
#define M77_RS485_FD            0x04    /* M77_PHYS_INT_SET arg for RS485 full duplex */
#define M77_RS232               0x07    /* M77_PHYS_INT_SET arg for RS232 */

#define M77_RX_EN               0x08    /* RX_EN bit mask */

/* automatic Xon Xoff flow control values */
#define M77_XON_CHAR            17      /* Xon character = ^Q */
#define M77_XOFF_CHAR           19      /* Xoff character = ^S */

/* Flag flow control */
#define M77_MODEM_HS_NONE       0x00
#define M77_MODEM_HS_AUTO_RTS   0x01
#define M77_MODEM_HS_AUTO_CTS   0x02
#define M77_MODEM_HS_AUTO_DSR   0x04
#define M77_MODEM_HS_AUTO_DTR   0x08
#define M77_MODEM_HS_XONXOFF    0x10

/* default handshake FIFO levels */
#define M77_DEF_FCL             0x40
#define M77_DEF_FCH             0x64

/* specific Getstat/Setstat codes */
#define M77_GET_MODID           M_DEV_OF+0x00  /**< G  : Get  */
#define M77_GET_UNIT_NUM        M_DEV_OF+0x02  /**< G  : Get the unit number */
#define M77_GET_SW_MODE         M_DEV_OF+0x0F  /**< G  : Get  */

#define M77_GET_BLK_OPTIONS     M_DEV_BLK_OF+0x01  /* G,S: signal */
#if 0
#define M77_GET_OFFSET          M_DEV_OF+0x01  /**< G  : Get  */
#define M77_GET_SIO_HW_OPTS     M_DEV_OF+0x03  /**< G  : Get  */
#define M77_GET_BAUDRATE        M_DEV_OF+0x04  /**< G  : Get  */
#define M77_GET_RX_BUFF_SIZE    M_DEV_OF+0x05  /**< G  : Get  */
#define M77_GET_TX_BUFF_SIZE    M_DEV_OF+0x06  /**< G  : Get  */
#define M77_GET_RX_FIFO_LEVEL   M_DEV_OF+0x07  /**< G  : Get  */
#define M77_GET_TX_FIFO_LEVEL   M_DEV_OF+0x08  /**< G  : Get  */
#define M77_GET_NO_FIFO         M_DEV_OF+0x09  /**< G  : Get  */
#define M77_GET_PHYS_INT        M_DEV_OF+0x0A  /**< G  : Get  */
#define M77_GET_ECHO_SUPPRESS   M_DEV_OF+0x0B  /**< G  : Get  */
#define M77_GET_HS_HI_FIFO_LVL  M_DEV_OF+0x0C  /**< G  : Get  */
#define M77_GET_HS_LO_FIFO_LVL  M_DEV_OF+0x0D  /**< G  : Get  */
#define M77_GET_HANDSHAKE_MODE  M_DEV_OF+0x0E  /**< G  : Get  */
#define M77_SET_SIGNAL          M_DEV_OF+0x10  /* G,S: signal */
#define M77_CLR_SIGNAL          M_DEV_OF+0x11  /*   S: signal */

#define M77_SET_BLK_EVENT       M_DEV_BLK_OF+0x00  /* G,S: signal */
#endif

/* defines for M-Module ID check */
#define MOD_ID_MAGIC            0x5346  /* eeprom identification (magic) */
#define MOD_ID_N_OFFSET         32000   /* offset of module id for
                                           M45N|M69N boards */
#define MOD_ID_M45N             (45 + MOD_ID_N_OFFSET)  /* module id M45N */
#define MOD_ID_M69N             (69 + MOD_ID_N_OFFSET)  /* module id M69N */
#define MOD_ID_M77              77      /* module id M77 */

#define M77_DEFAULT_BAUD        57600

#define M77_DEF_TX_BUFF_SIZE    512
#define M77_DEF_RX_BUFF_SIZE    512
#define M77_DEF_SIO_HW_OPTS     0x0e
#define M77_FIFO_SIZE           128
#define TX_FIFO_LEVEL_DEFAULT   16
#define RX_FIFO_LEVEL_DEFAULT   16

#define M45_MAX_CH     8
#define M69_MAX_CH     4
#define M77_MAX_CH     4

typedef struct
{
    int         rdBufSize;          /* created receive buffer size */
    int         wrtBufSize;         /* created transmitt buffer size */
    u_int8      ch_num;             /* channel number on uart 0...3 */
    u_int32     device_num;         /* number of the M77|M45N|M69N module
                                       in system */
    u_int32     offset;             /* offset address of channel */
    u_int32     baudrate;           /* baudrate */
    u_int8      echo_suppress;      /* echo suppression in HD-Mode */
    u_int8      fifo_enabled;       /* FIFO is enabled */

    u_int16     transmit_level;     /* transmit trigger level */
    u_int16     receive_level;      /* receive trigger level */
    u_int16     sio_hw_opts;        /* UART hardware options */
    u_int16     physInt;            /* settings for physical interface */
    u_int16     hsMode;             /* settings for handshake */
    u_int16     hsInBandMode;       /* settings for in band handshake */
    u_int16     hsHighFifoLevel;    /* flow control upper FIFO level */
    u_int16     hsLowFifoLevel;     /* flow control lower FIFO level */
    u_int8      tristate;           /* M45N tristate mode */

    MACCESS     ma;
} M77_OPTIONS;

typedef struct event_ctrl_blk {
    struct sigevent event;          // event to send to client
    int             rcvid;          // client channels rcvid
} EVENT_CTRL_BLK_T;

#ifdef __cplusplus
    }
#endif

#endif
