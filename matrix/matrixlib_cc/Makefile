PROJ = Matrix_lib.so
OBJDIR = obj

CSRC = \
    matrixlib.cc lib_version.cc get_tac.cc load_configuration.cc save_configuration.cc \
	set_maximum.cc set_minimum.cc set_range_mode.cc set_increment_size.cc set_dec_point.cc set_calibrate_zero.cc \
	set_calibrate_gane.cc set_calibrate_zero_correction.cc set_linearization.cc save_calibration.cc set_warm_up_time.cc \
	zero_tracking.cc set_zero_manual_range.cc set_zero_initial_range.cc zero_set.cc zero_reset.cc zero_save.cc \
	zero_restore.cc tare_mode.cc tare_set.cc tare_reset.cc tare_preset.cc tare_save.cc tare_restore.cc tare_auto_set.cc \
	tare_auto_clear.cc \
	processing_cur_measurement.cc filtering_perform.cc calculate_weight.cc normalize_weight.cc switch_unit.cc \
	is_motion.cc mode_x10.cc set_channel_on_off.cc \
	print.cc save_results.cc \
	set_wim_max_capacity.cc set_wim_min_capacity.cc set_wim_max_platform.cc set_wim_min_platform.cc \
	set_wim_max_wagon_mass.cc set_wim_min_wagon_mass.cc set_wim_max_train_mass.cc set_wim_min_train_mass.cc \
	set_wim_max_speed.cc set_wim_min_speed.cc set_wim_max_transit_speed.cc set_wim_adjusment.cc wim_calculate_speed.cc \
	wim_solver.cc wim_print.cc wim_save_results.cc reset_to_factory.cc set_maximum_channel.cc set_discret.cc \
	set_averag_time.cc add_scale.cc set_zero_indication.cc
	

HSRC = \
	interface_matrixlib.h typedef_matrixlib.h


OBJS = $(CSRC:.cc=.o)
HDRS = $(HSRC)

DEBUG = # -g

DEFINES =  -D_GNU_SOURCE -DDEBUG
CFLAGS = -Wall -pedantic -W -Werror -I. $(DEFINES) -fPIC
#CXXFLAGS = -m64 -pipe -O2 -fPIC -Wall -W -DEXPORT_TO_C $(DEFINES) -I$(COMMON)
CXXFLAGS =  -m64 -pipe -O2 -fPIC -Wall -W -DEXPORT_TO_C $(DEBUG)
LDFLAGS  = -shared $(DEBUG) -o$(PROJ) $(OBJS) #-lpq

ifeq (nto,$(SYSNAME))
CC = qcc
else
#CC = gcc
C = gcc
CXX = g++
endif


LINK.o = $(CXX) $(LDFLAGS) $(TARGET_ARCH)

OBJS = $(addprefix $(OBJDIR)/,$(CSRC:.cc=.o))

VPATH = %.cc

$(PROJ): $(OBJS) Makefile
	 $(LINK.o)

$(OBJDIR)/%.o : %.cc $(HDRS)
	$(COMPILE.cc) $(OUTPUT_OPTION) $<

clean: ; $(RM) $(OBJS) $(PROJ)

.PHONY : clean


