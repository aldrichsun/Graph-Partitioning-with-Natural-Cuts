#include "G_Graph.h"

///////////////////////public methods///////////////////////////

void G_Graph::read_graph(string co_path, string gr_path){

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

		char tc = 0;
		char skip[200];
		NodeID tid = 0;
		int tlt = 0, tlg = 0;
		while( !feof(co_f) ){

			//fscanf_s( co_f, "%c", &tc );
			tc = fgetc( co_f );
			if( tc != 'v' ){
				fgets( skip, 200, co_f );
				continue;
			}
			fscanf_s( co_f, "%u %d %d\n", &tid, &tlg, &tlt );
			//G_Node tnode(tid-1, tlt, tlg);
			G_Node tnode(tid-1);
			this->node_list.push_back( tnode );
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

			//fscanf_s( gr_f, "%c", &tc );
			tc = fgetc( gr_f );
			if( tc != 'a' ){
				fgets( skip, 200, gr_f );
				continue;
			}
			fscanf_s( gr_f, "%u %u %u\n", &ts, &tt, &tw );
			G_Edge e(ts-1, tt-1, tid++);
			this->edge_list.push_back(e);
			//this->node_list[ts-1].get_adj_list()[tt-1] = (G_Edge*)&(this->edge_list.back());
			this->node_list[ts-1].get_adj_list().push_back((G_Edge*)&(this->edge_list.back()));
		}
		fclose( co_f );

		//fill symmetric edge id
		this->sym_id.resize( this->edge_list.size(), 0);
		size_t eid = 0;
		vector<G_Edge>::const_iterator eit = this->edge_list.begin();
		for(; eit != this->edge_list.end(); eit++, eid++ ){

			vector<G_Edge*>::const_iterator syeit = 
				this->node_list[eit->get_target()].get_adj_list().begin();
			for(; syeit != this->node_list[eit->get_target()].get_adj_list().end(); syeit++ ){

				if( (*syeit)->get_target() == eit->get_source() ){
					this->sym_id[eid] = (*syeit)->get_id();
					break;
				}
			}
		}

		//intial contraction
		this->contract_to.resize( this->node_list.size() );
		for( size_t i = 0; i < this->node_list.size(); i++ )
			contract_to[i] = i;
		this->contract_node_list.resize( this->node_list.size() );
		for( size_t i = 0; i < this->node_list.size(); i++ )
			this->contract_node_list[i].push_back( i );

}

void G_Graph::dfs_tree( NodeID start, vector<bool>& edge_removed, NodeSize size_lim = 0 ){

		vector<bool> node_visited( this->node_list.size(), false );
		vector<NodeID> node_stack;
		node_stack.reserve( this->node_list.size() );
		////////
		// NodeSize total_sz = 0;
		////////

		node_stack.push_back(start);
		node_visited[start] = true;

		while( !node_stack.empty() ){

			NodeID n = node_stack.back();
			node_stack.pop_back();

			//map<NodeID, G_Edge*>::const_iterator it = this->node_list[n].get_adj_list().begin();
			vector<G_Edge*>::const_iterator it = this->node_list[n].get_adj_list().begin();
			for(; it != this->node_list[n].get_adj_list().end(); it++){

				//NodeID t = it->second->get_target();
				NodeID t = (*it)->get_target();
				if( !node_visited[t] ){

					//every two related edges should be updated at the same time
					//EdgeID e2t = it->second->get_id();
					EdgeID e2t = (*it)->get_id();
					edge_removed[e2t] = false;
					EdgeID e2s = this->sym_edge_id(e2t);
					edge_removed[e2s] = false;

					node_stack.push_back( t );
					node_visited[t] = true;
				}
			}//end for
		}//endf while
		return;
}

void G_Graph::two_cuts_edge_class( vector<bool>& edge_in_fi,
	vector< vector<NodeID> >& edge_equl_cls ){

	//fill b bits on each edge to classify them
	unsigned int b = 0;
	/*b = (unsigned int)ceil( ( log((double)graph.get_node_list().size())
		+ 2.0*log((double)graph.get_edge_list().size()) ) / log(2.0) );*/
	//for efficiency concern, we give up using vector<bool> or bitset,
	//but unsigned __int64 or unsigned long long instead
	b = 64;

	//vector<bool> b_bit( b, false );
	//vector< vector<bool> > b_bit_list( graph.get_edge_list().size(), b_bit );
	vector<Bits> b_bit_list( this->get_edge_list().size(), 0 );
	this->fill_b_bits( edge_in_fi, b_bit_list, b );

	//classify edges into different classes
	//edge id equivalent classes
	this->classify_edge( b_bit_list, edge_equl_cls );

	return;
}

void G_Graph::cnt_two_cuts( const vector< vector<EdgeID> >& edge_classes,
	NodeSize sz_lim ){

		//vector<bool> edge_closed( this->edge_list.size(), false );

		//edge_classes[0] is the class of 1-cut
		vector< vector<EdgeID> >::const_iterator ecit = edge_classes.begin() + 1;
		
		for(; ecit != edge_classes.end(); ecit++){ //deal with one edge class

			if( ecit->size() < 4 )
				continue;

			size_t comp_lim = ecit->size()/2 - 1; // k

			//close edges in this class
			//and initially mark class edges unvisited
			map<NodeID, bool> class_edge_visited;
			vector<EdgeID>::const_iterator eit = ecit->begin();
			for(; eit != ecit->end(); eit++){
				//edge_closed[*eit] = true;
				class_edge_visited[*eit] = false;
			}

			//start from first edge
			EdgeID eid = ecit->at(0); 
			class_edge_visited[eid] = true;
			class_edge_visited[this->sym_edge_id(eid)] = true;

			//initially put start nodes
			vector<bool> node_visited( this->node_list.size(), false );
			vector< vector<NodeID> > stacks;
			stacks.resize( 2 );
			stacks[0].push_back( this->edge_list[eid].get_source() );
			stacks[1].push_back( this->edge_list[eid].get_target() );

			//find small components to contract
			size_t di = 1;
			vector< vector<NodeID> > component;
			component.resize( 2 );
			EdgeID neid[2] = {0,0};
			size_t comp_cnt = 0;
			while( !stacks[0].empty() && !stacks[1].empty() ){

				/*
				 * do not use while( true ) because since the edge 
				 * can be contained by a already visited and contracted
				 * component
				 */

				di = 1 - di; //switch direction

				NodeID n = stacks[di].back();
				stacks[di].pop_back();

				if( node_visited[n] ) continue;
				this->mark_node_vis( n, node_visited );

				component[di].push_back( n ); //record the node id consisting of the compnent

				if( comp_lim == 1 ){
					if( component[di].size() > sz_lim && component[1-di].size() > sz_lim )
						break;
				}

				//if( this->contract_to[n] ){ //node is contracted
				NodeID m = this->contract_to[n];
				vector<NodeID>::const_iterator nit = this->contract_node_list[m].begin();
				for(; nit != this->contract_node_list[m].end(); nit++){

					vector<G_Edge*>::const_iterator trit = // all target nodes iterator
						this->node_list[*nit].get_adj_list().begin();
					for(; trit != this->node_list[*nit].get_adj_list().end(); trit++){

						//whether this edge is closed
						if( class_edge_visited.count((*trit)->get_id()) ){
							//mark next traverse edge
							if( !class_edge_visited[(*trit)->get_id()] ){
								//the edge has not been visited
								neid[di] = (*trit)->get_id();
								class_edge_visited[ neid[di] ] = true;
								class_edge_visited[this->sym_edge_id(neid[di])] = true;
							}
							continue;
						}

						if( node_visited[(*trit)->get_target()] )
							continue;
						stacks[di].push_back( (*trit)->get_target() );
					}
				}

				//}
				//else{ //node is not contracted
				//	
				//	vector<G_Edge*>::const_iterator trit = //all target nodes
				//		this->node_list[n].get_adj_list().begin();
				//	for(; trit != this->node_list[n].get_adj_list().end(); trit++){

				//		//whether this edge is closed
				//		if( class_edge_visited.count((*trit)->get_id()) ){
				//			//mark next traverse edge
				//			if( !class_edge_visited[(*trit)->get_id()] ){
				//				//the edge has not been visited
				//				neid[di] = (*trit)->get_id();
				//				class_edge_visited[ neid[di] ] = true;
				//				class_edge_visited[this->sym_edge_id(neid[di])] = true;
				//			}
				//			continue;
				//		}
				//		if( node_visited[(*trit)->get_target()] )
				//			continue;
				//		stacks[di].push_back( (*trit)->get_target() );
				//	}

				//}//end else

				// if an stack is empty, one component has been found
				// contract it. If k-1 components have been contracted,
				// then process next edge class
				if( stacks[di].empty() ){

					if( this->cal_node_size( component[di] ) <= sz_lim )
						this->contract_nodes( component[di] );
					component[di].clear();
					
					comp_cnt++;
					if( comp_cnt >= comp_lim ){
						//stacks[0].clear();
						//stacks[1].clear();
						break; //next edge class
					}
					//else next component
					stacks[di].push_back( this->edge_list[ neid[di] ].get_target() );
					//continue;
				}

			}//end while
		}//end for all edge classes
		return;
}

