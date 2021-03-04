#pragma once

#include "Headers.h"
#include "Interpret.h"

class DirectiveResolver
{
private:
	Interpret* interpret = NULL;
public:
	DirectiveResolver(Interpret* interpret);
	void resolve(AST_Block* block);
	void resolveDirective(AST_Directive* directive);
};

