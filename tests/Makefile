LIBS += $(shell pkg-config --libs cppunit) $(shell pkg-config --libs opencv) -ltesseract
CFLAGS +=-Wall -Werror -O2
LDFLAGS += $(LIBS)

SOURCES := main.cpp TestRecognizer.cpp ../src/recognizer.cpp ../src/ocr_char.cpp
OBJECTS := $(SOURCES:.cpp=.o)
TARGET := test

all: $(SOURCES) $(TARGET)

.cpp.o:
	$(CXX) $(CFLAGS) -c $< -o $@ 

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)
	./$(TARGET)

clean:
	rm -f *.o $(TARGET) 