//void G_Graph::fill_b_bits( vector<bool>& edge_removed, 
//	vector< vector<bool> >& b_bit_list, unsigned int b ){
//
//		/*fill initial edges not in tree*/
//
//		////mt19937 eng;
//		////uniform_int<int> unif(0,RANDOM_LEN);
//		//Release
//		//srand((unsigned int)time(NULL)); //initial here, since two seeds can be within 1'
//		//Debug
//		srand(40); //!!!!!!!!!Remmber to change when release!!!!!!!!!!!!!!!
//		int random = 0;
//		int middle = RANDOM_LEN/2;
//
//		vector<bool> edge_prsd( this->get_edge_list().size(), false );
//		vector<bool>::const_iterator eit = edge_removed.begin();
//		vector<bool>::iterator b_it, syb_it;
//		EdgeID b_id = 0;
//		for(; eit != edge_removed.end(); eit++, b_id++ ){
//
//			if( *eit && !edge_prsd[b_id] ){
//
//				b_it = b_bit_list[b_id].begin();
//				syb_it = b_bit_list[this->sym_edge_id(b_id)].begin();
//				for(; b_it - b_bit_list[b_id].begin() < b; b_it++, syb_it++ ){
//
//					random = (int)((rand()/(double)RAND_MAX)*(RANDOM_LEN+1)); //rand [0,1000)
//					if( random <= middle ){
//						*b_it = true;
//						*syb_it = true;
//					}
//				}
//				edge_prsd[b_id] = true;
//				edge_prsd[this->sym_edge_id(b_id)] = true;
//			}
//		}
//		//for( size_t i = 0 ; i < b; i++ ){
//
//		//	size_t ep = 0;
//		//	vector<bool>::const_iterator eit = edge_removed.begin();
//		//	////mt19937 eng;
//		//	////uniform_int<int> unif(0,RANDOM_LEN);
//		//	//srand((unsigned int)time(NULL));
//		//	for(; eit != edge_removed.end(); eit++ ){
//		//		
//		//		if( *eit ){ //if edge removed (in fi)
//		//			//random = unif(eng);
//		//			random = (int)((rand()/(double)RAND_MAX)*(RANDOM_LEN+1)); //rand [0,1000)
//		//			if( random <= middle )
//		//				b_bit_list[ep][i] = true;
//		//		}
//		//		ep++;
//		//	}
//		//}
//		////randomly make symetric edges contains same bits
//		//for(size_t ep = 0; ep < this->get_edge_list().size(); ep++){
//
//		//	if( edge_removed[ep] && !edge_prsd[ep] ){
//		//		random = (int)((rand()/(double)RAND_MAX)*(RANDOM_LEN+1)); //rand [0,1000)
//		//		if( random <= middle ){
//		//			b_bit_list[this->sym_edge_id(ep)].assign(
//		//				b_bit_list[ep].begin()
//		//				,b_bit_list[ep].end());
//		//		}
//		//		else{
//		//			b_bit_list[ep].assign(
//		//				b_bit_list[this->sym_edge_id(ep)].begin()
//		//				,b_bit_list[this->sym_edge_id(ep)].end());
//		//		}
//		//		edge_prsd[ep] = true;
//		//		edge_prsd[this->sym_edge_id(ep)] = true;
//		//	}
//		//}
//
//		/*fill all edges in tree*/
//
//		//count every node remain edges
//		vector<size_t> node_rem_edge( this->get_node_list().size() );
//		vector< set<NodeID> > degree_node;
//		vector<G_Node>::iterator nit = this->get_node_list().begin();
//		vector<size_t>::iterator cn_it = node_rem_edge.begin();
//		for(; nit != this->get_node_list().end(); nit++, cn_it++ ){
//
//			size_t remain_edge_count = nit->get_adj_list().size();
//			//map<NodeID, G_Edge*>::iterator eit = nit->get_adj_list().begin();
//			vector<G_Edge*>::iterator eit = nit->get_adj_list().begin();
//			for(; eit != nit->get_adj_list().end(); eit++){
//
//				//if( edge_removed[eit->second->get_id()] )
//				if( edge_removed[(*eit)->get_id()] )
//					remain_edge_count--;
//			}
//			*cn_it = remain_edge_count;
//			if( remain_edge_count + 1 > degree_node.size() )
//				degree_node.resize( remain_edge_count + 1 );
//			if( remain_edge_count )
//				degree_node[remain_edge_count].insert( nit->get_id() );
//		}
//
//		while( true ){
//
//			//no leaf edge, stop
//			if( degree_node[1].empty() )
//				break;
//			//find a leaf node
//			nit = this->node_list.begin() + (*degree_node[1].begin());
//			degree_node[1].erase( degree_node[1].begin() );
//
//			//find a leaf node
//			//nit = this->get_node_list().begin();
//			//cn_it = node_rem_edge.begin();
//			//for(; cn_it != node_rem_edge.end(); cn_it++, nit++){
//
//			//	if( 1 ==  *cn_it )//1 == remain_edge_count
//			//		break; //nit points to a leaf node
//			//}
//			//no leaf edge, stop
//			//if( nit == this->get_node_list().end() )
//			//	break;
//
//			//fill the leaf edge -- I
//			EdgeID eid = 0;
//			//map<NodeID, G_Edge*>::iterator eit = nit->get_adj_list().begin();
//			vector<G_Edge*>::iterator eit = nit->get_adj_list().begin();
//			while( eit != nit->get_adj_list().end() ){
//				//if( !edge_removed[eit->second->get_id()] ){
//				//	eid = eit->second->get_id(); //get the leaf edge id
//				//	break;
//				//}
//				if( !edge_removed[(*eit)->get_id()] ){
//					eid = (*eit)->get_id(); //get the leaf edge id
//					break;
//				}
//				++eit;
//			}
//
//			//remove the leaf node
//			edge_removed[eid] = true;
//			edge_removed[this->sym_edge_id(eid)] = true;
//			//node_rem_edge[eit->second->get_source()]--;
//			//node_rem_edge[eit->second->get_target()]--;
//			/*size_t sd = 0;
//			sd = node_rem_edge[(*eit)->get_source()];
//			degree_node[sd].erase( degree_node[sd].find((*eit)->get_source()) );
//			node_rem_edge[(*eit)->get_source()]--;
//			if( node_rem_edge[(*eit)->get_source()] )
//				degree_node[node_rem_edge[(*eit)->get_source()]].insert((*eit)->get_source());*/
//			size_t td = 0;
//			td = node_rem_edge[(*eit)->get_target()];
//			degree_node[td].erase( degree_node[td].find((*eit)->get_target()) );
//			node_rem_edge[(*eit)->get_target()]--;
//			if( node_rem_edge[(*eit)->get_target()] )
//				degree_node[node_rem_edge[(*eit)->get_target()]].insert((*eit)->get_target());
//
//			//fill the leaf edge -- II
//			eit = nit->get_adj_list().begin();
//			for(; eit != nit->get_adj_list().end(); eit++){
//
//				//if( eit->second->get_id() == eid )
//				if( (*eit)->get_id() == eid )
//					continue;
//				for(size_t i = 0; i < b; i++){
//					b_bit_list[eid][i] = b_bit_list[eid][i] ^ b_bit_list[(*eit)->get_id()][i];
//						//^ b_bit_list[eit->second->get_id()][i]);
//				}
//
//				b_bit_list[this->sym_edge_id(eid)].assign( b_bit_list[eid].begin(),
//					b_bit_list[eid].end() );
//			}
//		}//end while
//
//		//all edges have b-bit
//		return;
//}

