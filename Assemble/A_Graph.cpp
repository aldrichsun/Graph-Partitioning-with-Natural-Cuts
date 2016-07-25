#include "A_Graph.h"

void A_Graph::read_graph_n_idmap( vector< vector<NodeID> >& id_map, string co_path, string gr_path){

		FILE *co_f, *gr_f;
		fopen_s( &co_f, co_path.c_str(), "r");
		check_file( co_f, co_path.c_str() );
		fopen_s( &gr_f, gr_path.c_str(), "r");
		check_file( gr_f, gr_path.c_str() );

		//read in node
		NodeID node_count = 0;
		if( !feof( co_f ) ){
			fscanf_s( co_f, "%u\n", &node_count );
		}
		this->node_list.reserve( node_count );
		id_map.resize( node_count );

		NodeID tid = 0;
		NodeSize sz = 0;
		int i = 0;
		while( !feof(co_f) ){

			fscanf_s( co_f, "%u %u:", &tid, &sz );
			//G_Node tnode(tid-1, tlt, tlg);
			A_Node tnode( tid, sz );
			this->node_list.push_back( tnode );

			vector<NodeID> contain_id;
			for( i = 0; i < sz; i++ ){
				NodeID map_id = 0;
				fscanf_s( co_f, "%u ", &map_id );
				contain_id.push_back( map_id );
			}
			id_map[tid] = contain_id;
			//id_map[tid].assign( contain_id.begin(), contain_id.end() );
		}
		fclose( co_f );

		//read in edge
		NodeID edge_count = 0;
		if( !feof(gr_f) ){
			fscanf_s( gr_f, "%u\n", &edge_count );
		}
		this->edge_list.reserve( edge_count );

		NodeID ts = 0, tt = 0, tw = 0;
		tid = 0;
		while( !feof(gr_f) ){

			fscanf_s( gr_f, "%u %u %u\n", &ts, &tt, &tw );
			A_Edge e(ts, tt, tw, tid++);
			this->edge_list.push_back(e);
			this->node_list[ts].get_adj_list().push_back((A_Edge*)&(this->edge_list.back()));
		}
		fclose( co_f );

		//fill symmetric edge id
		this->sym_id.resize( this->edge_list.size(), 0);
		for( i = 0; i < this->edge_list.size(); i++ ){

			if( i%2 )
				this->sym_id[i] = i+1;
			else
				this->sym_id[i] = i-1;
		}

		//intial contraction
		this->contract_to.resize( this->node_list.size() );
		for( size_t i = 0; i < this->node_list.size(); i++ )
			contract_to[i] = i;
		this->contract_node_list.resize( this->node_list.size() );
		for( size_t i = 0; i < this->node_list.size(); i++ )
			this->contract_node_list[i].push_back( i );

}

void A_Graph::greedy_algorithm_heap( NodeSize sz_lim ){
		
		//initialize the logic edge structure
		vector<Logic_Edge> logic_edges;
		map<NodeID, size_t> logic_edge_counter;
		set<NodeID> node_deleted;
		vector<NodeID> available_new_id;

		NodeID s=0, t=0;
		double score = 0.0;
		Logic_Edge min_e;
		vector<Logic_Edge>::iterator leit;
		//size_t min_pos = 0;
		//min_e.score = numeric_limits<double>::max();

		vector<A_Edge>::const_iterator eit = this->edge_list.begin();
		logic_edges.reserve( 10 * this->edge_list.size() );
		for(; eit != this->edge_list.end(); eit++){

			s = this->contract_to[eit->get_source()];
			t = this->contract_to[eit->get_target()];
			Logic_Edge le(s, t, eit->get_weight());
			le.score = this->cal_edge_score( le );
			logic_edges.push_back( le );

			if( logic_edge_counter.count(s) )
				logic_edge_counter[s]++;
			else
				logic_edge_counter[s] = 1;

			if( logic_edge_counter.count(t) )
				logic_edge_counter[t]++;
			else
				logic_edge_counter[t] = 1;
			//sym edge
			eit++;
		}
		
		//initial heap
		std::make_heap( logic_edges.begin(), logic_edges.end() );
		//end initial

		NodeID new_id = 0;
		map<NodeID, EdgeWeight> accumulate_wet;
		vector<NodeID>::const_iterator nit;
		//assemble those nodes
		while( !logic_edges.empty() ){

			min_e = logic_edges.front();

			std::pop_heap( logic_edges.begin(), logic_edges.end() );
			logic_edges.pop_back();

			logic_edge_counter[min_e.source]--;
			logic_edge_counter[min_e.target]--;

			//recycle resources
			if( logic_edge_counter[min_e.source] == 0 ){
				logic_edge_counter.erase( min_e.source );
				if( node_deleted.count( min_e.source ) )
					available_new_id.push_back( min_e.source );
			}
			if( logic_edge_counter[min_e.target] == 0 ){
				logic_edge_counter.erase( min_e.target );
				if( node_deleted.count( min_e.target ) )
					available_new_id.push_back( min_e.target );
			}

			//if deleted
			if( node_deleted.count( min_e.source ) || node_deleted.count( min_e.target ) )
				continue;

			//contract the two cnodes and update logic edge list
			if( this->cal_cnode_size( min_e.source ) +
				this->cal_cnode_size( min_e.target ) <= sz_lim ){

					//the new node id is determined by available_new_id
					new_id = this->contract_cnodes( min_e.source, min_e.target, available_new_id );
					accumulate_wet.clear();

					//update the list
					//delete old edges
					node_deleted.insert( min_e.source ); //node ids source and target are
					node_deleted.insert( min_e.target ); //no longer valid, but this doesn't
														 //mean they can be new node id.
								//recycle resources
					if( !logic_edge_counter.count( min_e.source ) ){
							available_new_id.push_back( min_e.source );
					}
					if( !logic_edge_counter.count( min_e.target ) ){
							available_new_id.push_back( min_e.target );
					}

					//add new edges
					node_deleted.erase( new_id ); //it is ensured that new_id is not in heap

					nit = this->contract_node_list[new_id].begin();
					for(; nit != this->contract_node_list[new_id].end(); nit++){

						vector<A_Edge*>::const_iterator reit =
							this->node_list[*nit].get_adj_list().begin();
						for(; reit != this->node_list[*nit].get_adj_list().end(); reit++){

							NodeID target = this->contract_to[ (*reit)->get_target() ];
							if( target == new_id )
								continue;
							if( accumulate_wet.count( target ) ){
								accumulate_wet[target]++;
							}
							else{
								accumulate_wet[target] = 1;
							}
						}
					}//end for
					if( logic_edge_counter.count(new_id) )
						logic_edge_counter[new_id] += accumulate_wet.size();
					else
						logic_edge_counter[new_id] = accumulate_wet.size();

					map<NodeID, EdgeWeight>::const_iterator nleit = accumulate_wet.begin();
					for(; nleit != accumulate_wet.end(); nleit++){

						Logic_Edge le(new_id, nleit->first, nleit->second);
						le.score = this->cal_edge_score( le );

						logic_edges.push_back( le );
						std::push_heap( logic_edges.begin(), logic_edges.end() );

						if( logic_edge_counter.count(nleit->first) )
							logic_edge_counter[nleit->first]++;
						else
							logic_edge_counter[nleit->first] = 1;
					}

			}//end if contract

		}//end while

		this->del_cnt_node.assign( available_new_id.begin(), available_new_id.end() );

		return;
}

