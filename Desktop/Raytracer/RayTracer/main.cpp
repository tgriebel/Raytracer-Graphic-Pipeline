
#include<iostream>
#include<fstream>
#include"MVector.h"
#include"matrix.h"
#include<cmath>
#include<queue>
#include<sstream>
#include <assert.h> 
using namespace std;

#include"bitmap.h"

const double WIDTH = 640.0;
const double HEIGHT = 480.0;
const double HEIGHT_DIV2 = (HEIGHT*0.5);
const double WIDTH_DIV2 = (WIDTH*0.5);
const double ASPECT = 4./3.;
const double FOV = 0.8;

const double MAXD = 4;


class VertexStore{//registers vertices

};

class Color{//TODO: clean-up, make more efficient, use a union

private:

	typedef unsigned char	byte;
	typedef unsigned int	uint32;
	typedef unsigned short	uint16;
	typedef double			dec;

	static const byte RED_SHIFT		= 24;
	static const byte GREEN_SHIFT	= 16;
	static const byte BLUE_SHIFT	= 8;
	static const byte ALPHA_SHIFT	= 0;

	static const uint32 RED_MASK	= 0xFF000000;
	static const uint32 GREEN_MASK	= 0x00FF0000;
	static const uint32 BLUE_MASK	= 0x0000FF00;
	static const uint32 ALPHA_MASK	= 0x000000FF;

	union{
		struct{
			byte r, g, b, a;	
		}color_components;
		uint32 raw;
	}color;//TODO: actually utilize this

	byte r, g, b, a;

	inline dec			_floatColor(byte ci)								{ return static_cast<dec>(ci)/255.0f; }
	inline byte			_binColor(dec value)								{ return static_cast<byte>(value*255.0f); }
	inline uint32		_byteToWord( byte ci, uint32 mask, byte shift )		{ return (static_cast<uint32>(ci) << shift) & mask; }
	inline byte			_wordToByte( uint32 word, uint32 mask, byte shift )	{ return static_cast<uint32>((word & mask)>>shift); }

	Color( byte ri, byte gi, byte bi, byte ai)
	: r(ri), g(gi), b(bi), a(ai) {}

public:

	static const uint32 RED		= 0xFF0000FF;
	static const uint32 GREEN	= 0x00FF00FF;
	static const uint32 BLUE	= 0x0000FFFF;
	static const uint32 WHITE	= 0xFFFFFFFF;
	static const uint32 BLACK	= 0x000000FF;
	static const uint32 LGREY	= 0xEEEEEEFF;
	static const uint32 DGREY	= 0x333333FF;
	static const uint32 BROWN	= 0x111111FF;
	static const uint32 CYAN	= 0x00FFFFFF;
	static const uint32 YELLOW	= 0xFFFF00FF;
	static const uint32 PURPLE	= 0xFF00FFFF;
	static const uint32 ORANGE	= 0xFFA500FF;
	static const uint32 PINK	= 0xFF69B4FF;
	static const uint32 GOLD	= 0xFFD700FF;

	Color( uint32 color)
		:	r(_wordToByte(color, RED_MASK, RED_SHIFT)), 
			g(_wordToByte(color, GREEN_MASK, GREEN_SHIFT)), 
			b(_wordToByte(color, BLUE_MASK, BLUE_SHIFT)), 
			a(_wordToByte(color, ALPHA_MASK, ALPHA_SHIFT))
	{}
	Color( dec ri, dec gi, dec bi, dec ai)
	: r(_binColor(ri)), g(_binColor(gi)), b(_binColor(bi)), a(_binColor(ai))
	{}

	Color& operator=( const Color& c){
		r = c.r;
		g = c.g;
		b = c.b;
		a = c.a;
		return *this;
	}

	inline void setRed( dec red)		{	assert(red<=1.0f && red>=0.0f);		r = _binColor(red);		}
	inline void setGreen( dec green)	{	assert(green<=1.0f && green>=0.0f);	g = _binColor(green);	}
	inline void setBlue( dec blue)		{	assert(blue<=1.0f && blue>=0.0f);	b = _binColor(blue);	}
	inline void setAlpha( dec alpha)	{	assert(alpha<=1.0f && alpha>=0.0f);	a = _binColor(alpha);	}

