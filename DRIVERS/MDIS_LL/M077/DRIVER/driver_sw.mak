#***************************  M a k e f i l e  *******************************
#
#         Author: christophe.hannoyer@men.de
#          $Date: 2009/06/24 16:20:26 $
#      $Revision: 1.1 $
#
#    Description: Makefile definitions for the M77 driver with 
#                 SWAPED ACCESS !
#
#---------------------------------[ History ]---------------------------------
#
#   $Log: driver_sw.mak,v $
#   Revision 1.1  2009/06/24 16:20:26  channoyer
#   Initial Revision
#
#-----------------------------------------------------------------------------
# (c) Copyright 2009 by MEN Mikro Elektronik GmbH, Nuremberg, Germany
#*****************************************************************************

MAK_NAME=m77_sw

MAK_SWITCH=$(SW_PREFIX)MAC_MEM_MAPPED \
           $(SW_PREFIX)MAC_BYTESWAP \
           $(SW_PREFIX)ID_SW \
           $(SW_PREFIX)PLD_SW \
           $(SW_PREFIX)m77Drv=m77Drv_sw \
           $(SW_PREFIX)M77_SW \

MAK_LIBS=	\

MAK_INCL= \



MAK_INP1=m77_drv$(INP_SUFFIX)

MAK_INP=$(MAK_INP1)