void A_Graph::greedy_algorithm( NodeSize sz_lim ){
		
		//initialize the logic edge structure
		list<Logic_Edge> logic_edges;

		NodeID s=0, t=0;
		double score = 0.0;
		Logic_Edge min_e;
		list<Logic_Edge>::iterator leit;
		size_t min_pos = 0;
		min_e.score = numeric_limits<double>::max();

		vector<A_Edge>::const_iterator eit = this->edge_list.begin();
		for(; eit != this->edge_list.end(); eit++){

			s = this->contract_to[eit->get_source()];
			t = this->contract_to[eit->get_target()];
			Logic_Edge le(s, t, eit->get_weight());
			le.score = this->cal_edge_score( le );
			logic_edges.push_back( le );

			if( le.score < min_e.score ){
				min_e = le;
				min_pos = (eit - this->edge_list.begin())>>1;
			}
			//sym edge
			eit++;
		}

		//end initial
		leit = logic_edges.begin();
		advance( leit, min_pos );
		logic_edges.erase( leit );

		NodeID new_id = 0;
		map<NodeID, EdgeWeight> accumulate_wet;
		vector<NodeID>::const_iterator nit;
		//assemble those nodes
		while( min_e.score != numeric_limits<double>::max() ){

			//contract the two cnodes and update logic edge list
			if( this->cal_cnode_size( min_e.source ) +
				this->cal_cnode_size( min_e.target ) <= sz_lim ){

					new_id = this->contract_cnodes( min_e.source, min_e.target );
					accumulate_wet.clear();
					//update the list
					//delete old edges
					leit = logic_edges.begin();
					while( leit != logic_edges.end() ){

						if( leit->source == min_e.source || leit->source == min_e.target ||
							leit->target == min_e.source || leit->target == min_e.target ){
								leit = logic_edges.erase( leit );
								continue;
						}
						leit++;
					}
					//add new edges
					nit = this->contract_node_list[new_id].begin();
					for(; nit != this->contract_node_list[new_id].end(); nit++){

						vector<A_Edge*>::const_iterator reit =
							this->node_list[*nit].get_adj_list().begin();
						for(; reit != this->node_list[*nit].get_adj_list().end(); reit++){

							NodeID target = this->contract_to[ (*reit)->get_target() ];
							if( target == new_id )
								continue;
							if( accumulate_wet.count( target ) ){
								accumulate_wet[target]++;
							}
							else{
								accumulate_wet[target] = 1;
							}
						}
					}//end for
					map<NodeID, EdgeWeight>::const_iterator nleit = accumulate_wet.begin();
					for(; nleit != accumulate_wet.end(); nleit++){

						Logic_Edge le(new_id, nleit->first, nleit->second);
						le.score = this->cal_edge_score( le );
						logic_edges.push_back( le );
					}

			}//end if contract

			//find next minimun score logic edge, remove it and loop
			size_t pos_ct = 0;
			min_pos = 0;
			min_e.score = numeric_limits<double>::max();
			if( logic_edges.empty() )
				continue;
			leit = logic_edges.begin();
			for(; leit != logic_edges.end(); leit++, pos_ct++){

				if( leit->score < min_e.score ){
					min_e = *leit;
					min_pos = pos_ct;
				}
			}
			leit = logic_edges.begin();
			advance( leit, min_pos );
			logic_edges.erase( leit );

		}//end while
		return;
}

double A_Graph::cal_edge_score( Logic_Edge& le ){
		
		double r = 0.0;
		unsigned int random_a = (unsigned int)( rand() % 100 ); //rand [0,100)
		unsigned int random_b = 0;
		if( random_a < 3 ){

			random_b = 60 + (unsigned int)( rand() % 41 ); //rand [60,100]
		}
		else{

			random_b = (unsigned int)( rand() % 61 ); //rand [0,60]
		}
		r = random_b/100.0;
		NodeSize ss=0, ts=0;
		
		vector<NodeID>::const_iterator cnit = this->contract_node_list[le.source].begin();
		for(; cnit != this->contract_node_list[le.source].end(); cnit++)
			ss += this->node_list[ *cnit ].get_size();
		cnit = this->contract_node_list[le.target].begin();
		for(; cnit != this->contract_node_list[le.target].end(); cnit++)
			ts += this->node_list[ *cnit ].get_size();

		return r * le.weight * ( 1.0/sqrt((double)ss) + 1.0/sqrt((double)ts) );
}

NodeID A_Graph::contract_cnodes( const vector<NodeID>& cnode_list ){

		NodeID new_node_id = 0;
		if( !this->del_cnt_node.empty() ){
			new_node_id = this->del_cnt_node.back();
			this->del_cnt_node.pop_back();
		}
		else{
			new_node_id = this->contract_node_list.size();
			vector<NodeID> new_node;
			this->contract_node_list.push_back( new_node );
		}

		//set<NodeID> cntr_id_list;
		vector<NodeID>::const_iterator nit = cnode_list.begin();
		for(; nit != cnode_list.end(); nit++ ){

			//NodeID cnid = contract_to[*nit];
			NodeID cid = *nit;

			//if( cnid ){

			//whether this contracted node has been processed before
			//if( cntr_id_list.count( cnid ) ){
			//	continue;
			//}
			//cntr_id_list.insert( cnid );

			//In case 2012-12-07 Now, I think this is impossible, so remove it
			//if( cid == new_node_id )
			//	continue;

			vector<NodeID>::const_iterator cnit = this->contract_node_list[cid].begin();
			for(; cnit != this->contract_node_list[cid].end(); cnit++ ){

				contract_to[*cnit] = new_node_id;
				this->contract_node_list[new_node_id].push_back( *cnit );
			}

			//delete this contracted node
			this->contract_node_list[cid].clear();
			//do not waste time on managing memory
			//this->contract_node_list[cnid].shrink_to_fit();
			del_cnt_node.push_back( cid );

			//}
			/*else{
				contract_to[*nit] = new_node_id;
				this->contract_node_list[new_node_id].push_back( *nit );
			}*/
		}//end for all new nodes
		return new_node_id;
}

NodeID A_Graph::contract_cnodes( NodeID cm, NodeID cn ){

		vector<NodeID> tmp;
		tmp.push_back(cm); tmp.push_back(cn);

		return this->contract_cnodes( tmp );
}

NodeID A_Graph::contract_cnodes( NodeID cm, NodeID cn, vector<NodeID>& available_new_id ){

		//no need to handle del_cnt_node
		NodeID new_node_id = 0;
		if( !available_new_id.empty() ){
			new_node_id = available_new_id.back();
			available_new_id.pop_back();
		}
		else{
			new_node_id = this->contract_node_list.size();
			vector<NodeID> new_node;
			this->contract_node_list.push_back( new_node );
		}

		//set<NodeID> cntr_id_list;
		NodeID cid = cm;
		vector<NodeID>::const_iterator cnit = this->contract_node_list[cid].begin();
		for(; cnit != this->contract_node_list[cid].end(); cnit++ ){

			contract_to[*cnit] = new_node_id;
			this->contract_node_list[new_node_id].push_back( *cnit );
		}
		//delete this contracted node
		this->contract_node_list[cid].clear();
		//del_cnt_node.push_back( cid );

		cid = cn;
		cnit = this->contract_node_list[cid].begin();
		for(; cnit != this->contract_node_list[cid].end(); cnit++ ){

			contract_to[*cnit] = new_node_id;
			this->contract_node_list[new_node_id].push_back( *cnit );
		}
		//delete this contracted node
		this->contract_node_list[cid].clear();
		//del_cnt_node.push_back( cid );

		return new_node_id;
}

