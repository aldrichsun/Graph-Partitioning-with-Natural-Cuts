#ifndef __G_EDGE_H__
#define __G_EDGE_H__

#include "../share/common.h"

class G_Edge{

public:

	~G_Edge(){
	}

	G_Edge(NodeID s, NodeID t, EdgeID id): source(s), target(t), eid(id){
	}

	const NodeID get_source() const{
		return this->source;
	}

	const NodeID get_target() const{
		return this->target;
	}

	const EdgeID get_id() const{
		return this->eid;
	}

private:

	const NodeID source;
	const NodeID target;
	const EdgeID eid;
};



#endif __G_EDGE_H__