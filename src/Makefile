LIBS=$(shell pkg-config --libs opencv) 
CFLAGS=-Wall -ltesseract -O2
LDFLAGS=

SOURCES=main.cpp
TARGET=anpr

all: clean $(SOURCES) $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CFLAGS) $(LDFLAGS) $(SOURCES) -o $(TARGET) $(LIBS)

run:
	./$(TARGET)

clean:
	rm -f *.o $(TARGET)
