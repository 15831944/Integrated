#include "RunnerInsert.hpp"
#include "D:\\0_CAD\\UG\\0_Project\\BHRT\\Code\\ADOConn.hpp"
#include "D:\\0_CAD\\UG\\0_Project\\BHRT\\Code\\NXFunction.hpp"

using namespace std;
using namespace NXOpen;

RunnerInsert::RunnerInsert()
{	
	string file_name=NXFunction::GetFileName();
	string host_name=NXFunction::GetHostName();
	adoconn = new ADOConn(host_name,file_name);
	adoconn->OnInitADOConn();
}
RunnerInsert::~RunnerInsert()
{

}

void RunnerInsert::Main(long double manifold_H,long double runner_offset,long double runner_D)
{
	NXFunction::DeleteParts("RUNNERINSERT",0);
	NXFunction::DeleteParts("SUB_RUNNERINSERT",0);

	Session *theSession = Session::GetSession();
    Part *workPart(theSession->Parts()->Work());	

	NXString str_BHRT=NXFunction::GetTranslateVariable();
	H = manifold_H;
	D=adoconn->GetSeries();
	H_offset = runner_offset;
	
	Point3d point_origin = GetOriginPoint();//��������ڵ�λ
	
	if(GetNozzlePoints()==0) return;
	if(GetAllPoints()==0) return;

	int count = FilterPoints();
	for(int i=0;i<count;i++)
	{
		//��Ӧ��������
		Point3d nozzle_point = FindNearestNozzlePoint(g_insert_points[i]);
		
		//��������浽�����ľ���
		long double dis_wall = NXFunction::GetDistance(g_insert_points[i].point,nozzle_point);

		//�����׼������� ����ת
		string strsql="SELECT part_code,assembly_id FROM assembly_table where assembly_name='�������' and substring(part_code,1,4)='HR28' AND CAST(SUBSTRING(part_code,6,2) AS INT)="+std::to_string(runner_D);
		string code="HR28xxxx";
		string assembly_id="";
		_RecordsetPtr record=NULL;
		record = adoconn->GetRecordSet2(strsql.c_str());
		for(int j=0;!record->adoEOF;j++)
		{
			code = adoconn->GetStringCollect(record,"part_code");
			assembly_id = adoconn->GetStringCollect(record,"assembly_id");
			record->MoveNext();
		}
		Vector3d dir_target(0,0,0);		
		dir_target.X=g_insert_points[i].another_point.X-g_insert_points[i].point.X;		
		dir_target.Y=g_insert_points[i].another_point.Y-g_insert_points[i].point.Y;

		if(!NXFunction::CheckFileExist(str_BHRT+"\\Part_Base\\RUNNER_INSERT\\"+code+".prt","�������"))
			return;
		NXFunction::ImportAndRoation3D(str_BHRT+"\\Part_Base\\RUNNER_INSERT\\"+code+".prt",g_insert_points[i].point,dir_target,1);
		
		//��׼��������ĳ���
		Body *body = NXFunction::GetBodyByName("RUNNERINSERT");
		long double L = dis_wall;
		if(body->HasUserAttribute("L",NXObject::AttributeTypeAny,-1))
		{
			string str_L=body->GetUserAttributeAsString("L",NXObject::AttributeTypeAny,-1).GetLocaleText();
			L = std::atof(str_L.c_str());
		}
		
		//�޸ĳ��� (dis_move<0Ӧ��������)
		long double dis_move = L-dis_wall;
		if(abs(dis_move)>0.001)
			NXFunction::MoveFaceByFeature2("MOVE2","RUNNERINSERT",dis_move);	

		//���ϱ��뱣�浽ʵ����
		string name_part = "RUNNERINSERT";
		strsql = "select part_code,st_material_name from assembly_view where assembly_id='"+assembly_id+"'"; 
		_RecordsetPtr record1=NULL;
		record1=adoconn->GetRecordSet2(strsql.c_str());
		for(int j=0;!record1->adoEOF;j++)
		{
			NXFunction::SetAttribute(name_part,"���ϱ���",adoconn->GetStringCollect(record1,"st_material_name")
													  ,adoconn->GetStringCollect(record1,"part_code"));	
			
			record1->MoveNext();
		}
		if(abs(dis_move)>0.5)
		{
			code = "42801";
			string remark = "ʵ�ʳ���"+std::to_string(dis_wall);
			NXFunction::SetAttribute(name_part,"���ϱ���","�������",code);	
			NXFunction::SetAttribute(name_part,"���ϱ���","���������ע",remark);	
		}
		NXFunction::SetAttribute(name_part,"����","p1X",std::to_string((long double)g_insert_points[i].p1.X));
		NXFunction::SetAttribute(name_part,"����","p1Y",std::to_string((long double)g_insert_points[i].p1.Y));
		NXFunction::SetAttribute(name_part,"����","p2X",std::to_string((long double)g_insert_points[i].p2.X));
		NXFunction::SetAttribute(name_part,"����","p2Y",std::to_string((long double)g_insert_points[i].p2.Y));
		NXFunction::NumberingPart(i,"RUNNERINSERT",1);
		NXFunction::NumberingPart(i,"SUB_RUNNERINSERT",1);
	}
	
	string manifold = "MANIFOLD";
	NXFunction::SetAttribute(manifold,"���ղ���","�����������",std::to_string((long double)count));	
}
//�õ��������
Point3d RunnerInsert::GetOriginPoint()
{
	Point3d point_origin(0,0,-H/2+H_offset);
	return point_origin;
}
//�õ�������������
int RunnerInsert::GetNozzlePoints()
{
	std::vector<Block> blocks = adoconn->GetNozzleBlocks();
	if( blocks.size()==0 ) return 0;
	//2��������
	for(unsigned int i=0;i<blocks.size();i++)
	{
		//2.1������ȡ����
		Block nl_com=blocks[i];
		Point3d point_nozzle;
		point_nozzle.X=nl_com.X;
		point_nozzle.Y=nl_com.Y;
		point_nozzle.Z=-H/2+H_offset;
		g_nozzle_points.push_back(point_nozzle);		
	} 

	return g_nozzle_points.size();
}
//�õ����������˵�   
int RunnerInsert::GetAllPoints()
{
	Session *theSession = Session::GetSession();
	Part *workPart(theSession->Parts()->Work());
	g_runners.clear();
	g_runner_points.clear();

	//0��������ͼ����
	int runner_layer=NXFunction::GetLayerNum("2runner");
	if(runner_layer==-1) return 0;

	Layer::LayerManager *layer_manager=workPart->Layers();
	std::vector<NXObject *>objects1=layer_manager->GetAllObjectsOnLayer(runner_layer);	
	int runner_count=objects1.size();
	if(runner_count==0) return 0;

	//1�����õ�vector_runners ��vector_runner_points
	for(int i=0;i<runner_count;i++)
	{
		//1.1������������
		Line *line1(dynamic_cast<Line *>(objects1[i]));		
		Direction *direction1;
		direction1 = workPart->Directions()->CreateDirection(line1,SenseForward, SmartObject::UpdateOptionWithinModeling); 
		Vector3d vector3d1=direction1->Vector();		
		Point3d start_point=line1->StartPoint();
		Point3d end_point=line1->EndPoint();
		start_point.Z=start_point.Z-H/2+H_offset;//-H/2+H_runner_offset;//Ĭ��ֵ���ڷ��������� TODO
		end_point.Z  =end_point.Z-H/2+H_offset;  //-H/2+H_runner_offset;//Ĭ��ֵ���ڷ���������
		
		//1.2����runners
		Runner runner;
		runner.direction=vector3d1;
		runner.start_point=start_point;
		runner.end_point=end_point;
		runner.runner_index=i;

		g_runners.push_back(runner);
	    
		//1.3����points
		RunnerPoint point1;
		point1.direction=vector3d1;
		point1.point=start_point;
		point1.another_point=end_point;
		point1.is_runner_insert=1;
		point1.runner_index=i;

		RunnerPoint point2;
		point2.direction=vector3d1;
		point2.point=end_point;
		point2.another_point=start_point;
		point2.is_runner_insert=1;
		point2.runner_index=i;

		g_runner_points.push_back(point1);
		g_runner_points.push_back(point2);
	}

	return g_runner_points.size();
}
//�õ�����Ĳ����
int RunnerInsert::FilterPoints()
{
	Session *theSession = Session::GetSession();
	Part *workPart(theSession->Parts()->Work());

	g_insert_points.clear();

	//1�����õ�������������
	std::vector<Point3d> profile_points;
	int int_layer = NXFunction::GetLayerNum("1manifold");
	if  (int_layer==-1) return 0;
	
	std::vector<NXObject *>objects1=workPart->Layers()->GetAllObjectsOnLayer(int_layer);	
	if (objects1.size()==0) return 0;

	for (unsigned int i=0;i<objects1.size();i++)
	{	
		Curve *curve(dynamic_cast<Curve *>(objects1[i]));	
		if(curve==NULL) continue;
		if(NXFunction::IsCurve(objects1[i],1)==true)
		{			
			Spline *spline1(dynamic_cast<Spline *>(curve));		
			if(spline1!=NULL) 
			{
				std::vector<Point4d> ps=spline1->GetPoles();
				std::vector<double> knot = spline1->GetKnots();
				for(unsigned int j=0;j<ps.size();j++)
				{
					double w = ps[j].W; 
					Point3d p1(ps[j].X/w,ps[j].Y/w,ps[j].Z/w);
					profile_points.push_back(p1);
				}
			}
		}
		else if(NXFunction::IsCurve(objects1[i],3)==true)
		{
			Line *line(dynamic_cast<Line *>(curve));
			if(line!=NULL)
			{
				profile_points.push_back(line->StartPoint());
				profile_points.push_back(line->EndPoint());
			}
		}
	}

	int profile_size = profile_points.size();
	//2�����Ƚ������˵�������㣬�غ�=>�������
	for(unsigned int i=0;i<g_runner_points.size();i++)
	{		
		for(unsigned int j=0;j<profile_size;j++)
		{
			int k = j+1;
			if(k==profile_size) k=0;
			if(NXFunction::IsP1inP2P3_Plus(g_runner_points[i].point,profile_points[j],profile_points[k]))			
			{
				g_runner_points[i].p1 = profile_points[j];
				g_runner_points[i].p2 = profile_points[k];
				g_insert_points.push_back(g_runner_points[i]);
				break;
			}
		}
	}
	
	return g_insert_points.size();
}
//�ҵ������������������
Point3d RunnerInsert::FindNearestNozzlePoint(RunnerPoint insert_point)
{
	long double distance = 10000000;
	int p1=0;
	for(unsigned int i=0;i<g_nozzle_points.size();i++)
	{
		if(NXFunction::IsP1inP2P3(g_nozzle_points[i],insert_point.point,insert_point.another_point))
		{
			long double d = (g_nozzle_points[i].X-insert_point.point.X)*(g_nozzle_points[i].X-insert_point.point.X)+(g_nozzle_points[i].Y-insert_point.point.Y)*(g_nozzle_points[i].Y-insert_point.point.Y);
			if(d<distance)
			{
				p1 = i;
				distance = d;
			}
		}
	}
	return g_nozzle_points[p1];
}