PROG = libdsl_test
OBJS = libdsl_test.o

PROG_CFLAGS = $(CFLAGS) -fstrict-aliasing -I../
PROG_LDFLAGS = $(LDFLAGS) -L.. -ldsl

ifeq ($(TARGET_PLATFORM),INTEL)
PROG_LDFLAGS += -L/opt/intel/usr/lib -ldslfapi -lhelper -lsysfapi
endif

%.o: %.c
	$(CC) $(PROG_CFLAGS) $(FPIC) -c -o $@ $<

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(PROG_LDFLAGS) -o $@ $^

clean:
	rm -f *.o $(PROG)
