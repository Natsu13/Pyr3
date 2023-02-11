#pragma once

#include "Headers.h"
#include "Utils.h"
#include "TypeResolver.h"

TypeResolver::TypeResolver(Interpret* interpret) {
	this->interpret = interpret;
	this->copier = new Copier(interpret);
	any_change = false;
}

bool TypeResolver::has_changes() {
	return any_change/* && to_be_resolved.size() > 0*/; //@todo: the condition with the to be reoslved is not good because at end cycle when we resolve everything but have changes we dont do new cycle but we need it
}

AST_Literal* TypeResolver::make_string_literal(String value) {
	AST_Literal* literal = AST_NEW_EMPTY(AST_Literal);
	literal->value_type = LITERAL_STRING;
	literal->string_value = value;

	return literal;
}

AST_Literal* TypeResolver::make_number_literal(long long value) {
	AST_Literal* literal = AST_NEW_EMPTY(AST_Literal);
	literal->value_type = LITERAL_NUMBER;
	literal->integer_value = value;

	if (value < 0) literal->number_flags |= NUMBER_FLAG_SIGNED;

	return literal;
}

AST_Literal* TypeResolver::make_number_literal(int value) {
	AST_Literal* literal = AST_NEW_EMPTY(AST_Literal);
	literal->value_type = LITERAL_NUMBER;
	literal->integer_value = value;

	return literal;
}

AST_Literal* TypeResolver::make_number_literal(float value) {
	AST_Literal* literal = AST_NEW_EMPTY(AST_Literal);
	literal->value_type = LITERAL_NUMBER;
	literal->float_value = value;
	literal->number_flags |= NUMBER_FLAG_FLOAT;

	return literal;
}

void TypeResolver::resolve_main(AST_Block* block) {
	phase = 1;
	resolve(block);
	/*while (to_be_resolved.size() > 0) {
		resolveOther();
	}*/
}

AST_Type* TypeResolver::get_inferred_type(AST_Expression* expression) {
	if (expression->type == AST_IDENT) {
		auto ident = static_cast<AST_Ident*>(expression);
		return ident->type_declaration->inferred_type;
	}
	if (expression->type == AST_TYPE) {
		return static_cast<AST_Type*>(expression);
	}

	assert(false);
	return NULL;
}

void TypeResolver::addToResolve(AST_Expression* expression) {
	if (expression->is_resolved) return;
	For(to_be_resolved) {
		if (it == expression) return; //but we should look whe this can happend!
	}
	to_be_resolved.push_back(expression);
}

void TypeResolver::resolveOther() {
	phase += 1;

	if (any_change || phase == 2) {
		any_change = false;

		if (to_be_resolved.size() != 0) {
			vector<AST_Expression*> resolve;
			//because we adding to the resolve as we found it we shout reverse resolve it
			for (int i = to_be_resolved.size() - 1; i >= 0; i--) {
				auto expression = to_be_resolved[i];
				if (expression->is_resolved) continue;
				resolve.push_back(expression);
			}
			to_be_resolved.clear();

			for (int i = 0; i < resolve.size(); i++) {
				resolveExpression(resolve[i]);
			}
		}
	}
}

void TypeResolver::resolve(AST_Block* block) {
	if (block == NULL) return;

	vector<AST_Expression*> resolve_after;
	for (auto index = 0; index < block->expressions.size(); index++) {
		auto it = block->expressions[index];
		if (it->type == AST_DECLARATION || it->type == AST_OPERATOR) {
			if (it->type == AST_DECLARATION) {
				auto decl = static_cast<AST_Declaration*>(it);
				if (decl->value != NULL && decl->value->type != AST_PROCEDURE) {
					resolveExpression(it);
					continue;
				}
			}
			else if(it->type == AST_OPERATOR) {
				auto op = static_cast<AST_Operator*>(it);
				resolveExpression(op);
				continue;
			}
		}		
		resolve_after.push_back(it);
	}	

	for (auto index = 0; index < resolve_after.size(); index++) {
		auto it = resolve_after[index];
		auto type = resolveExpression(it);
	}
}

bool TypeResolver::is_static(AST_Expression* expression) {
	if (expression->type == AST_IDENT) {
		AST_Ident* ident = static_cast<AST_Ident*>(expression);
		auto decl = find_declaration(ident, ident->scope);
		AST_Expression* expr = decl->value;

		if (!((decl->flags & AST_IDENT_FLAG_CONSTANT) == AST_IDENT_FLAG_CONSTANT)) {
			return false;
		}

		return is_static(expr);
	}
	if (expression->type == AST_LITERAL) {
		return true;
	}
	if (expression->type == AST_BINARY) {
		AST_Binary* binop = static_cast<AST_Binary*>(expression);

		return is_static(binop->left) && is_static(binop->right);
	}
	if (expression->type == AST_BLOCK) {
		AST_Block* block = static_cast<AST_Block*>(expression);
		for (int i = 0; i < block->expressions.size(); i++) {
			if (!is_static(block->expressions[i])) {
				return false;
			}
		}
		return true;
	}

	return false;
}

String TypeResolver::get_string_from_literal(AST_Expression* expression) {
	if (expression->type == AST_LITERAL) {
		auto lit = static_cast<AST_Literal*>(expression);
		if(lit->value_type == LITERAL_STRING) 
			return lit->string_value;

		assert(false);
	}
	if (expression->type == AST_IDENT) {
		auto ident = static_cast<AST_Ident*>(expression);
		auto decl = find_declaration(ident, ident->scope);

		return get_string_from_literal(decl);
	}

	assert(false);
}

int TypeResolver::do_int_operation(int left, int right, int op) {
	switch (op) {
		case BINOP_PLUS: return left + right;
		case BINOP_MINUS: return left - right;
		case BINOP_TIMES: return left * right;
		case BINOP_DIV: return left / right;
		case BINOP_MOD: return left % right;
		case BINOP_ISEQUAL: return left == right;
		case BINOP_ISNOTEQUAL: return left != right;
		case BINOP_GREATER: return left > right;
		case BINOP_GREATEREQUAL: return left >= right;
		case BINOP_LESS: return left < right;
		case BINOP_LESSEQUAL: return left <= right;
		case BINOP_LOGIC_AND: return left && right;
		case BINOP_LOGIC_OR: return left || right;
		case BINOP_BITWISE_AND: return left & right;
		case BINOP_BITWISE_OR: return left | right;
	}

	assert(false);
}

int TypeResolver::calculate_size_of_static_expression(AST_Expression* expression) {
	if (!is_static(expression)) {
		assert(false && "Expression is not static!");
	}

	if (expression->type == AST_LITERAL) {
		AST_Literal* lit = static_cast<AST_Literal*>(expression);
		if (lit->value_type == LITERAL_NUMBER) {
			return lit->integer_value;
		}
		assert(false && "Literar must be type of integer");
	}
	if (expression->type == AST_IDENT) {
		AST_Ident* ident = static_cast<AST_Ident*>(expression);
		auto decl = find_declaration(ident, ident->scope);
		return calculate_size_of_static_expression(decl->value);
	}
	if (expression->type == AST_BINARY) {
		AST_Binary* binop = static_cast<AST_Binary*>(expression);

		int size_left = calculate_size_of_static_expression(binop->left);
		int size_right = calculate_size_of_static_expression(binop->right);

		return do_int_operation(size_left, size_right, binop->operation);
	}

	assert(false && "Unsuported expression type");
}

int TypeResolver::calculate_array_size(AST_Type* type, bool first) {
	uint64_t resolveSize = 1;

	if (type->kind == AST_TYPE_ARRAY) {
		AST_Array* arr = static_cast<AST_Array*>(type);
		if (arr->size == 0) {
			return 1; //array view
		}

		if (!is_static(arr->size)) {
			interpret->report_error(arr->token, "Size of array must be constant");
			return 0;
		}

		int resolveSize = calculate_size_of_static_expression(arr->size);
		if (arr->point_to->type == AST_TYPE) {
			AST_Type* type = static_cast<AST_Type*>(arr->point_to);

			return resolveSize * calculate_array_size(type, false);
		}

		return resolveSize;
	}
	else if (!first) {
		if (type->kind == AST_TYPE_DEFINITION) {
			AST_Type_Definition* type_def = static_cast<AST_Type_Definition*>(type);
			return type_def->size;
		}
		else if (type->kind == AST_TYPE_STRUCT) {
			AST_Struct* _struct = static_cast<AST_Struct*>(type);
			calculate_type_size(_struct);
			return _struct->size;
		}
		else if (type->kind == AST_TYPE_UNION) {
			AST_Union* _union = static_cast<AST_Union*>(type);
			calculate_type_size(_union);
			return _union->size;
		}
		else {
			return 1;
		}
	}

	return resolveSize;
}