	inline dec getRed()		{ return _floatColor(r); }
	inline dec getGreen()	{ return _floatColor(g); }
	inline dec getBlue()	{ return _floatColor(b); }
	inline dec getAlpha()	{ return _floatColor(a); }

	inline byte rawRed()	{ return r; }
	inline byte rawGreen()	{ return g; }
	inline byte rawBlue()	{ return b; }
	inline byte rawAlpha()	{ return a; }

	uint32 rawGetColor()
	{
		return	_byteToWord(r, RED_MASK,	RED_SHIFT)
			|	_byteToWord(g, GREEN_MASK,	GREEN_SHIFT)
			|	_byteToWord(b, BLUE_MASK,	BLUE_SHIFT) 
			|	_byteToWord(a, ALPHA_MASK,	ALPHA_SHIFT);
	}

	Color operator*(float value){
		float nr=_floatColor(r)*value;
		float ng=_floatColor(g)*value;
		float nb=_floatColor(b)*value;
		return Color(	r=_binColor(nr >= 1.0f ? 1.0f : nr),
						g=_binColor(ng >= 1.0f ? 1.0f : ng),
						b=_binColor(nb >= 1.0f ? 1.0f : nb),
						a);
	}

	Color operator+(float value){
		float nr=_floatColor(r)+value, ng=_floatColor(g)+value, nb=_floatColor(b)+value;
		return Color(	r=_binColor(nr >= 1.0f ? 1.0f : nr),
						g=_binColor(ng >= 1.0f ? 1.0f : ng),
						b=_binColor(nb >= 1.0f ? 1.0f : nb),
						a);
	}
};

struct Ray{

	Ray(): t(-1.0) {}
	vec3d d;
	vec3d o;
	double t;
};

struct vertex{//TODO: use smart-pointer vertices class
	vec4d pos;
	unsigned int color;
	int instances; // how many of this vertices exist? 
	
	vertex(double x, double y, double z){
		pos[0] = x;
		pos[1] = y;
		pos[2] = z;
		pos[3] = 1.0;
	};
	vertex(){};
	double x() const { return pos[0];}
	double y() const { return pos[1];}
	double z() const { return pos[2];}
	//TODO: move to vec3d layer
	void setPos(double x, double y, double z = 0.0){ pos[0] = x; pos[1] = y; pos[2] = z; pos[3] = 1.0;}
};

struct triangle{

	vertex* pts[3];
	vec3d v0, v1, v2;
	bool isTextured;
	double p0_u, p0_v, p1_u, p1_v, p2_u,p2_v;
	vec3d N;
	double D;

	triangle(vertex& pt0, vertex& pt1, vertex& pt2):
	p0_u(0), p0_v(0), p1_u(0), p1_v(0), p2_u(0),p2_v(0), isTextured(false)
	{
		init(pt0, pt1, pt2);
	}
	triangle(): p0_u(0), p0_v(0), p1_u(0), p1_v(0), p2_u(0),p2_v(0), isTextured(false){}

	//TODO: use proper uv instead of texture's (x, y)
	void setUV(double p0_u, double p0_v, double p1_u, double p1_v, double p2_u, double p2_v){
	
		this->p0_u = p0_u;
		this->p0_v = p0_v;
		this->p1_u = p1_u;
		this->p1_v = p1_v;
		this->p2_u = p2_u;
		this->p2_v = p2_v;

		isTextured = true;
	}

	void init(vertex& pt0, vertex& pt1,vertex& pt2){
	
		pts[0] = &pt0;
		pts[1] = &pt1;
		pts[2] = &pt2;

		refresh();
	}

