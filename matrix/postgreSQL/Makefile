PROJ = libpostgreSQL_scale_db.so

CSRC = \
    postgreSQL_scales.c

HSRC = \
	interface_scales.h


OBJS = $(CSRC:.c=.o)
HDRS = $(HSRC)

DEBUG = # -g

INC_POSTGRESQL = /usr/include/postgresql
DEFINES =  -D_GNU_SOURCE -DDEBUG
CFLAGS = -Wall -pedantic -W -Werror -std=c99 -I. -I$(INC_POSTGRESQL) $(DEFINES) -fPIC
#CXXFLAGS = -m64 -pipe -O2 -fPIC -Wall -W -DEXPORT_TO_C $(DEFINES) -I$(COMMON)
CXXFLAGS = -I$(INCDIR) -I../protobuf-c-rpc -I../protobuf-c -m64 -pipe -O2 -fPIC -Wall -W -DEXPORT_TO_C $(DEBUG)
LDFLAGS  = -shared $(DEBUG) -o$(PROJ) $(OBJS) -lpq

ifeq (nto,$(SYSNAME))
CC = qcc
else
#CC = gcc
C = gcc
CXX = g++
endif


LINK.o = $(C) $(LDFLAGS) $(TARGET_ARCH)

#OBJS = $(addprefix $(OBJDIR)/,$(CSRC:.c=.o))

VPATH = %.c

$(PROJ): $(OBJS) Makefile
	 $(LINK.o)

postgreSQL_scales.o : postgreSQL_scales.c interface_scales.h
	$(COMPILE.c) $(OUTPUT_OPTION) $<



clean: ; $(RM) $(OBJS) $(PROJ)

.PHONY : clean


