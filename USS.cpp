#include <algorithm>
#include <string>
#include <unordered_map>
#include <cstdlib>

#include "USS.h"
#include "params.h"
#include "result.h"

HyperUSS::HyperUSS(int M2)
	: M2(M2)
{
	bobhash = new BOBHash64(1005);
}

HyperUSS::~HyperUSS()
{
	delete bobhash;
}

void HyperUSS::clear()
{
	for (int i = 0; i < HU_d; i++)
		for (int j = 0; j <= M2 + 5; j++)
			HK[i][j].C = 0, HK[i][j].ID = "";
}

unsigned long long HyperUSS::Hash(std::string ST)
{
	return (bobhash->run(ST.c_str(), ST.size()));
}

void HyperUSS::insert(const std::string &x)
{
	int minv = 0x7fffffff;
	unsigned long long hash[HU_d];
	std::string hash_key = x;
	hash_key.push_back('0');
	for (int i = 0; i < HU_d; i++)
	{
		hash_key[KEY_LEN] = '0' + i;
		hash[i] = bobhash->run(hash_key.c_str(), KEY_LEN + 1) % (M2 - (2 * HU_d) + 2 * i + 3);
	}

	bool flag0 = false, flag1 = false;
	for (int i = 0; i < HU_d; i++)
	{
		if (HK[i][hash[i]].ID == x)
		{
			HK[i][hash[i]].C++;
			flag0 = true;
			break;
		}
	}
	if (!flag0)
	{
		for (int i = 0; i < HU_d; i++)
		{
			if (HK[i][hash[i]].ID == "")
			{
				HK[i][hash[i]].ID = x;
				HK[i][hash[i]].C = 1;
				flag1 = true;
				break;
			}
		}
	}
	if (!flag0 && !flag1)
	{
		int mini;
		for (int i = 0; i < HU_d; i++)
		{
			if (HK[i][hash[i]].C < minv)
			{
				minv = HK[i][hash[i]].C;
				mini = i;
			}
		}
		double p = 1.0 / (minv + 1);
		double q = 1.0 - p;
		double r = (double)rand() / RAND_MAX;
		if (r < p)
		{
			HK[mini][hash[mini]].ID = x;
			HK[mini][hash[mini]].C = 1 / p;
		}
		else
		{
			HK[mini][hash[mini]].C = minv / q;
		}
	}
}

void HyperUSS::work(int n)
{
	int CNT = 0;
	for (int i = 0; i < HU_d; i++)
	{
		for (int j = 0; j < M2; j++)
		{
			if (HK[i][j].ID != "")
			{
				mergename[n][i][j] = HK[i][j].ID;
				mergeresult1[n][i][j] = HK[i][j].C;
			}
			else
			{
				mergename[n][i][j] = "";
				mergeresult1[n][i][j] = 0;
			}
		}
	}
}

int HyperUSS::merge(int thresh, int opt)
{
	int CNT = 0;
	unsigned long long hash[HU_d];
	for (auto it = allflowname.begin(); it != allflowname.end(); it++)
	{
		it->second = 0;
	}
	for (int i = 0; i < HU_d; i++)
	{
		for (int j = 0; j < M2; j++)
		{
			int maxnum = -1;
			std::string maxid = "";
			std::unordered_map<std::string, int> temp;
			for (int k = 0; k < node_num; k++)
			{
				if (temp.find(mergename[k][i][j]) != temp.end())
				{
					temp[mergename[k][i][j]] += mergeresult1[k][i][j];
				}
				else
				{
					temp[mergename[k][i][j]] = mergeresult1[k][i][j];
				}
			}
			for (auto it = temp.begin(); it != temp.end(); it++)
			{
				if (maxnum < it->second)
				{
					maxnum = it->second;
					maxid = it->first;
				}
			}
			HK[i][j].ID = maxid;
			HK[i][j].C = maxnum;
		}
	}
	for (auto it = allflowname.begin(); it != allflowname.end(); it++)
	{
		std::string hash_key = it->first;
		hash_key.push_back('0');
		int result = 0;
		for (int i = 0; i < HU_d; i++)
		{
			hash_key[KEY_LEN] = '0' + i;
			hash[i] = bobhash->run(hash_key.c_str(), KEY_LEN + 1) % (M2 - (2 * HU_d) + 2 * i + 3);
			if (HK[i][hash[i]].ID == it->first)
			{
				result += HK[i][hash[i]].C;
			}
		}
		it->second = result;
		q[CNT].x = it->first;
		q[CNT].y = result;
		CNT++;
	}
	std::sort(q, q + CNT, cmp);
	int bigflow;
	for (bigflow = 0; q[bigflow].y > thresh && bigflow < CNT; bigflow++)
		;

	return bigflow;
}

std::string HyperUSS::get_name()
{
	return "HyperUSS";
}