int TypeResolver::get_size_of(AST_Expression* expr) {
	if (expr->type == AST_TYPE) {
		auto type = (AST_Type*)expr;
		
		if (type->kind == AST_TYPE_DEFINITION) {
			auto tdef = (AST_Type_Definition*)expr;
			return tdef->size;
		}
		else if (type->kind == AST_TYPE_ARRAY) {
			return calculate_array_size(type);
		}
		else if (type->kind == AST_TYPE_STRUCT || type->kind == AST_TYPE_UNION) {			
			calculate_type_size(type);

			if (type->kind == AST_TYPE_STRUCT) {
				auto _struct = (AST_Struct*)type;
				return _struct->size;
			}
			else if (type->kind == AST_TYPE_UNION) {
				auto _union = (AST_Union*)type;
				return _union->size;
			}
			assert(false);
		}
	}
	else if (expr->type == AST_DECLARATION) {
		auto declaration = (AST_Declaration*)expr;
		auto type_def = static_cast<AST_Type_Definition*>(declaration->inferred_type);
		return type_def->size;
	}

	return 0;
}

AST_Block* TypeResolver::get_members_of_type(AST_Type* _type) {
	if (_type->kind == AST_TYPE_STRUCT) {
		return ((AST_Struct*)_type)->members;
	}
	else if (_type->kind == AST_TYPE_UNION) {
		return ((AST_Union*)_type)->members;
	}
	
	assert(false);
}

bool TypeResolver::ensure_elements_is_resolved(AST_Type* _type) {
	auto members = get_members_of_type(_type);

	for (int i = 0; i < members->expressions.size(); i++) {
		auto it = members->expressions[i];
		if (!it->is_resolved) return false;
	}
	return true;
}

void TypeResolver::calculate_type_size(AST_Type* _type, int offset) {
	auto members = get_members_of_type(_type);

	int size = 0;
	int max_size = 0;
	for (int i = 0; i < members->expressions.size(); i++) {
		auto it = members->expressions[i];

		if (it->type == AST_DECLARATION) {
			auto declaration = static_cast<AST_Declaration*>(it);
			if (declaration->inferred_type->kind == AST_TYPE_DEFINITION) {
				auto type_def = static_cast<AST_Type_Definition*>(declaration->inferred_type);
				declaration->offset = offset + size;

				size+= type_def->size;
				if (type_def->size > max_size) max_size = type_def->size;

				continue;
			}
			else if (declaration->inferred_type->kind == AST_TYPE_ENUM) {
				auto _enum = static_cast<AST_Enum*>(declaration->inferred_type);
				declaration->offset = offset + size;
				int new_size = 0;
				if (_enum->enum_type == NULL)
					new_size = interpret->type_s32->size;
				else
					new_size = get_size_of(_enum->enum_type);
				
				size += new_size;
				if (new_size > max_size) max_size = new_size;

				continue;
			}
			else if (declaration->inferred_type->kind == AST_TYPE_UNION) {
				auto _union = static_cast<AST_Union*>(declaration->inferred_type);
				declaration->offset = offset + size;
				if (!_union->is_resolved) {
					resolveExpression(_union);
				}
				size += _union->size;
				if (_union->size > max_size) max_size = _union->size;

				continue;
			}
			else if (declaration->inferred_type->kind == AST_TYPE_POINTER) {
				declaration->offset = offset + size;
				
				size += interpret->type_pointer->size; 
				if (interpret->type_pointer->size > max_size) max_size = interpret->type_pointer->size;

				continue;
			}
			assert(false);
		}
		else if (it->type == AST_TYPE) {
			auto type = static_cast<AST_Type*>(it);
			if (type->kind == AST_STRUCT) {
				auto _type_struct = static_cast<AST_Struct*>(type);
				calculate_type_size(_type_struct, size);

				size += _type_struct->size;
				if (_type_struct->size > max_size) max_size = _type_struct->size;
				
				continue;
			}
			else {
				assert(false);
			}
		}

		assert(false);
	}

	if (_type->kind == AST_TYPE_STRUCT) {
		((AST_Struct*)_type)->size = size;
	}
	else {
		((AST_Union*)_type)->size = max_size;
	}
}

