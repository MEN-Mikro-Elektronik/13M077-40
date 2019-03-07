/*********************  P r o g r a m  -  M o d u l e ***********************/
/*!
 *        \file  tto.c
 *
 *      \author  christophe.hannoyer@men.de
 *        $Date: 2009/06/24 16:24:53 $
 *    $Revision: 1.1 $
 *
 *       \brief  A routine to transmit a byte, called by io-char.
 *
 *               It also provides support to control and read hardware control
 *               lines status, and provides support for the stty utility.
 *               io-char down call that uses the stty command to send output
 *               such as line ctrl and line status to the hardware.
 *
 *     Switches: -
 */
/*-------------------------------[ History ]---------------------------------
 *
 * $Log: tto.c,v $
 * Revision 1.1  2009/06/24 16:24:53  channoyer
 * Initial Revision
 *
 *---------------------------------------------------------------------------
 * (c) Copyright 2009 by MEN Mikro Elektronik GmbH, Nuremberg, Germany
 ****************************************************************************/

#include "externs.h"

int
tto(TTYDEV *ttydev, int action, int arg1) {
    TTYBUF          *bup    = &ttydev->obuf;
    DEV_M77         *devP   = (DEV_M77 *)ttydev;
    int              status = 0;
    unsigned char    c;
    int              bytesFree;
    volatile u_int16 tflReg;

    switch(action) {
    case TTO_STTY:
        ser_stty(devP);
        return(0);

    case TTO_CTRL:

        if(arg1 & _SERCTL_BRK_CHG)
            set_port(devP, M77_LCR_REG, M77_LCR_SBRK,
                     arg1 & _SERCTL_BRK ? M77_LCR_SBRK : 0);

        if(arg1 & _SERCTL_DTR_CHG)
            set_port(devP, M77_MCR_REG, M77_MCR_DTR,
                     arg1 & _SERCTL_DTR ? M77_MCR_DTR : 0);

        if(arg1 & _SERCTL_RTS_CHG)
            set_port(devP, M77_MCR_REG, M77_MCR_RTS,
                     arg1 & _SERCTL_RTS ? M77_MCR_RTS : 0);

        return(0);

    case TTO_LINESTATUS:
        return(((read550(devP, M77_MSR_REG) << 8) | read550(devP, M77_MCR_REG))
               & 0xf003);

    case TTO_DATA:
        break;

    default:
        return(0);
    }

    if( arg1 == NULL ) {
        // enable tx interrupt to transmit initial byte
        write550(devP, M77_IER_DLM_REG,
                 read550(devP, M77_IER_DLM_REG) | M77_IER_TBE );
    }

   /*
    * If the OSW_PAGED_OVERRIDE flag is set then allow
    * transmit of character even if output is suspended via
    * the OSW_PAGED flag. This flag implies that the next
    * character in the obuf is a software flow control
    * charater (STOP/START).
    * Note: tx_inject sets it up so that the contol
    *       character is at the start (tail) of the buffer.
    *
   */
    if(devP->tty.flags & (OHW_PAGED|OSW_PAGED) &&
       !(devP->tty.xflags & OSW_PAGED_OVERRIDE)) {
        // transmission stopped: disable tx interrupt
/*         write550(devP, M77_IER_DLM_REG, read550(devP, M77_IER_DLM_REG) */
/*                  & ~M77_IER_TBE ); */
        return(0);
    }

    if(bup->cnt > 0) {
        if( G_modM77P->m77opt[devP->unit].fifo_enabled ) {
            tflReg = readASRxFL(devP, M77_TFL_OFFSET);
            /* calculate number of free bytes in FIFO */
            bytesFree = M77_FIFO_SIZE - tflReg;
        } else {
            bytesFree = 1;
        }

        while (bytesFree--) {
            if(bup->cnt <= 0) break;
            dev_lock(&devP->tty);
            // Get the next character to print from the output buffer
            c = *bup->tail;
            if(c == '\n'  &&
               ((devP->tty.c_oflag & (OPOST | ONLCR)) == (OPOST | ONLCR)) &&
               ((devP->tty.flags & NL_INSERT) == 0)) {
                c = '\r';
                atomic_set(&devP->tty.flags, NL_INSERT);
            } else {
                atomic_clr(&devP->tty.flags, NL_INSERT);
                if(++bup->tail >= &bup->buff[bup->size])
                    bup->tail = &bup->buff[0];
                --bup->cnt;
            }

            // Print the character
            write550(devP, M77_HOLD_DLL_REG, c);

            /* Clear the OSW_PAGED_OVERRIDE flag as we only want
             * one character to be transmitted in this case.
             */
            if (devP->tty.xflags & OSW_PAGED_OVERRIDE) {
                atomic_clr(&devP->tty.xflags, OSW_PAGED_OVERRIDE);
                dev_unlock(&devP->tty);
                break;
            }
            dev_unlock(&devP->tty);
        }
    } else {

        // Check for notify conditions
        if(devP->tty.notify[1].cnt < bup->size - bup->cnt) {
            devP->tty.notify[1].cnt = (~0u) >> 1;    // Disarm
            atomic_set(&devP->tty.flags, EVENT_NOTIFY_OUTPUT);
            status = 1;
        }

        // Is anyone waiting for the output buffer to drain?
        if(devP->tty.waiting_drain && bup->cnt == 0) {
            atomic_set(&devP->tty.flags, EVENT_DRAIN);
            status = 1;
        }
    }

    devP->tty.un.s.tx_tmr = 3;  /* Timeout */

    // If anyone is waiting to write, kick them when buffer drains to 1/4 full.
    if(devP->tty.waiting_write && bup->cnt < bup->size/4) {
        atomic_set(&devP->tty.flags, EVENT_WRITE);
        return(1);
    }

    return(status);
}

