/*********************  P r o g r a m  -  M o d u l e ***********************/
/*!
 *        \file  init.c
 *
 *      \author  christophe.hannoyer@men.de
 *        $Date: 2009/07/10 15:48:53 $
 *    $Revision: 1.2 $
 *
 *       \brief  Initialization code
 *
 *     Switches: -
 */
/*-------------------------------[ History ]---------------------------------
 *
 * $Log: init.c,v $
 * Revision 1.2  2009/07/10 15:48:53  channoyer
 * R: HW access failed with x86
 * M: Use mmap_device_memory
 *
 * Revision 1.1  2009/06/24 16:22:40  channoyer
 * Initial Revision
 *
 *---------------------------------------------------------------------------
 * (c) Copyright 2009 by MEN Mikro Elektronik GmbH, Nuremberg, Germany
 ****************************************************************************/

#include "externs.h"

void
set_port(DEV_M77 *devP, unsigned port, unsigned mask, unsigned data) {
    unsigned char c;
    int mutLock;

    mutLock = pthread_mutex_lock( &(devP->mutexUartMode) );
    c = read550(devP, port);
    write550(devP, port, (c & ~mask) | (data & mask));
    mutLock = pthread_mutex_unlock( &(devP->mutexUartMode) );
}

static void
clear_device(DEV_M77 *devP) {
    unsigned char tmp;

    write550(devP, M77_IER_DLM_REG, 0);        // Disable all interrupts
    tmp = read550(devP, M77_LSR_REG);          // Clear Line Status Interrupt
    tmp = read550(devP, M77_HOLD_DLL_REG);     // Clear RX Interrupt
    tmp = read550(devP, M77_HOLD_DLL_REG);     // Clear TX Interrupt
    tmp = read550(devP, M77_MSR_REG);          // Clear Modem Interrupt
}

//
// Clean up the device then add it to the interrupt list and enable it.
//
void
ser_attach_intr(DEV_M77 *devP) {
    struct dev_list    **owner;
    struct dev_list *curr;

    // Clean the device so we get a level change on the intr line to the bus.
    // Enable out2 (gate intr to bus)
    set_port(devP, M77_MCR_REG, M77_MCR_INT_EN, M77_MCR_INT_EN);
    clear_device(devP);

    // Add it to the interrupt list

    owner = &G_devicesP;
    for( ;; ) {
        curr = *owner;
        if(curr == NULL) {
            curr = _smalloc(sizeof(*curr));
            *owner = curr;
            curr->next = NULL;
            curr->device = NULL;
            break;
        }
        if(curr->device->uartNum == devP->uartNum) break;
        owner = &curr->next;
    }
    // Delay interrupts while we're fiddling around with the list
    InterruptMask(devP->intr, -1);
    devP->next = curr->device;
    curr->device = devP;
    InterruptUnmask(devP->intr, -1);

    // Enable ALL interrupt sources.
    write550(devP, M77_IER_DLM_REG, 0x0f);
}


void
ser_detach_intr(DEV_M77 *devP) {
    struct dev_list        **list_owner;
    struct dev_list        *list_curr;
    DEV_M77            **dev_owner;
    DEV_M77            *dev_curr;

    // Disable ALL interrupt sources
    write550(devP, M77_IER_DLM_REG, 0x00);                // Disable interrupts
    set_port(devP, M77_MCR_REG, M77_MCR_INT_EN, 0x00);    // Disable out2

    //
    // Remove from list of devices to scan on an interrupt.
    //

    // Find the right interrupt list
    list_owner = &G_devicesP;
    for( ;; ) {
        list_curr = *list_owner;
        if(list_curr->device->intr == devP->intr) break;
        list_owner = &list_curr->next;
    }

    // Find the right device on the list
    dev_owner = &list_curr->device;
    for( ;; ) {
        dev_curr = *dev_owner;
        if(dev_curr == devP) break;
        dev_owner = &dev_curr->next;
    }
    // Delay interrupts while we're fiddling around with the list
    InterruptMask(devP->intr, -1);
    *dev_owner = devP->next;

    // If no more handlers, detach the interrupt.
    if((*list_owner)->device == NULL) {
        InterruptDetach(list_curr->iid);
        *list_owner = list_curr;
        _sfree(list_curr, sizeof(*list_curr));
    }
    clear_device(devP);
    InterruptUnmask(devP->intr, -1);

    devP->intr = _NTO_INTR_SPARE;
}


