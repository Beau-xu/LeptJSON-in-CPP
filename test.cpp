// #include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>

#include "leptjson.h"

namespace lept {
using std::cerr;
using std::cout;
using std::string;

static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;

#define EXPECT_EQ_BASE(equality, expect, actual, prec)                       \
    do {                                                                     \
        test_count++;                                                        \
        if (equality)                                                        \
            test_pass++;                                                     \
        else {                                                               \
            cerr << std::setprecision(prec) << __FILE__ << ':' << __LINE__   \
                 << ": expect: " << expect << " actual: " << actual << "\n"; \
            main_ret = 1;                                                    \
        }                                                                    \
    } while (0)

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, 0)
#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, 17)
#define EXPECT_EQ_STRING(expect, actual, alength) \
    EXPECT_EQ_BASE(sizeof(expect) - 1 == (alength) && expect == actual, expect, actual, 0)
#define EXPECT_TRUE(actual) EXPECT_EQ_BASE((actual) != 0, "true", "false", 0)
#define EXPECT_FALSE(actual) EXPECT_EQ_BASE((actual) == 0, "false", "true", 0)

static void test_parse_null() {
    value v;
    init(v);
    v.type = FALSE;
    EXPECT_EQ_INT(PARSE_OK, parse(v, " null"));
    EXPECT_EQ_INT(NONE, get_type(v));
    freeVal(v);
}

static void test_parse_true() {
    value v;
    init(v);
    v.type = FALSE;
    EXPECT_EQ_INT(PARSE_OK, parse(v, "true "));
    EXPECT_EQ_INT(TRUE, get_type(v));
    freeVal(v);
}

static void test_parse_false() {
    value v;
    init(v);
    v.type = TRUE;
    EXPECT_EQ_INT(PARSE_OK, parse(v, "false"));
    EXPECT_EQ_INT(FALSE, get_type(v));
    freeVal(v);
}

#define TEST_NUMBER(expect, json)                \
    do {                                         \
        value v;                                 \
        init(v);                                 \
        EXPECT_EQ_INT(PARSE_OK, parse(v, json)); \
        EXPECT_EQ_INT(NUMBER, get_type(v));      \
        EXPECT_EQ_DOUBLE(expect, get_number(v)); \
        freeVal(v);                              \
    } while (0)

static void test_parse_number() {
    TEST_NUMBER(0.0, "0");
    TEST_NUMBER(0.0, "-0");
    TEST_NUMBER(0.0, "-0.0");
    TEST_NUMBER(1.0, "1");
    TEST_NUMBER(-1.0, "-1");
    TEST_NUMBER(1.5, "1.5");
    TEST_NUMBER(-1.5, "-1.5");
    TEST_NUMBER(3.1416, "3.1416");
    TEST_NUMBER(1E10, "1E10");
    TEST_NUMBER(1e10, "1e10");
    TEST_NUMBER(1E+10, "1E+10");
    TEST_NUMBER(1E-10, "1E-10");
    TEST_NUMBER(-1E10, "-1E10");
    TEST_NUMBER(-1e10, "-1e10");
    TEST_NUMBER(-1E+10, "-1E+10");
    TEST_NUMBER(-1E-10, "-1E-10");
    TEST_NUMBER(1.234E+10, "1.234E+10");
    TEST_NUMBER(1.234E-10, "1.234E-10");

    TEST_NUMBER(1.0000000000000002, "1.0000000000000002");            /* the smallest number > 1 */
    TEST_NUMBER(2.2250738585072014e-308, "2.2250738585072014e-308");  // min normal
    TEST_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");
    TEST_NUMBER(1.7976931348623157e+308, "1.7976931348623157e+308");  // max
    TEST_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");
}

#define TEST_STRING(expect, json)                                      \
    do {                                                               \
        value v;                                                       \
        init(v);                                                       \
        EXPECT_EQ_INT(PARSE_OK, parse(v, json));                       \
        EXPECT_EQ_INT(STRING, get_type(v));                            \
        EXPECT_EQ_STRING(expect, get_string(v), get_string_length(v)); \
        freeVal(v);                                                    \
    } while (0)

static void test_parse_string() {
    TEST_STRING("", "\"\"");
    TEST_STRING("Hello", "\"Hello\"");
    TEST_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
    TEST_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
    // TEST_STRING("Hello\0World", "\"Hello\\u0000World\"");
    TEST_STRING("\x24", "\"\\u0024\"");                    /* Dollar sign U+0024 */
    TEST_STRING("\xC2\xA2", "\"\\u00A2\"");                /* Cents sign U+00A2 */
    TEST_STRING("\xE2\x82\xAC", "\"\\u20AC\"");            /* Euro sign U+20AC */
    TEST_STRING("\xF0\x9D\x84\x9E", "\"\\uD834\\uDD1E\""); /* G clef sign U+1D11E */
    TEST_STRING("\xF0\x9D\x84\x9E", "\"\\ud834\\udd1e\""); /* G clef sign U+1D11E */
}

#define TEST_ERROR(error, json)               \
    do {                                      \
        value v;                              \
        init(v);                              \
        v.type = FALSE;                       \
        EXPECT_EQ_INT(error, parse(v, json)); \
        EXPECT_EQ_INT(NONE, get_type(v));     \
        freeVal(v);                           \
    } while (0)

