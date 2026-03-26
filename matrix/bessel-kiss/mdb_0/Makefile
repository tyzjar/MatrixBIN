PROJ = mdb

CSRC = main.c sock_stuff.c mdb.c cJSON.c \
		mqtt_thread.c tcp_thread.c
OBJDIR = obj
#MQTTDIR = ../../mosquitto-2.0.15
MQTTDIR = ../../mqtt
MQTTINC = $(MQTTDIR)/include
MQTTLIB = $(MQTTDIR)/lib
#JSONDIR = $(MQTTDIR)/cjson
JSONDIR = ../cjson
COMMON = $(JSONDIR)

DEBUG = #-g

DEFINES =  -D_GNU_SOURCE -DDEBUG
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

