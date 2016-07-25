#ifndef __A_GRAPH_H__
#define __A_GRAPH_H__

#include <vector>
#include <stdio.h>
#include <time.h>
#include <list>
#include <map>
#include <set>
#include <deque>
#include <cmath>
#include <algorithm>

using namespace std;

#include "A_Node.h"
#include "../share/Utility.h"
#include "Logic_Edge.h"
#include "F_Graph.h"
#include "Pool_Item.h"

struct contract_struct{

	//deal with node contraction
	vector<NodeID> contract_to;
	vector< vector<NodeID> > contract_node_list;
	vector<NodeID> del_cnt_node; //record delelte contracted nodes, 
								 //also available contract slots

};

class A_Graph{

public:
	
	A_Graph(){
		FI = 16;
		M = 1;
		use_combine = false;
	}

	~A_Graph(){
	}

	vector<A_Node>& get_node_list(){
		return this->node_list;
	}

	vector<A_Edge>& get_edge_list(){
		return this->edge_list;
	}

	NodeID contract_id( NodeID nid ){
		return this->contract_to[nid];
	}

	vector<NodeID>& get_cont_node( NodeID nid ){
		return this->contract_node_list[nid];
	}

	A_Edge* const sym_edge( A_Edge* const e ){

		return &this->edge_list[ this->sym_id[e->get_id()] ];
	}

	EdgeID const sym_edge_id( EdgeID eid ){

		return this->sym_id[eid];
	}

	VERBOSE(
		vector<NodeID> get_del_node(){
			return this->del_cnt_node;
		}
	)

	/////////////////////////////main methods////////////////////////////

	void read_graph_n_idmap( vector< vector<NodeID> >& id_map, string co_path, string gr_path);

	void greedy_algorithm( NodeSize sz_lim );

	void greedy_algorithm_heap( NodeSize sz_lim );

	void local_search( vector< vector<NodeID> >& result, NodeSize sz_lim );

	void convert( F_Graph* fg, map<NodeID, NodeID>& old_to_new );

	void convert( F_Graph* fg );

	void auxiliary_instance( 
		//F_Graph& result_g,
		A_Graph& local_graph, 
		//EdgeID next_eid,
		//map<NodeID, NodeID>& old_to_final,
		map<NodeID, map<NodeID, size_t> >& logic_final_graph,
		pair<NodeID, NodeID>& next_edge,
		map<NodeID,NodeID>& old_to_local,
		//set<NodeID>& cnt_neighbor_id,
		//set<NodeID>& pair_fra_id
		EdgeWeight& total_weight
		);

	void form_new_contraction( 
		A_Graph& local_graph,
		map<NodeID, NodeID>& old_to_local,
		//set<NodeID>& cnt_neighbor_id,
		//set<NodeID>& pair_fra_id,
		pair<NodeID, NodeID>& next_edge,
		contract_struct* new_contraction,
		vector<NodeID>& delete_nodes,
		vector<NodeID>& add_nodes
		);

	bool compare_contraction( contract_struct& new_c );

	bool compare_local_contraction( A_Graph& local_graph, EdgeWeight total_old_weight );

	void convert_with_contract( F_Graph* g, contract_struct& c_s );

	void initial_logic_final_edges( map<NodeID, map<NodeID, size_t> >& logic_final_graph );

	void initial_logic_final_edges( map<NodeID, map<NodeID, size_t> >& logic_final_graph,
		set< pair<NodeID, NodeID> >& random_edges );

	void multistart_and_combination( vector< vector<NodeID> >& result, NodeSize sz_lim );

	void pick_cut_edges( const vector< vector<NodeID> >& node_clusters, set<EdgeID>& cut_edges );

public:

	unsigned int FI;
	unsigned int M;
	bool use_combine;

private:

	friend class G_Graph;

	//basic
	vector<A_Node> node_list;
	vector<A_Edge> edge_list;
	vector<NodeID> sym_id;

	//deal with node contraction
	vector<NodeID> contract_to;
	vector< vector<NodeID> > contract_node_list;
	vector<NodeID> del_cnt_node; //record delelte contracted nodes, 
								 //also available contract slots

	//////////////////////////main Internal Methods////////////////////////////

	void mark_node_vis( NodeID nid, vector<bool>& mark_list );

	NodeID contract_cnodes( const vector<NodeID>& cnode_list );

	NodeID contract_cnodes( NodeID cm, NodeID cn );

	NodeID contract_cnodes( NodeID cm, NodeID cn, vector<NodeID>& available_new_id );

	unsigned int node_degree( NodeID n );

	NodeSize cal_cnode_size( NodeID cid );

	NodeSize cal_cnode_size_with_contraction( NodeID cid, contract_struct* new_c );

	void mark_node_vis( NodeID nid, bool * mark_list );

	void contract_nodes( const deque<NodeID>& node_list );

	//////////////////////////newly add Internal Methods///////////////////////////

	double cal_edge_score(Logic_Edge& le);

	void next_local_search_edge( map<NodeID, map<NodeID, size_t> >& logic_final_graph,
		pair<NodeID, NodeID>& next_edge );

	void delete_logic_graph_edges( map<NodeID, map<NodeID, size_t> >& logic_final_graph, 
		vector<NodeID>& delete_nodes );

	void delete_logic_graph_edges( map<NodeID, map<NodeID, size_t> >& logic_final_graph, 
		vector<NodeID>& delete_nodes, set< pair<NodeID, NodeID> >& random_edges );

	void add_logic_graph_edges( map<NodeID, map<NodeID, size_t> >&logic_final_graph, 
		vector<NodeID>& add_nodes );

	void add_logic_graph_edges( map<NodeID, map<NodeID, size_t> >&logic_final_graph, 
		vector<NodeID>& add_nodes, set< pair<NodeID, NodeID> >& random_edges );

	void cal_cut_set_weight( Pool_Item& pi );

	void combine_two_pool_item( Pool_Item & pi_1, Pool_Item & pi_2, Pool_Item & pi_result,
		NodeSize si_lim );

	void pool_internal_random_combine( deque<Pool_Item>& pool, 
		Pool_Item & pi_result, NodeSize sz_lim );

	void replace_pool( deque<Pool_Item>& pool, Pool_Item& pi );

	size_t set_sym_dif( const set<EdgeID>& s1, const set<EdgeID>& s2 );

	void set_minus( const set<EdgeID>& head, const set<EdgeID>& tail, set<EdgeID>& result );

};


#endif __A_GRAPH_H__