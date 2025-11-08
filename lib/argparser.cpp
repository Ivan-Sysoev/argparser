#include "argparser.h"
#include <cstring>
#include <cstdlib>
#include <cstdio>

using namespace nargparse;

// =============================
//  Вспомогательные функции
// =============================
void ExpandArgs(ArgumentParser& parser) {
    if (parser.arg_count >= parser.arg_capacity) {
        int new_cap = (parser.arg_capacity == 0) ? 8 : parser.arg_capacity * 2;        
        ArgumentParser::ArgItem* new_args = new ArgumentParser::ArgItem[new_cap];

        if (parser.args) {
            std::memcpy(new_args, parser.args, parser.arg_capacity * sizeof(ArgumentParser::ArgItem));
            delete[] parser.args;
        }

        parser.args = new_args;
        parser.arg_capacity = new_cap;
    }
}

void ExpandRepeated(ArgumentParser::ArgItem& arg) {
    if (arg.repeated_count >= arg.repeated_capacity) {
        int new_cap = (arg.repeated_capacity == 0) ? 4 : arg.repeated_capacity * 2;
        void** new_args = new void*[new_cap];

        if (arg.repeated_values) {
            std::memcpy(new_args, arg.repeated_values, arg.repeated_count * sizeof(void*));
            delete[] arg.repeated_values;
        }

        arg.repeated_values = new_args;        
        arg.repeated_capacity = new_cap;
    }
}

ArgumentParser::ArgItem* FindArg(ArgumentParser& parser, const char* name) {
    for (int i = 0; i < parser.arg_count; i++) {
        ArgumentParser::ArgItem& a = parser.args[i];
        if ((a.long_name && strcmp(a.long_name, name) == 0) ||
            (a.short_name && strcmp(a.short_name, name) == 0))
            return &a;
    }
    return nullptr;
}

// =============================
//  Создание / Освобождение
// =============================
ArgumentParser CreateParser(const char* program_name, size_t max_arg_len) {
    ArgumentParser parser{};
    parser.program_name = program_name;
    parser.max_arg_len = max_arg_len;
    
    parser.args = nullptr;
    parser.arg_count = 0;
    parser.arg_capacity = 0;

    parser.help_added = false;
    parser.help_requested = false;
    return parser;
}

void FreeParser(ArgumentParser& parser) {
    if (parser.args) {
        for (int i = 0; i < parser.arg_count; i++) {
            ArgumentParser::ArgItem& arg = parser.args[i];
            if (arg.repeated_values) {
                for (int j = 0; j < arg.repeated_count; j++) {
                    delete arg.repeated_values[j];
                }
                delete[] arg.repeated_values;
            }
        }
        delete[] parser.args;
    }
    
    parser.args = nullptr;
    parser.arg_count = 0;
    parser.arg_capacity = 0;
}

// =============================
//  Добавление аргументов
// =============================
bool AddFlag(ArgumentParser& parser,
             const char* short_name,
             const char* long_name,
             bool* storage,
             const char* description,
             bool default_value=false) {
    *storage = default_value;
    
    ExpandArgs(parser);
    ArgumentParser::ArgItem& flag = parser.args[parser.arg_count++];
    std::memset(&flag, 0, sizeof(flag));

    flag.long_name = long_name;
    flag.short_name = short_name;
    flag.description = description;
    flag.storage = storage;
    flag.nargs = ArgNargs::kOptional;
    flag.is_required = false;
    flag.type = ArgType::kFlag;
    flag.repeated_values = nullptr;
    flag.repeated_count = 0;

    return true;
}

bool AddArgument(ArgumentParser& parser,
                           const char* short_name,
                           const char* long_name,
                           void* storage,
                           ArgType type,
                           const char* description,
                           ArgNargs nargs,
                           bool (*validator)(const void*),
                           const char* error_msg) {
    ExpandArgs(parser);
    ArgumentParser::ArgItem& arg = parser.args[parser.arg_count++];
    std::memset(&arg, 0, sizeof(arg));

    arg.short_name = short_name;
    arg.long_name = long_name;
    arg.description = description;
    arg.storage = storage;
    arg.type = type;
    arg.nargs = nargs;
    arg.is_required = (nargs == ArgNargs::kRequired);
    arg.validator = validator;
    arg.error_msg = error_msg;

    return true;
}

// =============================
//  Повторяющиеся аргументы
// =============================
int GetRepeatedCount(const ArgumentParser& parser, const char* long_name) {
    for (int i = 0; i < parser.arg_count; i++) {
        const ArgumentParser::ArgItem& arg = parser.args[i];
        if (arg.long_name && strcmp(arg.long_name, long_name) == 0)
            return arg.repeated_count;
    }
    return 0;
}

const int* GetRepeated(const ArgumentParser& parser, const char* long_name, int index) {
    for (int i = 0; i < parser.arg_count; i++) {
        const ArgumentParser::ArgItem& a = parser.args[i];
        
        if (a.long_name && strcmp(a.long_name, long_name) == 0 && a.type == ArgType::kInt) {
            if (index < 0 || index >= a.repeated_count) return nullptr;
            return (const int*)a.repeated_values[index];
        }
    }
    return nullptr;
}

// =============================
//  Парсинг аргументов
// =============================
bool Parse(ArgumentParser& parser, int argc, const char** argv) {
    char prev_char = '\0';
    for (int i = 1; i < argc; i++) {
        char cur_char = argv[i][0];
        if (cur_char == '-' && prev_char == '-') {
            
        }
    
        prev_char = cur_char;
    }
    
    return true;
}