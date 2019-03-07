#***************************  M a k e f i l e  *******************************
#
#         Author: christophe.hannoyer@men.de
#          $Date: 2009/07/10 15:52:10 $
#      $Revision: 1.2 $
#
#    Description: Makefile definitions for the M77 driver
#
#---------------------------------[ History ]---------------------------------
#
#   $Log: program.mak,v $
#   Revision 1.2  2009/07/10 15:52:10  channoyer
#   QNX 6.4 support
#
#   Revision 1.1  2009/06/24 16:23:54  channoyer
#   Initial Revision
#
#-----------------------------------------------------------------------------
# (c) Copyright 2009 by MEN Mikro Elektronik GmbH, Nuremberg, Germany
#*****************************************************************************

ifeq ($(DEBUG),dbg)
    MAK_NAME=devc-serm77_dbg
endif
ifeq ($(DEBUG),nodbg)
    MAK_NAME=devc-serm77
endif 

MAK_SWITCH=$(SW_PREFIX)MAC_MEM_MAPPED -fno-strict-aliasing

#MAK_LIBS=$(LIB_PREFIX)$(MEN_LIB_DIR)/$(COMP_PREFIX)desc_dbg$(LIB_SUFFIX)	\
#         $(LIB_PREFIX)$(MEN_LIB_DIR)/$(COMP_PREFIX)oss_dbg$(LIB_SUFFIX)	\
#         $(LIB_PREFIX)$(MEN_LIB_DIR)/$(COMP_PREFIX)id_dbg$(LIB_SUFFIX)	\
#         $(LIB_PREFIX)$(MEN_LIB_DIR)/$(COMP_PREFIX)dbg_dbg$(LIB_SUFFIX)	\
#         $(LIB_PREFIX)$(MEN_LIB_DIR)/$(COMP_PREFIX)men-libc_dbg$(LIB_SUFFIX)	\

#MAK_LIBS=	io-char pm ps m mdis_api usr_oss men_oss men_desc men_dbg men_men-libc men_bbis_kernel men_id \

MAK_LIBS=	io-char pm ps m mdis_api \


MAK_INCL= \



MAK_INP1=externs$(INP_SUFFIX)
MAK_INP2=init$(INP_SUFFIX)
MAK_INP3=intr$(INP_SUFFIX)
MAK_INP4=m77$(INP_SUFFIX)
MAK_INP5=main$(INP_SUFFIX)
MAK_INP6=options$(INP_SUFFIX)
MAK_INP7=query_mdisdev$(INP_SUFFIX)
MAK_INP8=sys_ttyinit$(INP_SUFFIX)
MAK_INP9=tedit$(INP_SUFFIX)
MAK_INP10=tto$(INP_SUFFIX)

MAK_INP=$(MAK_INP1) \
        $(MAK_INP2) \
        $(MAK_INP3) \
        $(MAK_INP4) \
        $(MAK_INP5) \
        $(MAK_INP6) \
        $(MAK_INP7) \
        $(MAK_INP8) \
        $(MAK_INP9) \
        $(MAK_INP10)
