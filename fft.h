#pragma once
#include <vector>
#include <complex>

template<typename T = float>
class FastFurieTransform
{
    std::vector<std::vector<std::complex<T>> > Allocated;
    std::vector<std::vector<std::complex<T>> > AllocatedForInp;
    void FastFurie(int n, std::complex<T> w, int offset, int depth);
public:
    std::vector<std::complex<T>> evaluate(std::vector<T> p);
};

