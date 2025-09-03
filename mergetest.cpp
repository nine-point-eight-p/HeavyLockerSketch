#include <iostream>
#include <fstream>
#include <cstdio>
#include <ctime>
#include <unordered_map>
#include <map>
#include <vector>
#include <string>
#include <cstring>
#include "unistd.h"
#include "params.h"
#include "result.h"
#include "BOBHash64.h"

#include "CMSketch.h"
#include "ElasticSketch.h"
#include "MVSketch.h"
#include "goodMSketch.h"
#include "USS.h"
#include "DASketch.h"
#include "WavingSketch.h"
#include "LDSketch.h"

using std::cout, std::cerr, std::endl;
using std::ios;

/********************Store results********************/
std::map<std::string, int> AAE;           
std::map<std::string, double> ARE;        
std::map<std::string, int> _sum;          //The true num of big flows detected by methods 
std::map<std::string, int> _all;          //The num of big flows detected by methods
std::map<std::string, double> insert_time;//time to insert all the packages
std::map<std::string, double> query_time; //time to merge and query the HH
std::vector<std::string> s[10];
std::vector<sketch::BaseSketch*> func[10];//Store different methods to facilitate the calls of interfaces
std::map<std::string, int> B, C;
int bigflow;
int packet_num=0;

//***********************************Parameters****************************//
double hh = 0.0001;                       // Define the threshold of big flows
int MEM = 50;                             // Memory per node (KB)
bool can_occur_same = true;				  // Whether the same flow can be present on different nodes (have influence on accuracy)
std::string output_file; 		          // Output file
std::string dataset_file = "0.dat";       // Dataset
std::string config_file = "config.txt";   // Config file

/********************Clean intermediate data for calls in loop********************/
void clear(){
	for(int i=0;i<10;i++){
		func[i].clear();
	}
	for(auto it = _sum.begin();it!=_sum.end();it++){
		it->second=0;
    }
	for(auto it = allflowname.begin();it!=allflowname.end();it++){
		it->second=0;
    }
	for(auto it=ARE.begin();it!=ARE.end();it++){
		it->second=0;
    }
	for(auto it=AAE.begin();it!=AAE.end();it++){
		it->second=0;
    }
	for(auto it=_all.begin();it!=_all.end();it++){
		it->second=0;
    }
	
	for(int i=0;i<10;i++){
		for(int j=0;j<10;j++){
			for(int k=0;k<MAX_MEM;k++) {
				mergename[i][j][k].clear();
			}
		}
	}
	memset(mergeresult1, 0, sizeof(mergeresult1));
	memset(mergeresult2, 0, sizeof(mergeresult2));
	memset(mergeresult3, 0, sizeof(mergeresult3));
}