void
ser_stty(DEV_M77 *devP) {
    unsigned         lcr = 0;
    unsigned        value;
    int             mutLock;

    // Set Baud rate
    value =
        (devP->tty.baud == 0) ? 0 : (devP->clk/(devP->tty.baud * devP->div));

    mutLock = pthread_mutex_lock( &(devP->mutexUartMode) );
    set_port(devP, M77_LCR_REG, M77_LCR_DLAB, M77_LCR_DLAB);
    set_port(devP, M77_HOLD_DLL_REG, 0xff, value & 0xff);
    set_port(devP, M77_IER_DLM_REG, 0xff, value >> 8);
    set_port(devP, M77_LCR_REG, M77_LCR_DLAB, 0);
    mutLock = pthread_mutex_unlock( &(devP->mutexUartMode) );

    // Set data bits
    switch(devP->tty.c_cflag & CSIZE) {
    case CS8: ++lcr;
    case CS7: ++lcr;
    case CS6: ++lcr;
    }

    // Set stop bits
    if(devP->tty.c_cflag & CSTOPB)
        lcr |= M77_LCR_2_STB;

    // Set parity bits
    if(devP->tty.c_cflag & PARENB)
        lcr |= M77_LCR_PEN;

    if((devP->tty.c_cflag & PARODD) == 0)
        lcr |= M77_LCR_EPS;

    set_port(devP, M77_LCR_REG, 0xFF, lcr);

    if((devP->tty.c_cflag & OHFLOW) == 0) {
        setAutoRTSEnable( devP, 0 );
        setAutoCTSEnable( devP, 0 );
    } else {
        setAutoRTSEnable( devP, 1 );
        setAutoCTSEnable( devP, 1 );
    }

    write650(devP, M77_XON1_OFFSET, devP->tty.c_cc[VSTART]);
    write650(devP, M77_XOFF1_OFFSET, devP->tty.c_cc[VSTOP]);

    value = 0;
    if((devP->tty.c_iflag & IXON) == IXON)
        value |= 0x02;
    if((devP->tty.c_iflag & IXOFF) == IXOFF)
        value |= 0x08;

    setInBandFlowControlMode( devP, value );

    // turn on DTR, RTS
    set_port(devP, M77_MCR_REG, M77_MCR_DTR|M77_MCR_RTS,
             M77_MCR_DTR|M77_MCR_RTS);
}
