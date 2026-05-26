/******************************************************************************
*                      КАФЕДРА №304 1 КУРС ПРОГИНЖ                            *
*                           Летняя Практика                                   *
*-----------------------------------------------------------------------------*
* Project Type  : Win32 Console Application                                   *
* Project Name  : MaiPractice                                                 *
* File Name     : main.cpp                                                    *
* Language      : C/C++                                                       *
* Programmer    : Георгий Алтынов                                             *
* Modified By   :                                                             *
* Created       : 5/05/26                                                     *
* Last Revision : 26/05/26                                                    *
* Comment(s)    : Работа со структурами и индексной сортировкой               *
******************************************************************************/


#include <iostream>
#include <fstream>
#include <iomanip>
#ifdef _WIN32
// Доступ к win api для смены кодировки консоли, если программа собирается из-под windows
#include <windows.h>
#endif

#include "read_write.h" // Реализация чтения и вспомогательных функций для вывода


/****************************************************************/
/*             О С Н О В Н А Я     П Р О Г Р А М М А            */
/****************************************************************/


// auto fname = "input.txt"; // Имя входного файла
auto fname = "tests/bad/all"; // Тест некорректных случаев
// auto fname = "tests/good/all"; // Тест некорректных случаев

// Индексная сортировка массива данных пузырьком
void bubble_index_sort(const Line *data, size_t shape, size_t *indexes);

// Вывод таблицы
void compile_table(const Line *data, size_t shape, const size_t *indexes);


int main() {
#ifdef _WIN32
    // Если программа собирается из-под windows, меняем кодировку консоли на UTF-8
    // В Linux и Darwin она выставлена по умолчанию
    SetConsoleOutputCP(CP_UTF8);
#endif

    std::ifstream input(fname); // Файловый поток для чтения
    if (!input.is_open()) {
        // Обработка ошибки открытия файла
        std::cerr << "Failed to open file: `" << fname << "`" << std::endl;
        return -1;
    }

    constexpr size_t buffsize = 512; // Размер буфера для чтения текущей строки
    constexpr size_t df_size = 1000; // Общий размер массива данных
    char buffer[buffsize]; // Буфер для текущей строки
    Line data[df_size]; // Массив данных АСУ ВД
    size_t indexes[df_size]; // Массив индексов для индексной сортировки
    size_t good_rows = 0; // Количество корректных строк
    size_t all_rows = 0; // Общее количество строк
    bool bad_data = false; // Флаг вывода шапки с некорректными строками

    // Чтение строк до ошибки потока (EOF или переполнение буфера)
    while (input.getline(buffer, buffsize)) {
        const size_t line_size = strlen(buffer); // Подсчет длины считанной строки
        if (ErrCode err = parse_line(buffer, data[good_rows]);
            err != GOOD || (err = check_logical(data, good_rows)) != GOOD) {
            // Получаем код ошибки от функции парсинга строки
            // Если он GOOD, берем код от функции поиска логических ошибок. Если он тоже GOOD - строка корректна
            // Иначе печатаем информацию об ошибке

            ++all_rows;
            if (!bad_data) {
                // Вывод шапки, если ее не было
                bad_data = true;
                std::cout << repeat{"=", 45} << std::endl;
                std::cout << "Bad lines:" << std::endl;
                std::cout << repeat{"=", 45} << std::endl << std::endl;
            }
            restore(buffer, line_size); // Восстановление строки перед печатью
            // Печать строки и информации об ошибке в ней
            std::cout << "#" << all_rows << " " << buffer << " ─ " << std::endl << err << std::endl << std::endl;
            continue;
        }

        ++all_rows;
        ++good_rows;
        if (good_rows == df_size) {
            // Если вышли за предел размера массива, заканчиваем чтение
            std::cerr << "DataFrame size limit exceeded. End parsing" << std::endl;
            break;
        }
    }
    // Вывод шапки с информацией о количестве считанных строк
    std::cout << repeat{"=", 45} << std::endl;
    std::cout << "Total parsed: " << good_rows << "/" << all_rows << std::endl;
    if (good_rows == 0) {
        std::cerr << "No good data found" << std::endl;
        return -1;
    }
    std::cout << repeat{"=", 45} << std::endl << std::endl;

    // Инициализация массива индексов
    for (size_t i = 0; i < good_rows; i++) {
        indexes[i] = i;
    }

    // Индексная сортировка пузырьком
    bubble_index_sort(data, good_rows, indexes);

    // Вывод таблицы с отсортированными данными
    std::cout << "Data table sorted by flight number:" << std::endl;
    compile_table(data, good_rows, indexes);
}


// Индексная сортировка массива данных пузырьком
void bubble_index_sort(const Line *data, const size_t shape, size_t *indexes) {
    // Сортируется массив индексов, массив структур не изменяется

    for (size_t i = 0; i < shape - 1; i++) {
        bool has_swp = false;
        for (size_t j = 0; j < shape - i - 1; j++) {
            // Сравнение бортовых номеров структур под номерами j и j+1
            if (data[indexes[j]].flight_number > data[indexes[j + 1]].flight_number) {
                // Местами меняются сами номера j и j+1
                has_swp = true;
                const size_t buff = indexes[j];
                indexes[j] = indexes[j + 1];
                indexes[j + 1] = buff;
            }
        }
        if (!has_swp) break;
    }
}


// Вывод таблицы
void compile_table(const Line *data, const size_t shape, const size_t *indexes) {
    // Шапка таблицы
    std::cout << "┌" << repeat{"─", 3} << "┬" << repeat{"─", 6} << "┬" << repeat{"─", 8} << "┬" <<
            repeat{"─", 17} << "┬" << repeat{"─", 5} << "┐" << std::endl;
    std::cout << "│" << std::setw(3) << "#" << "│" << std::setw(6) << "Bort" << "│" << std::setw(8)
            << "Flight" << "│" << std::setw(17) << "Aircraft" << "│" << std::setw(5) << "Time" << "│" << std::endl;

    for (size_t i = 0; i < shape; i++) {
        // i-ая строка таблицы
        const auto [bort_number, flight_number, aircraft, arrival] = data[indexes[i]];
        std::cout << "├" << repeat{"─", 3} << "┼" << repeat{"─", 6}
                << "┼" << repeat{"─", 8} << "┼" << repeat{"─", 17} << "┼"
                << repeat{"─", 5} << "┤" << std::endl;
        // Расчет выравнивания для марки ЛА с учетом особенностей UTF-8
        const int align = 17 + static_cast<int>(strlen(aircraft) - utf8_length(aircraft));

        // Вывод строки
        std::cout << std::setfill(' ') << "│" << std::setw(3) << i << "│" << bort_prefix << std::setfill('0') <<
                std::setw(4) << bort_number << "│" << flight_prefix << std::setw(4) << flight_number << "│" <<
                std::setfill(' ') << std::setw(align) << aircraft << "│" << std::setfill('0') << std::setw(2) <<
                arrival.hours << ":" << std::setw(2) << arrival.minutes << "│" << std::endl;
    }

    // Конец таблицы
    std::cout << "└" << repeat{"─", 3} << "┴" << repeat{"─", 6} << "┴" << repeat{"─", 8}
            << "┴" << repeat{"─", 17} << "┴" << repeat{"─", 5} << "┘" << std::endl;
}

/* -------------    END OF FILE main.cpp    ------------- */