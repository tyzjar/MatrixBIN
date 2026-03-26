# make d=COMPILATION_MODE
# COMPILATION_MODE: debug/full_debug/remote

PROJ = wclient
CSRC = wclient.c

DEBUG = #-g

CFLAGS = -Wall -O2 -std=c99 $(DEBUG) -I../common -DVERBOSE
#CFLAGS = -Wall -O2 $(DEBUG) -I../common -DVERBOSE
LDFLAGS += -lm
LDFLAGS  = $(DEBUG) -o$(PROJ)

ifeq (debug,$d)
CFLAGS += -D__DEBUG
endif
ifeq (full_debug,$d)
CFLAGS += -D__DEBUG -D__DEBUG__
endif
ifeq (remote,$d)
CFLAGS += -D__DEBUG -D__DEBUG__ -D__REMOTE_SERIALS__
endif

ifeq (nto,$(SYSNAME))
CC = qcc
LDFLAGS += -lsocket
else
CC = gcc
#CSRC += genkey.c
endif

OBJS = $(CSRC:.c=.o)
VPATH = %.c ../common

$(PROJ): $(OBJS) ; $(LINK.o) $(OBJS)

$(OBJS): Makefile

clean: ; $(RM) $(OBJS) $(PROJ)

