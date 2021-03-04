#pragma once
#include "Headers.h"

template<typename T>
class List
{
private:
	vector<T> items;
	int size = 0;
	int index = 0;

	void increase();

public:
	List();
	List(int size);
	List(std::vector<T> items);	
	inline void push(T item);
	inline void clear();
};

