#include "argparser.h"
#include <cstring>

using namespace nargparse;

ArgumentParser& CreateParser(const char* program_name, size_t max_arg_len) {
    ArgumentParser parser {};
    parser.program_name = program_name;
    parser.max_arg_len = max_arg_len;
    parser.args = nullptr;
    parser.arg_count = 0;
    parser.arg_capacity = 0;
    parser.help_added = false;
    parser.help_requested = false;
    return parser;
}
