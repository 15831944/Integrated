#include "Manifold.hpp"
#include "D:\\0_CAD\\UG\\0_Project\\BHRT\\Code\\ADOConn.hpp"
#include "D:\\0_CAD\\UG\\0_Project\\BHRT\\Code\\NXFunction.hpp"

#include <iostream>  
#include <vector>  
#include <algorithm> 
Manifold::Manifold()
{
	file_name=NXFunction::GetFileName();
	host_name=NXFunction::GetHostName();
	adoconn = new ADOConn(host_name,file_name);
	adoconn->OnInitADOConn();
}
Manifold::~Manifold()
{
	adoconn->ExitConnect();
	delete adoconn;
}
//0��������������������������������������������������������������������������������
int Manifold::Create(
	string str_ZF,
	string str_SR,
	long double manifold_H,
	long double runner_D,
	long double insulation_H_top,
	long double insulation_H_Bottom,
	long double cylinder_H_install,   //���װ�װ���߶�
	long double runner_offset        //����ƫ�� Z����
	)

{
	ZF = str_ZF;
	SR = str_SR;
	H=manifold_H;//Ĭ��ֵ
	H_top=insulation_H_top;
	H_Bottom=insulation_H_Bottom;
	H_cylinder=cylinder_H_install;
	H_runner_offset=runner_offset;
	D=adoconn->GetSeries();
	D_runner = runner_D;
	
	int suc = CreateManifold();
	if(suc!=0) 
	{
		Body *body = NXFunction::GetBodyByName("MANIFOLD");
		if(body->IsSheetBody()) return 0;
		
		Ear();		
		CreateBolt();
					
		NXFunction::SetBodyColor(body,1);			
		CreateGate();
		//���������CreateRunner();
	}
	SubManifold();//ɾ����ǰ�ķ����壬��������
	
	return 1;
}

