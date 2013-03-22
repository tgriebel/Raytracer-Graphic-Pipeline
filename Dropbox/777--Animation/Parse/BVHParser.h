#include <vector>
#include <string.h>

using namespace std;

//struct MOCAPSEGMENT 
//{ 
//	 char *name; // Name of motion capture file 
//	 NODE* root; // Pointer to the root node of the animation 
//	 MOCAPHEADER* header; // Pointer to a structure that containers global parameters 
//	 NODE** nodelist; // Array of pointers to skeletal nodes 
//}; 
//
//struct MOCAPHEADER 
//{ 
//	 // Assumes that all angles are in degrees if not then they need to be converted 
//	 int noofsegments; // Number of body segments 
//	 long noofframes; // Number of frames 
//	 int datarate; // Number of frames per second 
//	 int euler[3][3]; // Specifies how the euler angle is defined 
//	 float callib; // Scale factor for converting current translation units into meters
//	 bool degrees; // Are the rotational measurements in degrees 
//	 float scalefactor; // Global Scale factor 
//	 long currentframe; // Stores the current frame to render 
//	 float floor; // Specifies position of the floor along the y-axis 
//}; 
//
struct NODE // Start of structure representing a single bone in a skeleton 
{ 
	 char *name; 
	 float length; // Length of segment along the Y-Axis 
	 float offset[3]; // Transitional offset with respect to the end of the parent link 
	 //float euler[3]; // Rotation of base position 
	 //float colour[3]; // Colour used when displaying wire frame skeleton 
	 int noofchildren; // Number of child nodes 
	 NODE **children; // Array of pointers to child nodes 
	 NODE *parent; // Back pointer to parent node 
	 //float **froset; // Array of offsets for each frame 
	 //float **freuler; // Array of angles for each frame 
	 //float *scale; // Array of scalefactors for each frame 
	 int DOF; 
	 string* channels;
	 //vector<string> channels;
	 //BYTE DOFs; // Used to determine what DOFs the segment has 
	 //OBJECTINFO* object; // Used to point to a 3D object that is the limb 
	 //CONSTRAINT* constraints; // List of constraints - ordered in time 
}; 

class BVHParser 
{
	public:
		BVHParser();
		NODE* getFigure(const char *filename);
		vector<float> getMotions(const char *filename);
		bool ExportData(const char *filename);
		~BVHParser();

	private:
		int xpos, ypos, zpos;
		string getName(string s);
		vector<string> getChannels(string s);
		float* getOffset(string s);
		void addChild(NODE* parent, NODE* currentNode);

		float getFrames(string s);
		float getFrameTime(string s);
		vector<float> addMotions(string s, vector<float> motions);

		/*
		void IncreaseChildren(NODE* node);
		void RotateSegment(float &x, float &y, float &z, NODE* tnode);
		void ReCalcRotations(NODE* curnode);
		*/
};