#include <string>

/*struct NODE // Start of structure representing a single bone in a skeleton 
{ 
	 char *name; 
	 float length; // Length of segment along the Y-Axis 
	 float offset[3]; // Transitional offset with respect to the end of the parent link 
	 float euler[3]; // Rotation of base position 
	 float colour[3]; // Colour used when displaying wire frame skeleton 
	 int noofchildren; // Number of child nodes 
	 NODE **children; // Array of pointers to child nodes 
	 NODE *parent; // Back pointer to parent node 
	 float **froset; // Array of offsets for each frame 
	 float **freuler; // Array of angles for each frame 
	 float *scale; // Array of scalefactors for each frame 
	 int DOF; 
	 vector<string> channels;
	 //BYTE DOFs; // Used to determine what DOFs the segment has 
	 //OBJECTINFO* object; // Used to point to a 3D object that is the limb 
	 //CONSTRAINT* constraints; // List of constraints - ordered in time 
}; */

class Node 
{
	public:
		char *name;
		float offset[3];
		int noOfChildren;
		Node** children;
		Node* parent;
		string* channels;

};