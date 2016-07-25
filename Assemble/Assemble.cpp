#include <iostream>
#include <math.h>

using namespace std;

#include "A_Graph.h"
#include "R_Graph.h"

void greedy_algo( A_Graph& a_graph, NodeSize sz_lim ){

	VERBOSE(printf("first greedy algorithm...");)
	//a_graph.greedy_algorithm( sz_lim );
	a_graph.greedy_algorithm_heap( sz_lim );
	VERBOSE(printf("done!\n");)
}

void local_search( A_Graph& a_graph ,vector< vector<NodeID> >& result, NodeSize sz_lim){

	VERBOSE(printf("local search (it may take a long time)...");)
	a_graph.local_search( result, sz_lim );
	VERBOSE(printf("done!\n");)
}

void read_a_graph( A_Graph& a_graph, vector< vector<NodeID> >& id_map, string co_path, string gr_path ){

	VERBOSE(printf("read in assemble graph...");)
	a_graph.read_graph_n_idmap( id_map, co_path, gr_path );
	VERBOSE(printf("done!\n");)
	VERBOSE( printf("Asssemble graph has:\n%u nodes and %u edges\n", a_graph.get_node_list().size(), a_graph.get_edge_list().size()); )
}

void multistart_and_combination( A_Graph& a_graph, vector< vector<NodeID> >& result, NodeSize sz_lim ){

	VERBOSE(printf("multi-start and combination (it may take a long time)...\n");)
	a_graph.multistart_and_combination( result, sz_lim );
	VERBOSE(printf("done!\n");)
}

void write_result( R_Graph& g, vector< vector<NodeID> >& result, vector< vector<NodeID> >& id_map, 
	string co_path, string gr_path, string& r_path ){

	VERBOSE(printf("write result into target directory...");)
	EdgeWeight weight = 0;
	weight = g.write_result( result, id_map, co_path, gr_path, r_path );
	VERBOSE(printf("done!\n");)
	VERBOSE(printf("total weight of the cut edges is: %u\n", weight);)
}

int main( int argc, char** argv){

	srand((unsigned int)time(NULL)); //initial here, since two seeds can be within 1'

	if( argc != 7 && argc != 10 ){
		printf("usage:\n<arg1> U node size limit\n");
		printf("<arg2> original node file path, e.g. C:/GraphPatition/data/node.txt\n");
		printf("<arg3> original edge file path, e.g. C:/GraphPatition/data/edge.txt\n");
		printf("<arg4> assemble node file path, e.g. C:/GraphPatition/data/a_node.txt\n");
		printf("<arg5> assemble edge file path, e.g. C:/GraphPatition/data/a_edge.txt\n");
		printf("<arg6> result file directory, e.g. C:/GraphPatition/data/result/\n");
		printf("<arg7> (optional) FI each pair failure time, default 16\n");
		printf("<arg8> (optional) M multistart trying time,default 1\n");
		printf("<arg9> (optional) use combination Y/N, default N\n");
		exit(0);
	}

	NodeSize sz_lim = atoi( argv[1] );
	string co_path( argv[2] );
	string gr_path( argv[3] );
	string a_co_path( argv[4] );
	string a_gr_path( argv[5] );
	string result_path = ( argv[6] );

	//NodeSize sz_lim = 4;
	////NodeSize sz_lim = 1;
	//string co_path = "Y:/paper/NTUADSC/SPV/experiment/Exp/GraphPatition/data/sample.cnode";
	//string gr_path = "Y:/paper/NTUADSC/SPV/experiment/Exp/GraphPatition/data/sample.cedge";
	//string a_co_path = "Y:/paper/NTUADSC/SPV/experiment/Exp/GraphPatition/result/sample/anode.txt";
	//string a_gr_path = "Y:/paper/NTUADSC/SPV/experiment/Exp/GraphPatition/result/sample/aedge.txt";
	//string result_path = "Y:/paper/NTUADSC/SPV/experiment/Exp/GraphPatition/result/sample/";
	//VERBOSE( printf("Sample: \n"); )

	//NodeSize sz_lim = 4096;
	//string co_path = "Y:/paper/NTUADSC/SPV/experiment/Exp/GraphPatition/result/NY/anode.txt";
	//string gr_path = "Y:/paper/NTUADSC/SPV/experiment/Exp/GraphPatition/result/NY/aedge.txt";
	//string result_path = "Y:/paper/NTUADSC/SPV/experiment/Exp/GraphPatition/result/NY/";
	//VERBOSE( printf("NY: \n"); )

	//NodeSize sz_lim = 16384;
	//string co_path = "Y:/paper/NTUADSC/SPV/experiment/Exp/GraphPatition/data/USA-road-d.FLA.cnode";
	//string gr_path = "Y:/paper/NTUADSC/SPV/experiment/Exp/GraphPatition/data/USA-road-d.FLA.cedge";
	//string result_path = "Y:/paper/NTUADSC/SPV/experiment/Exp/GraphPatition/result/FLA/";
	//VERBOSE( printf("FLA: \n"); )

	//NodeSize sz_lim = 1048576;
	//string co_path = "Y:/paper/NTUADSC/SPV/experiment/Exp/GraphPatition/data/USA-road-d.USA.cnode";
	//string gr_path = "Y:/paper/NTUADSC/SPV/experiment/Exp/GraphPatition/data/USA-road-d.USA.cedge";
	//string result_path = "Y:/paper/NTUADSC/SPV/experiment/Exp/GraphPatition/result/USA/";
	//VERBOSE( printf("USA: \n"); )

	VERBOSE( printf("Node size limit: %d\n", sz_lim); )
	
	vector< vector<NodeID> > result;
	vector< vector<NodeID> > id_map;
	R_Graph r_g;
	{
		A_Graph a_g;

		if( argc == 10 ){
			a_g.FI = atoi( argv[7] );
			a_g.M = atoi( argv[8] );
			if( !strcmp(argv[9], "Y") )
				a_g.use_combine = true;
		}

		read_a_graph( a_g, id_map, a_co_path, a_gr_path );
		multistart_and_combination( a_g, result, sz_lim );
	}
	write_result( r_g, result, id_map, co_path, gr_path, result_path );

	return 0;
}