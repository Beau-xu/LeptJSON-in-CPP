#include <cassert> /* assert() */
// #include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "leptjson.h"

namespace lept {

using std::string;

#define EXPECT(c, ch)   \
    do {                \
        assert(c == c); \
    } while (0)

#define ISDIGIT1TO9(c) (c >= '1' && c <= '9')
#define ISDIGIT(c) (c >= '0' && c <= '9')

typedef struct {
    string strJson;
    string::const_iterator json;
} context;

static void parse_whitespace(context &c) {
    while (*c.json == ' ' || *c.json == '\t' || *c.json == '\n' || *c.json == '\r') ++c.json;
}

static int parse_literal(context &c, value &v, const string &literal, e_types type) {
    EXPECT(*c.json, literal[0]);
    for (auto itRef = literal.cbegin(); itRef != literal.cend(); ++itRef) {
        if (*c.json != *itRef) return PARSE_INVALID_VALUE;
        ++c.json;
    }
    v.type = type;
    return PARSE_OK;
}

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

static int parse_hex4(string::const_iterator &end, unsigned &u) {
    int i;
    u = 0;
    for (i = 0; i < 4; i++) {
        char ch = *end++;
        u <<= 4;  // *= 16;
        if (ch >= '0' && ch <= '9')
            u |= ch - '0';
        else if (ch >= 'A' && ch <= 'F')
            u |= ch - ('A' - 10);
        else if (ch >= 'a' && ch <= 'f')
            u |= ch - ('a' - 10);
        else
            return NULL;
    }
}

static void encode_utf8(string &s, unsigned u) {
    if (u <= 0x7F)
        s += u & 0xFF;
    else if (u <= 0x7FF) {
        s += 0xC0 | ((u >> 6) & 0xFF);
        s += 0x80 | (u & 0x3F);
    } else if (u <= 0xFFFF) {
        s += 0xE0 | ((u >> 12) & 0xFF);
        s += 0x80 | ((u >> 6) & 0x3F);
        s += 0x80 | (u & 0x3F);
    } else {
        assert(u <= 0x10FFFF);
        s += 0xF0 | ((u >> 18) & 0xFF);
        s += 0x80 | ((u >> 12) & 0x3F);
        s += 0x80 | ((u >> 6) & 0x3F);
        s += 0x80 | (u & 0x3F);
    }
}

static int parse_string(context &c, value &v) {
    EXPECT(*c.json, '\"');
    auto end = ++(c.json);
    char ch;
    string s = "";
    while (end != c.strJson.cend()) {
        ch = *end++;
        unsigned u, u2;
        switch (ch) {
            case '\"':
                c.json = end;
                set_string(v, s);
                return PARSE_OK;
            case '\\':
                switch (*end++) {
                    case '\"':
                        s += '\"';
                        break;
                    case '\\':
                        s += '\\';
                        break;
                    case '/':
                        s += '/';
                        break;
                    case 'b':
                        s += '\b';
                        break;
                    case 'f':
                        s += '\f';
                        break;
                    case 'n':
                        s += '\n';
                        break;
                    case 'r':
                        s += '\r';
                        break;
                    case 't':
                        s += '\t';
                        break;
                    case 'u':
                        if (!(parse_hex4(end, u))) return PARSE_INVALID_UNICODE_HEX;
                        if (u >= 0xD800 && u <= 0xDBFF) { /* surrogate pair */
                            if (*end++ != '\\') return PARSE_INVALID_UNICODE_SURROGATE;
                            if (*end++ != 'u') return PARSE_INVALID_UNICODE_SURROGATE;
                            if (!(parse_hex4(end, u2))) return PARSE_INVALID_UNICODE_HEX;
                            if (u2 < 0xDC00 || u2 > 0xDFFF) return PARSE_INVALID_UNICODE_SURROGATE;
                            u = (((u - 0xD800) << 10) | (u2 - 0xDC00)) + 0x10000;
                        }
                        encode_utf8(s, u);
                        break;
                    default:
                        return PARSE_INVALID_STRING_ESCAPE;
                }
                break;
            default:
                if ((unsigned char)ch < 0x20) {
                    return PARSE_INVALID_STRING_CHAR;
                }
                s += ch;
        }
    }
    return PARSE_MISS_QUOTATION_MARK;
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
        case '"':
            return parse_string(c, v);
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
    init(v);
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

void freeVal(value &v) {
    if (v.type == STRING) {
        delete v.s;
        v.s = nullptr;
    }
    v.type = NONE;
}

e_types get_type(const value &v) { return v.type; }

int get_boolean(const value &v) {
    assert(v.type == TRUE || v.type == FALSE);
    return v.type == TRUE;
}

void set_boolean(value &v, int b) {
    freeVal(v);
    v.type = b ? TRUE : FALSE;
}

double get_number(const value &v) {
    assert(v.type == NUMBER);
    return v.n;
}

void set_number(value &v, double n) {
    freeVal(v);
    v.n = n;
    v.type = NUMBER;
}

string &get_string(const value &v) {
    assert(v.type == STRING);
    return *v.s;
}

size_t get_string_length(const value &v) {
    assert(v.type == STRING);
    return (*v.s).size();
}

void set_string(value &v, const string &s) {
    freeVal(v);
    v.s = new string(s);
    v.type = STRING;
}

}  // namespace lept
