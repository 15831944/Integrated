#include <uf_defs.h>
#include <uf_ui_types.h>
#include <iostream>
#include <NXOpen/Session.hxx>
#include <NXOpen/UI.hxx>
#include <NXOpen/CurveCollection.hxx>

using namespace std;
using namespace NXOpen;

struct Components;
struct RunnerPoint;
struct Runner;
class ADOConn;

class RunnerInsert
{
public:
	RunnerInsert();
	virtual ~RunnerInsert();

	void Main(long double manifold_H,long double runner_offset,long double runner_D);
	Point3d GetOriginPoint();
	int GetNozzlePoints();
	int GetAllPoints();
	int FilterPoints();
	Point3d FindNearestNozzlePoint(RunnerPoint insert_point);
public:
	long double H;//分流板厚度
	long double D;//流道直径
	long double H_offset;
	NXString str_BHRT;
	ADOConn *adoconn;
	vector<Runner>  g_runners;
	vector<RunnerPoint>  g_runner_points;
	vector<RunnerPoint>  g_insert_points;
	vector<Point3d> g_nozzle_points;
};