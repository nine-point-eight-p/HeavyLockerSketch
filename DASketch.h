#ifndef _dasketch_H
#define _dasketch_H

#include "BaseSketch.h"
#include "params.h"
#include "BOBHash64.h"

#define CMM_d 1
#define TOP_d 4
#define opt1 0 // define the query method

class DASketch : public sketch::BaseSketch
{
private:
    struct cell
    {
        std::string ID;
        int Cs;
        int Cr;
    };
    struct bucket
    {
        cell cells[TOP_d];
    };
    bucket *bk;
    struct node
    {
        int C;
    } HK[CMM_d][MAX_MEM + 10];
    BOBHash64 *bobhash;
    int M2;
    int total;

public:
    DASketch(int M2);
    ~DASketch();

    void clear() override;
    void insert(const std::string &x) override;
    void work(int n) override;
    int merge(int thresh, int opt = 0) override;
    std::string get_name() override;

    unsigned long long Hash(std::string ST);
};

#endif