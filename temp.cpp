#include <iostream>
#include <string>
#include <vector>

using namespace std;

typedef struct value value;

struct value {
    string *s;
};

int main() {
    string s = string("hello");
    s.pop_back();
    cout << s << endl;


    return 0;
}
