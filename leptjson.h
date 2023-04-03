#ifndef LEPTJSON_H
#define LEPTJSON_H

#include <string>
#include <vector>
#include <assert.h>

namespace lept {

using std::string;
using std::vector;

#define KEY_NOT_EXIST ((size_t)-1)

class LeptValue;

typedef struct member member;

int parse(LeptValue& v, const string& strJson);

string stringify(const LeptValue& v, size_t* length);

typedef enum { NONE, FALSE, TRUE, NUMBER, STRING, ARRAY, OBJECT } e_types;

enum {
    PARSE_OK = 0,
    PARSE_EXPECT_VALUE,
    PARSE_INVALID_VALUE,
    PARSE_ROOT_NOT_SINGULAR,
    PARSE_NUMBER_TOO_BIG,
    PARSE_MISS_QUOTATION_MARK,
    PARSE_INVALID_STRING_ESCAPE,
    PARSE_INVALID_STRING_CHAR,
    PARSE_INVALID_UNICODE_HEX,
    PARSE_INVALID_UNICODE_SURROGATE,
    PARSE_MISS_COMMA_OR_SQUARE_BRACKET,
    PARSE_MISS_KEY,
    PARSE_MISS_COLON,
    PARSE_MISS_COMMA_OR_CURLY_BRACKET
};

class LeptValue {
   public:
    LeptValue() : type(NONE) {}
    void freeVal();
    bool get_boolean() const;
    void set_boolean(bool b);
    double get_number() const;
    void set_number(double n);
    string& get_string() const;
    size_t get_string_length() const;
    void set_string(const string s);
    void set_string(const string* s);
    void set_array(const vector<LeptValue>* arr);
    size_t get_array_size() const;
    // size_t get_array_capacity() const ;
    // void reserve_array(size_t capacity);
    // void shrink_array();
    // void clear_array();
    LeptValue& get_array_element(size_t index) const;
    // LeptValue& pushback_array_element();
    // void popback_array_element();
    // LeptValue& insert_array_element(size_t index);
    // void erase_array_element(size_t index, size_t count);
    void set_object(vector<member>* obj);
    size_t get_object_size() const;
    // size_t get_object_capacity() const;
    // void reserve_object(size_t capacity);
    // void shrink_object();
    // void clear_object();
    const string& get_object_key(size_t index) const;
    size_t get_object_key_length(size_t index) const;
    LeptValue& get_object_value(size_t index) const;
    // size_t find_object_index(const string& key);
    // LeptValue* find_object_LeptValue(const string& key);
    // LeptValue& set_object_LeptValue(const string& key);
    // void remove_object_LeptValue(size_t index);
    // void copy(LeptValue& dst, const LeptValue& src);
    // void move(LeptValue& dst, LeptValue& src);
    // void swap(LeptValue& lhs, LeptValue& rhs);
    e_types get_type() const;
    void set_type(e_types t);
    // bool is_equal(const LeptValue& lhs, const LeptValue& rhs);

   private:
    union {  // 不建议把具有析构函数的数据类型作为union内的成员，如string
        vector<member>* m;    /* object elements */
        vector<LeptValue>* e; /* array elements */
        string* s;            /* string elements */
        double n;             /* number */
    };
    e_types type;
};

struct member {
    string k;    /* member key string */
    LeptValue v; /* member LeptValue */
};

inline void LeptValue::freeVal() {
    switch (this->type) {
        case STRING:
            if (this->s != nullptr) {
                delete this->s;
                this->s = nullptr;
            }
            break;
        case ARRAY:
            if (this->e != nullptr) {
                for (LeptValue& val : *this->e) val.freeVal();
                delete this->e;
                this->e = nullptr;
            }
            break;
        case OBJECT:
            if (this->m != nullptr) {
                for (member& mem : *this->m) {
                    mem.v.freeVal();
                }
                this->m = nullptr;
            }
            break;
        default: break;
    }
    this->type = NONE;
}

inline e_types LeptValue::get_type() const { return this->type; }

inline void LeptValue::set_type(e_types t) { this->type = t; }

inline bool LeptValue::get_boolean() const {
    assert(this->type == TRUE || this->type == FALSE);
    return this->type == TRUE;
}

inline void LeptValue::set_boolean(bool b) {
    this->freeVal();
    this->type = b ? TRUE : FALSE;
}

inline double LeptValue::get_number() const {
    assert(this->type == NUMBER);
    return this->n;
}

inline void LeptValue::set_number(double n) {
    this->freeVal();
    this->n = n;
    this->type = NUMBER;
}

inline string& LeptValue::get_string() const {
    assert(this->type == STRING);
    return *this->s;
}

inline size_t LeptValue::get_string_length() const {
    assert(this->type == STRING);
    return this->s->size();
}

inline void LeptValue::set_string(const string s) {
    this->freeVal();
    this->s = new string(s);
    this->type = STRING;
}

inline void LeptValue::set_string(const string* s) {
    this->freeVal();
    if (s == nullptr)
        this->s = nullptr;
    else
        this->s = new string(*s);
    this->type = STRING;
}

inline void LeptValue::set_array(const vector<LeptValue>* arr) {
    this->freeVal();
    if (arr == nullptr)
        this->e = nullptr;
    else
        this->e = new vector<LeptValue>(*arr);
    this->type = ARRAY;
}

inline size_t LeptValue::get_array_size() const {
    assert(this->type == ARRAY);
    if (this->e == nullptr) return 0;
    return this->e->size();
}

inline void LeptValue::set_object(vector<member>* obj) {
    this->freeVal();
    if (obj == nullptr)
        this->m = nullptr;
    else
        this->m = new vector<member>(*obj);
    this->type = OBJECT;
}

inline LeptValue& LeptValue::get_array_element(size_t index) const {
    assert(this->type == ARRAY);
    assert(index < this->get_array_size());
    return (*this->e)[index];
}

inline size_t LeptValue::get_object_size() const {
    assert(this->type == OBJECT);
    if (this->m == nullptr) return 0;
    return this->m->size();
}

inline const string& LeptValue::get_object_key(size_t index) const {
    assert(this->type == OBJECT);
    assert(index < this->get_object_size());
    return (*this->m)[index].k;
}

inline size_t LeptValue::get_object_key_length(size_t index) const {
    assert(this->type == OBJECT);
    assert(index < this->get_object_size());
    return (*this->m)[index].k.size();
}

inline LeptValue& LeptValue::get_object_value(size_t index) const {
    assert(this->type == OBJECT);
    assert(index < this->get_object_size());
    return (*this->m)[index].v;
}

}  // namespace lept

#endif /* LEPTJSON_H */
