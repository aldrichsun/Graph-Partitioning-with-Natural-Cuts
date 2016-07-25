#include <iostream>
#include <math.h>

using namespace std;

#include "G_Graph.h"

//every two related edges should be updated at the same time
void read_in_graph( G_Graph &graph, string co_path, string gr_path ){

	//read in graph
	VERBOSE(printf("Reading input file...");)
	graph.read_graph(co_path, gr_path);
	VERBOSE(printf("done.\n");)
}


void contract_tiny_cuts( G_Graph &graph, NodeSize sz_lim ){

	//first pass: first ignored, then added behind for convenience
	
	//third pass: process 2-cuts classes elaborate
	//make a dfs tree on the graph

	VERBOSE(printf("get DFS tree...");)
	vector<bool> edge_in_fi( graph.get_edge_list().size(), true );
	graph.dfs_tree( 0, edge_in_fi, 0);
	VERBOSE(
		size_t ten = 0;
		for( int i = 0; i < edge_in_fi.size(); i++ ){
			if( edge_in_fi[i] == false )
				ten++;
		}
		printf("done!\nthere are %d edge (%.1f%%) in tree.\n", ten, ten*100.0/edge_in_fi.size());
	)

	VERBOSE(printf("find edge classes...");)
	vector< vector<EdgeID> > edge_equl_cls; 
	graph.two_cuts_edge_class( edge_in_fi, edge_equl_cls ); //edge_equl_cls[0] is the class of 1-cut
	VERBOSE(printf("done!\n");)

	VERBOSE(printf("contract two cuts...");)
	//handle each class, cut graph to components and contract small ones
	graph.cnt_two_cuts( edge_equl_cls, sz_lim );
	VERBOSE(
		printf("done!\ncontract %d nodes (%.1f%%)\n", graph.get_del_node().size(),
		graph.get_del_node().size()*100.0/graph.get_node_list().size());
	)

	/* we assume that 1-cut has nothing to do with 2-cut, so the first can be added
	/* after the third and second pass. Here we use the algorith same as dectecting
	/* 2-cuts to find all 1-cuts (bridges). During the above contraction process some
	/* of them are contained in the same contracted node, and they are thus ingored. */

	//edge_equl_cls[0] is the class of 1-cut
	VERBOSE(printf("contract one cuts...");)
	graph.cnt_one_cuts( edge_equl_cls[0], sz_lim );
	VERBOSE(
		printf("done!\ncontract %d nodes (%.1f%%)\n", graph.get_del_node().size(),
		graph.get_del_node().size()*100.0/graph.get_node_list().size());
	)

	VERBOSE(printf("contract two-degree-node paths...");)
	//second pass: contract 2-degree-node paths
	graph.cnt_two_degree_path( sz_lim );
	VERBOSE(
		printf("done!\ncontract %d nodes (%.1f%%)\n", graph.get_del_node().size(),
		graph.get_del_node().size()*100.0/graph.get_node_list().size());
	)

	return;
}

void contract_natural_cuts( G_Graph &graph, NodeSize sz_lim ){

	VERBOSE(printf("find all natural cuts...");)
	//vector<bool> natural_cuts( graph.get_edge_list().size(), false );
	bool * natural_cuts = NULL;
	natural_cuts = new bool[ graph.get_edge_list().size() ];
	if( !natural_cuts ){ printf("ERROR bad alloc: natural_cuts!\n"); exit(0); }
	memset( natural_cuts, false, graph.get_edge_list().size() );

	graph.find_natural_cuts( natural_cuts, sz_lim );

	VERBOSE(
		size_t ten = 0;
		for( int i = 0; i < graph.get_edge_list().size(); i++ ){
			if( natural_cuts[i] == true )
				ten++;
		}
		printf("done!\nthere are %d edges (%.1f%%) are natural cuts.\n",
			ten, ten*100.0/graph.get_edge_list().size());
	)

	VERBOSE(printf("contract natural cuts...");)
	graph.cnt_natural_cuts( natural_cuts );
	VERBOSE(
		printf("done!\ncontract %d nodes (%.1f%%)\n", graph.get_del_node().size(),
		graph.get_del_node().size()*100.0/graph.get_node_list().size());
	)

	delete[] natural_cuts;

	return;
}

void convert_and_output( G_Graph &graph, string r_path ){

	VERBOSE(printf("convert to assemble graph and output...");)
	graph.convert_n_output( r_path );
	VERBOSE(printf("done!\n");)

}

int main( int argc, char** argv){

	//srand(40);
	srand((unsigned int)time(NULL)); //initial here, since two seeds can be within 1'

	if( argc != 5 && argc != 7 ){
		printf("usage:\n<arg1> U node size limit\n");
		printf("<arg2> node file path, e.g. C:/GraphPatition/data/node.txt\n");
		printf("<arg3> edge file path, e.g. C:/GraphPatition/data/edge.txt\n");
		printf("<arg4> result file directory, e.g. C:/GraphPatition/data/result/\n");
		printf("<arg5> (optional) C detect natural cuts times, default 1\n");
		printf("<arg6> (optional) f sets the core size relative to the ring, default 10\n");
		exit(0);
	}

	NodeSize sz_lim = atoi( argv[1] );
	string co_path( argv[2] );
	string gr_path( argv[3] );
	string result_path = ( argv[4] );

	//NodeSize sz_lim = 4;
	//string co_path = "Y:/paper/NTUADSC/SPV/experiment/Exp/GraphPatition/data/sample.cnode";
	//string gr_path = "Y:/paper/NTUADSC/SPV/experiment/Exp/GraphPatition/data/sample.cedge";
	//string result_path = "Y:/paper/NTUADSC/SPV/experiment/Exp/GraphPatition/result/sample/";
	//VERBOSE( printf("Sample: \n"); )

	//NodeSize sz_lim = 4096;
	//string co_path = "Y:/paper/NTUADSC/SPV/experiment/Exp/GraphPatition/data/USA-road-d.NY.cnode";
	//string gr_path = "Y:/paper/NTUADSC/SPV/experiment/Exp/GraphPatition/data/USA-road-d.NY.cedge";
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
	
	G_Graph g;

	if( argc == 7 ){
		g.DNCC = atoi( argv[5] );
		g.DNCF = atoi( argv[6] );
	}

	//g.DNCF = 2;

	read_in_graph( g, co_path, gr_path );
	contract_tiny_cuts( g, sz_lim );
	contract_natural_cuts( g, sz_lim );
	convert_and_output( g, result_path );

	return 0;
}