#include <iostream>
#include <ctime>

using namespace std;

int main(int argc, char *argv[])
{
    const int N = 1024 * 1024;
    float x[N];

    for (int i = 0; i < N; i++)
        x[i] = static_cast<float>(i);

    for (int i = 0; i < N; i++)
        x[i] = x[i] * x[i];

    float sum = 0.0f;
    for (int i = 0; i < N; i++)
        sum += x[i];

    cout << sum << endl;

    return 0;
}
