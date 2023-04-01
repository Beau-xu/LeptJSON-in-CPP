#include <iostream>
#include <string>

using namespace std;

int main() {
    string s("hello\0world");
    cout << s << '\n';
    string s2(s);
    cout << s2 << endl;
}
