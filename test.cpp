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
#define EXPECT_EQ_STRING(expect, actual, alength)                                                \
    EXPECT_EQ_BASE(                                                                              \
        sizeof(expect) - 1 == (alength) && string(expect, sizeof(expect) - 1) == actual, expect, \
        actual, 0)
#define EXPECT_TRUE(actual) EXPECT_EQ_BASE((actual) != 0, "true", "false", 0)
#define EXPECT_FALSE(actual) EXPECT_EQ_BASE((actual) == 0, "false", "true", 0)

#if defined(_MSC_VER)
#define EXPECT_EQ_SIZE_T(expect, actual) \
    EXPECT_EQ_BASE((expect) == (actual), (size_t)expect, (size_t)actual, 0)
#else
#define EXPECT_EQ_SIZE_T(expect, actual) \
    EXPECT_EQ_BASE((expect) == (actual), (size_t)expect, (size_t)actual, 0)
#endif

static void test_parse_null() {
    LeptValue v;
    v.freeVal();
    v.set_type(FALSE);
    EXPECT_EQ_INT(PARSE_OK, parse(v, " null"));
    EXPECT_EQ_INT(NONE, v.get_type());
    v.freeVal();
}

static void test_parse_true() {
    LeptValue v;
    v.freeVal();
    v.set_type(FALSE);
    EXPECT_EQ_INT(PARSE_OK, parse(v, "true "));
    EXPECT_EQ_INT(TRUE, v.get_type());
    v.freeVal();
}

static void test_parse_false() {
    LeptValue v;
    v.freeVal();
    v.set_type(TRUE);
    EXPECT_EQ_INT(PARSE_OK, parse(v, "false"));
    EXPECT_EQ_INT(FALSE, v.get_type());
    v.freeVal();
}

