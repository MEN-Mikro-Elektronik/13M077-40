/*********************  P r o g r a m  -  M o d u l e ***********************/
/*!
 *        \file  m77.c
 *
 *      \author  christophe.hannoyer@men.de
 *        $Date: 2009/07/10 15:50:43 $
 *    $Revision: 1.2 $
 *
 *       \brief  M45N/M69N/M77 specific code
 *
 *     Switches: -
 */
/*-------------------------------[ History ]---------------------------------
 *
 * $Log: m77.c,v $
 * Revision 1.2  2009/07/10 15:50:43  channoyer
 * R: HW access failed with x86
 * M: Use direct memory access
 *
 * Revision 1.1  2009/06/24 16:23:05  channoyer
 * Initial Revision
 *
 *---------------------------------------------------------------------------
 * (c) Copyright 2009 by MEN Mikro Elektronik GmbH, Nuremberg, Germany
 ****************************************************************************/

#include "externs.h"

/******************************** m77PhysIntSet ******************************
 *
 *  Description: set pysical interface
 *
 *----------------------------------------------------------------------------
 *  Input......:  pChan         the channel descriptor
 *                drvMode       line driver mode RS232 etc.
 *
 *  Output.....:  Return        OK or ERROR
 *
 *  Globals....: m77DevData
 *****************************************************************************/
int m77PhysIntSet(DEV_M77 *devP, u_int16 drvMode)
{
    int retVal = EOK;

    DBGWRT_1 ((DBH, "m77PhysIntSet:\n"));

    switch (drvMode) {
        case M77_RS422_HD:

            if (G_modM77P->m77opt[devP->unit].echo_suppress == TRUE) {
                /* set RS422HD mode */
                MWRITE_D8 (G_modM77P->module_port,
                            M77_DCR_REG_BASE + (2 * devP->unit),
                            M77_RS422_HD);
            }
            else {
                /* set RS422HD mode + echo enable */
                MWRITE_D8 (G_modM77P->module_port,
                            M77_DCR_REG_BASE + (2 * devP->unit),
                            M77_RS422_HD | M77_RX_EN);
            }
            break;

        case M77_RS422_FD:

            MWRITE_D8 (G_modM77P->module_port,
                        M77_DCR_REG_BASE + (2 * devP->unit),
                        M77_RS422_FD);
            break;

        case M77_RS485_HD:

            if (G_modM77P->m77opt[devP->unit].echo_suppress == TRUE) {
                /* set M77_RS485_HD mode */
                MWRITE_D8 (G_modM77P->module_port,
                            M77_DCR_REG_BASE + (2 * devP->unit),
                            M77_RS485_HD);
            }
            else {
                /* set M77_RS485_HD mode + echo enable */
                MWRITE_D8 (G_modM77P->module_port,
                            M77_DCR_REG_BASE + (2 * devP->unit),
                            M77_RS485_HD | M77_RX_EN);
            }
            break;

        case M77_RS485_FD:
            /* set M77_RS485_FD mode + echo enable */
            MWRITE_D8 (G_modM77P->module_port,
                        M77_DCR_REG_BASE + (2 * devP->unit),
                        M77_RS485_FD);
            break;

        case M77_RS232:
            if( G_modM77P->modId != MOD_ID_M45N )
                MWRITE_D8 (G_modM77P->module_port,
                            M77_DCR_REG_BASE + (2 * devP->unit),
                            M77_RS232);
            break;

        default:
                DBGWRT_ERR ((DBH, " *** m77PhysIntSet: Invalid mode\n"));
                drvMode = G_modM77P->m77opt[devP->unit].physInt;
                retVal = EINVAL;

    }

    /* store new mode in device structure */
    G_modM77P->m77opt[devP->unit].physInt = drvMode;

    return (retVal);
}

/******************************** m77ChannelRest ******************************
 *
 *  Description: Reset the m77Channel
 *
 *----------------------------------------------------------------------------
 *  Input......:  devP          the device descriptor
 *
 *  Output.....:  Return        -
 *
 *  Globals....:
 *****************************************************************************/
void m77ChannelRest(DEV_M77 *devP)
{
    DBGWRT_1( ( DBH, "m77ChannelRest\n"));

    /* reset the channel usinfg CSR-register */
    writeICR (devP, M77_CSR_INDEX, 0x00);

    /* initialize error Rx-Error flags */
    devP->lastErr = 0;

    /* initialize shadow ACR value */
    devP->shadowACR = 0;

}

