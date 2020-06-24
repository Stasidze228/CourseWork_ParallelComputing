#include <iostream>
#include <filesystem>
#include <thread>
#include <map>
#include <fstream>
#include <mutex>
#include <stdio.h> 
#include <queue>
#include <set>

std::vector<std::string> words_from_data(std::string inStr) {
	std::vector<std::string> res;
	char* comp;
	char* buffer = new char[strlen(inStr.c_str())];
	std::string s_comp;
	strcpy(buffer, inStr.c_str());
	comp = strtok(buffer, "!?<>_-#$%^&*.,:; \\*\"\'");
	while (comp != NULL) {
		s_comp = std::string(comp);
		transform(s_comp.begin(), s_comp.end(), s_comp.begin(), ::tolower);
		res.push_back(s_comp);
		comp = strtok(NULL, "!?<>_-#$%^&*.,:; \\*\"\'");
	}
	return res;
}
void indexer(std::queue<std::filesystem::path>* q, std::mutex* mtx, std::map<std::string, std::set<std::filesystem::path>>* current_map) {
	std::filesystem::path current_path;
	std::ifstream istr;
	std::string current_data;
	while (true) {
		mtx->lock();
		if (q->empty()) {
			mtx->unlock();
			break;
		}
		current_path = q->front();
		q->pop();
		mtx->unlock();
		istr.open(current_path);
		current_data = { std::istreambuf_iterator<char>(istr), std::istreambuf_iterator<char>() };
		istr.close();
		for (std::string &i : words_from_data(current_data))
			(*current_map)[i].insert(current_path);
	}
}
int main()
{
	std::string dir;
	std::cout << "Enter directory:" << std::endl;
	std::cin >> dir;
	std::queue<std::filesystem::path>* q = new std::queue<std::filesystem::path>;
	for (std::filesystem::directory_entry entry : std::filesystem::directory_iterator(dir))
		q->push(entry);
	unsigned num_of_threads = std::thread::hardware_concurrency();
	std::vector<std::thread> *threads = new std::vector<std::thread>;
	std::mutex* mtx = new std::mutex;
	std::map<std::string, std::set<std::filesystem::path>>* maps = new std::map<std::string, std::set<std::filesystem::path>>[num_of_threads];
	auto tStart = std::chrono::steady_clock::now();
	for (unsigned i = 0; i < num_of_threads; i++)
		threads->insert(threads->begin(), std::thread(indexer, q, mtx, &(maps[i])));
	for (std::thread &t : *threads)
		t.join();
	auto tEnd = std::chrono::steady_clock::now();
	auto indexing_time = std::chrono::duration_cast<std::chrono::milliseconds>(tEnd - tStart);
	std::cout << "Indexing took " << indexing_time.count() << " milliseconds" << std::endl;
	std::string word;
	while (true) {
		std::cout << "Search: ";
		std::cin >> word;
		transform(word.begin(), word.end(), word.begin(), ::tolower);
		for (int i = 0; i < num_of_threads; i++)
			for (auto i : maps[i][word])
				std::cout << i << std::endl;
	}
	return 0;
}