CC=$(CROSS_COMPILE)gcc

LIBPACK_INC=../libpack/include
CFLAGS+=-Wall -std=gnu11 -fPIC -I$(LIBPACK_INC)
LIBPACK_DIR=../libpack/lib
LDFLAGS+= -L$(LIBPACK_DIR) -lpthread -lpack -lconfuse

INCLUDE +=

# VERSION
MAJOR=0
MINOR=0
PATCH=0
VERSION = -DVERSION_MAJOR=$(MAJOR) -DVERSION_MINOR=$(MINOR) -DVERSION_PATCH=$(PATCH)

.PHONY: all

all: devdrvd

.PHONY: debug

debug: CFLAGS += -O0 -DDEBUG -g -Wno-unused-variable
debug: all


devdrvd : devdrvd.o control_daemon.o connect.o read_config.o scales.o gpios.o static_emulation.o
	$(CC) $(CFLAGS) -o devdrvd devdrvd.o control_daemon.o connect.o read_config.o scales.o gpios.o static_emulation.o $(LDFLAGS)

devdrvd.o : src/devdrvd.c
	$(CC) $(CFLAGS) $(VERSION) -c src/devdrvd.c $(INCLUDE)

control_daemon.o : src/control_daemon.c
	$(CC) $(CFLAGS) $(VERSION) -c src/control_daemon.c $(INCLUDE)

connect.o : src/connect.c
	$(CC) $(CFLAGS) $(VERSION) -c src/connect.c $(INCLUDE)

read_config.o : src/read_config.c
	$(CC) $(CFLAGS) $(VERSION) -c src/read_config.c $(INCLUDE)

scales.o : src/modules/scales.c
	$(CC) $(CFLAGS) $(VERSION) -c src/modules/scales.c $(INCLUDE)

gpios.o : src/modules/gpios.c
	$(CC) $(CFLAGS) $(VERSION) -c src/modules/gpios.c $(INCLUDE)

static_emulation.o : src/static_emulation.c
	$(CC) $(CFLAGS) $(VERSION) -c src/static_emulation.c $(INCLUDE)

DESTDIR=
prefix = /usr/local
exec_prefix = $(DESTDIR)$(prefix)
sbindir = $(exec_prefix)/sbin

.PHONY: install uninstall
install: all
	mkdir -p $(sbindir)
	install -v -m 0755 devdrvd $(sbindir)/devdrvd
	mkdir -p $(DESTDIR)/etc/devdrvd
	install -v -m 0644 config/devdrvd.conf $(DESTDIR)/etc/devdrvd

uninstall:
	-rm $(sbindir)/devdrvd
	-rm $(DESTDIR)/etc/devdrvd/devdrvd.conf

clean:
	-rm devdrvd
	-rm *.o
