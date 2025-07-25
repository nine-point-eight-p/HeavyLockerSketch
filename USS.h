#ifndef _hyperuss_H
#define _hyperuss_H

#include "BaseSketch.h"
#include "params.h"
#include "BOBHash64.h"

#define HU_d 4

class HyperUSS : public sketch::BaseSketch
{
private:
	struct node
	{
		std::string ID;
		int C;
	} HK[HU_d][MAX_MEM + 10];
	BOBHash64 *bobhash;
	int M2;

public:
	HyperUSS(int M2);
	~HyperUSS();

	void clear() override;
	void insert(const std::string &x) override;
	void work(int n) override;
	int merge(int thresh, int opt = 0) override;
	std::string get_name() override;

	unsigned long long Hash(std::string ST);
};
#endif
