#include "argparser.h"
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <cstdio>

namespace nargparse {

bool SetArgumentValue(ArgumentParser::ArgItem& arg, const char* value, size_t max_arg_len);
bool StringToInt(const char* str, int* out);
bool StringToFloat(const char* str, float* out);

ArgumentParser CreateParser(const char* program_name, size_t max_arg_len) {
    ArgumentParser parser;
    parser.program_name = program_name;
    parser.max_arg_len = max_arg_len;
    parser.args = nullptr;
    parser.arg_count = 0;
    parser.arg_capacity = 0;
    parser.help_added = false;
    return parser;
}

void FreeParser(ArgumentParser& parser) {
    for (int i = 0; i < parser.arg_count; i++) {
        if (parser.args[i].repeated_values != nullptr) {
            for (int j = 0; j < parser.args[i].repeated_count; j++) {
                if (parser.args[i].type == ArgType::kString) {
                    delete[] (char*)parser.args[i].repeated_values[j];
                } else if (parser.args[i].type == ArgType::kInt) {
                    delete (int*)parser.args[i].repeated_values[j];
                } else if (parser.args[i].type == ArgType::kFloat) {
                    delete (float*)parser.args[i].repeated_values[j];
                }
            }
            delete[] parser.args[i].repeated_values;
        }
    }
    delete[] parser.args;
    parser.args = nullptr;
    parser.arg_count = 0;
    parser.arg_capacity = 0;
}

void ExpandArgs(ArgumentParser& parser) {
    if (parser.arg_count >= parser.arg_capacity) {
        int new_capacity = (parser.arg_capacity == 0) ? 10 : parser.arg_capacity * 2;
        ArgumentParser::ArgItem* new_args = new ArgumentParser::ArgItem[new_capacity];
        
        if (parser.args != nullptr) {
            for (int i = 0; i < parser.arg_count; i++) {
                new_args[i] = parser.args[i];
            }
            delete[] parser.args;
        }
        
        parser.args = new_args;
        parser.arg_capacity = new_capacity;
    }
}

void ExpandRepeated(ArgumentParser::ArgItem& arg) {
    if (arg.repeated_count >= arg.repeated_capacity) {
        int new_capacity = (arg.repeated_capacity == 0) ? 8 : arg.repeated_capacity * 2;
        void** new_values = new void*[new_capacity];
        

        if (arg.repeated_values != nullptr) {
            std::memcpy(new_values, arg.repeated_values, arg.repeated_count);
            delete[] arg.repeated_values;
        }
        
        arg.repeated_values = new_values;
        arg.repeated_capacity = new_capacity;
    }
}

ArgumentParser::ArgItem* FindArg(ArgumentParser& parser, const char* short_name, const char* long_name) {
    for (int i = 0; i < parser.arg_count; i++) {
        if (long_name != nullptr && parser.args[i].long_name != nullptr && 
            std::strcmp(parser.args[i].long_name, long_name) == 0) {
            return &parser.args[i];
        }
        if (short_name != nullptr && parser.args[i].short_name != nullptr && 
            std::strcmp(parser.args[i].short_name, short_name) == 0) {
            return &parser.args[i];
        }
    }
    return nullptr;
}

bool StringToInt(const char* str, int* out) {
    if (str == nullptr || str[0] == '\0') return false;
    
    char* end;
    long val = std::strtol(str, &end, 10);
    
    if (*end != '\0') return false;
    *out = (int)val;
    return true;
}

bool StringToFloat(const char* str, float* out) {
    if (str == nullptr || str[0] == '\0') return false;
    
    char* end;
    float val = std::strtof(str, &end);
    
    if (*end != '\0') return false;
    *out = val;
    return true;
}

bool AddFlag(ArgumentParser& parser,
             const char* short_name,
             const char* long_name,
             bool* storage,
             const char* description,
             bool default_value) {
    ExpandArgs(parser);
    
    ArgumentParser::ArgItem& arg = parser.args[parser.arg_count++];
    arg.short_name = short_name;
    arg.long_name = long_name;
    arg.description = description;
    arg.storage = storage;
    arg.type = ArgType::kFlag;
    arg.nargs = ArgNargs::kRequired;
    arg.is_required = false;
    arg.was_set = false;
    arg.validator = nullptr;
    arg.error_msg = nullptr;
    arg.repeated_values = nullptr;
    arg.repeated_count = 0;
    arg.repeated_capacity = 0;
    
    // Установить значение по умолчанию
    if (storage != nullptr) *storage = default_value;
    return true;
}

// =============================
//   AddArgument (общая)
// =============================

bool AddArgument(ArgumentParser& parser,
                 const char* short_name,
                 const char* long_name,
                 void* storage,
                 ArgType type,
                 const char* description,
                 ArgNargs nargs,
                 void* validator,
                 const char* error_msg) {
    ExpandArgs(parser);
    
    ArgumentParser::ArgItem& arg = parser.args[parser.arg_count++];
    arg.short_name = short_name;
    arg.long_name = long_name;
    arg.description = description;
    arg.storage = storage;
    arg.type = type;
    arg.nargs = nargs;
    arg.is_required = (nargs == ArgNargs::kRequired || nargs == ArgNargs::kOneOrMore);
    arg.was_set = false;
    arg.validator = validator;
    arg.error_msg = error_msg;
    arg.repeated_values = nullptr;
    arg.repeated_count = 0;
    arg.repeated_capacity = 0;
    
    return true;
}

// =============================
//   AddArgument - Positional
// =============================

bool AddArgument(ArgumentParser& parser,
                 int* storage,
                 const char* description,
                 ArgNargs nargs,
                 bool (*validator)(const int&),
                 const char* error_msg) {
    return AddArgument(parser, nullptr, nullptr, storage, ArgType::kInt, description, nargs,
                      (void*)validator, error_msg);
}

bool AddArgument(ArgumentParser& parser,
                 float* storage,
                 const char* description,
                 ArgNargs nargs,
                 bool (*validator)(const float&),
                 const char* error_msg) {
    return AddArgument(parser, nullptr, nullptr, storage, ArgType::kFloat, description, nargs,
                      (void*)validator, error_msg);
}

bool AddArgument(ArgumentParser& parser,
                 char (*storage)[128],
                 const char* description,
                 ArgNargs nargs,
                 bool (*validator)(const char* const&),
                 const char* error_msg) {
    return AddArgument(parser, nullptr, nullptr, storage, ArgType::kString, description, nargs,
                      (void*)validator, error_msg);
}

// =============================
//   AddArgument - Named
// =============================

bool AddArgument(ArgumentParser& parser,
                 const char* short_name,
                 const char* long_name,
                 int* storage,
                 const char* description,
                 ArgNargs nargs,
                 bool (*validator)(const int&),
                 const char* error_msg) {
    return AddArgument(parser, short_name, long_name, storage, ArgType::kInt, description, nargs,
                      (void*)validator, error_msg);
}

bool AddArgument(ArgumentParser& parser,
                 const char* short_name,
                 const char* long_name,
                 float* storage,
                 const char* description,
                 ArgNargs nargs,
                 bool (*validator)(const float&),
                 const char* error_msg) {
    return AddArgument(parser, short_name, long_name, storage, ArgType::kFloat, description, nargs,
                      (void*)validator, error_msg);
}

bool AddArgument(ArgumentParser& parser,
                 const char* short_name,
                 const char* long_name,
                 char (*storage)[128],
                 const char* description,
                 ArgNargs nargs,
                 bool (*validator)(const char* const&),
                 const char* error_msg) {
    return AddArgument(parser, short_name, long_name, storage, ArgType::kString, description, nargs, 
                      (void*)validator, error_msg);
}

bool AddHelp(ArgumentParser& parser,
             const char* long_name,
             const char* short_name,
             const char* description) {
    parser.help_added = true;
    bool flag = false;
    return AddFlag(parser, short_name, long_name, &flag, description, false);
}

bool SetArgumentValue(ArgumentParser::ArgItem& arg, const char* value, size_t max_arg_len) {
    if (arg.type == ArgType::kInt) {
        int val = 0;
        if (!StringToInt(value, &val)) return false;
        
        if (arg.validator != nullptr) {
            bool (*typed_validator)(const int&) = (bool(*)(const int&))arg.validator;
            if (!typed_validator(val)) return false;
        }
        
        if (arg.repeated_count == 0 && arg.storage != nullptr) {
            *(int*)arg.storage = val;
        }
        
        if (arg.nargs == ArgNargs::kZeroOrMore || arg.nargs == ArgNargs::kOneOrMore) {
            ExpandRepeated(arg);
            int* p = new int(val);
            arg.repeated_values[arg.repeated_count++] = p;
        }
        
        arg.was_set = true;
        return true;

    } else if (arg.type == ArgType::kFloat) {
        float val = 0.0f;
        if (!StringToFloat(value, &val)) return false;
        
        if (arg.validator != nullptr) {
            bool (*type_validator)(const float&) = (bool(*)(const float&))arg.validator;
            if (!type_validator(val)) return false;
        }
        
        if (arg.repeated_count == 0 && arg.storage != nullptr) {
            *(float*)arg.storage = val;
        }

        if (arg.nargs == ArgNargs::kZeroOrMore || arg.nargs == ArgNargs::kOneOrMore) {
            ExpandRepeated(arg);
            float* p = new float(val);
            arg.repeated_values[arg.repeated_count++] = p;
        }
        
        arg.was_set = true;
        return true;
        
    } else if (arg.type == ArgType::kString) {
        if (std::strlen(value) >= max_arg_len) return false;

        if (arg.validator != nullptr) {
            bool (*typed_validator)(const char* const&) = (bool(*)(const char* const&))arg.validator;
            if (!typed_validator(value)) return false;
        }
        
        if (arg.repeated_count == 0 && arg.storage != nullptr) {
            std::strcpy((char*)arg.storage, value);
        }
        
        if (arg.nargs == ArgNargs::kZeroOrMore || arg.nargs == ArgNargs::kOneOrMore) {
            ExpandRepeated(arg);
            char* p = new char[max_arg_len];
            std::strcpy(p, value);
            arg.repeated_values[arg.repeated_count++] = p;
        }
        
        arg.was_set = true;
    }
    return true;
}

bool ParsePositional(ArgumentParser& parser, int& positional_index, const char* arg) {
    ArgumentParser::ArgItem* item = nullptr;
    
    for (int j = positional_index; j < parser.arg_count; j++) {
        if (parser.args[j].short_name == nullptr && parser.args[j].type != ArgType::kFlag) {
            item = &parser.args[j];
            positional_index = j;
            break;
        }
    }
    
    if (item == nullptr) return false;
    
    if (item->repeated_count == 0 && item->nargs != ArgNargs::kZeroOrMore && item->nargs != ArgNargs::kOneOrMore) {
        positional_index++;
    }
    
    return SetArgumentValue(*item, arg, parser.max_arg_len);
}

int GetRepeatedCount(const ArgumentParser& parser, const char* long_name) {
    for (int i = 0; i < parser.arg_count; i++) {
        if (parser.args[i].description != nullptr && std::strcmp(parser.args[i].description, long_name) == 0) {
            return parser.args[i].repeated_count;
        }

        if (parser.args[i].long_name != nullptr && std::strcmp(parser.args[i].long_name, long_name) == 0) {
            return parser.args[i].repeated_count;
        }
    }
    return 0;
}

bool GetRepeated(const ArgumentParser& parser, const char* long_name, int index, void* out) {
    for (int i = 0; i < parser.arg_count; i++) {
        bool found = false;
        if (parser.args[i].description != nullptr && std::strcmp(parser.args[i].description, long_name) == 0) {
            found = true;
        } else if (parser.args[i].long_name != nullptr && std::strcmp(parser.args[i].long_name, long_name) == 0) {
            found = true;
        }
        
        if (found) {
            if (index < 0 || index >= parser.args[i].repeated_count) {
                return false;
            }
            
            void* value = parser.args[i].repeated_values[index];
            
            if (parser.args[i].type == ArgType::kInt) {
                *(int*)out = *(int*)value;
            } else if (parser.args[i].type == ArgType::kFloat) {
                *(float*)out = *(float*)value;
            } else if (parser.args[i].type == ArgType::kString) {
                *(const char**)out = (const char*)value;
            }
            
            return true;
        }
    }
    return false;
}

bool CheckPositional(ArgumentParser& parser) {
    for (int i = 0; i < parser.arg_count; i++) {
        if (parser.args[i].is_required && !parser.args[i].was_set) {
            return false;
        }
    }

    return true;
}

bool IsToken(const char* arg) {
    return arg != nullptr && arg[0] == '-';
}

bool IsShortToken(const char* arg) {
    return arg != nullptr && arg[0] == '-' && arg[1] != '-' && arg[1] != '\0';
}

bool IsLongToken(const char* arg) {
    return arg != nullptr && arg[0] == '-' && arg[1] == '-' && arg[2] != '\0';
}

void GetFlag(const char* token, char* name, const char** value) {
    const char* eq = std::strchr(token, '=');
    if (eq != nullptr) {
        *value = eq + 1;
        std::strncpy(name, token, eq - token);
        name[eq - token] = '\0';
    } else {
        std::strcpy(name, token);
    }
}

bool Parse(ArgumentParser& parser, int argc, const char** argv) {
    int positional_index = 0;

    for (int i = 1; i < argc; i++) {
        const char* token = argv[i];
        
        if (!IsToken(token)) {
            // Это позиционный аргумент
            if (!ParsePositional(parser, positional_index, token)) return false;
            
        } else {
            // Это флаг или именованный аргумент
            char name[256];
            const char* value = nullptr;
            GetFlag(token, name, &value);
            
            if (parser.help_added && 
                (std::strcmp(name, "--help") == 0 || std::strcmp(name, "-h") == 0)) {
                return true;
            }
            
            ArgumentParser::ArgItem* item = FindArg(parser, name, name);
            if (item == nullptr) return false;
            
            if (item->type == ArgType::kFlag) {
                if (item->storage != nullptr) {
                    *(bool*)item->storage = true;
                }
                item->was_set = true;

            } else {
                if (value == nullptr) {
                    if (i + 1 < argc) {
                        value = argv[++i];
                    } else {
                        return false;
                    }
                }
                
                if (item->was_set && item->nargs != ArgNargs::kZeroOrMore && item->nargs != ArgNargs::kOneOrMore) {
                    return false;  // Дублирование аргумента
                }
                
                if (!SetArgumentValue(*item, value, parser.max_arg_len)) {
                    return false;
                }
            }
        }
    }
    
    if (!CheckPositional(parser)) return false;
    
    return true;
}

}  // namespace nargparse