static void test_parse_expect_value() {
    TEST_ERROR(PARSE_EXPECT_VALUE, "");
    TEST_ERROR(PARSE_EXPECT_VALUE, " ");
}

static void test_parse_invalid_value() {
    TEST_ERROR(PARSE_INVALID_VALUE, "nul");
    TEST_ERROR(PARSE_INVALID_VALUE, "?");

    /* invalid number */
    TEST_ERROR(PARSE_INVALID_VALUE, "+0");
    TEST_ERROR(PARSE_INVALID_VALUE, "+1");
    TEST_ERROR(PARSE_INVALID_VALUE, ".123"); /* at least one digit before '.' */
    TEST_ERROR(PARSE_INVALID_VALUE, "1.");   /* at least one digit after '.' */
    TEST_ERROR(PARSE_INVALID_VALUE, "INF");
    TEST_ERROR(PARSE_INVALID_VALUE, "inf");
    TEST_ERROR(PARSE_INVALID_VALUE, "NAN");
    TEST_ERROR(PARSE_INVALID_VALUE, "nan");
}

static void test_parse_root_not_singular() {
    TEST_ERROR(PARSE_ROOT_NOT_SINGULAR, "null x");

    /* invalid number */
    TEST_ERROR(PARSE_ROOT_NOT_SINGULAR,
               "0123"); /* after zero should be '.' , 'E' , 'e' or nothing */
    TEST_ERROR(PARSE_ROOT_NOT_SINGULAR, "0x0");
    TEST_ERROR(PARSE_ROOT_NOT_SINGULAR, "0x123");
}

static void test_parse_number_too_big() {
    TEST_ERROR(PARSE_NUMBER_TOO_BIG, "1e309");
    TEST_ERROR(PARSE_NUMBER_TOO_BIG, "-1e309");
    TEST_ERROR(PARSE_NUMBER_TOO_BIG, "1e-10000"); /* must underflow */
}

static void test_parse_missing_quotation_mark() {
    TEST_ERROR(PARSE_MISS_QUOTATION_MARK, "\"");
    TEST_ERROR(PARSE_MISS_QUOTATION_MARK, "\"abc");
}

static void test_parse_invalid_string_escape() {
    TEST_ERROR(PARSE_INVALID_STRING_ESCAPE, "\"\\v\"");
    TEST_ERROR(PARSE_INVALID_STRING_ESCAPE, "\"\\'\"");
    TEST_ERROR(PARSE_INVALID_STRING_ESCAPE, "\"\\0\"");
    TEST_ERROR(PARSE_INVALID_STRING_ESCAPE, "\"\\x12\"");
}

static void test_parse_invalid_string_char() {
    TEST_ERROR(PARSE_INVALID_STRING_CHAR, "\"\x01\"");
    TEST_ERROR(PARSE_INVALID_STRING_CHAR, "\"\x1F\"");
}

static void test_parse_invalid_unicode_hex() {
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u0\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u01\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u012\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u/000\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\uG000\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u0/00\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u0G00\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u00/0\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u00G0\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u000/\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u000G\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u 123\"");
}

static void test_parse_invalid_unicode_surrogate() {
    TEST_ERROR(PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_SURROGATE, "\"\\uDBFF\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\\\\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uDBFF\"");
    TEST_ERROR(PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD800\\uE000\"");
}

static void test_parse() {
    test_parse_null();
    test_parse_true();
    test_parse_false();
    test_parse_number();
    test_parse_string();
    test_parse_expect_value();
    test_parse_invalid_value();
    test_parse_root_not_singular();
    test_parse_number_too_big();
    test_parse_missing_quotation_mark();
    test_parse_invalid_string_escape();
    test_parse_invalid_string_char();
    test_parse_invalid_unicode_hex();
    test_parse_invalid_unicode_surrogate();
}

static void test_access_null() {
    value v;
    init(v);
    set_string(v, "a");
    set_null(v);
    EXPECT_EQ_INT(NONE, get_type(v));
    freeVal(v);
}

static void test_access_boolean() {
    value v;
    init(v);
    set_string(v, "a");
    set_boolean(v, 1);
    EXPECT_TRUE(get_boolean(v));
    set_boolean(v, 0);
    EXPECT_FALSE(get_boolean(v));
    freeVal(v);
}

static void test_access_number() {
    value v;
    init(v);
    set_string(v, "a");
    set_number(v, 1234.5);
    EXPECT_EQ_DOUBLE(1234.5, get_number(v));
    freeVal(v);
}

static void test_access_string() {
    value v;
    init(v);
    set_string(v, string(""));
    EXPECT_EQ_STRING("", get_string(v), get_string_length(v));
    set_string(v, string("Hello"));
    EXPECT_EQ_STRING("Hello", get_string(v), get_string_length(v));
    freeVal(v);
}

static void test_access() {
    test_access_null();
    test_access_boolean();
    test_access_number();
    test_access_string();
}

}  // namespace lept

int main() {
    lept::test_parse();
    std::cout << lept::test_pass << '/' << lept::test_count << " (" << std::setprecision(5)
              << lept::test_pass * 100.0 / lept::test_count << "%) passed\n";
    return lept::main_ret;
}
