//2015-08-24 07:34
#include "Nozzle.hpp"
#include "D:\\0_CAD\\UG\\0_Project\\BHRT\\Code\\ADOConn.hpp"
#include "D:\\0_CAD\\UG\\0_Project\\BHRT\\Code\\NXFunction.hpp"
using namespace std;
using namespace NXOpen;

Nozzle::Nozzle(long double manifold_H,long double panel_H,long double H_top,bool is_rebulid)
{	
	//ɾ����������
	NXFunction::DeleteParts("CYLINDER",0);
	NXFunction::DeleteParts("SUB_CYLINDER",0);
	NXFunction::DeleteParts("FIXED_BLOCK",0);
	NXFunction::DeleteParts("SUB_FIXED_BLOCK",0);
	NXFunction::DeleteParts("VALVE2",0);
	NXFunction::DeleteParts("GASLINE",0);
	//��ʼ��
	string file_name=NXFunction::GetFileName();
	string host_name=NXFunction::GetHostName();
	adoconn = new ADOConn(host_name,file_name);
	adoconn->OnInitADOConn();

	str_BHRT=NXFunction::GetTranslateVariable();
	path_part_base=str_BHRT+"\\Part_Base\\";

	g_type = 1;//����
	g_install_type="����ʽ";
	g_thermal_expansion_coefficient = 0.0024;
	g_manifold_H = manifold_H;
	g_panel_H = panel_H;
	g_H_top = H_top;

	Session *theSession = Session::GetSession();
	Part *workPart(theSession->Parts()->Work());

	//0�����õ���������
	GetRunners();
	
	//1��������ͼ��
	std::vector<Block> nozzle_blocks = adoconn->GetNozzleBlocks();
	if(nozzle_blocks.size()==0) 
	{
		std::vector<Block> nozzle_cylinders = adoconn->GetCylinderBlocks();
		//2A������������ �͸�
		for(unsigned int i=0;i<nozzle_cylinders.size();i++)
		{
			try
			{
				ImportCylinder(nozzle_cylinders[i]); 
			} catch (exception &ex)
			{
				string str = ex.what();
				uc1601(const_cast<char*>(str.c_str()),1);
			}
		}
	}
	else  
	{
		//2B����������������� �͸�
		for(unsigned int i=0;i<nozzle_blocks.size();i++)
		{
			try{
				if(!is_rebulid) ImportNozzle(nozzle_blocks[i]); 
				Block cylinder_block = adoconn->GetCylinderBlock(nozzle_blocks[i]);
				ImportCylinderWithoutNozzle(cylinder_block); 
			} catch (exception &ex)
			{
				string str = ex.what();
				uc1601(const_cast<char*>(str.c_str()),1);
			}
		}
	}	

	//3������·��·
	GasLine();

	//4�������ؼ���
	std::vector<Body*> sub_bodies = NXFunction::GetBodiesByName("SUB_NOZZLE2-");
	NXFunction::HideBodies(sub_bodies,1);

	//���ղ���
	string name_part = "MANIFOLD";
	NXFunction::SetAttribute(name_part,"���ղ���","����",std::to_string((long double)(nozzle_blocks.size())));
}
Nozzle::~Nozzle()
{
	adoconn->ExitConnect();
	delete adoconn;
}

void Nozzle::GetRunners()
{
	Session *theSession = Session::GetSession();
	Part *workPart(theSession->Parts()->Work());
	
	runners.clear();
	
	//0��������ͼ����
	int runner_layer=NXFunction::GetLayerNum("2runner");
	if(runner_layer==-1) return ;

	Layer::LayerManager *layer_manager=workPart->Layers();
	std::vector<NXObject *>objects1=layer_manager->GetAllObjectsOnLayer(runner_layer);	
	int runner_count=objects1.size();
	if(runner_count==0) return ;

	//1�����õ�vector_runners ��vector_runner_points
	for(int i=0;i<runner_count;i++)
	{
		if(!NXFunction::IsCurve(objects1[i],3)) continue;

		Line *line1(dynamic_cast<Line *>(objects1[i]));		
		Direction *direction1;
		direction1 = workPart->Directions()->CreateDirection(line1,SenseForward, SmartObject::UpdateOptionWithinModeling); 
		Vector3d vector3d1=direction1->Vector();		
		Point3d start_point=line1->StartPoint();
		Point3d end_point=line1->EndPoint();
		
		Runner runner;
		runner.direction=vector3d1;
		runner.start_point=start_point;
		runner.end_point=end_point;
		runner.runner_index=i;

		runners.push_back(runner);
	}
}

