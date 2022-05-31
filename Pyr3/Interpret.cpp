#pragma once

#include "Interpret.h"
#include "Utils.h"

Interpret::Interpret() {
	initialize();
	counter = 0;
}

#define create_new_type(name, internalType, Size, Align) this->name = new AST_Type_Definition(); \
this->name->internal_type = internalType; \
this->name->size = Size; \
this->name->aligment = Align; \

void Interpret::initialize() {
	create_new_type(type_pointer, AST_Type_pointer, 4, 4); //pointer
	create_new_type(type_address, AST_Type_address, 4, 4); //address	
	create_new_type(type_c_call, AST_Type_c_call, 4, 4); //c_call
	create_new_type(type_bool, AST_Type_bool, 1, 1); //bit

	create_new_type(type_bit, AST_Type_bit, 1, 1); //bit
	create_new_type(type_float, AST_Type_float, 4, 4); //float
	create_new_type(type_long, AST_Type_long, 8, 8); //long

	create_new_type(type_s8, AST_Type_s8, 1, 1); //s8
	create_new_type(type_s16, AST_Type_s16, 2, 2); //s16
	create_new_type(type_s32, AST_Type_s32, 4, 4); //s32
	create_new_type(type_s64, AST_Type_s64, 8, 8); //s64

	create_new_type(type_u8, AST_Type_u8, 1, 1); //u8
	create_new_type(type_u16, AST_Type_u16, 2, 2); //u16
	create_new_type(type_u32, AST_Type_u32, 4, 4); //u32
	create_new_type(type_u64, AST_Type_u64, 8, 8); //u64

	create_new_type(type_char, AST_Type_char, 1, 1); //char
	create_new_type(type_string, AST_Type_string, 1, 1); //string
}

bool Interpret::isError() {
	return errorOccured;
}


void Interpret::report_error(String message) {
	errorOccured = true;

	printf("\nerror: ");
	printf(message.data);
}
void Interpret::report_error(String message, va_list argptr) {
	errorOccured = true;

	printf("\nerror: ");
	vfprintf(stdout, message.data, argptr);
}
void Interpret::report_error(String message, const char* file, int row, int column, va_list argptr) {
	errorOccured = true;

	printf("\n%s:%d,%d error: ", file, row, column);
	vfprintf(stdout, message.data, argptr);
}
void Interpret::report_warning(String message, va_list argptr) {
	printf("\nwarning: ");
	vfprintf(stdout, message.data, argptr);
}
void Interpret::report_warning(String message, const char* file, int row, int column, va_list argptr) {
	printf("\n%s:%d,%d warning: ", file, row, column);
	vfprintf(stdout, message.data, argptr);
}

string get_file_name(String file) {
	std::string _file = file.data;
	std::string file_name(_file.substr(_file.rfind("\\") + 1));
	return file_name;
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
void Interpret::report_error(Token* token, String message, ...) {
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
void Interpret::report_info(Token* token, const char* message, ...) {
	printf("\n%s:%d,%d : ", get_file_name(token->file_name).c_str(), token->row, token->column);
	va_list args;
	va_start(args, message);
	vfprintf(stdout, message, args);
	va_end(args);
}
void Interpret::report(const char* message, ...) {
	printf("\n\t\t");
	va_list args;
	va_start(args, message);
	vfprintf(stdout, message, args);
	va_end(args);
}
void Interpret::report_warning(Token* token, String message, ...) {
	printf("\n%s:%d,%d warning: ", get_file_name(token->file_name).c_str(), token->row, token->column);
	va_list args;
	va_start(args, message);
	vfprintf(stdout, message.data, args);
	va_end(args);
}