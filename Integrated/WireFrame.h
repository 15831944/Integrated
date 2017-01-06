#pragma once
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

class WireSegment
{
public:
	Line *line;
	Point3d p1;//start
	Point3d p2;//end
	bool added;
};
class WireLine//һ���������߼�
{
public:
	std::vector<Line*> lines;
	std::vector<Point3d> points;//ת�ǵ㣬���������һ���˵�
	Point3d sp;
	Point3d ep;
	long double length;
};

class WireFrame
{
public:
	WireFrame(long double manifold_H);
	~WireFrame(void);
	ADOConn *adoconn;

	void CreateWireFrame();

	int GetSegments();
	int GetWireLines();
	void CreateWireLine(WireLine wire_line,int index);
	

public:
	NXString str_BHRT;
	std::vector<WireSegment> wire_segments;//�����߼��߶�
	std::vector<WireLine> wire_lines;//�����߼�
	int int_layer;
	std::vector<Block> blocks;
	long double g_manifold_H;
};

