#include <algorithm>
#include <climits>
#include <unordered_map>
#include <utility>

#include "LDSketch.h"
#include "result.h"

inline uint64_t ld_aware_hash_impl(const char *data, uint64_t n);

LDSketch::LDSketch(int row_num, int col_num, int counter_num, int thresh)
    : row_num(row_num), col_num(col_num), counter_num(counter_num)
{
    buckets = new Bucket *[row_num];
    for (int i = 0; i < row_num; ++i)
    {
        buckets[i] = new Bucket[col_num];
        for (int j = 0; j < col_num; ++j)
        {
            buckets[i][j].max_len = counter_num;
            buckets[i][j].T = thresh;
        }
    }
}

LDSketch::~LDSketch()
{
    for (int i = 0; i < row_num; ++i)
    {
        delete[] buckets[i];
    }
    delete[] buckets;
}

void LDSketch::clear()
{
    for (int i = 0; i < row_num; ++i)
    {
        for (int j = 0; j < col_num; ++j)
        {
            buckets[i][j].array.clear();
            buckets[i][j].total = 0;
            buckets[i][j].decrement = 0;
        }
    }
}

void LDSketch::insert(const std::string &key)
{
    for (int i = 0; i < row_num; ++i)
    {
        int j = find_bucket(key, i);
        update_bucket(key, 1, i, j);
    }
}

void LDSketch::work(int n)
{
    for (int i = 0; i < row_num; ++i)
    {
        for (int j = 0; j < col_num; ++j)
        {
            int idx = i * col_num + j;
            int cnt = 0;
            for (auto it = buckets[i][j].array.begin(); it != buckets[i][j].array.end(); ++it)
            {
                mergename[n][idx][cnt] = it->first;
                mergeresult1[n][idx][cnt] = it->second;
                cnt++;
            }
            mergeresult2[n][idx][0] = buckets[i][j].decrement;
            mergeresult3[n][idx][0] = buckets[i][j].array.size();
        }
    }
}

int LDSketch::merge(int thresh, int opt)
{
    for (int i = 0; i < row_num; ++i)
    {
        for (int j = 0; j < col_num; ++j)
        {
            std::unordered_map<std::string, int64_t> temp;
            int idx = i * col_num + j;
            for (int k = 0; k < node_num; ++k)
            {
                for (int l = 0; l < mergeresult3[k][idx][0]; ++l)
                {
                    int val = mergeresult1[k][idx][l] + mergeresult2[k][idx][0];
                    if (temp.find(mergename[k][idx][l]) != temp.end())
                    {
                        temp[mergename[k][idx][l]] += val;
                    }
                    else
                    {
                        temp[mergename[k][idx][l]] = val;
                    }
                }
            }
            buckets[i][j].array = std::move(temp);
            buckets[i][j].decrement = 0;
        }
    }

    int cnt = 0;
    for (auto it = allflowname.begin(); it != allflowname.end(); ++it)
    {
        int freq = up_estimate(it->first);
        if (freq >= thresh)
        {
            q[cnt].x = it->first;
            q[cnt].y = freq;
            cnt++;
        }
    }
    std::sort(q, q + cnt, cmp);

    int bigflow = 0;
    while (q[bigflow].y > thresh && bigflow < cnt)
        bigflow++;
    return bigflow;
}

std::string LDSketch::get_name()
{
    return "LDSketch";
}

int LDSketch::find_bucket(const std::string &key, int row_idx) const
{
    std::string key_str = key;
    const char *p = reinterpret_cast<const char *>(&row_idx);
    key_str.append(p, sizeof(int));
    int col_idx = ld_aware_hash_impl(key_str.c_str(), key_str.length()) % row_num;
    return col_idx;
}

void LDSketch::update_bucket(const std::string &key, int val, int row_idx, int col_idx)
{
    Bucket &bucket = buckets[row_idx][col_idx];
    bucket.total += val;
    if (bucket.array.find(key) != bucket.array.end())
    {
        bucket.array[key] += val;
    }
    else if (bucket.array.size() < bucket.max_len)
    {
        bucket.array[key] = val;
    }
    else
    {
        int k = bucket.total / bucket.T;
        if ((k + 1) * (k + 2) - 1 > bucket.max_len)
        {
            // Expand and insert
            bucket.max_len = (k + 1) * (k + 2) - 1;
            bucket.array[key] = val;
        }
        else
        {
            int64_t array_min = std::min_element(bucket.array.begin(), bucket.array.end())->second;
            int64_t cur_decrement = std::min(bucket.total, array_min);
            bucket.decrement += cur_decrement;
            for (auto it = bucket.array.begin(); it != bucket.array.end(); ++it)
            {
                it->second -= cur_decrement;
                // TODO: check if it is valid after erase
                if (it->second < 0)
                    bucket.array.erase(it);
            }
            if (val > cur_decrement)
                bucket.array[key] = val - cur_decrement;
        }
    }
}

int LDSketch::low_estimate(const std::string &key) const
{
    int val = 0;
    for (int i = 0; i < row_num; ++i)
    {
        int j = find_bucket(key, i);
        auto it = buckets[i][j].array.find(key);
        int cur_val = it != buckets[i][j].array.end() ? it->second : 0;
        val = std::max(val, cur_val);
    }
    return val;
}

int LDSketch::up_estimate(const std::string &key) const
{
    int val = INT_MAX;
    for (int i = 0; i < row_num; ++i)
    {
        int j = find_bucket(key, i);
        auto it = buckets[i][j].array.find(key);
        int cur_val = it != buckets[i][j].array.end()
                          ? it->second + buckets[i][j].decrement
                          : 0;
        val = std::min(val, cur_val);
    }
    return val;
}

template <uint64_t BASE, uint64_t SCALE, uint64_t HARDENER>
static uint64_t ld_aware_hash(const char *data, uint64_t n)
{
    uint64_t hash = BASE;
    while (n)
    {
        hash *= SCALE;
        hash += *data++;
        n--;
    }
    return hash ^ HARDENER;
}

inline uint64_t ld_aware_hash_impl(const char *data, uint64_t n)
{
    return ld_aware_hash<388650253, 388650319, 1176845762>(data, n);
}