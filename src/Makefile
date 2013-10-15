LIBS += $(shell pkg-config --libs opencv) -ltesseract

CFLAGS +=-Wall -Werror -O2
LDFLAGS += $(LIBS)

SOURCES := main.cpp recognizer.cpp conio.cpp ocr_char.cpp util.cpp
OBJECTS := $(SOURCES:.cpp=.o)
TARGET := anpr

all: $(SOURCES) $(TARGET)

.cpp.o:
	$(CXX) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)

run:
	./$(TARGET)

test: $(OBJECTS) recognizer_quality.o
	$(CXX) recognizer.o recognizer_quality.o ocr_char.o util.o -o testing $(LDFLAGS)
	./testing

clean:
	rm -f $(TARGET)
	rm -f $(OBJECTS)
