CC=g++
CFLAGS=-c
LDFLAGS=
SOURCES=main.cpp compressor.cpp compressor_c.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=compressor

all: $(SOURCES) $(EXECUTABLE)
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@
.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
clean:
	rm -rf *o compressor
