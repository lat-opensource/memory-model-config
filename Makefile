# SPDX-License-Identifier: GPL-2.0

# Please keep these build lists sorted!

# If we are running by kernel building system
ifneq ($(KERNELRELEASE),)
# If we running without kernel build system
# core driver code
obj-m += mem-model-config.o
mem-model-config-objs := main.o tso.o stfill.o

else

CC = g++
CXXFLAGS = -Wall -Werror -O2 -g -std=c++11

BUILDSYSTEM_DIR:=/lib/modules/$(shell uname -r)/build
PWD:=$(shell pwd)

all: config
# run kernel build system to make module
	$(MAKE) -C $(BUILDSYSTEM_DIR) M=$(PWD) modules

config:
# build quick config program
	$(CC) $(CXXFLAGS) config.cpp -o config

clean:
# run kernel build system to cleanup in current directory
	$(MAKE) -C $(BUILDSYSTEM_DIR) M=$(PWD) clean
	rm -f config
endif
