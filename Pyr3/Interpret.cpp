#pragma once

#include "Interpret.h"
#include "Utils.h"

Interpret::Interpret() {
	initialize();
}

void Interpret::initialize() {
	//bit
	AST_Type_Definition* def_bit = AST_NEW_EMPTY(AST_Type_Definition);
	def_bit->internal_type = AST_Type_bit;
	def_bit->size = 1;
	this->type_def_bit = def_bit;

	//int
	AST_Type_Definition* def_int = AST_NEW_EMPTY(AST_Type_Definition);
	def_int->internal_type = AST_Type_int;
	def_int->size = 4;
	this->type_def_int = def_int;

	//float
	AST_Type_Definition* def_float = AST_NEW_EMPTY(AST_Type_Definition);
	def_float->internal_type = AST_Type_float;
	def_float->size = 4;
	this->type_def_float = def_float;

	//long
	AST_Type_Definition* def_long = AST_NEW_EMPTY(AST_Type_Definition);
	def_long->internal_type = AST_Type_long;
	def_long->size = 8;
	this->type_def_long = def_long;

	//s8
	AST_Type_Definition* def_s8 = AST_NEW_EMPTY(AST_Type_Definition);
	def_s8->internal_type = AST_Type_s16;
	def_s8->size = 1;
	this->type_def_s8 = def_s8;

	//s16
	AST_Type_Definition* def_s16 = AST_NEW_EMPTY(AST_Type_Definition);
	def_s16->internal_type = AST_Type_s16;
	def_s16->size = 2;
	this->type_def_s16 = def_s16;

	//s32
	AST_Type_Definition* def_s32 = AST_NEW_EMPTY(AST_Type_Definition);
	def_s32->internal_type = AST_Type_s32;
	def_s32->size = 4;
	this->type_def_s32 = def_s32;

	//s64
	AST_Type_Definition* def_s64 = AST_NEW_EMPTY(AST_Type_Definition);
	def_s64->internal_type = AST_Type_s64;
	def_s64->size = 8;
	this->type_def_s64 = def_s64;

	//char
	AST_Type_Definition* def_char = AST_NEW_EMPTY(AST_Type_Definition);
	def_char->internal_type = AST_Type_char;
	def_char->size = 1;
	this->type_def_char = def_char;
}

bool Interpret::isError() {
	return errorOccured;
}


void Interpret::report_error(CString message) {
	errorOccured = true;

	printf("\nerror: ");
	printf(message.data);
}
void Interpret::report_error(CString message, va_list argptr) {
	errorOccured = true;

	printf("\nerror: ");
	vfprintf(stdout, message.data, argptr);
}
void Interpret::report_error(CString message, const char* file, int row, int column, va_list argptr) {
	errorOccured = true;

	printf("\n%s:%d,%d error: ", file, row, column);
	vfprintf(stdout, message.data, argptr);
}
void Interpret::report_warning(CString message, va_list argptr) {
	printf("\nwarning: ");
	vfprintf(stdout, message.data, argptr);
}
void Interpret::report_warning(CString message, const char* file, int row, int column, va_list argptr) {
	printf("\n%s:%d,%d warning: ", file, row, column);
	vfprintf(stdout, message.data, argptr);
}

string get_file_name(string file) {
	std::string file_name(file.substr(file.rfind("\\") + 1));
	return file_name;
}

void Interpret::report_error(Token* token, const char* message, ...) {
	errorOccured = true;

	printf("\n%s:%d,%d error: ", get_file_name(token->file_name).c_str(), token->row, token->column);
	va_list args;
	va_start(args, message);
	vfprintf(stdout, message, args);
	va_end(args);
}
void Interpret::report_error(Token* token, CString message, ...) {
	errorOccured = true;

	printf("\n%s:%d,%d error: ", get_file_name(token->file_name).c_str(), token->row, token->column);
	va_list args;
	va_start(args, message);
	vfprintf(stdout, message.data, args);
	va_end(args);
}
void Interpret::report_warning(Token* token, const char* message, ...) {
	printf("\n%s:%d,%d warning: ", get_file_name(token->file_name).c_str(), token->row, token->column);
	va_list args;
	va_start(args, message);
	vfprintf(stdout, message, args);
	va_end(args);
}
void Interpret::report_warning(Token* token, CString message, ...) {
	printf("\n%s:%d,%d warning: ", get_file_name(token->file_name).c_str(), token->row, token->column);
	va_list args;
	va_start(args, message);
	vfprintf(stdout, message.data, args);
	va_end(args);
}