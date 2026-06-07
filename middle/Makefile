.SUFFIXES: .cpp .o
CC=g++
CFLAGS=-c -std=c++0x -O3 -DNDEBUG
# -DNVISIT
# The code is somewhat faster when the function calls to visit() are omitted (by defining NVISIT).
# This is in particular reasonable if the visit() function is empty (does nothing).
LDFLAGS=-static
SOURCES=vertex.cpp tree.cpp hamcycle.cpp main.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=middle
OTHERS=hamcycle.hpp tree.hpp vertex.hpp Makefile
ZIP=zip
ZIPFILE=middle.zip

all: $(EXECUTABLE) $(ZIPFILE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

$(ZIPFILE): $(SOURCES) $(OTHERS)
	$(ZIP) $(ZIPFILE) $(SOURCES) $(OTHERS)

clean:
	rm -f $(OBJECTS) $(EXECUTABLE) $(ZIPFILE)
