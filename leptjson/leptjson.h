#ifndef LEPTJSON_H
#define LEPTJSON_H

#include <assert.h>

#include <string>
#include <vector>

namespace lept {

using std::string;
using std::vector;

#define KEY_NOT_EXIST ((size_t)-1)

class LeptValue;

typedef struct Member Member;

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
    LeptValue(const LeptValue& v);
    ~LeptValue();
    LeptValue& operator=(const LeptValue&);
    void freeVal();
    void release();
    e_types get_type() const;
    void set_type(e_types t);
    bool get_boolean() const;
    void set_boolean(bool b);
    double get_number() const;
    void set_number(double n);
    const string& get_string() const;
    string& get_string();
    size_t get_string_length() const;
    void init_string();
    void set_string(const string& s);
    void init_array();
    void set_array(const vector<LeptValue>& arr);
    size_t get_array_size() const;
    size_t get_array_capacity() const;
    void shrink_array();
    void clear_array();
    const LeptValue& get_array_element(size_t index) const;
    LeptValue& get_array_element(size_t index);
    void pushback_array_element(const LeptValue& v);
    void popback_array_element();
    void insert_array_element(const LeptValue& v, size_t index);
    void erase_array_element(size_t index, size_t count);
    void init_object();
    void set_object(const vector<Member>& obj);
    size_t get_object_size() const;
    const string& get_object_key(size_t index) const;
    size_t get_object_key_length(size_t index) const;
    const LeptValue& get_object_value(size_t index) const;
    LeptValue& get_object_value(size_t index);
    size_t get_object_index(const string& key) const;
    LeptValue* get_object_value(const string& key) const;
    void pushback_object_member(const string& key, const LeptValue& v);
    void remove_object_member(size_t index);
    size_t get_object_capacity() const;
    void shrink_object();
    void clear_object();
    // void move(LeptValue& dst, LeptValue& src);
    // void swap(LeptValue& lhs, LeptValue& rhs);

   private:
    union {
        vector<Member> o;    /* object elements */
        vector<LeptValue> a; /* array elements */
        string s;            /* string elements */
        double n;            /* number */
    };
    e_types type;
};

struct Member {
    Member() = default;
    Member(const string& key, const LeptValue val) : k(key), v(val) {}
    string k;    /* Member key string */
    LeptValue v; /* Member LeptValue */
};

inline LeptValue::LeptValue(const LeptValue& v) { *this = v; }

inline LeptValue::~LeptValue() { this->freeVal(); }

inline LeptValue& LeptValue::operator=(const LeptValue& rhs) {
    if (this->type == rhs.type) {
        switch (rhs.type) {
            case NUMBER: this->n = rhs.n; break;
            case STRING: this->s = rhs.s; break;
            case ARRAY: this->a = rhs.a; break;
            case OBJECT: this->o = rhs.o; break;
            default: break;
        }
        return *this;
    }
    this->freeVal();
    switch (rhs.type) {
        case NUMBER: this->n = rhs.n; break;
        case STRING: this->set_string(rhs.s); break;
        case ARRAY: this->set_array(rhs.a); break;
        case OBJECT: this->set_object(rhs.o); break;
        default: break;
    }
    this->type = rhs.type;
    return *this;
}