/********************************* read550 ***********************************
 *
 *  Description: Read the 550 registers
 *
 *----------------------------------------------------------------------------
 *  Input......:  devP          the device descriptor
 *                offset        register offset
 *
 *  Output.....:  Return        value of the register
 *
 *  Globals....:
 *****************************************************************************/
u_int16 read550(DEV_M77 *devP, u_int16 offset)
{
    u_int16 result;
    int mutLock;

    mutLock = pthread_mutex_lock( &(devP->mutexUartMode) );
    result = *(u_int8 *) devP->port[offset];
    mutLock = pthread_mutex_unlock( &(devP->mutexUartMode) );
    return (result & 0x00ff);
}

/******************************** write550 ***********************************
 *
 *  Description: Write the 550 registers
 *
 *----------------------------------------------------------------------------
 *  Input......:  devP          the device descriptor
 *                offset        register offset
 *                value         value to write
 *
 *  Output.....:
 *
 *  Globals....:
 *****************************************************************************/
void write550(DEV_M77 *devP, u_int16 offset, u_int16 value)
{
    int mutLock;
    mutLock = pthread_mutex_lock( &(devP->mutexUartMode) );
    *(u_int8 *) devP->port[offset] = value;
    mutLock = pthread_mutex_unlock( &(devP->mutexUartMode) );
}

/********************************* read650 ***********************************
 *
 *  Description: Read the 650 specific registers
 *
 *----------------------------------------------------------------------------
 *  Input......:  devP          the device descriptor
 *                offset        register offset
 *
 *  Output.....:  Return        value of the register
 *
 *  Globals....:
 *****************************************************************************/
u_int16 read650(DEV_M77 *devP, u_int16 offset)
{
    u_int16    result, oldLCR;
    int mutLock;
    mutLock = pthread_mutex_lock( &(devP->mutexUartMode) );
    /* store old LCR and write access code */
    oldLCR = read550(devP, M77_LCR_REG);
    write550(devP, M77_LCR_REG, M77_LCR_650_ACCESS_KEY);
    /* read the register */
    result = read550(devP, offset);
    /* restore LCR */
    write550(devP ,M77_LCR_REG, oldLCR);
    mutLock = pthread_mutex_unlock( &(devP->mutexUartMode) );
    return (result & 0x00ff);
}

/******************************** write650 ***********************************
 *
 *  Description: Write the 650 specific registers
 *
 *----------------------------------------------------------------------------
 *  Input......:  devP          the device descriptor
 *                offset        register offset
 *                value         value to write
 *
 *  Output.....:
 *
 *  Globals....:
 *****************************************************************************/
void write650(DEV_M77 *devP, u_int16 offset, u_int16 value)
{
    u_int16    oldLCR;
    int mutLock;

    mutLock = pthread_mutex_lock( &(devP->mutexUartMode) );
    /* store old LCR and write access code */
    oldLCR = read550(devP, M77_LCR_REG);
    write550(devP, M77_LCR_REG, M77_LCR_650_ACCESS_KEY);
    /* write register */
    write550(devP, offset, value);
    /* restore LCR */
    write550(devP, M77_LCR_REG, oldLCR);
    mutLock = pthread_mutex_unlock( &(devP->mutexUartMode) );
}

/******************************** writeICR ***********************************
 *
 *  Description: Write to the ICR with the specified index and value
 *
 *----------------------------------------------------------------------------
 *  Input......:  devP          the channel descriptor
 *                index         index of the register
 *                value         value to write
 *
 *  Output.....:
 *
 *  Globals....:
 *****************************************************************************/
void writeICR(DEV_M77 *devP, u_int16 index, u_int16 value)
{
    int mutLock;
    mutLock = pthread_mutex_lock( &(devP->mutexUartMode) );
    /* writes the ICR set register
       index by the index parameter */
    write550(devP, M77_SPR_REG, index);
    write550(devP, M77_ICR_OFFSET, value);
    /* record changes made to ACR */
    if (index==M77_ACR_OFFSET) devP->shadowACR = value;
    mutLock = pthread_mutex_unlock( &(devP->mutexUartMode) );
}

/************************** UnlockAdditionalStatus ***************************
 *
 *  Description: Unlock the access to the 950 specific registers
 *
 *----------------------------------------------------------------------------
 *  Input......:  devP          the channel descriptor
 *
 *  Output.....:
 *
 *  Globals....:
 *****************************************************************************/
