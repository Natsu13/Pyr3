#pragma once

#include "Token.h"

Token::Token() {
}

Token::Token(String file_name) {
	this->file_name = file_name;
}

Token::Token(String file_name, int row, int column):Token(file_name) {
	this->row = row;
	this->column = column;
}