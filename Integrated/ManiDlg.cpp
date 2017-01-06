#include "ManiDlg.hpp"
#include "Manifold.hpp"
#include "Heater.hpp"
#include "InletBushing.hpp"
#include "Nozzle.hpp"
#include "RunnerInsert.hpp"
#include "WireFrame.h"
#include "D:\\0_CAD\\UG\\0_Project\\BHRT\\Code\\NXFunction.hpp"
#include <uf_disp.h>
#include <time.h>
using namespace NXOpen;
using namespace NXOpen::BlockStyler;

#define CreateDialogParamW CreateDialog
#define CreateDialogW CreateDialog

Session *(ManiDlg::theSession) = NULL;
UI *(ManiDlg::theUI) = NULL;

ManiDlg::ManiDlg()
{
    try
    {
        // Initialize the NX Open C++ API environment
        ManiDlg::theSession = NXOpen::Session::GetSession();
        ManiDlg::theUI = UI::GetUI();
        theDlxFileName = "ManiDlg.dlx";
        theDialog = ManiDlg::theUI->CreateDialog(theDlxFileName);
        // Registration of callback functions
        theDialog->AddApplyHandler(make_callback(this, &ManiDlg::apply_cb));
        theDialog->AddOkHandler(make_callback(this, &ManiDlg::ok_cb));
        theDialog->AddUpdateHandler(make_callback(this, &ManiDlg::update_cb));
        theDialog->AddCancelHandler(make_callback(this, &ManiDlg::cancel_cb));
        theDialog->AddInitializeHandler(make_callback(this, &ManiDlg::initialize_cb));
        theDialog->AddDialogShownHandler(make_callback(this, &ManiDlg::dialogShown_cb));
    }
    catch(exception& ex)
    {
        throw;
    }
}

ManiDlg::~ManiDlg()
{
    if (theDialog != NULL)
    {
        delete theDialog;
        theDialog = NULL;
    }
}

int ManiDlg::Show()
{
    try
    {
        theDialog->Show();
    }
    catch(exception& ex)
    {
        //---- Enter your exception handling code here -----
        ManiDlg::theUI->NXMessageBox()->Show("Block Styler", NXOpen::NXMessageBox::DialogTypeError, ex.what());
    }
    return 0;
}

void ManiDlg::initialize_cb()
{
    try
    {
		eZF = dynamic_cast<NXOpen::BlockStyler::Enumeration*>(theDialog->TopBlock()->FindBlock("eZF"));
		sSR = dynamic_cast<NXOpen::BlockStyler::StringBlock*>(theDialog->TopBlock()->FindBlock("sSR"));
		dRunnerD = dynamic_cast<NXOpen::BlockStyler::DoubleBlock*>(theDialog->TopBlock()->FindBlock("dRunnerD"));
        group = dynamic_cast<NXOpen::BlockStyler::Group*>(theDialog->TopBlock()->FindBlock("group"));
        dMH = dynamic_cast<NXOpen::BlockStyler::DoubleBlock*>(theDialog->TopBlock()->FindBlock("dMH"));
		dOffset = dynamic_cast<NXOpen::BlockStyler::DoubleBlock*>(theDialog->TopBlock()->FindBlock("dOffset"));
        group1 = dynamic_cast<NXOpen::BlockStyler::Group*>(theDialog->TopBlock()->FindBlock("group1"));
        dCH = dynamic_cast<NXOpen::BlockStyler::DoubleBlock*>(theDialog->TopBlock()->FindBlock("dCH"));
		sTin = dynamic_cast<NXOpen::BlockStyler::StringBlock*>(theDialog->TopBlock()->FindBlock("sTin"));
        sBin = dynamic_cast<NXOpen::BlockStyler::StringBlock*>(theDialog->TopBlock()->FindBlock("sBin"));
    }
    catch(exception& ex)
    {
        ManiDlg::theUI->NXMessageBox()->Show("Block Styler", NXOpen::NXMessageBox::DialogTypeError, ex.what());
    }
}

void ManiDlg::dialogShown_cb()
{
    try
    {
        //---- Enter your callback code here -----
    }
    catch(exception& ex)
    {
        //---- Enter your exception handling code here -----
        ManiDlg::theUI->NXMessageBox()->Show("Block Styler", NXOpen::NXMessageBox::DialogTypeError, ex.what());
    }
}

