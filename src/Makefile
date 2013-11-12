LIBS += $(shell pkg-config --libs opencv) -ltesseract

CFLAGS +=-Wall -Werror -O2
LDFLAGS += $(LIBS)

SOURCES := main.cpp recognizer.cpp conio.cpp ocr_char.cpp util.cpp
OBJECTS := $(SOURCES:.cpp=.o)
TARGET := anpr
TESTING_BINARY := testing_binary
RECOGNIZER_QUALITY_BINARY := recognizer_quality.o

all: $(SOURCES) $(TARGET)

.cpp.o:
	$(CROSS_COMPILE)-$(CXX) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS)
	$(CROSS_COMPILE)-$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)

run:
	./$(TARGET)

test: $(OBJECTS) $(RECOGNIZER_QUALITY_BINARY) 
	$(CROSS_COMPILE)-$(CXX) recognizer.o recognizer_quality.o ocr_char.o util.o -o $(TESTING_BINARY) $(LDFLAGS)
	./$(TESTING_BINARY)

clean:
	rm -f $(TARGET)
	rm -f $(OBJECTS)
	rm -f $(TESTING_BINARY)
	rm -f $(RECOGNIZER_QUALITY_BINARY)
