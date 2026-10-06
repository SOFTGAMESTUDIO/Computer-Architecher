#include <iostream>
using namespace std;

int main(){
    int a,b;
    cin >> a;
    cin >> b;

    a = a^b;  // 5 + 6
    b = a^b;  // 11 - 6 = 5
    a = a^b;  // 11-5 = 6

    cout << a << endl;
    cout << b << endl;

    return 0;
}  



