#include <iostream>
#include <fstream>
#include <iomanip>
#include "reader.h"


void bubble_index_sort(const Line *data, const size_t shape, size_t *indexes) {
    for (size_t i = 0; i < shape - 1; i++) {
        bool has_swp = false;
        for (size_t j = 0; j < shape - i - 1; j++) {
            if (data[indexes[j]].flight_number > data[indexes[j + 1]].flight_number) {
                has_swp = true;
                const size_t buff = indexes[j];
                indexes[j] = indexes[j + 1];
                indexes[j + 1] = buff;
            }
        }
        if (!has_swp) break;
    }
}


void concat(const char *prefix, unsigned int value, char *dest, const char sep) {
    size_t pos = 0;
    for (; prefix[pos] != '\0'; pos++) {
        dest[pos] = prefix[pos];
    }
    if (sep != '\0') {
        dest[pos] = sep;
        pos++;
    }
    unsigned int div = 1;
    while (value / div >= 10) {
        div *= 10;
    }
    while (div) {
        dest[pos] = '0' + value / div;
        value %= div;
        div /= 10;
        ++pos;
    }
    dest[pos] = '\0';
}


void time_to_str(const Time &time, char *dest) {
    dest[0] = '0' + time.hours / 10;
    dest[1] = '0' + time.hours % 10;
    dest[2] = ':';
    dest[3] = '0' + time.minutes / 10;
    dest[4] = '0' + time.minutes % 10;
    dest[5] = '\0';
}


void compile_table(const Line *data, const size_t shape, const size_t *indexes) {
    std::cout << std::setw(4) << "#" << std::setw(16) << "Bort" << std::setw(16) << "Flight" << std::setw(15) <<
            "Aircraft" << std::setw(9) << "Arrival" << std::endl;
    for (size_t i = 0; i < shape; i++) {
        const Line line = data[indexes[i]];
        char bort_num[MAX_BPREFIX_LENGTH + 11];
        char flight_num[MAX_FPREFIX_LENGTH + 11];
        char time[6];
        concat(line.bort_prefix, line.bort_number, bort_num, '-');
        concat(line.flight_prefix, line.flight_number, flight_num, '\0');
        time_to_str(line.arrival, time);

        std::cout << std::setw(4) << i + 1 << std::setw(16) << bort_num << std::setw(16) << flight_num << std::setw(15) <<
            line.aircraft << std::setw(9) << time << std::endl;
    }
}


int main() {
    std::cout << "BOOT" << std::endl;
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

    constexpr size_t buffsize = 512;
    constexpr size_t df_size = 1000;
    char buffer[buffsize];
    Line data[df_size];
    size_t good_rows = 0;
    size_t all_rows = 0;

    while (input.getline(buffer, buffsize)) {
        printf("Parsing line #%llu `%s`\n", all_rows + 1, buffer);
        if (!parse_line(buffer, data[good_rows])) {
            std::cerr << "Failed to parse line #" << all_rows + 1 << std::endl;
            ++all_rows;
            if (skip_bad_lines) {
                continue;
            }
            return -1;
        }
        ++all_rows;
        ++good_rows;
        std::cout << "No errors detected" << std::endl;
        if (good_rows == df_size) {
            std::cerr << "DataFrame size limit exceeded. End parsing" << std::endl;
            break;
        }
    }
    std::cout << "Total parsed: " << good_rows << "/" << all_rows << std::endl;
    if (good_rows == 0) {
        std::cerr << "No good data found" << std::endl;
        return -1;
    }

    size_t indexes[df_size];
    for (size_t i = 0; i < good_rows; i++) {
        indexes[i] = i;
    }

    bubble_index_sort(data, good_rows, indexes);

    compile_table(data, good_rows, indexes);

}
