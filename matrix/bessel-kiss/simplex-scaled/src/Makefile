PROJ = scaled

CSRC = config.c scaled.c rs232.c rs232_posix.c ldu_loop.c dis2_loop.c dis4_loop.c \
		devdrvd_communication.c \
		packet_daemon.c sock_stuff.c commands.c locbuf.c \
		json_stuff.c cJSON.c mqtt_thread.c
OBJDIR = obj
CONFUSEDIR = ../../libconfuse/src
RS232DIR = ../../librs232/include
PACKDIR = ../../libpack/include
#COMMON = ../../librs232/src ../../libpack/src
MQTTDIR = ../../mqtt
MQTTINC = $(MQTTDIR)/include
MQTTLIB = $(MQTTDIR)/lib
JSONDIR = ../../cjson
COMMON = $(JSONDIR) ../../librs232/src ../../libpack/src

DEBUG = #-g

DEFINES =  -D_GNU_SOURCE -DDEBUG -DVERBOSE
CFLAGS = -Wall -pedantic -W -Werror -std=c99 -I$(CONFUSEDIR) -I. -I$(RS232DIR) -I$(PACKDIR) -I$(MQTTINC) -I$(JSONDIR) -pthread $(DEFINES)
#CXXFLAGS = -m64 -pipe -O2 -fPIC -Wall -W -DEXPORT_TO_C $(DEFINES) -I$(COMMON)
CXXFLAGS = -m64 -pipe -O2 -fPIC -Wall -W -DEXPORT_TO_C -I$(COMMON)
LDFLAGS  = $(DEBUG) -o$(PROJ) $(OBJS)

LDFLAGS += -lm -lpthread -L$(CONFUSEDIR) -lconfuse -L$(MQTTLIB) -lmosquitto -lrt

ifeq (nto,$(SYSNAME))
CC = qcc
else
CC = gcc
CXX = g++
endif

OBJS = $(addprefix $(OBJDIR)/,$(CSRC:.c=.o))

VPATH = %.c . $(COMMON)

$(PROJ): $(OBJS) ; $(LINK.o)

$(OBJDIR)/%.o : %.c ; $(COMPILE.c) $(OUTPUT_OPTION) $<
$(OBJDIR)/%.o : %.c ; $(COMPILE.c) $(OUTPUT_OPTION) $<

clean: ; $(RM) $(OBJS) $(PROJ)

.PHONY : clean

