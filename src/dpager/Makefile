CC = gcc
CFLAGS = -g -Wall -m32 
TARGET = loader
OBJECT = main.o loadelf.o call.o

$(TARGET): $(OBJECT)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECT) -L./src -ldl

clean:
	rm -f *.o