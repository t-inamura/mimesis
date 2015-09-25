#
# Modified by Tetsunari Inamura on 2008-10-07
#

include ../../Make.rules

TARGET = generation learning recognition space_create test similarity extra_test synthesis synthesis_graph

CFLAGS  += -g -I$(PROJECT_INC_DIR) $(GLIB_CFLAGS) 
CXX_FLAGS = -g -Wall -O2 -D__cplusplus -I$(PROJECT_INC_DIR) $(GLIB_CFLAGS)

LDFLAGS += -L$(LIB_DIR) -lmimesis -liirlib

all: $(TARGET)

install: all
	install $(TARGET) $(PROJECT_BIN_DIR)

lib:
	(cd ../../basic; make)

test: test.o
	$(CXX) -o $@ $^ $(LDFLAGS)

extra_test: extra_test.o
	$(CXX) -o $@ $^ $(LDFLAGS)

learning: learning.o
	$(CXX) -o $@ $^ $(LDFLAGS)

space_create: space_create.o
	$(CXX) -o $@ $^ $(LDFLAGS)

generation: generation.o
	$(CXX) -o $@ $^ $(LDFLAGS)

recognition: recognition.o
	$(CXX) -o $@ $^ $(LDFLAGS)

similarity: similarity.o
	$(CXX) -o $@ $^ $(LDFLAGS)

synthesis: synthesis.o
	$(CXX) -o $@ $^ $(LDFLAGS)
synthesis_graph: synthesis_graph.o
	$(CXX) -o $@ $^ $(LDFLAGS)

clean:
	$(RM) $(OBJS) *~ *.$(TRASHSFX) *.o
	for TGT in $(TARGET); do $(RM) $$TGT$(EXESFX); done;
	$(RM) $(PROJECT_BIN_DIR)/$(TARGET)