NodeSize A_Graph::cal_cnode_size( NodeID cid ){

		NodeSize total_size = 0;
		vector<NodeID>::const_iterator nit = this->contract_node_list[cid].begin();
		for(; nit != this->contract_node_list[cid].end(); nit++){

			total_size += this->node_list[ *nit ].get_size();
		}
		return total_size;
}

NodeSize A_Graph::cal_cnode_size_with_contraction( NodeID cid, contract_struct* new_c ){

		NodeSize total_size = 0;
		vector<NodeID>::const_iterator nit = new_c->contract_node_list[cid].begin();
		for(; nit != new_c->contract_node_list[cid].end(); nit++){

			total_size += this->node_list[ *nit ].get_size();
		}
		return total_size;
}

void A_Graph::convert( F_Graph* ag, map<NodeID, NodeID>& old_to_new ){
		
		//map: old id (contracted node id) to new id
		//map<NodeID, NodeID> old_to_new;
		NodeID new_id = 0;
		for(size_t i = 0; i < this->contract_node_list.size(); i++){

			if( this->contract_node_list[i].empty() )
				continue;
			old_to_new[i] = new_id++;
		}

		//node
		ag->node_list.reserve( new_id );
		NodeID s_id = 0;
		for(size_t i = 0; i < this->contract_node_list.size(); i++){

			if( this->contract_node_list[i].empty() )
				continue;
			A_Node n( s_id, this->cal_cnode_size( i ) );
			ag->node_list.push_back( n );
			s_id++;
		}

		//edge
		ag->edge_list.reserve( this->edge_list.size() ); //large enough, just in case
		ag->sym_id.resize( this->edge_list.size(), 0 );

		map<NodeID, EdgeWeight> accumulate_wet;
		s_id = 0;
		NodeID t_id = 0;
		EdgeID e_id = 0;
		NodeID nt_id = 0;
		for(size_t i = 0; i < this->contract_node_list.size(); i++){

			if( this->contract_node_list[i].empty() )
				continue;
			accumulate_wet.clear();

			vector<NodeID>::const_iterator cnit = this->contract_node_list[i].begin();
			for(; cnit != this->contract_node_list[i].end(); cnit++){

				vector<A_Edge*>::const_iterator eit =
					this->node_list[ *cnit ].get_adj_list().begin();
				for(; eit != this->node_list[ *cnit ].get_adj_list().end(); eit++){

					t_id = this->contract_to[ (*eit)->get_target() ];
					if( t_id == i )
						continue;

					nt_id = old_to_new[t_id];
					if( nt_id < s_id )
						continue;

					if( accumulate_wet.count( nt_id ) )
						accumulate_wet[nt_id] += (*eit)->get_weight();
					else
						accumulate_wet[nt_id] = (*eit)->get_weight();
				}//for all edges
			}//for all contained nodes

			//make new nodes and incident edges
			map<NodeID, EdgeWeight>::const_iterator neit = accumulate_wet.begin();
			for(; neit != accumulate_wet.end(); neit++){

				A_Edge e( s_id, neit->first, neit->second, e_id );
				ag->sym_id[e_id] = e_id + 1;
				e_id++;

				A_Edge sym_e( neit->first,s_id, neit->second, e_id );
				ag->sym_id[e_id] = e_id - 1;
				e_id++;

				ag->edge_list.push_back(e);
				ag->node_list[s_id].get_adj_list().push_back((A_Edge*)&(ag->edge_list.back()));
				ag->edge_list.push_back(sym_e);
				ag->node_list[neit->first].get_adj_list().push_back((A_Edge*)&(ag->edge_list.back()));
			}
			//next node
			s_id++;
		}

		ag->sym_id.resize( e_id );
		//ag->edge_list.resize( e_id );

		return;
}

void A_Graph::convert( F_Graph* ag ){
		
		//map: old id (contracted node id) to new id
		map<NodeID, NodeID> old_to_new;
		NodeID new_id = 0;
		for(size_t i = 0; i < this->contract_node_list.size(); i++){

			if( this->contract_node_list[i].empty() )
				continue;
			old_to_new[i] = new_id++;
		}

		//node
		ag->node_list.reserve( new_id );
		NodeID s_id = 0;
		for(size_t i = 0; i < this->contract_node_list.size(); i++){

			if( this->contract_node_list[i].empty() )
				continue;
			A_Node n( s_id, this->cal_cnode_size( i ) );
			ag->node_list.push_back( n );
			s_id++;
		}

		//edge
		ag->edge_list.reserve( this->edge_list.size() ); //large enough, just in case
		ag->sym_id.resize( this->edge_list.size(), 0 );

		map<NodeID, EdgeWeight> accumulate_wet;
		s_id = 0;
		NodeID t_id = 0;
		EdgeID e_id = 0;
		NodeID nt_id = 0;
		for(size_t i = 0; i < this->contract_node_list.size(); i++){

			if( this->contract_node_list[i].empty() )
				continue;
			accumulate_wet.clear();

			vector<NodeID>::const_iterator cnit = this->contract_node_list[i].begin();
			for(; cnit != this->contract_node_list[i].end(); cnit++){

				vector<A_Edge*>::const_iterator eit =
					this->node_list[ *cnit ].get_adj_list().begin();
				for(; eit != this->node_list[ *cnit ].get_adj_list().end(); eit++){

					t_id = this->contract_to[ (*eit)->get_target() ];
					if( t_id == i )
						continue;

					nt_id = old_to_new[t_id];
					if( nt_id < s_id )
						continue;

					if( accumulate_wet.count( nt_id ) )
						accumulate_wet[nt_id] += (*eit)->get_weight();
					else
						accumulate_wet[nt_id] = (*eit)->get_weight();
				}//for all edges
			}//for all contained nodes

			//make new nodes and incident edges
			map<NodeID, EdgeWeight>::const_iterator neit = accumulate_wet.begin();
			for(; neit != accumulate_wet.end(); neit++){

				A_Edge e( s_id, neit->first, neit->second, e_id );
				ag->sym_id[e_id] = e_id + 1;
				e_id++;

				A_Edge sym_e( neit->first,s_id, neit->second, e_id );
				ag->sym_id[e_id] = e_id - 1;
				e_id++;

				ag->edge_list.push_back(e);
				ag->node_list[s_id].get_adj_list().push_back((A_Edge*)&(ag->edge_list.back()));
				ag->edge_list.push_back(sym_e);
				ag->node_list[neit->first].get_adj_list().push_back((A_Edge*)&(ag->edge_list.back()));
			}
			//next node
			s_id++;
		}

		ag->sym_id.resize( e_id );
		//ag->edge_list.resize( e_id );

		return;
}

