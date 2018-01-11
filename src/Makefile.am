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


AM_CFLAGS = -I$(top_srcdir)/include -static -Wall -mmcu=$(MCU) $(OPT_OPTIMS) $(OPT_WARN)

lib_LIBRARIES = libsangster_atmega328p.a
libsangster_atmega328p_a_SOURCES = sangster/lcd.c \
                                   sangster/ring_buff_8.c \
                                   sangster/rtc_1307.c \
                                   sangster/sd.c \
                                   sangster/sd/sd_card.c \
                                   sangster/sd/sd_file.c \
                                   sangster/sd/sd_file__dir.c \
                                   sangster/sd/sd_file__open.c \
                                   sangster/sd/sd_file__remove.c \
                                   sangster/sd/sd_file__time.c \
                                   sangster/sd/sd_file__write.c \
                                   sangster/sd/sd_volume.c \
                                   sangster/sonar.c \
                                   sangster/timer0.c \
                                   sangster/twi.c \
                                   sangster/usart.c \
                                   sangster/usart_p.c \
                                   sangster/util.c

nobase_include_HEADERS = sangster/lcd.h \
                         sangster/lcd_charmap.h \
                         sangster/led_builtin.h \
                         sangster/pinout.h \
                         sangster/ring_buff_8.h \
                         sangster/rtc_1307.h \
                         sangster/sd.h \
                         sangster/sd/fat_structs.h \
                         sangster/sd/sd_card.h \
                         sangster/sd/sd_fat_mainpage.h \
                         sangster/sd/sd_file.h \
                         sangster/sd/sd_volume.h \
                         sangster/sonar.h \
                         sangster/timer0.h \
                         sangster/twi.h \
                         sangster/usart.h \
                         sangster/usart_p.h \
                         sangster/util.h

# TODO: How to do this with a static library?
# libsangster_atmega328p_a_LDFLAGS = -no-undefined -release $(SANGSTER_LT_RELEASE) \
#                                    -version-info $(SANGSTER_LT_CURRENT):$(SANGSTER_LT_REVISION):$(SANGSTER_LT_AGE)


DISTCLEANFILES = Makefile.in
