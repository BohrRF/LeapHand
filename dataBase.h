#ifndef DATABASE_H_INCLUDED
#define DATABASE_H_INCLUDED

#include <vector>

static const int TPQN = 60;
class note
{
public:
    unsigned int channel;
    unsigned int velocity;
    unsigned int number;
    unsigned int tickSpan;
    void operator = (const note &rvar)
    {
        this->channel = rvar.channel;
        this->number = rvar.number;
        this->tickSpan = rvar.tickSpan;
        this->velocity = rvar.velocity;
    }
};

class tickNode
{
public:
    std::vector<note> notes;
    unsigned int tickOffset;
};

class beatSection
{
public:

    unsigned int tickBeatLength;
    unsigned int timeSigniture_num;
    unsigned int timeSigniture_den;

    std::vector<tickNode> tickSet;

    inline void setTimeSigniture(const unsigned int &num = 4, const unsigned int &den = 4)
    {
        tickBeatLength = TPQN * (4 / den); 
        timeSigniture_num = num;
        timeSigniture_den = den;           
    }
};

class musicData
{
public:
    std::vector<beatSection> beatSet;
};


#endif // DATABASE_H_INCLUDED
