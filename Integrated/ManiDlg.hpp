#ifndef MANIDLG_H_INCLUDED
#define MANIDLG_H_INCLUDED

#include <uf_defs.h>
#include <uf_ui_types.h>
#include <iostream>
#include <NXOpen/Session.hxx>
#include <NXOpen/UI.hxx>
#include <NXOpen/NXMessageBox.hxx>
#include <NXOpen/Callback.hxx>
#include <NXOpen/NXException.hxx>
#include <NXOpen/BlockStyler_UIBlock.hxx>
#include <NXOpen/BlockStyler_BlockDialog.hxx>
#include <NXOpen/BlockStyler_PropertyList.hxx>
#include <NXOpen/BlockStyler_Group.hxx>
#include <NXOpen/BlockStyler_DoubleBlock.hxx>
#include <NXOpen\BlockStyler_StringBlock.hxx>
#include <NXOpen/BlockStyler_Enumeration.hxx>

using namespace std;
using namespace NXOpen;
using namespace NXOpen::BlockStyler;

class Manifold;
class Heater;
class InletBushing;
class Nozzle;
class RunnerInsert;
class WireFrame;

class DllExport ManiDlg
{
    
public:
    static Session *theSession;
    static UI *theUI;
    ManiDlg();
    ~ManiDlg();
    int Show();

	Manifold *manifold;
	Heater *heater;
	InletBushing *inletbushing;
	Nozzle *nozzle;
	RunnerInsert *runner_insert;
	WireFrame *wire_frame;
    //------------------------------------------------------------------------------
    void initialize_cb();
    void dialogShown_cb();
    int apply_cb();
    int ok_cb();
    int cancel_cb();
    int update_cb(NXOpen::BlockStyler::UIBlock* block);
    
	long double manifold_H;
	long double cylinder_H_change;
	long double insulation_H_top;
	long double insulation_H_Bottom;
	long double runner_offset;//流道偏置 Z方向
	long double runner_D;//流道直径
	string ZF;
	string SR;
	int if_ok;

private:
    const char* theDlxFileName;
    NXOpen::BlockStyler::BlockDialog* theDialog;
    NXOpen::BlockStyler::Group* group;// Block type: Group
	NXOpen::BlockStyler::StringBlock* sSR;// Block type: String
	NXOpen::BlockStyler::Enumeration* eZF;// Block type: Enumeration
    NXOpen::BlockStyler::DoubleBlock* dMH;// Block type: Double
	NXOpen::BlockStyler::DoubleBlock* dOffset;// Block type: Double
    NXOpen::BlockStyler::Group* group1;// Block type: Group
    NXOpen::BlockStyler::DoubleBlock* dCH;// Block type: Double
    NXOpen::BlockStyler::StringBlock* sTin;// Block type: String
    NXOpen::BlockStyler::StringBlock* sBin;// Block type: String
	NXOpen::BlockStyler::DoubleBlock* dRunnerD;// Block type: Double
};
#endif //MANIDLG_H_INCLUDED
