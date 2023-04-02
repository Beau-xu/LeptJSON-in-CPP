#include <cassert> /* assert() */
// #include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "leptjson.h"

namespace lept {

using std::string;

#define EXPECT(c, char)    \
    do {                   \
        assert(c == char); \
    } while (0)

#define ISDIGIT1TO9(c) (c >= '1' && c <= '9')
#define ISDIGIT(c) (c >= '0' && c <= '9')

typedef struct {
    string strJson;
    string::const_iterator json;
} context;

static void parse_whitespace(context& c) {
    while (*c.json == ' ' || *c.json == '\t' || *c.json == '\n' || *c.json == '\r') ++c.json;
}

static int parse_literal(context& c, value& v, const string& literal, e_types type) {
    EXPECT(*c.json, literal[0]);
    for (auto itRef = literal.cbegin(); itRef != literal.cend(); ++itRef) {
        if (*c.json != *itRef) return PARSE_INVALID_VALUE;
        ++c.json;
    }
    v.type = type;
    return PARSE_OK;
}

static int parse_number(context& c, value& v) {
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
    } catch (std::out_of_range& e) {
        return PARSE_NUMBER_TOO_BIG;
    }
    c.json = p;
    v.type = NUMBER;
    return PARSE_OK;
}

static bool parse_hex4(string::const_iterator& end, unsigned& u) {
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
            return false;
    }
    return true;
}

