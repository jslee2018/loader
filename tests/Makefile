CC = gcc
CFLAGS = -m32 -pie -fPIE
BUILD = ./build
TARGET = $(shell ls | grep .c | tr '.c' ' ')
OBJECT = $(shell ls | grep .c | tr '.c' '.o')

all: $(TARGET)

$(TARGET): $(OBJECT)
	$(CC) $(CFLAGS) -o $(BUILD)/$@ $@.o

clean:
	rm -f *.o