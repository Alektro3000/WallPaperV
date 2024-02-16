#include "fft.h"


template<typename T>
void FastFurieTransform<T>::FastFurie(int n, std::complex<T> w, int offset, int depth)
{
    if (n == 1)
    {
        Allocated[depth][offset] = AllocatedForInp[depth][offset];
        return;
    }

    int k = n / 2;

    for (int i = 0; i < n; i++)
        AllocatedForInp[depth + 1][((i % 2) * k) + (i / 2) + offset] = AllocatedForInp[depth][offset + i];

    FastFurie(k, w* w, offset, depth + 1);
    FastFurie(k, w* w, offset + k, depth + 1);
    std::complex<T> wt = 1;
    for (int i = 0; i < n; i++) {
        Allocated[depth][offset + i] = Allocated[depth + 1][offset + (i % k)] + wt * Allocated[depth + 1][offset + k + (i % k)];
        wt *= w;
    }

}

template<typename T>
std::vector<std::complex<T>> FastFurieTransform<T>::evaluate(std::vector<T> p)
{
    int n = (int)p.size();
    int k = 0;
    int n1 = n;
    while (n1 > 0)
    {
        n1 /= 2;
        k++;
    }
    k += 2;
    AllocatedForInp.resize(k);
    for (int i = 0; i < k; i++)
        AllocatedForInp[i].resize(n);

    Allocated.resize(k);
    for (int i = 0; i < k; i++)
        Allocated[i].resize(n);


    for (int i = 0; i < n; i++)
        AllocatedForInp[0][i] = p[i];
    FastFurie(n, (std::complex<T>)(std::polar(1., 2 * (3.1415926) / p.size())), 0, 0);
    return Allocated[0];
}