void
set_950_mode(DEV_M77 *devP) {
    u_int16 efr;

    /*-----------------------+
    |   reset this channel   |
    +-----------------------*/
    m77ChannelRest(devP);

    /*-------------------------+
    |  set physical interface  |
    +-------------------------*/
    if( G_modM77P->modId == MOD_ID_M77 ) { /* only M77 supports RS485 HD */
        writeICR (devP, M77_ACR_OFFSET,
                  devP->shadowACR | M77_ACR_DTR_HD_H_DRIVER_CTL);
        if (m77PhysIntSet(devP, G_modM77P->m77opt[devP->unit].physInt) != EOK) {
            DBGWRT_ERR( ( DBH, "*** %s: illegal pyhsical interface %d (%d)\n",
                          __FUNCTION__, G_modM77P->m77opt[devP->unit].physInt,
                          devP->unit));
        }
    }

    /*--------------------+
    |  set tristate mode  |
    +--------------------*/
    if( G_modM77P->modId == MOD_ID_M45N ) { /* only M45N supports tristate */
        setM45nTristate(devP, G_modM77P->m77opt[devP->unit].tristate);
    }

    efr = read650(devP, M77_EFR_OFFSET);
    write650(devP, M77_EFR_OFFSET, (u_int16)(efr | M77_EFR_ENHANCED_MODE));

    if (G_modM77P->m77opt[devP->unit].fifo_enabled) {
        write550(devP, M77_ISR_FCR_REG, M77_FIFO_ENABLE);
        /* set trigger levels and enable 950 interrupt trigger levels */
        writeICR (devP, M77_TTL_OFFSET,
                  G_modM77P->m77opt[devP->unit].transmit_level);
        writeICR (devP, M77_RTL_OFFSET,
                  G_modM77P->m77opt[devP->unit].receive_level);
        writeICR (devP, M77_FCL_OFFSET,
                  G_modM77P->m77opt[devP->unit].hsLowFifoLevel);
        writeICR (devP, M77_FCH_OFFSET,
                  G_modM77P->m77opt[devP->unit].hsHighFifoLevel);
        writeICR (devP, M77_ACR_OFFSET,
                  (devP->shadowACR | M77_ACR_950_TRIG_EN));
        /* clear FIFOs */
        write550(devP, M77_ISR_FCR_REG,
                 (FCR_DMA_MODE | RxCLEAR | TxCLEAR | FIFO_ENABLE));

    }
    else {

        /* switch to DMA mode 1 FIFO disabled */
        write550(devP, M77_ISR_FCR_REG, FCR_DMA_MODE);
        /* clear the port */
        read550(devP, M77_HOLD_DLL_REG);
    }
}

void
create_device(TTYINIT *dip, unsigned unit) {
    DEV_M77             *devP;
    unsigned             i;
    uintptr_t            port;
    unsigned char        msr;
    pthread_mutexattr_t  attr;

    // Get a device entry and the input/output buffers for it.
    devP = (void *) _smalloc(sizeof(*devP));
    memset(devP, 0, sizeof(*devP));

    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setrecursive(&attr, PTHREAD_RECURSIVE_ENABLE );
    pthread_mutex_init(&(devP->mutexUartMode), &attr);

    // Get buffers.
    devP->tty.ibuf.head = devP->tty.ibuf.tail = devP->tty.ibuf.buff =
        _smalloc(devP->tty.ibuf.size = dip->isize);
    devP->tty.obuf.head = devP->tty.obuf.tail = devP->tty.obuf.buff =
        _smalloc(devP->tty.obuf.size = dip->osize);
    devP->tty.cbuf.head = devP->tty.cbuf.tail = devP->tty.cbuf.buff =
        _smalloc(devP->tty.cbuf.size = dip->csize);
    devP->tty.highwater = devP->tty.ibuf.size -
        (devP->tty.ibuf.size < 128 ? devP->tty.ibuf.size/4 : 32);

    strcpy(devP->tty.name, dip->name);

    /* if baud is not -1 then baud was init with command line
       else use the descriptor value */
    if( -1 != dip->baud ) {
        devP->tty.baud = dip->baud;
    } else {
        devP->tty.baud = G_modM77P->m77opt[unit].baudrate;
    }
    devP->tty.fifo = dip->fifo;

    // If TX fifo is set, set the XMIT override flag
    if(devP->tty.fifo & 0xF0) {
        devP->fifo_override = FIFO_XMIT_OVERRIDE;
    }

    port = (uintptr_t) mmap_device_memory( NULL,
        (sizeof(devP->port)/sizeof(devP->port[0])) << dip->port_shift ,
        PROT_READ|PROT_WRITE|PROT_NOCACHE, 0, dip->port);
    for(i = 0; i < sizeof(devP->port)/sizeof(devP->port[0]); ++i) {
        devP->port[i] = port;
        port += 1 << dip->port_shift;
    }
    devP->intr = dip->intr;
    devP->clk = dip->clk;
    devP->div = dip->div;
    devP->unit = unit;
    devP->uartNum = ((unit < 4) ? 0 : 1);

    devP->tty.flags = EDIT_INSERT | LOSES_TX_INTR;
    devP->tty.c_cflag = dip->c_cflag;
    devP->tty.c_iflag = dip->c_iflag;
    devP->tty.c_lflag = dip->c_lflag;
    devP->tty.c_oflag = dip->c_oflag;

    // Initialize termios cc codes to an ANSI terminal.
    ttc(TTC_INIT_CC, &devP->tty, 0);

    devP->tty.io_devctlext = m77_devctl;

    // Initialize the device's name.
    // Assume that the basename is set in device name.  This will attach
    // to the path assigned by the unit number/minor number combination
    unit = SET_NAME_NUMBER(G_modM77P->unit_num + unit) | NUMBER_DEV_FROM_USER;
    ttc(TTC_INIT_TTYNAME, &devP->tty, unit);

    // Initialize power management structures before attaching ISR
    ttc(TTC_INIT_POWER, &devP->tty, 0);

    set_950_mode(devP);
    ser_stty(devP);
    ser_attach_intr(devP);

    // get current MSR stat
    msr = read550(devP, M77_MSR_REG);

    if(msr & M77_MSR_DDCD)
        tti(&devP->tty, (msr & M77_MSR_DCD) ? TTI_CARRIER : TTI_HANGUP);

    if((msr & M77_MSR_DCTS)  &&  (devP->tty.c_cflag & OHFLOW))
        tti(&devP->tty, (msr & M77_MSR_CTS) ? TTI_OHW_CONT : TTI_OHW_STOP);

    // Attach the resource manager
    ttc(TTC_INIT_ATTACH, &devP->tty, 0);

}
