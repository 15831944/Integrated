//2015-08-24 07:34
#include <uf_defs.h>
#include <uf_ui_types.h>
#include <iostream>
#include <uf_group.h>
#include <uf_eval.h>
#include <NXOpen/Session.hxx>
#include <NXOpen/UI.hxx>


using namespace std;
using namespace NXOpen;

struct Runner;
class Block;
class ADOConn;

class Nozzle
{
public://函数
	
	Nozzle(long double manifold_H,long double panel_H,long double H_top,bool is_rebulid);
	virtual ~Nozzle();

	void GetRunners();
	void ImportNozzle(Block block);
	void ImportCylinder(Block nozzle_block);
	void ImportCylinderWithoutNozzle(Block cylinder_block);
	
	void ImportSplitCylinder(Block block);
	Vector3d GetRunnerDir(Block block);

	void ImportManifoldCylinder(Block block);

	void GasLine();


public:
	ADOConn *adoconn;

	std::vector<Runner> runners;

	int g_type; //1——气缸 2——油缸
	string g_install_type;//气缸的安装方式
	long double g_manifold_H,g_panel_H,g_H_top;
	long double g_thermal_expansion_coefficient;
	NXString str_BHRT;
	NXString path_part_base;
};