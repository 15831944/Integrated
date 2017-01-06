//2015-05-27 11:00 主射嘴 中心定位柱 隔热介子 定位销 热电偶
#include "InletBushing.hpp"
#include "D:\\0_CAD\\UG\\0_Project\\BHRT\\Code\\ADOConn.hpp"
#include "D:\\0_CAD\\UG\\0_Project\\BHRT\\Code\\NXFunction.hpp"
using namespace std;
using namespace NXOpen;

InletBushing::InletBushing()
{
	string file_name=NXFunction::GetFileName();
	string host_name=NXFunction::GetHostName();
	adoconn = new ADOConn(host_name,file_name);
	adoconn->OnInitADOConn();
}
InletBushing::~InletBushing()
{
	adoconn->ExitConnect();
	delete adoconn;
}
//主函数————————————————————————————————————————————
void InletBushing::Create(long double manifold_H,long double insulation_H_top,long double insulation_H_Bottom,int int_manifold_size,long double panel_H,bool is_rebulid)
{
	g_int_size=int_manifold_size;
	H=manifold_H;
	H_top=insulation_H_top;
	H_Bottom=insulation_H_Bottom;
	H_panel = panel_H;

	ImportInletBushing();
	ImportCentrePin();
	ImportDowelPins();
	ImportInsulators2();
	ImportTC();
	if(is_rebulid==false) ImportEV();
	if(is_rebulid==false) ImportOthers();
	//NXFunction::RemoveParameters();
}
//作 用：导入主射嘴
void InletBushing::ImportInletBushing()
{
	//1——获得图块数量与初始化
	NXString path_dll=NXFunction::GetTranslateVariable();
	std::vector<Block> blocks = adoconn->GetBlocksByAssemblyName("主射嘴");
	if (blocks.size()==0)return;

	NXFunction::DeleteParts("INLETBUSHING",0);
	NXFunction::DeleteParts("SUB_INLETBUSHING",0);
	NXFunction::DeleteParts("SUB_SCREW",0);

	Session *theSession = Session::GetSession();
    Part *workPart(theSession->Parts()->Work());
	//2——遍历
	for(unsigned int i=0;i<blocks.size();i++)
	{
		//2.1——读取数据
		NXString path_inletbushing=blocks[i].file_path;
		Point3d point_inletbushing(blocks[i].X,blocks[i].Y,0);
		//2.2——检查文件是否存在
		if(NXFunction::CheckFileExist(path_inletbushing,"主射嘴")==false) return;
		//2.3——插入
		NXFunction::ImportAndRoation(path_inletbushing,point_inletbushing,blocks[i].Angle,1);
		string namePart = "SUB_SCREW";
		NXFunction::RotatingPart(-blocks[i].Angle,point_inletbushing,namePart,1);
		//2.4——插入属性 assembly_id=[assembly_id]
		string body_name="INLETBUSHING";
		NXFunction::SetAttribute(body_name,"基本参数","assembly_id",blocks[i].assembly_id);
		//2.6——编号
		NXFunction::NumberingPart(blocks[i].index,"INLETBUSHING",1);
		NXFunction::NumberingPart(blocks[i].index,"SUB_INLETBUSHING",1);//TODO 给真体编号
		NXFunction::NumberingPart(blocks[i].index,"SUB_SCREW",1);
	}
}
//作 用：导入中心定位柱
void InletBushing::ImportCentrePin()
{
	//1——获得图块数量与初始化
	NXString path_dll=NXFunction::GetTranslateVariable();
	std::vector<Block> blocks = adoconn->GetBlocksByAssemblyName("中心定位柱");
	if( blocks.size()==0 )return;
	Session *theSession = Session::GetSession();
    Part *workPart(theSession->Parts()->Work());
	//2——遍历
	for(unsigned int i=0;i<blocks.size();i++)
	{
		//2.1——读取数据
		NXString path_centrepin=blocks[i].file_path;
		Point3d point_centrepin(blocks[i].X,blocks[i].Y,-H);
		//2.2——检查
		if(NXFunction::CheckFileExist(path_centrepin,"中心定位柱")==false) return;
		//2.3——插入
		NXFunction::ImportPart(path_centrepin,point_centrepin,1);
		//2.4——属性
		string body_name="CENTRE_PIN";
		NXFunction::SetAttribute(body_name,"基本参数","assembly_id",blocks[i].assembly_id);
		//2.5——编号
		NXFunction::NumberingPart(blocks[i].index,"CENTRE_PIN",1);
		NXFunction::NumberingPart(blocks[i].index,"SUB_CENTRE_PIN1",1);//
	}
}
//作 用：导入定位销
void InletBushing::ImportDowelPins()
{
	//1——获得图块数量与初始化
	NXString path_dll=NXFunction::GetTranslateVariable();
	std::vector<Block> blocks = adoconn->GetBlocksByAssemblyName("定位销");
	if( blocks.size()==0 )return;

	NXFunction::DeleteParts("DOWELPIN",0);
	NXFunction::DeleteParts("SUB_DOWELPIN1",0);
	NXFunction::DeleteParts("SUB_DOWELPIN2",0);

	Session *theSession = Session::GetSession();
    Part *workPart(theSession->Parts()->Work());
	//2——遍历
	for(unsigned int i=0;i<blocks.size();i++)
	{
		//2.1——读取数据
		NXString path_dowelpin=blocks[i].file_path;
		Point3d point_dowelpin(blocks[i].X,blocks[i].Y,-H);//TODO??
		//2.2——检查
		if(NXFunction::CheckFileExist(path_dowelpin,"定位销")==false) return;
		//2.3——插入
		NXFunction::ImportAndRoation(path_dowelpin,point_dowelpin,blocks[i].Angle,1);
		string body_name="DOWELPIN";
		NXFunction::SetAttribute(body_name,"基本参数","assembly_id",blocks[i].assembly_id);
		
		NXFunction::NumberingPart2(blocks[i].index,"DOWELPIN",1);
		NXFunction::NumberingPart2(blocks[i].index,"SUB_DOWELPIN1",1);
		NXFunction::NumberingPart2(blocks[i].index,"SUB_DOWELPIN2",1);
	}
}

