#include <stdio.h> 
#include <string.h> 
#include <math.h> 
#include <iostream>
#include <fstream>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include <vector>
#include "BVHParser.h" 

using namespace std;

BVHParser::BVHParser()
{
}

//Add checks for bad files
NODE* BVHParser::getFigure(const char *filename) 
{
	string line;
	ifstream bvhFile (filename);
	
	NODE *root = (NODE*) malloc(sizeof(NODE));
	NODE *currentNode = root;
	NODE *parent = NULL;

	bool buildingHierarchy = false;
	bool buildingRootNode = false;
	bool buildingJointNode = false;
	bool closingNode = false;

	if (bvhFile.is_open())
	{
		while (bvhFile.good())
		{
		  getline(bvhFile,line);
		  unsigned found = line.find("HIERARCHY");

		  if(found != string::npos)	//If HIERARCHY we're dealing with a new set of mocap data
		  {
				//cout<<"Hierarchy Found"<<endl;
				buildingHierarchy = true;

				found = line.find("ROOT");	//Check if ROOT is in the same line as HIERARCHY
				if(found == string::npos)	//If ROOT does not share the same line as HIERARCHY, get the next line
				{
					getline(bvhFile,line);
				}else{
					buildingRootNode = true;
				}
		  }

		  while(buildingHierarchy)
		  {
			  buildingJointNode = (line.find("JOINT") != string::npos);
			  closingNode = (line.find("}") != string::npos);

			  if(!buildingRootNode)		//if ROOT wasn't found in the same line as HIERARCHY
			  {
				  buildingRootNode = (line.find("ROOT") != string::npos);
				  
			  }else if(buildingJointNode){
					buildingRootNode = false;
			  }

			  if(buildingJointNode || buildingRootNode)	
			  {
				  if(buildingRootNode)
				  {
					  currentNode->parent = NULL;
				  }else
				  {
					  currentNode = (NODE*) malloc(sizeof(NODE));
					  currentNode->parent = parent;
				  }
				  
				  currentNode->children = NULL;
				  currentNode->noofchildren = 0;
				  currentNode->length = 0;
				  //Set Node's name
				  string nodeName = getName(line);
				  //cout<<nodeName<<endl;

				  currentNode->name = (char*) malloc(strlen(nodeName.c_str()) + 1);
				  strcpy(currentNode->name, nodeName.c_str());
				  //cout<<currentNode->name<<endl;

				  //May be able to assume that next line is curly brace, which means we can get rid of this chunk and replace with getline
				  getline(bvhFile,line);
				  found = line.find("{");
				  if(found != string::npos)
				  {
					  getline(bvhFile,line);
				  }

				  //Set offset of node
				  found = line.find("OFFSET");
				  if(found != string::npos)	
				  {
					  //cout<<"OFFSET Found"<<endl;
					  float* offset = getOffset(line);
					  memcpy(currentNode->offset, offset, sizeof currentNode->offset);
					  //cout<<currentNode->offset[0]<<endl;			//Debugging check
				  }

				  //Set degrees of freedom
				  getline(bvhFile,line);
				  found = line.find("CHANNELS");
				  if(found != string::npos)	
				  {

					  vector<string> channels = getChannels(line);
					  currentNode->channels = (string*)malloc(sizeof(string)*channels.size());
					  memcpy(currentNode->channels, channels.data(),sizeof(string)*channels.size());
					  currentNode->DOF = channels.size();
				  }
				  //cout<<currentNode->channels[1]<<endl;
				  //Checking if we've reached a leaf node
				  getline(bvhFile,line);
				  found = line.find("End Site");
				  if(found != string::npos)
				  {
					  getline(bvhFile,line);	//get beginning bracket
					  getline(bvhFile,line);	//get offset

					  found = line.find("OFFSET");
					  float* endOffset;
					  if(found != string::npos)	
					  {
						  endOffset = getOffset(line);
						  float length = (float)sqrt(pow( (endOffset[0]-currentNode->offset[0]), 2.0f) +pow((endOffset[1]-currentNode->offset[1]), 2.0f)+pow((endOffset[2]-currentNode->offset[2]), 2.0f));
						  currentNode->length = length;
					  }
					  getline(bvhFile,line);	//get closing bracket for End Site

					  if(line.find("}") != string::npos)
					  {
						  buildingJointNode = false;
						  buildingRootNode = false;
						  closingNode = true;
						  getline(bvhFile,line);
					  }
				  }else{
					  parent = currentNode;
				  }

			  }else if(closingNode)
			  {
				  if(parent != NULL)
				  {
					addChild(parent, currentNode);
					 float length = (float)sqrt(pow((parent->offset[0]-currentNode->offset[0]), 2.0f) +pow((parent->offset[1]-currentNode->offset[1]), 2.0f)+pow((parent->offset[2]*currentNode->offset[2]), 2.0f));
					 currentNode->length = length;
				  }
				  getline(bvhFile,line);
				  if(line.find("}") != string::npos)
				  {
					  currentNode = parent;
					  parent = parent->parent;

				  }else if(line.find("JOINT") != string::npos)
				  {
					  closingNode = false;
					  buildingJointNode = true;
				  }else if(line.find("MOTION") != string::npos)
				  {
					  buildingHierarchy = false;
				  }

			  }
		  }

		  //cout<<line<<endl;			//Debugging test
		}
		bvhFile.close();

		return currentNode;
	}
	else
	{
		cout<<"Error Reading File"<<endl;
		return NULL;
	}

}

