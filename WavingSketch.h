#ifndef WAVING_H
#define WAVING_H

#include <time.h>
#include <bitset>
#include <algorithm>
#include <random>

#include "BaseSketch.h"
#include "BOBHash32.h"

class WavingSketch : public sketch::BaseSketch
{
public:
    // Static parameters
    static constexpr int CELL_NUM = 8;
    static constexpr int COUNTER_NUM = 1;
    static constexpr double FACTOR = 1.0;

private:
    struct Cell
    {
        std::string id;
        int freq;
    };
    struct Bucket
    {
        Cell cells[CELL_NUM];
        int counters[COUNTER_NUM];
    };

    Bucket *buckets;
    int bucket_num;
    BOBHash32 *bobhash_bucket, *bobhash_s, *bobhash_counter;

    static bool is_clear;

public:
    WavingSketch(int bucket_num);
    ~WavingSketch();

    void clear() override;
    void insert(const std::string &str) override;
    void work(int n) override;
    int merge(int thresh, int opt = false) override;
    std::string get_name() override;
};

#endif
