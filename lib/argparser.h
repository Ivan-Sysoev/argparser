#pragma once
#include <iostream>

static const size_t kMaxArgLen = 128;

namespace nargparse {
    
struct ArgumentParser {
    const char* program_name;   // Имя программы
    size_t max_arg_len;         // Максимальная длина строковых аргументов

    // ===== Вложенная структура для описания одного аргумента =====
    struct ArgItem {
        const char* long_name;     // --option
        const char* short_name;    // -o
        const char* description;   // Описание для help

        void* storage;             // Указатель на переменную, куда записывать значение
        void* default_value;       // Значение по умолчанию (если есть)
        bool (*validator)(const void*);  // Проверка значения (по типу)

        int nargs;                 // Кол-во значений (kNargsRequired и т.д.)
        bool is_required;          // Обязательный аргумент?
        bool is_flag;              // Флаг (bool) или аргумент?
        int type;                  // 0=int, 1=float, 2=string, 3=flag

        // ===== Повторяющиеся значения =====
        void** repeated_values;    // Динамический массив указателей на значения
        int repeated_count;        // Текущее количество значений
        int repeated_capacity;     // Размер выделенного массива
    };

    // ===== Массивы для аргументов =====
    ArgItem* args;                // Динамический массив аргументов
    int arg_count;                // Количество добавленных аргументов
    int arg_capacity;             // Размер выделенного массива под аргументы

    // ===== Служебные поля =====
    bool help_added;              // Был ли добавлен --help
    bool help_requested;          // Пользователь вызвал --help
};

ArgumentParser& CreateParser(const char* program_name, size_t max_arg_len);

void FreeParser(ArgumentParser& parser);
bool AddArgument(ArgumentParser& parser);
void AddFlag(ArgumentParser& parser, const char* short_name = nullptr, const char* long_name, bool& reference, const char* description, bool default_value=false);
bool Parse(ArgumentParser& parser, int argc, const char* argv[]);

}