#define MULTI_THREAD_TEST
#ifdef MULTI_THREAD_TEST
#include <iostream>
#include <chrono>
#include <cstdlib>
#include <time.h>
#include <thread>
#include <vector>
#endif
#include "skiplist.h"

#ifndef MULTI_THREAD_TEST
int main() {
	SkipList<string, int> sklist;
	sklist.insert("A", 1);
	sklist.insert("B", 2);
	sklist.insert("C", 3);
	sklist.insert("D", 4);
	sklist.insert("E", 5);
	sklist.insert("F", 6);
	sklist.insert("G", 7);
	sklist.insert("H", 8);
	sklist.insert("I", 9);
	sklist.display_skiplist();

	sklist.dumpfile();
	//sklist.loadfile(SAVE_PATH);
	//sklist.display_skiplist();

	sklist.delete_elem("H");
	sklist.display_skiplist();

	sklist.update("H", 100);
	sklist.display_skiplist();

	sklist.update("J", 10);
	sklist.display_skiplist();

	auto res = sklist.search("H");
	cout << res.first << " : " << res.second << endl;
	cout << sklist.size() << endl;
	return 0;
}

#else

#define NUM_THREADS 5
#define TEST_COUNT 100000
SkipList<int, string> skipList;

void insertElement() {
	int tmp = TEST_COUNT / NUM_THREADS;
	for (int count = 0; count < tmp; count++) {
		skipList.insert(rand() % TEST_COUNT, "a");
	}
}

void getElement() {
	int tmp = TEST_COUNT / NUM_THREADS;
	for (int count = 0; count < tmp; count++) {
		skipList.search(rand() % TEST_COUNT);
	}
}

int main() {
	srand(static_cast<unsigned int>(time(nullptr)));
	{
		vector<thread> threads;
		auto start = chrono::high_resolution_clock::now();

		for (int i = 0; i < NUM_THREADS; ++i) {
			cout << "main() : creating thread, " << i << endl;
			threads.emplace_back(insertElement);
		}

		for (auto& th : threads) {
			if (th.joinable()) {
				th.join();
			}
		}

		auto finish = chrono::high_resolution_clock::now();
		chrono::duration<double> elapsed = finish - start;
		cout << "insert elapsed: " << elapsed.count() << "s" << endl;
	}

	{
		vector<thread> threads;
		auto start = chrono::high_resolution_clock::now();

		for (int i = 0; i < NUM_THREADS; ++i) {
			cout << "main() : creating thread, " << i << endl;
			threads.emplace_back(getElement);
		}

		for (auto& th : threads) {
			if (th.joinable()) {
				th.join();
			}
		}

		auto finish = chrono::high_resolution_clock::now();
		chrono::duration<double> elapsed = finish - start;
		cout << "get elapsed: " << elapsed.count() << "s" << endl;
	}
	return 0;
}
#endif
