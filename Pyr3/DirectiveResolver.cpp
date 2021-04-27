#include "DirectiveResolver.h"

DirectiveResolver::DirectiveResolver(Interpret* interpret)
{
	this->interpret = interpret;
}

void DirectiveResolver::resolve(AST_Block* block) {
	for (auto index = 0; index < block->expressions.size(); index++) {
		auto it = block->expressions[index];

		if (it->type != AST_DIRECTIVE/* && it->type != AST_DECLARATION && it->type != AST_BINARYOP*/) {
			continue;
		}		

		AST_Directive* directive = static_cast<AST_Directive*>(it);
		resolveDirective(directive);
	}
}

void DirectiveResolver::resolveDirective(AST_Directive* directive) {
	switch (directive->directive_type) {
	case D_IMPORT: {
			String fileName = "";
			if (directive->value0->type == AST_IDENT) {
				if (!(directive->flags & AST_DIRECTIVE_FLAG_INITIALIZED)) {
					interpret->report_error(directive->name->token, "Using unitialized directive");
				}	
				else {
					AST_Ident* ident = static_cast<AST_Ident*>(directive->value0);
					if (ident->flags & AST_IDENT_FLAG_CONSTANT) {

					}
					else {
						interpret->report_error(ident->name, "Can't use iden't that is not constant");
					}
				}
			}
			else if (directive->value0->type == AST_LITERAL) {
				AST_Literal* literal = static_cast<AST_Literal*>(directive->value0);
				if(literal->value_type == LITERAL_STRING)
					fileName = literal->string_value;
				else
					interpret->report_error(literal->token, "Can't use number for import");
			}
			
		}
		break;
	}
}