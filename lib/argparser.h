#pragma once
#include <iostream>

namespace nargparse {

struct ArgumentParser;

extern const int kNargsRequired;
extern const int kNargsOptional;
extern const int kNargsZeroOrMore;
extern const int kNargsOneOrMore;

ArgumentParser CreateParser(const char* program_name, size_t max_arg_len);
void FreeParser(ArgumentParser& parser);

bool AddFlag(ArgumentParser& parser,
              bool* storage,
              const char* long_name,
              const char* short_name = nullptr,
              const char* description = nullptr);

bool AddArgument(ArgumentParser& parser,
                 int* storage,
                 const char* long_name,
                 const char* short_name = nullptr,
                 int nargs = kNargsRequired,
                 bool required = false,
                 const int* default_value = nullptr,
                 bool (*validator)(const int&) = nullptr,
                 const char* description = nullptr);

bool AddArgument(ArgumentParser& parser,
                 float* storage,
                 const char* long_name,
                 const char* short_name = nullptr,
                 int nargs = kNargsRequired,
                 bool required = false,
                 const float* default_value = nullptr,
                 bool (*validator)(const float&) = nullptr,
                 const char* description = nullptr);

bool AddArgument(ArgumentParser& parser,
                 const char** storage,
                 const char* long_name,
                 const char* short_name = nullptr,
                 int nargs = kNargsRequired,
                 bool required = false,
                 const char* default_value = nullptr,
                 bool (*validator)(const char* const&) = nullptr,
                 const char* description = nullptr);

bool AddHelp(ArgumentParser& parser,
              const char* long_name = "--help",
              const char* short_name = "-h",
              const char* description = "Display help information");

bool Parse(ArgumentParser& parser, int argc, const char* argv[]);

int GetRepeatedCount(const ArgumentParser& parser, const char* long_name);

const int* GetRepeated(const ArgumentParser& parser,
                       const char* long_name,
                       int index);

const float* GetRepeated(const ArgumentParser& parser,
                         const char* long_name,
                         int index);

const char* GetRepeated(const ArgumentParser& parser,
                        const char* long_name,
                        int index);

}