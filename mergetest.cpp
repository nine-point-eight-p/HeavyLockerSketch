#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <algorithm>
#include <time.h>
#include <unordered_map>
#include <map>
#include <vector>
#include <string>
#include <cstring>
#include <sstream>
#include <string.h>
#include "unistd.h"
#include "params.h"
#include "result.h"
#include "BOBHash64.h"
/********************Store intermediate data for aggregation********************/
// unordered_map<string,int> allflowname;
// struct Node { string x; int y; };
// Node *q = nullptr, *p = nullptr;  // Allocate dynamically
// int cmp(Node i, Node j) { return i.y > j.y;}
// string ***mergename = nullptr;  // Allocate dynamically
// int ***mergeresult1 = nullptr;  // Allocate dynamically  
// int ***mergeresult2 = nullptr;  // Allocate dynamically
// int ***mergeresult3 = nullptr;  // Allocate dynamically
// int totalnum[10];

/********************Import different methods********************/
double  hh=0.0001;   //Define the threshold of big flows
int MEM=50;          //Memory per node (KB)
#include "CMSketch.h"
#include "ElasticSketch.h"
#include "MVSketch.h"
#include "goodMSketch.h"
#include "USS.h"
#include "DASketch.h"
using namespace std;

/********************Store results********************/
std::map<std::string, int> AAE;           
std::map<std::string, double> ARE;        
std::map<std::string, int> _sum;          //The true num of big flows detected by methods 
std::map<std::string, int> _all;          //The num of big flows detected by methods
std::map<std::string, double> insert_time;//time to insert all the packages
std::map<std::string, double> query_time; //time to merge and query the HH
vector<string> s[10];
std::vector<sketch::BaseSketch*> func[10];//Store different methods to facilitate the calls of interfaces
map <string ,int> B,C;
int bigflow;
int packet_num=0;

//***********************************Parameters****************************//
bool can_occur_same=true;				  //Whether the same flow can be present on different nodes (have influence on accuracy)
string resultFile = "None"; 			  //Outputfile
char dataset[60]="0.dat";				  //Dataset

/********************Clean intermediate data for calls in loop********************/
void clear(){
	for(int i=0;i<10;i++){
		func[i].clear();
	}
	for(map<string,int>::iterator it=_sum.begin();it!=_sum.end();it++){
		it->second=0;
    }
	for(unordered_map<string,int>::iterator it=allflowname.begin();it!=allflowname.end();it++){
		it->second=0;
    }
	for(map<string,double>::iterator it=ARE.begin();it!=ARE.end();it++){
		it->second=0;
    }
	for(map<string,int>::iterator it=AAE.begin();it!=AAE.end();it++){
		it->second=0;
    }
	for(map<string,int>::iterator it=_all.begin();it!=_all.end();it++){
		it->second=0;
    }
	
	// Clear the merge arrays properly - only clear the reasonable size we allocated
	int reasonable_merge_size = min(10000, MAX_MEM/100);
	for(int i=0;i<10;i++){
		for(int j=0;j<10;j++){
			for(int k=0;k<reasonable_merge_size;k++){
				if(mergename && mergename[i] && mergename[i][j]) {
					mergename[i][j][k]="";
				}
			}
			if(mergeresult1 && mergeresult1[i] && mergeresult1[i][j]) {
				memset(mergeresult1[i][j], 0, reasonable_merge_size * sizeof(int));
			}
			if(mergeresult2 && mergeresult2[i] && mergeresult2[i][j]) {
				memset(mergeresult2[i][j], 0, reasonable_merge_size * sizeof(int));
			}
			if(mergeresult3 && mergeresult3[i] && mergeresult3[i][j]) {
				memset(mergeresult3[i][j], 0, reasonable_merge_size * sizeof(int));
			}
		}
	}
}

