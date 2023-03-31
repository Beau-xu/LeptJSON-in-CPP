#include "leptjson.h"

#include <cassert> /* assert() */
#include <cmath>
#include <cstdlib>  // errno
#include <stdexcept>
#include <string>

namespace lept {

using std::string;

#define EXPECT(c, ch)            \
    do {                         \
        assert(*c.json == (ch)); \
        c.json++;                \
    } while (0)

typedef struct {
    const char *json;
} context;

static void parse_whitespace(context &c) {
    const char *p = c.json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++;
    c.json = p;
}

static int parse_literal(context &c, value &v, const char *literal, e_types type) {
    EXPECT(c, literal[0]);
    size_t i;
    for (i = 0; literal[i + 1]; ++i) {  // literal[i + 1] == '\0'为止
        if (c.json[i] != literal[i + 1]) return PARSE_INVALID_VALUE;
    }
    c.json += i;
    v.type = type;
    return PARSE_OK;
}

#define ISDIGIT1TO9(c) (c >= '1' && c <= '9')
#define ISDIGIT(c) (c >= '0' && c <= '9')

static int parse_number(context &c, value &v) {
    const char *p = c.json;
    if (*p == '-') ++p;
    if (*p == '0') {
        ++p;
    } else {
        if (!ISDIGIT1TO9(*p)) return PARSE_INVALID_VALUE;
        while (ISDIGIT(*p)) ++p;
    }
    if (*p == '.') {
        ++p;
        if (!ISDIGIT(*p)) return PARSE_INVALID_VALUE;
        while (ISDIGIT(*p)) ++p;
    }
    if (*p == 'e' || *p == 'E') {
        ++p;
        if (*p == '+' || *p == '-') ++p;
        if (!ISDIGIT(*p)) return PARSE_INVALID_VALUE;
        while (ISDIGIT(*p)) ++p;
    }
    try {
        v.n = strtod(c.json, NULL);
    } catch (std::out_of_range &e) {
        return PARSE_NUMBER_TOO_BIG;
    }
    c.json = p;
    v.type = NUMBER;
    return PARSE_OK;
}

static int parse_value(context &c, value &v) {
    switch (*c.json) {
        case 't':
            return parse_literal(c, v, "true", TRUE);
        case 'f':
            return parse_literal(c, v, "false", FALSE);
        case 'n':
            return parse_literal(c, v, "null", NONE);
        default:
            return parse_number(c, v);
        case '\0':
            return PARSE_EXPECT_VALUE;
    }
}

int parse(value &v, const char *json) {
    context c;
    int ret;
    // assert(v);
    c.json = json;
    v.type = NONE;  // 若解析失败则类型为NONE
    parse_whitespace(c);
    ret = parse_value(c, v);
    if (ret == PARSE_OK) {
        parse_whitespace(c);
        if (*c.json != '\0') {  // 字符串结尾
            v.type = NONE;
            ret = PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

e_types get_type(const value &v) {
    // assert(v);
    return v.type;
}

double get_number(const value &v) {
    assert(v.type == NUMBER);
    return v.n;
}

}  // namespace lept
