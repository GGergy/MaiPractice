#pragma once

#include <cstring>

inline auto bort_prefix = "а-";
inline auto flight_prefix = "пеия";

typedef struct {
    unsigned short hours;
    unsigned short minutes;
} Time;

typedef struct {
    unsigned int bort_number;
    unsigned int flight_number;
    const char *aircraft;
    Time *arrival;
} Line;


inline void free_df(Line **data) {
    for (int i = 0; data[i] != nullptr; i++) {
        delete data[i]->arrival;
        delete data[i];
    }
    delete [] data;
}


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


inline Time *parse_time(char *str) {
    const char *tok_h = strtok(str, ":");
    if (tok_h == nullptr) {
        std::cerr << "Could not parse hours" << std::endl;
        return nullptr;
    }
    const char *tok_m = strtok(nullptr, ":");
    if (tok_m == nullptr) {
        std::cerr << "Could not parse minutes" << std::endl;
        return nullptr;
    }
    if (strtok(nullptr, ":") != nullptr) {
        std::cerr << "Too many `:` in time" << std::endl;
        return nullptr;
    }
    const int hours = strtoi(tok_h);
    const int minutes = strtoi(tok_m);
    if (hours < 0 || minutes < 0 || hours > 23 || minutes > 59) {
        std::cerr << "Invalid time '" << hours << ":" << minutes << "'" << std::endl;
        return nullptr;
    }

    const auto result = new Time;
    result->hours = hours;
    result->minutes = minutes;
    return result;
}

inline int parse_with_prefix(const char *prefix, const char *str) {
    if (strstr(str, prefix) != str) { return -1; }
    return strtoi(str + strlen(prefix));
}


inline Line *parse_line(char *str) {
    const auto delim = " \t";

    const auto result = new Line;

    const char *tok_bnum = strtok(str, delim);
    if (tok_bnum == nullptr) {
        std::cerr << "Bort number is not specified!" << std::endl;
        delete result;
        return nullptr;
    }
    const int bnum = parse_with_prefix(bort_prefix, tok_bnum);
    if (bnum == -1) {
        std::cerr << "Bort number is invalid" << std::endl;
        delete result;
        return nullptr;
    }
    result->bort_number = bnum;

    const char *tok_flight = strtok(nullptr, delim);
    if (tok_flight == nullptr) {
        std::cerr << "Flight number is not specified!" << std::endl;
        delete result;
        return nullptr;
    }
    const int flight = parse_with_prefix(flight_prefix, tok_flight);
    if (flight == -1) {
        std::cerr << "Flight number is invalid!" << std::endl;
        delete result;
        return nullptr;
    }
    result->flight_number = flight;

    const char *tok_model = strtok(nullptr, delim);
    if (tok_model == nullptr) {
        std::cerr << "Aircraft model is not specified!" << std::endl;
        delete result;
        return nullptr;
    }
    const auto alloc = new char[strlen(tok_model) + 1];
    strcpy(alloc, tok_model);
    result->aircraft = alloc;

    char *tok_time = strtok(nullptr, delim);
    if (tok_time == nullptr) {
        std::cerr << "Arrival time is not specified!" << std::endl;
        delete result;
        return nullptr;
    }
    if (strtok(nullptr, delim) != nullptr) {
        delete result;
        std::cerr << "Too many rows in line!" << std::endl;
        return nullptr;
    }

    Time *time = parse_time(tok_time);
    if (time == nullptr) {
        delete result;
        delete time;
        return nullptr;
    }
    result->arrival = time;

    return result;
}