vector<float> BVHParser::getMotions(const char *filename)
{
	string line;
	ifstream bvhFile (filename);
	string::size_type found = string::npos;

	float frames = 0;
	float frameTime = 0;
	vector<float> motionData;

	bool buildingMotion = false;

	if (bvhFile.is_open())
	{
		while (bvhFile.good())
		{
		  while(found == string::npos)		//iterate through file until we find motion
		  {
			 getline(bvhFile,line);
			 found = line.find("MOTION");
		  }

		  getline(bvhFile,line);
		  if(line.find("Frames") !=string::npos)
		  {
			  frames = getFrames(line);
		  }
		  motionData.push_back(frames);

		  getline(bvhFile,line);
		  if(line.find("Frame Time") !=string::npos)
		  {
			  frameTime = getFrameTime(line);
		  }
		  motionData.push_back(frameTime);

		  getline(bvhFile,line);
		  bool lineAvailable = true;
		  while(lineAvailable)
		  {
			motionData = addMotions(line, motionData);
			lineAvailable = getline(bvhFile,line);
		  }

		}
	}

	return motionData;
}

string BVHParser::getName(string s)
{
	vector <string> nameData;
	boost::split(nameData,s, boost::is_any_of("\t "),boost::token_compress_on);

	while(*nameData.begin() == "")
	{
		nameData.erase(nameData.begin());		//any empty space in vector
	}

	nameData.erase(nameData.begin());		//Removes "JOINT" or "ROOT"
	return nameData[0];
}

vector<string> BVHParser::getChannels(string s)
{
	vector <string> channelData;
	boost::split(channelData,s, boost::is_any_of("\t "),boost::token_compress_on);

	while(*channelData.begin() == "")
	{
		channelData.erase(channelData.begin());		//any empty space in vector
	}

	channelData.erase(channelData.begin());		//Removes "CHANNEL"
	channelData.erase(channelData.begin());		//Removes Dof

	//string* channels = malloc(channelData.size()*sizeof(string)

	return channelData;
}

float* BVHParser::getOffset(string s)
{
	vector <string> offsetData;
	boost::split(offsetData,s, boost::is_any_of("\t "),boost::token_compress_on);
	
	while(*offsetData.begin() == "")
	{
		offsetData.erase(offsetData.begin());		//any empty space in vector
	}

	offsetData.erase(offsetData.begin());		//Removes "OFFSET"

	float offset[3];						//change to float?  delete later?
	int ctr = 0;
	string tmp;
	for (std::vector<string>::iterator it = offsetData.begin(); it != offsetData.end(); ++it)
	{
		tmp = *it; 
		offset[ctr] = (float) atof(tmp.c_str());
		ctr++;
	}

	return offset;
}

void BVHParser::addChild(NODE* parent, NODE* currentNode)
{
	parent->noofchildren++;
	if(parent->children == NULL)
	{
		parent->children = (NODE**) malloc(sizeof(NODE*));
		//memcpy(parent->children[0], currentNode, sizeof(NODE));	
		parent->children[0] = currentNode;
		
	}else{
		NODE **temp = (NODE**)realloc(parent->children,(parent->noofchildren)*sizeof(NODE*));
		parent->children = temp;
		parent->children[parent->noofchildren-1] = currentNode;

		//cout<<endl<<"Children("<<parent->name<<"):"<<endl;
		for(int i = 0; i<parent->noofchildren; i++)
		{
			//cout<<parent->children[i]->name<<endl;
		}

	}
	
}

float BVHParser::getFrames(string s)
{
	vector <string> frameData;
	//boost::split(frameData, s, boost::is_any_of( " /t" ) );
	boost::split(frameData,s, boost::is_any_of("\t "),boost::token_compress_on);
	float frames = 0;

	frameData.erase(frameData.begin());		//Removes "Frame:"

	frames = (float) atof((*frameData.begin()).c_str());
	return frames;
}

float BVHParser::getFrameTime(string s)
{
	vector <string> frameTimeData;
	//boost::split(frameTimeData, s, boost::is_any_of( " /t" ) );
	boost::split(frameTimeData,s, boost::is_any_of("\t "),boost::token_compress_on);
	float frameTime = 0;

	frameTimeData.erase(frameTimeData.begin());		//Removes "Frame"
	frameTimeData.erase(frameTimeData.begin());		//Removes "Time:"

	frameTime = (float) atof((*frameTimeData.begin()).c_str());

	return frameTime;
}

vector<float> BVHParser::addMotions(string s, vector<float> motions)
{
	vector <string> stringData;
	boost::split(stringData, s, boost::is_any_of( " \t" ) );

	for(std::vector<string>::iterator it = stringData.begin(); it != stringData.end(); ++it)
	{
		if(*it != "")
		{
			motions.push_back((float)atof((*it).c_str()));
		}
	}
	return motions;
}
BVHParser::~BVHParser()
{
}