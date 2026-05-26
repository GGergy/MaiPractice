#pragma once

#include <cstring>

constexpr size_t MAX_NAME_LENGTH = 18;

inline auto bort_prefix = "Б-";
inline auto flight_prefix = "РЕЙС";


enum ErrCode {
    ERR_BNUM_VAL, ERR_FLIGHT_VAL, ERR_NAME_LENGTH, ERR_TIME_VAL, ERR_TOK_COUNT,
    ERR_MODELS_MISMATCH, ERR_SAME_TIME, GOOD
};

typedef struct Time {
    unsigned short hours;
    unsigned short minutes;
} Time;

typedef struct Line {
    unsigned int bort_number;
    unsigned int flight_number;
    char aircraft[MAX_NAME_LENGTH];
    Time arrival;
} Line;

typedef struct {
    const char *str;
    int count;
} repeat;


std::ostream &operator<<(std::ostream &os, const repeat &r);

std::ostream &operator<<(std::ostream &os, ErrCode err);

size_t utf8_length(const char *str);

bool parse_time(char *str, Time &obj);

int parse_with_prefix(const char *prefix, const char *str);

int strtoi(const char *str);

void restore(char *str, size_t size);

ErrCode parse_line(char *str, Line &row);


inline std::ostream &operator<<(std::ostream &os, const repeat &r) {
    for (int i = 0; i < r.count; ++i) {
        os << r.str;
    }
    return os;
}


inline std::ostream &operator<<(std::ostream &os, const ErrCode err) {
    switch (err) {
        case ERR_BNUM_VAL:
            os << "Бортовой номер не соответствует шаблону Б-XXXX";
            break;
        case ERR_FLIGHT_VAL:
            os << "Номер рейса не соответствует шаблону РЕЙСXXXX";
            break;
        case ERR_NAME_LENGTH:
            os << "Модель самолета превышает допустимую длину";
            break;
        case ERR_TIME_VAL:
            os << "Время прибытия не соответствует шаблону HH:MM";
            break;
        case ERR_TOK_COUNT:
            os << "В строке некорректное количество данных (ожидается 4 столбца)";
            break;
        case ERR_MODELS_MISMATCH:
            os << "Под одним бортовым номером должна быть одна модель";
            break;
        case ERR_SAME_TIME:
            os << "Один и тот же рейс/бортовой номер не может вылетать дважды в одно время";
            break;
        case GOOD:
            os << "Строка корректна";
            break;
        default:
            os << "UNKNOWN ERROR";
            break;
    }
    return os;
}


inline size_t utf8_length(const char *str) {
    size_t length = 0;
    for (const char *c = str; *c != '\0'; c++) {
        if ((*c & 0b1100'0000) != 0b1000'0000) {
            length++;
        }
    }
    return length;
}


inline int strtoi(const char *str) {
    int res = 0;
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] >= '0' && str[i] <= '9') {
            res = res * 10 + (str[i] - '0');
        } else {
            return -1;
        }
    }
    return res;
}


inline bool parse_time(char *str, Time &obj) {
    const size_t length = strlen(str);
    if (length != 4 && length != 5) {
        return false;
    }

    const size_t delim_pos = length - 3;
    if (str[delim_pos] != ':') return false;

    str[delim_pos] = '\0';
    const int hours = strtoi(str);
    const int minutes = strtoi(str + delim_pos + 1);
    str[delim_pos] = ':';
    if (hours > 23 || minutes > 59 || hours == -1 || minutes == -1) return false;

    obj.hours = hours;
    obj.minutes = minutes;
    return true;
}


inline int parse_with_prefix(const char *prefix, const char *str) {
    if (strstr(str, prefix) != str) return -1;
    return strtoi(str + strlen(prefix));
}


inline void restore(char *str, const size_t size) {
    for (int i = 0; i < size; i++) {
        if (str[i] == '\0') {
            str[i] = ' ';
        }
    }
}


inline bool time_cmp(const Time &a, const Time &b) {
    return a.hours == b.hours && a.minutes == b.minutes;
}


inline ErrCode check_logical(const Line *data, const size_t size) {
    Line cur = data[size];
    for (const Line *line = data; line < data + size; line++) {
        if (line->bort_number == cur.bort_number && strcmp(line->aircraft, cur.aircraft) != 0) {
            return ERR_MODELS_MISMATCH;
        }
        if (line->bort_number == cur.bort_number && time_cmp(line->arrival, cur.arrival)) {
            return ERR_SAME_TIME;
        }
        if (line->flight_number == cur.flight_number && time_cmp(line->arrival, cur.arrival)) {
            return ERR_SAME_TIME;
        }
    }
    return GOOD;
}


inline ErrCode parse_line(char *str, Line &row) {
    const auto delim = " \t";

    const char *tok_bnum = strtok(str, delim);
    if (tok_bnum == nullptr) {
        return ERR_TOK_COUNT;
    }
    if (strlen(tok_bnum) != strlen(bort_prefix) + 4) {
        return ERR_BNUM_VAL;
    }
    const int bnum_val = parse_with_prefix(bort_prefix, tok_bnum);
    if (bnum_val == -1) {
        return ERR_BNUM_VAL;
    }
    row.bort_number = bnum_val;

    const char *tok_flight = strtok(nullptr, delim);
    if (tok_flight == nullptr) {
        return ERR_TOK_COUNT;
    }
    if (strlen(tok_flight) != strlen(flight_prefix) + 4) {
        return ERR_FLIGHT_VAL;
    }
    const int flight_val = parse_with_prefix(flight_prefix, tok_flight);
    if (flight_val == -1) {
        return ERR_FLIGHT_VAL;
    }
    row.flight_number = flight_val;

    const char *tok_model = strtok(nullptr, delim);
    if (tok_model == nullptr) {
        return ERR_TOK_COUNT;
    }
    if (strlen(tok_model) >= MAX_NAME_LENGTH) {
        return ERR_NAME_LENGTH;
    }
    strcpy(row.aircraft, tok_model);

    char *tok_time = strtok(nullptr, delim);
    if (tok_time == nullptr) {
        return ERR_TOK_COUNT;
    }
    if (strtok(nullptr, delim) != nullptr) {
        return ERR_TOK_COUNT;
    }

    if (!parse_time(tok_time, row.arrival)) {
        return ERR_TIME_VAL;
    }

    return GOOD;
}
