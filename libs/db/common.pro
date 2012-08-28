# kexidb global rules

include( $(KEXI)/common.pro )

win32:DEFINES += __KEXIDB__

win32:QMAKE_CXXFLAGS += /FI$(KEXI)/kexidb/global.h
win32:QMAKE_CFLAGS += /FI$(KEXI)/kexidb/global.h
