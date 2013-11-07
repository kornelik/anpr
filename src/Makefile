LIBS += $(shell pkg-config --libs opencv) -ltesseract

CFLAGS +=-Wall -Werror -O2
LDFLAGS += $(LIBS)

SOURCES := main.cpp recognizer.cpp conio.cpp ocr_char.cpp util.cpp
OBJECTS := $(SOURCES:.cpp=.o)
TARGET := anpr

all: $(SOURCES) $(TARGET)

.cpp.o:
	$(CROSS_COMPILE)-$(CXX) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS)
	$(CROSS_COMPILE)-$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)

run:
	./$(TARGET)

test: $(OBJECTS) recognizer_quality.o
	$(CROSS_COMPILE)-$(CXX) recognizer.o recognizer_quality.o ocr_char.o util.o -o testing $(LDFLAGS)
	./testing

clean:
	rm -f $(TARGET)
	rm -f $(OBJECTS)
