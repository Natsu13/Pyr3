#pragma once
template<typename T>
class Array
{
private:
	void* items = nullptr;
	int _size = 0;
	int index = 0;
	int offset = 0;

	void resize(int newsize) {
		if (_size == 0) {
			_size = newsize;

			items = malloc(_size * sizeof(T));
			if (items == nullptr)
				throw "Can't alloc enought memory";

			return;
		}

		int oldSize = newsize;
		_size = newsize;
		auto oldData = items;

		items = (T*)malloc(_size * sizeof(T));
		if (items == nullptr)
			throw "Can't alloc enought memory";

		memcpy(items, oldData, oldSize);
	}
public:
	Array() {
		index = 0;
		resize(1);
	}
	Array(int size) {
		index = 0;
		resize(size);
	}
	T& operator [] (int index) {
		//return &items + index * sizeof(T);
		if ((index+ offset) > _size) {
			resize(index + offset + 1); //if we want save to index outside of _size just expand it :3 soo unlimited size array!
		}

		return (T&)*(&items + (index + offset) * sizeof(T));
	}
	T operator [] (int index) const {
		return &items + (index + offset) * sizeof(T);
	}
	void reserve(int size) {
		resize(size);
	}
	void reserve(int size, T fill) {
		resize(size);

		if (fill != NULL) {
			memset(items, fill, size);
		}
		else {
			memset(items, NULL, size);
		}
	}
	void push_back(T* object) {
		//memcpy(&items + (index * sizeof(T))+1, object, sizeof(T));
		auto indexPtr = (T*)items + (index + offset);
		indexPtr = object;
		index++;
	}
	int size() {
		return _size;
	}
	void set_offset(int o) {
		offset = o;
	}
	int get_offset() {
		return offset;
	}
};