#define resolved(expression) if(expression->is_resolved == false) { any_change = true; expression->is_resolved = true; }
#define set_type_and_break(_type) type = _type; break;
AST_Type* TypeResolver::resolveExpression(AST_Expression* expression) {
	AST_Type* type = NULL;
	switch (expression->type) {
		case AST_IDENT: {
			AST_Ident* ident = static_cast<AST_Ident*>(expression);
			set_type_and_break(resolveIdent(ident));
		}
		case AST_CONDITION: {
			AST_Condition* condition = static_cast<AST_Condition*>(expression);

			resolveExpression(condition->condition);
			resolveExpression(condition->body_pass);
			if (condition->body_fail != NULL)
				resolveExpression(condition->body_fail);
			set_type_and_break(NULL);
		}
		case AST_BLOCK: {
			AST_Block* _block = static_cast<AST_Block*>(expression);
			resolve(_block);
			set_type_and_break(NULL);
		}
		case AST_DECLARATION: {
			AST_Declaration* declaration = static_cast<AST_Declaration*>(expression);
			set_type_and_break(resolveDeclaration(declaration));
		}
		case AST_TYPE: {
			AST_Type* ast_type = static_cast<AST_Type*>(expression);
			set_type_and_break(resolveType(ast_type));
		}
		case AST_LITERAL: {
			AST_Literal* literal = static_cast<AST_Literal*>(expression);
			set_type_and_break(resolveLiteral(literal));
		}
		case AST_BINARY: {
			AST_Binary* binop = static_cast<AST_Binary*>(expression);
			set_type_and_break(resolveBinary(binop));
		}
		case AST_PROCEDURE: {
			AST_Procedure* procedure = static_cast<AST_Procedure*>(expression);
			
			bool isGenericFunction = false;
			if (procedure->header != NULL) {
				vector<AST_Type*> generic_to_add;

				resolve(procedure->header);

				for (int index = 0; index < procedure->header->expressions.size(); index++) {
					AST_Expression* expr = procedure->header->expressions[index];
					expr->flags |= DECLARATION_IN_HEAD;
					if ((expr->flags & TYPE_DEFINITION_GENERIC) == TYPE_DEFINITION_GENERIC) {
						isGenericFunction = true;
						resolved(expr);						

						//we know it's generic soo we know the assigment type is AST_Ident
						auto ident = (AST_Ident*)((AST_Declaration*)expr)->assigmet_type;
						auto ast_generic = AST_NEW_EMPTY(AST_Generic);
						ast_generic->type_declaration = AST_NEW_EMPTY(AST_Declaration);
						ast_generic->type_declaration->ident = ident;
						generic_to_add.push_back(ast_generic);
					}
				}
				
				For(generic_to_add) {
					procedure->header->expressions.push_back(it);
				}

				generic_to_add.clear();
			}		

			if (isGenericFunction) {
				procedure->flags |= AST_PROCEDURE_FLAG_GENERIC;
			}

			if (procedure->foreign_library_expression != NULL) {
				resolveExpression(procedure->foreign_library_expression);
				if (procedure->foreign_library_expression->type != AST_LITERAL) {
					if (!is_static(procedure->foreign_library_expression)) {
						interpret->report_error(procedure->foreign_library_expression->token, "Name of foreign library must be constant");
					}
					else {
						auto old = procedure->foreign_library_expression;
						auto news = get_string_from_literal(old);
						procedure->foreign_library_expression = make_string_literal(news);
						copier->copy_token(old, procedure->foreign_library_expression);
					}
				}
			}

			resolve(procedure->body);			

			if(procedure->returnType != NULL)
				resolveExpression(procedure->returnType);
			
			resolved(expression);

			if (procedure->returnType != NULL) {
				if (procedure->returnType->type == AST_PARAMLIST) {
					set_type_and_break(interpret->type_pointer);//@todo: maybe for now???
				}
				set_type_and_break(find_typeof(procedure->returnType));
			}
			set_type_and_break(interpret->type_pointer);//@todo: void!!!
		}
		case AST_PARAMLIST: {
			AST_ParamList* param_list = static_cast<AST_ParamList*>(expression);			

			if ((param_list->flags & AST_PARAMLIST_IS_DECLARATION) == AST_PARAMLIST_IS_DECLARATION) {
				/*resolveExpression(param_list->expressions[0]); //we resolve the first expression
				auto ident = (AST_Ident*)param_list->expressions[0];
				if (ident->type_declaration == NULL) {
					addToResolve(param_list);
					set_type_and_break(NULL);
				}*/

				auto first = param_list->expressions[0];
				if (first->type != AST_IDENT) break;

				auto ident = (AST_Ident*)first;
				auto decl = find_declaration(ident, ident->scope);
				auto value = decl->value;
				if (value == NULL) { // a, b: s64;
					//assert(false);
				}
				else if (value->type == AST_UNARYOP) {  // a, b := call();
					auto unary = (AST_Unary*)value;
					auto proc = find_procedure(unary->left, unary->arguments);
					if (proc == NULL) {
						addToResolve(expression);
						set_type_and_break(NULL);
					}

					//auto proc = (AST_Procedure*)value;
					auto ret = proc->returnType;
					if (ret->type != AST_PARAMLIST) {
						interpret->report_error(param_list->token, "Procedure don't match, it must return param list");//@todo: fix error message
						param_list->is_resolved = true;
						set_type_and_break(NULL);
					}

					auto ret_params = (AST_ParamList*)ret;

					if (ret_params->expressions.size() != param_list->expressions.size()) {
						interpret->report_error(param_list->token, "Procedure don't match, it must return param list at same size");//@todo: fix error message
						interpret->report_info(param_list->token, "Got: %d, but want: %d", ret_params->expressions.size(), param_list->expressions.size());
						param_list->is_resolved = true;
						set_type_and_break(NULL);
					}

					For(param_list->expressions) {
						if (it->type == AST_DECLARATION) continue;
						auto left = (AST_Ident*)it;
						auto right = ret_params->expressions[it_index];

						auto decl = make_declaration(left, NULL);
						decl->assigmet_type = right;
						left->type_declaration = decl; //@todo: maybe not!
						param_list->expressions[it_index] = decl;
						addToResolve(decl);
					}
					resolved(param_list);
				}
			} else {
				For(param_list->expressions) {
					resolveExpression(it);
				}
				resolved(param_list);
			}
			set_type_and_break(NULL);
		}
		case AST_UNARYOP: {
			AST_Unary* unary = static_cast<AST_Unary*>(expression);
			set_type_and_break(resolveUnary(unary));
		}
		case AST_RETURN: {
			AST_Return* ast_return = static_cast<AST_Return*>(expression);
			if (ast_return->value == NULL) {
				resolved(ast_return);
				break;
			}

			if (ast_return->value->type == AST_PARAMLIST) {
				auto param_list = (AST_ParamList*)ast_return->value;

				For(param_list->expressions) {
					resolveExpression(it);
				}
			}
			set_type_and_break(resolveExpression(ast_return->value));
		}
		case AST_DIRECTIVE: {
			AST_Directive* directive = static_cast<AST_Directive*>(expression);
			resolveDirective(directive);
			set_type_and_break(NULL);
		}
		case AST_CAST: {
			AST_Cast* cast = static_cast<AST_Cast*>(expression);

			if (cast->cast_to->type != AST_TYPE) {
				interpret->report_error(cast->cast_to->token, "Cast to must by type");
				set_type_and_break(NULL);
			}

			resolveExpression(cast->cast_to);
			set_type_and_break(resolveExpression(cast->cast_expression));
		}
		case AST_WHILE: {
			AST_While* ast_while = static_cast<AST_While*>(expression);
			resolveExpression(ast_while->condition);
			resolve(ast_while->block);
			ast_while->block->scope = expression->scope;
			set_type_and_break(NULL);
		}
		case AST_OPERATOR: {
			AST_Operator* op = static_cast<AST_Operator*>(expression);
			set_type_and_break(resolveOperator(op));
		}
		case AST_FOR: {
			AST_For* ast_for = static_cast<AST_For*>(expression);

			if (ast_for->key != NULL) ast_for->header->expressions.push_back(ast_for->key);
			if (ast_for->value != NULL) ast_for->header->expressions.push_back(ast_for->value);

			ast_for->block->scope = expression->scope;

			resolveExpression(ast_for->each);
			resolve(ast_for->header);
			resolve(ast_for->block);
			
			resolved(ast_for);
			set_type_and_break(NULL);
		}
		case AST_RANGE: {
			AST_Range* range = static_cast<AST_Range*>(expression);
			resolveExpression(range->from);
			resolveExpression(range->to);
			
			resolved(range);
			set_type_and_break(NULL);
		}
		case AST_TYPESIZEOF: {
			AST_TypeSizeOf* ast = static_cast<AST_TypeSizeOf*>(expression);

			resolveExpression(ast->of);

			if ((ast->flags & AST_TYPESIZEOF_IS_TYPEOF) == AST_TYPESIZEOF_IS_TYPEOF) {
				set_type_and_break(interpret->type_definition);
			}
			if ((ast->flags & AST_TYPESIZEOF_IS_SIZEOF) == AST_TYPESIZEOF_IS_SIZEOF) {
				set_type_and_break(interpret->type_s64);
			}			
		}
		default:
			assert(false);
			break;
	}

	if (type != NULL) {
		if (((expression->type == AST_TYPE) && expression->is_resolved) || expression->type != AST_TYPE)
			 resolved(expression);
	}

	return type;
}

AST_Operator* TypeResolver::findOperator(int Operator, AST_Type* type1, AST_Type* type2) {
	OperatorKey param = { Operator, type1, type2 };

	auto opfind = operatorTable.find(param, [](OperatorKey param, OperatorKey key, AST_Operator* op) {
		if (param == key) {
			return true;
		}
		return false;
	});

	return opfind;
}

AST_Type* TypeResolver::resolveOperator(AST_Operator* op) {
	auto expr = resolveExpression(op->procedure);	
	auto procedure = (AST_Procedure*)op->procedure;
	auto arg1 = procedure->header->expressions[0];

	if (!arg1->is_resolved) {
		addToResolve(arg1);
		return expr;
	}

	auto arg1_type = find_typeof(arg1);
	OperatorKey key;

	if (op->op->type == TOKEN_EMPTY_INDEX) { // extra handle special operators like one argument etc... 
		//op_name = (String)"[]_" + typeToString(arg1_type);
		key.Operator = TOKEN_EMPTY_INDEX;
		key.Type1 = arg1_type;
	}
	else {
		auto arg2 = procedure->header->expressions[1];
		auto arg2_type = find_typeof(arg2);

		//op_name = (String)token_to_string(op->op->type) + "_" + typeToString(arg1_type) + "_" + typeToString(arg2_type);
		key.Operator = op->op->type;
		key.Type1 = arg1_type;
		key.Type2 = arg2_type;
	}

	if (operatorTable.exist(key)) {
		interpret->report_error(op->token, "This operator was already created");
	}
	else {
		operatorTable.insert(key, op);
	}

	return expr;
}

void TypeResolver::calcaulate_index_of_enum(AST_Enum* _enum){
	for (int index = 0; index < _enum->members->expressions.size(); index++) {
		auto element = _enum->members->expressions[index];

		assert(element->type == AST_DECLARATION);

		AST_Declaration* dec = (AST_Declaration*)element;
		if (dec->value == NULL) {
			dec->value = make_number_literal(_enum->index++);
		}
		else {
			if (is_static(dec->value)) {
				auto value = calculate_size_of_static_expression(dec->value);
				_enum->index = value + 1;
				if (!is_number(dec->value)) {
					dec->value = make_number_literal(value);
				}
			}
			else {
				interpret->report_error(dec->token, "Value in enum must be constant number");
			}
		}
	}
}

