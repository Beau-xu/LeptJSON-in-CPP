#include <iostream>
#include <string>
#include <vector>

using namespace std;

typedef struct value value;

struct value {
    string *s;
};

int main() {
    string* s = nullptr;
    delete s;


    return 0;
}
