#ifndef _mvsketch_H
#define _mvsketch_H

#include "BaseSketch.h"
#include "params.h"
#include "BOBHash64.h"

#define MV_d 4

class MVSketch : public sketch::BaseSketch
{
private:
	struct node
	{
		int C, S;
		std::string FP;
	} HK[MV_d][MAX_MEM + 10];
	BOBHash64 *bobhash_[MV_d];
	BOBHash64 *bobhash;
	int K, M2, sum;

public:
	MVSketch(int M2);
	~MVSketch();

	void clear() override;
	void insert(const std::string& str) override;
	std::pair<std::string, int> query_top(int k) override;
	int query(const std::string& str) override;
	void work(int n) override;
	int merge(int thresh, int opt = 0) override;
	std::string get_name() override;
};
#endif