void A_Graph::local_search( vector< vector<NodeID> >& result, NodeSize sz_lim ){
		
		//F_Graph result_g;
		//map<NodeID, NodeID> old_to_final;
		//this->convert( &result_g, old_to_final );
		
		//logic_final_graph: < Source, < Target, Counter > >
		map< NodeID, map<NodeID, size_t> > logic_final_graph;
		set< pair<NodeID, NodeID> > random_edges;
		//this->initial_logic_final_edges( logic_final_graph );
		this->initial_logic_final_edges( logic_final_graph, random_edges );

		//map<NodeID, map<NodeID, size_t> >::const_iterator neit = logic_final_graph.begin();
		//for(; neit != logic_final_graph.end(); neit++){

		//	map<NodeID, size_t>::const_iterator lseit = neit->second.begin();
		//	for(; lseit != neit->second.end(); lseit++){

		//		pair<NodeID, NodeID> p( neit->first, lseit->first );
		//		random_edges.insert( p );
		//	}
		//}

		//contract_struct final_contraction;
		////initalize new contraction
		//final_contraction.contract_node_list.resize( this->contract_node_list.size() );
		//vector< vector<NodeID> >::const_iterator cnt_it = this->contract_node_list.begin();
		//for(size_t i = 0; cnt_it != this->contract_node_list.end(); cnt_it++){
		//	final_contraction.contract_node_list[i++].assign( cnt_it->begin(), cnt_it->end() );
		//}
		//final_contraction.contract_to.assign( this->contract_to.begin(), this->contract_to.end() );
		//final_contraction.del_cnt_node.assign( this->del_cnt_node.begin(), this->del_cnt_node.end() );

		//vector<size_t> edge_opt_cter( result_g.get_edge_list().size(), 0 );
		//EdgeID next_eid = 0; // or pair
		//size_t fail_time_ct = 0;
		//while( next_eid < result_g.get_edge_list().size() ){
		pair<NodeID, NodeID> next_edge;
		set< pair<NodeID, NodeID> >::const_iterator next_pos;
		while( !random_edges.empty() ){
			
			//this->next_local_search_edge( logic_final_graph, next_edge );
			next_pos = random_edges.begin();
			advance(next_pos, rand() % random_edges.size() );
			next_edge = *next_pos;

			//if( next_edge.first == 0xffffffff && next_edge.second ==  0xffffffff )
			//	break;

			A_Graph local_graph;
			map<NodeID, NodeID> old_to_local; //contracted neighbor represented by first fragment ID

			EdgeWeight total_old_weight = 0;

			this->auxiliary_instance( local_graph, logic_final_graph, next_edge, 
				old_to_local, total_old_weight );

			//run greedy algorithm on G'
			//local_graph.greedy_algorithm( sz_lim );
			local_graph.greedy_algorithm_heap( sz_lim );

			//judge wheter H' is better than H
			bool better = false;
			better = this->compare_local_contraction( local_graph, total_old_weight );

			//debug
			//better = true;

			//yes, H = H'
			if( better ){

				//convert G' into new contraction (H')
				contract_struct new_contraction;
				vector<NodeID> delete_nodes;
				vector<NodeID> add_nodes;
				this->form_new_contraction( local_graph, old_to_local,
					next_edge, &new_contraction, delete_nodes, add_nodes );

				//keep the better contraction
				this->contract_node_list.assign( new_contraction.contract_node_list.begin(),
					new_contraction.contract_node_list.end() );
				this->contract_to.assign( 
					new_contraction.contract_to.begin(), new_contraction.contract_to.end() );
				this->del_cnt_node.assign( 
					new_contraction.del_cnt_node.begin(), new_contraction.del_cnt_node.end() );

				//update relevant logic final graph
				//delete old edges
				//this->delete_logic_graph_edges( logic_final_graph, delete_nodes );
				this->delete_logic_graph_edges( logic_final_graph, delete_nodes, random_edges );
				//add new edges
				//this->add_logic_graph_edges( logic_final_graph, add_nodes );
				this->add_logic_graph_edges( logic_final_graph, add_nodes, random_edges );
				continue;
			}

			//no, keep H, fail time counter + 1
			logic_final_graph[next_edge.first][next_edge.second]++;
			if( logic_final_graph[next_edge.first][next_edge.second] == 2 )
				random_edges.erase( next_edge );
			logic_final_graph[next_edge.second][next_edge.first]++;
			if( logic_final_graph[next_edge.second][next_edge.first] == 2 )
				random_edges.erase( std::make_pair( next_edge.second, next_edge.first ) );
			//continue
		}//end for all pair edges

		//and turn the final contraction to result
		vector< vector<NodeID> >::const_iterator rit = this->contract_node_list.begin();
		for(; rit != this->contract_node_list.end(); rit++){
			if( rit->empty() )
				continue;
			result.push_back( *rit );
		}
		return;
}