	void refresh(){
		v0[0]=pts[0]->pos[0]; v0[1]=pts[0]->pos[1]; v0[2]=pts[0]->pos[2];
		v1[0]=pts[1]->pos[0]; v1[1]=pts[1]->pos[1]; v1[2]=pts[1]->pos[2];
		v2[0]=pts[2]->pos[0]; v2[1]=pts[2]->pos[1]; v2[2]=pts[2]->pos[2];

		N = ( v1-v0 ) ^ ( v2-v0 );
		N = N.normalize();
		D = v0 * N;
	}

	void setPoint(unsigned pt_n, double x0, double y0, double z0){
		//const vertex t_vert(x0, y0, z0);
		assert(false);
	}

};

const int length = 36;
vertex vertList[length];
triangle triList[length/3];

uint32 color_blend( uint32 c1, uint32 c2);

//TODO: resolve overlapping intersection problem
bool intersect( Ray &r, const triangle& tri, vec3d& it_pt, uint32& color){ 

	vec3d edge1 = tri.v1 - tri.v0; 
	vec3d edge2 = tri.v2 - tri.v0; 
	vec3d pvec = r.d^edge2; 
	double det = edge1*pvec; 
	if (det == 0) return false; 
	double invDet = 1. / det; 
	vec3d tvec = r.o - tri.v0; 
	double u = tvec*pvec * invDet; 
	
	if (u < 0 || u > 1) return false; 
	
	vec3d qvec = tvec^edge1; 
	
	double v = invDet*(r.d*qvec); 
	
	if (v < 0 || u + v > 1)
		return false; 
	
	double t = (edge2*qvec) * invDet; 

	if( t <= r.t && t >= 0)
	{ 
		it_pt = r.o + t*r.d;

		r.t = t;
		color = tri.pts[0]->color;

		return true; 
	}else{
		return false;
	}
}

double lerp(double px, double x0, double y0, double x1, double y1){
	return ((x1 - px) * y0 + (px - x0) * y1) / (x1 - x0);
}

void drawTrianle(const triangle& tri){
	tri.pts[0]->x();
	assert(false);
}

void floodFill(Bitmap& bitmap, uint32 x, uint32 y, uint32 tcolor, uint32 rcolor){
	
	bool outBounds = (x >= bitmap.getWidth() || x < 0) || (y >= bitmap.getHeight() || y < 0);
	if(outBounds) return;
	if(bitmap.getPixel(x, y) != tcolor) return;
	
	struct node_s{
		node_s(uint32 xi, uint32 yi) : x(xi), y(yi) {};
		uint32 x, y;
	} node(x, y);

	deque<node_s> q;
	q.push_back(node);

	while(!q.empty()){
	
		node_s n = q.front();
		q.pop_front();

		if(bitmap.getPixel(n.x, n.y) == tcolor){
			node_s w(n), e(n);
			
			for(;(bitmap.getPixel(w.x - 1, w.y) == tcolor) && (w.x != 0); w.x--);//find min horizonal bond
			for(;(bitmap.getPixel(e.x + 1, e.y) == tcolor) && (e.x != bitmap.getWidth() - 1); e.x++); //find max bond

			for(uint32 i(w.x); i <= e.x; ++i)		//fill line
				bitmap.setPixel(i, w.y, rcolor);

			for(uint32 i(w.x + 1); i < e.x; ++i){ //explore outside of filled line
				node_s t_node(i, 0);
				
				t_node.y = w.y + 1;	//north of node
				if( (t_node.y > 0) && (bitmap.getPixel(t_node.x, t_node.y) == tcolor))	q.push_back(t_node);
				
				t_node.y = w.y - 1;	//south of node
				if( (t_node.y < bitmap.getHeight()) && (bitmap.getPixel(t_node.x, t_node.y) == tcolor))	q.push_back(t_node);
			}
		}
	}

	return;
}
void bresenham(Bitmap& bitmap, int x0, int y0, int x1, int y1, unsigned int color){
	//optimized bresenham algorithm
	int dx = abs(x1 - x0);
	int dy = abs(y1 - y0);

	int sx, sy;
	(x0 < x1)? sx = 1: sx = -1;
	(y0 < y1)? sy = 1: sy = -1;

	int e = (dx - dy);

	while( (x0 != x1) || (y0 != y1) )
	{
		bitmap.setPixel(x0, y0, color);

		int e2 = 2*e;

		if(e2 > -dy)
		{
			e	-= dy;
			x0	+= sx;
		}

		if(e2 < dx)
		{
			e	+= dx;
			y0	+= sy;
		}
	}

	bitmap.setPixel(x0, y0, color);
}
/*
double sines[37], cosines[37];
void computeTrig(){

	double c = 3.14159/180.0;

	for(int deg = -180; deg <= 180; deg += 10){
		double degr = c*deg;
		sines[deg/10 + 18] = sin(degr);
		cosines[deg/10 + 18] = cos(degr);
	}
}*/

