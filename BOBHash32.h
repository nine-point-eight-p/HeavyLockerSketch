#ifndef _BOBHASH32_H
#define _BOBHASH32_H

typedef unsigned int uint;
typedef unsigned long long int uint64;

class BOBHash32
{
public:
	BOBHash32();
	~BOBHash32();
	BOBHash32(uint prime32Num);
	void initialize(uint prime32Num);
	uint run(const char * str, uint len);
private:
	uint prime32Num;
};

#endif //_BOBHASH32_H
