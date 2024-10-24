#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <mutex>
#include <shared_mutex>
using namespace std;


#define MAX_LEVEL 17  // 最高MAX_LEVEL层
#define SAVE_PATH "D:\\file.txt"

template<class K, class V>
struct Node {
private:
	K key;
	V value;
public:
	Node** next;
	Node(K key, V value, int level);
	~Node();
	const K& getKey() const;
	const V& getValue() const;
	void setValue(const V& v);
};

template<class K, class V>
Node<K, V>::Node(K key, V value, int level) {  // node在第level层
	this->key = key;
	this->value = value;
	next = new Node * [level + 1];
	memset(next, NULL, sizeof(Node*) * (level + 1));
}

template<class K, class V>
Node<K, V>::~Node() {
	delete[] next;
}

template<class K, class V>
const K& Node<K, V>::getKey() const {
	return this->key;
}

template<class K, class V>
const V& Node<K, V>::getValue() const {
	return this->value;
}

template<class K, class V>
void Node<K, V>::setValue(const V& v) {
	this->value = v;
}


template<class K, class V>
class SkipList {
private:
	Node<K, V>* head;
	int SkipListLevel;
	int nodeCount;
	int getRandomLevel();
	mutable shared_mutex mtx;
public:
	SkipList();
	~SkipList();
	bool insert(const K& key, const V& value);
	pair<bool, V> search(const K& key) const;
	void delete_elem(const K& key);
	bool update(const K& key, const V& value);
	void display_skiplist();
	int size() const;
	void dumpfile();
	void loadfile(const string& path);
};

template<class K, class V>
int SkipList<K, V>::getRandomLevel() {
	int level = 0;
	/*while (rand() % 2)
		level++;
	return level < MAX_LEVEL ? level : MAX_LEVEL;*/
	while ((rand() % 100) < 50 && level < MAX_LEVEL) {
		level++;
	}
	return level;
}

template<class K, class V>
SkipList<K, V>::SkipList() {
	this->nodeCount = 0;
	this->SkipListLevel = 0;
	this->head = new Node<K, V>(K(), V(), MAX_LEVEL);
}

template<class K, class V>
SkipList<K, V>::~SkipList() {
	while (head->next[0]) {
		Node<K, V>* cur = head->next[0];
		head->next[0] = cur->next[0];
		delete cur;
	}
	delete head;
}

template<class K, class V>
bool SkipList<K, V>::insert(const K& key, const V& value) {
	Node<K, V>* cur = this->head;
	Node<K, V>** prePtr = new Node<K, V>* [MAX_LEVEL + 1];
	memset(prePtr, NULL, (MAX_LEVEL + 1) * sizeof(Node<K, V>*));

	unique_lock<shared_mutex> lk(mtx);
	for (int i = SkipListLevel; i >= 0; i--) {
		while (cur->next[i] && cur->next[i]->getKey() < key) {
			cur = cur->next[i];
		}
		prePtr[i] = cur;
	}

	cur = cur->next[0];
	if (cur && cur->getKey() == key) {
		cout << key << " is exist:" << key << " : " << cur->getValue() << endl;
		return false;
	}

	int level = getRandomLevel();
	if (level > SkipListLevel) {
		int i = SkipListLevel + 1;
		while (i <= level)
			prePtr[i++] = head;
		SkipListLevel = level;
	}

	Node<K, V>* newNode = new Node<K, V>(key, value, level);
	for (int i = level; i >= 0; i--) {
		newNode->next[i] = prePtr[i]->next[i];
		prePtr[i]->next[i] = newNode;
	}

	nodeCount++;

	cout << key << " : " << value << " insert success" << endl;

	return true;
}

