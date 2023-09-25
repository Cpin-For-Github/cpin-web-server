ROOT = .
TARGET = $(ROOT)\bin\tests\target
BIN = $(ROOT)\bin\tests

main: test01.o pool.o thread.o
	g++ $(TARGET)\test01.o $(TARGET)\pool.o $(TARGET)\thread.o -o $(BIN)\test01

pool.o: $(ROOT)\src\_Pool.cpp
	g++ $(ROOT)\src\_Pool.cpp -c -I $(ROOT)\src -o $(TARGET)\pool.o

thread.o: $(ROOT)\src\Thread.cpp
	g++ $(ROOT)\src\Thread.cpp -c -I $(ROOT)\src -o $(TARGET)\thread.o

test01.o: $(ROOT)\tests\test01_pool.cpp
	g++ $(ROOT)\tests\test01_pool.cpp -I $(ROOT)\src -c -o $(TARGET)\test01.o


.PHONY: clean

clean:
	del $(TARGET)\*o $(BIN)\*.exe