#define TEST_NUMBER(expect, json)                 \
    do {                                          \
        LeptValue v;                              \
        v.freeVal();                              \
        EXPECT_EQ_INT(PARSE_OK, parse(v, json));  \
        EXPECT_EQ_INT(NUMBER, v.get_type());      \
        EXPECT_EQ_DOUBLE(expect, v.get_number()); \
        v.freeVal();                              \
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

#define TEST_STRING(expect, json)                                        \
    do {                                                                 \
        LeptValue v;                                                     \
        v.freeVal();                                                     \
        EXPECT_EQ_INT(PARSE_OK, parse(v, json));                         \
        EXPECT_EQ_INT(STRING, v.get_type());                             \
        EXPECT_EQ_STRING(expect, v.get_string(), v.get_string_length()); \
        v.freeVal();                                                     \
    } while (0)

static void test_parse_string() {
    TEST_STRING("", "\"\"");
    TEST_STRING("Hello", "\"Hello\"");
    TEST_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
    TEST_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
    TEST_STRING("Hello\0World", "\"Hello\\u0000World\"");
    TEST_STRING("\x24", "\"\\u0024\"");                    /* Dollar sign U+0024 */
    TEST_STRING("\xC2\xA2", "\"\\u00A2\"");                /* Cents sign U+00A2 */
    TEST_STRING("\xE2\x82\xAC", "\"\\u20AC\"");            /* Euro sign U+20AC */
    TEST_STRING("\xF0\x9D\x84\x9E", "\"\\uD834\\uDD1E\""); /* G clef sign U+1D11E */
    TEST_STRING("\xF0\x9D\x84\x9E", "\"\\ud834\\udd1e\""); /* G clef sign U+1D11E */
}

static void test_parse_array() {
    size_t i, j;
    LeptValue v;

    v.freeVal();
    EXPECT_EQ_INT(PARSE_OK, parse(v, "[ ]"));
    EXPECT_EQ_INT(ARRAY, v.get_type());
    EXPECT_EQ_SIZE_T(0, v.get_array_size());
    v.freeVal();

    v.freeVal();
    EXPECT_EQ_INT(PARSE_OK, parse(v, "[ null , false , true , 123 , \"abc\" ]"));
    EXPECT_EQ_INT(ARRAY, v.get_type());
    EXPECT_EQ_SIZE_T(5, v.get_array_size());
    EXPECT_EQ_INT(NONE, v.get_array_element(0).get_type());
    EXPECT_EQ_INT(FALSE, v.get_array_element(1).get_type());
    EXPECT_EQ_INT(TRUE, v.get_array_element(2).get_type());
    EXPECT_EQ_INT(NUMBER, v.get_array_element(3).get_type());
    EXPECT_EQ_INT(STRING, v.get_array_element(4).get_type());
    EXPECT_EQ_DOUBLE(123.0, v.get_array_element(3).get_number());
    EXPECT_EQ_STRING("abc", v.get_array_element(4).get_string(),
                     v.get_array_element(4).get_string_length());
    v.freeVal();

    v.freeVal();
    EXPECT_EQ_INT(PARSE_OK, parse(v, "[ [ ] , [ 0 ] , [ 0 , 1 ] , [ 0 , 1 , 2 ] ]"));
    EXPECT_EQ_INT(ARRAY, v.get_type());
    EXPECT_EQ_SIZE_T(4, v.get_array_size());
    for (i = 0; i < 4; i++) {
        LeptValue& a = v.get_array_element(i);
        EXPECT_EQ_INT(ARRAY, a.get_type());
        EXPECT_EQ_SIZE_T(i, a.get_array_size());
        for (j = 0; j < i; j++) {
            LeptValue& e = a.get_array_element(j);
            EXPECT_EQ_INT(NUMBER, e.get_type());
            EXPECT_EQ_DOUBLE((double)j, e.get_number());
        }
    }
    v.freeVal();
}

static void test_parse_object() {
    LeptValue v;
    size_t i;

    v.freeVal();
    EXPECT_EQ_INT(PARSE_OK, parse(v, " { } "));
    EXPECT_EQ_INT(OBJECT, v.get_type());
    EXPECT_EQ_SIZE_T(0, v.get_object_size());
    v.freeVal();

    v.freeVal();
    EXPECT_EQ_INT(PARSE_OK, parse(v,
                                  " { "
                                  "\"n\" : null , "
                                  "\"f\" : false , "
                                  "\"t\" : true , "
                                  "\"i\" : 123 , "
                                  "\"s\" : \"abc\", "
                                  "\"a\" : [ 1, 2, 3 ],"
                                  "\"o\" : { \"1\" : 1, \"2\" : 2, \"3\" : 3 }"
                                  " } "));
    EXPECT_EQ_INT(OBJECT, v.get_type());
    EXPECT_EQ_SIZE_T(7, v.get_object_size());
    EXPECT_EQ_STRING("n", v.get_object_key(0), v.get_object_key_length(0));
    EXPECT_EQ_INT(NONE, v.get_object_value(0).get_type());
    EXPECT_EQ_STRING("f", v.get_object_key(1), v.get_object_key_length(1));
    EXPECT_EQ_INT(FALSE, v.get_object_value(1).get_type());
    EXPECT_EQ_STRING("t", v.get_object_key(2), v.get_object_key_length(2));
    EXPECT_EQ_INT(TRUE, v.get_object_value(2).get_type());
    EXPECT_EQ_STRING("i", v.get_object_key(3), v.get_object_key_length(3));
    EXPECT_EQ_INT(NUMBER, v.get_object_value(3).get_type());
    EXPECT_EQ_DOUBLE(123.0, v.get_object_value(3).get_number());
    EXPECT_EQ_STRING("s", v.get_object_key(4), v.get_object_key_length(4));
    EXPECT_EQ_INT(STRING, v.get_object_value(4).get_type());
    EXPECT_EQ_STRING("abc", v.get_object_value(4).get_string(),
                     v.get_object_value(4).get_string_length());
    EXPECT_EQ_STRING("a", v.get_object_key(5), v.get_object_key_length(5));
    EXPECT_EQ_INT(ARRAY, v.get_object_value(5).get_type());
    EXPECT_EQ_SIZE_T(3, v.get_object_value(5).get_array_size());
    for (i = 0; i < 3; i++) {
        LeptValue& e = v.get_object_value(5).get_array_element(i);
        EXPECT_EQ_INT(NUMBER, e.get_type());
        EXPECT_EQ_DOUBLE(i + 1.0, e.get_number());
    }
    EXPECT_EQ_STRING("o", v.get_object_key(6), v.get_object_key_length(6));
    {
        LeptValue& o = v.get_object_value(6);
        EXPECT_EQ_INT(OBJECT, o.get_type());
        for (i = 0; i < 3; i++) {
            LeptValue& ov = o.get_object_value(i);
            EXPECT_TRUE('1' + i == o.get_object_key(i)[0]);
            EXPECT_EQ_SIZE_T(1, o.get_object_key_length(i));
            EXPECT_EQ_INT(NUMBER, ov.get_type());
            EXPECT_EQ_DOUBLE(i + 1.0, ov.get_number());
        }
    }
    v.freeVal();
}

#define TEST_ERROR(error, json)               \
    do {                                      \
        LeptValue v;                          \
        v.freeVal();                          \
        v.set_type(FALSE);                    \
        EXPECT_EQ_INT(error, parse(v, json)); \
        EXPECT_EQ_INT(NONE, v.get_type());    \
        v.freeVal();                          \
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

static void test_parse_miss_comma_or_square_bracket() {
    TEST_ERROR(PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1");
    TEST_ERROR(PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1}");
    TEST_ERROR(PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[1 2");
    TEST_ERROR(PARSE_MISS_COMMA_OR_SQUARE_BRACKET, "[[]");
}

static void test_parse_miss_key() {
    TEST_ERROR(PARSE_MISS_KEY, "{:1,");
    TEST_ERROR(PARSE_MISS_KEY, "{1:1,");
    TEST_ERROR(PARSE_MISS_KEY, "{true:1,");
    TEST_ERROR(PARSE_MISS_KEY, "{false:1,");
    TEST_ERROR(PARSE_MISS_KEY, "{null:1,");
    TEST_ERROR(PARSE_MISS_KEY, "{[]:1,");
    TEST_ERROR(PARSE_MISS_KEY, "{{}:1,");
    TEST_ERROR(PARSE_MISS_KEY, "{\"a\":1,");
}

static void test_parse_miss_colon() {
    TEST_ERROR(PARSE_MISS_COLON, "{\"a\"}");
    TEST_ERROR(PARSE_MISS_COLON, "{\"a\",\"b\"}");
}

static void test_parse_miss_comma_or_curly_bracket() {
    TEST_ERROR(PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1");
    TEST_ERROR(PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1]");
    TEST_ERROR(PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":1 \"b\"");
    TEST_ERROR(PARSE_MISS_COMMA_OR_CURLY_BRACKET, "{\"a\":{}");
}

static void test_parse() {
    test_parse_null();
    test_parse_true();
    test_parse_false();
    test_parse_number();
    test_parse_string();
    test_parse_array();
    test_parse_object();
    test_parse_expect_value();
    test_parse_invalid_value();
    test_parse_root_not_singular();
    test_parse_number_too_big();
    test_parse_missing_quotation_mark();
    test_parse_invalid_string_escape();
    test_parse_invalid_string_char();
    test_parse_invalid_unicode_hex();
    test_parse_invalid_unicode_surrogate();
    test_parse_miss_comma_or_square_bracket();
    test_parse_miss_key();
    test_parse_miss_colon();
    test_parse_miss_comma_or_curly_bracket();
}

#define TEST_ROUNDTRIP(json)                     \
    do {                                         \
        LeptValue v;                             \
        string jsonOut;                          \
        size_t length;                           \
        v.freeVal();                             \
        EXPECT_EQ_INT(PARSE_OK, parse(v, json)); \
        jsonOut = stringify(v, &length);         \
        EXPECT_EQ_STRING(json, jsonOut, length); \
        v.freeVal();                             \
    } while (0)

static void test_stringify_number() {
    TEST_ROUNDTRIP("0");
    TEST_ROUNDTRIP("-0");
    TEST_ROUNDTRIP("1");
    TEST_ROUNDTRIP("-1");
    TEST_ROUNDTRIP("1.5");
    TEST_ROUNDTRIP("-1.5");
    TEST_ROUNDTRIP("3.25");
    TEST_ROUNDTRIP("1e+20");
    TEST_ROUNDTRIP("1.234e+20");
    TEST_ROUNDTRIP("1.234e-20");

    TEST_ROUNDTRIP("1.0000000000000002");      /* the smallest number > 1 */
    TEST_ROUNDTRIP("2.2250738585072014e-308"); /* Min normal positive double */
    TEST_ROUNDTRIP("-2.2250738585072014e-308");
    TEST_ROUNDTRIP("1.7976931348623157e+308"); /* Max double */
    TEST_ROUNDTRIP("-1.7976931348623157e+308");
}

static void test_stringify_string() {
    TEST_ROUNDTRIP("\"\"");
    TEST_ROUNDTRIP("\"Hello\"");
    TEST_ROUNDTRIP("\"Hello\\nWorld\"");
    TEST_ROUNDTRIP("\"\\\" \\\\ / \\b \\f \\n \\r \\t\"");
    TEST_ROUNDTRIP("\"Hello\\u0000World\"");
}

static void test_stringify_array() {
    TEST_ROUNDTRIP("[]");
    TEST_ROUNDTRIP("[null,false,true,123,\"abc\",[1,2,3]]");
}

static void test_stringify_object() {
    TEST_ROUNDTRIP("{}");
    TEST_ROUNDTRIP(
        "{\"n\":null,\"f\":false,\"t\":true,\"i\":123,\"s\":\"abc\",\"a\":[1,2,3],\"o\":{\"1\":1,"
        "\"2\":2,\"3\":3}}");
}

static void test_stringify() {
    TEST_ROUNDTRIP("null");
    TEST_ROUNDTRIP("false");
    TEST_ROUNDTRIP("true");
    test_stringify_number();
    test_stringify_string();
    test_stringify_array();
    test_stringify_object();
}

static void test_access_null() {
    LeptValue v;
    v.freeVal();
    v.set_string("a");
    v.freeVal();
    EXPECT_EQ_INT(NONE, v.get_type());
    v.freeVal();
}

static void test_access_boolean() {
    LeptValue v;
    v.freeVal();
    v.set_string("a");
    v.set_boolean(1);
    EXPECT_TRUE(v.get_boolean());
    v.set_boolean(0);
    EXPECT_FALSE(v.get_boolean());
    v.freeVal();
}

static void test_access_number() {
    LeptValue v;
    v.freeVal();
    v.set_string("a");
    v.set_number(1234.5);
    EXPECT_EQ_DOUBLE(1234.5, v.get_number());
    v.freeVal();
}

static void test_access_string() {
    LeptValue v;
    v.freeVal();
    v.set_string(string(""));
    EXPECT_EQ_STRING("", v.get_string(), v.get_string_length());
    v.set_string(string("Hello"));
    EXPECT_EQ_STRING("Hello", v.get_string(), v.get_string_length());
    v.freeVal();
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
    lept::test_stringify();
    lept::test_access();
    std::cout << lept::test_pass << '/' << lept::test_count << " (" << std::setprecision(5)
              << lept::test_pass * 100.0 / lept::test_count << "%) passed\n" << std::endl;
    return lept::main_ret;
}