template<class K, class V>
pair<bool, V> SkipList<K, V>::search(const K& key) const {
	Node<K, V>* cur = this->head;
	shared_lock<shared_mutex> lk(mtx);
	for (int i = SkipListLevel; i >= 0; i--) {
		while (cur->next[i] && cur->next[i]->getKey() < key)
			cur = cur->next[i];

		if (cur->next[i] && cur->next[i]->getKey() == key) {
			cout << "find success " << key << " : " << cur->next[i]->getValue() << endl;
			return make_pair(true, cur->next[i]->getValue());
		}
	}

	cout << "not find" << endl;
	return make_pair(false, V());
}

template <class K, class V>
void SkipList<K, V>::delete_elem(const K& key) {
	auto res = search(key);
	if (res.first == false) {
		cout << "cant find " << key << endl;
		cout << key << " delete failed" << endl;
		return;
	}

	Node<K, V>* cur = this->head;
	Node<K, V>** prePtr = new Node<K, V>* [SkipListLevel + 1];
	memset(prePtr, NULL, (SkipListLevel + 1) * sizeof(Node<K, V>*));

	unique_lock<shared_mutex> lk(mtx);

	for (int i = SkipListLevel; i >= 0; i--) {
		while (cur->next[i] && cur->next[i]->getKey() < key)
			cur = cur->next[i];
		prePtr[i] = cur;
	}
	cur = cur->next[0];
	if (cur && cur->getKey() == key) {
		for (int i = 0; i <= SkipListLevel; i++) {
			if (prePtr[i]->next[i] && prePtr[i]->next[i]->getKey() == key) {
				prePtr[i]->next[i] = cur->next[i];
			}
			else
				break;
		}
		while (SkipListLevel > 0 && head->next[SkipListLevel] == nullptr) {
			SkipListLevel--;
		}
	}

	delete cur;
	nodeCount--;

	cout << key << " : " << res.second << " delete success" << endl;
}

template<class K, class V>
bool SkipList<K, V>::update(const K& key, const V& value) {
	Node<K, V>* cur = this->head;

	unique_lock<shared_mutex> lk(mtx);

	for (int i = SkipListLevel; i >= 0; i--) {
		while (cur->next[i] && cur->next[i]->getKey() < key)
			cur = cur->next[i];

		if (cur->next[i] && cur->next[i]->getKey() == key) {
			cur->next[i]->setValue(value);
			cout << key << " : " << value << " update success" << endl;
			return true;
		}
	}
	cout << key << " not found" << endl;
	cout << key << " update failed" << endl;
	return false;
}

template<class K, class V>
void SkipList<K, V>::display_skiplist() {
	for (int i = SkipListLevel; i >= 0; i--) {
		Node<K, V>* cur = this->head->next[i];
		cout << "Level: " << i << "   ";
		while (cur) {
			cout << cur->getKey() << " : " << cur->getValue() << "  ";
			cur = cur->next[i];
		}
		cout << endl;
	}
}

template<class K, class V>
int SkipList<K, V>::size() const {
	return nodeCount;
}

template<class K, class V>
void SkipList<K, V>::dumpfile() {
	ofstream out(SAVE_PATH, ios::trunc);
	if (!out.is_open()) {
		cerr << SAVE_PATH << " can not open success" << endl;
		return;
	}

	Node<K, V>* cur = this->head->next[0];
	while (cur) {
		out << cur->getKey() << " : " << cur->getValue() << '\n';
		cur = cur->next[0];
	}
	out.flush();
	out.close();
	cout << "data dump success" << endl;
}

template<class K, class V>
void SkipList<K, V>::loadfile(const string& path) {
	ifstream in(path);
	if (!in.is_open()) {
		cerr << path << " can not open success" << endl;
		return;
	}
	string line;
	while (getline(in, line)) {
		auto pos = line.find(':');
		if (pos != string::npos) {
			string key = line.substr(0, pos);
			string value = line.substr(pos + 1);

			key.erase(remove(key.begin(), key.end(), ' '), key.end());
			value.erase(remove(value.begin(), value.end(), ' '), value.end());

			insert(key, stoi(value));
		}
	}
	in.close();
	cout << "data load success" << endl;
}

