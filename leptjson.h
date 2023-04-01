#ifndef LEPTJSON_H
#define LEPTJSON_H

#include <string>
#include <vector>

namespace lept {

using std::string;
using std::vector;

typedef enum { NONE, FALSE, TRUE, NUMBER, STRING, ARRAY, OBJECT } e_types;

typedef struct value value;
void freeVal(value &v);

struct value {
    union {  // 不建议把具有析构函数的数据类型作为union内的成员，如string
        vector<value>* e;
        string* s;
        double n;
    };
    e_types type;
    // ~value() {
    //     freeVal(*this);
    // }
};

enum {
    PARSE_OK = 0,
    PARSE_EXPECT_VALUE,
    PARSE_INVALID_VALUE,
    PARSE_ROOT_NOT_SINGULAR,
    PARSE_NUMBER_TOO_BIG,
    PARSE_MISS_QUOTATION_MARK,
    PARSE_INVALID_STRING_ESCAPE,
    PARSE_INVALID_STRING_CHAR,
    PARSE_INVALID_UNICODE_HEX,
    PARSE_INVALID_UNICODE_SURROGATE,
    PARSE_MISS_COMMA_OR_SQUARE_BRACKET
};

#define init(v)        \
    do {               \
        v.type = NONE; \
    } while (0)

int parse(value& v, const string& strJson);

void freeVal(value& v);

e_types get_type(const value& v);

#define set_null(v) freeVal(v)

int get_boolean(const value& v);
void set_boolean(value& v, int b);

double get_number(const value& v);
void set_number(value& v, double n);

string& get_string(const value& v);
size_t get_string_length(const value& v);
void set_string(value& v, const string& s);

size_t get_array_size(const value& v);
value& get_array_element(const value& v, size_t index);
}  // namespace lept

#endif /* LEPTJSON_H */
