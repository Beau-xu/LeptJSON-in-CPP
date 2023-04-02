#ifndef LEPTJSON_H
#define LEPTJSON_H

#include <string>
#include <vector>

namespace lept {

using std::string;
using std::vector;

typedef enum { NONE, FALSE, TRUE, NUMBER, STRING, ARRAY, OBJECT } e_types;

typedef struct value value;
typedef struct member member;

struct value {
    union {  // 不建议把具有析构函数的数据类型作为union内的成员，如string
        vector<member>* m; /* object elements */
        vector<value>* e;  /* array elements */
        string* s;         /* string elements */
        double n;          /* number */
    };
    e_types type;
};

struct member {
    string k; /* member key string */
    value v;  /* member value */
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
    PARSE_MISS_COMMA_OR_SQUARE_BRACKET,
    PARSE_MISS_KEY,
    PARSE_MISS_COLON,
    PARSE_MISS_COMMA_OR_CURLY_BRACKET
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

size_t get_object_size(const value& v);
const string& get_object_key(const value& v, size_t index);
size_t get_object_key_length(const value& v, size_t index);
value& get_object_value(const value& v, size_t index);

}  // namespace lept

#endif /* LEPTJSON_H */
