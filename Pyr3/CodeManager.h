#pragma once
#include "Parser.h"
#include "Interpret.h"

class CodeManager
{
private:
	Interpret* interpret;
public:
	CodeManager(Interpret* interpret);
	void manage(AST_Block* block);
};