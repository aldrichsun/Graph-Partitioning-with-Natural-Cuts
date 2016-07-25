#include<iostream>
#include<fstream>
#include<map>
#include<set>

using namespace std;

typedef pair<double, double> point;

int main(int argc, char** argv){

	if( argc!=3 ){
		printf("usage: <arg1> input file name <arg2> output file name\n");
		exit(0);
	}
	//argv[1] = "Y:\\paper\\NTUADSC\\SPV\\experiment\\Exp\\GraphPatition\\showResult\\USA\\cut_edges.txt";
	//argv[2] = "Y:\\paper\\NTUADSC\\SPV\\experiment\\Exp\\GraphPatition\\showResult\\USA\\new_cut_edges.txt";

	FILE* fin;
	FILE* fout;
	fopen_s(&fin, argv[1], "r");
	if( fin == NULL ){
		printf("can not open input file\n");
		exit(0);
	}
	fopen_s(&fout, argv[2], "w");
	if( fout == NULL ){
		printf("can not open output file\n");
		exit(0);
	}

	map< point, set<point> > edge_set;

	//while( !originalFile.eof() ){
	while( !feof(fin) ){

		point p1, p2;
		//originalFile >> c >> s >> t >> w;
		fscanf_s(fin, "%lf %lf %lf %lf\n", &p1.first, &p1.second,
			&p2.first, &p2.second);
		if( edge_set.count( p1 ) ){
			edge_set[p1].insert( p2 );
		}
		else if( edge_set.count( p2 ) ){
			edge_set[p2].insert( p1 );
		}
		else{
			edge_set[p1].insert( p2 );
		}
	}

	map< point, set<point> >::const_iterator iter = edge_set.begin();
	while( iter != edge_set.end() ){

		set<point>::const_iterator piter = iter->second.begin();
		while( piter != iter->second.end() ){

			fprintf_s(fout, "%lf %lf %lf %lf\n", 
				iter->first.first, iter->first.second,
				piter->first, piter->second);
			piter++;
		}
		iter++;
	}
	printf("New file has been made\n");

	return 0;
}