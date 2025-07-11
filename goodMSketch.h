#ifndef _BBSKETCH_H
#define _BBSKETCH_H

#include "BaseSketch.h"
#include "BOBHash64.h"
#include "params.h"
#include "LossyStrategy.h"

#define maxdepth 20
extern int depth;
extern int hashnum;
extern double lock_thre;

class MSketch : public sketch::BaseSketch
{
private:
	struct bucket_t
	{
		std::string fingerprint[maxdepth];
		int counter[maxdepth];
		bool lock_bit = 0;
	};
	int bucket_num;
	bucket_t *bucket;
	BOBHash64 *bobhash;
	double hh_ratio;
	int total_packet = 0;
	int hash[10];
	Lossy::Context newContext = Lossy::Context(2);

public:
	MSketch(uint _bucket, double _hh_ratio);
	~MSketch();

	void clear() override;
	void insert(const std::string &key) override;
	std::pair<std::string, int> query_top(int k) override;
	int query(const std::string& str) override;
	void work(int n) override;
	int merge(int thresh, int opt = 0) override;
	std::string get_name() override;

	unsigned long long Hash(std::string ST);
	bool plus(bucket_t *b, int j);
};
#endif //_CCCOUNTER_H