//void A_Graph::auxiliary_instance( F_Graph& result_g, A_Graph& local_graph, 
//	EdgeID next_eid, map<NodeID, NodeID>& old_to_final, map<NodeID,NodeID>& old_to_local,
//	set<NodeID>& cnt_neighbor_id, set<NodeID>& pair_fra_id ){
void A_Graph::auxiliary_instance( A_Graph& local_graph, map<NodeID, map<NodeID, size_t> >& logic_final_graph,
	pair<NodeID, NodeID>& next_edge, map<NodeID,NodeID>& old_to_local, EdgeWeight& total_weight ){//,
	//set<NodeID>& cnt_neighbor_id, set<NodeID>& pair_fra_id ){

		//map<NodeID, NodeID> final_to_old;
		//map<NodeID, NodeID>::const_iterator otnit = old_to_final.begin();
		//for(; otnit != old_to_final.end(); otnit++){
		//	final_to_old[ otnit->second ] = otnit->first;
		//}

		//create the auxiliary instance G'=(V',E')

		set<NodeID> cnt_neighbor_id; //contains the contracted neighbor ID
		set<NodeID> pair_fra_id; //contains all the fragments ID of the pair
		{
			//neighbor ID
			NodeID pn1=0, pn2=0;
			//pn1 = result_g.edge_list[next_eid].get_source();
			//pn2 = result_g.edge_list[next_eid].get_target();
			pn1 = next_edge.first;
			pn2 = next_edge.second;

			//vector<A_Edge*>::const_iterator nei_it = 
			//	result_g.node_list[pn1].get_adj_list().begin();
			//for(; nei_it != result_g.node_list[pn1].get_adj_list().end(); nei_it++){

			//	cnt_neighbor_id.insert( final_to_old[(*nei_it)->get_target()] );
			//}
			//nei_it = result_g.node_list[pn2].get_adj_list().begin();
			//for(; nei_it != result_g.node_list[pn2].get_adj_list().end(); nei_it++){

			//	cnt_neighbor_id.insert( final_to_old[(*nei_it)->get_target()] );
			//}
			map<NodeID, size_t>::const_iterator nei_it = logic_final_graph[pn1].begin();
			for(; nei_it != logic_final_graph[pn1].end(); nei_it++){

				cnt_neighbor_id.insert( nei_it->first );
			}
			nei_it = logic_final_graph[pn2].begin();
			for(; nei_it != logic_final_graph[pn2].end(); nei_it++){

				cnt_neighbor_id.insert( nei_it->first );
			}

			/*
			 * here to calculate the old total weight
			 */
			total_weight = 0;
			set<NodeID>::const_iterator tw_cnit = cnt_neighbor_id.begin();
			for(; tw_cnit != cnt_neighbor_id.end(); tw_cnit++ ){

				NodeID source = *tw_cnit;
				vector<NodeID>::const_iterator tw_nit = 
					this->contract_node_list[source].begin();
				for(; tw_nit != this->contract_node_list[source].end(); tw_nit++){

					vector<A_Edge*>::const_iterator tw_eit =
						this->node_list[*tw_nit].get_adj_list().begin();
					for(; tw_eit != this->node_list[*tw_nit].get_adj_list().end(); tw_eit++){

						NodeID target = this->contract_to[(*tw_eit)->get_target()];
						if( target <= source ) continue;
						if( !cnt_neighbor_id.count( target ) ) continue;
						total_weight += (*tw_eit)->get_weight();
					}
				}
			}
			/*
			 * end calculate the old total weight
			*/
			cnt_neighbor_id.erase(pn1);
			cnt_neighbor_id.erase(pn2);


			//fragments ID
			//NodeID opn1 = final_to_old[pn1];
			//NodeID opn2 = final_to_old[pn2];
			NodeID opn1 = pn1;
			NodeID opn2 = pn2;
			vector<NodeID>::const_iterator cnit = this->contract_node_list[opn1].begin();
			for(; cnit != this->contract_node_list[opn1].end(); cnit++){
				pair_fra_id.insert( *cnit );
			}
			cnit = this->contract_node_list[opn2].begin();
			for(; cnit != this->contract_node_list[opn2].end(); cnit++){
				pair_fra_id.insert( *cnit );
			}

			//now create the nodes of G' (franments local id > neighbors local id)
			local_graph.node_list.reserve( pair_fra_id.size() + cnt_neighbor_id.size() );
			set<NodeID>::const_iterator fit = pair_fra_id.begin();
			NodeID loc_id = 0;
			for(; fit != pair_fra_id.end(); fit++, loc_id++){

				A_Node ln( loc_id, this->node_list[ *fit ].get_size() );
				local_graph.node_list.push_back( ln );
				old_to_local[ *fit ] = loc_id;
			}
			set<NodeID>::const_iterator nei_cell_it = cnt_neighbor_id.begin();
			for(; nei_cell_it != cnt_neighbor_id.end(); nei_cell_it++, loc_id++){

				A_Node ln( loc_id, this->cal_cnode_size( *nei_cell_it ) );
				local_graph.node_list.push_back( ln );
				//in case of duplication, use the first internal node id to represent
				//the contracted node ID or cell old id
				old_to_local[ this->contract_node_list[ *nei_cell_it ][0] ] = loc_id; 
			}
		}
		{	
			//create the edges of G' (franments local id > neighbors local id)
			local_graph.edge_list.reserve( this->edge_list.size() ); //large enough, just in case
			local_graph.sym_id.resize( this->edge_list.size(), 0 );
			
			map<NodeID, EdgeWeight> add_wet;
			NodeID ls=0, lt=0;
			EdgeID loc_e_id = 0;

			//edges from fragments-fragments and fragments-neighbors
			set<NodeID>::const_iterator fnit = pair_fra_id.begin();
			for(; fnit != pair_fra_id.end(); fnit++){

				add_wet.clear();
				ls = old_to_local[*fnit];
				vector<A_Edge*>::const_iterator eit = 
					this->node_list[ *fnit ].get_adj_list().begin();
				for(; eit != this->node_list[ *fnit ].get_adj_list().end(); eit++){

					NodeID target = (*eit)->get_target();
						
					if( pair_fra_id.count( target ) ){
							lt = old_to_local[target];
					}
					else if( cnt_neighbor_id.count( this->contract_to[target] ) ){
						lt = old_to_local[ this->contract_node_list[this->contract_to[target]][0] ];
					}
					else{
						printf("Error occurs: local search: build G' from fragments\n");
						exit(0);
					}
					if( lt < ls )
						continue;
					if( add_wet.count( lt ) )
						add_wet[lt] += (*eit)->get_weight();
					else
						add_wet[lt] = (*eit)->get_weight();
				}//end for all edges
					
				//new edges
				map<NodeID, EdgeWeight>::const_iterator neit = add_wet.begin();
				for(; neit != add_wet.end(); neit++){

					A_Edge e( ls, neit->first, neit->second, loc_e_id );
					local_graph.sym_id[loc_e_id] = loc_e_id + 1;
					loc_e_id++;

					A_Edge sym_e( neit->first, ls, neit->second, loc_e_id );
					local_graph.sym_id[loc_e_id] = loc_e_id - 1;
					loc_e_id++;

					local_graph.edge_list.push_back(e);
					local_graph.node_list[ls].get_adj_list().push_back((A_Edge*)&(local_graph.edge_list.back()));
					local_graph.edge_list.push_back(sym_e);
					local_graph.node_list[neit->first].get_adj_list().push_back((A_Edge*)&(local_graph.edge_list.back()));
				}
			}//end for all fragment nodes

			//edges from neighbors-neighbors
			set<NodeID>::const_iterator nnit = cnt_neighbor_id.begin();
			for(; nnit != cnt_neighbor_id.end(); nnit++){

				add_wet.clear();
				ls = old_to_local[ this->contract_node_list[*nnit][0] ];
				vector<NodeID>::const_iterator ncnit = this->contract_node_list[*nnit].begin();
				for(; ncnit != this->contract_node_list[*nnit].end(); ncnit++){

					vector<A_Edge*>::const_iterator eit = 
						this->node_list[ *ncnit ].get_adj_list().begin();
					for(; eit != this->node_list[ *ncnit ].get_adj_list().end(); eit++){

						NodeID target = (*eit)->get_target();
						
						//no need to check for fragments again, cause it is for sure
						//that (franments local id > neighbors local id), either point
						//to fragments or beyound local scope, continue
						if( !cnt_neighbor_id.count( this->contract_to[target] ) )
							continue;
						lt = old_to_local[ this->contract_node_list[this->contract_to[target]][0] ];

						if( lt <= ls )
							continue;
						if( add_wet.count( lt ) )
							add_wet[lt] += (*eit)->get_weight();
						else
							add_wet[lt] = (*eit)->get_weight();
					}//end for all edges
				}//end for all contracted nodes

				//new edges
				map<NodeID, EdgeWeight>::const_iterator neit = add_wet.begin();
				for(; neit != add_wet.end(); neit++){

					A_Edge e( ls, neit->first, neit->second, loc_e_id );
					local_graph.sym_id[loc_e_id] = loc_e_id + 1;
					loc_e_id++;

					A_Edge sym_e( neit->first, ls, neit->second, loc_e_id );
					local_graph.sym_id[loc_e_id] = loc_e_id - 1;
					loc_e_id++;

					local_graph.edge_list.push_back(e);
					local_graph.node_list[ls].get_adj_list().push_back((A_Edge*)&(local_graph.edge_list.back()));
					local_graph.edge_list.push_back(sym_e);
					local_graph.node_list[neit->first].get_adj_list().push_back((A_Edge*)&(local_graph.edge_list.back()));
				}
			}//end for all neighbor nodes

			local_graph.sym_id.resize( loc_e_id );
			//local_graph.edge_list.resize( loc_e_id );

			//intial contraction
			size_t new_id = local_graph.node_list.size();
			local_graph.contract_to.resize( new_id );
			for( size_t i = 0; i < new_id; i++ )
				local_graph.contract_to[i] = i;
			local_graph.contract_node_list.resize( new_id );
			for( size_t i = 0; i < new_id; i++ )
				local_graph.contract_node_list[i].push_back( i );

		}//end for local variables
		//end for create G'
		return;
}

