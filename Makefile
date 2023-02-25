# Makefile for easy-soc-libs

subdirs ?= $(dir $(wildcard */))

.PHONY: all clean install

all clean install:
	for i in $(subdirs) ; do [ -d $$i ] && $(MAKE) -C $$i $@ || exit; done
