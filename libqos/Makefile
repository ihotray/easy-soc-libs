CC = gcc

-include ../Makefile.diag.inc
CFLAGS = -O2 -g

LIBQOS_CFLAGS += -I./include -I../libeasy
LIBQOS_LDFLAGS += -L../libeasy
LIBS += -leasy

OBJ_LIB = src/qos.o

ifeq ($(PLATFORM),BROADCOM)
LIB_DIR = broadcom
ifneq (,$(filter -DCONFIG_BCM963178 -DCONFIG_BCM947622, $(CFLAGS)))
LIBQOS_CFLAGS += -I./$(STAGING_DIR)/usr/include/bcm963xx/bcmdrivers/opensource/include/bcm963xx/
OBJ_LIB += $(addprefix $(LIB_DIR)/src/, bcm_qos_archer.o )
else
LIBQOS_CFLAGS += -I$(STAGING_DIR)/usr/include/bcm963xx/rdp/drivers/rdpa_gpl/include
LIBQOS_CFLAGS += -I$(STAGING_DIR)/usr/include/bcm963xx/bcmdrivers/opensource/include/bcm963xx
LIBQOS_CFLAGS += -I$(STAGING_DIR)/usr/include/bcm963xx/rdp/drivers/rdpa_user/include
LIBQOS_CFLAGS += -I$(STAGING_DIR)/usr/include/bcm963xx/rdp/drivers/bdmf/framework
LIBQOS_CFLAGS += -I$(STAGING_DIR)/usr/include/bcm963xx/rdp/drivers/bdmf/system/linux
LIBQOS_CFLAGS += -I$(STAGING_DIR)/usr/include/bcm963xx/rdp/drivers/bdmf/system/linux/cm
ifneq (,$(filter -DCONFIG_BCM963158 -DCONFIG_BCM968880 -DCONFIG_BCM96856, $(CFLAGS)))
LIBQOS_CFLAGS += -DKERNEL_64
endif
OBJ_LIB += $(addprefix $(LIB_DIR)/src/, bcm_qos_runner.o )
endif
LIBQOS_CFLAGS += -I../libethernet/bcm
LIBQOS_CFLAGS += -I$(STAGING_DIR)/usr/include/bcm963xx/shared/opensource/include/bcm963xx/
LIBQOS_CFLAGS += -I$(STAGING_DIR)/usr/include/bcm963xx/bcmdrivers/opensource/include/bcm963xx
LIBQOS_CFLAGS += -I$(STAGING_DIR)/usr/include/bcm963xx/bcmdrivers/opensource/include/bcm963xx/bcm
endif

ifeq ($(PLATFORM),TEST)
LIB_DIR = test
OBJ_LIB += $(addprefix $(LIB_DIR)/, test.o )
LIBQOS_CFLAGS += $(DIAG_CFLAGS) -Werror -DIOPSYS_TEST -I./$(LIB_DIR) -I/usr/include/libnl3
endif

ifeq ($(PLATFORM),ECONET)
LIB_DIR = econet
OBJ_LIB += $(addprefix $(LIB_DIR)/src/, ecnt_qos.o )
LIBQOS_CFLAGS += $(DIAG_CFLAGS) -Werror
endif

ifeq ($(PLATFORM),IPQ95XX)
LIB_DIR = linux
LIBS += -lnl-3
OBJ_LIB += $(addprefix $(LIB_DIR)/, linux_qos.o )
LIBQOS_CFLAGS += $(DIAG_CFLAGS) -Werror -I./$(LIB_DIR)
endif

ifeq ($(PLATFORM),LINUX)
LIB_DIR = linux
LIBS += -lnl-3
OBJ_LIB += $(addprefix $(LIB_DIR)/, linux_qos.o )
LIBQOS_CFLAGS += $(DIAG_CFLAGS) -Werror -I./$(LIB_DIR)
endif

all: libqos.so

%.o: %.c
	$(CC) $(CFLAGS) $(LIBQOS_CFLAGS) -fPIC -c -o $@ $<

libqos.so: $(OBJ_LIB)
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(LDFLAGS) $(LIBQOS_LDFLAGS) \
		-shared -o $@ $^ $(XXFLAGS) $(LIBS)

clean:
	rm -f *.o *.so