void InletBushing::ImportInsulators2()
{
	Session *theSession = Session::GetSession();
    Part *workPart(theSession->Parts()->Work());
	NXFunction::DeleteParts("INSULATION",0);
	NXFunction::DeleteParts("SUB_INSULATION",0);
	//2——获得blocks
	std::vector<Block> blocks_both = adoconn->GetBlocksByPrefixID("insulator_both");
	std::vector<Block> blocks_top = adoconn->GetBlocksByPrefixID("insulator_top");
	std::vector<Block> blocks_bottom = adoconn->GetBlocksByPrefixID("insulator_bottom");
	for(unsigned int i = 0;i<blocks_both.size();i++)
	{
		blocks_top.push_back(blocks_both[i]);
		blocks_bottom.push_back(blocks_both[i]);
	}
	
	//3——插入TOP
	InsertTop(blocks_top);	

	//4——插入BOTTOM
	InsertBottom(blocks_bottom);
	
}

void InletBushing::InsertTop(std::vector<Block> blocks)
{
	if(blocks.size()==0) return;

	string strBHRT=NXFunction::GetTranslateVariable().GetLocaleText();
	string path_part_base=strBHRT+"\\Part_Base\\Heat_Insulation_Ring\\";

	string strTop = GetStrTop().GetLocaleText();
	string path_insulation_top = "";

	//0——获得直径
	string strDiameter = "19";
	string name_top="";
	double diameter = 19;
	try
	{
		string id =  blocks[0].assembly_id;
		strDiameter = id.substr(id.find_last_of('_')+1,2);
		diameter = std::atof(strDiameter.c_str());
	}catch(exception &ex)
	{
		strDiameter = "19";
	}

	name_top = strDiameter+"X"+strTop;
	int standard_top = 1;
	//1——判断非标，获得路径
	if(H_top==5 || H_top==10 ||H_top==15 ||H_top==25 ||H_top==35)
	{
		standard_top = 1;
		path_insulation_top=path_part_base+name_top+".prt";
	}
	else
	{
		standard_top = 0;
		path_insulation_top=path_part_base+"insulator-"+strDiameter+".prt";
	}
	
	Point3d point0(blocks[0].X,blocks[0].Y,0);
	NXFunction::ImportPart(path_insulation_top,point0,1);			
	NXFunction::NumberingPart2(0,"INSULATION",1);
	NXFunction::NumberingPart2(0,"SUB_INSULATION",1);		
	string body_name="INSULATION-0";
	string sub_body_name = "SUB_INSULATION-0";
	Body *sub_body = NXFunction::GetBodyByName(body_name);
	Body *body = NXFunction::GetBodyByName(sub_body_name);

	NXFunction::SetAttribute(body_name,"基本参数","assembly_id",name_top);
	//非标的移动面
	if(standard_top!=1)
	{
		NXFunction::MoveFaceByFeature2("MOVE_INSULATION-0",body_name,10-H_top);
		if(H_top>35)
		{
			string h = std::to_string((long double)floor(H_top+5));
			string spec = "φ"+std::to_string((long double)(diameter+1))+"X"+h;
			NXFunction::SetAttribute(body_name,"基本参数","规格型号",spec);
			NXFunction::SetAttribute(body_name,"基本参数","备注","下料,实际高度="+std::to_string(H_top));
		}
		else
		{
			NXFunction::SetAttribute(body_name,"基本参数","备注",name_top+"二次加工,实际高度="+std::to_string(H_top));
		}	
	}		
			
	for(unsigned int i = 1;i<blocks.size();i++)
	{
		Point3d point_insulator(blocks[i].X,blocks[i].Y,0);
		NXFunction::CopyBodyP2P(body,point0,point_insulator,i);
		NXFunction::CopyBodyP2P(sub_body,point0,point_insulator,i);
	}
}
void InletBushing::InsertBottom(std::vector<Block> blocks)
{
	if(blocks.size()==0) return;

	string strBHRT=NXFunction::GetTranslateVariable().GetLocaleText();
	string path_part_base=strBHRT+"\\Part_Base\\Heat_Insulation_Ring\\";

	string strBottom = GetStrBottom().GetLocaleText();
	string path_insulation_bottom = "";

	//0——获得直径
	string strDiameter = "19";
	double diameter = 19;
	string name_bottom="";
	try
	{
		string id =  blocks[0].assembly_id;
		strDiameter = id.substr(id.find_last_of('_')+1,2);
		diameter = std::atof(strDiameter.c_str());
	}catch(exception &ex)
	{
		strDiameter = "19";
	}

	name_bottom = strDiameter+"X"+strBottom;
	int standard_bottom = 1;
	//1——判断非标，获得路径
	if(H_Bottom==5 || H_Bottom==10 ||H_Bottom==15 ||H_Bottom==25 ||H_Bottom==35)
	{
		standard_bottom = 1;
		path_insulation_bottom=path_part_base+name_bottom+".prt";
	}
	else
	{
		standard_bottom = 0;
		path_insulation_bottom=path_part_base+"insulator-"+strDiameter+".prt";
	}
	
	Point3d point0(blocks[0].X,blocks[0].Y,-H);
	NXFunction::ImportPart(path_insulation_bottom,point0,-1);			
	NXFunction::NumberingPart2(1,"INSULATION",1);
	NXFunction::NumberingPart2(1,"SUB_INSULATION",1);		
	string body_name="INSULATION-1";
	string sub_body_name = "SUB_INSULATION-1";
	Body *sub_body = NXFunction::GetBodyByName(body_name);
	Body *body = NXFunction::GetBodyByName(sub_body_name);

	NXFunction::SetAttribute(body_name,"基本参数","assembly_id",name_bottom);
	//非标的移动面
	if(standard_bottom!=1)
	{
		NXFunction::MoveFaceByFeature2("MOVE_INSULATION-1",body_name,10-H_Bottom);
		if(H_Bottom>35)
		{
			string h = std::to_string((long double)floor(H_Bottom+5));
			string spec = "φ"+std::to_string((long double)(diameter+1))+"X"+h;
			NXFunction::SetAttribute(body_name,"基本参数","规格型号",spec);
			NXFunction::SetAttribute(body_name,"基本参数","备注","下料,实际高度="+std::to_string(H_Bottom));
		}
		else
		{
			NXFunction::SetAttribute(body_name,"基本参数","备注","二次加工,实际高度="+std::to_string(H_Bottom));
		}	
	}		
			
	for(unsigned int i = 1;i<blocks.size();i++)
	{
		Point3d point_insulator(blocks[i].X,blocks[i].Y,-H);
		NXFunction::CopyBodyP2P(body,point0,point_insulator,i);
		NXFunction::CopyBodyP2P(sub_body,point0,point_insulator,i);
	}

}

