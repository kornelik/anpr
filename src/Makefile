LIBS=$(shell pkg-config --libs opencv)
CFLAGS=-Wall -Werror -O2
LDFLAGS=

SOURCES=main.cpp recognizer.cpp conio.cpp ocr_char.cpp
TARGET=anpr

all: clean $(SOURCES) $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CFLAGS) $(LDFLAGS) $(SOURCES) -o $(TARGET) $(LIBS)

run:
	./$(TARGET)

test: clean recognizer.cpp recognizer_quality.cpp ocr_char.cpp
	$(CXX) $(CFLAGS) $(LDFLAGS) recognizer.cpp recognizer_quality.cpp ocr_char.cpp -o testing $(LIBS)
	./testing

clean:
	rm -f *.o $(TARGET)
