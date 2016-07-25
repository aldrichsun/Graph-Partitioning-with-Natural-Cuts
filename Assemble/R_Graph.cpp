#include "R_Graph.h"

void R_Graph::read_graph(string co_path, string gr_path){

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
			R_Node tnode(tlt/REALLATLNG, tlg/REALLATLNG);
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
			R_Edge e(ts-1, tt-1);
			this->edge_list.push_back(e);
		}
		fclose( gr_f );
}

void R_Graph::fill_contract_to( vector< vector<NodeID> >& final_result ){

		this->contract_to.resize( this->node_list.size(), 0 );
		vector< vector<NodeID> >::const_iterator cit = final_result.begin();
		for(size_t i = 0; cit != final_result.end(); cit++, i++){

			vector<NodeID>::const_iterator nit = cit->begin();
			for(; nit != cit->end(); nit++){
				this->contract_to[ *nit ] = i;
			}
		}
		return;
}

EdgeWeight R_Graph::write_result( vector< vector<NodeID> >& result, vector< vector<NodeID> >& id_map, 
	string& co_path, string& gr_path, string& r_path ){

		vector< vector<NodeID> > final_result( result.size() );
		vector< vector<NodeID> >::const_iterator cit = result.begin();
		for(size_t i = 0; cit != result.end(); cit++, i++){

			vector< NodeID >::const_iterator nit = cit->begin();
			final_result[i].reserve( 10 * cit->size() );
			for(; nit != cit->end(); nit++){

				final_result[i].insert(final_result[i].end(), id_map[*nit].begin(), id_map[*nit].end() );
			}
		}

		this->read_graph( co_path, gr_path );
		this->fill_contract_to( final_result );

		string node_clusters_p = "node_clusters.txt";
		string cut_edges_p = "cut_edges.txt";

		node_clusters_p.insert( node_clusters_p.begin(), r_path.begin(), r_path.end() );
		cut_edges_p.insert( cut_edges_p.begin(), r_path.begin(), r_path.end() );

		FILE *fpcl, *fpct;

		fopen_s( &fpcl, node_clusters_p.c_str(), "w" );
		check_file( fpcl, "write result: node_clusters_p" );

		fprintf_s( fpcl, "there are total %u partitions\nthey are:\n", final_result.size());
		cit = final_result.begin();
		for(size_t i = 0; cit != final_result.end(); cit++, i++){

			fprintf_s( fpcl, "============================\n" );
			fprintf_s( fpcl, "partition %u:\n", i);
			vector< NodeID >::const_iterator nit = cit->begin();
			for(; nit != cit->end(); nit++){
				fprintf_s( fpcl, "%u %lf, %lf\n", (*nit)+1,
					this->node_list[*nit].get_lat(),
					this->node_list[*nit].get_lng() ); 
			}
		}
		fprintf_s( fpcl, "============================\n" );
		fflush( fpcl );
		fclose( fpcl );

		fopen_s( &fpct, cut_edges_p.c_str(), "w" );
		check_file( fpct, "write result: cut_edges_p" );
		size_t t_w = 0;
		vector<R_Edge>::const_iterator eit = this->edge_list.begin();
		for(; eit != this->edge_list.end(); eit++){

			NodeID s = eit->get_source();
			NodeID t = eit->get_target();
			if( this->contract_to[s] !=	this->contract_to[t] ){
				t_w++;
				fprintf_s( fpct, "%lf %lf %lf %lf\n",
					this->node_list[s].get_lat(),
					this->node_list[s].get_lng(),
					this->node_list[t].get_lat(),
					this->node_list[t].get_lng() );
			}
		}
		return t_w>>1;
}