void G_Graph::fill_b_bits( vector<bool>& edge_removed, 
	vector< Bits >& b_bits, unsigned int b ){

		/*fill initial edges not in tree*/
		unsigned int random = 0;

		vector<bool> edge_prsd( this->get_edge_list().size(), false );
		vector<bool>::const_iterator eit = edge_removed.begin();
		EdgeID b_id = 0;
		for(; eit != edge_removed.end(); eit++, b_id++ ){

			if( *eit && !edge_prsd[b_id] ){

				random = (unsigned int)rand(); //rand (0, 2^15)
				random &= 0x0000000f;
				b_bits[b_id] |= random;
				for( size_t i = 0; i < 4 ; i++ ){

					random = (unsigned int)rand(); //rand (0, 2^15)
					b_bits[b_id] <<= 15;
					b_bits[b_id] |= random;
				}
				b_bits[this->sym_edge_id(b_id)] = b_bits[b_id];

				edge_prsd[b_id] = true;
				edge_prsd[this->sym_edge_id(b_id)] = true;
			}
		}
		
		/*fill all edges in tree*/

		//count every node remain edges
		vector<size_t>			node_rem_edge( this->get_node_list().size() );
		vector< set<NodeID> >	degree_node;

		vector<G_Node>::iterator nit = this->get_node_list().begin();
		vector<size_t>::iterator cn_it = node_rem_edge.begin();
		for(; nit != this->get_node_list().end(); nit++, cn_it++ ){

			size_t remain_edge_count = nit->get_adj_list().size();
			//map<NodeID, G_Edge*>::iterator eit = nit->get_adj_list().begin();
			vector<G_Edge*>::iterator eit = nit->get_adj_list().begin();
			for(; eit != nit->get_adj_list().end(); eit++){

				//if( edge_removed[eit->second->get_id()] )
				if( edge_removed[(*eit)->get_id()] )
					remain_edge_count--;
			}
			*cn_it = remain_edge_count;
			if( remain_edge_count + 1 > degree_node.size() )
				degree_node.resize( remain_edge_count + 1 );
			if( remain_edge_count )
				degree_node[remain_edge_count].insert( nit->get_id() );
		}

		while( true ){

			//no leaf edge, stop
			if( degree_node[1].empty() )
				break;
			//find a leaf node
			nit = this->node_list.begin() + (*degree_node[1].begin());
			degree_node[1].erase( degree_node[1].begin() );

			//fill the leaf edge -- I
			EdgeID eid = 0;
			vector<G_Edge*>::iterator eit = nit->get_adj_list().begin();
			while( eit != nit->get_adj_list().end() ){

				if( !edge_removed[(*eit)->get_id()] ){
					eid = (*eit)->get_id(); //get the leaf edge id
					break;
				}
				++eit;
			}

			//remove the leaf node
			edge_removed[eid] = true;
			edge_removed[this->sym_edge_id(eid)] = true;

			size_t td = 0;
			td = node_rem_edge[(*eit)->get_target()];
			degree_node[td].erase( degree_node[td].find((*eit)->get_target()) );
			node_rem_edge[(*eit)->get_target()]--;
			if( node_rem_edge[(*eit)->get_target()] )
				degree_node[node_rem_edge[(*eit)->get_target()]].insert((*eit)->get_target());

			//fill the leaf edge -- II
			eit = nit->get_adj_list().begin();
			for(; eit != nit->get_adj_list().end(); eit++){

				if( (*eit)->get_id() == eid )
					continue;
				b_bits[eid] = b_bits[eid] ^ b_bits[(*eit)->get_id()];
			}
			b_bits[this->sym_edge_id(eid)] = b_bits[eid];
		}//end while

		//all edges have b-bit
		return;
}

void G_Graph::classify_edge( const vector< Bits >& b_bit_list,
		vector< vector<EdgeID> >& edge_classes ){

			//O(n^2) find equal edges
			//vector<bool> edge_prsd( this->get_edge_list().size(), false );

			//O(nlogn) sort, need more spaces
			list< Edge_sort > sort_bits;
			for( size_t i = 0; i < b_bit_list.size(); i++ ){

				Edge_sort tes( i, b_bit_list[i] );
				sort_bits.push_back( tes );
			}
			//sort( sort_bits.begin(), sort_bits.end() );
			sort_bits.sort();

			edge_classes.resize( this->edge_list.size() );
			size_t new_class = 0;
			Edge_sort cmp( sort_bits.front() );
			//edge_classes[new_class].push_back( cmp.eid );

			list< Edge_sort >::const_iterator esit = sort_bits.begin();
			for(; esit != sort_bits.end(); esit++ ){

				if( *esit == cmp ){
					edge_classes[new_class].push_back( esit->eid );
					continue;
				}
				else{
					++new_class;
					cmp.eid = esit->eid;
					cmp.bits = esit->bits;
					edge_classes[new_class].push_back( cmp.eid );
				}
			}
			edge_classes.resize(new_class+1);
			return;
}


void G_Graph::mark_node_vis( NodeID nid, vector<bool>& mark_list ){

		NodeID cid = this->contract_to[nid];
		//if( cid ){ //have been contracted
		vector<NodeID>::const_iterator nit = this->contract_node_list[cid].begin();
		for(; nit != this->contract_node_list[cid].end(); nit++)
			mark_list[*nit] = true;
		return;
		//}
		//mark_list[nid] = true;
}

void G_Graph::contract_nodes( const vector<NodeID>& node_list ){

		if( node_list.size() <= 1 ) //do nothing
			return;

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
		vector<NodeID>::const_iterator nit = node_list.begin();
		for(; nit != node_list.end(); nit++ ){

			NodeID cnid = contract_to[*nit];

			//if( cnid ){

			//whether this contracted node has been processed before
			//if( cntr_id_list.count( cnid ) ){
			//	continue;
			//}
			//cntr_id_list.insert( cnid );

			//In case 2012-12-07 Now, I think this is impossible, so remove it
			//if( cnid == new_node_id )
			//	continue;

			vector<NodeID>::const_iterator cnit = this->contract_node_list[cnid].begin();
			for(; cnit != this->contract_node_list[cnid].end(); cnit++ ){

				contract_to[*cnit] = new_node_id;
				this->contract_node_list[new_node_id].push_back( *cnit );
			}

			//delete this contracted node
			this->contract_node_list[cnid].clear();
			//do not waste time on managing memory
			//this->contract_node_list[cnid].shrink_to_fit();
			del_cnt_node.push_back( cnid );

			//}
			/*else{
				contract_to[*nit] = new_node_id;
				this->contract_node_list[new_node_id].push_back( *nit );
			}*/
		}//end for all new nodes
		return;
}

NodeID G_Graph::contract_cnodes( const list<NodeID>& node_list ){

		if( node_list.size() <= 1 ) //do nothing
			return node_list.front();

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
		list<NodeID>::const_iterator nit = node_list.begin();
		for(; nit != node_list.end(); nit++ ){

			NodeID cnid = *nit;

			vector<NodeID>::const_iterator cnit = this->contract_node_list[cnid].begin();
			for(; cnit != this->contract_node_list[cnid].end(); cnit++ ){

				contract_to[*cnit] = new_node_id;
				this->contract_node_list[new_node_id].push_back( *cnit );
			}

			//delete this contracted node
			this->contract_node_list[cnid].clear();
			del_cnt_node.push_back( cnid );

		}//end for all new nodes
		return new_node_id;
}