void A_Graph::form_new_contraction( A_Graph& local_graph,
	map<NodeID, NodeID>& old_to_local, pair<NodeID, NodeID>& next_edge,
	contract_struct* new_contraction, vector<NodeID>& delete_nodes, vector<NodeID>& add_nodes ){

		map<NodeID, NodeID> local_to_old;
		map<NodeID, NodeID>::const_iterator totlit = old_to_local.begin();
		for(; totlit != old_to_local.end(); totlit++){
			local_to_old[ totlit->second ] = totlit->first;
		}

		//initalize new contraction
		new_contraction->contract_node_list.resize( this->contract_node_list.size() );
		vector< vector<NodeID> >::const_iterator cnt_it = this->contract_node_list.begin();
		for(size_t i = 0; cnt_it != this->contract_node_list.end(); cnt_it++){
			new_contraction->contract_node_list[i++].assign( cnt_it->begin(), cnt_it->end() );
		}
		new_contraction->contract_to.assign( this->contract_to.begin(), this->contract_to.end() );
		new_contraction->del_cnt_node.assign( this->del_cnt_node.begin(), this->del_cnt_node.end() );

		//form the new contraction
		vector< vector<NodeID> >::const_iterator cit = local_graph.contract_node_list.begin();
		for(; cit != local_graph.contract_node_list.end(); cit++){

			if( cit->empty() )
				continue;
			vector<NodeID> tmp_cn;
			vector<NodeID>::const_iterator lcnit = cit->begin();
			for(; lcnit != cit->end(); lcnit++){

				NodeID cnt_old_id = local_to_old[ *lcnit ];
				NodeID c_old_id = new_contraction->contract_to[cnt_old_id];
				if( c_old_id == next_edge.first || c_old_id == next_edge.second ){

					tmp_cn.push_back( cnt_old_id );
				}
				else{
					//it must in cnt_neighbor_id
					vector<NodeID>::const_iterator tmp_it = 
						new_contraction->contract_node_list[c_old_id].begin();
					for(; tmp_it !=	new_contraction->contract_node_list[c_old_id].end(); tmp_it++)
						tmp_cn.push_back( *tmp_it );
					//delete the contract node
					new_contraction->contract_node_list[c_old_id].clear();
					new_contraction->del_cnt_node.push_back( c_old_id );
					delete_nodes.push_back( c_old_id );
				}
			}//end for this contracted node

			NodeID new_node_id = 0;
			if( !new_contraction->del_cnt_node.empty() ){
				new_node_id = new_contraction->del_cnt_node.back();
				new_contraction->del_cnt_node.pop_back();
			}
			else{
				new_node_id = new_contraction->contract_node_list.size();
				vector<NodeID> new_node;
				new_contraction->contract_node_list.push_back( new_node );
			}

			lcnit = tmp_cn.begin();
			for(; lcnit != tmp_cn.end(); lcnit++){

				new_contraction->contract_to[ *lcnit ] = new_node_id;
			}
			new_contraction->contract_node_list[new_node_id].assign( tmp_cn.begin(), tmp_cn.end() );
			add_nodes.push_back( new_node_id );
		}//for all contracted node

		//after the use of judgement of uncontracted fragments and contracted neighbors
		//delete the two uncontracted fragments of the pair (two end points of next_edge)
		new_contraction->contract_node_list[next_edge.first].clear();
		new_contraction->del_cnt_node.push_back( next_edge.first );
		delete_nodes.push_back( next_edge.first );
		new_contraction->contract_node_list[next_edge.second].clear();
		new_contraction->del_cnt_node.push_back( next_edge.second );
		delete_nodes.push_back( next_edge.second );

}

bool A_Graph::compare_local_contraction( A_Graph& local_graph, EdgeWeight total_old_weight ){

		set<NodeID> final_node_set;
		vector< vector<NodeID> >::const_iterator cnit = local_graph.contract_node_list.begin();
		for(NodeID i = 0; cnit != local_graph.contract_node_list.end(); cnit++, i++){
			if( !cnit->empty() )
				final_node_set.insert( i );
		}

		EdgeWeight new_total_weight = 0;
		set<NodeID>::const_iterator tw_cnit = final_node_set.begin();
		for(; tw_cnit != final_node_set.end(); tw_cnit++ ){

			NodeID source = *tw_cnit;
			vector<NodeID>::const_iterator tw_nit = 
				local_graph.contract_node_list[source].begin();
			for(; tw_nit != local_graph.contract_node_list[source].end(); tw_nit++){

				vector<A_Edge*>::const_iterator tw_eit =
					local_graph.node_list[*tw_nit].get_adj_list().begin();
				for(; tw_eit != local_graph.node_list[*tw_nit].get_adj_list().end(); tw_eit++){

					NodeID target = local_graph.contract_to[(*tw_eit)->get_target()];
					if( target <= source ) continue;
					if( !final_node_set.count( target ) ) continue;
					new_total_weight += (*tw_eit)->get_weight();
				}
			}
		}
		return new_total_weight < total_old_weight;
}

bool A_Graph::compare_contraction( contract_struct& new_c ){
		
		F_Graph g1, g2;
		this->convert( &g1 );
		this->convert_with_contract( &g2, new_c );
		
		EdgeWeight tw1=0, tw2=0;
		vector<A_Edge>::const_iterator eit;

		eit = g1.edge_list.begin();
		for(; eit != g1.edge_list.end(); eit = eit + 2)
			tw1 += eit->get_weight();
		eit = g2.edge_list.begin();
		for(; eit != g2.edge_list.end(); eit = eit + 2)
			tw2 += eit->get_weight();

		return tw2 < tw1;
}

void A_Graph::convert_with_contract( F_Graph* g, contract_struct& c_s ){

		//map: old id (contracted node id) to new id
		map<NodeID, NodeID> old_to_new;
		NodeID new_id = 0;
		for(size_t i = 0; i < c_s.contract_node_list.size(); i++){

			if( c_s.contract_node_list[i].empty() )
				continue;
			old_to_new[i] = new_id++;
		}

		//node
		g->node_list.reserve( new_id );
		NodeID s_id = 0;
		for(size_t i = 0; i < c_s.contract_node_list.size(); i++){

			if( c_s.contract_node_list[i].empty() )
				continue;
			A_Node n( s_id, this->cal_cnode_size_with_contraction( i, &c_s ) );
			g->node_list.push_back( n );
			s_id++;
		}

		g->edge_list.reserve( this->edge_list.size() ); //large enough, just in case
		g->sym_id.resize( this->edge_list.size(), 0 );

		map<NodeID, EdgeWeight> accumulate_wet;
		s_id = 0;
		NodeID t_id = 0;
		EdgeID e_id = 0;
		NodeID nt_id = 0;
		for(size_t i = 0; i < c_s.contract_node_list.size(); i++){

			if( c_s.contract_node_list[i].empty() )
				continue;
			accumulate_wet.clear();

			vector<NodeID>::const_iterator cnit = c_s.contract_node_list[i].begin();
			for(; cnit != c_s.contract_node_list[i].end(); cnit++){

				vector<A_Edge*>::const_iterator eit =
					this->node_list[ *cnit ].get_adj_list().begin();
				for(; eit != this->node_list[ *cnit ].get_adj_list().end(); eit++){

					t_id = c_s.contract_to[ (*eit)->get_target() ];
					if( t_id == i )
						continue;

					nt_id = old_to_new[t_id];
					if( nt_id < s_id )
						continue;

					if( accumulate_wet.count( nt_id ) )
						accumulate_wet[nt_id] += (*eit)->get_weight();
					else
						accumulate_wet[nt_id] = (*eit)->get_weight();
				}//for all edges
			}//for all contained nodes

			//make new nodes and incident edges
			map<NodeID, EdgeWeight>::const_iterator neit = accumulate_wet.begin();
			for(; neit != accumulate_wet.end(); neit++){

				A_Edge e( s_id, neit->first, neit->second, e_id );
				g->sym_id[e_id] = e_id + 1;
				e_id++;

				A_Edge sym_e( neit->first,s_id, neit->second, e_id );
				g->sym_id[e_id] = e_id - 1;
				e_id++;

				g->edge_list.push_back(e);
				g->node_list[s_id].get_adj_list().push_back((A_Edge*)&(g->edge_list.back()));
				g->edge_list.push_back(sym_e);
				g->node_list[neit->first].get_adj_list().push_back((A_Edge*)&(g->edge_list.back()));
			}
			//next node
			s_id++;
		}

		g->sym_id.resize( e_id );
		//ag->edge_list.resize( e_id );

		return;
}

