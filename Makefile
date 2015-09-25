include ./Make.rules

all:
	for TARGET in $(SITE_TARGETS); do ($(CD) $$TARGET;  $(MAKE) $@;) done;

clean:
	for TARGET in $(SITE_TARGETS); do ($(CD) $$TARGET;  $(MAKE) $@;) done;
	$(RM) *~ *.stackdump

tags:
	etags basic/*.cpp basic/*.h app/test/*.cpp

tarball:
	$(MAKE) clean
	tgz Mimesis$(DATE).tgz $(SITE_TARGETS)
	mv Mimesis$(DATE).tgz program_backup/.
#	tgz Mimesis$(DATE).tgz $(SITE_TARGETS) update.log etc data

