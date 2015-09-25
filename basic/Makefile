include ../Make.rules

CFLAGS    += -I. $(GLIB_CFLAGS)
CXX_FLAGS  = -Wall -g -O2 -D__cplusplus $(GLIB_CFLAGS) -I../include -I$(INC_DIR)
LDFLAGS    = -L$(PROJECT_LIB_DIR) -L$(LIB_DIR)

HEADERS = JHMM.h Behavior.h Mds.h MotionDB.h HTKUnit.h PsymbolSpace.h WorkSpace.h
OBJS    = Common.o JHMM.o Behavior.o Mds.o MotionDB.o HTKUnit.o PsymbolSpace.o WorkSpace.o

ifeq ($(OS),Linux)
TARGET = libmimesis.so
else
TARGET = cygmimesis.dll
endif


OS = $(shell /bin/uname | sed s/-.*//)
ifeq ($(OS), CYGWIN_NT)
LINK	 = g++ -shared
LPFX	 = cyg
LSFX	 = dll
else
LINK	 = g++ -shared
LPFX	 = lib
LSFX	 = so
CFLAGS    += -fPIC
CXX_FLAGS += -fPIC
endif

TARGET   = $(LIB_DIR)/$(LPFX)mimesis.$(LSFX)

all:
	make $(TARGET)

install:
	install $(TARGET)  $(LIB_DIR)
#	install -d $(INC_DIR)/mimesis
#	install $(HEADERS) $(INC_DIR)/mimesis
#	install mimesis.h  $(INC_DIR)

#uninstall:
#	rm -f $(LIB_DIR)/$(TARGET)
#	rm -f $(INC_DIR)/mimesis/*

$(TARGET).a:	$(OBJS)
	$(LIB) $(TARGET) $(OBJS)
	$(RANLIB) $(TARGET)

$(TARGET):	$(OBJS)
	g++ $(LDFLAGS) -shared -o $@ $^ $(GLIB_LDFLAGS) -liirlib

$(OBJS): $(HEADERS)


Behavior.o: Behavior.cpp Behavior.h
	$(CXX) $(CXX_FLAGS) -c $< -I$(HTK_INCLUDE_DIR) -o $@

clean:
	$(RM) -f *.o *~ *.a *.so $(OBJS)
