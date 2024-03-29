﻿#include "Headers.h"
#include "Pyr3.h"
#include "Utils.h"
#include "Interpret.h"
#include "Lexer.h"
#include "Parser.h"
#include "TypeResolver.h"
#include "CodeManager.h"
#include "DirectiveResolver.h"
#include "BytecodeBuilder.h"
#include "BytecodeDebuger.h"
#include "BytecodeRunner.h"
#include <clocale>

using namespace std::chrono;
using namespace std;

AST_Block* main_block = NULL;

void printType(AST_Expression* it);

int main(int argc, char* argv[])
{
    std::locale::global(std::locale("cz_CS.UTF-8"));
    //std::setlocale(LC_ALL, ".UTF-8");
    //std::cout << "Arguments: " << argc << "!\n";

    auto start = high_resolution_clock::now();
    
    char* file_name;
    bool presection = false;
    
    for (int i = 1; i < argc; i++) {
        if (!presection) {
            file_name = argv[i];
        }
    }

    if (&file_name != nullptr) {
        printf("File to parse: %s\n", file_name);

        auto path = Utils::to_string(Utils::ExePath());
        auto path_to_file = Utils::path_combine(path, file_name);

        //printf("%s\n", path_to_file.c_str());

        if (!Utils::file_exists(path_to_file)) {
            printf("File %s not exists!", path_to_file.c_str());
        }
        else {
            //printf("File %s exists.", path_to_file.c_str());

            Interpret* interpreter = new Interpret();
            
            Lexer* lexer = new Lexer(interpreter, path_to_file.c_str(), "\0"); //test :: () -> bool #foreign; x = \"test\"
            lexer->lex();
            auto tokens = lexer->getTokens();

            Parser* parser = NULL;
            TypeResolver* type_resolver = NULL;
            CodeManager* code_manager = NULL;

            if (!interpreter->isError()) {              
                type_resolver = new TypeResolver(interpreter);
                code_manager = new CodeManager(interpreter, type_resolver);

                parser = new Parser(interpreter, lexer, type_resolver);
                main_block = parser->parse();
                
                code_manager->manage(main_block);
                type_resolver->resolve_main(main_block);

                while (!interpreter->isError() && type_resolver->has_changes()) {
                    code_manager->manage(main_block);
                    type_resolver->resolveOther();
                }

            }

            if (!interpreter->isError()) {                              
                DirectiveResolver* directive_resolver = new DirectiveResolver(interpreter);
                directive_resolver->resolve(main_block);

                BytecodeBuilder* builder = new BytecodeBuilder(interpreter, type_resolver);
                builder->build(main_block);

#if _DEBUG
                BytecodeDebuger* debuger = new BytecodeDebuger(interpreter, type_resolver, builder->get_instructions(), builder->get_types());
                debuger->debug();
#endif

                BytecodeRunner* runner = new BytecodeRunner(interpreter, type_resolver, builder->get_instructions(), builder->get_types(), builder->get_output_register_size(), builder->get_output_stack_size());
                //runner->run(0);
                runner->set_current_address(runner->get_address_of_procedure("main", main_block));
                runner->loop();
            }
            else {
                printf("\nCan't parse because error occured");
            }            

            printf("\n\n--------------------\n");
            printf("\nTotal tokens: %d", tokens.size());
        }
    }

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);

    auto xo = (std::chrono::duration_cast<std::chrono::seconds>(duration).count());
    auto x2 = (float)(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count()) / 1000;
    auto el = xo + x2;
    
#ifdef _DEBUG
    printf("\n!!! DEBUG MODE");    
#endif // _debug

    printf("\nTotal time: %4.3f ms", el);

    /* Windows handler */
    MSG  msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        DispatchMessage(&msg);
    }
    /* Windows handler */

    //just for now
    /** /
    int x;
    cin >> x;
    /**/
}