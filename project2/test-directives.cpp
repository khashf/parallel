#include <iostream>
using namespace std;

int main() {
    #if MODE == 0
    cout << "static" << endl;
    #elif MODE == 1
    cout << "dynamic" << endl;
    #else
    cout << "MODE directive undefined";
    #endif
    return 0;
}