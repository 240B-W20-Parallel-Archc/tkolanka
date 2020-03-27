default: all

CC = g++
FLAGS = -std=c++11 -O3 -fopenmp

all: memory_time.cpp 
	$(CC) $(FLAGS) memory_time.cpp -o memory_time