#ifndef __COMMON_H__
#define __COMMON_H__

#include<vector>
#include<time.h>
#include<assert.h>
using namespace std;

//debug
#define VERBOSE(x) x
//#define VERBOSE(x)
//#define NDEBUG

//node
typedef unsigned int NodeID ;
//typedef size_t NodeID ;
typedef double Coordinate;
typedef unsigned int NodeSize;
//typedef size_t NodeSize;
#define REALLATLNG 1000000.0

//edge
typedef unsigned int EdgeID;
typedef unsigned int EdgeWeight;
//typedef size_t EdgeID;
//typedef size_t EdgeWeight;
typedef unsigned long long Bits;

//random
#define RANDOM_LEN 999

//parameter
//#define DNCC 1 //detect natural cuts C (times)
////#define DNCF 5 //detect natrual cuts f NOTE: only for sample
//#define DNCF 10 //detect natrual cuts f, which sets the core size relative to the ring
//#define FI 2 //each pair reoptimize fail times limit
//#define M  4 //multistart and combination number of iteration


#endif __COMMON_H__