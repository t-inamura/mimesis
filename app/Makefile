#
# Makefile for MCMS
# Last modified by Tetsunari Inamura on 2004 Nov 18th
#

include ../Make.rules

SUBDIRS = test

all:
	for TARGET in $(SUBDIRS); do ($(CD) $$TARGET; $(MAKE) $@;) done;

clean:
	for TARGET in $(SUBDIRS); do ($(CD) $$TARGET; $(MAKE) $@;) done;
	$(RM) *~