void G_Graph::contract_nodes( NodeID m, NodeID n ){

		vector<NodeID> tmp;
		tmp.push_back(m); tmp.push_back(n);
		this->contract_nodes( tmp );
		return;
}

void G_Graph::cnt_two_degree_path( NodeSize sz_lim ){
		
		vector<bool> node_visited( this->node_list.size(), false );

		vector<G_Node>::iterator nit = this->node_list.begin();
		for(; nit != this->node_list.end(); nit++ ){

			if( this->node_degree( nit->get_id() ) != 2 )
				node_visited[ nit->get_id() ] = true;
		}

							   nit = this->node_list.begin();
		vector<bool>::iterator vis_it = node_visited.begin();
		size_t nid = 0;
		for(; nit != this->node_list.end(); nit++, vis_it++, nid++ ){

			if( *vis_it ) continue;

			vector<NodeID> stack;
			stack.push_back( nid );

			vector<NodeID> two_de_path;

			while( !stack.empty() ){

				NodeID n = stack.back();
				stack.pop_back();

				if( node_visited[n] ) continue;
				this->mark_node_vis( n, node_visited );

				two_de_path.push_back( n ); //one node is enough to represent the contracted node

				NodeID m = this->contract_to[n];
				vector<NodeID>::const_iterator nit = this->contract_node_list[m].begin();
				for(; nit != this->contract_node_list[m].end(); nit++){

					vector<G_Edge*>::const_iterator trit = this->node_list[*nit].get_adj_list().begin();
					for(; trit != this->node_list[*nit].get_adj_list().end(); trit++){

						if( node_visited[(*trit)->get_target()] )
							continue;

						stack.push_back( (*trit)->get_target() );
					}
				}

				if( stack.empty() ){

					if( this->cal_node_size( two_de_path ) <= sz_lim )
						this->contract_nodes( two_de_path );
					two_de_path.clear();
				}

			}//end while

		}//end for all nodes
		return;
}

unsigned int G_Graph::node_degree( NodeID n ){
		
		unsigned int degree = 0;
		NodeID cid = this->contract_to[n];
		vector<NodeID>::const_iterator cnit = 
			this->contract_node_list[cid].begin();
		for(; cnit != this->contract_node_list[cid].end(); cnit++ ){

			vector<G_Edge*>::const_iterator trit = 
				this->node_list[*cnit].get_adj_list().begin();
			for(; trit != this->node_list[*cnit].get_adj_list().end(); trit++){

				NodeID trid = (*trit)->get_target();
				if( this->contract_to[trid] == cid )
					continue;
				degree++;
			}
		}
		return degree;
}

NodeSize G_Graph::cal_node_size( const vector<NodeID>& n_list ){

		NodeSize total_size = 0;
		vector<NodeID>::const_iterator nit = n_list.begin();
		for(; nit != n_list.end(); nit++){

			NodeID cid = this->contract_to[*nit];
			total_size += this->contract_node_list[cid].size();
		}
		return total_size;
}

NodeSize G_Graph::cal_comp_size( const list<NodeID>& cn_list ){

		NodeSize total_size = 0;
		list<NodeID>::const_iterator nit = cn_list.begin();
		for(; nit != cn_list.end(); nit++){

			total_size += this->contract_node_list[*nit].size();
		}
		return total_size;
}

void G_Graph::find_natural_cuts( bool natural_cuts[], NodeSize sz_lim ){

	NodeSize core_lim = sz_lim/DNCF;
	
	//dectect natural cuts C times
	for( int t = 0; t < DNCC; t++ ){

		//Release
		//srand((unsigned int)time(NULL)); //every time different:: no need

		bool * node_in_core = NULL;
		node_in_core = new bool[this->node_list.size()];
		check_new( node_in_core, "find natural cuts: node in core");
		memset( node_in_core, false, this->node_list.size() );

		NodeID nc = 0;

		while( true ){

			nc = this->next_center( node_in_core );
			if( nc == -1u ) //0xffffffff )
				break;

			bool * node_visited = NULL;
			node_visited = new bool[this->node_list.size()];
			check_new( node_visited, "find natural cuts: node visited");
			memset( node_visited, false, this->node_list.size() );

			deque<NodeID> core; //core and between_nodes all contain the contracted id
			vector<NodeID> between_nodes;
			between_nodes.reserve( sz_lim );
			//between nodes: to calculate s-t cut, new id is the index
			//old id is the content, and index 0 is reserved for core
			between_nodes.push_back( 0 );

			deque<NodeID> nc_queue;
			nc_queue.push_back( nc );
			this->mark_node_vis( nc, node_visited );

			bool first_always_add = true;

			NodeSize total_size = 0;

			while( !nc_queue.empty() ){

				NodeID n = nc_queue.front();
				nc_queue.pop_front();

				/*if( node_visited[n] ) continue;
				this->mark_node_vis( n, node_visited );*/

				NodeID cid = this->contract_to[n];

				total_size += this->contract_node_list[cid].size();
				if( total_size > sz_lim ){

					//no need to record neighbor, if a target node
					//cannot be found in all recorded nodes, it is
					//the neighbor (ring) node
					//nc_queue.push_front( n );
					//natural cuts mark array, core, ring-core between nodes
					this->natural_st_cuts_from_s( natural_cuts, core, between_nodes );
					//this->natural_st_cuts_from_t( natural_cuts, core, between_nodes );

					break;
				}
				if( total_size <= core_lim || first_always_add ){

					//record the contracted node id
					core.push_back( cid );
					this->mark_node_vis( n, node_in_core );
					first_always_add = false;
				}
				else{

					between_nodes.push_back( cid );
				}

				vector<NodeID>::const_iterator cnit = this->contract_node_list[cid].begin();
				for(; cnit != this->contract_node_list[cid].end(); cnit++){

					vector<G_Edge*>::const_iterator eit =
						this->node_list[*cnit].get_adj_list().begin();
					for(; eit != this->node_list[*cnit].get_adj_list().end(); eit++){

						if( node_visited[(*eit)->get_target()] )
							continue;

						nc_queue.push_back( (*eit)->get_target() );
						this->mark_node_vis( (*eit)->get_target(), node_visited );

					}//end for all targets
				}//end for all original nodes in the contracted node

			}//end while

			delete[] node_visited;
		}

		delete[] node_in_core;

	}
	return;
}

NodeID G_Graph::next_center( bool node_in_core[] ){

		vector<NodeID> remain_id;
		remain_id.reserve( this->node_list.size() );
		for(int i = 0; i < this->node_list.size(); i++){
			if( !node_in_core[i] )
				remain_id.push_back(i);
		}

		if( remain_id.empty() )
			return -1u;

		//random = (int)((rand()/(double)RAND_MAX)*(RANDOM_LEN+1));
		int random = (int)( rand() % remain_id.size() );

		//DEBUG
		//random = 0;

		return remain_id[random];
}

void G_Graph::mark_node_vis( NodeID nid, bool * mark_list ){

		NodeID cid = this->contract_to[nid];
		//if( cid ){ //have been contracted
		vector<NodeID>::const_iterator nit = this->contract_node_list[cid].begin();
		for(; nit != this->contract_node_list[cid].end(); nit++)
			mark_list[*nit] = true;
		return;
		//}
		//mark_list[nid] = true;
}

