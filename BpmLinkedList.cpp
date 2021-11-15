#include "BpmLinkedList.h"
#include <iostream>

using namespace std;

void BpmList::clear()
{
    first = pointer;
    last = first;
    n_count = 0;
}


int BpmList::push(const int64_t& time)
{
    pointer->bpm = 60 * (1000000.0 / (time - lastBeatTimeStamp));
    lastBeatTimeStamp = time;
    pointer = pointer->next;
    if (n_count == n_max)
    {
        first = pointer;
        last = pointer->before;
    }
    else if (n_count > 0)
    {
        last = pointer->before;
        n_count++;
    }
    else
    {
        first = pointer->before;
        last = first;
        last->bpm = 0;
        n_count++;
    }
    return n_count;
}



double BpmList::calAverage(const int& length) const
{
    int n = (length <= n_count ? length : n_count);
    if(n == 0) return 0;
    auto ptr = last;
    double bpm = 0;
    for (int i = 0; i < n; i++)
    {
        bpm += ptr->bpm;
        ptr = ptr->before;
    }
    
    return bpm / n;
}

int BpmList::count_node() const
{
    return n_count;
}


const double& BpmList::history(const int& his) const
{
    auto ptr = last;
    for (int i = his; i > 0; i--) ptr = ptr->before;
    return ptr->bpm;
}

void BpmList::output() const
{
    auto temp_ptr = last;

    // n_count-1 here because the speed in the first position is not usable
    for (int i = n_count - 1; i > 0; i--, temp_ptr = temp_ptr->before)
        cout << temp_ptr->bpm << endl;
}