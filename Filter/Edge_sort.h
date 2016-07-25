#ifndef __EDGE_SORT_H__
#define __EDGE_SORT_H__

#include "../share/common.h"

class Edge_sort{

public:

	Edge_sort( const EdgeID id, const Bits bit_list ):eid(id), bits(bit_list){
	}

	Edge_sort( const Edge_sort& es ): eid(es.eid), bits(es.bits){
	}

	~Edge_sort(){
	}

	bool operator < ( const Edge_sort& es ) const {
		return this->bits < es.bits;
	}

	bool operator == ( const Edge_sort& es ) const {
		return this->bits == es.bits;
	}

	//Edge_sort( const EdgeID id, const vector<bool>* bits ):eid(id), bits_pt(bits){
	//}

	//Edge_sort( const Edge_sort& es ): eid(es.eid), bits_pt(es.bits_pt){
	//}

	//~Edge_sort(){
	//}

	//bool operator < ( const Edge_sort& es ) const {

	//	size_t len = this->bits_pt->size();
	//	vector<bool>::const_iterator bit1 = this->bits_pt->begin();
	//	vector<bool>::const_iterator bit2 = es.bits_pt->begin();

	//	while( bit1 != this->bits_pt->end() && bit2 != es.bits_pt->end() ){

	//		if( !(*bit1) && (*bit2) ) // 1-false 2-true
	//			return true;
	//		if( (*bit1) && !(*bit2) ) // 1-true 2-false
	//			return false;
	//		//1-false 2-false || 1-true 2-true
	//		//next and continue
	//		bit1++; bit2++;
	//	}
	//	return false;
	//}

	//bool operator == ( const Edge_sort& es ) const {

	//	size_t len = this->bits_pt->size();
	//	vector<bool>::const_iterator bit1 = this->bits_pt->begin();
	//	vector<bool>::const_iterator bit2 = es.bits_pt->begin();

	//	while( bit1 != this->bits_pt->end() && bit2 != es.bits_pt->end() ){

	//		if( !(*bit1) && (*bit2) ) // 1-false 2-true
	//			return false;
	//		if( (*bit1) && !(*bit2) ) // 1-true 2-false
	//			return false;
	//		//1-false 2-false || 1-true 2-true
	//		//next and continue
	//		bit1++; bit2++;
	//	}
	//	return true;
	//}

	EdgeID eid;
	//const vector<bool>* bits_pt;
	Bits bits;
};

#endif __EDGE_SORT_H__