void writeOutMesh( const string& fileName ){

	fstream file;
	file.open( fileName, std::fstream::out );

	for( unsigned i = 0; i < length; ++i){
		
		file << "v " << vertList[i].pos[0] << " " << vertList[i].pos[1] << " " << vertList[i].pos[2] << "\n";
	}

	for( unsigned i = 0; i < length/3; ++i){
		
		vec3d b1 = vec3d( triList[i].v1 - triList[i].v0 );
		vec3d b2 = vec3d( triList[i].v2 - triList[i].v0 );
		
		vec3d norm = b1^b2;

		file << "fn " << norm[0] << " " << norm[1] << " " << norm[2] << "\n";
	}

	for( unsigned i = 0; i < length/3; ++i){

		file << "f " << (i*3 + 1) << " " << (i*3+1 + 1) << " " << (i*3+2 + 1) << "\n";
	}
}

vec3d calcBarycentricCoord(vec3d& pt, vec3d& v1, vec3d& v2, vec3d& v3){

	vec3d e1 = v3 - v2;
	vec3d e2 = v1 - v3;
	vec3d e3 = v2 - v1;

	vec3d d1 = pt - v1;
	vec3d d2 = pt - v2;
	vec3d d3 = pt - v3;

	vec3d n = (e1^e2)/((e1^e2).magnitude());

	return vec3d( (((e1^d3)*n))/((e1^e2)*n), (((e2^d1)*n))/((e1^e2)*n), (((e3^d2)*n))/((e1^e2)*n) );
}