//0������������������
int Manifold::CreateManifold()
{
	int int_layer = NXFunction::GetLayerNum("1manifold");
	if (int_layer==-1) return 0;

	perimeter = 0;
	Session *theSession = Session::GetSession();
	Part *workPart(theSession->Parts()->Work());	
	Section *section1=workPart->Sections()->CreateSection();

	int size = NXFunction::Layer2Section2("1manifold",section1,&perimeter);
	if (size==0) return 0;

	Features::Feature *null_feature(NULL);
	Features::ExtrudeBuilder *extrudeBuilder= workPart->Features()->CreateExtrudeBuilder(null_feature);
	
	extrudeBuilder->SetSection(section1);
	
	extrudeBuilder->Limits()->StartExtend()->Value()->SetRightHandSide("0");
	extrudeBuilder->Limits()->EndExtend()->Value()->SetRightHandSide(std::to_string(H));

	Point3d origin1(0.0, 0.0, 0.0);
	Vector3d vector1(0.0, 0.0, -1.0);
	Direction *direction1;
	direction1 = workPart->Directions()->CreateDirection(origin1, vector1, SmartObject::UpdateOptionWithinModeling);
    
	extrudeBuilder->SetDirection(direction1);
	
	g_extrude=extrudeBuilder->CommitFeature();//�ύ
	
	extrudeBuilder->Destroy();

	//����
	Body *body1(dynamic_cast<Body *>(workPart->Bodies()->FindObject(g_extrude->JournalIdentifier())));
	if(body1->IsSheetBody()) uc1601("�������������Ƭ�壡���������պϣ�",1);
	NXFunction::RemoveParameters2(g_extrude->JournalIdentifier());
	//��ʵ���ƶ���40ͼ��	
	std::vector<DisplayableObject *> objectArray1(1);
	objectArray1[0]=body1;
	workPart->Layers()->MoveDisplayableObjects(40, objectArray1);
	//����
	long double X=0;
	long double Y=0;
	GetMaxXY(&X,&Y);
	string remark = std::to_string(X) + " * " + std::to_string(Y) + " * " + std::to_string(H+0.5)+" (������:��0.5���߶ȹ���:+0~+0.2��ƽ�жȹ����0.05������ֲڶ�1.6��HRC 28~32��";
	
	//������ǰ��ʵ����������Ե���ʵ�壬Ȼ��ɾ���ɵ�
	body1->SetName("MANIFOLD_NEW");
	Body *source = NXFunction::GetBodyByName("MANIFOLD");
	NXFunction::CodyAttribute(source,body1);
	NXFunction::DeletePart(source);
	body1->SetName("MANIFOLD");

	//��������
	string name_part = "MANIFOLD";
	NXFunction::SetAttribute(name_part,"���ϱ���","������","40301");
	NXFunction::SetAttribute(name_part,"���ϱ���","��������",remark);
	
	NXFunction::SetAttribute(name_part,"��������","name","������H"+std::to_string(H)+"D"+std::to_string(D));
	NXFunction::SetAttribute(name_part,"��������","H",std::to_string(H));
	NXFunction::SetAttribute(name_part,"��������","H_offset",std::to_string(H_runner_offset));
	NXFunction::SetAttribute(name_part,"��������","D",std::to_string(D));
	NXFunction::SetAttribute(name_part,"��������","D_runner",std::to_string(D_runner));
	NXFunction::SetAttribute(name_part,"��������","insulation_H_top",std::to_string(H_top));
	NXFunction::SetAttribute(name_part,"��������","insulation_H_Bottom",std::to_string(H_Bottom));
	NXFunction::SetAttribute(name_part,"��������","H_cylinder",std::to_string(H_cylinder));

	NXFunction::SetAttribute(name_part,"���ղ���","����������뾶",SR);
	NXFunction::SetAttribute(name_part,"���ղ���","��������",std::to_string(H));
	NXFunction::SetAttribute(name_part,"���ղ���","�������ܳ�",std::to_string(10*H)); 
	NXFunction::SetAttribute(name_part,"���ղ���","���������",std::to_string(GetArea()));
	
	if(ZF=="����ʽ")
		NXFunction::SetAttribute(name_part,"���ղ���","��װ��ʽ","0");
	else if(ZF=="����ʽ")
		NXFunction::SetAttribute(name_part,"���ղ���","��װ��ʽ","1");
	else 
		NXFunction::SetAttribute(name_part,"���ղ���","��װ��ʽ","2");
	return size;
}
//1��������
void Manifold::Ear()
{
	int int_layer = NXFunction::GetLayerNum("ear");
	if (int_layer==-1) return ;
	long double ear_count = 0;
	long double ear_length = 0;
	std::vector<Body *> target_bodies = NXFunction::GetBodiesByName("MANIFOLD");
	Session *theSession = Session::GetSession();
	Part *workPart(theSession->Parts()->Work());

	//������ �������ȵ�һ��
	string ear_h = std::to_string(H/2);
	/*string ear_h = "15";
	if(ZF == "����ʽ") ear_h = "20";*/

	std::vector<NXObject *>objects1=workPart->Layers()->GetAllObjectsOnLayer(int_layer);	
	for(unsigned int i=0;i<objects1.size();i++)
	{
		try
		{
			Annotations::Hatch *hatch1(dynamic_cast<Annotations::Hatch *>(objects1[i]));
			if(hatch1==NULL) continue;

			Annotations::HatchBuilder * hatchbuilder = workPart->Annotations()->Hatches()->CreateHatchBuilder(hatch1);
			Annotations::BoundaryBuilder *boundary = hatchbuilder->Boundary();
			SectionList *list = boundary->CurveBoundaries();
			if(list->Length()==0) continue;

			Section *section1=workPart->Sections()->CreateSection();
			int size = NXFunction::Sections2Section(list,section1,&ear_length);
			Features::Feature *null_feature(NULL);
			Features::ExtrudeBuilder *extrudeBuilder= workPart->Features()->CreateExtrudeBuilder(null_feature);
	
			extrudeBuilder->SetSection(section1);
	
			extrudeBuilder->Limits()->StartExtend()->Value()->SetRightHandSide("0");
			extrudeBuilder->Limits()->EndExtend()->Value()->SetRightHandSide(ear_h);

			Point3d origin1(0.0, 0.0, 0.0);
			Vector3d vector1(0.0, 0.0, -1.0);
			Direction *direction1;
			direction1 = workPart->Directions()->CreateDirection(origin1, vector1, SmartObject::UpdateOptionWithinModeling);
			extrudeBuilder->SetDirection(direction1);
	
			extrudeBuilder->BooleanOperation()->SetType(GeometricUtilities::BooleanOperation::BooleanTypeSubtract);
			extrudeBuilder->BooleanOperation()->SetTargetBodies(target_bodies);
			
			extrudeBuilder->Commit();//�ύ
			extrudeBuilder->Destroy();
			ear_count++;
		} 
		catch(exception &ex)
		{
			string str =  ex.what();
			str = "��"+std::to_string((long double)(ear_count+1))+"���������"+ str;
			
			uc1601(const_cast<char*>(str.c_str()),1);
		}
	}
	string name_part="MANIFOLD";
	NXFunction::SetAttribute(name_part,"���ղ���","������",ear_h);
	NXFunction::SetAttribute(name_part,"���ղ���","�����ܳ�",std::to_string(ear_length));
	NXFunction::SetAttribute(name_part,"���ղ���","�������",std::to_string(ear_count));
	return ;
}
//2����������̶���˿
void Manifold::CreateBolt()
{
	int int_layer = NXFunction::GetLayerNum("1manifold");
	if (int_layer==-1) return;

	//ɾ����ǰ��
	NXFunction::DeleteParts("SPACER",1);
	NXFunction::DeleteParts("SUB_BOLT",1);
	NXFunction::DeleteParts("COTTER",1);
	NXFunction::DeleteParts("SUB_COTTER",1);
	NXFunction::DeleteParts("BOLT",1);
	NXFunction::DeleteParts("WASHER",1);

	Session *theSession = Session::GetSession();
	Part *workPart(theSession->Parts()->Work());
	std::vector<NXObject *> objs=workPart->Layers()->GetAllObjectsOnLayer(int_layer);
	if(objs.size()==0) return;

	//1. ѡ����˿�ͺ� bolt_spec bolt_m
	string bolt_spec = "M10"; 
	long double bolt_m = 10;
	if(ZF == "����ʽ" || ZF == "������ʽ")
	{	
		if(D<=12)
		{
			bolt_spec = "M8"; 
			bolt_m = 8;
		}
	}
	else
	{
		bolt_spec = "M12";
		bolt_m=12;
	}

	//2. ֧�����ͺź͸߶ȣ��Ѿ��ƶ���߶ȣ�֧���ײ����߶ȣ������ļ���
	string spacer_spec = "Spacer_"+bolt_spec;
	long double default_spacer_h = 10;
	if(bolt_spec=="M12") default_spacer_h = 25;
	long double move_spacer = default_spacer_h - H_Bottom;//����Ӧ���쳤
	string path_spacer=adoconn->part_base+"Bolt\\" +spacer_spec+".prt";

	//3. ֧�������ϱ���
	string spacer_code = "44101";	
	string spacer_remark = "";
	long double spacer_h = 0;
	string d = "16";
	if(bolt_m==8)       d="16";
	else if(bolt_m==10) d="18";
	else if(bolt_m==12) d="24";
	string strsql="SELECT top 1 cast(SUBSTRING(st_code,11,3) as int) as EXPR1 ";
	strsql += " FROM standard_table where SUBSTRING(st_code,1,4)='HR41' AND series='0'";
	strsql += " and cast(SUBSTRING(st_code,6,2) as int)="+d;
	strsql += " and cast(SUBSTRING(st_code,11,3) as int)>="+std::to_string(H_Bottom) + " order by EXPR1";
	spacer_h = adoconn->GetDoubleValue(strsql.c_str());
	if(spacer_h!=H_Bottom) spacer_remark = "���μӹ���ʵ�ʸ߶�" + std::to_string(H_Bottom);

	strsql="SELECT top 1 st_code as EXPR1 ";
	strsql += " FROM standard_table where SUBSTRING(st_code,1,4)='HR41' AND series='0'";
	strsql += " and cast(SUBSTRING(st_code,6,2) as int)="+d;
	strsql += " and cast(SUBSTRING(st_code,11,3) as int)>="+std::to_string(H_Bottom) + " order by cast(SUBSTRING(st_code,11,3) as int)";
	spacer_code = adoconn->GetStringValue(strsql.c_str(),"44101").GetLocaleText();

	//4. �ж��Ƿ�Ҫ��Ȧ
	long double washer_h = 0;
	long double expand = 0;
	for(int i=0;i<objs.size();i++)
	{
		Curve *curve(dynamic_cast<Curve *>(objs[i]));	
		if(curve==NULL) continue;
		if(NXFunction::IsCurve(objs[i],2)==false) continue;

		NXOpen::Arc *arc(dynamic_cast<NXOpen::Arc *>(curve));
		long double angle = arc->StartAngle() - arc->EndAngle();
		if(abs(angle)>0.01 && abs(abs(angle)-2*3.14159262)>0.01 ) continue;//��Բ

		long double X = arc->CenterPoint().X;
		long double Y = arc->CenterPoint().Y;

		if(sqrt((X*X + Y*Y))*0.0024>expand)
			expand = sqrt((X*X + Y*Y))*0.0024;
	}
	if(expand>=1 && ( bolt_spec=="M8" || bolt_spec=="M10")) washer_h = 3;//��ǰ��1.5
	else if(expand>=1 && bolt_spec=="M12") washer_h = 4;

	//5. ȷ������λ��
	std::vector<Point3d> points;
	std::vector<Point3d> points_spacer;
	long double Z = -H/2+washer_h;
	for(int i=0;i<objs.size();i++)
	{
		Curve *curve(dynamic_cast<Curve *>(objs[i]));	
		if(curve==NULL) continue;
		if(NXFunction::IsCurve(objs[i],2)==false) continue;

		NXOpen::Arc *arc(dynamic_cast<NXOpen::Arc *>(curve));

		if(arc->Radius()>=9) continue;

		long double angle = arc->StartAngle() - arc->EndAngle();
		if(abs(angle)>0.01 && abs(abs(angle)-2*3.14159262)>0.01 ) continue;//��Բ

		Point3d point_spacer(arc->CenterPoint().X,arc->CenterPoint().Y,-H+move_spacer);
		Point3d point_insert(arc->CenterPoint().X,arc->CenterPoint().Y,Z);

		points.push_back(point_insert);
		points_spacer.push_back(point_spacer);
	}

	//6. �Ȳ���һ��
	if(points.size()==0) return;
	//6.1 ����֧���ײ��ƶ���(M12�Ϳ���) ��� ���ϱ���
	CreateBolt_Spacer(0,path_spacer,points_spacer[0],move_spacer,spacer_code,spacer_remark);
	//6.2 �������������ж��Ƿ��е�Ȧ���͵�Ȧ�߶�
	if(washer_h>0) CreateBolt_Washer(0,points[0],bolt_spec);
	//6.3 ������˿
	string path_bolt = adoconn->part_base +"Bolt\\"+bolt_spec+".prt";
	NXFunction::ImportAndRoation(path_bolt.c_str(),points[0],0,1);
	//6.4 �޸���˿����,bolt_l��5
	long double bolt_l = 0;
	if(bolt_m==8)
		bolt_l = H/2+washer_h+H_Bottom+2*bolt_m; //H/2�ǲ������ȥ�ĸ߶�
	else if(bolt_m=10)
		bolt_l = H/2+washer_h+H_Bottom+2*bolt_m; 
	else
		bolt_l = H/2+washer_h+H_Bottom+2*bolt_m; 

	double re = (int)bolt_l % 5;
	bolt_l= (int)bolt_l - re;

	NXFunction::MoveFaceByFeature2("MOVE4","BOLT",bolt_l-1.25);
	//6.5 ����assembly_id		
	strsql="select dbo.QD_GetBoltAssemblyID("+std::to_string((long double)bolt_m)+","+std::to_string(bolt_l)+") as Expr1";
	string assembly_id = adoconn->GetStringValue(strsql.c_str(),"�Ǳ걭ͷ��˿").GetLocaleText();
	string name_part = "BOLT";
	NXFunction::SetAttribute(name_part,"���ϱ���","assembly_id",assembly_id);
	NXFunction::NumberingPart2(0,name_part,1);
	
	Body *spacer = NXFunction::GetBodyByName("SPACER-0");
	Body *sub_bolt = NXFunction::GetBodyByName("SUB_BOLT-0");
	Body *bolt = NXFunction::GetBodyByName("BOLT-0");
	Body *washer = NXFunction::GetBodyByName("WASHER-0");
	
	//7. ��������������
	for(int i=1;i<points.size();i++)
	{
		NXFunction::CopyBodyP2P(spacer,points[0],points[i],i);
		NXFunction::CopyBodyP2P(sub_bolt,points[0],points[i],i);
		NXFunction::CopyBodyP2P(bolt,points[0],points[i],i);
		NXFunction::CopyBodyP2P(washer,points[0],points[i],i);
	}
}
//����2.1������˿֧����
void Manifold::CreateBolt_Spacer(int index,NXString path_spacer,Point3d point_spacer
								,long double move_spacer
								,string code,string remark)
{
	try
	{
		NXFunction::ImportAndRoation(path_spacer,point_spacer,0,1);
		NXFunction::MoveFaceByFeature2("MOVE3","SPACER",move_spacer);
		NXFunction::MoveBodyByIncrement("COTTER",0,0,-move_spacer);
		NXFunction::MoveBodyByIncrement("SUB_COTTER",0,0,-move_spacer);

		NXFunction::DeleteCodeAttribute("SPACER");
		NXFunction::DeleteCodeAttribute("COTTER");
		//֧�������ϱ���	
		string name_part = "SPACER";
		NXFunction::SetAttribute(name_part,"���ϱ���","�̶���˿֧����",code.c_str());
		if(remark!="")
		{
			NXFunction::SetAttribute(name_part,"���ϱ���","�̶���˿֧���ױ�ע",remark.c_str());
		}
	
		string name_part2="COTTER";
		NXFunction::SetAttribute(name_part2,"���ϱ���","�̶���˿����","HR45017800020A");

		NXFunction::NumberingPart2(index,"SPACER",1);
		NXFunction::NumberingPart2(index,"SUB_BOLT",1);
		NXFunction::NumberingPart2(index,"COTTER",1);
		NXFunction::NumberingPart2(index,"SUB_COTTER",1);
	} catch(exception &ex) {
		string str = ex.what();
		str = "�����"+std::to_string((long double)index)+"���̶���˿֧����ʧ��" + str;
		uc1601(const_cast<char*>(str.c_str()),1);
	}
}
//����2.2������˿��Ȧ
void Manifold::CreateBolt_Washer(int index,Point3d arc_p,string bolt_spec)
{
	string path_wash = adoconn->part_base +"Bolt\\Washer_"+bolt_spec+".prt";
	
	Point3d point_insert(arc_p.X,arc_p.Y,-H/2);
	//if(ZF=="����ʽ") point_insert.Z = -20;

	NXFunction::ImportAndRoation(path_wash.c_str(),point_insert,0,1);
	NXFunction::NumberingPart2(index,"WASHER",1);

	return ;
}
//3��������������
void Manifold::CreateGate()
{
	Session *theSession = Session::GetSession();
	Part *workPart(theSession->Parts()->Work());

	try
	{
		std::vector<Body*> bodies = NXFunction::GetBodiesByName("MANIFOLD");	
		Point3d p(0,0,0);
		Vector3d dir(0,0,-1);
		NXFunction::CreateCylinder(p,dir,D,H/2-H_runner_offset,bodies,1,"");
	
	} catch(exception& ex){
		uc1601("����������ʧ��",1);
	}
}
//4������������
void Manifold::CreateRunner()
{
	Session *theSession = Session::GetSession();
	Part *workPart(theSession->Parts()->Work());
    Features::Feature *nullFeatures_Feature(NULL);

	//��֤
	int int_layer = NXFunction::GetLayerNum("2runner");
	if  (int_layer==-1) return;
	std::vector<Body *> target_bodies = NXFunction::GetBodiesByName("MANIFOLD");
	if(target_bodies.size()==0) return;


	Layer::LayerManager *layer_manager=workPart->Layers();
	std::vector<NXObject *>objects1=layer_manager->GetAllObjectsOnLayer(int_layer);	
	int cylinder_count=objects1.size();

	long double total_length = 0;
	std::vector<LineStruct> lines;
	for (int i=0;i<cylinder_count;i++)
	{
		Line *line1(dynamic_cast<Line *>(objects1[i]));
		if(line1==NULL) 
			continue;
		LineStruct lineStruct;
		lineStruct.line=line1;
		lineStruct.len=line1->GetLength();
		lines.push_back(lineStruct);
	}
	
	//std::sort(lines.begin(),lines.end(),LineSort);

	for(int i=0;i<lines.size();i++)
	{
		Line* line1=lines[i].line;
		long double line_length = lines[i].len;
			
		total_length +=line_length;
				
		Point3d startPoint=line1->StartPoint();
				startPoint.Z=-H/2+H_runner_offset;
			
		Point3d endPoint=line1->EndPoint();
				endPoint.Z=startPoint.Z;

		Vector3d direction(endPoint.X-startPoint.X,endPoint.Y-startPoint.Y,endPoint.Z-startPoint.Z);	
		//NXFunction::CreateCylinder(startPoint,direction,D_runner,line_length,target_bodies,0,"ARUNNER",i);
		NXFunction::CreateCylinderWithSphere(startPoint,endPoint,direction,D_runner,line_length,"ARUNNER",i);
		
		NXFunction::SubPart("MANIFOLD","ARUNNER-"+std::to_string((long double)i),1,1);
	}
	/*NXFunction::UnitePart2("ARUNNER","RUNNER");
	NXFunction::SubPart("MANIFOLD","RUNNER",1,1);*/	
	//NXFunction::RemoveParameters();
	string name_part = "MANIFOLD";
	NXFunction::SetAttribute(name_part,"���ղ���","��������ܳ�",std::to_string(total_length));
	NXFunction::SetAttribute(name_part,"���ղ���","�����������",std::to_string((long double)cylinder_count));
}
bool Manifold::LineSort(const LineStruct &v1, const LineStruct &v2)
{
	return v1.len<v2.len;//��������
}
//6����������������壬�Ƴ��������ƶ���42�� ��ɾ����ǰ�ķ����壩
void Manifold::SubManifold()
{
	int int_layer = NXFunction::GetLayerNum("1submanifold");
	if(int_layer==-1)return;

	Session *theSession = Session::GetSession();
	Part *workPart(theSession->Parts()->Work());

	workPart->Layers()->SetState(42,Layer::StateWorkLayer);//������42��

	std::vector<NXObject *>objects1=workPart->Layers()->GetAllObjectsOnLayer(int_layer);
	int intsize=objects1.size();

	//ɾ����ǰ��sub_manifold
	NXFunction::DeleteParts("SUB_MANIFOLD",1);

	Features::Feature *null_feature(NULL);
	Features::ExtrudeBuilder *extrudeBuilder= workPart->Features()->CreateExtrudeBuilder(null_feature);
	Section *section1=workPart->Sections()->CreateSection();
	extrudeBuilder->SetSection(section1);
	
	for (int i=0;i<intsize;i++)
	{
		Curve *spline1=dynamic_cast<Curve *>(objects1[i]);
		std::vector<Curve  *> curves1(1,spline1);
		CurveDumbRule *curveDumbRule1;
		curveDumbRule1 = workPart->ScRuleFactory()->CreateRuleCurveDumb(curves1);
		std::vector<SelectionIntentRule *> rules1(1,curveDumbRule1);
		NXObject *nullNXObject(NULL);
		Point3d helpPoint1(0.0,0.0,0.0);
		section1->AddToSection(rules1, nullNXObject,nullNXObject, nullNXObject, helpPoint1, Section::ModeCreate);
	}
	extrudeBuilder->Limits()->StartExtend()->Value()->SetRightHandSide(std::to_string((-1)*H_top));
	extrudeBuilder->Limits()->EndExtend()->Value()->SetRightHandSide(std::to_string(H+H_Bottom));

	Point3d origin1(0.0, 0.0, 0.0);
	Vector3d vector1(0.0, 0.0, -1.0);
	Direction *direction1;
	direction1 = workPart->Directions()->CreateDirection(origin1, vector1, SmartObject::UpdateOptionWithinModeling);
    
	extrudeBuilder->SetDirection(direction1);
	NXObject *sub_manifold=extrudeBuilder->CommitFeature();
	extrudeBuilder->Destroy();

	//���Ρ���������������������������������������������������������������������������������������������������
	NXString str1=sub_manifold->JournalIdentifier();
    Body *body1(dynamic_cast<Body *>(workPart->Bodies()->FindObject(str1)));//�������������ʵ��
	NXFunction::RemoveParameters2(str1);
	//����
	body1->SetName("Sub_Manifold");
	//��ɫ
	DisplayModification *displayModification1;
    displayModification1 = theSession->DisplayManager()->NewDisplayModification();    
    displayModification1->SetApplyToAllFaces(false);    
    displayModification1->SetApplyToOwningParts(false);    
    displayModification1->SetNewColor(130);
    
    std::vector<DisplayableObject *> objects10(1);
    objects10[0] = body1;
    displayModification1->Apply(objects10);
    
    delete displayModification1;
    
    theSession->DisplayManager()->BlankObjects(objects10);    
    workPart->ModelingViews()->WorkView()->FitAfterShowOrHide(View::ShowOrHideTypeHideOnly);
}

