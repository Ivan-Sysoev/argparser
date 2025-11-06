#include "argparser.h"
#include <cstring>
#include <cstdlib>
#include <cstdio>

using namespace nargparse;

// =============================
//  Вспомогательные функции
// =============================
static void ExpandArgs(ArgumentParser& parser) {
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

static void ExpandRepeated(ArgumentParser::ArgItem& arg) {
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

static ArgumentParser::ArgItem* FindArg(ArgumentParser& parser, const char* name) {
    for (int i = 0; i < parser.arg_count; i++) {
        auto& a = parser.args[i];
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
bool AddFlag(ArgumentParser& parser, const char* short_name, const char* long_name, bool* storage, const char* description, bool default_value = false) {
    ExpandArgs(parser);

    *storage = default_value;

    ArgumentParser::ArgItem& flag = parser.args[parser.arg_count++];
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

// nargparse::AddArgument(parser, &first_value, "Positive numbers", nargparse::kNargsZeroOrMore, IsPositive, "must be positive");
// nargparse::AddArgument(parser, "-n", "--count", &count, "Count value");
bool AddArgument(ArgumentParser& parser,
                 int* storage,
                 const char* short_name = nullptr,
                 const char* long_name = nullptr,
                 ArgNargs nargs = ArgNargs::kRequired,
                 bool required = false,
                 const int* default_value = nullptr,
                 bool (*validator)(const int&) = nullptr,
                 const char* description = nullptr) {
    ExpandArgs(parser);
    ArgumentParser::ArgItem& arg = parser.args[parser.arg_count++];
    memset(&arg, 0, sizeof(arg));

    arg.long_name = long_name;
    arg.short_name = short_name;
    arg.description = description;
    arg.storage = storage;
    arg.default_value = (void*)default_value;
    arg.nargs = nargs;
    arg.is_required = required;
    arg.type = ArgType::kInt;
    arg.validator = (bool (*)(const void*))validator;

    return true;
}

// =============================
//  Повторяющиеся аргументы
// =============================
int GetRepeatedCount(const ArgumentParser& parser, const char* long_name) {
    for (int i = 0; i < parser.arg_count; i++) {
        const auto& a = parser.args[i];
        if (a.long_name && strcmp(a.long_name, long_name) == 0)
            return a.repeated_count;
    }
    return 0;
}

const int* GetRepeated(const ArgumentParser& parser, const char* long_name, int index) {
    for (int i = 0; i < parser.arg_count; i++) {
        const auto& a = parser.args[i];
        if (a.long_name && strcmp(a.long_name, long_name) == 0 && a.type == ARG_INT) {
            if (index < 0 || index >= a.repeated_count) return nullptr;
            return (const int*)a.repeated_values[index];
        }
    }
    return nullptr;
}

// =============================
//  Парсинг аргументов
// =============================
bool Parse(ArgumentParser& parser, int argc, const char* argv[]) {
    for (int i = 1; i < argc; i++) {
        const char* token = argv[i];

        // --- Флаг или именованный аргумент ---
        if (token[0] == '-') {
            const char* name = (token[1] == '-') ? token : token;
            ArgumentParser::ArgItem* arg = FindArg(parser, name);
            if (!arg) {
                fprintf(stderr, "Unknown option: %s\n", token);
                return false;
            }

            if (arg->type == ARG_FLAG) {
                if (arg->storage) *((bool*)arg->storage) = true;
                continue;
            }

            // --- Аргумент с числовым значением ---
            if (i + 1 >= argc) {
                fprintf(stderr, "Missing value for %s\n", token);
                return false;
            }
            const char* val_str = argv[i++];
            int val = atoi(val_str);

            if (arg->validator && !arg->validator(&val)) {
                fprintf(stderr, "Invalid value for %s: %s\n", token, val_str);
                return false;
            }

            if (arg->nargs == NARGS_REQUIRED || arg->nargs == NARGS_OPTIONAL) {
                if (arg->storage) *((int*)arg->storage) = val;
            } else { // Повторяющийся
                ExpandRepeated(*arg);
                int* stored = (int*)malloc(sizeof(int));
                *stored = val;
                arg->repeated_values[arg->repeated_count++] = stored;
            }
            continue;
        }

        // --- Позиционный аргумент ---
        // Можно добавить аналогично, если требуется
    }

    // Проверка обязательных аргументов
    for (int i = 0; i < parser.arg_count; i++) {
        auto& a = parser.args[i];
        if (a.is_required && !a.is_flag && a.nargs >= NARGS_REQUIRED && !a.storage) {
            fprintf(stderr, "Missing required argument: %s\n", a.long_name);
            return false;
        }
    }

    return true;
}
