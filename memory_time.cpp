#include <iostream>
#include <algorithm>
#include <vector>
#include <chrono>
#include <cmath>
#include <numeric>
#include <omp.h>

using namespace std;
using namespace std::chrono;

#define NUM_THREADS 1
//#define RMW

struct Node {
	uint64_t data = 0;
	Node* next = nullptr;
};

// Returns nanoseconds per element.
double time(uint64_t N, uint64_t iters) {
	// Allocate all the memory required for creating the linked list of N elements
	vector<Node> memory(N);

	// Initialize the node pointers
	vector<Node*> nodes(N);
	for (uint64_t i=0; i<N; ++i) {
		nodes[i] = &memory[i];
	}
	
	// Randomize the linked list so that each memory access is random.
	// This is the worst case scenario for a linked list, which we want to measure.
	random_shuffle(begin(nodes), end(nodes));

	// Creating the linked list from the nodes
	for (uint64_t i=0; i<N-1; ++i) {
		nodes[i]->next = nodes[i+1];
	}

	Node* head = nodes[0];

	nodes.clear(); 
	nodes.shrink_to_fit(); // Free up unused memory

	// Actual memory access time measurement

	auto start = steady_clock::now();

	for (uint64_t it=0; it<iters; ++it) {
		// Access all elements in the linked list
		uint64_t count = 0;
		Node* node = head;
		while (node) {
			#ifdef RMW
				auto data = node->data;
				data += count + it;
				node->data = data;
				count += 5;
			#endif
			
			node = node->next;
		}
	}

	auto stop = steady_clock::now();
	auto duration = duration_cast<nanoseconds>(stop - start);

	return duration.count() / double(N * iters);
}

int main(int argc, const char * argv[])
{
	cout << "#bytes    ns/elem" << endl;

	uint64_t steps = 4; // Number of measurements per size
	uint64_t minFactor = 6;  // First measurement: 2^minFactor.
	uint64_t maxFactor = 30; // Last measurement: 2^maxFactor.

	uint64_t min = steps * minFactor;
	uint64_t max = steps * maxFactor;

	for (uint64_t i=min; i<=max; ++i) {
		uint64_t N = (uint64_t)round(pow(2.0, double(i) / steps));
		uint64_t reps = (uint64_t)round(2e10 / pow(N, 1.5));
		if (reps<1) 
			reps = 1;
		
		auto total_time = 0.0;
		#pragma omp parallel num_threads(NUM_THREADS)
		{
			auto ind_time = time(N, reps);
			#pragma omp atomic
				total_time += ind_time;
		}
		auto avg_time = total_time/NUM_THREADS;
		
		cout << (N*sizeof(Node)) << "   " << avg_time << "   # (N=" << N << ", reps=" << reps << ") " << (i-min+1) << "/" << (max-min+1) << endl;
	}
}
