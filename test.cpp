// #include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>

#include "leptjson.h"

using lept::parse, lept::get_type;
using lept::value;
using std::cout, std::cerr;
using std::string;

static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;

#define EXPECT_EQ_BASE(equality, expect, actual)                                                   \
    do {                                                                                           \
        test_count++;                                                                              \
        if (equality)                                                                              \
            test_pass++;                                                                           \
        else {                                                                                     \
            cerr << __FILE__ << ':' << __LINE__ << ": expect: " << expect << " actual: " << actual \
                 << "\n";                                                                          \
            main_ret = 1;                                                                          \
        }                                                                                          \
    } while (0)

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual)

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

static void test_parse_expect_value() {
    value v;

    v.type = lept::FALSE;
    EXPECT_EQ_INT(lept::PARSE_EXPECT_VALUE, parse(v, ""));
    EXPECT_EQ_INT(lept::NONE, get_type(v));

    v.type = lept::FALSE;
    EXPECT_EQ_INT(lept::PARSE_EXPECT_VALUE, parse(v, " "));
    EXPECT_EQ_INT(lept::NONE, get_type(v));
}

static void test_parse_invalid_value() {
    value v;
    v.type = lept::FALSE;
    EXPECT_EQ_INT(lept::PARSE_INVALID_VALUE, parse(v, "nul"));
    EXPECT_EQ_INT(lept::NONE, get_type(v));

    v.type = lept::FALSE;
    EXPECT_EQ_INT(lept::PARSE_INVALID_VALUE, parse(v, "?"));
    EXPECT_EQ_INT(lept::NONE, get_type(v));
}

static void test_parse_root_not_singular() {
    value v;
    v.type = lept::FALSE;
    EXPECT_EQ_INT(lept::PARSE_ROOT_NOT_SINGULAR, parse(v, "null x"));
    EXPECT_EQ_INT(lept::NONE, get_type(v));
}

static void test_parse() {
    test_parse_null();
    test_parse_true();
    test_parse_false();
    test_parse_expect_value();
    test_parse_invalid_value();
    test_parse_root_not_singular();
}

int main() {
    test_parse();
    cout << test_pass << '/' << test_count << ' (' << std::setprecision(5)
         << test_pass * 100.0 / test_count << "%) passed\n";
    return main_ret;
}
