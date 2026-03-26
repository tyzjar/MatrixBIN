PROJ = matrixlib.so
OBJDIR = obj

CSRC = \
    matrixlib.c lib_version.c get_tac.c load_configuration.c save_configuration.c \
	set_maximum.c set_minimum.c set_range_mode.c set_increment_size.c set_dec_point.c set_calibrate_zero.c \
	set_calibrate_gane.c set_calibrate_zero_correction.c set_linearization.c save_calibration.c set_warm_up_time.c \
	zero_tracking.c set_zero_manual_range.c set_zero_initial_range.c zero_set.c zero_reset.c zero_save.c \
	zero_restore.c tare_mode.c tare_set.c tare_reset.c tare_preset.c tare_save.c tare_restore.c tare_auto_set.c \
	tare_auto_clear.c \
	processing_cur_measurement.c filtering_perform.c calculate_weight.c normalize_weight.c switch_unit.c is_motion.c \
	mode_x10.c set_channel_on_off.c \
	print.c save_results.c \
	set_wim_max_capacity.c set_wim_min_capacity.c set_wim_max_platform.c set_wim_min_platform.c \
	set_wim_max_wagon_mass.c set_wim_min_wagon_mass.c set_wim_max_train_mass.c set_wim_min_train_mass.c \
	set_wim_max_speed.c set_wim_min_speed.c set_wim_max_transit_speed.c set_wim_adjusment.c wim_calculate_speed.c \
	wim_solver.c wim_print.c wim_save_results.c reset_to_factory.c

HSRC = \
	interface_matrixlib.h typedef_matrixlib.h


OBJS = $(CSRC:.c=.o)
HDRS = $(HSRC)

DEBUG = # -g

DEFINES =  -D_GNU_SOURCE -DDEBUG
CFLAGS = -Wall -pedantic -W -Werror -std=c99 -I. $(DEFINES) -fPIC
#CXXFLAGS = -m64 -pipe -O2 -fPIC -Wall -W -DEXPORT_TO_C $(DEFINES) -I$(COMMON)
CXXFLAGS =  -m64 -pipe -O2 -fPIC -Wall -W -DEXPORT_TO_C $(DEBUG)
LDFLAGS  = -shared $(DEBUG) -o$(PROJ) $(OBJS) -lpq

ifeq (nto,$(SYSNAME))
CC = qcc
else
#CC = gcc
C = gcc
CXX = g++
endif


LINK.o = $(C) $(LDFLAGS) $(TARGET_ARCH)

OBJS = $(addprefix $(OBJDIR)/,$(CSRC:.c=.o))

VPATH = %.c

$(PROJ): $(OBJS) Makefile
	 $(LINK.o)

$(OBJDIR)/%.o : %.c $(HDRS)
	$(COMPILE.c) $(OUTPUT_OPTION) $<

clean: ; $(RM) $(OBJS) $(PROJ)

.PHONY : clean


