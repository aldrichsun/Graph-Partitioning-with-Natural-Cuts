#include<iostream>
#include<fstream>
#include<map>

using namespace std;

int main(int argc, char** argv){

	if( argc!=3 ){
		printf("usage: <arg1> input file name <arg2> output file name\n");
		exit(0);
	}

	/*ifstream originalFile(argv[1]);
	if( !originalFile.is_open() ){
		printf("can not open input file");
		exit(0);
	}
	ofstream newFile(argv[2]);
	if( !newFile.is_open() ){
		printf("can not open output file");
		exit(0);
	}*/

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

	map< pair<int, int>, int > edge_map;
	char c;
	char buff[200];
	int s, t, w, v, d, a;

	d = 0;
	a = 0;
	//while( !originalFile.eof() ){
	while( !feof(fin) ){
		
		//c = originalFile.peek();
		c = fgetc(fin);
		if( c != 'a' ){
			//originalFile.getline(buff,200);
			fgets(buff,200,fin);
			continue;
		}

		//originalFile >> c >> s >> t >> w;
		fscanf_s(fin, "%d %d %d\n", &s, &t, &w);
		a++;

		pair<int,int> p(s,t);

		if( edge_map.count( p ) ){
			d++;
			v = edge_map[p];
			if( v > w )
				edge_map[p] = w;
		}else
			edge_map[p] = w;
	}

	map< pair<int, int>, int >::const_iterator iter = edge_map.begin();

	fprintf_s(fout, "%d\n", edge_map.size());
	fprintf_s(fout, "Original Edges: %d. Duplicated edges: %d\n", a, d);
	while( iter != edge_map.end() ){

		//newFile << 'a' << iter->first.first << ' ' << iter->first.second
		//	<< ' ' << iter->second << endl;
		fprintf_s(fout, "a %d %d %d\n", iter->first.first, iter->first.second, iter->second);

		iter++;
	}

	printf("New file has been made\n");

	return 0;
}