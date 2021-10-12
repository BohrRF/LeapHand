#ifndef FOURIER_H_INCLUDED
#define FOURIER_H_INCLUDED

#include <memory>
#include <vector>

#include "LinkedList.h"

static const int N = 128;
static const double PI = 3.14159265358979323846;

static inline void swap(Complex &a, Complex &b)
{
    Complex t;
    t = a;
    a = b;
    b = t;
}

class Fourier
{
    Complex *x;
    Clist data_list;
    double average;
    void bitrp(Complex x[], int n) const;

public:
    Fourier(const int &length):data_list(length)
    {
        x = new Complex[length];
        average = 0;
    }
    ~Fourier()
    {
        delete [] x;
    }
    void push(const int64_t& time, const double& posx, const double& posy);
    void FFT() const;
    void reset();

    double getAverage() const;
    void find_max_freq(double list[], const int &n) const;
    void outSpec() const;
    int getListLength() const;
    const Cdata& history(const int& his = 0);
    std::unique_ptr<double[]> getSpec() const;
    double getSpeedVariance(const int64_t& tm) const;
    int64_t calCurAccel(int64_t startTimeStamp);
};

#endif // FOURIER_H_INCLUDED
