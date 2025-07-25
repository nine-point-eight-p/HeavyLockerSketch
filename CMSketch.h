#ifndef _cmsketch_H
#define _cmsketch_H

#include "BaseSketch.h"
#include "params.h"
#include "ssummary.h"
#include "BOBHash64.h"

#define CM_d 4

// use CM sketch+space summary to make the result invertible.
class CMSketch : public sketch::BaseSketch
{
private:
	ssummary *ss;
	struct node
	{
		int C;
	} HK[CM_d][MAX_MEM + 10];
	BOBHash64 *bobhash;
	int M2, K, d;

public:
	CMSketch(int Mem, int Knum);
	~CMSketch();

	void clear() override;
	void insert(const std::string &x) override;
	void work(int n) override;
	int merge(int thresh, int opt = false) override;
	std::string get_name() override;

	unsigned long long Hash(std::string ST);
};

#endif
