CC = gcc

-include ../Makefile.diag.inc
CFLAGS = -O2 -g $(DIAG_CFLAGS) -Werror

LIBETH_CFLAGS += -I. -I../libeasy
LIBETH_LDFLAGS += -L../libeasy
LIBS += -lnl-3 -lnl-route-3 -lnl-genl-3
LIBS += -leasy

objs_lib = ethernet.o

ifeq ($(PLATFORM),BROADCOM)
objs_lib += bcm/bcm.o
endif

ifeq ($(PLATFORM),TEST)
CFLAGS += -DIOPSYS_TEST -I/usr/include/libnl3
LIBETH_CFLAGS += $(DIAG_CFLAGS) -Werror
objs_lib += test_stub/stub.o
endif

ifeq ($(PLATFORM),ECONET)
CFLAGS += -Itest_stub
LIBETH_CFLAGS += $(DIAG_CFLAGS) -Werror
objs_lib += econet/econet.o econet/ecnt_prvt.o
LIBS += -lapi_lib_switchmgr
endif

ifeq ($(PLATFORM),IPQ95XX)
objs_lib += ipq95xx/ipq95xx.o
endif

ifeq ($(PLATFORM),LINUX)
objs_lib += linux/linux_eth.o
endif

all: libethernet.so

%.o: %.c
	$(CC) $(CFLAGS) $(LIBETH_CFLAGS) -fPIC -c -o $@ $<

libethernet.so: $(objs_lib)
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(LDFLAGS) $(LIBETH_LDFLAGS) \
		-shared -o $@ $^ $(XXFLAGS) $(LIBS)

clean:
	rm -f *.o *.so
