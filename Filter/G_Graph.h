#ifndef __G_GRAPH_H__
#define __G_GRAPH_H__

#include <vector>
#include <stdio.h>
#include <list>
#include <map>
#include <set>
//#include <algorithm>
#include <deque>

using namespace std;

#include "../share/Utility.h"
#include "../Assemble/A_Graph.h"

#include "G_Node.h"
#include "Edge_sort.h"
#include "PushRelabel.h"

struct edge_cncted_comp{

	list<NodeID> component; //here the node ID is the contracted node ID
	list<NodeID> children; //the node ID is actually size_t, position of its children
	NodeID parent; //the node ID is actually size_t, position of its parent
	NodeID neighbor_id_in_parent;
	NodeSize subtree_size; //zero means the tree node is removed or merged with parent
};

class G_Graph{

public:
	
	G_Graph(){

		//by default
		DNCC = 1;
		DNCF = 10;
	}

	~G_Graph(){
	}

	vector<G_Node>& get_node_list(){
		return this->node_list;
	}

	vector<G_Edge>& get_edge_list(){
		return this->edge_list;
	}

	NodeID contract_id( NodeID nid ){
		return this->contract_to[nid];
	}

	vector<NodeID>& get_cont_node( NodeID nid ){
		return this->contract_node_list[nid];
	}

	G_Edge* const sym_edge( G_Edge* const e ){

		//return this->get_node_list()[e->get_target()].get_adj_list()[e->get_source()];
		return &this->edge_list[ this->sym_id[e->get_id()] ];
	}

	//G_Edge* const sym_edge( EdgeID eid ){

		//G_Edge& e = this->get_edge_list()[eid];
		//return this->get_node_list()[e.get_target()].get_adj_list()[e.get_source()];
	//}

	EdgeID const sym_edge_id( EdgeID eid ){

		//G_Edge& e = this->get_edge_list()[eid];
		//return this->get_node_list()[e.get_target()].get_adj_list()[e.get_source()]->get_id();
		return this->sym_id[eid];
	}

	VERBOSE(
		vector<NodeID> get_del_node(){
			return this->del_cnt_node;
		}
	)

	/////////////////////////////main methods////////////////////////////

	void read_graph(string co_path, string gr_path);

	void dfs_tree( NodeID start, vector<bool>& edge_removed, NodeSize size_lim );

	void bfs_tree( NodeID start, vector<bool>& node_added, NodeSize size_lim ){
		//////////////////////////
	}

	void two_cuts_edge_class( vector<bool>& edge_in_fi,
		vector< vector<NodeID> >& edge_equl_cls );

	void cnt_two_cuts( const vector< vector<EdgeID> >& edge_classes,
		NodeSize sz_lim );

	void cnt_two_degree_path( NodeSize sz_lim );

	void find_natural_cuts( bool natural_cuts[], NodeSize sz_lim);

	void cnt_natural_cuts( bool natural_cuts[] );

	//void convert( A_Graph* ag, vector< vector<NodeID> >& id_map );
	void convert_n_output( string r_path );

	void cnt_one_cuts( const vector<EdgeID>& one_cut_edges, NodeSize sz_lim );

public:
	//natural cuts parameters
	unsigned int DNCC;
	unsigned int DNCF;

private:
	//basic
	vector<G_Node> node_list;
	vector<G_Edge> edge_list;
	//record symmetric edge id
	vector<NodeID> sym_id;

	//deal with node contraction
	vector<NodeID> contract_to;
	vector< vector<NodeID> > contract_node_list;
	vector<NodeID> del_cnt_node; //record delelte contracted nodes, 
								 //also available contract slots

	//////////////////////////main Internal Methods////////////////////////////

	//void fill_b_bits( vector<bool>& edge_removed, 
	//	vector< vector<bool> >& b_bit_list, unsigned int b );
	void fill_b_bits( vector<bool>& edge_removed, 
		vector< Bits >& b_bit_list, unsigned int b );

	//void classify_edge( const vector< vector<bool> >& b_bit_list,
	//	vector< vector<EdgeID> >& edge_classes );
	void classify_edge( const vector< Bits >& b_bit_list,
		vector< vector<EdgeID> >& edge_classes );

	void mark_node_vis( NodeID nid, vector<bool>& mark_list );

	void contract_nodes( const vector<NodeID>& node_list );

	void contract_nodes( NodeID m, NodeID n );

	unsigned int node_degree( NodeID n );

	NodeSize cal_node_size( const vector<NodeID>& node_list );

	NodeSize cal_comp_size( const list<NodeID>& cnode_list );

	NodeID next_center( bool node_in_core[] );

	void mark_node_vis( NodeID nid, bool * mark_list );

	void natural_st_cuts_from_s( bool * natural_cuts, const deque<NodeID>& core,
		const vector<NodeID>& between_nodes );

	void natural_st_cuts_from_t( bool * natural_cuts, const deque<NodeID>& core,
		const vector<NodeID>& between_nodes );

	void contract_nodes( const deque<NodeID>& node_list );

	size_t build_component_tree( const vector<EdgeID>& one_cut_edges, 
		vector<edge_cncted_comp>& component_tree );

	NodeSize fill_subtree_size( vector<edge_cncted_comp>& component_tree, size_t root_p );

	void cnt_proper_tree_components( vector<edge_cncted_comp>& component_tree, size_t root_p,
		NodeSize sz_lim );

	void link_component( vector<edge_cncted_comp>& component_tree, map<NodeID, size_t>&
		comp_cnodes_to_pos, size_t search_pos, size_t parent_pos );

	NodeID contract_cnodes( const list<NodeID>& cnode_list );

	NodeID contract_child_to_parent( NodeID child, NodeID parent );

	NodeID contract_subtree( vector<edge_cncted_comp>& component_tree, size_t root_pos );

	void find_all_comp( vector<edge_cncted_comp>& component_tree, size_t root_pos, 
		list<NodeID>& all_comp );

};


#endif __G_GRAPH_H__