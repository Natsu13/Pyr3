#pragma once
#include "Table.h"

template<typename T>
struct Pipe {
	T element;
};

template<typename T>
class Pipes
{
private:
	Table<int, vector<Pipe<T>>> pipes;
	int index = 0;
	void check();
public:
	Pipes();
	int push(T element);
	vector<Pipe<T>> get_pipes(int index);
	int current_pipe();
	int next_pipe();
};