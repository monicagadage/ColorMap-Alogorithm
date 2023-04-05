#include <iostream>
#include <list>
using namespace std;
#include <vector>
#include<map>
#include<stack>
#include <sstream>
#include <fstream>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/grid_graph.hpp>
#include <regex>
#include <gtk/gtk.h>

vector< vector<int> > pos;
vector< vector<int> > aL;
vector<int> nColor;

bool drawCallback(GtkWidget *widget, cairo_t *cr, void* data);

void drawGraph(int argc, char** argv)
{ 
	GtkWidget *window, *drawing_area;
	gtk_init(&argc, &argv);
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

	drawing_area = gtk_drawing_area_new();
	gtk_container_add(GTK_CONTAINER(window), drawing_area);
	gtk_widget_set_size_request (drawing_area, 480, 480);
	g_signal_connect (G_OBJECT (drawing_area), "draw", G_CALLBACK(drawCallback),NULL);
	gtk_widget_show_all(window);
	gtk_main();
}

bool drawCallback( GtkWidget *widget, cairo_t *cr, void* data)
{
	int width, height;
	GdkRGBA color;
	GtkStyleContext *context;
	vector<vector<int>> coords = pos;
	context = gtk_widget_get_style_context(widget);
	width = gtk_widget_get_allocated_width (widget);
	height = gtk_widget_get_allocated_height(widget);
	gtk_render_background(context, cr, 0, 0, width, height);

	for (int i = 0; i < (int) aL.size(); i++)
	{
		for (int j = 0; j < (int) aL[i].size(); j++)
		{
			cairo_move_to(cr, coords[i][0]+20, coords[i][1]+40);
			cairo_line_to(cr, coords[aL[i][j]][0]+20, coords[aL[i][j]][1]+40);

			cairo_stroke(cr);

		}
		cairo_stroke(cr);
	}

	for(int i = 0; i < (int) coords.size(); ++i)
	{
		string l = to_string(i);
		const char* lb = l.c_str();
		GtkWidget *label = gtk_label_new(lb);
		gtk_label_set_markup (GTK_LABEL (label), "<small>Small text</small>");
		cairo_arc (cr, coords[i][0]+20,coords[i][1] + 40, MIN(width,height)/20.0,0,4 * G_PI);
		gtk_style_context_get_color(context, gtk_style_context_get_state(context), &color);
		if (nColor[i] == 1)
			color.red = 1;

		else if (nColor[i] == 2)
			color.blue = 1;

		else if (nColor[i] == 3)
			color.green = 1;

		else if (nColor[i] == 4)
		{
			color.red = 0.5;
			color.blue = 0.5;
		}
		else if (nColor[i] == 0)
		{
			color.blue = 0.5;
			color.green = 0.5;
		}

		gdk_cairo_set_source_rgba (cr, &color);
		gdk_cairo_set_source_rgba (cr, &color);
		cairo_fill (cr);
	}
	return FALSE;
}

void visual(map<int, vector<int>> edges, int argc, char** argv)
{
	boost::adjacency_list<> graph(edges.size()-1);
	vector<int> vertex_coor;
	vector<int> node_order;
	vector<int>::iterator it;
	int padding = 20;
	int scale = 100;
	int s = 0;	
	std::ofstream gout;
	gout.open("test.dot");
	boost::add_vertex(graph);

	for (int i = 0; i < (int) edges.size(); i++)
	{
		vector<int> n = edges.at(i);
		s += n.size();
		it = find(node_order.begin(), node_order.end(), i);
		if(it == node_order.end())
		{
			node_order.push_back(i);
		}
		for (int j = 0; j < (int) n.size(); j++)
		{
			it = find(node_order.begin(), node_order.end(), n[j]);
			if(it == node_order.end())
			{
				node_order.push_back(n[j]);
			}
			boost::add_edge(i, n[j], graph);
		}

	}

	write_graphviz(gout, graph);

	gout.close();

	system("neato -Tpng test.dot > dot.png");
	system("neato -Tdot test.dot > dot.txt");

	regex reg("\"\\d*\\.?\\d*,\\d*\\.?\\d*\"");
	ifstream glay;
	string line;
	std::smatch m;
	pos.resize(edges.size());

	glay.open("dot.txt");
	int i = 0;
	while (std::getline(glay, line))
	{
		if (regex_search(line, m, reg))
		{
			line = m.str();
			pos[node_order[i]].push_back(stoi(line.substr(1, line.find(",") - 1)));
			line.erase(line.end() - 1, line.end());
			pos[node_order[i]].push_back(stoi(line.substr(line.find(",") + 1, (int)line.length())));
			i++;
		}

	}
	glay.close();

	drawGraph(argc, argv);
}

