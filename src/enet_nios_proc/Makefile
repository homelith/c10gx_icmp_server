#-------------------------------------------------------------------------------
# Makefile
# - CLI automated compilation environment for Nios II SBT project
#-------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
# MIT License
#
# Copyright (c) 2020 homelith
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#-------------------------------------------------------------------------------

TARGET          :=
SUBDIRS         :=
.PHONY:         default single modgen sysgen softgen clean $(SUBDIRS)

PROJ_NAME       := enet_nios_proc
BSP_NAME        := enet_nios_proc_bsp
BSP_DIR         := $(shell echo $(abspath $(BSP_NAME)) | sed "s/\//\\\\\//g")\/
QPF_DIR         := $(shell echo $(abspath ../../syn) | sed "s/\//\\\\\//g")\/
SOPC_DIR        := $(shell echo $(abspath ../system/system) | sed "s/\//\\\\\//g")\/
SOPC_FILE       := $(shell echo $(abspath ../system/system/system.sopcinfo) | sed "s/\//\\\\\//g")

default:        single

single:
	sed -i "s/^BSP_ROOT_DIR\s*:=.*/BSP_ROOT_DIR := $(BSP_DIR)/" $(PROJ_NAME)/Makefile
	sed -i "s/^QUARTUS_PROJECT_DIR\s*:=.*/QUARTUS_PROJECT_DIR := $(QPF_DIR)/" $(PROJ_NAME)/Makefile
	sed -i "s/^BSP_DIR\s*=.*/BSP_DIR=$(BSP_DIR)/" $(PROJ_NAME)/create-this-app
	sed -i "s/^QUARTUS_PROJECT_DIR\s*=.*/QUARTUS_PROJECT_DIR=$(QPF_DIR)/" $(PROJ_NAME)/create-this-app
	sed -i "s/^SOPC_FILE\s*:=.*/SOPC_FILE := $(SOPC_FILE)/" $(BSP_NAME)/Makefile
	sed -i "s/^SOPCINFO_FILE\s*:=.*/SOPCINFO_FILE := $(SOPC_FILE)/" $(BSP_NAME)/public.mk
	sed -i "s/^SOPC_DIR\s*=.*/SOPC_DIR=$(SOPC_DIR)/" $(BSP_NAME)/create-this-bsp
	sed -i "s/^SOPC_FILE\s*=.*/SOPC_FILE=$(SOPC_FILE)/" $(BSP_NAME)/create-this-bsp
	sed -i "s/<SopcDesignFile>.*<\/SopcDesignFile>/<SopcDesignFile>$(SOPC_FILE)<\/SopcDesignFile>/" $(BSP_NAME)/settings.bsp
	eclipse-nios2 -noSplash -data . -application org.eclipse.cdt.managedbuilder.core.headlessbuild -importAll .
	nios2-bsp-generate-files --settings=$(BSP_NAME)/settings.bsp --bsp-dir=$(BSP_NAME)
	make -C $(PROJ_NAME) all
	make -C $(PROJ_NAME) mem_init_generate

modgen:         COMMAND := modgen
modgen:         $(SUBDIRS)

sysgen:         COMMAND := sysgen
sysgen:         $(SUBDIRS)

softgen:        COMMAND := softgen
softgen:        $(SUBDIRS)
	make single

clean:          COMMAND := clean
clean:          $(SUBDIRS)
	make -C $(PROJ_NAME) clean_all
	make -C $(BSP_NAME) clean
	rm -rf .metadata RemoteSystemsTempFiles

$(SUBDIRS):
	make -C $@ $(COMMAND)
