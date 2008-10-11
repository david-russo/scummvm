MODULE := gui

MODULE_OBJS := \
	about.o \
	browser.o \
	chooser.o \
	console.o \
	debugger.o \
	dialog.o \
	editable.o \
	EditTextWidget.o \
	launcher.o \
	ListWidget.o \
	massadd.o \
	message.o \
	newgui.o \
	object.o \
	options.o \
	PopUpWidget.o \
	ScrollBarWidget.o \
	TabWidget.o \
	theme.o \
	themebrowser.o \
	ThemeEngine.o \
	ThemeEval.o \
	ThemeLayout.o \
	ThemeParser.o \
	widget.o

# Include common rules
include $(srcdir)/rules.mk
