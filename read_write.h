#pragma once

#include <cstring>


constexpr size_t MAX_NAME_LENGTH = 18; // Максимальный размер названия модели самолёта

auto bort_prefix = "Б-"; // Начало бортового номера
auto flight_prefix = "РЕЙС"; // Начало номера рейса


// Коды ошибок
enum ErrCode {
    ERR_BNUM_VAL, // Некорректный бортовой номер
    ERR_FLIGHT_VAL, // Некорректный номера рейса
    ERR_NAME_LENGTH, // Слишком длинное имя модели
    ERR_TIME_VAL, // Некорректное время
    ERR_TOK_COUNT, // Некорректное число токенов в строке (не 4 - борт номер; номер рейса; модель; время)
    ERR_MODELS_MISMATCH, // Логическая ошибка несоответствия моделей под одним борт номером
    ERR_SAME_TIME, // Логическая ошибка вылета одного номера дважды за одно время
    GOOD // Нет ошибки
};

// Структура для хранения времени
typedef struct Time {
    unsigned short hours;
    unsigned short minutes;
} Time;

// Структура для хранения записи АСУ ВД
typedef struct Line {
    unsigned int bort_number; // Бортовой номер
    unsigned int flight_number; // Номер рейса
    char aircraft[MAX_NAME_LENGTH]; // Модель ЛА
    Time arrival; // Время захода в зону
} Line;

// Структура для вывода строки str count раз
typedef struct {
    const char *str;
    int count;
} repeat;


// Вывод строки структуры repeat count раз (замена std::setfill для псевдографики)
std::ostream &operator<<(std::ostream &os, const repeat &r);

// Вывод ошибки по коду
std::ostream &operator<<(std::ostream &os, ErrCode err);

// Подсчет длины последовательности Unicode символов с учётом их многобайтовости
size_t utf8_length(const char *str);

// Обработка времени
bool parse_time(char *str, Time &obj);

// Обработка числа с префиксом (bort_prefix и flight_prefix)
int parse_with_prefix(const char *prefix, const char *str, bool check_lead_zero = false);

// Парсинг неотрицательного целого числа из строки
int strtoi(const char *str, bool check_lead_zero = false);

// Проверка равенства времени
bool time_cmp(const Time &a, const Time &b);

// Восстановление строки после обработки strtok (убирает лишние \0)
void restore(char *str, size_t size);

// Проверка данных на логические ошибки
ErrCode check_logical(const Line *data, size_t size);

// Обработка строки данных
ErrCode parse_line(char *str, Line &row);


// Вывод строки структуры repeat count раз (замена std::setfill для псевдографики)
inline std::ostream &operator<<(std::ostream &os, const repeat &r) {
    for (int i = 0; i < r.count; ++i) {
        os << r.str;
    }
    return os;
}


// Вывод ошибки по коду
inline std::ostream &operator<<(std::ostream &os, const ErrCode err) {
    switch (err) {
        case ERR_BNUM_VAL:
            os << "Бортовой номер не соответствует шаблону Б-XXXX";
            break;
        case ERR_FLIGHT_VAL:
            os << "Номер рейса не соответствует шаблону РЕЙС{DIGIT<10**9}";
            break;
        case ERR_NAME_LENGTH:
            os << "Модель самолета превышает допустимую длину";
            break;
        case ERR_TIME_VAL:
            os << "Время прибытия не соответствует шаблону HH:MM и H:MM";
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


// Подсчет длины последовательности Unicode символов с учётом их многобайтовости
inline size_t utf8_length(const char *str) {
    /*
    * 0xxxxxxx для однобайтового символа, 110xxxxx для 2, 1110xxxx для 3, 11110xxx для 4.
    * При этом второй и последующий байты всегда имеют вид 10xxxxxx
    * Функция выявляет такие биты с помощью маски 11000000 - не первый байт даст 10000000
    * (первые два справа разряда совпадут с интересующим байтом, остальные нули)
    */
    size_t length = 0;
    for (const char *c = str; *c != '\0'; c++) {
        if ((*c & 0b1100'0000) != 0b1000'0000) {
            length++;
        }
    }
    return length;
}


// Парсинг неотрицательного целого числа из строки
// check_lead_zero - проверка на ведущий 0, по умолчанию отключена
inline int strtoi(const char *str, const bool check_lead_zero) {
    int res = 0;
    const bool lead_zero = str[0] == '0';
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] >= '0' && str[i] <= '9') {
            if (check_lead_zero && lead_zero && i > 0) return -1;
            res = res * 10 + (str[i] - '0');
        } else {
            // Возврат -1, если встретился символ не из 0-9, так как для корректного числа функция вернет res >= 0
            return -1;
        }
    }
    return res;
}


