MODULE := engines/avalanche

MODULE_OBJS = \
	acci.o \
	animation.o \
	avalanche.o \
	avalot.o \
	background.o \
	closing.o \
	console.o \
	detection.o \
	graphics.o \
	gyro.o \
	lucerna.o \
	menu.o \
	parser.o \
	pingo.o \
	scrolls.o \
	sequence.o \
	sound.o \
	timer.o
	
# This module can be built as a plugin
ifeq ($(ENABLE_AVALANCHE), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