void G_Graph::natural_st_cuts_from_s( bool * natural_cuts, const deque<NodeID>& core,
		const vector<NodeID>& between_nodes ){

			//core and between_nodes form the tree T
			PushRelabel pr( between_nodes.size() + 1 );
			NodeID neighborID = between_nodes.size();

			map<NodeID, NodeID> old_to_new;
			{ //to destroy temp variable
				//map: old id --> new id
				vector<NodeID>::const_iterator nit = between_nodes.begin() + 1;
				NodeID new_id = 1;
				for(; nit != between_nodes.end(); nit++, new_id++ ){
					old_to_new[ *nit ] = new_id;
				}
				deque<NodeID>::const_iterator cit = core.begin();
				for(; cit != core.end(); cit++ ){
					old_to_new[ *cit ] = 0 ; //new index of core is 0
				}

				// map: target --> weight
				map<NodeID, EdgeWeight> accumulate_wet;

				//first, handle core, which contains contracted node id
				//node_visited[0] = true; //core id is 0
				cit = core.begin();
				for(; cit != core.end(); cit++ ){

					vector<NodeID>::const_iterator cnit =
						this->contract_node_list[ *cit ].begin();
					for(; cnit != this->contract_node_list[ *cit ].end(); cnit++){

						vector<G_Edge*>::const_iterator eit = 
							this->node_list[ *cnit ].get_adj_list().begin();
						for(; eit != this->node_list[ *cnit ].get_adj_list().end(); eit++){

							NodeID new_tid = 0;
							if( old_to_new.count( this->contract_to[(*eit)->get_target()] ) ){
								new_tid = old_to_new[this->contract_to[(*eit)->get_target()]];
							}
							else{
								new_tid = neighborID;
							}
							if( new_tid > 0 ){
								if( accumulate_wet.count( new_tid ) )
									accumulate_wet[new_tid]++;
								else
									accumulate_wet[new_tid] = 1;
							}
						}
					}//for all node in the contracted node

				}//for all contracted node in core
				map<NodeID, EdgeWeight>::const_iterator i_eit = accumulate_wet.begin();//i = 0
				for(; i_eit != accumulate_wet.end(); i_eit++){
					pr.AddEdge( 0, i_eit->first, i_eit->second );
					//////////////////newly added/////////////////
					pr.AddEdge( i_eit->first, 0, i_eit->second );
				}

				//second, handle between nodes
				nit = between_nodes.begin() + 1;
				NodeID new_sid = 1;
				for(; nit != between_nodes.end(); nit++, new_sid++){

					accumulate_wet.clear();

					vector<NodeID>::const_iterator cnit = 
						this->contract_node_list[ *nit ].begin();
					for(; cnit != this->contract_node_list[ *nit ].end(); cnit++){

						vector<G_Edge*>::const_iterator eit =
							this->node_list[ *cnit ].get_adj_list().begin();
						for(; eit != this->node_list[ *cnit ].get_adj_list().end(); eit++){

							NodeID new_tid = 0;
							if( old_to_new.count( this->contract_to[(*eit)->get_target()] ) ){
								new_tid = old_to_new[this->contract_to[(*eit)->get_target()]];
							}
							else{
								new_tid = neighborID;
							}
							if( new_tid > new_sid ){
								if( accumulate_wet.count( new_tid ) )
									accumulate_wet[new_tid]++;
								else
									accumulate_wet[new_tid] = 1;
							}
						}
					}//for all contracted nodes

					i_eit = accumulate_wet.begin();//i = new_sid
					for(; i_eit != accumulate_wet.end(); i_eit++){
						pr.AddEdge( new_sid, i_eit->first, i_eit->second );
						//////////////////newly added/////////////////
						pr.AddEdge( i_eit->first, new_sid, i_eit->second );
					}
				}//for all between nodes

				pr.GetMaxFlow( 0, neighborID );
			}
			
			//mark natural cuts

			//record node sets of S, every other node is in T
			set<NodeID> nc_source( core.begin(), core.end() );

			bool * node_visited = NULL;
			node_visited = new bool[ between_nodes.size() + 1 ];
			check_new( node_visited, "natural_st_cuts: node_visited" );
			memset( node_visited, false, between_nodes.size() + 1 );

			node_visited[0] = true;
			node_visited[neighborID] = true;

			deque<NodeID> nc_que;
			nc_que.push_back( 0 ); //start from 0

			while( !nc_que.empty() ){

				NodeID n = nc_que.front();
				nc_que.pop_front();

				//if( node_visited[ n ] ) continue;
				//node_visited[n] = true;

				if( n ){
					//between_nodes can work as new_to_old
					nc_source.insert( between_nodes[n] );
				}

				vector<Edge>::const_iterator eit = pr.G[n].begin();
				for(; eit != pr.G[n].end(); eit++){

					if( node_visited[ eit->to ] )
						continue;

					//if( eit->cap == 0 || eit->cap > eit->flow ){
					//if( eit->cap != 0 && eit->cap > eit->flow ){
					if( eit->cap > eit->flow ){
						nc_que.push_back( eit->to );
						node_visited[ eit->to ] = true;
					}
				}
			}

			set<NodeID>::const_iterator sr_it = nc_source.begin();
			for(; sr_it != nc_source.end(); sr_it++){

				NodeID cid = *sr_it;
				vector<NodeID>::const_iterator cnit = 
					this->contract_node_list[ cid ].begin();
				for(; cnit != this->contract_node_list[cid].end(); cnit++){

					vector<G_Edge*>::const_iterator eit = 
						this->node_list[ *cnit ].get_adj_list().begin();
					for(; eit != this->node_list[ *cnit ].get_adj_list().end(); eit++){

						NodeID target = this->contract_to[ (*eit)->get_target() ];
						if( nc_source.count( target ) )
							continue;//else

						/////////////////////have a try//////////////////////
						NodeID pr_s = old_to_new[cid];
						NodeID pr_t = old_to_new[target];
						bool satr = true;
						vector<Edge>::const_iterator pr_eit = pr.G[pr_s].begin();
						for(; pr_eit != pr.G[pr_s].end(); pr_eit++){

							if( pr_eit->to == pr_t ){
								//if( eit->cap == 0 || eit->cap > eit->flow ){
								if( pr_eit->cap > pr_eit->flow ){
									satr = false;
								}
								break;
							}
						}
						if( !satr )
							continue;
						/////////////////////////////////////////////////////

						natural_cuts[ (*eit)->get_id() ] = true;
						///////////////////have a try//////////////////////
						//natural_cuts[ this->sym_edge_id( (*eit)->get_id() ) ] = true;
						///////////////////////////////////////////////////
					}//for all edges from this contracted node
				}//for all contracted nodes
			}//for all nodes in S

			delete[] node_visited;

			return;
}

