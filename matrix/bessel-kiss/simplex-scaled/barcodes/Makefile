PROJ = bc_scan

CSRC = bc_scan.c find_ACM.c mqtt_thread.c

OBJDIR = obj
RS232DIR = ../../librs232
//MQTTDIR = ../../mosquitto-2.0.15
MQTTDIR = ../../mqtt
MQTTINC = $(MQTTDIR)/include
MQTTLIB = $(MQTTDIR)/lib
JSONDIR = $(MQTTDIR)/cjson
#COMMON = $(JSONDIR)

DEBUG = #-g

DEFINES =  -D_GNU_SOURCE -DDEBUG
#CFLAGS = -Wall -pedantic -W -Werror -std=c99 -I$(RS232DIR)/include -I. -I$(MQTTINC) -I$(JSONDIR) $(DEFINES)
CFLAGS = -Wall -pedantic -W -Werror -std=c99 -I$(RS232DIR)/include -I. -I$(JSONDIR) $(DEFINES)
#CXXFLAGS = -m64 -pipe -O2 -fPIC -Wall -W -DEXPORT_TO_C $(DEFINES) -I$(COMMON)
CXXFLAGS = -m64 -pipe -O2 -fPIC -Wall -W -DEXPORT_TO_C -I$(COMMON)
LDFLAGS  = $(DEBUG) -o$(PROJ) $(OBJS)

#LDFLAGS += -lm -L$(MQTTLIB) -lmosquitto -lrt -lpthread
LDFLAGS += -lm -lmosquitto -lrt -lpthread

ifeq (nto,$(SYSNAME))
CC = qcc
else
CC = gcc
CXX = g++
endif

OBJS = $(addprefix $(OBJDIR)/,$(CSRC:.c=.o))

VPATH = %.c $(RS232DIR)/src $(JSONDIR)

$(PROJ): $(OBJS) ; $(LINK.o)

$(OBJDIR)/%.o : %.c ; $(COMPILE.c) $(OUTPUT_OPTION) $<
$(OBJDIR)/%.o : %.c ; $(COMPILE.c) $(OUTPUT_OPTION) $<

clean: ; $(RM) $(OBJS) $(PROJ)

.PHONY : clean

