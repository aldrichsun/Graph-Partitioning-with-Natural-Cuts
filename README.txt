/*****************************************
* Graph Partition Program
*
* This software implements the graph partition algorithm in
* "Graph Partitioning with Natural Cuts, Daniel Delling etc., IEEE International Parallel & Distributed Processing Symposium, 2011".
* 
* Author: Yu Sun, Nanyang Technological University
* Last update date: 18th Jan. 2013
******************************************/

Use the following three steps to run the software:
----------------------------
1. '%D:\SPV\GraphPartition\BIN>'Preprocess.exe ..\data\USA-road-d.NY.gr ..\data\USA-road-d.NY.new.gr

2. '%D:\SPV\GraphPartition\BIN>'Filter.exe 4096 ..\data\USA-road-d.NY.co ..\data\USA-road-d.NY.new.gr ..\result\NY\

3. '%D:\SPV\GraphPartition\BIN>'Assemble.exe 4096 ..\data\USA-road-d.NY.co ..\data\USA-road-d.NY.new.gr ..\result\NY\anode.txt ..\result\NY\aedge.txt ..\result\NY\
----------------------------

The result is written in two files in the given output folder (..\result\NY\ in the above example): 'cut_edges.txt' and 'node_clusters.txt'. The format of the result file is self-explanatory.

The format of the input file follows that of the '9th DIMACS Implementation Challenge - Shortest Paths', and the datasets can be downloaded from http://www.dis.uniroma1.it/challenge9/download.shtml. Some sample files are in the \data folder. Note that the \result\NY\ folder needs to be created manually.

=========================================================
Details:

The first step is to remove duplicated edges, which is necessary.
The parameters for Preprocess.exe is
----------------
<arg1> input file name (including path)
<arg2> output file name
----------------

The second step is the Filter phase, and its parameters (in this order) are
-----------------------
<arg1> U node size limit
<arg2> node file path, e.g. C:/GraphPartition/data/node.txt
<arg3> edge file path, e.g. C:/GraphPartition/data/edge.txt
<arg4> result file directory, e.g. C:/GraphPartition/data/result/
<arg5> (optional) C detect natural cuts times, default 1
<arg6> (optional) f sets the core size relative to the ring, default 10
-----------------------

The third step is the Assemble phase. Its parameters are
-----------------------
<arg1> U node size limit
<arg2> original node file path, e.g. C:/GraphPartition/data/node.txt
<arg3> original edge file path, e.g. C:/GraphPartition/data/edge.txt
<arg4> assemble node file path, e.g. C:/GraphPartition/data/anode.txt
<arg5> assemble edge file path, e.g. C:/GraphPartition/data/aedge.txt
<arg6> result file directory, e.g. C:/GraphPartition/data/result/
<arg7> (optional) FI each pair failure time, default 16
<arg8> (optional) M multistart trying time,default 1
<arg9> (optional) use combination Y/N, default N
-----------------------

To visualize the result, use the showResult.html in the showResult folder.