void drawTriangles(Bitmap& bitmap, double degree, bool x_rot, bool y_rot, bool z_rot, vec3d& trans){
	
	int size = 3;
	const int nLights = 3;

	vec3d light( 150.0, 150.0, -500.0);

	vec3d lights[nLights];

	//TODO: replace with .obj loader
	lights[0] = vec3d( 150.0, 150.0, -500.0);
	lights[1] = vec3d( 20.0, 20.0, -500.0);
	lights[2] = vec3d( 20.0, 150.0, 25.0);

	//front, red
	vertList[0].setPos(0.0, 0.0, 0.0); 
	vertList[1].setPos(50.0, 50.0, 0.0);
	vertList[2].setPos(50.0, 0.0, 0.0); 

	vertList[3].setPos(0.0, 0.0, 0.0); 
	vertList[4].setPos(0.0, 50.0, 0.0); 
	vertList[5].setPos(50.0, 50.0, 0.0);

	for(int i(0); i < 6; ++i) vertList[i].color = 0xFF0000FF;
	//lside, green
	vertList[6].setPos(0.0, 0.0, 0.0); 
	vertList[7].setPos(0.0, 50.0, 50.0);
	vertList[8].setPos(0.0, 50.0, 0.0); 

	vertList[9].setPos(0.0, 0.0, 0.0); 
	vertList[10].setPos(0.0, 0.0, 50.0); 
	vertList[11].setPos(0.0, 50.0, 50.0);

	for(int i(6); i < 12; ++i) vertList[i].color = 0x00FF00FF;
	//rside, blue
	vertList[12].setPos(50.0, 0.0, 0.0); 
	vertList[13].setPos(50.0, 50.0, 0.0); 
	vertList[14].setPos(50.0, 50.0, 50.0); 

	vertList[15].setPos(50.0, 0.0, 0.0); 
	vertList[16].setPos(50.0, 50.0, 50.0); 
	vertList[17].setPos(50.0, 0.0, 50.0); 

	for(int i(12); i < 18; ++i) vertList[i].color = 0x0000FFFF;
	//bside, yellow
	vertList[18].setPos(50.0, 0.0, 0.0); 
	vertList[19].setPos(0.0, 0.0, 50.0);
	vertList[20].setPos(0.0, 0.0, 0.0);

	vertList[21].setPos(50.0, 0.0, 0.0); 
	vertList[22].setPos(50.0, 0.0, 50.0); 
	vertList[23].setPos(0.0, 0.0, 50.0);

	for(int i(18); i < 24; ++i) vertList[i].color = 0xFFFF00FF;
	//tside, cyan
	vertList[24].setPos(0.0, 50.0, 0.0); 
	vertList[25].setPos(0.0, 50.0, 50.0); 
	vertList[26].setPos(50.0, 50.0, 0.0); 

	vertList[27].setPos(50.0, 50.0, 0.0); 
	vertList[28].setPos(0.0, 50.0, 50.0); 
	vertList[29].setPos(50.0, 50.0, 50.0);

	for(int i(24); i < 30; ++i) vertList[i].color = 0x00FFFFFF;
	//back, purple
	vertList[30].setPos(0.0, 0.0, 50.0); 
	vertList[31].setPos(50.0, 0.0, 50.0); 
	vertList[32].setPos(50.0, 50.0, 50.0); 

	vertList[33].setPos(0.0, 0.0, 50.0); 
	vertList[34].setPos(50.0, 50.0, 50.0);
	vertList[35].setPos(0.0, 50.0, 50.0); 
	for(int i(30); i < 36; ++i) vertList[i].color = 0xFF00FFFF;

	double theta = degree;
	double a[] = { 1.0, 0.0, 0.0, 0.0,
				  0.0, cos(theta), -sin(theta), 0.0,
				  0.0, sin(theta), cos(theta), 0.0,
				  0.0, 0.0, 0.0, 1.0
				};
	double b[] = { cos(theta), 0.0, sin(theta), 0.0,
				  0.0, 1.0, 0, 0.0,
				  -sin(theta), 0.0, cos(theta), 0.0,
				  0.0, 0.0, 0.0, 1.0
				};
	double c[] = { cos(theta), -sin(theta), 0.0, 0.0,
				  sin(theta), cos(theta), 0.0, 0.0,
				  0.0, 0.0, 1.0, 0.0,
				  0.0, 0.0, 0.0, 1.0
				};
	double t[] = { 1.0, 0.0, 0.0, 60.0,
				  0.0, 1.0, 0.0, 60.0,
				  0.0, 0.0, 1, 0.0,
				  0.0, 0.0, 0.0, 1.0
				};
	double i[] = { 1.0, 0.0, 0.0, 0.0,
				0.0, 1.0, 0.0, 0.0,
				0.0, 0.0, 1.0, 0.0,
				0.0, 0.0, 0.0, 1.0
			};
	double p[] = { 1.0, 0.0, 0.0, 0.0,
				0.0, 1.0, 0.0, 0.0,
				0.0, 0.0, -1.0, 1./4.,
				0.0, 0.0, 0.0, 0.0
			};

	//TODO: better matrix contructor interface?
	mat4x4d mi(i);
	mat4x4d mp(p);
	mat4x4d mrx(a);
	mat4x4d mry(b);
	mat4x4d mrz(c);
	mat4x4d mt(t);
	mat4x4d ms = mi*2.0;

	for(int i(0); i < length; ++i){
		if(y_rot) vertList[i].pos = mry*vertList[i].pos;
		if(x_rot) vertList[i].pos = mrx*vertList[i].pos;

		vertList[i].pos = ms*vertList[i].pos;
		vertList[i].pos = vertList[i].pos*mt;
	}

	triList[0].init( vertList[0], vertList[1], vertList[2]);
	triList[0].setUV( 0, 0, 256, 256, 0, 256);
	triList[1].init( vertList[3], vertList[4], vertList[5]);
	triList[1].setUV( 0, 0, 256, 0, 256, 256);
	triList[2].init( vertList[6], vertList[7], vertList[8]);
	triList[3].init( vertList[9], vertList[10], vertList[11]);
	triList[4].init( vertList[12], vertList[13], vertList[14]);
	triList[5].init( vertList[15], vertList[16], vertList[17]);
	triList[6].init( vertList[18], vertList[19], vertList[20]);
	triList[7].init( vertList[21], vertList[22], vertList[23]);
	triList[8].init( vertList[24], vertList[25], vertList[26]);
	triList[9].init( vertList[27], vertList[28], vertList[29]);
	triList[10].init( vertList[30], vertList[31], vertList[32]);
	triList[11].init( vertList[33], vertList[34], vertList[35]);

	Bitmap tex("tex2.bmp");

	for( uint32 y(0); y < bitmap.getHeight(); ++y)
	{
		for( uint32 x(0); x < bitmap.getWidth(); ++x)
		{
			//TODO: actually build frustrum, convert to world space
			//double rx = tanf(FOV*0.5f)*(x/WIDTH_DIV2 - 1.0f)/ASPECT;
			//double ry = tanf(FOV*0.5f)*(1.0f-y/HEIGHT_DIV2);
			double max_t = 1000;

			Ray ray;
			ray.o[0] = (double)x; ray.o[1] = (double)y; ray.o[2] = -100.0;
			ray.d[0] = 0.0; ray.d[1] = 0.0;	ray.d[2] = max_t;
			ray.t = max_t;

			for(int d(0); d < 1/*MAXD*/; ++d){//TODO: MAKE THIS AN ACTUAL RAYTRACER!!!!
							
				uint32 color;
				vec3d it_pt;
				int it_tri = -1;

				for( int t(0); t < 12; ++t)
				{
					bool ret = intersect(ray, triList[t], it_pt, color);

					if(ret) it_tri = t;
				}

				if(ray.t == max_t || it_tri == -1){
					bitmap.setPixel(x, y, 0x000000FF);
					break;
				}else{

					bool light_hit(false);
					Color current_color(triList[it_tri].pts[0]->color);
					if(triList[it_tri].isTextured){
					
						vec3d b_coords = calcBarycentricCoord(it_pt, triList[it_tri].v0, triList[it_tri].v1, triList[it_tri].v2);
						
						vec2d tex_pt = b_coords[0]*vec2d(triList[it_tri].p0_u, triList[it_tri].p0_v) + b_coords[1]*vec2d(triList[it_tri].p1_u, triList[it_tri].p1_v) + b_coords[2]*vec2d(triList[it_tri].p2_u, triList[it_tri].p2_v);
						int t1 = (int)tex_pt[0];
						int t2 = (int)tex_pt[1];
					
						uint32 raw_tex_color = tex.getPixel(t1, t2);
						current_color = Color(raw_tex_color);
					}

					Color final_color(0x0);

					for(int l_i(0); l_i < nLights; ++l_i){			
	 
						Ray shadowRay; 
						shadowRay.d = lights[l_i] - it_pt;

						if (triList[it_tri].N * shadowRay.d <= 0.0f){
							continue;
						}

						bool occuded = false;
						for( int t(0); t < 12; ++t)
						{
							if(intersect(shadowRay, triList[t], it_pt, color)){
								occuded = true;
							}
						}

						if(!occuded){

							vec3d light_dir = shadowRay.d;
							light_dir = light_dir.normalize();
							double lambert = (light_dir * triList[it_tri].N) * .8;
						
							Color tmp = current_color*lambert;
							double tmp_float_color = final_color.getRed() + tmp.getRed();
							final_color.setRed( tmp_float_color >= 1.0f ? 1.0f : tmp_float_color);

							tmp_float_color = final_color.getGreen() + tmp.getGreen();
							final_color.setGreen( tmp_float_color >= 1.0f ? 1.0f : tmp_float_color);

							tmp_float_color = final_color.getBlue() + tmp.getBlue();
							final_color.setBlue( tmp_float_color >= 1.0f ? 1.0f : tmp_float_color);
							light_hit=true;
						}
					}
					if(light_hit){
						bitmap.setPixel(x, y, final_color.rawGetColor());
					}else{
						//double reflect = 2.0f * (ray.d * triList[it_tri].N);
						//ray.o[0] = it_pt[0]; ray.o[1] = it_pt[1]; ray.o[2] = it_pt[2];
						//ray.d = ray.d - reflect*triList[it_tri].N;
						
						bitmap.setPixel(x, y, 0x000000FF);
					}
				}
			}//end depth loop
		}//end x loop
	}//end y loop

	for(int l_i(0); l_i < nLights; ++l_i){
		bitmap.setPixel((uint32)lights[l_i][0], (uint32)lights[l_i][1], 0xFFFFFFFF);
	}
}