static void encode_utf8(string& s, unsigned u) {
    // 与运算将二进制填充至8位（补0），或运算将前缀改为UTF-8要求（10,110,1110,11110）
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

static int parse_string_raw(context& c, string& s) {
    EXPECT(*c.json, '\"');
    auto end = ++(c.json);
    char ch;
    while (end != c.strJson.cend()) {
        ch = *end++;
        unsigned u, u2;
        switch (ch) {
            case '\"':
                c.json = end;  // 收引号的下一位
                return PARSE_OK;
            case '\\':
                switch (*end++) {
                    case '\"': s += '\"'; break;
                    case '\\': s += '\\'; break;
                    case '/': s += '/'; break;
                    case 'b': s += '\b'; break;
                    case 'f': s += '\f'; break;
                    case 'n': s += '\n'; break;
                    case 'r': s += '\r'; break;
                    case 't': s += '\t'; break;
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
                    default: return PARSE_INVALID_STRING_ESCAPE;
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

static int parse_string(context& c, value& v) {
    int ret;
    string s("");
    if ((ret = parse_string_raw(c, s)) == PARSE_OK) set_string(v, s);
    return ret;
}

static int parse_value(context& c, value& v);

static int parse_array(context& c, value& v) {
    EXPECT(*(c.json)++, '[');
    parse_whitespace(c);
    if (*c.json == ']') {
        c.json++;
        v.type = ARRAY;
        v.e = nullptr;
        return PARSE_OK;
    }
    int ret;
    vector<value> vecVal;
    while (true) {
        value val;
        init(val);
        if ((ret = parse_value(c, val)) != PARSE_OK) break;
        vecVal.push_back(val);
        parse_whitespace(c);
        if (*c.json == ',') {
            c.json++;
            parse_whitespace(c);
        } else if (*c.json == ']') {
            c.json++;
            v.type = ARRAY;
            v.e = new vector<value>(vecVal);
            return PARSE_OK;  // 直接返回且不释放 vecVal
            break;
        } else {
            ret = PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
            break;
        }
    }
    for (auto val : vecVal) freeVal(val);
    return ret;
}

static int parse_object(context& c, value& v) {
    EXPECT(*(c.json++), '{');
    parse_whitespace(c);
    if (*c.json == '}') {
        c.json++;
        v.type = OBJECT;
        v.m = nullptr;
        return PARSE_OK;
    }
    int ret;
    vector<member> vecMem;
    while (true) {
        member mem;
        mem.k = string("");
        init(mem.v);
        if (*c.json != '\"') {
            ret = PARSE_MISS_KEY;
            break;
        }
        if ((ret = parse_string_raw(c, mem.k) != PARSE_OK)) break;
        parse_whitespace(c);
        if (*(c.json++) != ':') {
            ret = PARSE_MISS_COLON;
            break;
        }
        parse_whitespace(c);
        if ((ret = parse_value(c, mem.v)) != PARSE_OK) break;
        vecMem.push_back(mem);
        // mem.k = string(""); /* ownership is transferred to member on stack */
        parse_whitespace(c);
        if (*c.json == ',') {
            c.json++;
            parse_whitespace(c);
        } else if (*c.json == '}') {
            c.json++;
            v.type = OBJECT;
            v.m = new vector<member>(vecMem);
            return PARSE_OK;
        } else {
            ret = PARSE_MISS_COMMA_OR_CURLY_BRACKET;
            break;
        }
    }
    for (auto mem : vecMem) freeVal(mem.v);
    return ret;
}

static int parse_value(context& c, value& v) {
    if (c.json == c.strJson.cend()) return PARSE_EXPECT_VALUE;
    switch (*c.json) {
        case 't': return parse_literal(c, v, "true", TRUE);
        case 'f': return parse_literal(c, v, "false", FALSE);
        case 'n': return parse_literal(c, v, "null", NONE);
        case '"': return parse_string(c, v);
        case '[': return parse_array(c, v);
        case '{': return parse_object(c, v);
        default: return parse_number(c, v);
    }
}

int parse(value& v, const string& strJson) {
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

static void stringify_string(const string& sOfVal, string& s) {
    static const char hex_digits[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                                      '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    s += '"';
    for (auto it = sOfVal.cbegin(); it != sOfVal.cend(); it++) {
        switch (*it) {
            case '\"':
                s += '\\';
                s += '\"';
                break;
            case '\\':
                s += '\\';
                s += '\\';
                break;
            case '\b':
                s += '\\';
                s += 'b';
                break;
            case '\f':
                s += '\\';
                s += 'f';
                break;
            case '\n':
                s += '\\';
                s += 'n';
                break;
            case '\r':
                s += '\\';
                s += 'r';
                break;
            case '\t':
                s += '\\';
                s += 't';
                break;
            default:
                if (*it < 0x20) {
                    s += '\\';
                    s += 'u';
                    s += '0';
                    s += '0';
                    s += hex_digits[*it >> 4];
                    s += hex_digits[*it & 15];
                } else
                    s += *it;
        }
    }
    s += '"';
}

static void stringify_value(const value& v, string& s) {
    std::stringstream ss;
    switch (v.type) {
        case NONE: s += "null"; break;
        case FALSE: s += "false"; break;
        case TRUE: s += "true"; break;
        case NUMBER:
            ss << std::setprecision(17) << v.n;
            s += ss.str();
            break;
        case STRING: stringify_string(*v.s, s); break;
        case ARRAY:
            s += '[';
            if (v.e) {
                for (const value& val : *v.e) {
                    stringify_value(val, s);
                    s += ',';
                }
                s.pop_back();
            }
            s += ']';
            break;
        case OBJECT:
            s += '{';
            if (v.m) {
                for (const member& mem : *v.m) {
                    stringify_string(mem.k, s);
                    s += ':';
                    stringify_value(mem.v, s);
                    s += ',';
                }
                s.pop_back();
            }
            s += '}';
            break;
        default: throw "Invalid value type";
    }
}

string stringify(const value& v, size_t* length = nullptr) {
    string s("");
    stringify_value(v, s);
    if (length != nullptr) *length = s.size();
    return s;
}

void freeVal(value& v) {
    switch (v.type) {
        case STRING:
            if (v.s != nullptr) {
                delete v.s;
                v.s = nullptr;
            }
            break;
        case ARRAY:
            if (v.e != nullptr) {
                for (auto val : *v.e) freeVal(val);
                delete v.e;
                v.e = nullptr;
            }
            break;
        case OBJECT:
            if (v.m != nullptr) {
                for (auto mem : *v.m) {
                    freeVal(mem.v);
                }
                v.m = nullptr;
            }
            break;
        default: break;
    }
    v.type = NONE;
}

e_types get_type(const value& v) { return v.type; }

int get_boolean(const value& v) {
    assert(v.type == TRUE || v.type == FALSE);
    return v.type == TRUE;
}

void set_boolean(value& v, int b) {
    freeVal(v);
    v.type = b ? TRUE : FALSE;
}

double get_number(const value& v) {
    assert(v.type == NUMBER);
    return v.n;
}

void set_number(value& v, double n) {
    freeVal(v);
    v.n = n;
    v.type = NUMBER;
}

string& get_string(const value& v) {
    assert(v.type == STRING);
    return *v.s;
}

size_t get_string_length(const value& v) {
    assert(v.type == STRING);
    return v.s->size();
}

void set_string(value& v, const string& s) {
    freeVal(v);
    v.s = new string(s);
    v.type = STRING;
}

size_t get_array_size(const value& v) {
    assert(v.type == ARRAY);
    if (v.e == nullptr) return 0;
    return v.e->size();
}

value& get_array_element(const value& v, size_t index) {
    assert(v.type == ARRAY);
    assert(index < get_array_size(v));
    return (*v.e)[index];
}

size_t get_object_size(const value& v) {
    assert(v.type == OBJECT);
    if (v.m == nullptr) return 0;
    return v.m->size();
}

const string& get_object_key(const value& v, size_t index) {
    assert(v.type == OBJECT);
    assert(index < get_object_size(v));
    return (*v.m)[index].k;
}

size_t get_object_key_length(const value& v, size_t index) {
    assert(v.type == OBJECT);
    assert(index < get_object_size(v));
    return (*v.m)[index].k.size();
}

value& get_object_value(const value& v, size_t index) {
    assert(v.type == OBJECT);
    assert(index < get_object_size(v));
    return (*v.m)[index].v;
}

}  // namespace lept
