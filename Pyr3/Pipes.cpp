#include "Pipes.h"

template<typename T>
Pipes<T>::Pipes() {
	vector<Pipe<T>> pipe = vector<Pipe<T>>();
	pipes.insert(index++, pipe);
}

template<typename T>
void Pipes<T>::check() {
	if (pipes.exist(index)) {
		return;
	}

	vector<Pipe<T>> pipe = vector<Pipe<T>>();
	pipes.insert(index++, pipe);
}

template<typename T>
vector<Pipe<T>> Pipes<T>::get_pipes(int index) {
	check();
	return pipes.find(index);
}

template<typename T>
int Pipes<T>::push(T element) {
	check();
	Pipe<T> pipe;
	pipe.element = element;
	auto _pipes = pipes.find(index);
	_pipes.push(pipe);
	return index;
}

template<typename T>
int Pipes<T>::current_pipe() {
	return index;
}

template<typename T>
int Pipes<T>::next_pipe() {
	index++;
	check();
	return index;
}