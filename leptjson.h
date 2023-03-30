#ifndef LEPTJSON_H
#define LEPTJSON_H

namespace lept {

typedef enum { NONE, FALSE, TRUE, NUMBER, STRING, ARRAY, OBJECT } e_types;

typedef struct {
    e_types type;
} value;

enum { PARSE_OK = 0, PARSE_EXPECT_VALUE, PARSE_INVALID_VALUE, PARSE_ROOT_NOT_SINGULAR };

int parse(value &v, const char *json);

e_types get_type(const value &v);

}  // namespace lept

#endif /* LEPTJSON_H */
