#include <iostream>
#include <chrono>

using namespace std;

int main(int argc, char *argv[])
{
    const int N = 1024 * 1024;
    float x[N];

    auto t0 = chrono::system_clock::now();
    for (int i = 0; i < N; i++)
        x[i] = static_cast<float>(i);
    auto t1 = chrono::system_clock::now();
    auto e = chrono::duration_cast<chrono::microseconds>(t1 - t0).count();
    cout << "Elapsed " << e << " usecs" << endl;

    t0 = chrono::system_clock::now();
    for (int i = 0; i < N; i++)
        x[i] = x[i] * x[i];
    t1 = chrono::system_clock::now();
    e = chrono::duration_cast<chrono::microseconds>(t1 - t0).count();
    cout << "Elapsed " << e << " usecs" << endl;

    t0 = chrono::system_clock::now();
    float sum = 0.0f;
    for (int i = 0; i < N; i++)
        sum += x[i];
    cout << sum << endl;
    t1 = chrono::system_clock::now();
    e = chrono::duration_cast<chrono::microseconds>(t1 - t0).count();
    cout << "Elapsed " << e << " usecs" << endl;

    return 0;
}
