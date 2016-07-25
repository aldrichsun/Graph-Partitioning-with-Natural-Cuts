#ifndef __POOL_ITEM__
#define __POOL_ITEM__

#include "../share/common.h"

class Pool_Item{

public:

	vector< vector<NodeID> > node_clusters;
	set<EdgeID> cut_edges;
	EdgeWeight item_weight;

};


#endif __POOL_ITEM__