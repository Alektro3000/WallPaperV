#pragma once
#include <vector>
#include <complex>

template<typename T = float>
class FastFurieTransform
{
    void FastFurie(std::vector<std::complex<T> >& Allocated, int n, std::complex<T> w, int offset);
public:
    std::vector<std::complex<T>> evaluate(std::vector<std::complex<T> > p);
};

