// #include <cstdlib>
#include "leptjson.h"

#include <cassert> /* assert() */
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

static int parse_null(context &c, value &v) {
    EXPECT(c, 'n');
    if (c.json[0] != 'u' || c.json[1] != 'l' || c.json[2] != 'l')
        return PARSE_INVALID_VALUE;
    c.json += 3;
    v.type = NONE;
    return PARSE_OK;
}

static int parse_true(context &c, value &v) {
    EXPECT(c, 't');
    if (c.json[0] != 'r' || c.json[1] != 'u' || c.json[2] != 'e')
        return PARSE_INVALID_VALUE;
    c.json += 3;
    v.type = TRUE;
    return PARSE_OK;
}

static int parse_false(context &c, value &v) {
    EXPECT(c, 'f');
    if (c.json[0] != 'a' || c.json[1] != 'l' || c.json[2] != 's' || c.json[3] != 'e')
        return PARSE_INVALID_VALUE;
    c.json += 4;
    v.type = FALSE;
    return PARSE_OK;
}

static int parse_value(context &c, value &v) {
    switch (*c.json) {
        case 'n':
            return parse_null(c, v);
        case 't':
            return parse_true(c, v);
        case 'f':
            return parse_false(c, v);
        case '\0':
            return PARSE_EXPECT_VALUE;
        default:
            return PARSE_INVALID_VALUE;
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
        if (*c.json != '\0')  // 字符串结尾
            ret = PARSE_ROOT_NOT_SINGULAR;
    }
    return ret;
}

e_types lept_get_type(const value &v) {
    // assert(v);
    return v.type;
}

}  // namespace lept