//TODO: stop being paranoid and lay-off some of the masking
uint32 color_blend( uint32 c1, uint32 c2){
	
	uint32 color1 = (0xFFFFFF00 & c1);
	uint32 color2 = (0xFFFFFF00 & c2);
	double alpha1 = ((double)(0x000000FF & c1)) / 255.0f;
	double alpha2 = ((double)(0x000000FF & c2)) / 255.0f;

	return   ((uint32)((color1 & 0xFF000000)*alpha1 + (color2 & 0xFF000000)*alpha2*(1 - alpha1)) & 0xFF000000)
			| ((uint32)((color1 & 0x00FF0000)*alpha1 + (color2 & 0x00FF0000)*alpha2*(1 - alpha1)) & 0x00FF0000)
			| ((uint32)((color1 & 0x0000FF00)*alpha1 + (color2 & 0x0000FF00)*alpha2*(1 - alpha1)) & 0x0000FF00);
}

Bitmap composite(Bitmap& bitmap1, Bitmap& bitmap2){

	Bitmap tbitmap(bitmap1.getWidth(), bitmap1.getHeight());
	
	for(uint32 j(0); j < bitmap1.getHeight(); ++j){
		for(uint32 i(0); i < bitmap1.getWidth(); ++i){

			uint32 newColor = color_blend( bitmap1.getPixel(i, j), bitmap2.getPixel(i, j));

			tbitmap.setPixel(i, j, newColor);
		}
	}
	
	return tbitmap;
}