/* Node class to hold the Node parameters*/
class Node{
	int number; 
	int node_color;
	vector <string> color; //total number of color 
	vector<int> adj_list; // adjacent list of the node
	bool visited;

	public:


	//set various parameters of the Node
	void add_details(int n , vector<int> edges , vector <string> col_s, int node_colo = -1){
		number = n;
		adj_list = edges;
		color = col_s;
		node_color = node_colo;
		visited = false;
	}

	//Getter Setter 
	int getNumber(){
		return number;
	}
	vector<int>  getAdj(){
		return adj_list;
	}

	vector<string>  getColor(){
		return color;
	}

	void setColor(vector <string> colo){
		color = colo;
	}
	void setNodeColor( int colo){
		node_color = colo;
	}

	int getNodeColor(){
		return node_color;
	}
	bool getVisited(){
		return visited;
	}

	void setVisited( bool visit){
		visited = visit;
	}


};


//Recursive function to find the optimal solution for graph coloring
bool hamCycle(Node& node , int number_nodes, vector<Node>& myInstant , stack<int> order);

//check if the node is safe to use
bool isSafe(Node node , string color, vector<Node> myInstant)
{
	vector <int> adj =  node.getAdj();
	bool safe = true;
	for (int i = 0; i < adj.size(); i++){
		//if only one color left then return false
		if (myInstant[adj[i]].getColor().size()==1){
			if(myInstant[adj[i]].getColor()[0] == color){
				return false;
			}
		}
	}
	return true;
}

//delete the color from adjacent nodes color list
void deleteColor_adj(Node& adj, string color , vector<Node>& myInstant ) {

	for (int i = 0 ; i < adj.getAdj().size(); i ++){
		if(!myInstant[adj.getAdj()[i]].getVisited()){
			vector<string> c ;
			for (int j = 0 ; j <myInstant[adj.getAdj()[i]].getColor().size(); j ++){

				if (myInstant[adj.getAdj()[i]].getColor()[j] != color){
					c.push_back(myInstant[adj.getAdj()[i]].getColor()[j]);
				}

			}
			myInstant[adj.getAdj()[i]].setColor(c);
		}
	}


}


//util function for recursive call to check the various condition on the Nodes
bool hamCycleUtil( Node& node , int number_nodes , vector<Node>& myInstant, stack<int> order)
{
	if (node.getNumber() == number_nodes){
		return true;
	}
	for (int i = 0; i < node.getColor().size() ; i ++){

		if(isSafe(node , node.getColor().at(i) , myInstant)){

			node.setNodeColor(std::stol (node.getColor().at(i)));
			deleteColor_adj(node,node.getColor().at(i) , myInstant);
			if(node.getNumber() == number_nodes-1){
				return true;
			}
			else{
				hamCycle(myInstant[node.getNumber()+1],number_nodes,myInstant,order);
			}
			return true;
		}
		else if( (i == node.getColor().size()-1) && isSafe(node , node.getColor().at(i) , myInstant) == false){
			return false;
		}

	}
	return false;
}



//erase the color from the node because it's wrong color choice
void eraseColor(string color , Node& node){
	auto it = find(node.getColor().begin(),node.getColor().end(), color);
	bool test = it!=node.getColor().end();
	vector <string> c ;
	for(int i = 0; i < node.getColor().size(); i++){
		if (node.getColor()[i] != color){
			c.push_back(node.getColor()[i]);
		}
	}
	node.setColor(c);

} 

//reset the changes for the node adjacent list because of wrong color choice
void resetChanges(Node& node , vector<Node>& myInstant){
	vector<int> adj = node.getAdj();
	for ( int i = 0; i < adj.size(); i++){
		if(!myInstant[adj[i]].getVisited()){
			vector<string> col = myInstant[adj[i]].getColor();
			col.push_back(std::to_string(node.getNodeColor()));
			myInstant[adj[i]].setColor(col);
		}
	}

}