AST_Type* TypeResolver::resolveType(AST_Type* type, bool as_declaration, AST_Expression* value) {
	if (type->kind == AST_TYPE_DEFINITION) {
		AST_Type_Definition* def = static_cast<AST_Type_Definition*>(type);
		return def;
	}
	else if (type->kind == AST_TYPE_POINTER) {
		AST_Pointer* pointer = static_cast<AST_Pointer*>(type);
		pointer->point_type = resolveExpression(pointer->point_to);
		return pointer;
	}
	/*else if (type->kind == AST_TYPE_STRUCT) {
		AST_Struct* _struct = static_cast<AST_Struct*>(type);
		resolve(_struct->members);
		if (!ensure_elements_is_resolved(_struct)) {
			addToResolve(_struct);
		}
		else {
			calculate_type_size(_struct);		
		}
		return _struct;
	}*/
	else if (type->kind == AST_TYPE_UNION || type->kind == AST_TYPE_STRUCT) {
		//AST_Union* _union = static_cast<AST_Union*>(type);
		if (!ensure_elements_is_resolved(type)) {
			auto expresions = get_members_of_type(type)->expressions;
			For(expresions) {
				addToResolve(it);
			}
			addToResolve(type);
 		}
		else {
			calculate_type_size(type);
		}
		return type;
	}
	else if (type->kind == AST_TYPE_ENUM) {
		AST_Enum* _enum = static_cast<AST_Enum*>(type);
		resolve(_enum->members);
		if (_enum->enum_type == NULL)
			_enum->enum_type = interpret->type_s32;
		resolveExpression(_enum->enum_type);		
		calcaulate_index_of_enum(_enum);
		return _enum;
	}
	else if(type->kind == AST_TYPE_ARRAY) {
		AST_Array* _array = static_cast<AST_Array*>(type);		

		if (_array->flags & ARRAY_AUTO_SIZE) {
			if (value != NULL) {
				if (value->type != AST_BLOCK) {
					interpret->report_error(_array->token, "When you don't specify size you must assing array to list of values type of array");
					return NULL;
				}

				AST_Block* block = (AST_Block*)value;
				_array->size = make_number_literal((int)block->expressions.size());
				resolveExpression(_array->size);
			}
		}
		else if (_array->flags & ARRAY_DYNAMIC) {

		}
		else {
			resolveExpression(_array->size);
		}
		
		auto op = findOperator(TOKEN_EMPTY_INDEX, find_typeof(_array->point_to), NULL);

		resolveExpression(_array->point_to);

		if (as_declaration && _array->size != NULL) {
			if (!is_static(_array->size)) {
				interpret->report_error(_array->token, "Declaration of array size must be constant");
				return NULL;
			}

			int size = calculate_array_size(_array);
			if (size < 1) {
				interpret->report_error(_array->token, "Declaration of array size must be more then 0");
				return NULL;
			}
		}

		if (op != NULL) {
			assert(op->procedure->type == AST_PROCEDURE);
			auto proc = (AST_Procedure*)op->procedure;
			resolveExpression(proc);
			assert(proc->returnType->type == AST_TYPE);
			return (AST_Type*)proc->returnType;
		}

		resolved(type);
		return type;
	}

	assert(false);
	return NULL;
}

bool TypeResolver::is_pointer(AST_Expression* expression) {
	if (expression->type == AST_UNARYOP) {
		auto unary = static_cast<AST_Unary*>(expression);
		if (unary->operation == UNOP_REF) {
			return true;
		}
	}

	return false;
}

bool TypeResolver::is_number(AST_Expression* expression) {
	if (expression->type == AST_LITERAL) {
		auto literar = static_cast<AST_Literal*>(expression);
		if (literar->value_type == LITERAL_NUMBER || literar->value_type == LITERAL_FLOAT) {
			return true;
		}
	}

	return false;
}

bool TypeResolver::is_type_integer(AST_Type* type) {
	if (type->kind == AST_TYPE_DEFINITION) {
		auto type_def = static_cast<AST_Type_Definition*>(type);
		auto intern = type_def->internal_type;
		if (intern == AST_Type_s8 || intern == AST_Type_s16 || intern == AST_Type_s32 || intern == AST_Type_s64 ||
			intern == AST_Type_u8 || intern == AST_Type_u16 || intern == AST_Type_u32 || intern == AST_Type_u64 || intern == AST_Type_bool)
			return true;
	}

	return false;
}

AST_Type* TypeResolver::resolveStructDereference(AST_Struct* _struct, AST_Expression* expression) {
	if (expression->type == AST_IDENT) { // struct.member
		auto ident = static_cast<AST_Ident*>(expression);
		auto type = find_declaration(ident, _struct->members);
		ident->type_declaration = type;

		resolveExpression(type);
		resolveExpression(ident);
		return type->inferred_type;
	}
	
	if (expression->type == AST_TYPE) { //struct.arr[0]
		auto type = (AST_Type*)expression;
		assert(type->kind == AST_TYPE_ARRAY);
		
		auto arr = (AST_Array*)type;
		arr->scope = _struct->members;

		return resolveArray(arr);
	}

	auto binary = static_cast<AST_Binary*>(expression);
	auto ident = static_cast<AST_Ident*>(binary->left);
	auto type = find_declaration(ident, _struct->members);
	ident->type_declaration = type;

	if (type->assigmet_type->type == AST_TYPE) {
		auto struct_type = static_cast<AST_Type*>(type->assigmet_type);
		//assert(struct_type->kind == AST_TYPE_STRUCT) ????

		return resolveStructDereference(static_cast<AST_Struct*>(struct_type), binary->right);		
	}
}

AST_Type* TypeResolver::resolveArray(AST_Array* arr) { //arr.data[x] data: *s64;
	if (arr->point_to->type == AST_TYPE_ARRAY) { //wtf???
		return resolveArray(static_cast<AST_Array*>(arr->point_to));
	}

	resolveExpression(arr->size);
	arr->point_to->scope = arr->scope;
	return resolveExpression(arr->point_to);
}

AST_Type* TypeResolver::resolveBinary(AST_Binary* binop) {
	AST_Type* type;
	if (binop->left->type == AST_TYPE && static_cast<AST_Type*>(binop->left)->kind == AST_TYPE_ARRAY) {
		auto arr = static_cast<AST_Array*>(binop->left);
		type = resolveArray(arr);
	}
	else {
		type = resolveExpression(binop->left);
	}

	if (binop->operation == BINOP_DOT) { // l.r
		auto ident = static_cast<AST_Ident*>(binop->left);
		auto type = find_typedefinition(ident, binop->scope); //We know left can be only ident
		if (type != NULL && type->type == AST_TYPE) {
			ident->type_declaration = find_declaration(ident, binop->scope);

			if (type->kind == AST_TYPE_STRUCT) {
				auto _struct = static_cast<AST_Struct*>(type);
				binop->right->scope = _struct->members;

				return resolveStructDereference(_struct, binop->right);
			}
			else if (type->kind == AST_TYPE_ENUM) {
				auto _enum = static_cast<AST_Enum*>(type);
				binop->right->scope = _enum->members;
				/*if (_enum->enum_type == NULL)
					return interpret->type_s32;*/
				return find_typeof(_enum->enum_type);
			}
			else if (type->kind == AST_TYPE_ARRAY) {

			}
			else {
				assert(false && "Just now we can dereference only struct and enum");
			}
		}
	}
	else if (binop->operation == BINOP_ASIGN) { // l = r 
		auto type = find_typeof(binop->left);	// auto convert unsigned to signed
		if (type->kind == AST_TYPE_DEFINITION) {
			
		}
	}

	resolveExpression(binop->right);

	bool left_ispointer = is_pointer(binop->left);
	bool right_ispointer = is_pointer(binop->right);

	if (!left_ispointer && !right_ispointer) {
		return type;
	}

	if (left_ispointer && right_ispointer) {
		interpret->report_error(binop->left->token, "Can't sum two pointers");
		return NULL;
	}

	if (!left_ispointer && right_ispointer) {
		auto temp = binop->left;
		binop->left = binop->right;
		binop->right = temp;
	}

	auto unary = static_cast<AST_Unary*>(binop->left);
	auto pointer = static_cast<AST_Pointer*>(resolveExpression(unary));

	auto size = find_typedefinition_from_type(pointer->point_type)->aligment; //wtf aligment why?

	/*AST_Literal* literar = AST_NEW_EMPTY(AST_Literal);
	literar->value_type = LITERAL_NUMBER;
	literar->integer_value = size;*/

	AST_Binary* count_binop = AST_NEW_EMPTY(AST_Binary);
	count_binop->left = binop->right;
	count_binop->operation = BINOP_TIMES;
	count_binop->right = make_number_literal(size);

	binop->right = count_binop;

	return type;
}

