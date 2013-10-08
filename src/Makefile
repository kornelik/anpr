LIBS=$(shell pkg-config --libs opencv) 
CFLAGS=-Wall -ltesseract -O2
LDFLAGS=

SOURCES=main.cpp recognizer.cpp
TARGET=anpr

all: clean $(SOURCES) $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CFLAGS) $(LDFLAGS) $(SOURCES) -o $(TARGET) $(LIBS)

run:
	./$(TARGET)

test: clean recognizer.cpp recognizer_ut.cpp
	$(CXX) $(CFLAGS) $(LDFLAGS) recognizer.cpp recognizer_ut.cpp -o testing $(LIBS)
	./testing

clean:
	rm -f *.o $(TARGET)
