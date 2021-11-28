#pragma once

/*
template<typename T>
class Array
{
private:
    T* items = nullptr;
    int _size = 0;
    int index = 0;
    int offset = 0;

    void resize(int newsize) {
        if (_size == 0) {
            _size = newsize;

            items = (T*)malloc(_size * sizeof(T));
            if (items == nullptr)
                throw "Can't alloc enought memory";

            return;
        }

        int oldSize = _size;
        auto oldData = items;
        _size = newsize + 1;

        items = (T*)malloc(_size * sizeof(T));
        if (items == nullptr)
            throw "Can't alloc enought memory";

        memcpy(items, oldData, oldSize);

        if (oldData != nullptr)
            free(oldData);
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
        if ((index + offset) >= _size) {
            resize((index + offset) * 2); //if we want save to index outside of _size just expand it :3 soo unlimited size array!
        }

        auto addr = &(*(items + index + offset));
        //auto addr2 = (T&)*(items + (index + offset) * sizeof(T));
        auto add = &addr;
        //auto add2 = &addr2;
        return (T&)(*(items + index + offset));
    }
    T operator [] (int index) const {
        return items + (index + offset);
    }
    T get(int index) {
        return items + (index + offset);
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
*/