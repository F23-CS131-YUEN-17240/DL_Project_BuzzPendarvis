CC = g++

CFLAGS = 

TARGET = TomasuloDemo

all: $(TARGET)

$(TARGET): main.o cpu.o
	$(CC) $(CFLAGS) -o $(TARGET) main.o cpu.o

main.o: main.cpp cpu.hpp
	$(CC) $(CFLAGS) -c main.cpp

cpu.o: cpu.hpp