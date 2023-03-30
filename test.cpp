// #include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>

#include "leptjson.h"

using lept::get_number;
using lept::get_type;
using lept::parse;
using lept::value;
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

static void test_parse_null() {
    value v;
    v.type = lept::FALSE;
    EXPECT_EQ_INT(lept::PARSE_OK, parse(v, "null"));
    EXPECT_EQ_INT(lept::NONE, get_type(v));
}

static void test_parse_true() {
    value v;
    v.type = lept::FALSE;
    EXPECT_EQ_INT(lept::PARSE_OK, parse(v, "true"));
    EXPECT_EQ_INT(lept::TRUE, get_type(v));
}

static void test_parse_false() {
    value v;
    v.type = lept::TRUE;
    EXPECT_EQ_INT(lept::PARSE_OK, parse(v, "false"));
    EXPECT_EQ_INT(lept::FALSE, get_type(v));
}

#define TEST_NUMBER(expect, json)                      \
    do {                                               \
        value v;                                       \
        EXPECT_EQ_INT(lept::PARSE_OK, parse(v, json)); \
        EXPECT_EQ_INT(lept::NUMBER, get_type(v));      \
        EXPECT_EQ_DOUBLE(expect, get_number(v));       \
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
    TEST_NUMBER(0.0, "1e-10000"); /* must underflow */

    TEST_NUMBER(1.0000000000000002, "1.0000000000000002");           /* the smallest number > 1 */
    TEST_NUMBER(4.9406564584124654e-324, "4.9406564584124654e-324"); /* minimum denormal */
    TEST_NUMBER(-4.9406564584124654e-324, "-4.9406564584124654e-324");
    TEST_NUMBER(2.2250738585072009e-308, "2.2250738585072009e-308"); /* Max subnormal double */
    TEST_NUMBER(-2.2250738585072009e-308, "-2.2250738585072009e-308");
    TEST_NUMBER(2.2250738585072014e-308,
                "2.2250738585072014e-308"); /* Min normal positive double */
    TEST_NUMBER(-2.2250738585072014e-308, "-2.2250738585072014e-308");
    TEST_NUMBER(1.7976931348623157e+308, "1.7976931348623157e+308"); /* Max double */
    TEST_NUMBER(-1.7976931348623157e+308, "-1.7976931348623157e+308");
}

#define TEST_ERROR(error, json)                 \
    do {                                        \
        value v;                                \
        v.type = lept::FALSE;                   \
        EXPECT_EQ_INT(error, parse(v, json));   \
        EXPECT_EQ_INT(lept::NONE, get_type(v)); \
    } while (0)

static void test_parse_expect_value() {
    TEST_ERROR(lept::PARSE_EXPECT_VALUE, "");
    TEST_ERROR(lept::PARSE_EXPECT_VALUE, " ");
}

static void test_parse_invalid_value() {
    TEST_ERROR(lept::PARSE_INVALID_VALUE, "nul");
    TEST_ERROR(lept::PARSE_INVALID_VALUE, "?");

    /* invalid number */
    TEST_ERROR(lept::PARSE_INVALID_VALUE, "+0");
    TEST_ERROR(lept::PARSE_INVALID_VALUE, "+1");
    TEST_ERROR(lept::PARSE_INVALID_VALUE, ".123"); /* at least one digit before '.' */
    TEST_ERROR(lept::PARSE_INVALID_VALUE, "1.");   /* at least one digit after '.' */
    TEST_ERROR(lept::PARSE_INVALID_VALUE, "INF");
    TEST_ERROR(lept::PARSE_INVALID_VALUE, "inf");
    TEST_ERROR(lept::PARSE_INVALID_VALUE, "NAN");
    TEST_ERROR(lept::PARSE_INVALID_VALUE, "nan");
}

static void test_parse_root_not_singular() {
    TEST_ERROR(lept::PARSE_ROOT_NOT_SINGULAR, "null x");

    /* invalid number */
    TEST_ERROR(lept::PARSE_ROOT_NOT_SINGULAR,
               "0123"); /* after zero should be '.' , 'E' , 'e' or nothing */
    TEST_ERROR(lept::PARSE_ROOT_NOT_SINGULAR, "0x0");
    TEST_ERROR(lept::PARSE_ROOT_NOT_SINGULAR, "0x123");
}

static void test_parse_number_too_big() {
#if 0
    TEST_ERROR(lept::PARSE_NUMBER_TOO_BIG, "1e309");
    TEST_ERROR(lept::PARSE_NUMBER_TOO_BIG, "-1e309");
#endif
}

static void test_parse() {
    test_parse_null();
    test_parse_true();
    test_parse_false();
    test_parse_number();
    test_parse_expect_value();
    test_parse_invalid_value();
    test_parse_root_not_singular();
    test_parse_number_too_big();
}

int main() {
    test_parse();
    cout << test_pass << '/' << test_count << " (" << std::setprecision(5)
         << test_pass * 100.0 / test_count << "%) passed\n";
    return main_ret;
}
