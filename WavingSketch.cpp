#include <cstdlib>
#include <climits>

#include "WavingSketch.h"
#include "result.h"

bool WavingSketch::is_clear = false;

static constexpr int SIGN[2] = {1, -1};

WavingSketch::WavingSketch(int bucket_num)
    : bucket_num(bucket_num)
{
    buckets = new Bucket[bucket_num];
    // TODO: change seeds
    bobhash_bucket = new BOBHash32(995);
    bobhash_s = new BOBHash32(1000);
    bobhash_counter = new BOBHash32(1001);
}

WavingSketch::~WavingSketch()
{
    delete[] buckets;
    delete bobhash_bucket;
    delete bobhash_s;
    delete bobhash_counter;
}

void WavingSketch::clear()
{
    for (int i = 0; i < bucket_num; ++i)
    {
        for (int j = 0; j < CELL_NUM; ++j)
        {
            buckets[i].cells[j].id.clear();
            buckets[i].cells[j].freq = 0;
        }
        for (int j = 0; j < COUNTER_NUM; ++j)
        {
            buckets[i].counters[j] = 0;
        }
    }
}

void WavingSketch::insert(const std::string &key)
{
    int bucket_pos = bobhash_bucket->run(key.c_str(), key.length()) % bucket_num;
    Bucket &bucket = buckets[bucket_pos];

    int sign_choice = bobhash_s->run(key.c_str(), key.length()) & 1;
    int sign = SIGN[sign_choice];

    int counter_pos = bobhash_counter->run(key.c_str(), key.length()) % COUNTER_NUM;

    // The error free item's counter is negative, which is a trick to
    // be differentiated from items which are not error free.
    int min_freq = INT_MAX, min_pos = -1;
    for (int i = 0; i < CELL_NUM; ++i)
    {
        if (bucket.cells[i].id == key)
        {
            // Case 1: item exists in bucket
            if (bucket.cells[i].freq < 0)
            {
                // flag is false (error free item)
                bucket.cells[i].freq--;
            }
            else
            {
                // flag is true (error item)
                bucket.cells[i].freq++;
                bucket.counters[counter_pos] += sign;
            }
            return;
        }
        else if (bucket.cells[i].freq == 0)
        {
            // Case 2: item does not exist in bucket, and bucket is NOT full
            // TODO: return is_empty = true?
            bucket.cells[i].id = key;
            bucket.cells[i].freq = -1;
            return;
        }

        int freq_val = std::abs(bucket.cells[i].freq);
        if (freq_val < min_freq)
        {
            min_freq = freq_val;
            min_pos = i;
        }
    }

    // Case 3: item does not exist in bucket, and bucket is full
    // Insert into counter
    bucket.counters[counter_pos] += sign;

    // Replace the item with minimum frequency if the new item's
    // estimated frequency is larger than the minimum frequency
    int estimated_freq = bucket.counters[counter_pos] * sign;
    if (estimated_freq >= int(min_freq * FACTOR))
    {
        Cell &min_cell = bucket.cells[min_pos];
        if (min_cell.freq < 0)
        {
            // If the item with minimum frequency has true flag (error free item),
            // insert it into the counter
            int min_counter_pos = bobhash_counter->run(min_cell.id.c_str(), min_cell.id.length()) % COUNTER_NUM;
            int min_choice = bobhash_s->run(min_cell.id.c_str(), min_cell.id.length()) & 1;
            int min_sign = SIGN[min_choice];
            bucket.counters[min_counter_pos] -= min_cell.freq * min_sign; // since freq is negative, to add abs(freq) is the same as to substract freq
        }
        // Replace
        min_cell.id = key;
        min_cell.freq = min_freq + 1;
    }
}

void WavingSketch::work(int n)
{
    if (!is_clear)
    {
        for (auto it = allflowname.begin(); it != allflowname.end(); ++it)
            it->second = 0;
        is_clear = true;
    }

    for (int i = 0; i < bucket_num; ++i)
    {
        for (int j = 0; j < CELL_NUM; ++j)
        {
            if (buckets[i].cells[j].freq == 0)
                break;
            allflowname[buckets[i].cells[j].id] += std::abs(buckets[i].cells[j].freq);
        }
    }
}

int WavingSketch::merge(int thresh, int opt)
{
    int cnt = 0;
    for (auto it = allflowname.begin(); it != allflowname.end(); it++)
    {
        q[cnt].x = it->first;
        q[cnt].y = it->second;
        cnt++;
    }
    std::sort(q, q + cnt, cmp);
    int bigflow = 0;
    while (q[bigflow].y > thresh && bigflow < cnt)
        bigflow++;
    return bigflow;
}

std::string WavingSketch::get_name()
{
    return "WavingSketch";
}