// Обработка времени
inline bool parse_time(char *str, Time &obj) {
    const size_t length = strlen(str);
    if (length != 4 && length != 5) {
        // Время может быть длины 4, если записано в формате H:MM, или 5 в формате HH:MM. Оба формата корректны
        return false;
    }

    // Двоеточие всегда идет перед MM, то есть length - 3
    const size_t delim_pos = length - 3;
    // Если не встретилось двоеточие на ожидаемом месте - ошибка
    if (str[delim_pos] != ':') return false;

    str[delim_pos] = '\0'; // Ставим временный разделитель между минутами и часами
    const int hours = strtoi(str); // Парсинг минут
    const int minutes = strtoi(str + delim_pos + 1); // Парсинг часов
    str[delim_pos] = ':'; // Возврат ':' на место
    // Проверка на корректность считанного времени (или -1, если парсинг не удался)
    if (hours > 23 || minutes > 59 || hours == -1 || minutes == -1) return false;

    // Инициализация полей структуры
    obj.hours = hours;
    obj.minutes = minutes;
    return true;
}


// Обработка числа с префиксом (bort_prefix и flight_prefix)
inline int parse_with_prefix(const char *prefix, const char *str, const bool check_lead_zero) {
    // Поиск нужного префикса в начале строки. Не находим - ошибка
    if (strstr(str, prefix) != str) return -1;
    // Число будет лежать сдвинутым на префикс
    return strtoi(str + strlen(prefix), check_lead_zero);
}


// Восстановление строки после обработки strtok (убирает лишние \0)
inline void restore(char *str, const size_t size) {
    // Заменяем все \0 обратно на пробел кроме последнего (после size)
    for (size_t i = 0; i < size; i++) {
        if (str[i] == '\0') {
            str[i] = ' ';
        }
    }
}


// Проверка равенства времени
inline bool time_cmp(const Time &a, const Time &b) {
    // Время А равно времени Б тогда и только тогда, когда равны между собой их часы и минуты
    return a.hours == b.hours && a.minutes == b.minutes;
}


// Проверка данных на логические ошибки
inline ErrCode check_logical(const Line *data, const size_t size) {
    Line cur = data[size]; // Берем строку с конца массива и делаем проверки с каждой предыдущей
    for (const Line *line = data; line < data + size; line++) {
        // Логическая ошибка несоответствия моделей под одним борт номером
        if (line->bort_number == cur.bort_number && strcmp(line->aircraft, cur.aircraft) != 0) {
            return ERR_MODELS_MISMATCH;
        }
        // Логическая ошибка вылета одного борт номера дважды за одно время
        if (line->bort_number == cur.bort_number && time_cmp(line->arrival, cur.arrival)) {
            return ERR_SAME_TIME;
        }
        // Логическая ошибка вылета одного номера рейса дважды за одно время
        if (line->flight_number == cur.flight_number && time_cmp(line->arrival, cur.arrival)) {
            return ERR_SAME_TIME;
        }
    }
    // Если нет ошибок, вернем GOOD
    return GOOD;
}


// Обработка строки данных
inline ErrCode parse_line(char *str, Line &row) {
    const auto delim = " \t"; // Разделители между токенами - пробел и таб

    const char *tok_bnum = strtok(str, delim); // Токен бортового номера
    if (tok_bnum == nullptr) {
        // Проверка на наличие, если нет - ошибка в объеме данных
        return ERR_TOK_COUNT;
    }
    if (strlen(tok_bnum) != strlen(bort_prefix) + 4) {
        // Проверка на размер - длина префикса + 4 цифры
        return ERR_BNUM_VAL;
    }
    const int bnum_val = parse_with_prefix(bort_prefix, tok_bnum); // Парсинг числа
    if (bnum_val == -1) {
        // Обработка ошибки парсинга
        return ERR_BNUM_VAL;
    }
    // Инициализация поля структуры
    row.bort_number = bnum_val;

    // То же самое для номера рейса
    const char *tok_flight = strtok(nullptr, delim);
    if (tok_flight == nullptr) {
        return ERR_TOK_COUNT;
    }
    if (strlen(tok_flight) <= strlen(flight_prefix) || strlen(tok_flight) > strlen(flight_prefix) + 9) {
        return ERR_FLIGHT_VAL;
    }
    const int flight_val = parse_with_prefix(flight_prefix, tok_flight, true);
    if (flight_val == -1) {
        return ERR_FLIGHT_VAL;
    }
    row.flight_number = flight_val;

    const char *tok_model = strtok(nullptr, delim); // Токен модели ЛА
    if (tok_model == nullptr) {
        return ERR_TOK_COUNT;
    }
    if (strlen(tok_model) >= MAX_NAME_LENGTH) {
        // Проверка на длину
        return ERR_NAME_LENGTH;
    }
    strcpy(row.aircraft, tok_model); // Инициализация через strcpy

    char *tok_time = strtok(nullptr, delim); // Токен времени посадки
    if (tok_time == nullptr) {
        return ERR_TOK_COUNT;
    }
    if (strtok(nullptr, delim) != nullptr) {
        // Если остались еще токены после всех обработок - ошибка в объеме данных (перебор)
        return ERR_TOK_COUNT;
    }

    // Обработка времени
    if (!parse_time(tok_time, row.arrival)) {
        return ERR_TIME_VAL;
    }

    // Если все прошло успешно - возврат GOOD
    return GOOD;
}
