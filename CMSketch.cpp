#include <cassert>
#include <algorithm>
#include <string>

#include "CMSketch.h"
#include "params.h"
#include "result.h"

CMSketch::CMSketch(int Mem, int Knum)
    : M2(Mem), K(Knum)
{
    ss = new ssummary(K);
    ss->clear();
    bobhash = new BOBHash64(1005);
}

CMSketch::~CMSketch()
{
    delete ss;
    delete bobhash;
}

void CMSketch::clear()
{
    for (int i = 0; i < CM_d; i++)
        for (int j = 0; j <= M2 + 5; j++)
            HK[i][j].C = 0;
}

unsigned long long CMSketch::Hash(std::string ST)
{
    return (bobhash->run(ST.c_str(), ST.size()));
}

void CMSketch::insert(const std::string& x)
{
    // insert the package to CMsketch
    bool mon = false;
    int p = ss->find(x);
    if (p)
        mon = true;

    int minv = 0x7fffffff;
    std::string hash_key = x;
    hash_key.push_back('0');
    assert(hash_key.size() == KEY_LEN + 1);
    for (int i = 0; i < CM_d; i++)
    {
        hash_key[KEY_LEN] = '0' + i;
        auto hash = bobhash->run(hash_key.c_str(), KEY_LEN + 1) % (M2 - (2 * CM_d) + 2 * i + 3);
        HK[i][hash].C++;
        minv = std::min(minv, HK[i][hash].C);
    }
    
    // use space summary to map the id to a specific value
    if (!mon)
    {
        if (minv - (ss->getmin()) > 0 || ss->tot < K)
        {
            int i = ss->getid();
            ss->add2(ss->location(x), i);
            ss->str[i] = x;
            ss->sum[i] = minv;
            ss->link(i, 0);
            while (ss->tot > K)
            {
                int t = ss->Right[0];
                int tmp = ss->head[t];
                ss->cut(ss->head[t]);
                ss->recycling(tmp);
            }
        }
    }
    else
    {
        if (minv > ss->sum[p])
        {
            int tmp = ss->Left[ss->sum[p]];
            ss->cut(p);
            if (ss->head[ss->sum[p]])
                tmp = ss->sum[p];
            ss->sum[p] = minv;
            ss->link(p, tmp);
        }
    }
}

void CMSketch::work(int n)
{
    for (int i = N; i; i = ss->Left[i])
    {
        for (int j = ss->head[i]; j; j = ss->Next[j])
        {
            allflowname[ss->str[j]] += ss->sum[j];
        }
    }
}

int CMSketch::merge(int thresh, int opt)
{
    int CNT = 0;
    for (auto it = allflowname.begin(); it != allflowname.end(); it++)
    {
        q[CNT].x = it->first;
        q[CNT].y = it->second;
        CNT++;
    }
    std::sort(q, q + CNT, cmp);
    int bigflow;
    for (bigflow = 0; q[bigflow].y > thresh && bigflow < CNT; bigflow++)
        ;
    return bigflow;
}

std::pair<std::string, int> CMSketch::query_top(int k)
{
    return make_pair(q[k].x, q[k].y);
}

int CMSketch::query(const std::string& str)
{
    if (allflowname.find(str) != allflowname.end())
    {
        return allflowname[str];
    }
    else
    {
        return 0;
    }
}

std::string CMSketch::get_name()
{
    return "cmsketch";
}