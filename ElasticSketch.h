#ifndef _elasticsketch_H
#define _elasticsketch_H

#include "BaseSketch.h"
#include "params.h"
#include "BOBHash64.h"

#define BN 4	// the depth of heavy part
#define lambd 8 // the param of vote method

class ElasticSketch : public sketch::BaseSketch
{
private:
	struct heavy
	{
		std::string FP;
		unsigned int pvote, Flag;
	} HK[MAX_MEM + 10][BN];
	struct light
	{
		unsigned int C;
	} LK[MAX_MEM + 10];
	BOBHash64 *bobhash;
	BOBHash64 *bobhash_;
	BOBHash64 *bobhash_test[4];
	int M1, M2;

public:
	ElasticSketch(int M1, int M2);
	~ElasticSketch();

	void clear() override;
	void insert(const std::string &str) override;
	void work(int n) override;
	int merge(int thresh, int opt = 0) override;
	std::string get_name() override;
};
#endif
