#ifndef _mvsketch_H
#define _mvsketch_H

#include "BaseSketch.h"
#include "params.h"
#include "BOBHash64.h"

#define MV_d 4

class MVSketch : public sketch::BaseSketch
{
private:
	struct Bucket
	{
		int v, c;
		std::string key;
	};
	Bucket *buckets[MV_d];
	BOBHash64 *bobhash[MV_d];
	int col_num;

public:
	MVSketch(int col_num);
	~MVSketch();

	void clear() override;
	void insert(const std::string& str) override;
	void work(int n) override;
	int merge(int thresh, int opt = 0) override;
	std::string get_name() override;
};
#endif
