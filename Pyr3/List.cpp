#include "Headers.h"
#include "List.h"

template<typename T>
inline List<T>::List()
{
	this->size = 1;
	this->items.resize(1);
}

template<typename T>
List<T>::List(int size)
{
	if (size < 1 || size > INT32_MAX) {
		throw new invalid_argument("Argument size can't be less than 1 or more then " + INT32_MAX);
	}

	this->size = size;
	this->items.resize(size);
}

template<typename T>
List<T>::List(std::vector<T> items)
{
	this->size = items.size();
	this->items.resize(this->size);
	for (int i = 0; i < this->size; i++) {
		this->items[i] = items[i];
	}
}

template<typename T>
void List<T>::increase()
{
	this->size *= 2;
	this->items.resize(this->size);
}

template<typename T>
inline void List<T>::push(T item)
{
	if (this->index + 1 >= this->size) {
		this->increase();
	}

	this->items[this->index++] = item;
}

template<typename T>
inline void List<T>::clear()
{
	this->index = 0;
	this->size = 1;
	this->items.clear();
	this->items.resize(1);
}
