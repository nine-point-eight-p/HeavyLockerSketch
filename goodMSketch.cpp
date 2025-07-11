#include <algorithm>
#include <string>
#include <unordered_map>
#include <utility>

#include "goodMSketch.h"
#include "params.h"
#include "result.h"

int depth = 6;
int hashnum = 1;
double lock_thre = 0.5;

MSketch::MSketch(uint _bucket, double _hh_ratio)
{
    hh_ratio = _hh_ratio * lock_thre;
    bucket_num = _bucket;
    bobhash = new BOBHash64(1005);
    for (int i = 0; i < 1; i++)
    {
        bucket = new bucket_t[bucket_num];
    }
}

unsigned long long MSketch::Hash(std::string ST)
{
    return (bobhash->run(ST.c_str(), ST.size()));
}

void MSketch::clear()
{
    for (int i = 0; i < bucket_num; i++)
    {
        for (int j = 0; j < depth; j++)
        {
            bucket[i].fingerprint[j] = "";
            bucket[i].counter[j] = 0;
        }
    }
}

bool MSketch::plus(bucket_t *b, int j)
{
    b->counter[j - 1]++;
    if (j == depth)
        return true;
    if (b->counter[j - 1] > b->counter[j])
    {
        std::swap(b->counter[j - 1], b->counter[j]);
        std::swap(b->fingerprint[j - 1], b->fingerprint[j]);
    }
    return true;
}

void MSketch::insert(const std::string &key)
{
    total_packet++;
    char *fp = const_cast<char *>(key.c_str());
    int point = 0, min = INT32_MAX;
    for (int i = 0; i < hashnum; i++)
    {
        fp[KEY_LEN] = '0' + i;
        hash[i] = bobhash->run(fp, KEY_LEN + 1) % (bucket_num);
        for (int j = depth; j >= 1; j--)
        {
            if (bucket[hash[i]].fingerprint[j - 1] == key && bucket[hash[i]].counter[j - 1] != 0)
            {
                plus(&bucket[hash[i]], j);
                return;
            }
            else if (bucket[hash[i]].fingerprint[j - 1] == "" && bucket[hash[i]].counter[j - 1] == 0)
            {
                bucket[hash[i]].fingerprint[j - 1] = key;
                plus(&bucket[hash[i]], j);
                return;
            }
        }
        if (min > bucket[hash[i]].counter[0])
        {
            min = bucket[hash[i]].counter[0];
            point = i;
        }
    }

    if (min < (total_packet * hh_ratio))
    {
        newContext.ContextInterface(bucket[hash[point]].counter[0], bucket[hash[point]].fingerprint[0], key);
        return;
    }
    return;
}

std::pair<std::string, int> MSketch::query_top(int k)
{
    return make_pair(q[k].x, q[k].y);
}

int MSketch::query(const std::string& str)
{
    int result = 0;
    char *fp = const_cast<char *>(str.c_str());
    for (int i = 0; i < hashnum; i++)
    {
        fp[KEY_LEN] = '0' + i;
        hash[i] = bobhash->run(fp, KEY_LEN + 1) % (bucket_num);
        for (int j = depth - 1; j >= 0; j--)
        {
            if (bucket[hash[i]].fingerprint[j] == str)
                result += bucket[hash[i]].counter[j];
        }
    }
    return result;
}

void MSketch::work(int n)
{
    for (int i = 0; i < bucket_num; i++)
    {
        for (int j = 0; j < depth; j++)
        {
            mergename[n][j][i] = bucket[i].fingerprint[j];
            mergeresult1[n][j][i] = bucket[i].counter[j];
        }
    }
}

int MSketch::merge(int thresh, int opt)
{
    int CNT = 0;
    for (auto it = allflowname.begin(); it != allflowname.end(); it++)
    {
        it->second = 0;
    }
    for (int i = 0; i < bucket_num; i++)
    {
        CNT = i * depth;
        std::unordered_map<std::string, int> temp;
        for (int j = 0; j < node_num; j++)
        {
            for (int d = depth - 1; d >= 0; d--)
            {
                if (temp.find(mergename[j][d][i]) != temp.end())
                {
                    temp[mergename[j][d][i]] += mergeresult1[j][d][i];
                }
                else
                {
                    temp[mergename[j][d][i]] = mergeresult1[j][d][i];
                }
            }
        }
        for (auto it = temp.begin(); it != temp.end(); it++)
        {
            q[CNT].x = it->first;
            q[CNT].y = it->second;
            CNT++;
        }
        std::sort(q + i * depth, q + CNT, cmp);

        for (int d = depth - 1; d >= 0; d--)
        {
            bucket[i].fingerprint[d] = q[i * depth + d].x;
            bucket[i].counter[d] = q[i * depth + d].y;
        }
    }
    std::sort(q, q + CNT, cmp);
    for (auto it = allflowname.begin(); it != allflowname.end(); it++)
    {
        it->second = 0;
    }
    for (int i = 0; i < CNT; i++)
    {
        allflowname[q[i].x] += q[i].y;
    }
    int bigflow = 0;
    for (auto it = allflowname.begin(); it != allflowname.end(); it++)
    {
        if (it->second > thresh)
            bigflow++;
    }
    return bigflow;
}

std::string MSketch::get_name()
{
    return "Mysketch";
}

MSketch::~MSketch()
{
    for (int i = 0; i < 1; i++)
    {
        delete[] bucket;
    }
    for (int i = 0; i < 1; i++)
    {
        delete bobhash;
    }
}
