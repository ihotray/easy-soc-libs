-include ../Makefile.diag.inc

LIBDSL = libdsl.so

CFLAGS += $(DIAG_CFLAGS) -Werror
HEADER_DEPS := xdsl.h common.h xtm.h

ifeq ($(PLATFORM),BROADCOM)
SRCS := $(shell ls broadcom/*.c ./*.c)
LIBDSL_CFLAGS += -I./broadcom -DLINUX -DLINUX_FW_EXTRAVERSION=50204 -DSUPPORT_DSL_GFAST -DCONFIG_VDSL_SUPPORTED
HEADER_DEPS += ./broadcom/adsldrv.h ./broadcom/AdslMibDef.h ./broadcom/bcmadsl.h
else ifeq ($(PLATFORM),INTEL)
SRCS := $(shell ls intel/*.c ./*.c)
else ifeq ($(PLATFORM),TEST)
SRCS := $(shell ls test_stub/*.c ./*.c)
LIBDSL_CFLAGS += -I. -Wall
else
$(error Unknown PLATFORM: $(PLATFORM))
endif
OBJS := $(SRCS:.c=.o)

all: $(LIBDSL)

%.o: %.c $(HEADER_DEPS)
	$(CC) $(LIBDSL_CFLAGS) $(CFLAGS) -fPIC -c -o $@ $<

libdsl.so: $(OBJS)
	$(CC) $(LIBDSL_CFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(LDFLAGS) -shared -o $@ $^

clean:
	rm -f *.o ./*/*.o $(LIBDSL)

export SRCS OBJS CFLAGS LOCAL_CFLAGS
debug:
	@echo "PLATFORM = $$PLATFORM"
	@echo "SRCS = $$SRCS"
	@echo "OBJS = $$OBJS"
	@echo "CFLAGS = $$CFLAGS"

.PHONY: all clean debug