void A_Graph::initial_logic_final_edges( map<NodeID, map<NodeID, size_t> >& logic_final_graph ){

		size_t s_id = 0;
		vector< vector<NodeID> >::const_iterator sit = 
			this->contract_node_list.begin();
		for(; sit != this->contract_node_list.end(); sit++, s_id++){

			if( sit->empty() )
				continue;

			vector<NodeID>::const_iterator cnit = sit->begin();
			for(; cnit != sit->end(); cnit++){

				vector<A_Edge*>::const_iterator eit =
					this->node_list[*cnit].get_adj_list().begin();
				for(; eit != this->node_list[*cnit].get_adj_list().end(); eit++){

					NodeID t_id = this->contract_to[ (*eit)->get_target() ];
					if( t_id == s_id )
						continue;
					logic_final_graph[s_id][t_id] = 0;
				}//for all edges
			}//for all contracted nodes

		}//end for the node list
		return;
}

void A_Graph::initial_logic_final_edges( map<NodeID, map<NodeID, size_t> >& logic_final_graph,
	set< pair<NodeID, NodeID> >& random_edges){

		size_t s_id = 0;
		vector< vector<NodeID> >::const_iterator sit = 
			this->contract_node_list.begin();
		for(; sit != this->contract_node_list.end(); sit++, s_id++){

			if( sit->empty() )
				continue;

			vector<NodeID>::const_iterator cnit = sit->begin();
			for(; cnit != sit->end(); cnit++){

				vector<A_Edge*>::const_iterator eit =
					this->node_list[*cnit].get_adj_list().begin();
				for(; eit != this->node_list[*cnit].get_adj_list().end(); eit++){

					NodeID t_id = this->contract_to[ (*eit)->get_target() ];
					if( t_id == s_id )
						continue;
					logic_final_graph[s_id][t_id] = 0;
					random_edges.insert( make_pair( s_id, t_id ) );
				}//for all edges
			}//for all contracted nodes

		}//end for the node list
		return;
}

void A_Graph::next_local_search_edge( map<NodeID, map<NodeID, size_t> >& logic_final_graph,
	pair<NodeID, NodeID>& next_edge ){

		vector< pair<NodeID, NodeID> > random;
		map<NodeID, map<NodeID, size_t> >::const_iterator neit = logic_final_graph.begin();
		for(; neit != logic_final_graph.end(); neit++){

			map<NodeID, size_t>::const_iterator lseit = neit->second.begin();
			for(; lseit != neit->second.end(); lseit++){

				if( lseit->second < FI ){
					pair<NodeID, NodeID> p( neit->first, lseit->first );
					random.push_back( p );
				}
			}
		}

		if( random.empty() ){
			next_edge.first = -1;
			next_edge.second = -1;
			return;
		}
		size_t r_pos = (size_t)( rand() % random.size() );
		next_edge.first = random[r_pos].first;
		next_edge.second = random[r_pos].second;
		return;
}

void A_Graph::delete_logic_graph_edges( map<NodeID, map<NodeID, size_t> >& logic_final_graph, 
	vector<NodeID>& delete_nodes ){

		vector<NodeID>::const_iterator dnit = delete_nodes.begin();
		for(; dnit != delete_nodes.end(); dnit++){

			map<NodeID, size_t>::const_iterator adjit = logic_final_graph[ *dnit ].begin();
			for(; adjit != logic_final_graph[*dnit].end(); adjit++){

				//erase <target, source>
				logic_final_graph[adjit->first].erase( *dnit );
			}
			//erase <source, target list>
			logic_final_graph.erase( *dnit );
		}
		return;
}

void A_Graph::delete_logic_graph_edges( map<NodeID, map<NodeID, size_t> >& logic_final_graph, 
	vector<NodeID>& delete_nodes, set< pair<NodeID, NodeID> >& random_edges ){

		vector<NodeID>::const_iterator dnit = delete_nodes.begin();
		for(; dnit != delete_nodes.end(); dnit++){

			map<NodeID, size_t>::const_iterator adjit = logic_final_graph[ *dnit ].begin();
			for(; adjit != logic_final_graph[*dnit].end(); adjit++){

				//erase <target, source>
				logic_final_graph[adjit->first].erase( *dnit );
				random_edges.erase( make_pair( adjit->first, *dnit ) );
				random_edges.erase( make_pair( *dnit, adjit->first ) );
			}
			//erase <source, target list>
			logic_final_graph.erase( *dnit );
		}
		return;
}

void A_Graph::add_logic_graph_edges( map<NodeID, map<NodeID, size_t> >& logic_final_graph, 
	vector<NodeID>& add_nodes ){

		//only traverse souce node list
		vector<NodeID>::const_iterator anit = add_nodes.begin();
		for(; anit != add_nodes.end(); anit++){

			NodeID source = *anit;
			vector<NodeID>::const_iterator cnit = this->contract_node_list[ source ].begin();
			for(; cnit != this->contract_node_list[source].end(); cnit++){

				vector<A_Edge*>::const_iterator eit =
					this->node_list[ *cnit ].get_adj_list().begin();
				for(; eit != this->node_list[*cnit].get_adj_list().end(); eit++){

					NodeID target = this->contract_to[ (*eit)->get_target() ];
					if( source == target )
						continue;
					logic_final_graph[source][target] = 0;
					logic_final_graph[target][source] = 0;
				}
			}
		}
		return;
}

void A_Graph::add_logic_graph_edges( map<NodeID, map<NodeID, size_t> >& logic_final_graph, 
	vector<NodeID>& add_nodes, set< pair<NodeID, NodeID> >& random_edges ){

		//only traverse souce node list
		vector<NodeID>::const_iterator anit = add_nodes.begin();
		for(; anit != add_nodes.end(); anit++){

			NodeID source = *anit;
			vector<NodeID>::const_iterator cnit = this->contract_node_list[ source ].begin();
			for(; cnit != this->contract_node_list[source].end(); cnit++){

				vector<A_Edge*>::const_iterator eit =
					this->node_list[ *cnit ].get_adj_list().begin();
				for(; eit != this->node_list[*cnit].get_adj_list().end(); eit++){

					NodeID target = this->contract_to[ (*eit)->get_target() ];
					if( source == target )
						continue;
					logic_final_graph[source][target] = 0;
					logic_final_graph[target][source] = 0;
					random_edges.insert( make_pair( source, target ) );
					random_edges.insert( make_pair( target, source ) );
				}
			}
		}
		return;
}