void G_Graph::natural_st_cuts_from_t( bool * natural_cuts, const deque<NodeID>& core,
		const vector<NodeID>& between_nodes ){

			deque<NodeID> neighbor;

			//core and between_nodes form the tree T
			PushRelabel pr( between_nodes.size() + 1 );
			NodeID neighborID = between_nodes.size();

			map<NodeID, NodeID> old_to_new;
			{ //to destroy temp variable
				//map: old id --> new id
				vector<NodeID>::const_iterator nit = between_nodes.begin() + 1;
				NodeID new_id = 1;
				for(; nit != between_nodes.end(); nit++, new_id++ ){
					old_to_new[ *nit ] = new_id;
				}
				deque<NodeID>::const_iterator cit = core.begin();
				for(; cit != core.end(); cit++ ){
					old_to_new[ *cit ] = 0 ; //new index of core is 0
				}

				// map: target --> weight
				map<NodeID, EdgeWeight> accumulate_wet;

				//first, handle core, which contains contracted node id
				//node_visited[0] = true; //core id is 0
				cit = core.begin();
				for(; cit != core.end(); cit++ ){

					vector<NodeID>::const_iterator cnit =
						this->contract_node_list[ *cit ].begin();
					for(; cnit != this->contract_node_list[ *cit ].end(); cnit++){

						vector<G_Edge*>::const_iterator eit = 
							this->node_list[ *cnit ].get_adj_list().begin();
						for(; eit != this->node_list[ *cnit ].get_adj_list().end(); eit++){

							NodeID new_tid = 0;
							if( old_to_new.count( this->contract_to[(*eit)->get_target()] ) ){
								new_tid = old_to_new[this->contract_to[(*eit)->get_target()]];
							}
							else{
								new_tid = neighborID;
								neighbor.push_back( this->contract_to[(*eit)->get_target()] );
							}
							if( new_tid > 0 ){
								if( accumulate_wet.count( new_tid ) )
									accumulate_wet[new_tid]++;
								else
									accumulate_wet[new_tid] = 1;
							}
						}
					}//for all node in the contracted node

				}//for all contracted node in core
				map<NodeID, EdgeWeight>::const_iterator i_eit = accumulate_wet.begin();//i = 0
				for(; i_eit != accumulate_wet.end(); i_eit++){
					pr.AddEdge( 0, i_eit->first, i_eit->second );
					//////////////////newly added/////////////////
					//pr.AddEdge( i_eit->first, 0, i_eit->second );
				}

				//second, handle between nodes
				nit = between_nodes.begin() + 1;
				NodeID new_sid = 1;
				for(; nit != between_nodes.end(); nit++, new_sid++){

					accumulate_wet.clear();

					vector<NodeID>::const_iterator cnit = 
						this->contract_node_list[ *nit ].begin();
					for(; cnit != this->contract_node_list[ *nit ].end(); cnit++){

						vector<G_Edge*>::const_iterator eit =
							this->node_list[ *cnit ].get_adj_list().begin();
						for(; eit != this->node_list[ *cnit ].get_adj_list().end(); eit++){

							NodeID new_tid = 0;
							if( old_to_new.count( this->contract_to[(*eit)->get_target()] ) ){
								new_tid = old_to_new[this->contract_to[(*eit)->get_target()]];
							}
							else{
								new_tid = neighborID;
								neighbor.push_back( this->contract_to[(*eit)->get_target()] );
							}
							if( new_tid > new_sid ){
								if( accumulate_wet.count( new_tid ) )
									accumulate_wet[new_tid]++;
								else
									accumulate_wet[new_tid] = 1;
							}
						}
					}//for all contracted nodes

					i_eit = accumulate_wet.begin();//i = new_sid
					for(; i_eit != accumulate_wet.end(); i_eit++){
						pr.AddEdge( new_sid, i_eit->first, i_eit->second );
						//////////////////newly added/////////////////
						//pr.AddEdge( i_eit->first, new_sid, i_eit->second );
					}
				}//for all between nodes

				pr.GetMaxFlow( 0, neighborID );
			}
			
			//mark natural cuts

			//record node sets of T, every other node is in S
			set<NodeID> nc_target( neighbor.begin(), neighbor.end() );
			deque<NodeID> pr_target;

			bool * node_visited = NULL;
			node_visited = new bool[ between_nodes.size() + 1 ];
			check_new( node_visited, "natural_st_cuts: node_visited" );
			memset( node_visited, false, between_nodes.size() + 1 );

			node_visited[0] = true;
			node_visited[neighborID] = true;

			deque<NodeID> nc_que;
			nc_que.push_back( neighborID ); //start from t (neighborID)

			//build the reverse flow graph to traverse from t
			Rv_Graph r_pr( pr.N );

			vector< vector<Edge> >::const_iterator onit = pr.G.begin();
			for(; onit != pr.G.end(); onit++){

				vector<Edge>::const_iterator oeit = onit->begin();
				for(; oeit != onit->end(); oeit++){

					if( oeit->cap == 0 ) //consider no residual edges
						continue;
					r_pr.add_reverse_edge( *oeit );
				}
			}
			//build done and start traversing
			while( !nc_que.empty() ){

				NodeID n = nc_que.back();
				nc_que.pop_back();

				pr_target.push_back( n );
				if( n != neighborID ){
					//between_nodes can work as new_to_old
					nc_target.insert( between_nodes[n] );
				}

				vector<Edge>::const_iterator eit = r_pr.G[n].begin();
				for(; eit != r_pr.G[n].end(); eit++){

					if( node_visited[ eit->to ] )
						continue;

					if( eit->cap > eit->flow ){
						nc_que.push_back( eit->to );
						node_visited[ eit->to ] = true;
					}
				}
			}

			set<NodeID>::const_iterator tr_it = nc_target.begin();
			for(; tr_it != nc_target.end(); tr_it++){

				NodeID cid = *tr_it;
				vector<NodeID>::const_iterator cnit = 
					this->contract_node_list[ cid ].begin();
				for(; cnit != this->contract_node_list[cid].end(); cnit++){

					vector<G_Edge*>::const_iterator eit = 
						this->node_list[ *cnit ].get_adj_list().begin();
					for(; eit != this->node_list[ *cnit ].get_adj_list().end(); eit++){

						NodeID target = this->contract_to[ (*eit)->get_target() ];
						if( nc_target.count( target ) )
							continue;//else

						natural_cuts[ (*eit)->get_id() ] = true;
						///////////////////have a try//////////////////////
						natural_cuts[ this->sym_edge_id( (*eit)->get_id() ) ] = true;
						///////////////////////////////////////////////////
					}//for all edges from this contracted node
				}//for all contracted nodes
			}//for all nodes in S

			//unmark those wrong natural cuts
			memset( node_visited, false, between_nodes.size() + 1 );

			deque<NodeID>::const_iterator prnit = pr_target.begin();
			for(; prnit != pr_target.end(); prnit++){

				NodeID s = *prnit;
				if( node_visited[ s ] )
					continue;

				vector<NodeID> pr_que;
				pr_que.push_back( s );

				//dfs from s
				while( !pr_que.empty() ){

					NodeID n = pr_que.back();
					pr_que.pop_back();

					if( node_visited[n] )
						continue;
					node_visited[n] = true;

					//neighbor (or t) has no such edges
					vector<Edge>::const_iterator preit = pr.G[n].begin();
					for(; preit != pr.G[n].end(); preit++){

						NodeID nt = preit->to;
						if( preit->cap == 0 ) //consider no residual edges
							continue;
						if( preit->cap == preit->flow ){ //satr edge

							pr_que.push_back( nt );

							//nt cannot be core (or s/0)
							if( nt == neighborID ){
								deque<NodeID>::const_iterator neibit = neighbor.begin();
								for(; neibit != neighbor.end(); neibit++){

									NodeID cid = *neibit;
									vector<NodeID>::const_iterator nit = 
										this->contract_node_list[cid].begin();
									for(; nit != this->contract_node_list[cid].end(); nit++){

										NodeID nid = *nit;
										vector<G_Edge*>::const_iterator eit = 
											this->node_list[nid].get_adj_list().begin();
										for(; eit != this->node_list[nid].get_adj_list().end(); eit++){

											NodeID target = this->contract_to[(*eit)->get_target()];
											if( target == between_nodes[n] ){
												natural_cuts[ (*eit)->get_id() ] = false;
												natural_cuts[ this->sym_edge_id( (*eit)->get_id() ) ] = false;
											}
										}
									}
								}//end for all neighbors in the ring
							}//if neighbor ID
							else{
								NodeID cid = between_nodes[nt];
								vector<NodeID>::const_iterator nit = 
									this->contract_node_list[cid].begin();
								for(; nit != this->contract_node_list[cid].end(); nit++){

									NodeID nid = *nit;
									vector<G_Edge*>::const_iterator eit = 
										this->node_list[nid].get_adj_list().begin();
									for(; eit != this->node_list[nid].get_adj_list().end(); eit++){

										NodeID target = this->contract_to[(*eit)->get_target()];
										if( target == between_nodes[n] ){
											natural_cuts[ (*eit)->get_id() ] = false;
											natural_cuts[ this->sym_edge_id( (*eit)->get_id() ) ] = false;
										}
									}
								}
							}//else between nodes
						}//if satr edge
					}//all out going edges
				}//dfs from s
			}//for all nodes in the search tree

			delete[] node_visited;

			return;
}

