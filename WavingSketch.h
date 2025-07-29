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
    static constexpr int CELL_NUM = 16;
    static constexpr int COUNTER_NUM = 10;
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

public:
    WavingSketch(int bucket_num);
    ~WavingSketch();

    void clear() override;
    void insert(const std::string &str) override;
    // std::pair<std::string, int> query_top(int k) override;
    // int query(const std::string& str) override;
    void work(int n) override;
    int merge(int thresh, int opt = false) override;
    std::string get_name() override;
};

#endif
