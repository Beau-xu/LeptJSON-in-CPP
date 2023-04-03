#include <iostream>
#include <string>
#include <vector>

using namespace std;

typedef struct value value;

struct value {
    string *s;
};

string* test(string* ps) {
    return new string(*ps);
}

int main() {
    string s = string("hello");
    string* ps = nullptr;
    cout << ps << '\n' << *ps;
    return 0;
}
