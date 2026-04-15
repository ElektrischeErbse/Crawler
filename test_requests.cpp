//
// Created by tom on 26-4-14.
//

#include <requests/requests.h>
#include <iostream>

using namespace std;

int main()
{
    Session s;
    auto r = s.get("https://www.baidu.com");
    if (r.ok()) {
        cout << r.text << endl;
    }
    s.download("https://pic4.zhimg.com/v2-c34c61a90095abb8713de9d1dca7ec7b_r.jpg", "xxx.jpg");
    return 0;
}