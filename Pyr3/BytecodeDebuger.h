#pragma once
#include "Interpret.h"
#include "Bytecode.h"
#include "CString.h"

class BytecodeDebuger
{
private:
	vector<ByteCode*> instructions;
public:
	BytecodeDebuger(vector<ByteCode*> inst) :instructions(inst) {};
	void debug();
};