void Nozzle::ImportNozzle(Block block)
{
	try
	{
		NXString path_nozzle=block.file_path;
		Point3d point_nozzle(block.X,block.Y,-g_manifold_H);
		//����
		NXFunction::ImportAndRoation(path_nozzle,point_nozzle,block.Angle,1);
		//���
		NXFunction::NumberingPart(block.index,"NOZZLE",1,point_nozzle,block.Angle,"NaN");
		NXFunction::NumberingPart2(block.index,"SUB_NOZZLE1",1);
		NXFunction::NumberingPart2(block.index,"SUB_NOZZLE2",1);
	} catch (exception &ex)
	{
		string str = ex.what();
		str = "��" + std::to_string((long double)block.index) + "�����׳���:" +str;
		uc1601(const_cast<char*>(str.c_str()),1);
	}
}

void Nozzle::ImportCylinder(Block nozzle_block)
{
	Session *theSession = Session::GetSession();
	Part *workPart(theSession->Parts()->Work());

	Block cylinder_block = adoconn->GetCylinderBlock(nozzle_block);
	if(cylinder_block.assembly_id=="") return;
	
	if(cylinder_block.assembly_name=="����ʽ����" || cylinder_block.assembly_name=="����ʽ����") 
		g_type = 1;
	else//�͸�
		g_type = 2;

	if(cylinder_block.assembly_name=="����ʽ����" || cylinder_block.assembly_name=="����ʽ�͸�") 
	{
		g_install_type = "����ʽ";
		ImportSplitCylinder(cylinder_block);
	}
	else
	{
		g_install_type = "����ʽ";	
		ImportManifoldCylinder(cylinder_block);
	}
	
}
void Nozzle::ImportCylinderWithoutNozzle(Block cylinder_block)
{
	Session *theSession = Session::GetSession();
	Part *workPart(theSession->Parts()->Work());

	if(cylinder_block.assembly_id=="") return;
	
	if(cylinder_block.assembly_name=="����ʽ����" || cylinder_block.assembly_name=="����ʽ����") 
		g_type = 1;
	else//�͸�
		g_type = 2;

	if(cylinder_block.assembly_name=="����ʽ����" || cylinder_block.assembly_name=="����ʽ�͸�") 
	{
		g_install_type = "����ʽ";
		ImportSplitCylinder(cylinder_block);
	}
	else
	{
		g_install_type = "����ʽ";	
		ImportManifoldCylinder(cylinder_block);
	}
}

//����ʽ����/�͸�
void Nozzle::ImportSplitCylinder(Block block)
{
	NXString path_cylinder=block.file_path;
	Point3d point_cylinder(block.X,block.Y,g_panel_H+g_H_top);
	
	//����
	NXFunction::ImportPart(path_cylinder,point_cylinder,1);
	
	//��ת�Ƕ�
	NXFunction::RotatingPart(block.Angle,point_cylinder,"CYLINDER",1);
	NXFunction::RotatingPart(block.Angle,point_cylinder,"SUB_CYLINDER",1);
	Vector3d dir_default(0,1,0);
	Vector3d dir_target = GetRunnerDir(block);
	Point3d p(block.X,block.Y,0);
	NXFunction::RotatingPart(dir_default,dir_target,p,"FIXED_BLOCK",1);
	NXFunction::RotatingPart(dir_default,dir_target,p,"SUB_FIXED_BLOCK",1);

	//�߶��ƶ��� �� �������ƶ�
	Body *body_cylinder = NXFunction::GetBodyByName("CYLINDER");
	if(!body_cylinder->HasUserAttribute("H",NXObject::AttributeTypeAny,-1))
		body_cylinder->SetUserAttribute("H",-1,"80",Update::OptionLater);
	NXString strH=body_cylinder->GetStringUserAttribute("H",-1);
	long double cylinder_Height=std::atof(strH.GetLocaleText());//����ԭ���ĸ߶�	

	long double X=g_thermal_expansion_coefficient*(-block.X);
	long double Y=g_thermal_expansion_coefficient*(-block.Y);	
	long double Z = cylinder_Height+15-g_panel_H-g_H_top;//15:3D PRT��Ĭ�ϸ��Ƚ��Ӹ߶�

	NXFunction::MoveBodyByIncrement("FIXED_BLOCK",X,Y,Z);
	NXFunction::MoveBodyByIncrement("SUB_FIXED_BLOCK",X,Y,Z);

	//����
	string body_name="CYLINDER";
	NXFunction::SetAttribute(body_name,"��������","assembly_id",block.assembly_id);
	NXFunction::SetAttribute(body_name,"��������","��װ��ʽ","����ʽ");
	NXFunction::SetAttribute(body_name,"��������","�������",block.assembly_name);

	//���
	NXFunction::NumberingPart(block.index,"CYLINDER",1,point_cylinder,block.Angle,"NaN");
	NXFunction::NumberingPart(block.index,"SUB_CYLINDER",1);
	NXFunction::NumberingPart(block.index,"FIXED_BLOCK",1);
	NXFunction::NumberingPart(block.index,"SUB_FIXED_BLOCK",1);
	NXFunction::NumberingPart(block.index,"VALVE2",1);	
}