AST_Type_Definition* TypeResolver::find_typedefinition_from_type(AST_Type* type) {
	if (type->kind == AST_TYPE_POINTER) {
		auto pointer = static_cast<AST_Pointer*>(type);
		assert(pointer->point_to->type == AST_TYPE);
		return find_typedefinition_from_type((AST_Type*)pointer->point_to);
	}
	if (type->kind == AST_TYPE_ARRAY) {
		auto arr = static_cast<AST_Array*>(type);
		if (arr->point_to->type == AST_TYPE) {
			auto t = static_cast<AST_Type*>(arr->point_to);
			if (t->kind == AST_TYPE_ARRAY) {
				return find_typedefinition_from_type(t);
			}
			if (t->kind == AST_TYPE_DEFINITION) {
				return static_cast<AST_Type_Definition*>(t);
			}
			assert(false);
		}
		if (arr->point_to->type == AST_IDENT) {
			auto ident = static_cast<AST_Ident*>(arr->point_to);
			auto tdef = find_typedefinition(ident, ident->scope);
			return find_typedefinition_from_type(tdef);
		}
	}

	auto typdef = static_cast<AST_Type_Definition*>(type);
	return typdef;
}

void TypeResolver::resolveDirective(AST_Directive* directive) {
	switch (directive->directive_type) {
	case AST_DIRECTIVE_TYPE_IMPORT: {
			if (directive->value0->type == AST_IDENT) {
				AST_Ident* ident = static_cast<AST_Ident*>(directive->value0);
				if (directive->scope != NULL) ident->scope = directive->scope;

				resolveIdent(ident);
			}
		}
		break;
	};
}

AST_Type* TypeResolver::resolveLiteral(AST_Literal* literal) {
	if (literal->value_type == LITERAL_STRING)
		return interpret->type_string;
	return interpret->type_s64;
}

AST_Type* TypeResolver::resolveIdent(AST_Ident* ident) {
	auto type = find_typedefinition(ident, ident->scope);	

	if (type == NULL) {
		if (phase > 50)
			interpret->report_error(ident->name, "Unkown ident '%s'", ident->name->value);
		else 
			addToResolve(ident); //We don't have this type jet maybe it's defined after this expression
	}
	
	return type;
}

AST_Declaration* TypeResolver::make_declaration(String name, AST_Expression* value) {
	auto decl = AST_NEW_EMPTY(AST_Declaration);
	decl->ident = AST_NEW_EMPTY(AST_Ident);
	decl->ident->name = new Token();
	decl->ident->name->value = name;
	decl->value = value;
	return decl;
}

AST_Declaration* TypeResolver::make_declaration(AST_Ident* ident, AST_Expression* value) {
	auto decl = AST_NEW_EMPTY(AST_Declaration);
	decl->ident = ident;
	decl->value = value;
	return decl;
}

AST_Type* TypeResolver::resolveUnary(AST_Unary* unary) {
	if (unary->operation == UNOP_DEF) { //*
		auto type = resolveExpression(unary->left);
		/*
		AST_Pointer* pointer = AST_NEW_EMPTY(AST_Pointer);
		pointer->scope = unary->scope;
		pointer->point_to = unary->left;
		pointer->point_type = type;
		*/
		return interpret->type_pointer;
	}
	else if (unary->operation == UNOP_REF) { //&
		auto type = resolveExpression(unary->left);
		//AST_Declaration* declaration = find_expression_declaration(unary->left);
		//assert(declaration != NULL);

		AST_Pointer* address = AST_NEW_EMPTY(AST_Pointer);
		address->scope = unary->scope;
		address->point_to = unary->left;
		address->point_type = type;

		unary->substitution = address;

		return address;
	}
	else if (unary->operation == UNOP_CALL) { // ()
		resolveExpression(unary->left);
		resolveExpression(unary->arguments);

		auto procedure = find_procedure(unary->left, unary->arguments);
		if (procedure == NULL) {			
			resolved(unary); //@todo: maybe not?
			return NULL;
		}

		if (!procedure->is_resolved) {
			addToResolve(unary);
			return NULL;
		}

		bool is_generic = (procedure->flags & AST_PROCEDURE_FLAG_GENERIC) == AST_PROCEDURE_FLAG_GENERIC;
		bool is_intristic = (procedure->flags & AST_PROCEDURE_FLAG_INTRINSIC) == AST_PROCEDURE_FLAG_INTRINSIC;

		if (is_generic && !is_intristic) { //this all block should do some generic resolver
			//generic - we need to create copy of this procedure if the copy not alerady exist and fill the generic parameters
			auto copy = copier->copy_procedure(procedure);
			copy->flags = copy->flags & ~AST_PROCEDURE_FLAG_GENERIC;

			vector<AST_Expression*> append;
			For(copy->header->expressions) {
				auto decl = (AST_Declaration*)it;
				auto arg = unary->arguments->expressions[it_index];

				assert(decl != NULL && arg != NULL);				
				
				if ((decl->flags & TYPE_DEFINITION_GENERIC) == TYPE_DEFINITION_GENERIC) {
					decl->flags = decl->flags & ~TYPE_DEFINITION_GENERIC;

					//we need to set type to type as argument in future we will support $T/type but this will not be bother of it xD the find_procedure must handle it
					auto type = find_typeof(arg);					
					//we need to add the generic type to the head
					auto decl_type = AST_NEW_EMPTY(AST_Declaration);
					auto ident = AST_NEW_EMPTY(AST_Ident);
					ident->name = new Token();
					ident->name->value = ((AST_Ident*)(decl->assigmet_type))->name->value;
					decl_type->ident = ident;
					decl_type->assigmet_type = type;
					decl_type->flags |= DECLARATION_IS_GENERIC_TYPE_DEFINTION;
					append.push_back(decl_type);

					decl->assigmet_type = type;
				}
			}
			ForPush(append, copy->header->expressions);

			//now we need to replace procedure header that has generic arguments with it types provided
			assert(unary->left->type == AST_IDENT);
			auto ident_name = (AST_Ident*)unary->left;
			auto new_function_from_generic_declaration = make_declaration(ident_name->name->value, copy);
			new_function_from_generic_declaration->scope = unary->scope;

			addToResolve(new_function_from_generic_declaration);
			procedure->scope->expressions.push_back(new_function_from_generic_declaration);
			unary->left->expression = copy;

			return NULL;
		}

		if (procedure != NULL) {
			unary->left->expression = procedure;
		}
		else {
			/*if (unary->left->type == AST_IDENT) {
				interpret->report_error(unary->left->token, "Procedure '%s' not found", ((AST_Ident*)unary->left)->name->value);
			}
			else {
				interpret->report_error(unary->left->token, "Internal compiler error!!!");
			}*/
			return NULL;
		}

		if (procedure->returnType != NULL) {
			if (procedure->returnType->type == AST_TYPE) {
				auto type = static_cast<AST_Type*>(procedure->returnType);
				if (type->kind == AST_TYPE_DEFINITION) {
					auto tdef = static_cast<AST_Type_Definition*>(type);
					return tdef;
				}
			}
			else if (procedure->returnType->type == AST_PARAMLIST) {
				return interpret->type_pointer; //@todo: for now???
			}
			else {
				return find_typeof(procedure->returnType); //ident like generic T etc...
			}
		}
		

		//return interpret->type_void;
		return interpret->type_s64; //WTF? on top we check if return type is type definition else we drop here o.o, this should definitely be void!
	}
	else if (unary->operation == UNOP_INCREMENT || unary->operation == UNOP_DECREMENT) {
		auto type = resolveExpression(unary->left);
		return type;
	}

	assert(false && "this type of unary not handled");
}

