# 
# Copyright 2001, QNX Software Systems Ltd. All Rights Reserved
#  
# This source code has been published by QNX Software Systems Ltd. (QSSL).
# However, any use, reproduction, modification, distribution or transfer of
# this software, or any software which includes or is based upon any of this
# code, is only permitted under the terms of the QNX Community License version
# 1.0 (see licensing.qnx.com for details) or as otherwise expressly authorized
# by a written license agreement from QSSL. For more information, please email
# licensing@qnx.com.
#  
#
# General purpose makefile for building a Neutrino character device driver
#
ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

NAME=devc-$(SECTION)
EXTRA_SILENT_VARIANTS+=$(subst -, ,$(SECTION))

LIBS=io-char pm ps m
EXCLUDE_OBJS+=tedit.o
USEFILE=$(SECTION_ROOT)/options.c

INSTALLDIR=sbin

define PINFO
PINFO DESCRIPTION=
endef

include $(MKFILES_ROOT)/qmacros.mk
include $(SECTION_ROOT)/pinfo.mk

TINY_NAME=$(subst devc-,devc-t,$(BUILDNAME))

ifneq (,$(filter tedit.c, $(notdir $(SRCS))))

POST_TARGET=$(TINY_NAME)
EXTRA_ICLEAN=$(TINY_NAME)*

define POST_INSTALL
	-$(CP_HOST) $(TINY_NAME) $(INSTALL_DIRECTORY)/
endef

endif

include $(MKFILES_ROOT)/qtargets.mk

-include $(PROJECT_ROOT)/roots.mk
ifndef LIBIOCHAR_ROOT
LIBIOCHAR_ROOT=$(PRODUCT_ROOT)
endif

#
# Some makefile mopery-popery to get devc-t*.pinfo generated properly
#
$(TINY_NAME): INSTALLNAME=$(INSTALL_DIRECTORY)/$(TINY_NAME)$(VARIANT_TAG)

$(TINY_NAME): tedit.o $(OBJS) $(LIBNAMES)
	$(TARGET_BUILD)
