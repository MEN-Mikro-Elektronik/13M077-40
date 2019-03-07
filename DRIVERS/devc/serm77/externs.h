/***********************  I n c l u d e  -  F i l e  ************************/
/*!
 *        \file  externs.h
 *
 *      \author  christophe.hannoyer@men.de
 *        $Date: 2009/06/24 16:22:29 $
 *    $Revision: 1.1 $
 *
 *       \brief  Includes the required headers and declares the global data
 *
 *     Switches: -
 */
/*-------------------------------[ History ]---------------------------------
 *
 * $Log: externs.h,v $
 * Revision 1.1  2009/06/24 16:22:29  channoyer
 * Initial Revision
 *
 *---------------------------------------------------------------------------
 * (c) Copyright 2009 by MEN Mikro Elektronik GmbH, Nuremberg, Germany
 ****************************************************************************/

#ifndef _EXTERNS_H
#define _EXTERNS_H

#ifdef __cplusplus
    extern "C" {
#endif

#ifdef DEFN
    #define EXT
    #define INIT1(a)                = { a }
#else
    #define EXT extern
    #define INIT1(a)
#endif

#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <malloc.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/neutrino.h>
#include <termios.h>
#include <devctl.h>
#include <sys/dcmd_chr.h>
#include <sys/iomsg.h>
#include <atomic.h>
#include <hw/inout.h>
#include <sys/io-char.h>
#include <ctype.h>
#include <sys/mman.h>
#include <pthread.h>

#include <MEN/men_typs.h>
#include <MEN/dbg.h>
#include <MEN/oss.h>
#include <MEN/desc.h>
#include <MEN/maccess.h>
#include <MEN/mdis_api.h>
#include <MEN/ll_defs.h>
#include <MEN/ll_entry.h>
#include <MEN/bb_defs.h>
#include <MEN/bb_entry.h>
#include <MEN/bbis_bk.h>
#include <MEN/usr_oss.h>
#include <MEN/mdis_err.h>
#include <MEN/mdis_mk.h>
#include <MEN/mdis_com.h>
#include <MEN/mbuf.h>
#include <MEN/modcom.h>

#include <MEN/m77_drv.h>

#define DBG_MYLEVEL mk_dbglevel
extern u_int32             mk_dbglevel;

#define MAX_DEV_NAME 10

typedef struct dev_m77 {
    TTYDEV          tty;
    struct dev_m77  *next;
    unsigned        intr;
    unsigned        clk;
    unsigned        div;
    unsigned char   rx_fifo;            /* rx fifo size */
    unsigned char   tx_fifo;            /* tx fifo size */
    unsigned char   fifo_override;      /* fifo transmit override */
    uintptr_t       port[M77_REG_TOTAL];

    u_int16         shadowACR;          /* the current value of ACR */
    u_int8          lastErr;            /* represents LSR value */
    u_int8          unit;
    u_int8          uartNum;
    pthread_mutex_t mutexUartMode;
} DEV_M77;

struct dev_list {
    struct dev_list    *next;
    DEV_M77            *device;
    int                iid;
};

typedef struct
{
    char            mdis_dev_name[MAX_DEV_NAME];
    u_int32            unit_num;
    int32           nbrOfChannels;
    M77_OPTIONS     m77opt[8];
    MDIS_PATH_QNX  *mp;
    MDIS_NATDEV_RETURN mdis;
    u_int32         modId;
    u_int32         swMode;
    uintptr_t        module_port;
} MODULE_M77;

EXT TTYCTRL                        ttyctrl;
EXT struct dev_list                *G_devicesP;
EXT MODULE_M77                  *G_modM77P;

/* Value used as an argument to TTO to override the check if the tx buffer
 * is empty.  This should only be used in the isr at the tx interrupt
 * since this is the only time we can be sure that the tx fifo is empty.
 * This is required since there is no way to determine the number of char's
 * in the TX fifo - i.e. the amount of free space in the TX fifo.
 * Since the TX buffer empty flag is cleared in the II REG on both reading
 * the II REG and writing to the TX fifo, there shouldn't be any issues
 * regarding nested TX interrupts overwriting data in the TX fifo. */
#define FIFO_XMIT_OVERRIDE        0x80

#include "proto.h"

#ifdef __cplusplus
    }
#endif

#endif