/*
 * refer to contract_nodes( const vector<NodeID>& node_list )
*/
void G_Graph::contract_nodes( const deque<NodeID>& node_list ){

		if( node_list.size() <= 1 ) //do nothing
			return;

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

		deque<NodeID>::const_iterator nit = node_list.begin();
		for(; nit != node_list.end(); nit++ ){

			NodeID cnid = contract_to[*nit];

			//2012-12-12 Just in case
			if( cnid == new_node_id )
				continue;

			vector<NodeID>::const_iterator cnit = this->contract_node_list[cnid].begin();
			for(; cnit != this->contract_node_list[cnid].end(); cnit++ ){

				contract_to[*cnit] = new_node_id;
				this->contract_node_list[new_node_id].push_back( *cnit );
			}

			//delete this contracted node
			this->contract_node_list[cnid].clear();
			//do not waste time on managing memory
			//this->contract_node_list[cnid].shrink_to_fit();
			del_cnt_node.push_back( cnid );

		}//end for all new nodes
		return;
}

void G_Graph::cnt_natural_cuts( bool * natural_cuts ){

		bool * node_visited = NULL;
		node_visited = new bool[ this->node_list.size() ];
		check_new( node_visited, "cnt natural cuts: node_visited" );
		memset( node_visited, false, this->node_list.size() );

		deque<NodeID> component;
				
		vector<NodeID> stack;
		stack.reserve( this->node_list.size()/100 );

		for(int i = 0; i < this->node_list.size(); i++){

			if( node_visited[i] )
				continue;

			component.clear();
			stack.clear();

			//node i is not visited
			stack.push_back( i );
			this->mark_node_vis( i, node_visited );

			while( !stack.empty() ){

				NodeID n = stack.back();
				stack.pop_back();

				component.push_back( n );
				NodeID cid = this->contract_to[n];
				
				vector<NodeID>::const_iterator cnit = 
					this->contract_node_list[cid].begin();
				for(; cnit != this->contract_node_list[cid].end(); cnit++){

					vector<G_Edge*>::const_iterator eit =
						this->node_list[ *cnit ].get_adj_list().begin();
					for(; eit != this->node_list[*cnit].get_adj_list().end(); eit++){

						if( natural_cuts[(*eit)->get_id()] )
							continue;

						NodeID target = (*eit)->get_target();
						if( node_visited[target] )
							continue;
						stack.push_back( target );
						this->mark_node_vis( target, node_visited );
					}//for all edges
				}//for all contained node
			}//for the connected component

			this->contract_nodes( component );
		}//for all nodes

		delete[] node_visited;
		return;
}

/*void G_Graph::convert( A_Graph* ag, vector< vector<NodeID> >& id_map ){
		
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
		id_map.resize( new_id );
		NodeID s_id = 0;
		for(size_t i = 0; i < this->contract_node_list.size(); i++){

			if( this->contract_node_list[i].empty() )
				continue;
			//all internal node size is 1
			A_Node n( s_id, this->contract_node_list[i].size() );
			ag->node_list.push_back( n );
			id_map[s_id].assign( 
				this->contract_node_list[i].begin(),
				this->contract_node_list[i].end() );
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

				vector<G_Edge*>::const_iterator eit =
					this->node_list[ *cnit ].get_adj_list().begin();
				for(; eit != this->node_list[ *cnit ].get_adj_list().end(); eit++){

					t_id = this->contract_to[ (*eit)->get_target() ];
					if( t_id == i )
						continue;

					nt_id = old_to_new[t_id];
					if( nt_id < s_id )
						continue;

					if( accumulate_wet.count( nt_id ) )
						accumulate_wet[nt_id]++;
					else
						accumulate_wet[nt_id] = 1;
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

		//intial contraction
		ag->contract_to.resize( new_id );
		for( size_t i = 0; i < new_id; i++ )
			ag->contract_to[i] = i;
		ag->contract_node_list.resize( new_id );
		for( size_t i = 0; i < new_id; i++ )
			ag->contract_node_list[i].push_back( i );

		return;
}*/

