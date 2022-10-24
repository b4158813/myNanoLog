#include <bits/stdc++.h>
using namespace std;

int main() {

    std::time_t time_now_us = std::chrono::duration_cast<std::chrono::microseconds>((std::chrono::high_resolution_clock::now().time_since_epoch())).count();
    std::time_t time_now_s = time_now_us / 1000000;
    time_now_s += 28800; // UTC + 8
    std::tm* time_now = std::gmtime(&time_now_s);

    char buff[32], usbuff[7];
    strftime(buff, 32, "%Y-%m-%d %H:%M:%S.", time_now);
    sprintf(usbuff, "%06llu", time_now_us % 1000000);
    cout << buff << usbuff << endl;
    return 0;
}