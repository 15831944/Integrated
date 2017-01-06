#include <uf_defs.h>
#include <uf_ui_types.h>
#include <iostream>
#include <NXOpen/Session.hxx>
#include <NXOpen/UI.hxx>
#include <NXOpen/CurveCollection.hxx>

using namespace std;
using namespace NXOpen;

class ADOConn;
class Block;

class HeaterSegment
{
public:
	Curve *curve;
	Point3d p1;
	Point3d p2;
	bool added;
	int data;//1——spline 2——arc圆弧 3——line  4——arc整圆
	Vector3d dir;
};
class HeaterLine//一条完整的加热线中心线
{
public:
	std::vector<Curve*> curves;
	Point3d start_point;
	Point3d end_point;
	Point3d other_point;
	int other;//1-end point,2-start point
	long double length;
	Vector3d dir;
};
class Heater
{
public://函数
	Heater();
	virtual ~Heater();
	ADOConn *adoconn;
	void CreateHeater(long double manifold_H);
	int GetSegments(long double *total_length);
	HeaterLine GetHeaterLine(Curve *aline);
	void CreateTube(int index,HeaterLine heater_line,long double outer_D,long double innner_D,long double Z);
	void SaveCode2Tube(int curve_index,long double length);
	void CreateBox(int curve_index,Vector3d dir,HeaterLine heater_line,long double Z);
		
	void CreateSwept(int curve_index,HeaterLine heater_line);
	
	void CreateText(int index,HeaterLine heater_line);
	void Crave();

	static bool LineSort(const HeaterLine &v1, const HeaterLine &v2);
public:
	long double H;//分流板厚度
	NXString str_BHRT;
	std::vector<HeaterSegment> heater_segments;//所有加热线
	int int_layer;
};