AST_Type* TypeResolver::resolveDeclaration(AST_Declaration* declaration) {
	AST_Type* valueType = NULL;
	AST_Type* type = NULL;	

	if (declaration->param_list != NULL) {
		resolveExpression(declaration->param_list);
	}

	if (declaration->value != NULL) {
		valueType = resolveExpression(declaration->value);
		if (valueType == NULL && declaration->value->is_resolved) {			
			resolved(declaration);
			return NULL;
		}
		if (valueType != NULL && (valueType->kind == AST_TYPE_STRUCT || valueType->kind == AST_TYPE_UNION)) {
			valueType->expression = declaration->ident;
		}
	}

	if (declaration->assigmet_type != NULL && declaration->assigmet_type->type == AST_TYPE) {		
		type = resolveType(static_cast<AST_Type*>(declaration->assigmet_type), true, declaration->value);
		declaration->inferred_type = type;
	}
	else if (declaration->assigmet_type != NULL) {
		AST_Ident* assing_type = NULL;
		if (declaration->assigmet_type->type == AST_IDENT) {
			assing_type = static_cast<AST_Ident*>(declaration->assigmet_type);
		}

		assert(declaration->assigmet_type != NULL);
		assert(assing_type != NULL);

		if (declaration->flags & AST_DECLARATION_FLAG_CONSTANT) {
			if (declaration->assigmet_type->type == AST_IDENT) {
				assing_type->flags |= AST_IDENT_FLAG_CONSTANT;
			}
		}

		auto type_def = find_typedefinition(assing_type, declaration->scope);
		if (type_def == NULL) {
			if (!(declaration->flags & TYPE_DEFINITION_GENERIC)) {
				addToResolve(declaration);
			}
			return NULL;
		}

		declaration->assigmet_type = type_def;

		if (type_def->type == AST_PROCEDURE) {
			AST_Procedure* proc = (AST_Procedure*)type_def;
			if (proc->flags & AST_PROCEDURE_FLAG_C_CALL) {
				declaration->inferred_type = interpret->type_c_call;
			}
		}
		
		
		if(declaration->inferred_type == NULL) {
			declaration->inferred_type = type_def;
		}
		type = type_def;
	}
	else if (declaration->value != NULL) {
		declaration->assigmet_type = valueType;

		if (declaration->value->type == AST_IDENT) {
			auto ident = static_cast<AST_Ident*>(declaration->value);
			if (ident->type_declaration->type == AST_DECLARATION) {
				type = find_typedefinition(ident, declaration->scope);
				declaration->assigmet_type = type;//value = type????
			}
		}		

		if (declaration->value->type == AST_TYPE) {
			type = static_cast<AST_Type*>(declaration->value);
		}
		
		if ((declaration->is_resolved || type != NULL) && declaration->assigmet_type != NULL && declaration->assigmet_type->type == AST_TYPE) {
			type = static_cast<AST_Type*>(declaration->assigmet_type);
			declaration->inferred_type = type;
		}
	}

	if ((declaration->flags & AST_DECLARATION_FLAG_CONSTANT) == AST_DECLARATION_FLAG_CONSTANT && 
		type != NULL && 
		type->kind != AST_TYPE_STRUCT && 
		type->kind != AST_TYPE_ENUM &&
		type->kind != AST_TYPE_UNION) {
		if (declaration->value != NULL && declaration->value->type != AST_PROCEDURE) {
			if (!is_static(declaration->value)) {
				interpret->report_error(declaration->value->token, "You must pass only constatnt values to constant declaration");
			}

			if (declaration->value->type != AST_LITERAL) {				
				if (is_type_integer(type)) {
					auto old = declaration->value;
					declaration->value = make_number_literal(calculate_size_of_static_expression(declaration->value));
					copier->copy_token(old, declaration->value);
				}
			}
		}
	}

	if (type == NULL) addToResolve(declaration); //@todo: check this!!

	return type;
}

AST_Declaration* TypeResolver::find_expression_declaration(AST_Expression* expression) {
	if (expression->type == AST_IDENT) {
		auto ident = static_cast<AST_Ident*>(expression);
		auto decl = find_declaration(ident, ident->scope);
		if (decl->type == AST_DECLARATION && decl->value != NULL && decl->value->type != AST_PROCEDURE) {
			return find_expression_declaration(decl->value);
		}

		return decl;
	}
	else if (expression->type == AST_UNARYOP) {
		auto unary = static_cast<AST_Unary*>(expression);
		return find_expression_declaration(unary->left);
	}
	else if (expression->type == AST_BINARY) {
		auto binary = static_cast<AST_Binary*>(expression);
		return find_expression_declaration(binary->left);
	}

	//assert(false);
	return NULL;
}

AST_Procedure* TypeResolver::find_procedure(AST_Expression* expression, AST_Block* arguments) {
	assert(expression->type == AST_IDENT);

	AST_Ident* ident = (AST_Ident*)expression;

	auto decls = find_declarations(ident, ident->scope);
	vector<AST_Procedure*> found;
	vector<AST_Procedure*> foundAll;
	vector<AST_Procedure*> foundNotGeneric;

	{
		For(decls) {
			assert(it != NULL);
			if (it->param_list != NULL) continue;

			assert(it->value->type == AST_PROCEDURE);
			auto procedure = (AST_Procedure*)it->value;

			int generic_defitnion_count;
			if (check_procedure_arguments(procedure->header, arguments, &generic_defitnion_count)) {
				if(generic_defitnion_count > 0) 
					found.push_back(procedure);
				else {
					foundNotGeneric.push_back(procedure);
				}
			}
			else {
				foundAll.push_back(procedure);
			}
		}
	}

	if (foundNotGeneric.size() == 1) {
		auto proc = foundNotGeneric.front();
		return proc;
	}

	if (found.size() == 1) {
		auto proc = found.front();
		return proc;
	}
	
	if (found.size() > 0) {
		interpret->report_error(expression->token, "Found this procedures but we can't match your provided arguments: ");
		{	
			For(found) {
				print_procedure(it);
				print_arguments_compare(arguments, it);
				interpret->report("");
			}
		}
		{
			For(foundNotGeneric) {
				print_procedure(it);
				print_arguments_compare(arguments, it);
				interpret->report("");
			}
		}
	}

	if (foundAll.size() > 0) {
		interpret->report_error(expression->token, "Found this procedures but we can't match your provided arguments: ");
		{
			For(foundAll) {
				print_procedure(it);
				print_arguments_compare(arguments, it);
				interpret->report("");
			}
		}
	}	

	return NULL;
}

void TypeResolver::print_arguments_compare(AST_Block* call_arguments, AST_Procedure* procedure) {
	String output = "... Types given: (";
	int arguments_wanted = procedure->header->expressions.size();
	int arguments_given = call_arguments->expressions.size();

	{
		For(call_arguments->expressions) {
			if (it_index != 0) output += ", ";
			output += expressionTypeToString(it);
		}
	}
	output += "), Types wanted: (";
	{
		For(procedure->header->expressions) {			
			if (it->type == AST_TYPE) {
				auto type = (AST_Type*)it;
				if (type->kind == AST_TYPE_GENERIC) {
					arguments_wanted--;
					continue;
				}
			}
			if (it_index != 0) output += ", ";
			output += expressionTypeToString(it);
		}
	}
	output += ")";
	interpret->report(output.data);

	if (arguments_wanted != arguments_given) {
		interpret->report("... Wanted %d arguments, but given %d arguments.", arguments_wanted, arguments_given);
	}	
}

