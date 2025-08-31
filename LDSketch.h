#ifndef LDSKETCH_H
#define LDSKETCH_H

#include <unordered_map>

#include "BaseSketch.h"

class LDSketch : public sketch::BaseSketch
{
public:
    // Static parameters
    static constexpr int ROW_NUM = 4;
    static constexpr int ARRAY_SIZE = 8;

private:
    struct Bucket
    {
        // A_{i,j}: associative array
        // TODO: LD-aware hash?
        std::unordered_map<std::string, int> array;
        // V_{i,j}: total sum
        int total;
        // l_{i,j}: max length of counters allowed
        int max_len;
        // e_{i,j}: total number of decrement
        int decrement;
    };
    
    Bucket *buckets[ROW_NUM];
    int col_num;
    // T: expansion parameter
    int T;

    int find_bucket(const std::string &key, int row_idx) const;
    void update_bucket(const std::string &key, int val, int row_idx, int col_idx);

public:
    LDSketch(int col_num, int thresh);
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