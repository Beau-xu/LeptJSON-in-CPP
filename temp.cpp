#include <iostream>
#include <string>
#include <vector>

using namespace std;

typedef struct value value;

struct value {
    string *s;
};

int main() {
    string s("hello");
    vector<value> vecVal;
    do {
        value val;
        val.s = new string(s);
        vecVal.push_back(val);
        delete val.s;
    } while(0);
    cout << *vecVal[0].s << endl;
    return 0;
}
