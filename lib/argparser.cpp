#include "argparser.h"
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <cstdio>

namespace nargparse {

// =============================
//   Forward declarations
// =============================
bool SetArgumentValue(ArgumentParser::ArgItem& arg, const char* value, size_t max_arg_len);
bool StringToInt(const char* str, int* out);
bool StringToFloat(const char* str, float* out);

// =============================
//   Создание и уничтожение парсера
// =============================

// Создает новый парсер с пустым списком аргументов
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

// Освобождает всю память, связанную с парсером
// Очищает all arguments, their values, and metadata
void FreeParser(ArgumentParser& parser) {
    for (int i = 0; i < parser.arg_count; i++) {
        // Освобождаем повторяющиеся значения аргумента
        if (parser.args[i].repeated_values != nullptr) {
            for (int j = 0; j < parser.args[i].repeated_count; j++) {
                // Тип очистки зависит от типа аргумента
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
    // Финальная очистка главного массива аргументов
    delete[] parser.args;
    parser.args = nullptr;
    parser.arg_count = 0;
    parser.arg_capacity = 0;
}

// =============================
//   Динамическое расширение массивов
// =============================

// Расширяет массив аргументов, если достигнута максимальная емкость
// Использует стратегию удвоения размера (начиная с 10 элементов)
void ExpandArgs(ArgumentParser& parser) {
    if (parser.arg_count >= parser.arg_capacity) {
        // Если это первое выделение, начни с 10 элементов; иначе удвой размер
        int new_capacity = (parser.arg_capacity == 0) ? 10 : parser.arg_capacity * 2;
        ArgumentParser::ArgItem* new_args = new ArgumentParser::ArgItem[new_capacity];
        
        // Копируем старые элементы в новый массив (если они есть)
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

// Расширяет массив повторяющихся значений для конкретного аргумента
// Используется когда аргумент может быть повторен (kZeroOrMore, kOneOrMore)
void ExpandRepeated(ArgumentParser::ArgItem& arg) {
    if (arg.repeated_count >= arg.repeated_capacity) {
        int new_capacity = (arg.repeated_capacity == 0) ? 8 : arg.repeated_capacity * 2;
        void** new_values = new void*[new_capacity];
        
        // Копируем старые значения в новый массив
        if (arg.repeated_values != nullptr) {
            std::memcpy(new_values, arg.repeated_values, arg.repeated_count);
            delete[] arg.repeated_values;
        }
        
        arg.repeated_values = new_values;
        arg.repeated_capacity = new_capacity;
    }
}

// =============================
//   Поиск аргумента
// =============================
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

// =============================
//   Преобразование типов (string -> типизированные значения)
// =============================

bool StringToInt(const char* str, int* out) {
    if (str == nullptr || str[0] == '\0') return false;
    
    char* end;
    long val = std::strtol(str, &end, 10);
    
    // Проверяем, была ли вся строка успешно преобразована
    if (*end != '\0') return false;
    *out = (int)val;
    return true;
}

// Преобразует строку в число с плавающей точкой
bool StringToFloat(const char* str, float* out) {
    if (str == nullptr || str[0] == '\0') return false;
    
    char* end;
    float val = std::strtof(str, &end);
    
    // Проверяем, была ли вся строка успешно преобразована
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

// =============================
//   AddHelp
// =============================

bool AddHelp(ArgumentParser& parser,
             const char* long_name,
             const char* short_name,
             const char* description) {
    parser.help_added = true;
    bool flag = false;
    return AddFlag(parser, short_name, long_name, &flag, description, false);
}

// =============================
//   Вспомогательные функции парсинга
// =============================

// Устанавливает значение для аргумента
// Обрабатывает все типы (int, float, string), валидацию и повторения
// Возвращает true если успешно, false если конвертация или валидация не удалась
bool SetArgumentValue(ArgumentParser::ArgItem& arg, const char* value, size_t max_arg_len) {
    // ===== ОБРАБОТКА ЦЕЛЫХ ЧИСЕЛ =====
    if (arg.type == ArgType::kInt) {
        int val = 0;
        // Шаг 1: Преобразуем строку в число
        if (!StringToInt(value, &val)) {
            return false;  // Ошибка парсинга
        }
        
        // Шаг 2: Валидируем значение (если задан validator)
        if (arg.validator != nullptr) {
            bool (*typed_validator)(const int&) = (bool(*)(const int&))arg.validator;
            if (!typed_validator(val)) {
                return false;  // Валидация провалилась
            }
        }
        
        // Шаг 3: Сохраняем значение в основное хранилище (для первого значения)
        if (arg.repeated_count == 0 && arg.storage != nullptr) {
            *(int*)arg.storage = val;
        }
        
        // Шаг 4: Если это повторяющийся аргумент, добавляем в массив
        if (arg.nargs == ArgNargs::kZeroOrMore || arg.nargs == ArgNargs::kOneOrMore) {
            ExpandRepeated(arg);
            int* p = new int(val);
            arg.repeated_values[arg.repeated_count++] = p;
        }
        
        arg.was_set = true;  // Отмечаем, что аргумент был установлен
        return true;
        
    // ===== ОБРАБОТКА ЧИСЕЛ С ПЛАВАЮЩЕЙ ТОЧКОЙ =====
    } else if (arg.type == ArgType::kFloat) {
        float val = 0.0f;
        // Шаг 1: Преобразуем строку в число
        if (!StringToFloat(value, &val)) {
            return false;
        }
        
        // Шаг 2: Валидируем значение
        if (arg.validator != nullptr) {
            bool (*typed_validator)(const float&) = (bool(*)(const float&))arg.validator;
            if (!typed_validator(val)) {
                return false;
            }
        }
        
        // Шаг 3: Сохраняем в основное хранилище
        if (arg.repeated_count == 0 && arg.storage != nullptr) {
            *(float*)arg.storage = val;
        }
        
        // Шаг 4: Добавляем в массив если это повторяющийся аргумент
        if (arg.nargs == ArgNargs::kZeroOrMore || arg.nargs == ArgNargs::kOneOrMore) {
            ExpandRepeated(arg);
            float* p = new float(val);
            arg.repeated_values[arg.repeated_count++] = p;
        }
        
        arg.was_set = true;
    } else if (arg.type == ArgType::kString) {
        if (std::strlen(value) >= max_arg_len) {
            return false;
        }
        if (arg.validator != nullptr) {
            bool (*typed_validator)(const char* const&) = (bool(*)(const char* const&))arg.validator;
            if (!typed_validator(value)) {
                return false;
            }
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

// Обработать позиционный аргумент
// positional_index - индекс следующего позиционного аргумента для обработки
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
    
    // Если это первое значение и аргумент не повторяющийся, то перейти к следующему позиционному
    if (item->repeated_count == 0 && item->nargs != ArgNargs::kZeroOrMore && item->nargs != ArgNargs::kOneOrMore) {
        positional_index++;
    }
    
    return SetArgumentValue(*item, arg, parser.max_arg_len);
}

// =============================
//   Получение повторяющихся значений
// =============================

// Возвращает количество значений для повторяющегося аргумента
// Ищет аргумент по description или long_name
int GetRepeatedCount(const ArgumentParser& parser, const char* long_name) {
    for (int i = 0; i < parser.arg_count; i++) {
        // Сначала пробуем найти по description (работает для всех аргументов)
        if (parser.args[i].description != nullptr && std::strcmp(parser.args[i].description, long_name) == 0) {
            return parser.args[i].repeated_count;
        }
        // Затем по long_name (для именованных аргументов)
        if (parser.args[i].long_name != nullptr && std::strcmp(parser.args[i].long_name, long_name) == 0) {
            return parser.args[i].repeated_count;
        }
    }
    return 0;
}

// Возвращает значение на определенной позиции для повторяющегося аргумента
// out - указатель куда записать результат (тип зависит от arg type)
// Возвращает true если успешно получено, false если индекс неверный
bool GetRepeated(const ArgumentParser& parser, const char* long_name, int index, void* out) {
    for (int i = 0; i < parser.arg_count; i++) {
        // Ищем аргумент по description или long_name
        bool found = false;
        if (parser.args[i].description != nullptr && std::strcmp(parser.args[i].description, long_name) == 0) {
            found = true;
        } else if (parser.args[i].long_name != nullptr && std::strcmp(parser.args[i].long_name, long_name) == 0) {
            found = true;
        }
        
        if (found) {
            // Проверяем, что индекс в допустимых пределах
            if (index < 0 || index >= parser.args[i].repeated_count) {
                return false;
            }
            
            // Извлекаем значение из массива
            void* value = parser.args[i].repeated_values[index];
            
            // Копируем значение в правильном формате в зависимости от типа
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