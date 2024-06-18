#include "fft.h"


template<typename T>
void FastFurieTransform<T>::FastFurie(std::vector<std::complex<T> >& Allocated, int n, std::complex<T> w, int offset)
{
    if (n == 1)
        return;

    int k = n/2;

    FastFurie(Allocated, k, w* w, offset);
    FastFurie(Allocated, k, w* w, offset + k);

    std::complex<T> wt = 1;
    for (int i = 0; i < k; i++) {
        auto t = wt * Allocated[offset+k+i];

        Allocated[offset + i + k] = Allocated[offset + i] - t;
        Allocated[offset + i] = Allocated[offset + i] + t;

        wt *= w;
    }

}

template<typename T>
std::vector<std::complex<T>> FastFurieTransform<T>::evaluate(std::vector< std::complex<T> > p)
{


    int n = (int)p.size();


    for (int i = 0; i < n; i++) {
        int k = (n-1) ^ i;

        //int k = 0;
        //for (int l = 0; l < logn; l++)
        //    k |= ((i >> l & 1) << (logn - l - 1));

        if (i < k)
            std::swap(p[i], p[k]);
    }
    FastFurie(p, n, (std::complex<T>)(std::polar(1.,-2 * (3.1415926) / p.size())), 0);
    return p;
}