//�������������֮��
long double Manifold::GetArea()
{
	long double value = 0;

	Session *theSession = Session::GetSession();
    Part *workPart(theSession->Parts()->Work());
	Body* body = NXFunction::GetBodyByName("MANIFOLD");
	std::vector<Face*> faces= body->GetFaces();
	Unit *unit1(dynamic_cast<Unit *>(workPart->UnitCollection()->FindObject("MilliMeter")));
	
	for(int i=0;i<faces.size();i++)
	{
		std::vector<IParameterizedSurface *> ifaces(1);
		ifaces[0] = faces[i];
		MeasureFaces  *mf;	
		mf = workPart->MeasureManager()->NewFaceProperties(unit1,unit1,0.001,ifaces);
		long double ivalue = mf->Area();
		if(ivalue>value) value = ivalue;	
	}
    
	return value*2;
}

//������ϳߴ�
void Manifold::GetMaxXY(long double* X,long double* Y)
{
	long double x_max=0;
	long double x_min=0;
	long double y_max=0;
	long double y_min=0;

	Session *theSession = Session::GetSession();
	Part *workPart(theSession->Parts()->Work());
	//�õ�������������
	std::vector<Point3d> profile_points;
	int int_layer = NXFunction::GetLayerNum("1manifold");
	if  (int_layer==-1) return ;
	std::vector<NXObject *>objects1=workPart->Layers()->GetAllObjectsOnLayer(int_layer);	
	if (objects1.size()==0) return ;

	for (unsigned int i=0;i<objects1.size();i++)
	{	
		Curve *curve(dynamic_cast<Curve *>(objects1[i]));	
		if(curve==NULL) continue;
		if(NXFunction::IsCurve(objects1[i],1))
		{
			Spline *spline1(dynamic_cast<Spline *>(curve));		
			//if(spline1==NULL) continue;
			std::vector<Point4d> ps=spline1->GetPoles();
			std::vector<double> knot = spline1->GetKnots();
			for(unsigned int j=0;j<ps.size();j++)
			{
				double w = ps[j].W; 
				Point3d p1(ps[j].X/w,ps[j].Y/w,ps[j].Z/w);

				if(p1.X > x_max) x_max = p1.X;
				else if(p1.X < x_min) x_min = p1.X;

				if(p1.Y > y_max) y_max = p1.Y;
				else if(p1.Y < y_min) y_min = p1.Y;
			}
		}
		else if(NXFunction::IsCurve(objects1[i],2))
		{
			NXOpen::Arc *arc1(dynamic_cast<NXOpen::Arc *>(curve));	
			
			Point3d o = arc1->CenterPoint();
			long double r = arc1->Radius();
			long double s = arc1->StartAngle();
			long double e = arc1->EndAngle();

			Point3d p1(o.X + r*cos(s),o.Y + r*sin(s),0);
			Point3d p2(o.X + r*cos(e),o.Y + r*sin(e),0);

			if(p1.X > x_max) x_max = p1.X;
			else if(p1.X < x_min) x_min = p1.X;
			if(p1.Y > y_max) y_max = p1.Y;
			else if(p1.Y < y_min) y_min = p1.Y;

			if(p2.X > x_max) x_max = p2.X;
			else if(p2.X < x_min) x_min = p2.X;
			if(p2.Y > y_max) y_max = p2.Y;
			else if(p2.Y < y_min) y_min = p2.Y;
		}
		else if(NXFunction::IsCurve(objects1[i],3))
		{
			Line *line(dynamic_cast<Line *>(curve));	
			if(line==NULL)continue;
			Point3d p1 = line->StartPoint();
			Point3d p2 = line->EndPoint();
			if(p1.X > x_max) x_max = p1.X;
			else if(p1.X < x_min) x_min = p1.X;
			if(p1.Y > y_max) y_max = p1.Y;
			else if(p1.Y < y_min) y_min = p1.Y;

			if(p2.X > x_max) x_max = p2.X;
			else if(p2.X < x_min) x_min = p2.X;
			if(p2.Y > y_max) y_max = p2.Y;
			else if(p2.Y < y_min) y_min = p2.Y;
		}
	}

	*X = ceil(x_max - x_min);
	*Y = ceil(y_max - y_min);
}