//�������������ķ���
Vector3d Nozzle::GetRunnerDir(Block block)
{
	Vector3d dir(1,0,0);
	Point3d p(block.X,block.Y,0);
	for(unsigned int i=0;i<runners.size();i++)
	{		
		if(NXFunction::IsP1inP2P3(p,runners[i].start_point,runners[i].end_point))			
		{
			dir.X = runners[i].direction.X;
			dir.Y = runners[i].direction.Y;
			break;
		}
	}
	return dir;
}

void Nozzle::GasLine()
{
	if(g_install_type=="����ʽ") return;

	int int_layer = NXFunction::GetLayerNum("7gasline");
	if(int_layer==-1) return;
	Session *theSession = Session::GetSession();
    Part *workPart(theSession->Parts()->Work());
	std::vector<Body*> bodies;
	std::vector<DisplayableObject *> dispalys;

	long double gasline_H1,gasline_H2;
	gasline_H1=g_panel_H+g_H_top-10-15;
	gasline_H2=g_panel_H+g_H_top-35-15;
	
	
	workPart->Layers()->SetState(42,Layer::StateWorkLayer);
	Layer::LayerManager *layer_manager=workPart->Layers();
	std::vector<NXObject *>objects1=layer_manager->GetAllObjectsOnLayer(int_layer);	
	int gasline_count=objects1.size();
	
	Features::Feature *nullFeatures_Feature(NULL);
	try
	{
		for (int i=0;i<gasline_count;i++)
		{
			if(NXFunction::IsCurve(objects1[i],3)==false) continue;
			Features::CylinderBuilder *cylinderBuilder1;
			cylinderBuilder1 = workPart->Features()->CreateCylinderBuilder(nullFeatures_Feature);
			//�����λ��
			Line *line1(dynamic_cast<Line *>(objects1[i]));
			Direction *direction1;
			direction1 = workPart->Directions()->CreateDirection(line1,SenseForward, SmartObject::UpdateOptionWithinModeling); 
			Vector3d vector3d1=direction1->Vector();		
			Point3d startPoint=line1->StartPoint();			
			Point3d endPoint=line1->EndPoint();

			startPoint.Z=startPoint.Z+gasline_H1;
			endPoint.Z=endPoint.Z+gasline_H1;

			cylinderBuilder1->SetDirection(vector3d1); 
			cylinderBuilder1->SetOrigin(startPoint);
			//��������
			long double cylinder_height=sqrt(pow(startPoint.X-endPoint.X,2)+pow(startPoint.Y-endPoint.Y,2)+pow(startPoint.Z-endPoint.Z,2));
			cylinderBuilder1->Diameter()->SetRightHandSide("8.0");    
			cylinderBuilder1->Height()->SetRightHandSide(std::to_string(cylinder_height));
			NXObject *gas_ob=cylinderBuilder1->Commit();
			cylinderBuilder1->Destroy();			
			//��ɫ
			NXString str1=gas_ob->JournalIdentifier();
			Body *body1(dynamic_cast<Body *>(workPart->Bodies()->FindObject(str1)));
			//����
			body1->SetName("GASLINE");
			bodies.push_back(body1);
			dispalys.push_back(body1);
		}
	} catch(exception& ex){
		uc1601("��·����ʧ��",1);
	}
	NXFunction::SetColor(dispalys,130);
	NXFunction::CopyBodyByIncrement(bodies,0,0,gasline_H2-gasline_H1);
}

//����ʽ�����͸�
void Nozzle::ImportManifoldCylinder(Block block)
{
	NXString path_cylinder=block.file_path;
	Point3d point_cylinder(block.X,block.Y,0);
	
	//����
	NXFunction::ImportAndRoation(path_cylinder,point_cylinder,block.Angle,1);
	
	//������
	long double X=g_thermal_expansion_coefficient*(-block.X);
	long double Y=g_thermal_expansion_coefficient*(-block.Y);	
	NXFunction::MoveBodyByIncrement("CYLINDER",X,Y,0);
	NXFunction::MoveBodyByIncrement("SUB_CYLINDER_BOTTOM",X,Y,0);

	//����
	string body_name="CYLINDER";
	NXFunction::SetAttribute(body_name,"��������","assembly_id",block.assembly_id);
	NXFunction::SetAttribute(body_name,"��������","��װ��ʽ","����ʽ");
	NXFunction::SetAttribute(body_name,"��������","�������",block.assembly_name);

	//���
	NXFunction::NumberingPart(block.index,"CYLINDER",1,point_cylinder,block.Angle,"NaN");
	NXFunction::NumberingPart(block.index,"SUB_CYLINDER",1);
	NXFunction::NumberingPart(block.index,"SUB_CYLINDER_BOTTOM",1);
	NXFunction::NumberingPart(block.index,"SUB_CYLINDER_TOP",1);
	NXFunction::NumberingPart(block.index,"VALVE2",1);	
	
}