bool TypeResolver::check_procedure_arguments(AST_Block* header, AST_Block* arguments, int* generic_definition) {
	int need = 0;
	*generic_definition = 0;

	int argument_size = arguments->expressions.size();
	if (argument_size > header->expressions.size()) return false; //we need check if there is varguments

	For(header->expressions) {
		assert(it != NULL);
		if (it->type == AST_TYPE) {
			if (it_index < argument_size) return false; //we are at end of the arguments but we have more passed and what left is just generic
			continue; //skip this it's generic
		}

		assert(it->type == AST_DECLARATION);
		need++;		
		
		auto declaration = (AST_Declaration*)it;

		if (it_index >= arguments->expressions.size()) {
			if ((declaration->flags & DECLARATION_IS_GENERIC_TYPE_DEFINTION) == DECLARATION_IS_GENERIC_TYPE_DEFINTION) return true; //it's just define the type of generic argument
			return false; //we need check if the argument is optional
		}

		auto arg = arguments->expressions[it_index];

		auto type_need = find_typeof(declaration->assigmet_type);
		auto type_have = find_typeof(arg);
			 
		if ((declaration->flags & TYPE_DEFINITION_GENERIC) == TYPE_DEFINITION_GENERIC) {
			*generic_definition = *generic_definition + 1;
			continue;
		}

		if (type_have == NULL) return false; //still not type resolved

		if (!compare_type(type_need, type_have)) {
			return false;
		}
	}

	return true;
}

//we can compare basic types
//we can't compare struct, enum, etc...
bool TypeResolver::compare_type(AST_Expression* left, AST_Expression* right) {
	if (left->type == AST_TYPE) {
		if(right->type != AST_TYPE) return false;

		auto type_left = (AST_Type*)left;
		auto type_right = (AST_Type*)right;

		if (type_left->kind != type_right->kind) return false;
		if (type_left->kind == AST_TYPE_DEFINITION) {
			auto type_def_left = (AST_Type_Definition*)type_left;
			auto type_def_right = (AST_Type_Definition*)type_right;

			if (type_def_left->internal_type == type_def_right->internal_type) return true;
		}
		if (type_left->kind == AST_TYPE_ARRAY) {
			auto arr_left = (AST_Array*)type_left;
			auto arr_right = (AST_Array*)type_right;

			return compare_type(arr_left->point_to, arr_right->point_to);
		}
	}

	return false;
}

void TypeResolver::print_procedure(AST_Procedure* procedure) {
	String output = "";

	if (procedure->name == NULL)
		output += "UnnamedProcedure_" + procedure->serial;
	else
		output += procedure->name->value;

	output += " :: (";
	For(procedure->header->expressions) {
		if (it->type == AST_TYPE) continue;
		assert(it != NULL && it->type == AST_DECLARATION);

		if (it_index != 0) output += ", ";

		auto declaration = (AST_Declaration*)it;	

		output += declaration->ident->name->value;
		output += ": ";
		output += expressionTypeToString(declaration);
	}
	output += ")";

	if (procedure->returnType != NULL) {
		output += " -> ";
		output += expressionTypeToString(procedure->returnType);
	}

	interpret->report_info(procedure->token, output.data);
}

vector<AST_Declaration*> TypeResolver::find_declarations(AST_Ident* ident, AST_Block* scope) {
	vector<AST_Declaration*> declarations;

	for (int i = 0; i < scope->expressions.size(); i++) {
		auto expression = scope->expressions[i];
		if (expression->type == AST_DECLARATION) {
			auto declaration = static_cast<AST_Declaration*>(expression);
			if (declaration->param_list != NULL) {
				For(declaration->param_list->expressions) {
					if (it->type != AST_DECLARATION) {
						//continue; //we don't finish type resolve of this param list
						declarations.push_back(declaration); //we add the default declaration wher the a,b = x; contains for now for type definition!
						continue; 
					}

					auto param_declaration = (AST_Declaration*)it;
					if (param_declaration->ident->name->value == ident->name->value) {
						declarations.push_back(param_declaration);
					}
				}
			} 
			else if (declaration->ident->name->value == ident->name->value) {
				if (declaration->value != NULL && declaration->value->type == AST_IDENT) {
					auto new_ident = static_cast<AST_Ident*>(declaration->value);

					if ((scope->flags & AST_BLOCK_FLAG_MAIN_BLOCK) != AST_BLOCK_FLAG_MAIN_BLOCK) { //if we go again inside main it's bad
						auto declarations_found = find_declarations(new_ident, new_ident->scope);
						ForPush(declarations_found, declarations);
					}
				}
				declarations.push_back(declaration);
			}
		}
	}

	if (scope->belongs_to == AST_BLOCK_BELONGS_TO_PROCEDURE) {
		auto declarations_found = find_declarations(ident, scope->belongs_to_procedure->header);
		ForPush(declarations_found, declarations);

		if(scope->belongs_to_procedure->header->scope->serial == scope->scope->serial) //if the scopes are same we can just return
			return declarations;
	}

	if (scope->belongs_to == AST_BLOCK_BELONGS_TO_FOR) {
		auto declarations_found = find_declarations(ident, scope->belongs_to_for->header);
		ForPush(declarations_found, declarations);

		if (scope->belongs_to_for->header->scope->serial == scope->scope->serial) //if the scopes are same we can just return
			return declarations;
	}

	if (scope->scope != NULL) {
		auto declarations_found = find_declarations(ident, scope->scope);
		ForPush(declarations_found, declarations);
	}

	return declarations;
}

AST_Declaration* TypeResolver::find_declaration(AST_Ident* ident, AST_Block* scope) {
	auto declarations = find_declarations(ident, scope);
	if (declarations.size() == 0) return NULL;
	auto declaration = declarations.front();
	return declaration;
}

AST_Type* TypeResolver::find_typeof(AST_Expression* expression, bool deep) {
	while (expression->substitution != NULL) {
		expression = expression->substitution;
	}

	if (expression->type == AST_TYPE) {
		auto type = static_cast<AST_Type*>(expression);
		if (type->kind == AST_TYPE_POINTER) {
			auto point = static_cast<AST_Pointer*>(expression);
			if (!deep) return point;

			return find_typeof(point->point_to);
		}
		else if (type->kind == AST_TYPE_ARRAY) {
			auto arr = static_cast<AST_Array*>(expression);
			//if (!deep) return arr;
			/*
			auto type = find_typeof(arr->point_to);
			if (type->kind == AST_TYPE_POINTER) {
				return find_typeof(type);
			}*/ //@todo: hmm idk this
			return arr;
		}

		return type;
	}
	else if (expression->type == AST_CAST) {
		auto cast = static_cast<AST_Cast*>(expression);
		return find_typeof(cast->cast_to);
	}
	else if (expression->type == AST_IDENT) {
		auto ident = static_cast<AST_Ident*>(expression);
		return find_typedefinition(ident, ident->scope);
	}
	else if (expression->type == AST_BINARY) {
		auto bin = static_cast<AST_Binary*>(expression);
		return find_typeof(bin->left);
	}
	else if (expression->type == AST_UNARYOP) {
		auto unar = static_cast<AST_Unary*>(expression);
		return find_typeof(unar->left);
	}
	else if (expression->type == AST_DECLARATION) {
		auto declaration = static_cast<AST_Declaration*>(expression);
		/*if ((declaration->flags & TYPE_DEFINITION_GENERIC) == TYPE_DEFINITION_GENERIC) { //call expressionTypeToString() not typeToString()
			auto g = AST_NEW_EMPTY(AST_Generic);
			g->type_declaration = declaration;
			return g;
		}*/
		return find_typeof(declaration->assigmet_type);
	}
	else if (expression->type == AST_LITERAL) {
		auto literal = static_cast<AST_Literal*>(expression);
		if (literal->value_type == LITERAL_NUMBER) return interpret->type_s64;
		if (literal->value_type == LITERAL_FLOAT) return interpret->type_float;
		if (literal->value_type == LITERAL_STRING) return interpret->type_string;
	}
	else if (expression->type == AST_TYPESIZEOF) {
		return NULL;//@todo maybe we can try find the type but then why we have codemanager???
	}
	/*else if (expression->type == AST_TYPE_ARRAY) {
		auto arr = static_cast<AST_Array*>(expression);
		return find_typeof(arr->point_to);
	}*/
	
	assert(false && "Unkown type for typeof");
}