void UnlockAdditionalStatus(DEV_M77 *devP)
{
    int mutLock;
    /* Set the top bit of ACR to enable
       950 specific register set access */
    mutLock = pthread_mutex_lock( &(devP->mutexUartMode) );
    writeICR(devP, M77_ACR_OFFSET, devP->shadowACR | M77_ACR_950_READ_EN);
}

/**************************** LockAdditionalStatus ***************************
 *
 *  Description: Lock the access to the 950 specific registers
 *
 *----------------------------------------------------------------------------
 *  Input......:  devP          the channel descriptor
 *
 *  Output.....:
 *
 *  Globals....:
 *****************************************************************************/
void LockAdditionalStatus(DEV_M77 *devP)
{
    int mutLock;
    /* Clear the top bit of ACR to disable
       950 specific register set access */
    writeICR(devP, M77_ACR_OFFSET, devP->shadowACR & (~M77_ACR_950_READ_EN));
    mutLock = pthread_mutex_unlock( &(devP->mutexUartMode) );
}

/******************************** readASRxFL **********************************
 *
 *  Description: Write to the ASR register
 *               NOTE: Only Bit[0] can be written
 *
 *----------------------------------------------------------------------------
 *  Input......:  devP          the channel descriptor
 *                index         index of the register
 *
 *  Output.....:
 *
 *  Globals....:
 *****************************************************************************/
u_int16 readASRxFL(DEV_M77 *devP, u_int16 offset)
{
    /* Returns the data stored in the ASR register */
    u_int16    retVal;
    UnlockAdditionalStatus(devP);
    retVal = read550(devP, offset);
    LockAdditionalStatus(devP);
    return (retVal & 0x00ff);
}

/***************************** setAutoCTSEnable ******************************
 *
 *  Description: Enables/Disables the automatic CTS mode (on chip)
 *
 *----------------------------------------------------------------------------
 *  Input......:  devP          the channel descriptor
 *                state         1: enable, 0: disable
 *
 *  Output.....:
 *
 *  Globals....:
 *****************************************************************************/
void setAutoCTSEnable(DEV_M77 *devP, u_int8 state)
{
    /* Sets the state of automatic CTS flow control enable bit to state */
    u_int16 efr = read650(devP, M77_EFR_OFFSET);
    /* Set the bit according to the state requested */
    if(state) efr |= M77_EFR_FLOWCTL_CTS;
    else      efr &= ~M77_EFR_FLOWCTL_CTS;
    /* Write new value */
    write650(devP, M77_EFR_OFFSET, efr);
}

/***************************** setAutoRTSEnable ******************************
 *
 *  Description: Enables/Disables the automatic RTS mode (on chip)
 *
 *----------------------------------------------------------------------------
 *  Input......:  devP          the channel descriptor
 *                state         1: enable, 0: disable
 *
 *  Output.....:
 *
 *  Globals....:
 *****************************************************************************/
void setAutoRTSEnable(DEV_M77 *devP, u_int8 state)
{
    /* Sets the state of automatic RTS flow control enable bit to state */
    u_int16 efr = read650(devP, M77_EFR_OFFSET);
    /* Set the bit according to the state requested */
    if(state) efr |= M77_EFR_FLOWCTL_RTS;
    else      efr &= ~M77_EFR_FLOWCTL_RTS;
    /* Write new value */
    write650(devP, M77_EFR_OFFSET, efr);
}

/***************************** setAutoDSREnable ******************************
 *
 *  Description: Enables/Disables the automatic DSR mode (on chip)
 *
 *----------------------------------------------------------------------------
 *  Input......:  devP          the channel descriptor
 *                state         1: enable, 0: disable
 *
 *  Output.....:
 *
 *  Globals....:
 *****************************************************************************/
void setAutoDSREnable(DEV_M77 *devP, u_int8 state)
{
    u_int16     shadowACR;

    shadowACR = devP->shadowACR;
    /* Sets the state of automatic DSR flow control enable bit to state */
    if(state) shadowACR |= M77_ACR_AUTO_DSR_EN;
    else      shadowACR &= ~M77_ACR_AUTO_DSR_EN;
    /* Write new value */
    writeICR(devP, M77_ACR_OFFSET, shadowACR);
}

/***************************** setAutoDTREnable ******************************
 *
 *  Description: Enables/Disables the automatic DTR mode (on chip)
 *
 *----------------------------------------------------------------------------
 *  Input......:  devP          the channel descriptor
 *                state         1: enable, 0: disable
 *
 *  Output.....:
 *
 *  Globals....:
 *****************************************************************************/