int ManiDlg::apply_cb()
{
	if(UF_initialize()!=0) return 0;
	int errorCode = 0;
    try
    {
		if_ok=1;
		manifold_H=this->dMH->Value();
		runner_D=this->dRunnerD->Value();
		runner_offset=this->dOffset->Value();
		cylinder_H_change=this->dCH->Value();
		insulation_H_top=std::atof(this->sTin->Value().GetLocaleText());
		insulation_H_Bottom=std::atof(this->sBin->Value().GetLocaleText());
		ZF = this->eZF->ValueAsString().GetLocaleText();
		SR = this->sSR->Value().GetLocaleText();
		
		//int ifail = UF_DISP_set_display(UF_DISP_SUPPRESS_DISPLAY);

		time_t start,stop;
		start = time(NULL);
    
		string str_ZF = this->ZF;
		string str_SR = this->SR;
		double manifold_H           =this->manifold_H;		
		double panel_H              =this->cylinder_H_change;
		double insulation_H_top     =this->insulation_H_top;		
		double insulation_H_Bottom  =this->insulation_H_Bottom;
		double runner_offset        =this->runner_offset;
		double runner_D             =this->runner_D;

		//����40Ϊ����ͼ�㣬����40 42���������ͼ��
		NXFunction::SetLayers();
		bool is_rebulid = NXFunction::CheckBodyExist("MANIFOLD");

		/*
			�ڶ��ε㡰һ�廯��ơ���ť�����Ϊ��
			1. ������ʵ�壺�������� ���Ը��Ƶ��µ�ʵ���� 
				1.1 ������ ����
				1.2 ����   ����
				1.3 �̶���˿ ����
			2. ��������壺ɾ����ǰ�ģ��������� 
			3. �����ߣ��߲ۣ�ɾ����ǰ�ķ��ȹܣ��߲ۿ϶�û�ˣ���Ϊ�����˷����壩
			4. �����죬���Ķ�λ��������
			5. �����������ŷ��飺 ����
			6. ��λ�������Ƚ��ӣ���������
			7. ���죺����
			8. ���ף�����
			9. ����������߼ܣ�����
		*/

		manifold = new Manifold();
		/*int int_manifold_size = manifold->CheckBlockAndLayer();
		if  (int_manifold_size==0) return 0;*/
		
		int int_manifold_size = manifold->Create(str_ZF,
											str_SR,
											manifold_H,
											runner_D,
											insulation_H_top,
											insulation_H_Bottom,
											panel_H,
											runner_offset);
		

		heater = new Heater();
		heater->CreateHeater(manifold_H);

		inletbushing = new InletBushing();
		inletbushing->Create(manifold_H,
							insulation_H_top,
							insulation_H_Bottom,
							int_manifold_size,
							panel_H,
							is_rebulid);

		nozzle = new Nozzle(manifold_H,panel_H,insulation_H_top,is_rebulid);
		
		wire_frame = new WireFrame(manifold_H);
		wire_frame->CreateWireFrame();
		
		manifold->CreateRunner();

		std::vector<Body*> bodies = NXFunction::GetBodiesByName("SUB_");
		NXFunction::MoveBodies2Layer(42,bodies);

		//ifail = UF_DISP_set_display(UF_DISP_UNSUPPRESS_DISPLAY);
		//ifail = UF_DISP_regenerate_display();
		stop = time(NULL);
		string str = std::to_string((long double)stop-start);
		//uc1601(const_cast<char*>(str.c_str()),1);

    }
    catch(exception& ex)
    {
        errorCode = 1;
        ManiDlg::theUI->NXMessageBox()->Show("Block Styler", NXOpen::NXMessageBox::DialogTypeError, ex.what());
    }
	UF_terminate();
    return errorCode;
}

int ManiDlg::update_cb(NXOpen::BlockStyler::UIBlock* block)
{
    try
    {
        if(block == dMH)
        {
        //---------Enter your code here-----------
        }
        else if(block == dCH)
        {
        //---------Enter your code here-----------
        }
		else if(block == sTin)
        {
        //---------Enter your code here-----------
        }
        else if(block == sBin)
        {
        //---------Enter your code here-----------
        }
    }
    catch(exception& ex)
    {
        //---- Enter your exception handling code here -----
        ManiDlg::theUI->NXMessageBox()->Show("Block Styler", NXOpen::NXMessageBox::DialogTypeError, ex.what());
    }
    return 0;
}

int ManiDlg::ok_cb()
{
    int errorCode = 0;
    try
    {
        errorCode = apply_cb();
    }
    catch(exception& ex)
    {
        errorCode = 1;
        ManiDlg::theUI->NXMessageBox()->Show("Block Styler", NXOpen::NXMessageBox::DialogTypeError, ex.what());
    }
    return errorCode;
}

int ManiDlg::cancel_cb()
{
    try
    {
		if_ok=0;
    }
    catch(exception& ex)
    {
        ManiDlg::theUI->NXMessageBox()->Show("Block Styler", NXOpen::NXMessageBox::DialogTypeError, ex.what());
    }
    return 0;
}