String TypeResolver::expressionTypeToString(AST_Expression* type) {
	if (type->type == AST_TYPE) {
		AST_Type* _type = static_cast<AST_Type*>(type);
		return typeToString(_type);
	}
	else if (type->type == AST_DECLARATION) {
		AST_Declaration* decl = static_cast<AST_Declaration*>(type);
		if ((decl->flags & TYPE_DEFINITION_GENERIC) == TYPE_DEFINITION_GENERIC) {
			return (String)"$" + expressionTypeToString(decl->assigmet_type);
		}
		return expressionTypeToString(decl->assigmet_type);
	}
	else if (type->type == AST_IDENT) {
		AST_Ident* ident = static_cast<AST_Ident*>(type);
		if(ident->type_declaration != NULL && ident->type_declaration->assigmet_type != NULL)
			return expressionTypeToString(ident->type_declaration->assigmet_type);

		return ident->name->value;
	}
	else if (type->type == AST_PARAMLIST) {
		AST_ParamList* params = static_cast<AST_ParamList*>(type);
		String ret = "";
		For(params->expressions) {
			if (it_index != 0) ret += ", ";
			ret += expressionTypeToString(it);
		}
		return ret;
	}
	
	auto _found_type = find_typeof(type);
	return typeToString(_found_type);
	//assert(false && "Only type can be translated");
}

String TypeResolver::typeToString(AST_Type* type) {
	if (type->kind == AST_TYPE_DEFINITION) {
		auto tdef = static_cast<AST_Type_Definition*>(type);
		if (tdef->internal_type == AST_Type_s8)  return "s8";
		if (tdef->internal_type == AST_Type_s16) return "s16";
		if (tdef->internal_type == AST_Type_s32) return "s32";
		if (tdef->internal_type == AST_Type_s64) return "s64";
		if (tdef->internal_type == AST_Type_u8)	 return "u8";
		if (tdef->internal_type == AST_Type_u16) return "u16";
		if (tdef->internal_type == AST_Type_u32) return "u32";
		if (tdef->internal_type == AST_Type_u64) return "u64";
		if (tdef->internal_type == AST_Type_char) return "char";
		if (tdef->internal_type == AST_Type_float) return "float";
		if (tdef->internal_type == AST_Type_long) return "long";
		if (tdef->internal_type == AST_Type_bit) return "bit";
		if (tdef->internal_type == AST_Type_string) return "string";
		if (tdef->internal_type == AST_Type_c_call) return "c_call";
		if (tdef->internal_type == AST_Type_bool) return "bool";
		if (tdef->internal_type == AST_Type_int) return "int";
		if (tdef->internal_type == AST_Type_pointer) return "void*";
	}
	else if (type->kind == AST_TYPE_STRUCT) {
		auto strct = static_cast<AST_Struct*>(type);
		if (strct->expression != NULL && strct->expression->type == AST_IDENT) {
			auto ident = (AST_Ident*)strct->expression;
			return (String)"struct<" + ident->name->value.data + ">";
		}
		return "struct<>";
	}
	else if (type->kind == AST_TYPE_UNION) {
		auto _union = static_cast<AST_Union*>(type);
		if (_union->expression != NULL && _union->expression->type == AST_IDENT) {
			auto ident = (AST_Ident*)_union->expression;
			return (String)"union<" + ident->name->value.data + ">";
		}
		return "union<>";
	}
	else if (type->kind == AST_TYPE_POINTER) {
		auto point = static_cast<AST_Pointer*>(type);
		return (String)("*")+expressionTypeToString(point->point_to);
	}
	else if (type->kind == AST_TYPE_ARRAY) {
		AST_Array* arr = static_cast<AST_Array*>(type);
		return expressionTypeToString(arr->point_to) + "[]";
	}
	else if (type->kind == AST_TYPE_GENERIC) {
		AST_Generic* ast_generic = static_cast<AST_Generic*>(type);
		return "$";
	}
	else if (type->kind == AST_TYPE_ENUM) {
		AST_Enum* ast_enum = static_cast<AST_Enum*>(type);
		return "enum";
	}

	assert(false && "This type is not transatable");
}

AST_Type* TypeResolver::find_typedefinition(AST_Ident* ident, AST_Block* scope) {
	auto name = ident->name->value;

	if (scope->belongs_to == AST_BLOCK_BELONGS_TO_PROCEDURE) {
		auto in_header = find_typedefinition(ident, scope->belongs_to_procedure->header);
		if (in_header != NULL)
			return in_header;
	}

	if (scope->belongs_to == AST_BLOCK_BELONGS_TO_FOR) {
		auto in_header = find_typedefinition(ident, scope->belongs_to_for->header);
		if (in_header != NULL)
			return in_header;
	}

	for (auto i = 0; i < scope->expressions.size(); i++) {
		AST_Expression* it = scope->expressions[i];

		if (it->type == AST_TYPE) {
			AST_Type* type = static_cast<AST_Type*>(it);
			if (type->kind == AST_TYPE_GENERIC) {
				AST_Generic* ast_generic = static_cast<AST_Generic*>(type);
				if(ast_generic->type_declaration->ident->name->value == name)
					return type;
			}
		}
		else if (it->type == AST_DECLARATION) {
			AST_Declaration* declaration = static_cast<AST_Declaration*>(it);
			if (declaration->param_list != NULL) {
				For(declaration->param_list->expressions) {
					if (it->type != AST_DECLARATION) {
						continue; //we don't finish type resolve of this param list
					}

					auto param_declaration = (AST_Declaration*)it;
					if (param_declaration->ident->name->value == ident->name->value) {
						if (param_declaration->assigmet_type != NULL) {
							ident->type_declaration = param_declaration;

							if (param_declaration->assigmet_type->type == AST_TYPE) {
								return resolveType(static_cast<AST_Type*>(param_declaration->assigmet_type));
							}
							else if (param_declaration->assigmet_type->type == AST_IDENT) {
								return resolveIdent(static_cast<AST_Ident*>(param_declaration->assigmet_type));
							}
						}
						return NULL;
					}
				}
			}
			else if (declaration->ident->name->value == name) {
				ident->type_declaration = declaration;

				if (declaration->flags & AST_DECLARATION_FLAG_CONSTANT) {
					ident->flags |= AST_IDENT_FLAG_CONSTANT;
				}
				if (declaration->flags & TYPE_DEFINITION_GENERIC) {
					ident->flags |= AST_IDENT_FLAG_GENERIC;

					if (declaration->assigmet_type != NULL && declaration->assigmet_type->type == AST_IDENT) {
						AST_Generic* ast_generic = AST_NEW_EMPTY(AST_Generic);
						ast_generic->type_declaration = declaration;
						ast_generic->found_in_scope = scope;
						return ast_generic;
					}
				}

				if (declaration->assigmet_type != NULL && declaration->assigmet_type->type == AST_TYPE) {
					AST_Type* type = static_cast<AST_Type*>(declaration->assigmet_type);
					return resolveType(type);
				}
				else if (declaration->assigmet_type != NULL && declaration->assigmet_type->type == AST_IDENT) {
					return resolveIdent(static_cast<AST_Ident*>(declaration->assigmet_type));
				}
				else if (declaration->value != NULL && declaration->value->type == AST_TYPE) {
					auto type = static_cast<AST_Type*>(declaration->value);
					if (type->kind == AST_TYPE_DEFINITION)
						return static_cast<AST_Type_Definition*>(declaration->value);
					return (AST_Type*)declaration->value; //todo: maybe not??? ArrayType :: enum u64
				}
				else {		
					//assert(false && "only handling ast type definition");					
					return NULL;
				}
			}
		}
		else if (it->type == AST_IDENT) {
			AST_Ident* ident = static_cast<AST_Ident*>(it);
			if (ident->name->value == name) {
				if (ident->serial != it->serial) { //we find the same ident
					auto definition = find_typedefinition(ident, ident->scope);
					if (definition != NULL) {
						return definition;
					}
				}
			}
		}
	}

	if (scope->flags & AST_BLOCK_FLAG_MAIN_BLOCK)
		return NULL;
	
	if (scope->scope != NULL)
		return find_typedefinition(ident, scope->scope);

	return NULL;
}