int main(void){
	//TODO: keep testing matrices!!!
	/*
	double m_values[] = {-1,2, 3, 4,5,6, 7, 8, 9, 10, 1, 12, 13, -14, 15, 16};

	mat4x4d m(m_values);

	bool canInvert=false;
	double det_value = det(m);
	std::cout << det_value << endl;
	mat4x4d m_inverse = inverse(m, canInvert);

	std::cout << m << endl;
	std::cout << m.transpose() << endl;
	std::cout << inverse(m_inverse, canInvert) << endl;
	std::cout << m << endl;

	if(canInvert)
		std::cout << "inverse[0][0]: " << m_inverse[0][0] << endl;
	*/

	//TODO: test colors a little more, actually just rewrite the class
	//Color c(1.0f, 1.0f, 1.0f, 1.0f);
	//std::cout << hex << c.rawGetColor() << endl;

	Bitmap bitty(WIDTH, HEIGHT);

	int images = 10;
	double dPhi = 3.14/(double)images;
	double degree = 0;
	for(int i=0; i < images; ++i){

		int size = 50;
		int* s;

		degree += dPhi;
		drawTriangles(bitty, degree, true, true, false, vec3d(0.0, 0.0, 0.0) );

		stringstream ss;
		ss << i;

		bitty.write("out" + ss.str() + ".bmp");
		std::cout << "Le Image out" << i << ".bmp done." << std::endl;
	}
	std::cout << "Finished." << endl;

	return 1;
}