NXString InletBushing::GetStrTop()
{
	NXString name_top = "35H";

	if(H_top<=5)      name_top="5H";
	else if(H_top<=10 && H_top>5) name_top="10H";
	else if(H_top<=15 && H_top>10) name_top="15H";
	else if(H_top<=25 && H_top>15) name_top="25H";
	else if(H_top<=35 && H_top>25) name_top="35H";

	return name_top;
}
NXString InletBushing::GetStrBottom()
{
	NXString name_bottom="35H";

	if(H_Bottom<=5)      name_bottom="5H";
	else if(H_Bottom<=10 && H_Bottom>5) name_bottom="10H";
	else if(H_Bottom<=15 && H_Bottom>10) name_bottom="15H";
	else if(H_Bottom<=25 && H_Bottom>15) name_bottom="25H";
	else if(H_Bottom<=35 && H_Bottom>25) name_bottom="35H";
	
	return name_bottom;
}
//作 用：导入热电偶 
void InletBushing::ImportTC()
{
	//1——获得图块数量与初始化
	NXString path_dll=NXFunction::GetTranslateVariable();
	std::vector<Block> blocks = adoconn->GetBlocksByAssemblyName("热电偶");
	if( blocks.size()==0 )return;
	Session *theSession = Session::GetSession();
    Part *workPart(theSession->Parts()->Work());
	//2——遍历
	for(unsigned int i=0;i<blocks.size();i++)
	{
		//2.1——读取数据
		NXString path_tc=blocks[i].file_path;
		Point3d point_tc(blocks[i].X,blocks[i].Y,0);
		//2.2——检查
		if(NXFunction::CheckFileExist(path_tc,"热电偶")==false) return;
		//2.3——插入
		NXFunction::ImportAndRoation(path_tc,point_tc,blocks[i].Angle,1);
		string body_name="TC";
		NXFunction::SetAttribute(body_name,"基本参数","assembly_id",blocks[i].assembly_id);
		//2.6——编号
		NXFunction::NumberingPart(blocks[i].index,"TC",1);
		//NXFunction::NumberingPart(blocks[i].index,"TC_M",1);
		NXFunction::NumberingPart(blocks[i].index,"SUB_TC",1);
	}
}
//作 用：导入电磁阀
void InletBushing::ImportEV()
{
	//1——获得图块数量与初始化
	NXString path_dll=NXFunction::GetTranslateVariable();
	std::vector<Block> blocks = adoconn->GetBlocksByAssemblyName("电磁阀阀组");
	if( blocks.size()==0 )return;
	Session *theSession = Session::GetSession();
    Part *workPart(theSession->Parts()->Work());
	//2——遍历
	for(unsigned int i=0;i<blocks.size();i++)
	{
		//2.1——读取数据
		NXString path_tc=blocks[i].file_path;
		Point3d point_tc(blocks[i].X,blocks[i].Y,H_top+H_panel-25);
		//2.2——检查
		if(NXFunction::CheckFileExist(path_tc,"电磁阀阀组")==false) return;
		//2.3——插入
		NXFunction::ImportAndRoation(path_tc,point_tc,blocks[i].Angle,1);
	}
}
//作 用：导入其他图块中的3D
void InletBushing::ImportOthers()
{
	//1——获得图块数量与初始化
	NXString path_dll=NXFunction::GetTranslateVariable();
	std::vector<Block> blocks = adoconn->GetBlocksOthers();
	if( blocks.size()==0 )return;

	Session *theSession = Session::GetSession();
    Part *workPart(theSession->Parts()->Work());
	//2——遍历
	for(unsigned int i=0;i<blocks.size();i++)
	{
		//2.1——读取数据
		NXString path_tc=blocks[i].file_path;
		Point3d point_tc(blocks[i].X,blocks[i].Y,0);
		//2.2——检查
		if(NXFunction::CheckFileExist(path_tc,"")==false) return;
		//2.3——插入
		NXFunction::ImportAndRoation(path_tc,point_tc,blocks[i].Angle,1);
		string body_name=blocks[i].assembly_id;
		NXFunction::SetAttribute(body_name,"基本参数","assembly_id",blocks[i].assembly_id);
	
		NXFunction::NumberingPart2(blocks[i].index,body_name,1);
	}
}
