#ifndef __R_GRAPH_H__
#define __R_GRAPH_H__

#include <vector>
#include <stdio.h>

using namespace std;

#include "R_Node.h"
#include "../share/Utility.h"

class R_Graph{

public:
	
	R_Graph(){
	}

	~R_Graph(){
	}

	vector<R_Node>& get_node_list(){
		return this->node_list;
	}

	vector<R_Edge>& get_edge_list(){
		return this->edge_list;
	}

	NodeID contract_id( NodeID nid ){
		return this->contract_to[nid];
	}

	/////////////////////////////main methods////////////////////////////

	void read_graph(string co_path, string gr_path);

	void fill_contract_to( vector< vector<NodeID> >& final_result );

	EdgeWeight write_result( vector< vector<NodeID> >& result, vector< vector<NodeID> >& id_map, 
		string& co_path, string& gr_path, string& r_path );

private:
	//basic
	vector<R_Node> node_list;
	vector<R_Edge> edge_list;

	vector<NodeID> contract_to;

};


#endif __R_GRAPH_H__