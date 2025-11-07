#pragma once
#include <cstddef>
#include <cstdbool>

namespace nargparse {

// -------------------------
//   Количество значений аргумента
// -------------------------
enum class ArgNargs {
    kRequired     = 1,   // Один обязательный аргумент
    kOptional     = 0,   // Один необязательный аргумент
    kZeroOrMore   = -1,  // 0 или больше аргументов
    kOneOrMore    = -2   // 1 или больше аргументов
};

// -------------------------
//   Тип аргумента
// -------------------------
enum class ArgType {
    kInt,
    kFloat,
    kString,
    kFlag
};

// -------------------------
//   Основная структура парсера
// -------------------------
struct ArgumentParser {
    const char* program_name;
    size_t max_arg_len;

    struct ArgItem {
        const char* long_name;
        const char* short_name;
        const char* description;

        void* storage;
        void* default_value;
        bool (*validator)(const void*);
        ArgNargs nargs;
        bool is_required;
        ArgType type;

        void** repeated_values;
        int repeated_count;
        int repeated_capacity;

        const char* error_msg;
    };

    ArgItem* args;
    int arg_count;
    int arg_capacity;

    bool help_added;
    bool help_requested;
};

// -------------------------
//   Интерфейс
// -------------------------
ArgumentParser CreateParser(const char* program_name, size_t max_arg_len);
void FreeParser(ArgumentParser& parser);

bool AddFlag(ArgumentParser& parser,
             const char* short_name,
             const char* long_name,
             bool* storage,
             const char* description,
             bool default_value=false);

// -------------------------
//   Positional argument
// -------------------------
bool AddArgument(ArgumentParser& parser,
                 int* storage,
                 const char* description,
                 ArgNargs nargs = ArgNargs::kRequired,
                 bool (*validator)(const int&) = nullptr,
                 const char* error_msg = nullptr);

bool AddArgument(ArgumentParser& parser,
                 float* storage,
                 const char* description,
                 ArgNargs nargs = ArgNargs::kRequired,
                 bool (*validator)(const float&) = nullptr,
                 const char* error_msg = nullptr);

bool AddArgument(ArgumentParser& parser,
                 char* storage,
                 const char* description,
                 ArgNargs nargs = ArgNargs::kRequired,
                 bool (*validator)(const char*) = nullptr,
                 const char* error_msg = nullptr);

// -------------------------
//   Named argument
// -------------------------
bool AddArgument(ArgumentParser& parser,
                 const char* short_name,
                 const char* long_name,
                 int* storage,
                 const char* description,
                 ArgNargs nargs = ArgNargs::kRequired,
                 bool (*validator)(const int&) = nullptr,
                 const char* error_msg = nullptr);

bool AddArgument(ArgumentParser& parser,
                 const char* short_name,
                 const char* long_name,
                 float* storage,
                 const char* description,
                 ArgNargs nargs = ArgNargs::kRequired,
                 bool (*validator)(const float&) = nullptr,
                 const char* error_msg = nullptr);

bool AddArgument(ArgumentParser& parser,
                 const char* short_name,
                 const char* long_name,
                 char* storage,
                 const char* description,
                 ArgNargs nargs = ArgNargs::kRequired,
                 bool (*validator)(const char*) = nullptr,
                 const char* error_msg = nullptr);


bool Parse(ArgumentParser& parser, int argc, const char* argv[]);

int GetRepeatedCount(const ArgumentParser& parser, const char* long_name);
const int* GetRepeated(const ArgumentParser& parser, const char* long_name, int index);

bool AddHelp(ArgumentParser& parser,
             const char* long_name = "--help",
             const char* short_name = "-h",
             const char* description = "Default help");
}