inline void LeptValue::freeVal() {
    switch (this->type) {
        case STRING: this->s.~string(); break;
        case ARRAY: this->a.~vector(); break;
        case OBJECT: this->o.~vector(); break;
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

inline const string& LeptValue::get_string() const {
    assert(this->type == STRING);
    return this->s;
}

inline string& LeptValue::get_string() {
    assert(this->type == STRING);
    return this->s;
}

inline size_t LeptValue::get_string_length() const {
    assert(this->type == STRING);
    return this->s.size();
}

inline void LeptValue::init_string() {
    if (this->type == STRING) {
        this->s = "";
        return;
    }
    this->freeVal();
    new (&this->s) string("");
    this->type = STRING;
}

inline void LeptValue::set_string(const string& str) {
    if (this->type == STRING) {
        this->s = str;
        return;
    }
    this->freeVal();
    new (&this->s) string(str);
    this->type = STRING;
}

inline void LeptValue::init_array() {
    if (this->type == ARRAY) {
        this->a = vector<LeptValue>();
        return;
    }
    this->freeVal();
    new (&this->a) vector<LeptValue>();
    this->type = ARRAY;
}

inline void LeptValue::set_array(const vector<LeptValue>& arr) {
    if (this->type == ARRAY) {
        this->a = vector<LeptValue>();
        return;
    }
    this->freeVal();
    new (&this->a) vector<LeptValue>(arr);
    this->type = ARRAY;
}

inline size_t LeptValue::get_array_size() const {
    assert(this->type == ARRAY);
    return this->a.size();
}

inline size_t LeptValue::get_array_capacity() const {
    assert(this->type == ARRAY);
    return (this->a).capacity();
}

inline void LeptValue::shrink_array() {
    assert(this->type == ARRAY);
    (this->a).shrink_to_fit();
}

inline void LeptValue::clear_array() {
    assert(this->type == ARRAY);
    (this->a).clear();
}

inline const LeptValue& LeptValue::get_array_element(size_t index) const {
    assert(this->type == ARRAY);
    assert(index < this->get_array_size());
    return this->a[index];
}

inline LeptValue& LeptValue::get_array_element(size_t index) {
    assert(this->type == ARRAY);
    assert(index < this->get_array_size());
    return this->a[index];
}

inline void LeptValue::pushback_array_element(const LeptValue& v) {
    assert(this->type == ARRAY);
    (this->a).push_back(v);
}

inline void LeptValue::popback_array_element() {
    assert(this->type == ARRAY);
    (this->a).pop_back();
}

inline void LeptValue::insert_array_element(const LeptValue& v, size_t index) {
    assert(this->type == ARRAY && index <= (this->a).size());
    auto it = (this->a).begin() + index;
    (this->a).insert(it, v);
}

inline void LeptValue::erase_array_element(size_t _start, size_t _count) {
    assert(this->type == ARRAY && _start + _count <= (this->a).size());
    size_t num = 0;
    auto it = (this->a).begin() + _start;
    while (num < _count) {
        it = (this->a).erase(it);
        ++num;
    }
}

inline void LeptValue::init_object() {
    if (this->type == OBJECT) {
        this->o = vector<Member>();
        return;
    }
    this->freeVal();
    new (&this->o) vector<Member>();
    this->type = OBJECT;
}

inline void LeptValue::set_object(const vector<Member>& obj) {
    if (this->type == OBJECT) {
        this->o = vector<Member>();
        return;
    }
    this->freeVal();
    new (&this->o) vector<Member>(obj);
    this->type = OBJECT;
}

inline size_t LeptValue::get_object_size() const {
    assert(this->type == OBJECT);
    return this->o.size();
}

inline const string& LeptValue::get_object_key(size_t index) const {
    assert(this->type == OBJECT);
    assert(index < this->get_object_size());
    return (this->o)[index].k;
}

inline size_t LeptValue::get_object_key_length(size_t index) const {
    assert(this->type == OBJECT);
    assert(index < this->get_object_size());
    return (this->o)[index].k.size();
}

inline const LeptValue& LeptValue::get_object_value(size_t index) const {
    assert(this->type == OBJECT);
    assert(index < this->get_object_size());
    return this->o[index].v;
}

inline LeptValue& LeptValue::get_object_value(size_t index) {
    assert(this->type == OBJECT);
    assert(index < this->get_object_size());
    return this->o[index].v;
}

inline size_t LeptValue::get_object_index(const std::string& key) const {
    assert(this->type == OBJECT);
    size_t i = 0;
    for (const Member& mem : this->o) {
        if (mem.k == key) return i;
        ++i;
    }
    return KEY_NOT_EXIST;
}

inline void LeptValue::pushback_object_member(const string& key, const LeptValue& v) {
    assert(this->type == OBJECT);
    (this->o).push_back(Member(key, v));
}

inline void LeptValue::remove_object_member(size_t index) {
    assert(this->type == OBJECT && index < (this->o).size());
    (this->o).erase((this->o).begin() + index);
}

inline size_t LeptValue::get_object_capacity() const {
    assert(this->type == OBJECT);
    return (this->o).capacity();
}

inline void LeptValue::shrink_object() {
    assert(this->type == OBJECT);
    (this->o).shrink_to_fit();
}

inline void LeptValue::clear_object() {
    assert(this->type == OBJECT);
    (this->o).clear();
}

}  // namespace lept

#endif /* LEPTJSON_H */
