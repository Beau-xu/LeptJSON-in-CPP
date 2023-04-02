#include <iostream>
#include <string>
#include <vector>

using namespace std;

typedef struct value value;

struct value {
    string *s;
};

int main() {
    string s = string("");
    string* ps = &s;
    cout << (*ps).size() << endl;


    return 0;
}
