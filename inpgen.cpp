/* Global */
#include <iostream>
#include <string>
#include <vector>
/* read_argv */
#include <algorithm>
#include <ctime>
#include <cstdlib>
/* test */
#include <set>
/* experimental installation */
#define TREE23_EXPERIMENTAL
/* debug: #define TREE23_DEBUG */
#include "Tree23.h"
/* clocks */
#include <chrono>
#include <cmath>

#define SEP "\r " << string(40, ' ') << "\r"
#define ALL(x) x.begin(), x.end()

using namespace std;
using namespace std::chrono;

struct op_t {
	long long rand;
	char type;
	long long val;
};

int min_size = 0;
int n,m,t,find_cnt;
int act_size, max_size;
long long sum_size;
string patt;
vector<op_t> test_data;
vector<bool> out_st, out_nd;

void read_argv(int argc, char** argv) {	
	if (argc < 4) {
		cerr << "Arguments expected:" << endl;
		cerr << "   inpgen PATTERN #PATTERNS #TESTS [MIN_SIZE]" << endl;
		cerr << "   PATTERN: string of {F,I,E} (find, insert, erase)" << endl;
		cerr << "   \texample: 'IFFEF' - for each: insert value," << endl;
		cerr << "   \tfind it twice, erase and check it." << endl;
		cerr << "   #PATTERNS: number of values inserted by pattern" << endl;
		cerr << "   #TESTS: number of tests" << endl;
		cerr << "   MIN_SIZE: number of inserts before the first erase" << endl;		
		exit(1);
	}
	patt = argv[1];	
	n = atoi(argv[2]);
	t = atoi(argv[3]);
	if (argc == 5) min_size = atoi(argv[4]);
	m = patt.length();
	test_data.resize(n*m);
	find_cnt = 0;
	for (auto op: patt) if (op == 'F') find_cnt++;
	out_st.resize(n*find_cnt); out_nd=out_st;
}

void generate_input() {
	srand(time(NULL));
	for (int i=0; i<n; i++) {
		vector<long long> rpos(m);
		for (auto &r: rpos) r = rand();
		sort(ALL(rpos));
		for (int j=0; j<m; j++) {
			test_data[m*i+j] = {rpos[j], patt[j], i};
		}
	}
	sort(ALL(test_data), 
		[](const op_t &a, const op_t &b) { return a.rand < b.rand; });
	/* cause of min_size */
	auto dist = test_data.begin();
	for (int cnt=0; dist!=test_data.end(); dist++) {
		if (cnt >= min_size) break;
		if (dist->type=='I') cnt++;
	}
	sort(test_data.begin(), dist, 
		[](const op_t &a, const op_t &b) { 
			if (a.type=='E' != (b.type=='E')) return b.type=='E';
			return a.rand < b.rand;
		}
	);
	//*
	for (const op_t &op : test_data) {
		if (op.type == 'I') act_size++;
		else if (op.type == 'E') act_size--;
		max_size = max(max_size, act_size);
		sum_size += act_size;
	}//*/
}

void test_set() {
	set<int> std_set;
	int i = 0;
	for (op_t &op: test_data) {
		switch (op.type) {
		  case 'I':
			std_set.insert(op.val);
			continue;
		  case 'E':
			std_set.erase(op.val);
			continue;
		  case 'F':
			out_st[i] = (std_set.find(op.val) != std_set.end());
			continue;
		}
	}
}	
	
void test_tree() {	
	Tree23<int> my_tree;
	int i = 0;
	for (op_t &op: test_data) {
		switch (op.type) {
		  case 'I':
			my_tree.insert(op.val);		
			continue;
		  case 'E':
			my_tree.erase(op.val);					
			continue;
		  case 'F':
			out_nd[i] = my_tree.find(op.val);				
			continue;
		}		
	}
}

void check_output() {
	for (int i=0; i<find_cnt; i++) {
		if (out_st[i] != out_nd[i]) {
			cerr << "ERROR in structure" << endl;
			exit(1);
		}
	}
}

int main(int argc, char** argv) {
	ios::sync_with_stdio(false);
	read_argv(argc, argv);
	long long time_tree=0, time_set=0;
	act_size = max_size = sum_size = 0;
	for (int i=0; i<t; i++) {
		auto t0 = high_resolution_clock::now();
		generate_input();
		auto t1 = high_resolution_clock::now();
		test_set();
		auto t2 = high_resolution_clock::now();
		test_tree();
		auto t3 = high_resolution_clock::now();
		check_output();
		auto dur_gen  = duration_cast<milliseconds>( t1 - t0 ).count();
		auto dur_set  = duration_cast<milliseconds>( t2 - t1 ).count();
		auto dur_tree = duration_cast<milliseconds>( t3 - t2 ).count();
		time_tree += dur_tree; time_set += dur_set;
		cerr << SEP << i+1 << "\t"<< "OK"<<"\t";
		cerr << dur_tree/1000.0 << "s\t" << dur_set/1000.0 << "s";
		cerr << "\t" << dur_gen/1000.0;
	}
	cerr << SEP << "     -- tests OK --" << endl;		
	/* result */
	cout << "Total time" << "\t" << "tree: " << time_tree/1000.0 << "s";
	cout << ", set: " << time_set/1000.0 << "s" << endl; 
	cout << "Size" << "\t\t" << "average: " << round(1.0*sum_size/m/n/t);
	cout << ", max: " << max_size << endl;
	cout << "Result" << "\t\t";
	cout << "Tree23/set::set: " << (time_tree*100.0/time_set) << "%" << endl;
	return 0;
}