void A_Graph::multistart_and_combination( vector< vector<NodeID> >& result, NodeSize sz_lim ){
		
		deque<Pool_Item> pool;
		for(size_t iter = 0; iter < M; iter++){

			VERBOSE(printf("Iteration %u ...\n", iter);)
			Pool_Item pi;	//set<EdgeID> cut_edges;
							//vector< vector<NodeID> > node_clusters;
							//EdgeWeight item_weight;
			//this->greedy_algorithm( sz_lim );
			VERBOSE(printf("first greedy algorithm...");)
			this->greedy_algorithm_heap( sz_lim );
			VERBOSE(printf("done!\n");)

			VERBOSE(printf("local search...");)
			this->local_search( pi.node_clusters, sz_lim );
			VERBOSE(printf("done!\n");)
			this->pick_cut_edges( pi.node_clusters, pi.cut_edges );
			this->cal_cut_set_weight( pi );

			//recover original contraction
			this->contract_to.clear();
			this->contract_node_list.clear();
			this->del_cnt_node.clear();

			size_t node_num = this->node_list.size();
			this->contract_to.resize( node_num );
			this->contract_node_list.resize( node_num );
			for( size_t i = 0; i < node_num; i++ ){
				this->contract_to[i] = i;
				this->contract_node_list[i].push_back( i );
			}


			//pool item is ready
			if( !this->use_combine || iter < (size_t)ceil(sqrt((double)M)) ){
				pool.push_back( pi );
			}
			else{
				VERBOSE(printf("combine and replace...");)
				//pi = P, pi_1 = P', pi_2 = P''
				Pool_Item pi_1, pi_2;
				this->pool_internal_random_combine( pool, pi_1, sz_lim );
				this->combine_two_pool_item( pi, pi_1, pi_2, sz_lim );
				
				//insert in the order of P'', P', P
				this->replace_pool( pool, pi_2 );
				this->replace_pool( pool, pi_1 );
				this->replace_pool( pool, pi );
				VERBOSE(printf("done!\n");)
			}
		}

		EdgeWeight min_weight = numeric_limits<unsigned int>::max();
		deque<Pool_Item>::iterator min_pos;
		deque<Pool_Item>::iterator piit = pool.begin();
		for(; piit != pool.end(); piit++){

			if( piit->item_weight < min_weight ){
				min_weight = piit->item_weight;
				min_pos = piit;
			}
		}
		result.assign( min_pos->node_clusters.begin(), min_pos->node_clusters.end() );
		return;
}

void A_Graph::pick_cut_edges( const vector< vector<NodeID> >& node_clusters, set<EdgeID>& cut_edges ){

		vector< vector<NodeID> >::const_iterator cluit = node_clusters.begin();
		for(; cluit != node_clusters.end(); cluit++){

			vector<NodeID>::const_iterator nit = cluit->begin();
			for(; nit != cluit->end(); nit++){

				NodeID source = *nit;
				vector<A_Edge*>::const_iterator eit =
					this->node_list[ source ].get_adj_list().begin();
				for(; eit != this->node_list[ source ].get_adj_list().end(); eit++){

					NodeID target = (*eit)->get_target();
					if( source > target ) //only consider small id to big id
						continue;
					if( this->contract_to[source] != this->contract_to[target] ){

						cut_edges.insert( (*eit)->get_id() );

					}
				}//for all edges
			}//for all contracted nodes
		}//for all contracted clusters
		return;
}

void A_Graph::cal_cut_set_weight( Pool_Item& pi ){
		
		pi.item_weight = 0;
		set<EdgeID>::const_iterator eit = pi.cut_edges.begin();
		for(; eit != pi.cut_edges.end(); eit++){
			pi.item_weight += this->edge_list[ *eit ].get_weight();
		}
		return;
}

void A_Graph::combine_two_pool_item( Pool_Item& pi_1, Pool_Item& pi_2, Pool_Item& pi_result,
	NodeSize sz_lim ){

		A_Graph tmp_g; //need deep copy
		//node
		tmp_g.node_list.reserve( this->node_list.size() );
		vector<A_Node>::const_iterator nit = this->node_list.begin();
		for(; nit != this->node_list.end(); nit++){

			A_Node n( nit->get_id(), nit->get_size() );
			tmp_g.node_list.push_back( n );
		}
		//edge
		tmp_g.edge_list.reserve( this->edge_list.size() );
		vector<A_Edge>::const_iterator eit = this->edge_list.begin();
		for(; eit != this->edge_list.end(); eit++){

			EdgeWeight w = eit->get_weight();
			size_t num_of_contain = 0;
			if( pi_1.cut_edges.count( eit->get_id() ) ||
				pi_1.cut_edges.count( this->sym_id[eit->get_id()] ) )
				num_of_contain++;
			if( pi_2.cut_edges.count( eit->get_id() ) ||
				pi_2.cut_edges.count( this->sym_id[eit->get_id()] ) )
				num_of_contain++;
			switch( num_of_contain ){
				case 0: w *= 5;
					break;
				case 1: w *= 3;
					break;
				case 2: w *= 2;
					break;
				default: break;
			};
			A_Edge e( eit->get_source(), eit->get_target(), w, eit->get_id() );
			tmp_g.edge_list.push_back( e );
			tmp_g.node_list[ eit->get_source() ].get_adj_list().push_back( 
				(A_Edge*)&(tmp_g.edge_list.back()) );
		}
		//sym_id;
		tmp_g.sym_id.assign( this->sym_id.begin(), this->sym_id.end() );
		//recover original contraction
		tmp_g.contract_to.assign( this->contract_to.begin(), this->contract_to.end() );
		tmp_g.contract_node_list.assign( this->contract_node_list.begin(),
			this->contract_node_list.end() );
		tmp_g.del_cnt_node.assign( this->del_cnt_node.begin(), this->del_cnt_node.end() );
		//end deep copy

		//tmp_g.greedy_algorithm( sz_lim );
		tmp_g.greedy_algorithm_heap( sz_lim );
		tmp_g.local_search( pi_result.node_clusters, sz_lim );
		tmp_g.pick_cut_edges( pi_result.node_clusters, pi_result.cut_edges );
		this->cal_cut_set_weight( pi_result );

		return;
}

void A_Graph::pool_internal_random_combine( deque<Pool_Item>& pool, 
	Pool_Item& pi_result, NodeSize sz_lim ){

		size_t p1=0, p2=0;
		p1 = (size_t)( rand() % pool.size() );
		p2 = (size_t)( rand() % pool.size());
		while( p2 == p1 ){
			p2 = (size_t)( rand() % pool.size());
		}

		this->combine_two_pool_item( pool[p1], pool[p2], pi_result, sz_lim );
		return;
}

void A_Graph::replace_pool( deque<Pool_Item>& pool, Pool_Item& pi ){
		
		size_t min_dif = numeric_limits<size_t>::max();
		deque<Pool_Item>::iterator del_pos;
		deque<Pool_Item>::iterator piit = pool.begin();
		for(; piit != pool.end(); piit++){

			if( piit->item_weight < pi.item_weight )
				continue;
			size_t dif = this->set_sym_dif( piit->cut_edges, pi.cut_edges );
			if( dif < min_dif ){

				min_dif = dif;
				del_pos = piit;
			}
		}
		if( min_dif != numeric_limits<size_t>::max() ){

			pool.erase( del_pos );
			pool.push_back( pi );
		}
		return;
}

size_t A_Graph::set_sym_dif( const set<EdgeID>& s1, const set<EdgeID>& s2 ){
		
		set<EdgeID> s1_s2, s2_s1;
		this->set_minus( s1, s2, s1_s2 );
		this->set_minus( s2, s1, s2_s1 );
		s1_s2.insert( s2_s1.begin(), s2_s1.end() );
		return s1_s2.size();
}

void A_Graph::set_minus( const set<EdgeID>& head, const set<EdgeID>& tail, set<EdgeID>& result ){

		set<EdgeID>::const_iterator eidit = head.begin();
		for(; eidit != head.end(); eidit++){

			if( tail.count( *eidit ) )
				continue;
			result.insert( *eidit );
		}
		return;
}