//Recursive function to find the optimal solution for graph coloring
bool hamCycle(Node& node , int number_nodes, vector<Node>& myInstant , stack<int> order)
{
	node.setVisited(true);
	//order to prcoess the node
	order.push(node.getNumber());
	if (hamCycleUtil(node , number_nodes , myInstant , order) == false )
	{
		cout << "In backtrack for " << node.getNumber()<<endl;
		if(!order.empty()){
			int number = node.getNumber();
			while(myInstant[number].getColor().size()==1 && myInstant[number].getVisited() && !order.empty()){
				myInstant[number].setVisited(false);
				order.pop();
				if(!order.empty())
					number = order.top();
				else 
					return false;
			}
			resetChanges(myInstant[number], myInstant);
			eraseColor(std::to_string(myInstant[number].getNodeColor()) , myInstant[number]);
			if (hamCycle(myInstant[number],number_nodes,myInstant,order) == false){
				return false;
			}
		}
		else
			return false;
	}
	return true;
}


//color map function which calls the recursive function for first time
void colorMap( int number_nodes, vector<Node>& myInstant , stack<int> order){
	bool sol = true;

	for (int i = 0 ; i < number_nodes; i ++){

		if(!myInstant[i].getVisited()){
			sol =  hamCycle(myInstant[i],number_nodes, myInstant, order);
		}
	}

	cout << "solution " << endl;
	if(sol == true){
		for (int i = 0 ; i < number_nodes; i ++){
			cout << " Node: " <<myInstant[i].getNumber() <<  " has color : " <<myInstant[i].getNodeColor() << endl;
			//adding the nodes for gui
			nColor.push_back(myInstant[i].getNodeColor());
		}
	}
	else
		cout << "No solution";

}

//main function
int main(int argv, char* argc[])
{
	//check the file argument
	if(argv < 2)
	{
		cerr << "usage: " << argc[0] << " [input_file]" << endl;
		exit(1);
	}

	char* input_file = argc[1];

	ifstream inf;
	inf.open(input_file);
	if(!inf.good())
	{
		cerr << "Error opening input file" << endl;
		exit(1);
	}

	vector<string> colors;
	string line;

	getline(inf,line);
	istringstream ist(line);
	string number_nodes1;
	int number_nodes;
	while(ist >> number_nodes1)
	{
		number_nodes = std::stoi(number_nodes1);
		cout << "Number of Nodes: " <<number_nodes << endl;
	} 
	ist.clear();

	string number_edges1;
	int number_edges;
	getline(inf,line);
	ist.str(line);
	while(ist >> number_edges1)
	{
		number_edges = std::stoi(number_edges1);
		cout << "Number of Edges: "<< number_edges << endl;
	} 
	ist.clear();
	getline(inf,line);
	ist.str(line);
	string maximum_color1;
	int maximum_color;
	while(ist >> maximum_color1)
	{
		maximum_color = std::stoi(maximum_color1);
		cout << "Maximum Color: "<< maximum_color<< endl;
	} 
	ist.clear();


	Node myInstance[number_nodes+1];
	int n , m;
	map<int, vector<int>> edge;
	vector<int> k;
	for (int i = 0; i < number_nodes ; i ++){
		edge.insert(make_pair(i,k));

	}

	aL.resize((int) edge.size());

	while(getline(inf, line))
	{
		int from, to;
		string first, second;
		ist.str(line);

		ist >> from;
		ist >> to;
		edge[from].push_back(to);
		edge[to].push_back(from);
		aL[from].push_back(to);
		aL[to].push_back(from);
		ist.clear();
	}

	ist.clear();

	vector<string > c_color;
	for (int i = 0 ; i < maximum_color ; i ++){
		c_color.push_back(std::to_string(i+1));
	}
	vector<Node> node;

	for (auto it = edge.begin(); it!= edge.end(); it++){
		myInstance[it->first].add_details(it->first, it->second, c_color);
		node.push_back( myInstance[it->first]);
	}

	stack<int> order;
	bool sol = true;

	colorMap(number_nodes, node, order);

	visual(edge, argv, argc);

	return 0;
}
