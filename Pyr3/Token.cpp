#pragma once

#include "Token.h"

Token::Token() {
}

Token::Token(string file_name) {
	this->file_name = file_name;
}

Token::Token(string file_name, int row, int column):Token(file_name) {
	this->row = row;
	this->column = column;
}