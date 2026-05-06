#pragma once

#include <cstring>

constexpr size_t MAX_NAME_LENGTH = 15;
constexpr size_t MAX_FPREFIX_LENGTH = 5;
constexpr size_t MAX_BPREFIX_LENGTH = 4;


typedef struct Time {
    unsigned short hours;
    unsigned short minutes;
} Time;

typedef struct Line {
    char bort_prefix[MAX_BPREFIX_LENGTH];
    unsigned int bort_number;
    char flight_prefix[MAX_FPREFIX_LENGTH];
    unsigned int flight_number;
    char aircraft[MAX_NAME_LENGTH];
    Time arrival;
} Line;


inline int strtoi(const char *str) {
    int res = 0;
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] >= '0' && str[i] <= '9') {
            res = res * 10 + (str[i] - '0');
        } else {
            std::cerr << "Integer parsing: invalid number '" << str << "'" << std::endl;
            return -1;
        }
    }
    return res;
}


inline bool parse_time(char *str, Time &obj) {
    const char *tok_h = strtok(str, ":");
    if (tok_h == nullptr) {
        std::cerr << "Could not parse hours" << std::endl;
        return false;
    }
    const char *tok_m = strtok(nullptr, ":");
    if (tok_m == nullptr) {
        std::cerr << "Could not parse minutes" << std::endl;
        return false;
    }
    if (strtok(nullptr, ":") != nullptr) {
        std::cerr << "Too many `:` in time" << std::endl;
        return false;
    }
    const int hours = strtoi(tok_h);
    const int minutes = strtoi(tok_m);
    if (hours < 0 || minutes < 0 || hours > 23 || minutes > 59) {
        std::cerr << "Invalid time '" << hours << ":" << minutes << "'" << std::endl;
        return false;
    }

    obj.hours = hours;
    obj.minutes = minutes;
    return true;
}


inline bool parse_flight_num(const char *str, Line &obj) {
    const size_t delim_pos = strcspn(str, "0123456789");
    if (delim_pos >= MAX_FPREFIX_LENGTH || delim_pos >= strlen(str)) {
        return false;
    }
    const int fnum = strtoi(str + delim_pos);
    if (fnum == -1) {
        return false;
    }
    obj.flight_number = fnum;
    for (int i = 0; i < delim_pos; i++) {
        obj.flight_prefix[i] = str[i];
    }
    obj.flight_prefix[delim_pos] = '\0';

    return true;
}


inline bool parse_bort_num(const char *str, Line &obj) {
    const size_t delim_pos = strcspn(str, "-");
    if (delim_pos >= MAX_BPREFIX_LENGTH || delim_pos >= strlen(str) - 1) {
        return false;
    }
    const int bnum = strtoi(str + delim_pos + 1);
    if (bnum == -1) {
        return false;
    }
    obj.bort_number = bnum;
    for (int i = 0; i < delim_pos; i++) {
        obj.bort_prefix[i] = str[i];
    }
    obj.bort_prefix[delim_pos] = '\0';

    return true;
}


inline bool parse_line(char *str, Line &row) {
    const auto delim = " \t";

    const char *tok_bnum = strtok(str, delim);
    if (tok_bnum == nullptr) {
        std::cerr << "Bort number is not specified!" << std::endl;
        return false;
    }
    if (!parse_bort_num(tok_bnum, row)) {
        std::cerr << "Bort number is invalid" << std::endl;
        return false;
    }

    const char *tok_flight = strtok(nullptr, delim);
    if (tok_flight == nullptr) {
        std::cerr << "Flight number is not specified!" << std::endl;
        return false;
    }
    if (!parse_flight_num(tok_flight, row)) {
        std::cerr << "Flight number is invalid!" << std::endl;
        return false;
    }

    const char *tok_model = strtok(nullptr, delim);
    if (tok_model == nullptr) {
        std::cerr << "Aircraft model is not specified!" << std::endl;
        return false;
    }
    const size_t name_size = strlen(tok_model);
    std::cout << name_size << std::endl;
    if (name_size >= MAX_NAME_LENGTH) {
        std::cerr << "Model name is too long!" << std::endl;
        return false;
    }
    for (int i = 0; i < name_size; i++) {
        row.aircraft[i] = tok_model[i];
    }
    row.aircraft[name_size] = '\0';

    char *tok_time = strtok(nullptr, delim);
    if (tok_time == nullptr) {
        std::cerr << "Arrival time is not specified!" << std::endl;
        return false;
    }
    if (strtok(nullptr, delim) != nullptr) {
        std::cerr << "Too many rows in line!" << std::endl;
        return false;
    }

    if (!parse_time(tok_time, row.arrival)) {
        return false;
    }

    return true;
}