int Manifold::CheckBlockAndLayer()
{
	std::vector<string> warnings;
	string layers[7]={"1manifold","1submanifold","2runner","3heater","wireframe","7gasline","ear"};
	for(int i=0;i<7;i++)
	{
		int int_layer = NXFunction::GetLayerNum(layers[i].c_str());
		if(int_layer==-1)
		{
			string warning = "ͼ�㣺 "+layers[i];
			warnings.push_back(warning);
		}
	}

	string assembly_names[5] = {"������","���Ķ�λ��","��λ��","�ȵ�ż","�߼�"};
	for(int i=0;i<5;i++)
	{
		std::vector<Block> bs = adoconn->GetBlocksByAssemblyName(assembly_names[i].c_str());
		if(bs.size()==0)
		{
			string warning = "ͼ�飺"+assembly_names[i];
			warnings.push_back(warning);
		}
	}

	std::vector<Block> nozzle_blocks = adoconn->GetNozzleBlocks();
	if(nozzle_blocks.size()==0)
	{
		string warning = "ͼ�飺����";
		warnings.push_back(warning);
	}
	std::vector<Block> nozzle_cylinders = adoconn->GetCylinderBlocks();
	if(nozzle_cylinders.size()==0)
	{
		string warning = "ͼ�飺�����͸�";
		warnings.push_back(warning);
	}

	if(warnings.size()>0)
	{
		UI *theUI = UI::GetUI();
		std::vector<NXString> nxstrs;
		for(int i=0;i<warnings.size();i++)
			nxstrs.push_back(warnings[i].c_str());
		if(theUI->NXMessageBox()->Show("ȱ������ͼ���ͼ��:",NXMessageBox::DialogTypeQuestion,nxstrs)==2)
			return 0;
	}

	return 1;
}

