PROJ = libpack.so
CSRC = crc16.c devdrvd_communication.c
OBJDIR = obj

INCLUDEDIR = "../include"
DEFINES =  -D_GNU_SOURCE -DDEBUG -DVERBOSE
CFLAGS = -fPIC -Wall -pedantic -W -Werror -std=c99 -I$(CONFUSEDIR) -I. -I$(INCLUDEDIR)
LDFLAGS = -shared  -o$(PROJ)  $(OBJS)

CC = gcc
CXX = g++

OBJS = $(addprefix $(OBJDIR)/,$(CSRC:.c=.o))

lib/$(PROJ): $(OBJS) ; $(LINK.o)

$(OBJDIR)/%.o : %.c ; $(COMPILE.c) $(OUTPUT_OPTION) $<

clean: ; $(RM) $(OBJS) $(PROJ)

.PHONY : clean