/********************Output the results to csv or command line********************/
void writeResultToCSV(int n) {
	std::ofstream fout(output_file, ios::app);
    if (fout.is_open()) {
        for (int z=0;z<n;z++) {
			if(output_file.empty()){
				cout << 
            	    "MEM  " << MEM << ","<< endl <<
					"name  " << func[z][0]->get_name() << ","  << endl <<
					"trueHH  "<< bigflow<<","<< endl <<
					"findHH  "<<_all[func[z][0]->get_name()]<<","<< endl <<
            	    "right_in_find  "<< _sum[func[z][0]->get_name()]<<"," <<  endl <<
					"precision  "<< (_sum[func[z][0]->get_name()] / (_all[func[z][0]->get_name()] + 0.0))<< "," <<  endl <<
					"recall  "<< (_sum[func[z][0]->get_name()] / (bigflow + 0.0))<< "," <<  endl <<
					"F1 score  "<< 2* _sum[func[z][0]->get_name()]/(_all[func[z][0]->get_name()]+bigflow+0.0)<< "***************" << endl <<
            	    "AAE  " << AAE[func[z][0]->get_name()] / (bigflow + 0.0) << "***************" <<  endl <<
            	    "ARE  " << ARE[func[z][0]->get_name()] / (bigflow + 0.0) << "***************" <<  endl <<
            	    "insert  " << 1000.0 * packet_num/insert_time[func[z][0]->get_name()] << "," << endl <<
            	    "query  " << 1000.0 * packet_num/query_time[func[z][0]->get_name()] << "," << endl << 
					endl;
			}else{
		    	fout << 
            	    MEM <<"KB"<< ","<<func[z][0]->get_name() << ","  << 
					bigflow<<","<<_all[func[z][0]->get_name()]<<","<<
            	    _sum[func[z][0]->get_name()]<<"," << 
					(_sum[func[z][0]->get_name()] / (_all[func[z][0]->get_name()] + 0.0))<< "," << 
					(_sum[func[z][0]->get_name()] / (bigflow + 0.0))<< "," <<
					2* _sum[func[z][0]->get_name()]/(_all[func[z][0]->get_name()]+bigflow+0.0)<< "," <<
            	    AAE[func[z][0]->get_name()] / (bigflow + 0.0) << "," << 
            	    ARE[func[z][0]->get_name()] / (bigflow + 0.0) << "," << 
            	    1000.0 * packet_num/insert_time[func[z][0]->get_name()] << "," <<
            	    1000.0 * packet_num/query_time[func[z][0]->get_name()] << "," << endl;
			}
	    }
        fout.close();
    } else {
        cout << "Unable to open file" << endl;
    }
}

