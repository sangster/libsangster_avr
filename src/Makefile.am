AM_CFLAGS = -I$(top_srcdir)/include -Wall -mmcu=$(MCU)
lib_LTLIBRARIES = libsangster_atmega328p.la
libsangster_atmega328p_la_SOURCES = sangster/usart.h sangster/usart.c
nobase_include_HEADERS = sangster/usart.h

libsangster_atmega328p_la_LDFLAGS = -no-undefined -release $(SANGSTER_LT_RELEASE) -version-info $(SANGSTER_LT_CURRENT):$(SANGSTER_LT_REVISION):$(SANGSTER_LT_AGE)
