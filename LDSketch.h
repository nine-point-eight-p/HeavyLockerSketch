#ifndef LDSKETCH_H
#define LDSKETCH_H

#include <unordered_map>
#include <cstdint>

#include "BaseSketch.h"

class LDSketch : sketch::BaseSketch
{
private:
    struct Bucket
    {
        // A_{i,j}: associative array
        // TODO: LD-aware hash?
        std::unordered_map<std::string, int64_t> array;
        // V_{i,j}: total sum
        int64_t total;
        // l_{i,j}: max length of counters allowed
        uint32_t max_len;
        // e_{i,j}: total number of decrement
        uint32_t decrement;
        // T: expansion parameter
        int64_t T;
    };

    Bucket **buckets;
    int row_num, col_num;
    int counter_num;

    int find_bucket(const std::string &key, int row_idx) const;
    void update_bucket(const std::string &key, int val, int row_idx, int col_idx);

public:
    LDSketch(int row_num, int col_num, int counter_num, int thresh);
    ~LDSketch();

    void clear() override;
    void insert(const std::string &key) override;
    void work(int n) override;
    int merge(int thresh, int opt = 0) override;
    std::string get_name() override;

    int low_estimate(const std::string &key) const;
    int up_estimate(const std::string &key) const;
};

#endif