/********************The core function********************/
void resolve() {
	clear();
	bool debug=false;
	cout<<"dataset: "<<dataset_file<<endl;
    cout<<"MEM="<<MEM<<"KB"<<endl;
	cout<<"Node_num="<<node_num<<endl;
	cout<<"Thresh="<<hh<<endl;
	cout<<"hashnum="<<hashnum<<endl;
	cout<<"can occur same flow?"<<can_occur_same<<endl;
    cout<<"**********preparing all algorithms**********"<<endl;

	int threshold = packet_num * hh;//get the threshold of the big flow

	//CMsketch
	int method=0;
	int CM_M,K;
	for (CM_M=1; 32*CM_M*CM_d<=MEM*1024*8*0.8; CM_M++);
		--CM_M;
	for (K=1; 64*K<=MEM*1024*8*0.2; K++);//This is the space occupied by the heap
		--K;
	cout<<"CM"<<CM_M<<" K"<<K<<endl;
	for(int i=0;i<node_num;i++){
		func[method].push_back(new CMSketch(CM_M,K));
	}
	method++;
	
	//Elastic
	int ES_M;
	for (ES_M=1; 64*BN*ES_M+8*BN*ES_M<=MEM*1024*8; ES_M++);//Heavy part and light part
	ES_M--;
	cout<<"EL"<<ES_M<<"M2"<<ES_M*8<<endl;
	for(int i=0;i<node_num;i++){
		func[method].push_back(new ElasticSketch(ES_M, ES_M*8));
	}
	method++;

	//MVsketch
	int MV_M;
	for (MV_M=1; 96*MV_M*MV_d<=MEM*1024*8; MV_M++);
	MV_M--;
	cout<<"MV"<<MV_M<<endl;
	for(int i=0;i<node_num;i++){
		func[method].push_back(new MVSketch(MV_M));
	}
	method++;

	//USS
	int HU_M;
	for (HU_M=1; 64*HU_M*HU_d<=MEM*1024*8; HU_M++);
	--HU_M;
	cout<<"HU"<<HU_M<<endl;
	for(int i=0;i<node_num;i++){
		func[method].push_back(new HyperUSS(HU_M));
	}
	method++;

	//DAsketch
	int DA_M;
	for (DA_M=1; 96*DA_M*TOP_d+32*DA_M*CMM_d<=MEM*1024*8; DA_M++);if (DA_M%2==0) DA_M--;
	cout<<"DA"<<DA_M<<endl;
	for(int i=0;i<node_num;i++){
		func[method].push_back(new DASketch(DA_M));
	}
	method++;

	// Waving Sketch
	int waving_bucket_num = 1;
	int waving_bucket_size = 64 * WavingSketch::CELL_NUM + 32 * WavingSketch::COUNTER_NUM;
	while (waving_bucket_size * waving_bucket_num <= MEM * 1024 * 8)
		waving_bucket_num++;
	waving_bucket_num--;
	cout << "Waving" << waving_bucket_num << endl;
	for (int i = 0; i < node_num; i++)
		func[method].push_back(new WavingSketch(waving_bucket_num));
	method++;

	// LD-Sketch
	int ld_col_num = 1;
	int ld_bucket_size = 64 * LDSketch::ARRAY_SIZE + 32 * 3;
	while (ld_bucket_size * LDSketch::ROW_NUM * ld_col_num <= MEM * 1024 * 8)
		ld_col_num++;
	ld_col_num--;
	cout << "LD" << ld_col_num << endl;
	for (int i = 0; i < node_num; i++)
		func[method].push_back(new LDSketch(ld_col_num, threshold));
	method++;

	//My method
	int My_M;
	for (My_M=1; 64*My_M*depth<=MEM*1024*8; My_M++);
	My_M--;
	cout<<"My"<<My_M<<endl;
	for(int i=0;i<node_num;i++){
		func[method].push_back(new MSketch(My_M,hh));
	}
	method++;

	//Read the dataset
	timespec time1, time2;

	//Clear
	for(int z=0;z<method;z++){
		for(int i=0;i<node_num;i++){
			func[z][i]->clear();
			cout<<func[z][i]->get_name()<<endl;
		}
		
	}

	//Inserting
	cout<<"start insert"<<endl;
	for(int z=0;z<method;z++){
		srand(1);
		clock_gettime(CLOCK_MONOTONIC, &time1);
		for(int i=0;i<node_num;i++){			//Go through the nodes
			int s_size=s[i].size();
			for(int j=0;j<s_size;j++){          //Go through the packages
				func[z][i]->insert(s[i][j]);    //Use the z method to insert the package j into node i
			}
		}
		clock_gettime(CLOCK_MONOTONIC, &time2);
		insert_time[func[z][0]->get_name()]=time2.tv_nsec+time2.tv_sec * 1000000000 -time1.tv_sec * 1000000000 -time1.tv_nsec; //get the time
	}

	//Work and merge
	cout<<"work and merge"<<endl;
	for(int z=0;z<method;z++){
		// TODO: clear allflowname out of merge
		// for (auto it = allflowname.begin(); it != allflowname.end(); ++it)
		// {
		// 	it->second = 0;
		// }

		clock_gettime(CLOCK_MONOTONIC, &time1);
		for(int i=0;i<node_num;i++){
			func[z][i]->work(i);
		}
		_all[func[z][0]->get_name()]=func[z][0]->merge(threshold,can_occur_same);
		int num;
		for(int j=0;j<bigflow;j++){
			num = func[z][0]->query(p[j].x);
			AAE[func[z][0]->get_name()] += abs(B[p[j].x] - num);
            ARE[func[z][0]->get_name()] += abs(B[p[j].x] - num) / (B[p[j].x] + 0.0);
			if (num>threshold) {
                _sum[func[z][0]->get_name()]++;
            }
			if(debug){
				cout<<p[j].y<<"    "<<num<<"    "<<p[j].y-num<<endl;
			}
		}
		clock_gettime(CLOCK_MONOTONIC, &time2);
		query_time[func[z][0]->get_name()]=time2.tv_nsec+time2.tv_sec * 1000000000 -time1.tv_sec * 1000000000 -time1.tv_nsec;
		cout<<func[z][0]->get_name()<<"is ok"<<endl;
	}

	//Output the result
	writeResultToCSV(method);
}

