#include <algorithm>
#include <numeric>
#include <climits>

#include "MVSketch.h"
#include "params.h"
#include "result.h"

MVSketch::MVSketch(int col_num)
    : col_num(col_num)
{
    for (int i = 0; i < MV_d; ++i)
    {
        buckets[i] = new Bucket[col_num];
        bobhash[i] = new BOBHash64(i + 1000);
    }
    clear();
}

MVSketch::~MVSketch()
{
    for (int i = 0; i < MV_d; i++)
    {
        delete[] buckets[i];
        delete bobhash[i];
    }
}

void MVSketch::clear()
{
    for (int i = 0; i < MV_d; i++)
    {
        for (int j = 0; j < col_num; j++)
        {
            buckets[i][j].v = 0;
            buckets[i][j].c = 0;
            buckets[i][j].key.clear();
        }
    }
}

void MVSketch::insert(const std::string &str)
{
    for (int i = 0; i < MV_d; i++)
    {
        int j = bobhash[i]->run(str.c_str(), KEY_LEN) % col_num;
        buckets[i][j].v++;
        if (buckets[i][j].key == str)
        {
            buckets[i][j].c++;
        }
        else if (buckets[i][j].key.empty())
        {
            buckets[i][j].key = str;
            buckets[i][j].c = 1;
        }
        else
        {
            buckets[i][j].c--;
            if (buckets[i][j].c < 0)
            {
                buckets[i][j].key = str;
                buckets[i][j].c = -buckets[i][j].c;
            }
        }
    }
}

void MVSketch::work(int n)
{
    for (int i = 0; i < MV_d; i++)
    {
        for (int j = 0; j < col_num; j++)
        {
            mergename[n][i][j] = buckets[i][j].key;
            mergeresult1[n][i][j] = buckets[i][j].v;
            mergeresult2[n][i][j] = buckets[i][j].c;
        }
    }
}

int MVSketch::merge(int thresh, int opt)
{
    for (auto it = allflowname.begin(); it != allflowname.end(); it++)
    {
        it->second = 0;
    }

    for (int i = 0; i < MV_d; ++i)
    {
        for (int j = 0; j < col_num; ++j)
        {
            int v_sum = 0;
            for (int k = 0; k < node_num; ++k)
            {
                v_sum += mergeresult1[k][i][j];
            }

            int max_estimate = -1, max_pos = -1;
            for (int x = 0; x < node_num; ++x)
            {
                int estimate = 0;
                for (int k = 0; k < node_num; ++k)
                {
                    estimate += mergename[x][i][j] == mergename[k][i][j]
                                    ? (mergeresult1[k][i][j] + mergeresult2[k][i][j]) / 2
                                    : (mergeresult1[k][i][j] - mergeresult2[k][i][j]) / 2;
                }
                if (estimate > max_estimate)
                {
                    max_pos = x;
                    max_estimate = estimate;
                }
            }

            buckets[i][j].key = mergename[max_pos][i][j];
            buckets[i][j].v = v_sum;
            buckets[i][j].c = std::max(2 * max_estimate - v_sum, 0);
        }
    }

    int CNT = 0;
    for (auto it = allflowname.begin(); it != allflowname.end(); it++)
    {
        int freq = INT_MAX;
        for (int i = 0; i < MV_d; i++)
        {
            int j = bobhash[i]->run(it->first.c_str(), KEY_LEN) % col_num;
            int estimate = buckets[i][j].key == it->first
                               ? (buckets[i][j].v + buckets[i][j].c) / 2
                               : (buckets[i][j].v - buckets[i][j].c) / 2;
            freq = std::min(estimate, freq);
        }

        q[CNT].x = it->first;
        q[CNT].y = freq;
        CNT++;

        it->second = freq;
    }
    std::sort(q, q + CNT, cmp);

    int bigflow;
    for (bigflow = 0; q[bigflow].y > thresh && bigflow < CNT; bigflow++)
        ;
    return bigflow;
}

std::string MVSketch::get_name()
{
    return "MVSketch";
}
