#ifndef __R_EDGE_H__
#define __R_EDGE_H__

#include "../share/common.h"

class R_Edge{

public:

	~R_Edge(){
	}

	R_Edge(NodeID s, NodeID t): source(s), target(t){
	}

	const NodeID get_source() const{
		return this->source;
	}

	const NodeID get_target() const{
		return this->target;
	}

private:

	const NodeID source;
	const NodeID target;
};


#endif __R_EDGE_H__