void setAutoDTREnable(DEV_M77 *devP, u_int8 state)
{
    u_int16     shadowACR;

    shadowACR = devP->shadowACR;
    /* Sets the state of automatic DTR flow control enable bit to state */
    if(state) shadowACR |= M77_ACR_DTR_RX_CTL;
    else      shadowACR &= ~M77_ACR_DTR_RX_CTL;
    /* Write new value */
    writeICR(devP, M77_ACR_OFFSET, shadowACR);
}

/************************** setInBandFlowControlMode *************************
 *
 *  Description: Sets the automatic in band flow control to the given mode
 *
 *----------------------------------------------------------------------------
 *  Input......:  devP          the channel descriptor
 *                mode          the desired mode (see OX16C954 manual)
 *
 *  Output.....:
 *
 *  Globals....:
 *****************************************************************************/
void setInBandFlowControlMode(DEV_M77 *devP, u_int8 mode)
{
    u_int16 efr = read650(devP, M77_EFR_OFFSET) & 0xF0;
    write650(devP, M77_EFR_OFFSET, (u_int16)(efr | mode));
}

/************************** setM45nTristate **********************************
 *
 *  Description: Sets the M45N line drivers to tristate
 *
 *----------------------------------------------------------------------------
 *  Input......:  devP          the channel descriptor
 *                state         0: normal
 *                              1: tristate
 *
 *  Output.....:
 *
 *  Globals....:
 *****************************************************************************/
void setM45nTristate(DEV_M77 *devP, u_int8 state)
{
    u_int8 mask = 0;
    u_int8 offset = 0x00;

    switch ( devP->unit )
    {
        case 0:
            mask  |= M45N_TCR1_CHAN0;
            offset = M45N_TCR1_REG;
            break;
        case 1:
            mask  |= M45N_TCR1_CHAN1;
            offset = M45N_TCR1_REG;
            break;
        case 2:
        case 3:
            mask  |= M45N_TCR1_CHAN23;
            offset = M45N_TCR1_REG;
            break;
        case 4:
        case 5:
            mask |= M45N_TCR2_CHAN45;
            offset = M45N_TCR2_REG;
            break;
        default:
            mask |= M45N_TCR2_CHAN67;
            offset = M45N_TCR2_REG;
            break;

    }

    if( state ) {
        MSETMASK_D8( G_modM77P->module_port, offset,  mask );
    } else {
        MCLRMASK_D8( G_modM77P->module_port, offset,  mask );
    }

    return;
}

int
m77_devctl(resmgr_context_t *ctp, io_devctl_t *msg, iofunc_ocb_t *ocb) {
    TTYDEV        *dev = ocb->attr;
    DEV_M77     *devP = (DEV_M77 *)dev;
    int         *valP = _DEVCTL_DATA(msg->i);
    int         status = EOK;

    switch(msg->i.dcmd) {

        /*
         * IOCTL for M77 Echo suppression
         */
    case M77_ECHO_SUPPRESS:
        DBGWRT_2(( DBH, "M77_ECHO_SUPPRESS\n"));
        if (G_modM77P->modId != MOD_ID_M77) {
            status = ENOTTY;
        } else {
            if (*valP == 1) {
                G_modM77P->m77opt[devP->unit].echo_suppress = TRUE;
            } else {
                G_modM77P->m77opt[devP->unit].echo_suppress = FALSE;
            }
            status = m77PhysIntSet (devP, G_modM77P->m77opt[devP->unit].physInt);
        }

        break;

        /*
         * IOCTL for M77 physical Mode setting
         */
    case M77_PHYS_INT_SET:
        DBGWRT_2( ( DBH, "ioctl M77_PHYS_INT_SET\n" ));
        if (G_modM77P->modId != MOD_ID_M77) {
            status = ENOTTY;
        } else {
            status = m77PhysIntSet (devP, *valP);
        }

        break;

        /*
         * IOCTL for M45N Tristate settings
         */
    case M45_TIO_TRI_MODE:
        DBGWRT_2( ( DBH, "ioctl M45_TIO_TRI_MODE " ));
        if (G_modM77P->modId != MOD_ID_M45N) {
            status = ENOTTY;
        } else {
            if( *valP ) {
                setM45nTristate(devP, 1);
            } else {
                setM45nTristate(devP, 0);
            }
        }
        break;

    default:
            status = EINVAL;
    }

    return(status);
}
