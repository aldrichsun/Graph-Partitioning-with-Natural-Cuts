#ifndef __A_NODE_H__
#define __A_NODE_H__

#include <vector>
#include <map>

using namespace std;

#include "../share/common.h"
#include "A_Edge.h"

class A_Node{

public:

	~A_Node(){
	}

	A_Node( NodeID id, NodeSize sz ): nid(id), sz(sz){
	}

	const NodeID get_id() const{
		return this->nid;
	}

	const NodeSize get_size() const{
		return this->sz;
	}

	vector<A_Edge*>& get_adj_list(){
		return this->adj_list;
	}

private:

	const NodeID nid;
	vector<A_Edge*> adj_list;
	const NodeSize sz;
};


#endif __G_NODE_H__