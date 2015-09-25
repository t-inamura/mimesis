OS = $(shell /bin/uname | sed s/-.*//)

ifeq ($(OS),Linux)

include Makefile.Linux

else

include Makefile.Cygwin

endif
