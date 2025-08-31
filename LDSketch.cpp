#include <algorithm>
#include <cstdint>
#include <climits>
#include <unordered_map>
#include <utility>

#include "LDSketch.h"
#include "result.h"

inline uint64_t ld_aware_hash_impl(const char *data, uint64_t n);

LDSketch::LDSketch(int col_num, int thresh)
    : col_num(col_num), T(thresh)
{
    for (int i = 0; i < ROW_NUM; ++i)
    {
        buckets[i] = new Bucket[col_num];
        for (int j = 0; j < col_num; ++j)
        {
            buckets[i][j].max_len = ARRAY_SIZE;
        }
    }
}

LDSketch::~LDSketch()
{
    for (int i = 0; i < ROW_NUM; ++i)
    {
        delete[] buckets[i];
    }
}

void LDSketch::clear()
{
    for (int i = 0; i < ROW_NUM; ++i)
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
    for (int i = 0; i < ROW_NUM; ++i)
    {
        int j = find_bucket(key, i);
        update_bucket(key, 1, i, j);
    }
}

void LDSketch::work(int n)
{
    for (int i = 0; i < ROW_NUM; ++i)
    {
        for (int j = 0; j < col_num; ++j)
        {
            int p = j * ARRAY_SIZE;
            for (auto it = buckets[i][j].array.begin(); it != buckets[i][j].array.end(); ++it)
            {
                mergename[n][i][p] = it->first;
                mergeresult1[n][i][p] = it->second;
                p++;
            }
            mergeresult2[n][i][j] = buckets[i][j].decrement;
            mergeresult3[n][i][j] = buckets[i][j].array.size();
        }
    }
}

int LDSketch::merge(int thresh, int opt)
{
    for (auto it = allflowname.begin(); it != allflowname.end(); ++it)
    {
        it->second = 0;
    }

    for (int i = 0; i < ROW_NUM; ++i)
    {
        for (int j = 0; j < col_num; ++j)
        {
            // Merge arrays with corrected frequencies
            std::unordered_map<std::string, int> temp;
            for (int k = 0; k < node_num; ++k)
            {
                for (int p = j * ARRAY_SIZE; p < j * ARRAY_SIZE + mergeresult3[k][i][j]; ++p)
                {
                    int val = mergeresult1[k][i][p] + mergeresult2[k][i][j];
                    if (temp.find(mergename[k][i][p]) != temp.end())
                    {
                        temp[mergename[k][i][p]] += val;
                    }
                    else
                    {
                        temp[mergename[k][i][p]] = val;
                    }
                }
            }

            // Truncate merged array to fixed size
            int cnt = 0;
            for (auto it = temp.begin(); it != temp.end(); ++it)
            {
                q[cnt].x = it->first;
                q[cnt].y = it->second;
                cnt++;
            }
            std::sort(q, q + cnt, cmp);
            

            buckets[i][j].array.clear();
            for (int k = 0; k < std::min(ARRAY_SIZE, cnt); ++k)
            {
                buckets[i][j].array[q[k].x] = q[k].y;
            }
            buckets[i][j].decrement = 0;
        }
    }

    int cnt = 0;
    for (auto it = allflowname.begin(); it != allflowname.end(); ++it)
    {
        int freq = up_estimate(it->first);
        it->second = freq;
        q[cnt].x = it->first;
        q[cnt].y = freq;
        cnt++;
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
    int col_idx = ld_aware_hash_impl(key_str.c_str(), key_str.length()) % col_num;
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
        // NOTE: different from original design, always replace when array is full
        // int k = bucket.total / T;
        // if ((k + 1) * (k + 2) - 1 > bucket.max_len)
        // {
        //     bucket.max_len = (k + 1) * (k + 2) - 1;
        //     bucket.array[key] = val;
        // }
        // else
        // {
        int array_min = std::min_element(bucket.array.begin(), bucket.array.end())->second;
        int cur_decrement = std::min(bucket.total, array_min);
        bucket.decrement += cur_decrement;
        for (auto it = bucket.array.begin(); it != bucket.array.end();)
        {
            it->second -= cur_decrement;
            if (it->second <= 0)
            {
                it = bucket.array.erase(it);
            }
            else
            {
                ++it;
            }
        }
        if (val > cur_decrement)
            bucket.array[key] = val - cur_decrement;
        // }
    }
    // assert(bucket.array.size() <= ARRAY_SIZE);
}

int LDSketch::low_estimate(const std::string &key) const
{
    int val = 0;
    for (int i = 0; i < ROW_NUM; ++i)
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
    for (int i = 0; i < ROW_NUM; ++i)
    {
        int j = find_bucket(key, i);
        auto it = buckets[i][j].array.find(key);
        // NOTE: different from original paper, skip the rows without the key
        // instead of counting them as 0
        // if (it == buckets[i][j].array.end())
        //     continue;
        // int cur_val = it->second + buckets[i][j].decrement;
        // TODO
        int cur_val = it != buckets[i][j].array.end()
                        ? it->second + buckets[i][j].decrement
                        : 0;
        val = std::min(val, cur_val);
    }
    // if (val == INT_MAX)
    //    val = 0;
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
