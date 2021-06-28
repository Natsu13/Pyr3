#pragma once
#include "Interpret.h"
#include "Bytecode.h"
#include "String.h"

class BytecodeDebuger
{
private:
	Interpret* interpret;
	vector<ByteCode*> instructions;
	vector<AST_Type*> types;
public:
	BytecodeDebuger(Interpret* inter, vector<ByteCode*> inst, vector<AST_Type*> ts) :instructions(inst), types(ts), interpret(inter) {};
	void debug();
};

