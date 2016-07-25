#ifndef __F_GRAPH_H__
#define __F_GRAPH_H__

#include <vector>
#include <stdio.h>
#include <time.h>
#include <list>
#include <map>
#include <set>
#include <deque>
#include <cmath>

using namespace std;

#include "A_Node.h"
#include "../share/Utility.h"
#include "Logic_Edge.h"

class F_Graph{

public:
	
	F_Graph(){

	}

	~F_Graph(){
	}

	vector<A_Node>& get_node_list(){
		return this->node_list;
	}

	vector<A_Edge>& get_edge_list(){
		return this->edge_list;
	}

	A_Edge* const sym_edge( A_Edge* const e ){

		return &this->edge_list[ this->sym_id[e->get_id()] ];
	}

	EdgeID const sym_edge_id( EdgeID eid ){

		return this->sym_id[eid];
	}

	/////////////////////////////main methods////////////////////////////

private:

	friend class A_Graph;

	//basic
	vector<A_Node> node_list;
	vector<A_Edge> edge_list;
	vector<NodeID> sym_id;

};


#endif __A_GRAPH_H__