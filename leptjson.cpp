#include "leptjson.h"

#include <cassert> /* assert() */
#include <cmath>
#include <cstdlib>  // errno
#include <iostream>
#include <stdexcept>
#include <string>

namespace lept {

using std::string;

#define EXPECT(it, itRef)      \
    do {                       \
        assert(*it == *itRef); \
    } while (0)

typedef struct {
    string strJson;
    string::const_iterator json;
} context;

static void parse_whitespace(context &c) {
    while (*c.json == ' ' || *c.json == '\t' || *c.json == '\n' || *c.json == '\r') ++c.json;
}

static int parse_literal(context &c, value &v, const string &literal, e_types type) {
    EXPECT(c.json, literal.cbegin());
    for (auto itRef = literal.cbegin(); itRef != literal.cend(); ++itRef) {
        if (*c.json != *itRef) return PARSE_INVALID_VALUE;
        ++c.json;
    }
    v.type = type;
    return PARSE_OK;
}

#define ISDIGIT1TO9(c) (c >= '1' && c <= '9')
#define ISDIGIT(c) (c >= '0' && c <= '9')

static int parse_number(context &c, value &v) {
    auto p = c.json;
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
        v.n = std::stod(string(c.json, c.strJson.cend()));
    } catch (std::out_of_range &e) {
        return PARSE_NUMBER_TOO_BIG;
    }
    c.json = p;
    v.type = NUMBER;
    return PARSE_OK;
}

static int parse_value(context &c, value &v) {
    if (c.json == c.strJson.cend()) return PARSE_EXPECT_VALUE;
    switch (*c.json) {
        case 't':
            return parse_literal(c, v, "true", TRUE);
        case 'f':
            return parse_literal(c, v, "false", FALSE);
        case 'n':
            return parse_literal(c, v, "null", NONE);
        default:
            return parse_number(c, v);
    }
}

int parse(value &v, const string &strJson) {
    context c;
    int ret;
    // assert(v);
    c.strJson = strJson;
    c.json = c.strJson.cbegin();
    v.type = NONE;  // 若解析失败则类型为NONE
    parse_whitespace(c);
    ret = parse_value(c, v);
    if (ret == PARSE_OK) {
        parse_whitespace(c);
        if (c.json != c.strJson.cend()) {  // 字符串结尾
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
