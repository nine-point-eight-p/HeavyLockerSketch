#include <algorithm>

#include "MVSketch.h"
#include "params.h"
#include "result.h"

MVSketch::MVSketch(int M2)
    : M2(M2)
{
    sum = 0;
    bobhash = new BOBHash64(995);
    for (int i = 0; i < MV_d; i++)
        bobhash_[i] = new BOBHash64(i + 1000);
    clear();
}

MVSketch::~MVSketch()
{
    delete bobhash;
    for (int i = 0; i < MV_d; i++)
        delete bobhash_[i];
}

void MVSketch::clear()
{
    for (int i = 0; i < MV_d; i++)
        for (int j = 0; j <= M2 + 5; j++)
        {
            HK[i][j].C = HK[i][j].S = 0;
            HK[i][j].FP = "";
        }
}

void MVSketch::insert(const std::string& str)
{
    sum += 1;
    int tmpmaxv = 0;
    int maxv = 0;
    unsigned int hash[MV_d];
    for (int i = 0; i < MV_d; i++)
        hash[i] = bobhash_[i]->run(str.c_str(), KEY_LEN) % M2;

    for (int i = 0; i < MV_d; i++)
    {
        HK[i][hash[i]].S++;
        if (HK[i][hash[i]].FP == str)
        {
            HK[i][hash[i]].C++;
        }
        else if (HK[i][hash[i]].FP == "")
        {
            HK[i][hash[i]].FP = str;
            HK[i][hash[i]].C = 1;
        }
        else
        {
            HK[i][hash[i]].C--;
            if (HK[i][hash[i]].C < 0)
            {
                HK[i][hash[i]].FP = str;
                HK[i][hash[i]].C = -HK[i][hash[i]].C;
            }
        }
    }
}

std::pair<std::string, int> MVSketch::query_top(int k)
{
    return make_pair(q[k].x, q[k].y);
}

int MVSketch::query(const std::string& str)
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

void MVSketch::work(int n)
{
    for (int d = 0; d < MV_d; d++)
    {
        for (int w = 0; w < M2; w++)
        {
            mergename[n][d][w] = HK[d][w].FP;
            mergeresult1[n][d][w] = HK[d][w].C;
            mergeresult2[n][d][w] = HK[d][w].S;
        }
    }
}

int MVSketch::merge(int thresh, int opt)
{
    long countarr[node_num];
    long sumarr[node_num];
    std::string keyarr[node_num];
    long est[node_num];
    for (auto it = allflowname.begin(); it != allflowname.end(); it++)
    {
        it->second = 0;
    }
    for (int d = 0; d < MV_d; d++)
    {
        for (int w = 0; w < M2; w++)
        {
            long total = 0;
            for (int i = 0; i < node_num; i++)
            {
                countarr[i] = mergeresult1[i][d][w];
                sumarr[i] = mergeresult2[i][d][w];
                keyarr[i] = mergename[i][d][w];
                total += sumarr[i];
            }
            int pointer = 0;
            long counttmp = 0;
            for (int i = 0; i < node_num; i++)
            {
                est[i] = 0;
                for (int j = 0; j < node_num; j++)
                {
                    if (keyarr[i] == keyarr[j])
                    {
                        est[i] += (sumarr[j] + countarr[j]) / 2;
                    }
                    else
                    {
                        est[i] += (sumarr[j] - countarr[j]) / 2;
                    }
                }
                if (est[i] > est[pointer])
                    pointer = i;
            }
            counttmp = 2 * est[pointer] - total;
            counttmp = counttmp > 0 ? counttmp : 0;
            if (counttmp == 0)
            {
                HK[d][w].FP = keyarr[pointer];
                HK[d][w].C = counttmp;
                HK[d][w].S = 2 * est[pointer];
            }
            else
            {
                HK[d][w].FP = keyarr[pointer];
                HK[d][w].C = counttmp;
                HK[d][w].S = total;
            }
        }
    }
    unsigned int hash[MV_d];
    int CNT = 0;
    for (auto it = allflowname.begin(); it != allflowname.end(); it++)
    {
        std::string fname = it->first;
        int result = 0;
        for (int i = 0; i < MV_d; i++)
        {
            hash[i] = bobhash_[i]->run(fname.c_str(), KEY_LEN) % M2;
            int tempnum = 0;
            if (HK[i][hash[i]].FP == fname)
            {
                tempnum = (HK[i][hash[i]].S - HK[i][hash[i]].C) / 2 + HK[i][hash[i]].C;
            }
            else
            {
                tempnum = (HK[i][hash[i]].S - HK[i][hash[i]].C) / 2;
            }
            if (i == 0)
                result = tempnum;
            else
                result = std::min(tempnum, result);
        }
        q[CNT].x = fname;
        q[CNT].y = result;
        CNT++;
        it->second = result;
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