// example:./merge -d 0.dat -m 50 -n 3 -s 1 -t 0.0002
void parseArgs(int argc, char **argv)
{
	int c;
	while ((c = getopt(argc, argv, "d:c:o:m:n:s:t:r:e:l:h:")) != -1)
	{
		switch (c)
		{
		case 'd': // the path of dataset
			dataset_file = optarg;
			break;
		case 'c': // the path of config file
			config_file = optarg;
			break;
		case 'o': // the path of output file
			output_file = optarg;
			break;
		case 'm': // memeory in KB
			MEM = atoi(optarg);
			break;
		case 'n': // num of nodes
			node_num = atoi(optarg);
			break;
		case 's': // whether the same flow can be present on different nodes
			can_occur_same = atoi(optarg);
			break;
		case 't': // the threshold of heavyhitter
			hh = atof(optarg);
			break;
		case 'e': // the depth of heavylocker
			depth = atoi(optarg);
			break;
		case 'l': // the lock_thresh of heavyLocker
			lock_thre = atof(optarg);
			break;
		case 'h': // the num of hash functions
			hashnum = atoi(optarg);
			break;
		}
	}
}

std::unordered_map<std::string, int> parseConfig() {
	std::ifstream fin(config_file);
	if (!fin)
	{
		cerr << "Config file does not exist!" << endl;
		exit(-1);	
	}
	
	std::unordered_map<std::string, int> result;
	std::string line;
	while (std::getline(fin, line))
	{
		int p = line.find('=');
		result[line.substr(0, p)] = atoi(line.substr(p + 1).c_str());
	}
	return result;
}

int main(int argc, char** argv){
	// Parse arguments
	parseArgs(argc, argv);
	
	// Load config
	auto config = parseConfig();
	int entry_len = config["entry_len"];
	int key_start = config["key_start"];
	int key_len = config["key_len"];
	
	std::ifstream fin(dataset_file, ios::in|ios::binary);
	if(!fin) {printf("Dataset not exists!\n");return -1;}
	char buf[105];
	BOBHash64 * smallhash=new BOBHash64(node_num);
	
	// Read dataset and count unique flows
	for (int i = 1; i <= MAX_INSERT; i++)
	{
		fin.read(buf, entry_len);
		if (fin.eof()) break;
		buf[entry_len] = '\0';
        packet_num++;
		
		std::string key(buf + key_start, key_len);
		auto which = can_occur_same
			? rand() % node_num
			: smallhash->run(key.c_str(), key_len) % node_num;
		s[which].push_back(key);
		B[key]++;
	}
    printf("flow num = %d\n", packet_num);
    printf("flow type = %d\n", (int)B.size());
	
	/********************Processing output file********************/
	if(!output_file.empty()){
		std::ofstream fout(output_file);
    	if (fout.is_open()) {
			fout << 
    	    "MEM" << ","<<"name" << ","  << 
			"trueHH"<<","<<"findHH"<<","<<
    	    "right_in_find"<<"," << 
			"precision"<< "," <<
			"recall"<< "," << 
			"F1 score"<< "," <<
    	    "AAE" << "," << 
    	    "ARE" << "," << 
    	    "insert" << "," <<
    	    "query" << "," << endl;
			fout.close();
		}
	}
	/********************prepare the true result********************/
	int cnt=0;
    for (auto sit=B.begin(); sit!=B.end(); sit++)
    {
        p[cnt].x=sit->first;
        p[cnt].y=sit->second;
		allflowname[sit->first]=0;
		cnt++;
    }
    std::sort(p,p+cnt,cmp);

	/********************process and merge the data by different method********************/
	for(bigflow=0; p[bigflow].y>=packet_num * hh; bigflow++);
	cout << "heavy hitter thresh: " << (int)(packet_num * hh) << endl;
	cout<<"have "<<bigflow<<"big flows"<<endl;
	cout<<"++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"<<endl;
	cout<<"now is"<<hh<<endl;
	resolve();		
}