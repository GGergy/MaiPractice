#include <iostream>
#include <fstream>
#include <iomanip>
#include "reader.h"


void bubble_index_sort(Line **data, const size_t shape, size_t *indexes) {
    for (size_t i = 0; i < shape - 1; i++) {
        bool has_swp = false;
        for (size_t j = 0; j < shape - i - 1; j++) {
            if (data[indexes[j]]->flight_number > data[indexes[j + 1]]->flight_number) {
                has_swp = true;
                const size_t buff = indexes[j];
                indexes[j] = indexes[j + 1];
                indexes[j + 1] = buff;
            }
        }
        if (!has_swp) break;
    }
}


void compile_table(Line **data, const size_t shape, const size_t *indexes) {
    std::cout << "#\tBort\tFlight\tAircraft\tArrival\n";
    for (size_t i = 0; i < shape; i++) {
        const Line *line = data[indexes[i]];
        printf("%llu\t\t%s%d\t\t%s%d\t\t%s\t\t%d:%d\n", i + 1, bort_prefix, line->bort_number, flight_prefix, line->flight_number,
           line->aircraft, line->arrival->hours, line->arrival->minutes);
    }
}


void print_line(const Line *line) {
    printf("Bort=`%d`; Flight=`%d`; Aircraft=`%s`; Time=`%d:%d`\n", line->bort_number, line->flight_number,
           line->aircraft, line->arrival->hours, line->arrival->minutes);
}

int main() {
    setlocale(LC_ALL, "Russian");
    const auto fname = "input.txt";
    std::ifstream input(fname);
    if (!input.is_open()) {
        std::cerr << "Failed to open " << fname << std::endl;
        return -1;
    }

    std::cout << "Ignore bad lines (fail on them if No) [Y/n]?...";
    char tmp;
    std::cin >> tmp;
    const bool skip_bad_lines = tmp == 'Y' || tmp == 'y';

    constexpr size_t buffsize = 2000;
    constexpr size_t df_size = 10000;
    char buffer[buffsize];
    const auto data = new Line *[df_size];
    for (size_t i = 0; i < df_size; i++) {
        data[i] = nullptr;
    }
    size_t good_rows = 0;
    size_t all_rows = 0;

    while (input.getline(buffer, buffsize)) {
        printf("Parsing line #%llu `%s`\n", all_rows + 1, buffer);
        Line *line = parse_line(buffer);
        if (line == nullptr) {
            std::cerr << "Failed to parse line #" << all_rows + 1 << std::endl;
            ++all_rows;
            if (skip_bad_lines) {
                continue;
            }
            free_df(data);
            return -1;
        }
        data[good_rows] = line;
        ++all_rows;
        ++good_rows;
        std::cout << "No errors detected" << std::endl;
        if (good_rows == df_size) {
            std::cerr << "DataFrame size limit exceeded. End parsing" << std::endl;
            break;
        }
    }
    std::cout << "Total parsed: " << good_rows << "/" << all_rows << std::endl;

    const auto indexes = new size_t[good_rows];
    for (size_t i = 0; i < good_rows; i++) {
        indexes[i] = i;
    }

    bubble_index_sort(data, good_rows, indexes);

    compile_table(data, good_rows, indexes);

    delete [] indexes;
    free_df(data);
}