/********************Output the results to csv or command line********************/
void writeResultToCSV(int n) {
	ofstream in(resultFile, ios::app);
    if (in.is_open()) {
        for (int z=0;z<n;z++) {
			if(resultFile=="None"){
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
		    	in << 
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
        in.close();
    } else {
        cout << "Unable to open file" << endl;
    }
}

/********************The core function********************/
void resolve() {
	clear();
	bool debug=false;
	cout<<"dataset: "<<dataset<<endl;
    cout<<"MEM="<<MEM<<"KB"<<endl;
	cout<<"Node_num="<<node_num<<endl;
	cout<<"Thresh="<<hh<<endl;
	cout<<"hashnum="<<hashnum<<endl;
	cout<<"can occur same flow?"<<can_occur_same<<endl;
    cout<<"**********preparing all algorithms**********"<<endl;
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
	int threshold = packet_num * hh;//get the threshold of the big flow

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

void parse() {

}

int main(int argc, char** argv){
	int c;
	while((c=getopt(argc, argv, "d:o:m:n:s:t:r:e:l:h:"))!=-1) {//example:./merge -d 0.dat -m 50 -n 3 -s 1 -t 0.0002
        switch(c) {
            case 'd'://the path of dataset
                strcpy(dataset,optarg);
                break;
            case 'o'://the path of outputfile
                resultFile=optarg;
                break;
            case 'm'://memeory in KB
                MEM=atoi(optarg);
                break;
			case 'n'://num of nodes
				node_num=atoi(optarg);
				node_num=node_num;
				break;
			case 's'://whether the same flow can be present on different nodes
				can_occur_same=atoi(optarg);
				break;
			case 't'://the threshold of heavyhitter
				hh=atof(optarg);
				break;
			case 'e'://the depth of heavylocker
				depth=atoi(optarg);
				break;
			case 'l'://the lock_thresh of heavyLocker
				lock_thre=atof(optarg);
				break;
			case 'h'://the num of hash functions
				hashnum=atoi(optarg);
				break;
        }
    }

	/********************Allocate dynamic memory for large arrays********************/
	// First read the dataset to determine actual memory needs
	ifstream fin(dataset, ios::in|ios::binary);
	if(!fin) {printf("Dataset not exists!\n");return -1;}
	char tmp[105];
	BOBHash64 * smallhash=new BOBHash64(node_num);
	
	// Read dataset and count unique flows
	for (int i = 1; i <= MAX_INSERT; i++)
	{
		fin.read(tmp, KEY_LEN);
		if (fin.eof()) break;
        packet_num++;
		tmp[KEY_LEN]='\0';
		string temp(tmp, KEY_LEN);
		long long which;
		if(can_occur_same==false){
			which=smallhash->run(temp.c_str(), KEY_LEN)%node_num;
		}else{
			which=rand()%node_num;
		}
		s[which].push_back(temp);
		B[temp]++;
	}
    printf("flow num = %d\n", packet_num);
    printf("flow type = %d\n", (int)B.size());
	
	// Now allocate arrays based on actual needs
	int actual_flows = B.size();
	int safe_flow_count = actual_flows + 1000; // Add some buffer
	
	// Allocate Node arrays with proper size
	q = new Node[safe_flow_count];
	p = new Node[safe_flow_count];
	cerr << "Allocated memory for " << safe_flow_count << " flows (actual: " << actual_flows << ")" << endl;
	
	// Allocate 3D arrays - only allocate what's needed for merge operations
	// Most sketch algorithms don't need the full MAX_MEM dimension
	int reasonable_merge_size = min(10000, MAX_MEM/100); // Much more reasonable size
	
	mergename = new string**[10];
	mergeresult1 = new int**[10];
	mergeresult2 = new int**[10];
	mergeresult3 = new int**[10];
	
	for(int i = 0; i < 10; i++) {
		mergename[i] = new string*[10];
		mergeresult1[i] = new int*[10];
		mergeresult2[i] = new int*[10];
		mergeresult3[i] = new int*[10];
		
		for(int j = 0; j < 10; j++) {
			mergename[i][j] = new string[reasonable_merge_size];
			mergeresult1[i][j] = new int[reasonable_merge_size];
			mergeresult2[i][j] = new int[reasonable_merge_size];
			mergeresult3[i][j] = new int[reasonable_merge_size];
		}
	}
	cerr << "Allocated merge arrays with size " << reasonable_merge_size << endl;
	/********************Processing output file********************/
	if(resultFile!="None"){
		ofstream in(resultFile);
    	if (in.is_open()) {
			in << 
    	    "MEM" << ","<<"name" << ","  << 
			"trueHH"<<","<<"findHH"<<","<<
    	    "right_in_find"<<"," << 
			"recall"<< "," << 
			"percision"<< "," <<
			"F1 score"<< "," <<
    	    "AAE" << "," << 
    	    "ARE" << "," << 
    	    "insert" << "," <<
    	    "query" << "," << endl;
    	in.close();
		}
	}
	/********************prepare the true result********************/
	int cnt=0;
    for (map <string,int>::iterator sit=B.begin(); sit!=B.end(); sit++)
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