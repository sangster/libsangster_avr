AUTOMAKE_OPTIONS = subdir-objects

OPT_OPTIMS  = -Os
OPT_OPTIMS += -ffunction-sections -fdata-sections
OPT_OPTIMS += -mrelax

OPT_WARN =  -Wall
OPT_WARN += -pedantic
OPT_WARN += -Wextra
OPT_WARN += -Wmissing-declarations
OPT_WARN += -Wmissing-field-initializers
OPT_WARN += -Wsystem-headers
OPT_WARN += -Wno-variadic-macros

AM_CFLAGS = -I$(top_srcdir)/include -static -mmcu=atmega328p $(OPT_OPTIMS) $(OPT_WARN)

lib_LIBRARIES = libsangster_avr_atmega328p.a

libsangster_avr_atmega328p_a_CFLAGS = -L/usr/avr/lib $(COMMON_CFLAGS) $(AM_CFLAGS)
libsangster_avr_atmega328p_a_LDFLAGS = -L/usr/avr/lib $(COMMON_LIBS)
libsangster_avr_atmega328p_a_SOURCES = sangster/atmega328p/timer0.c \
                                       sangster/atmega328p/twi.c \
                                       sangster/atmega328p/usart.c \
                                       sangster/atmega328p/usart_p.c

nobase_include_HEADERS = sangster/atmega328p/led_builtin.h \
                         sangster/atmega328p/timer0.h \
                         sangster/atmega328p/twi.h \
                         sangster/atmega328p/usart.h \
                         sangster/atmega328p/usart_p.h

DISTCLEANFILES = Makefile.in
