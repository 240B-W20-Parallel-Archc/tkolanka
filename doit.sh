# Compile the program
make

# Run program with single thread and read-only case and store the data in rd_thread_1 file
./memory_time | tee rd_thread_1

# In order to run the code with different thread numbers and/or read-modify-write case the necessary variables 
# must be modified in the memory_time.cpp file
