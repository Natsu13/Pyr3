#pragma once
#include "Headers.h"

template<typename TKey, typename TValue>
struct TableRecord {
	TKey key;
	TValue value;
};

template<typename TKey, typename TValue>
class Table {
private:
	vector<TableRecord<TKey, TValue>> records;
	uint32_t size = 0;
public:
	Table() {
		records = vector<TableRecord<TKey, TValue>>();
		size = 0;
	}

	typedef bool (*TableFindCallback)(TKey, TValue);
	typedef bool (*TableFindCallbackParam)(TKey, TKey, TValue);

	void insert(TKey key, TValue value) {
		if (exist(key)) throw "Key alerady exists in Table";

		TableRecord<TKey, TValue> tr = { key, value };
		records.push_back(tr);
		size++;
	}

	bool exist(TKey key) {
		for (int index = 0; index < size; index++) {
			TableRecord<TKey, TValue> tr = records[index];
			if (tr.key == key) {
				return true;
			}
		}
		return false;
	}

	TValue find(TKey key) {
		for (int index = 0; index < size; index++) {
			TableRecord<TKey, TValue> tr = records[index];
			if (tr.key == key) {
				return tr.value;
			}
		}

		return NULL;
	}

	TValue find(TableFindCallback callback) {
		for (int index = 0; index < size; index++) {
			TableRecord<TKey, TValue> tr = records[index];
			if (callback(tr.key, tr.value)) {
				return tr.value;
			}
		}

		return NULL;
	}

	TValue find(TKey param, TableFindCallbackParam callback) {
		for (int index = 0; index < size; index++) {
			TableRecord<TKey, TValue> tr = records[index];
			if (callback(param, tr.key, tr.value)) {
				return tr.value;
			}
		}

		return NULL;
	}

	TKey findKey(TableFindCallback callback) {
		for (int index = 0; index < size; index++) {
			TableRecord<TKey, TValue> tr = records[index];
			if (callback(tr.key, tr.value)) {
				return tr.key;
			}
		}

		return NULL;
	}
};