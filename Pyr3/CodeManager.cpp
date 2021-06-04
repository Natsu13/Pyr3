#pragma once

#include "CodeManager.h"

//This will resolver stuff like all types struct, string, int, function
CodeManager::CodeManager(Interpret* interpret) {
	this->interpret = interpret;
}

void CodeManager::manage(AST_Block* block) {

}

//lexer->current_line_number
//current_file_name
//add_directive_import AST_Directive_Import
//add_directive_load AST_Directive_Load