void G_Graph::convert_n_output( string r_path ){
		
		A_Graph * ag;
		ag = new A_Graph;
		vector< vector<NodeID> > id_map;

		//map: old id (contracted node id) to new id
		map<NodeID, NodeID> old_to_new;
		NodeID new_id = 0;
		size_t i = 0;
		for( i = 0; i < this->contract_node_list.size(); i++){

			if( this->contract_node_list[i].empty() )
				continue;
			old_to_new[i] = new_id++;
		}

		//node
		ag->node_list.reserve( new_id );
		id_map.resize( new_id );
		NodeID s_id = 0;
		for( i = 0; i < this->contract_node_list.size(); i++){

			if( this->contract_node_list[i].empty() )
				continue;
			//all internal node size is 1
			A_Node n( s_id, this->contract_node_list[i].size() );
			ag->node_list.push_back( n );
			id_map[s_id].assign( 
				this->contract_node_list[i].begin(),
				this->contract_node_list[i].end() );
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
		for( i = 0; i < this->contract_node_list.size(); i++){

			if( this->contract_node_list[i].empty() )
				continue;
			accumulate_wet.clear();

			vector<NodeID>::const_iterator cnit = this->contract_node_list[i].begin();
			for(; cnit != this->contract_node_list[i].end(); cnit++){

				vector<G_Edge*>::const_iterator eit =
					this->node_list[ *cnit ].get_adj_list().begin();
				for(; eit != this->node_list[ *cnit ].get_adj_list().end(); eit++){

					t_id = this->contract_to[ (*eit)->get_target() ];
					if( t_id == i )
						continue;

					nt_id = old_to_new[t_id];
					if( nt_id < s_id )
						continue;

					if( accumulate_wet.count( nt_id ) )
						accumulate_wet[nt_id]++;
					else
						accumulate_wet[nt_id] = 1;
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

		//intial contraction
		ag->contract_to.resize( new_id );
		for( i = 0; i < new_id; i++ )
			ag->contract_to[i] = i;
		ag->contract_node_list.resize( new_id );
		for( i = 0; i < new_id; i++ )
			ag->contract_node_list[i].push_back( i );

		//convert done, start writing
		FILE *node_f, *edge_f;
		string node_n = "anode.txt";
		string edge_n = "aedge.txt";

		node_n.insert( node_n.begin(), r_path.begin(), r_path.end() );
		edge_n.insert( edge_n.begin(), r_path.begin(), r_path.end() );

		//write node and id map
		fopen_s( &node_f, node_n.c_str(), "w" );
		check_file( node_f, node_n.c_str() );

		fprintf_s( node_f, "%u\n", ag->node_list.size() );

		vector<A_Node>::const_iterator anit = ag->node_list.begin();
		vector< vector<NodeID> >::const_iterator idmit = id_map.begin();
		for(; anit != ag->node_list.end() && idmit != id_map.end(); anit++, idmit++){

			fprintf_s( node_f, "%u %u: ", anit->get_id(), anit->get_size() );
			vector<NodeID>::const_iterator idmnit = idmit->begin();
			for(; idmnit != idmit->end(); idmnit++){

				fprintf_s( node_f, "%u ", *idmnit );
			}
			fprintf_s( node_f, "\n" );
		}

		fflush( node_f );
		fclose( node_f );

		//write edge
		fopen_s( &edge_f, edge_n.c_str(), "w" );
		check_file( edge_f, edge_n.c_str() );

		fprintf_s( edge_f, "%u\n", ag->edge_list.size() );

		vector<A_Edge>::const_iterator aeit = ag->edge_list.begin();
		for(; aeit != ag->edge_list.end(); aeit++){

			fprintf_s( edge_f, "%u %u %u\n", aeit->get_source(),
				aeit->get_target(), aeit->get_weight() );
		}
		fflush( edge_f );
		fclose( edge_f );

		delete ag;
		return;
}

void G_Graph::cnt_one_cuts( const vector<EdgeID>& one_cut_edges, NodeSize sz_lim ){

		vector<edge_cncted_comp> component_tree;

		size_t root_pos;
		//build component tree T
		root_pos = this->build_component_tree( one_cut_edges, component_tree );

		//fill subtree size
		this->fill_subtree_size( component_tree, root_pos );

		//contract those components on the condition of first pass
		this->cnt_proper_tree_components( component_tree, root_pos, sz_lim );
		
		return;
}

size_t G_Graph::build_component_tree( const vector<EdgeID>& one_cut_edges, 
	vector<edge_cncted_comp>& component_tree ){

		set<NodeID> comp_start_node; //the node id is the contracted node id
		vector<EdgeID>::const_iterator cutit = one_cut_edges.begin();
		for(; cutit != one_cut_edges.end(); cutit++){

			if( this->contract_to[this->edge_list[*cutit].get_source()] ==
				this->contract_to[this->edge_list[*cutit].get_target()] )
				continue; //cut edge in the same contracted node

			comp_start_node.insert( this->contract_to[this->edge_list[*cutit].get_source()] );
			comp_start_node.insert( this->contract_to[this->edge_list[*cutit].get_target()] );
		}

		set<EdgeID> cut_edges( one_cut_edges.begin(), one_cut_edges.end() );

		bool * node_visited = NULL;
		node_visited = new bool[this->contract_node_list.size()];
		check_new( node_visited , "build component tree: node visited");
		memset( node_visited, false, this->contract_node_list.size() );

		size_t max_comp_pos = 0;
		size_t i = 0;
		NodeSize max_comp_size = numeric_limits<NodeSize>::min();
		set<NodeID>::const_iterator stnit = comp_start_node.begin();
		for(; stnit != comp_start_node.end(); stnit++, i++){

			NodeID start = *stnit;

			vector<NodeID> stack;
			list<NodeID> component;
			set<NodeID> children;
			stack.push_back( start ); //the contracted node id

			//DFS search to find the component
			while( !stack.empty() ){

				NodeID n = stack.back();
				stack.pop_back();

				if( node_visited[n] )
					continue;
				node_visited[n] = true;
				component.push_back( n );

				NodeID cnid = n;
				vector<NodeID>::const_iterator cnit = this->contract_node_list[cnid].begin();
				for(; cnit != this->contract_node_list[cnid].end(); cnit++){

					NodeID nid = *cnit;
					vector<G_Edge*>::const_iterator eit = 
						this->node_list[nid].get_adj_list().begin();
					for(; eit != this->node_list[nid].get_adj_list().end(); eit++){

						NodeID target = this->contract_to[(*eit)->get_target()];
						if( target == cnid )
							continue;
						if( cut_edges.count( (*eit)->get_id() ) ){
							children.insert( target );
							continue;
						}
						if( node_visited[target] )
							continue;
						stack.push_back( target );
					}//for all edges
				}//for all contained nodes
			}//while the component is expanding

			if( component.empty() )
				continue;

			edge_cncted_comp tree_node;
			tree_node.component = component;
			NodeSize comp_size = this->cal_comp_size( component );
			if( comp_size > max_comp_size ){
				max_comp_pos = i;
				max_comp_size = comp_size;
			}
			tree_node.subtree_size = comp_size; //subtree includes the root
			//borrow the children space
			tree_node.children.assign( children.begin(), children.end() );

			component_tree.push_back( tree_node );

		}

		//now we have all the tree nodes and their sizes, then we build the tree
		map<NodeID, size_t> comp_cnodes_to_pos;
		for( i = 0; i < component_tree.size(); i++ ){

			list<NodeID>::const_iterator cnit = component_tree[i].component.begin();
			for(; cnit != component_tree[i].component.end(); cnit++)
				comp_cnodes_to_pos[*cnit] = i;
		}

		//recursively link the tree
		this->link_component( component_tree ,comp_cnodes_to_pos, max_comp_pos, -1u );

		return max_comp_pos;
}

NodeSize G_Graph::fill_subtree_size( vector<edge_cncted_comp>& component_tree, size_t root_p ){
		
		if( component_tree[root_p].children.empty() )
			return component_tree[root_p].subtree_size; //leaf node size

		list<NodeID>::const_iterator chlit = component_tree[root_p].children.begin();
		for(; chlit != component_tree[root_p].children.end(); chlit++){

			component_tree[root_p].subtree_size += 
				this->fill_subtree_size( component_tree, *chlit );
		}
		return component_tree[root_p].subtree_size;
}

void G_Graph::cnt_proper_tree_components( vector<edge_cncted_comp>& component_tree, size_t root_p,
	NodeSize sz_lim ){

		//just in case
		if( !component_tree[root_p].subtree_size ) //component_tree[root_p].subtree_size is 0
			return;

		if( component_tree[root_p].subtree_size <= sz_lim ){
			
			NodeID new_node_id;
			new_node_id = this->contract_subtree( component_tree, root_p );
			component_tree[root_p].component.clear();
			component_tree[root_p].component.push_back( new_node_id );
			component_tree[root_p].children.clear();
			//shrink further
			if( component_tree[root_p].subtree_size <= 5 ){

				if( component_tree[root_p].subtree_size + 
					this->contract_node_list[component_tree[root_p].neighbor_id_in_parent].size()
					<= sz_lim ){

						NodeID new_merge_id = this->contract_child_to_parent( new_node_id,
							component_tree[root_p].neighbor_id_in_parent );

						/* actually I think there is no need to mantain the tree
						/* for we just use it for once and no more operations on it
						 */
						//size_t parent = component_tree[root_p].parent;
						//component_tree[parent].children.remove( root_p );

						component_tree[root_p].neighbor_id_in_parent = new_merge_id;
						component_tree[root_p].subtree_size = 0;
				}
			}
			return;
		}

		list<NodeID>::const_iterator chlit = component_tree[root_p].children.begin();
		while( chlit != component_tree[root_p].children.end() ){

			NodeID next_chl = *chlit;
			chlit++;	//to deal with iterator being invalidated
			this->cnt_proper_tree_components( component_tree, next_chl, sz_lim );
		}
		return;
}

void G_Graph::link_component( vector<edge_cncted_comp>& component_tree, map<NodeID, size_t>&
	comp_cnodes_to_pos, size_t search_pos, size_t parent_pos ){

		if( parent_pos != -1u )
			component_tree[search_pos].parent = parent_pos;

		vector<size_t> children_pos;
		list<NodeID>::const_iterator chlit = component_tree[search_pos].children.begin();
		for(; chlit != component_tree[search_pos].children.end(); chlit++){

			if( comp_cnodes_to_pos[(*chlit)] == parent_pos ){
				component_tree[search_pos].neighbor_id_in_parent = *chlit;
			}
			else{
				children_pos.push_back( comp_cnodes_to_pos[(*chlit)] );
			}
		}
		component_tree[search_pos].children.assign( children_pos.begin(), children_pos.end() );
		//recursively link
		chlit = component_tree[search_pos].children.begin();
		for(; chlit != component_tree[search_pos].children.end(); chlit++){

			size_t new_search_pos = *chlit;
			this->link_component( component_tree, comp_cnodes_to_pos, new_search_pos, search_pos );
		}
		return;
}

NodeID G_Graph::contract_subtree( vector<edge_cncted_comp>& component_tree, size_t root_pos ){
		
		list<NodeID> all_comp;
		this->find_all_comp( component_tree, root_pos, all_comp );
		return this->contract_cnodes( all_comp );
}

void G_Graph::find_all_comp( vector<edge_cncted_comp>& component_tree, size_t root_pos, 
	list<NodeID>& all_comp ){

		all_comp.insert( all_comp.end(), component_tree[root_pos].component.begin(),
			component_tree[root_pos].component.end() );

		list<NodeID>::const_iterator chlit = component_tree[root_pos].children.begin();
		for(; chlit != component_tree[root_pos].children.end(); chlit++){

			size_t new_root_pos = *chlit;
			this->find_all_comp( component_tree, new_root_pos, all_comp );
		}
		return;

}

NodeID G_Graph::contract_child_to_parent( NodeID child, NodeID parent ){
		
		NodeID new_node_id = parent;

		NodeID cid = child;
		vector<NodeID>::const_iterator cnit = this->contract_node_list[cid].begin();
		for(; cnit != this->contract_node_list[cid].end(); cnit++ ){

			contract_to[*cnit] = new_node_id;
			this->contract_node_list[new_node_id].push_back( *cnit );
		}

		//delete this contracted node
		this->contract_node_list[cid].clear();
		del_cnt_node.push_back( cid );

		return new_node_id;

}