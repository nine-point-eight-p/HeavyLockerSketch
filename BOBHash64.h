#ifndef _BOBHASH64_H
#define _BOBHASH64_H

typedef unsigned int uint;
typedef unsigned long long int uint64;

class BOBHash64
{
public:
	BOBHash64();
	~BOBHash64();
	BOBHash64(uint prime64Num);
	void initialize(uint prime64Num);
	uint64 run(const char * str, uint len);
private:
	uint prime64Num;
};

#endif //_BOBHASH64_H
