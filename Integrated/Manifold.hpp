//2015-08-22 19:45 beta
#include <uf_defs.h>
#include <uf_ui_types.h>
#include <uf.h>
#include <ug_session.hxx>
#include <uf_ui.h>
#include <iostream>
#include <NXOpen/Session.hxx>
#include <NXOpen/UI.hxx>
#include <NXOpen/CurveCollection.hxx>
#include <NXOpen/CurveDumbRule.hxx>
#include <NXOpen/CurveFeatureRule.hxx>
#include <NXOpen/IParameterizedSurface.hxx>

using namespace std;
using namespace NXOpen;

class ADOConn;

class LayerCurve
{
public:
	NXObject *object;
	bool added;//�Ƿ���ӵ���������
	bool ear_added;//�Ƿ��������
	int type;//0 δȷ�� 1 ��������� 2 ��������� 3 һ��
	int data;//1����spline 2����arcԲ�� 3����line  4����arc��Բ
	Point3d p1;
	Point3d p2;
};
class Manifold
{
public:
	struct LineStruct
	{
		Line* line;
		long double len;
	};

	Manifold();
	virtual ~Manifold();
	ADOConn *adoconn;
	int Create(string str_ZF,string str_SR,long double manifold_H,long double runner_D,long double insulation_H_top,long double insulation_H_Bottom,long double cylinder_H_install,long double runner_offset);//���룺��������
	
	int CreateManifold();
	void Ear();
	void CreateBolt();
	void CreateBolt_Spacer(int index,NXString path_spacer,Point3d point_spacer,long double move_spacer,string code,string remark);
	void CreateBolt_Washer(int index,Point3d arc_p,string bolt_spec);
	void CreateRunner();
	
	void CreateGate();
	void SubManifold();
	
	long double GetArea();
	void GetMaxXY(long double* X,long double* Y);
	int CheckBlockAndLayer();
	static bool LineSort(const LineStruct &v1, const LineStruct &v2);
public:
	std::vector<LayerCurve> all_curves;//����ͼ��Ԫ��
	NXObject	*g_extrude;//����������
	
	std::vector<NXObject *>deleNXObjects();
	string file_name,host_name;
	string ZF;
	string SR;
	long double H;//��������
	long double D;//ϵ��
	long double D_runner;//����ֱ��
	long double H_top;
	long double H_Bottom;
	long double H_cylinder;//����
	long double H_runner_offset;
	long double perimeter;//�������ܳ�
};