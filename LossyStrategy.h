#ifndef LOSSYSTRATEGY_H
#define LOSSYSTRATEGY_H

#include <cstdint>
#include <cstdlib>
#include <cmath>

namespace Lossy {

class BaseStrategy {
public:
    virtual void AlgorithmInterface(int32_t &num,std::string &oldstr,std::string newstr) = 0;
};
//0
class MinusOneStrategy : public BaseStrategy {
public:
    void AlgorithmInterface(int32_t &num,std::string &oldstr,std::string newstr){
        if (num > 0) {
            --num;
        }
		if (num<=0){
			oldstr = newstr;
			num = 1;
		}
    }
};
//1
class HeavyKeeperStrategy : public BaseStrategy {
public:
    void AlgorithmInterface(int32_t &num,std::string &oldstr,std::string newstr){
        if (!(rand()%int(pow(1.08, num)))){			//heavykeeper
	    	num--;
	    }
	    if (num<=0){
			oldstr = newstr;
			num = 1;
	    }
    }
};
//2
class RAPStrategy : public BaseStrategy {
public:
    void AlgorithmInterface(int32_t &num,std::string &oldstr,std::string newstr)  {
		if (!(rand()%(num+1))){					// RAP replacement
			oldstr = newstr;
			num++;
		}
    }
};
//3
class USSStrategy : public BaseStrategy {
public:
    void AlgorithmInterface(int32_t &num,std::string &oldstr,std::string newstr)  {
		num++;									// USS replacement
		if (!(rand()%(num))){						
			oldstr = newstr;
		}
    }
};

class Context
{
public:
	BaseStrategy *strategy;
    Context(int AlgorithmMode){
    	switch (AlgorithmMode) {
    	case 0:
    	    this->strategy = new MinusOneStrategy();
    	    break;

    	case 1:
    	    this->strategy = new HeavyKeeperStrategy();
    	    break;

    	case 2:
    	    this->strategy = new RAPStrategy();
    	    break;
			
    	case 3:
    	    this->strategy = new USSStrategy();
    	    break;

    	default:
    	    this->strategy = nullptr;
    	    break;
    	}
	}

    void ContextInterface(int32_t &num,std::string &oldstr,std::string newstr){
    	this->strategy->AlgorithmInterface(num,oldstr,newstr);
	}
};
} // namespace Lossy

#endif