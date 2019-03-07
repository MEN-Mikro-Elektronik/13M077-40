/*********************  P r o g r a m  -  M o d u l e ***********************/
/*!
 *        \file  intr.c
 *
 *      \author  christophe.hannoyer@men.de
 *        $Date: 2009/06/24 16:22:53 $
 *    $Revision: 1.1 $
 *
 *       \brief  Interrupt handler routines
 *
 *     Switches: -
 */
/*-------------------------------[ History ]---------------------------------
 *
 * $Log: intr.c,v $
 * Revision 1.1  2009/06/24 16:22:53  channoyer
 * Initial Revision
 *
 *---------------------------------------------------------------------------
 * (c) Copyright 2009 by MEN Mikro Elektronik GmbH, Nuremberg, Germany
 ****************************************************************************/

#include "externs.h"

/*
 * Process data in a line status register
 */
static int
process_lsr(DEV_M77 *devP, unsigned char lsr) {
    unsigned key = 0, eventflag = 0;

    // Return immediately if no errors.
    if((lsr & (M77_LSR_BI|M77_LSR_OE|M77_LSR_FE|M77_LSR_PE)) == 0)
        return(0);

    // Save the error as out-of-band data which can be retrieved via devctl().
    devP->tty.oband_data |= (lsr >> 1) & 0x0f;
    atomic_set(&devP->tty.flags, OBAND_DATA);

    // Read whatever input data happens to be in the buffer to "eat" the
    // spurious data associated with break, parity error, etc.
    key = read550(devP, M77_HOLD_DLL_REG);

    if(lsr & M77_LSR_BI)
        key |= TTI_BREAK;
    else if(lsr & M77_LSR_OE)
        key |= TTI_OVERRUN;
    else if(lsr & M77_LSR_FE)
        key |= TTI_FRAME;
    else if(lsr & M77_LSR_PE)
        key |= TTI_PARITY;

    return(tti(&devP->tty, key) | eventflag);
}

/*
 * Serial interrupt handler
 */
struct sigevent *
ser_intr(void *area, int id) {
    struct dev_list    *list = area;
    int                status = 0;
    int                something_happened;
    unsigned char    msr, lsr;
    DEV_M77            *devP;
    struct sigevent *event = NULL;
    volatile int  readBytes;

    do {
        something_happened = 0;

        for(devP = list->device; devP != NULL; devP = devP->next) {
            unsigned    iir;

            status = 0;

            iir = read550(devP, M77_ISR_FCR_REG) & M77_ISR_MASK;
            switch(iir) {
            /* serial error or break */
            case M77_ISR_RX_ERROR:
                lsr = read550(devP, M77_LSR_REG);
                status |= process_lsr(devP, lsr);
                break;

            /* data received interrupt */
            case M77_ISR_RX_DATA_AVAIL:
            case M77_ISR_RX_TIMEOUT:
                /* read RFL twice a. compare the values to be sure data are
                   valid */
                do {
                    readBytes = readASRxFL(devP, M77_RFL_OFFSET);
                    IDBGWRT_2( ( DBH,
                    ">>> m77Int: m77LeaveRxInBuf, FIFO enbl, readBytes = %d\n" ,
                    readBytes));
                } while (readBytes != readASRxFL(devP, M77_RFL_OFFSET) );
                while (readBytes--) {
                    status |= tti(&devP->tty, read550(devP, M77_HOLD_DLL_REG));
                    lsr = read550(devP, M77_LSR_REG);
                    status |= process_lsr(devP, lsr);
                }
                break;

               /* transmit interrupt */
               case M77_ISR_THR_EMPTY:
                devP->tty.un.s.tx_tmr = 0;
                status |= tto(&devP->tty, TTO_DATA, 0);
                break;

               /* modem status interrupt */
               case M77_ISR_MODEM_STATE:
                msr = read550(devP, M77_MSR_REG);

                if(msr & M77_MSR_DDCD)
                    status |= tti(&devP->tty,
                                  (msr & M77_MSR_DCD) ? TTI_CARRIER : TTI_HANGUP);

                if((msr & M77_MSR_DCTS)  &&  (devP->tty.c_cflag & OHFLOW))
                    status |= tti(&devP->tty,
                                  (msr & M77_MSR_CTS) ? TTI_OHW_CONT : TTI_OHW_STOP);
                break;

            default:
                continue;

            }

            something_happened = 1;
            if(status) {
                if((devP->tty.flags & EVENT_QUEUED) == 0) {
                    event = &ttyctrl.event;
                    dev_lock(&ttyctrl);
                    ttyctrl.event_queue[ttyctrl.num_events++] = &devP->tty;
                    atomic_set(&devP->tty.flags, EVENT_QUEUED);
                    dev_unlock(&ttyctrl);
                    }
                }
            }

        } while(something_happened);

    return(event);
}
