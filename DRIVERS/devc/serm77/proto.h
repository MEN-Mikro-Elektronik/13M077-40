/***********************  I n c l u d e  -  F i l e  ************************/
/*!
 *        \file  proto.h
 *
 *      \author  christophe.hannoyer@men.de
 *        $Date: 2009/06/24 16:24:06 $
 *    $Revision: 1.1 $
 *
 *       \brief  Header file for xxx
 *
 *     Switches: -
 */
/*-------------------------------[ History ]---------------------------------
 *
 * $Log: proto.h,v $
 * Revision 1.1  2009/06/24 16:24:06  channoyer
 * Initial Revision
 *
 *---------------------------------------------------------------------------
 * (c) Copyright 2009 by MEN Mikro Elektronik GmbH, Nuremberg, Germany
 ****************************************************************************/

#ifndef _PROTO_H
#define _PROTO_H

#ifdef __cplusplus
    extern "C" {
#endif

/*--------------------------------------+
|   PROTOTYPES                          |
+--------------------------------------*/

void create_device(TTYINIT *dip, unsigned unit);
void ser_stty(DEV_M77 *dev);
void ser_ctrl(DEV_M77 *dev, unsigned flags);
void sys_ttyinit(TTYINIT *dip);
void *query_default_device(TTYINIT *dip, void *link);

void *query_mdis_device();

void set_port(DEV_M77 *devP, unsigned port, unsigned mask, unsigned data);
void writeICR(DEV_M77 *devP, u_int16 index, u_int16 value);

int m77PhysIntSet(DEV_M77 *devP, u_int16 drvMode);
void m77ChannelRest(DEV_M77 *devP);
u_int16 read550(DEV_M77 *devP, u_int16 offset);
void write550(DEV_M77 *devP, u_int16 offset, u_int16 value);
u_int16 read650(DEV_M77 *devP, u_int16 offset);
void write650(DEV_M77 *devP, u_int16 offset, u_int16 value);
void UnlockAdditionalStatus(DEV_M77 *devP);
void LockAdditionalStatus(DEV_M77 *devP);
u_int16 readASRxFL(DEV_M77 *devP, u_int16 offset);
void setAutoCTSEnable(DEV_M77 *devP, u_int8 state);
void setAutoRTSEnable(DEV_M77 *devP, u_int8 state);
void setAutoDSREnable(DEV_M77 *devP, u_int8 state);
void setAutoDTREnable(DEV_M77 *devP, u_int8 state);
void setInBandFlowControlMode(DEV_M77 *devP, u_int8 mode);
int  m77_devctl(resmgr_context_t *ctp, io_devctl_t *msg, iofunc_ocb_t *ocb);
void setM45nTristate(DEV_M77 *devP, u_int8 state);

struct sigevent *ser_intr(void *area, int id);

unsigned options(int argc, char *argv[]);

#ifdef __cplusplus
    }
#endif

#endif
