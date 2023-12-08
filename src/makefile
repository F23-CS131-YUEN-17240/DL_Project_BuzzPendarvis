CC = g++

CFLAGS = 

TARGET = TomasuloDemo

all: $(TARGET)

$(TARGET): main.o architecture.o resstation.o exec.o cpu.o
	$(CC) $(CFLAGS) -o $(TARGET) main.o architecture.o resstation.o exec.o cpu.o

main.o: main.cpp architecture.cpp resstation.cpp exec.cpp cpu.cpp
	$(CC) $(CFLAGS) -c main.cpp

architecture.o: architecture.hpp

resstation.o: resstation.hpp architecture.hpp

exec.o: exec.hpp resstation.hpp architecture.hpp

cpu.o: cpu.hpp exec.hpp resstation.hpp architecture.hpp