#include <vector>
#include <thread>
#include <cmath>
#include "utimer.cpp"

using namespace std;

int main(int argc, char **argv)
{
    int n = (argc == 1 ? 10 : atoi(argv[1])); // length of the vectors
    int nw = (argc == 2 ? 4 : atoi(argv[2])); // number of workers
    long elapsed_time = 0;

    vector<double> v(n), res(n);

    for (int i = 0; i < n; i++)
        v[i] = rand() % 100;

    auto g = [](double x)
    {
        for (int i = 0; i < 1000; i++)
            x = sin(x);
        return (x);
    }; // function used to map v

    auto body = [&](int k) { // body of the worker thread
        int delta = n / nw;
        int from = k * delta;                           // compute assigned iteration
        int to = (k == (nw - 1) ? n : (k + 1) * delta); // (poor load balancing)

        {
            utimer t1("seq");
            for (int i = from; i < to; i++) // then apply function to the
                res[i] = g(v[i]);           // assigned vectors
        }
        return;
    };

    vector<thread> pool;

    {
        utimer timer1("Parallel computation", &elapsed_time);
        for (int i = 0; i < nw; i++)
        {
            pool.push_back(thread(body, i));
        }

        for (int t = 0; t < nw; t++)
            pool[t].join();
    }

    {
        utimer timer2("Sequential computation", &elapsed_time);
        for (int i = 0; i < n; i++)
            res[i] = g(v[i]);
    }

    return 0;
}