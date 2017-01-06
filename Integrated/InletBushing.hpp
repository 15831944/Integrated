//2016-04-22 12:00
#include <uf.h>
#include <uf_ui.h>
#include <iostream>

#include <NXOpen/Session.hxx>
#include <NXOpen/UI.hxx>

using namespace std;
using namespace NXOpen;

class ADOConn;
class Block;

class InletBushing
{
public:
	InletBushing();
	virtual ~InletBushing();
	
	void Create(long double manifold_H,long double insulation_H_top,long double insulation_H_Bottom,int int_manifold_size,long double panel_H,bool is_rebulid);
		
	void ImportInletBushing();
	void ImportCentrePin();
	void ImportDowelPins();
	void ImportInsulators();
	void ImportInsulators2();
	NXString GetStrTop();
	NXString GetStrBottom();
	void InsertTop(std::vector<Block> blocks);
	void InsertBottom(std::vector<Block> blocks);
	void InsertBottom();
	void ImportTC();
	void ImportEV();
	void ImportOthers();

public:
	ADOConn *adoconn;
	/*NXString path_insulation_top;
	NXString path_insulation_bottom;*/

	int g_int_size;
	long double H;
	long double H_top;
	long double H_Bottom;
	long double H_panel;
	
	NXString g_manifold,g_subscrew,g_subnozzle;
	NXString g_centrepin,g_subcentrepin1,g_subcentrepin2,g_subdowelpin;
};