#include <iostream>
#include "base64.h"
#include "EpollServer.h"

using namespace std;

int main()
{
    const string orig =
        "René Nyffenegger\n"
        "http://www.renenyffenegger.ch\n"
        "passion for data\n";

    string encoded = base64_encode(orig);
    string decoded = base64_decode(encoded);

    cout << encoded << endl;
    cout << decoded << endl;
    
    shared_ptr<EpollServer> epserver = make_shared<EpollServer>(8);
    epserver->run();

    while (true) {

    }
}