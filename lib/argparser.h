#pragma once
#include <cstddef>
#include <cstdbool>

namespace nargparse {
const size_t kMaxArgLen = 256;

enum class ArgNargs {
    kRequired,
    kOptional,
    kZeroOrMore,
    kOneOrMore
};

constexpr ArgNargs kNargsZeroOrMore = ArgNargs::kZeroOrMore;
constexpr ArgNargs kNargsOneOrMore  = ArgNargs::kOneOrMore;
constexpr ArgNargs kNargsRequired   = ArgNargs::kRequired;
constexpr ArgNargs kNargsOptional   = ArgNargs::kOptional;

enum class ArgType {
    kInt,
    kFloat,
    kString,
    kFlag
};

struct ArgumentParser {
    const char* program_name;
    size_t max_arg_len;

    struct ArgItem {
        const char* long_name;
        const char* short_name;
        const char* description;
        
        void* storage;
        void* validator;
        ArgNargs nargs;
        bool is_required;
        bool was_set;
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
};


ArgumentParser CreateParser(const char* program_name, size_t max_arg_len=kMaxArgLen);
void FreeParser(ArgumentParser& parser);

bool AddFlag(ArgumentParser& parser,
             const char* short_name,
             const char* long_name,
             bool* storage,
             const char* description,
             bool default_value=false);

// =============================
//   AddArgument - общая
// =============================
bool AddArgument(ArgumentParser& parser,
                 const char* short_name,
                 const char* long_name,
                 void* storage,
                 ArgType type,
                 const char* description,
                 ArgNargs nargs,
                 void* validator,
                 const char* error_msg);

// =============================
//   AddArgument - Positional
// =============================

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
                 char (*storage)[128],
                 const char* description,
                 ArgNargs nargs = ArgNargs::kRequired,
                 bool (*validator)(const char* const&) = nullptr,
                 const char* error_msg = nullptr);

// =============================
//   AddArgument - Named
// =============================

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
                 char (*storage)[128],
                 const char* description,
                 ArgNargs nargs = ArgNargs::kRequired,
                 bool (*validator)(const char* const&) = nullptr,
                 const char* error_msg = nullptr);

// =============================
//   Parsing
// =============================

bool Parse(ArgumentParser& parser, int argc, const char** argv);

int GetRepeatedCount(const ArgumentParser& parser, const char* long_name);
bool GetRepeated(const ArgumentParser& parser, const char* long_name, int index, void* out);

bool AddHelp(ArgumentParser& parser,
             const char* long_name = "--help",
             const char* short_name = "-h",
             const char* description = "Default help");

// =============================
//  Вспомогательные функции
// =============================
void ExpandArgs(ArgumentParser& parser);
void ExpandRepeated(ArgumentParser::ArgItem& arg);
ArgumentParser::ArgItem* FindArg(ArgumentParser& parser, const char* short_name, const char* long_name);

}