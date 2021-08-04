#pragma once
#include "Interpret.h"
#include "Bytecode.h"
#include "String.h"
#include "TypeResolver.h"

class BytecodeDebuger
{
private:
	Interpret* interpret;
	TypeResolver* typeResolver;
	vector<ByteCode*> instructions;
	vector<AST_Type*> types;
public:
	BytecodeDebuger(Interpret* inter, TypeResolver* typeres, vector<ByteCode*> inst, vector<AST_Type*> ts) : typeResolver(typeres), instructions(inst), types(ts), interpret(inter) {};
	void debug();
};

