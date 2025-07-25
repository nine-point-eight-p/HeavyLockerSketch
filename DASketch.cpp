#include <algorithm>
#include <string>
#include <unordered_map>
#include <cstdlib>
#include <climits>

#include "DASketch.h"
#include "params.h"
#include "result.h"

DASketch::DASketch(int M2) : M2(M2)
{
    bk = new bucket[M2];
    bobhash = new BOBHash64(1005);
    total = 0;
}

DASketch::~DASketch()
{
    delete[] bk;
    delete bobhash;
}

void DASketch::clear()
{
    for (int i = 0; i < M2; i++)
        for (int j = 0; j < TOP_d; j++)
        {
            bk[i].cells[j].ID = "";
            bk[i].cells[j].Cs = 0;
            bk[i].cells[j].Cr = 0;
        }
    for (int i = 0; i < CMM_d; i++)
        for (int j = 0; j <= M2 + 5; j++)
            HK[i][j].C = 0;
    total = 0;
}

unsigned long long DASketch::Hash(std::string ST)
{
    return (bobhash->run(ST.c_str(), ST.size()));
}

void DASketch::insert(const std::string& x)
{
    const char *fp = x.c_str();
    int h = bobhash->run(fp, KEY_LEN) % M2;
    bool match = false;
    bool empty = false;
    int minv = 0x7fffffff;
    int minp = -1;
    for (int i = 0; i < TOP_d; i++)
    {
        if (bk[h].cells[i].ID == x)
        {
            bk[h].cells[i].Cs++;
            bk[h].cells[i].Cr++;
            match = true;
            break;
        }
    }
    if (!match)
    {
        for (int i = 0; i < TOP_d; i++)
        {
            if (bk[h].cells[i].ID == "")
            {
                bk[h].cells[i].ID = x;
                bk[h].cells[i].Cs = 1;
                bk[h].cells[i].Cr = 1;
                empty = true;
                break;
            }
        }
    }
    if (!match && !empty)
    {
        for (int i = 0; i < TOP_d; i++)
        {
            if (bk[h].cells[i].Cs < minv)
            {
                minv = bk[h].cells[i].Cs;
                minp = i;
            }
        }
        double p = 1.0 / (minv + 1);
        double r = rand() / double(RAND_MAX);
        if (r < p)
        {
            std::string evicted = bk[h].cells[minp].ID;
            int evicted_cr = bk[h].cells[minp].Cr;
            bk[h].cells[minp].ID = x;
            bk[h].cells[minp].Cs = minv + 1;
            bk[h].cells[minp].Cr = 1;

            evicted.push_back('0');
            for (int i = 0; i < CMM_d; i++)
            {
                evicted[KEY_LEN] = '0' + i;
                auto hash = bobhash->run(evicted.c_str(), KEY_LEN + 1) % (M2 - (2 * CMM_d) + 2 * i + 3);
                HK[i][hash].C += evicted_cr;
            }
            total += evicted_cr;
        }
        else
        {
            std::string hash_key = x;
            hash_key.push_back('0');
            for (int i = 0; i < CMM_d; i++)
            {
                hash_key[KEY_LEN] = '0' + i;
                auto hash = bobhash->run(hash_key.c_str(), KEY_LEN + 1) % (M2 - (2 * CMM_d) + 2 * i + 3);
                HK[i][hash].C++;
            }
            total++;
        }
    }
}

void DASketch::work(int n)
{
    for (int i = 0; i < TOP_d; i++)
    {
        for (int j = 0; j < M2; j++)
        {
            mergename[n][i][j] = bk[j].cells[i].ID;
            mergeresult1[n][i][j] = bk[j].cells[i].Cs;
            mergeresult2[n][i][j] = bk[j].cells[i].Cr;
        }
    }
    for (int i = 0; i < CMM_d; i++)
    {
        for (int j = 0; j < M2; j++)
        {
            mergeresult3[n][i][j] = HK[i][j].C;
        }
    }
    totalnum[n] = total;
}

int DASketch::merge(int thresh, int opt)
{
    int CNT = 0;
    for (auto it = allflowname.begin(); it != allflowname.end(); it++)
    {
        it->second = 0;
    }
    for (int i = 0; i < M2; i++)
    {
        int cnt = 0;
        std::unordered_map<std::string, int> temp;
        for (int z = 0; z < node_num; z++)
        {
            for (int j = 0; j < TOP_d; j++)
            {
                if (temp.find(mergename[z][j][i]) != temp.end())
                {
                    temp[mergename[z][j][i]] += mergeresult1[z][j][i];
                }
                else
                {
                    temp[mergename[z][j][i]] = mergeresult1[z][j][i];
                }
                allflowname[mergename[z][j][i]] = mergeresult2[z][j][i];
            }
        }
        for (auto it = temp.begin(); it != temp.end(); it++)
        {
            q[cnt].x = it->first;
            q[cnt].y = it->second;
            cnt++;
        }
        std::sort(q, q + cnt, cmp);
        for (int j = 0; j < TOP_d; j++)
        {
            bk[i].cells[j].Cs = q[j].y;
            bk[i].cells[j].ID = q[j].x;
            bk[i].cells[j].Cr = allflowname[q[j].x];
        }
    }
    if (opt == false)
    {
        for (int i = 1; i < node_num; i++)
        {
            totalnum[0] = std::max(totalnum[i], totalnum[0]);
        }
        for (int i = 0; i < M2; i++)
        {
            for (int j = 0; j < CMM_d; j++)
            {
                int maxv = -1;
                for (int z = 0; z < node_num; z++)
                {
                    maxv = std::max(maxv, mergeresult3[z][j][i]);
                }
                HK[j][i].C = maxv;
            }
        }
    }
    else
    {
        for (int i = 1; i < node_num; i++)
        {
            totalnum[0] += totalnum[i];
        }
        for (int i = 0; i < M2; i++)
        {
            for (int j = 0; j < CMM_d; j++)
            {
                int maxv = 0;
                for (int z = 0; z < node_num; z++)
                {
                    maxv += mergeresult3[z][j][i];
                }
                HK[j][i].C = maxv;
            }
        }
    }

    for (auto it = allflowname.begin(); it != allflowname.end(); it++)
    {
        int h = Hash(it->first) % M2;
        int result = 0;
        unsigned long long hash[CMM_d];
        if (opt1 == 1)
        { // get estimated value from heavy part and light part
            int temp = 0;
            for (int j = 0; j < TOP_d; j++)
            {
                if (bk[h].cells[j].ID == it->first)
                {
                    temp = bk[h].cells[j].Cr;
                }
            }
            int light_num = INT_MAX;
            for (int j = 0; j < CMM_d; j++)
            {
                hash[j] = Hash(it->first + std::to_string(j)) % (M2 - (2 * CMM_d) + 2 * j + 3);
                light_num = std::min(light_num, HK[j][hash[j]].C);
            }
            light_num = light_num - CMM_d * (totalnum[0] - light_num) / (M2 - 1);
            result = temp + light_num;
        }
        else
        { // only query the heavy part
            for (int j = 0; j < TOP_d; j++)
            {
                if (bk[h].cells[j].ID == it->first)
                {
                    result = bk[h].cells[j].Cs;
                }
            }
        }
        it->second = result;
    }
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

std::string DASketch::get_name()
{
    return "DASketch";
}
