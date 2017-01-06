#include "Heater.hpp"
#include "D:\\0_CAD\\UG\\0_Project\\BHRT\\Code\\ADOConn.hpp"
#include "D:\\0_CAD\\UG\\0_Project\\BHRT\\Code\\NXFunction.hpp"
#include <iostream>  
#include <vector>  
#include <algorithm>  
//using namespace std;
//using namespace NXOpen;

Heater::Heater()
{	
	string file_name=NXFunction::GetFileName();
	string host_name=NXFunction::GetHostName();
	adoconn = new ADOConn(host_name,file_name);
	adoconn->OnInitADOConn();

	int_layer = NXFunction::GetLayerNum("3heater");
}
Heater::~Heater()
{

}
//主函数
void Heater::CreateHeater(long double manifold_H)
{
	if  (int_layer==-1) return;
	NXFunction::DeleteParts("TUBE",0);

	Session *theSession = Session::GetSession();
	Part *workPart(theSession->Parts()->Work());
	
	str_BHRT=NXFunction::GetTranslateVariable();
	H=manifold_H;
	long double Z = -3.7;

	//0——得到所有加热线线段	
	long double total_length=0;
	GetSegments(&total_length);
	if(heater_segments.size()<0) return;
	
	//1——遍历线段，获得一条条的线
	std::vector<HeaterLine> heaterLines;	
	for(int i=0;i<heater_segments.size();i++)
	{
		if(heater_segments[i].added == true || heater_segments[i].data!=3) continue;
		//获得中心线
		heater_segments[i].added = true;
		HeaterLine heater_line = GetHeaterLine(heater_segments[i].curve);
		heaterLines.push_back(heater_line);			
	}
	std::sort(heaterLines.begin(),heaterLines.end(),LineSort);
	//2——生成
	for(int i=0;i<heaterLines.size();i++)
	{
		if(heaterLines[i].curves.size()<1) continue;		
		//创建文字
		CreateText(i+1,heaterLines[i]);
		//生成上下两层的线槽		
		CreateSwept(i+1,heaterLines[i]);	
		NXFunction::MirrorBody("SWEPT-"+std::to_string((long double)(i+1)),-H/2);
		//生成上层的管道，并插入属性
		CreateTube(i+1,heaterLines[i],6.5,0,Z);
		//生成上层陶瓷接线盒，并和管道合并
		CreateBox(i+1,heaterLines[i].dir,heaterLines[i],Z);
		//复制生成下层加热线
		NXFunction::CopyBodyByIncrement("TUBE-"+std::to_string((long double)(i+1)),0,0,-H-2*Z);
	}
	//3——后续
	Body *body = NXFunction::GetBodyByName("MANIFOLD");
	if(body!=NULL)
		if(!body->IsSheetBody()) 
			NXFunction::MultiSubPart("MANIFOLD","SWEPT-",1);

	NXFunction::RemoveParameters("MANIFOLD");
	//刻字
	Crave();
	//工艺参数
	string name_part = "MANIFOLD";
	NXFunction::SetAttribute(name_part,"工艺参数","发热管总长",std::to_string(total_length*2));
	NXFunction::SetAttribute(name_part,"工艺参数","发热管槽数量",std::to_string((long double)(2*heaterLines.size())));

	std::vector<Body*> bodies = NXFunction::GetBodiesByName("TUBE-");
	NXFunction::MoveBodies2Layer(40,bodies);
}
bool Heater::LineSort(const HeaterLine &v1, const HeaterLine &v2)
{
	//y轴矢量 （0,1）
	long double cos1 =(v1.start_point.Y)/sqrt(v1.start_point.X*v1.start_point.X+v1.start_point.Y*v1.start_point.Y); 
	if(v1.start_point.X<0) cos1=-2-cos1;

	long double cos2 =(v2.start_point.Y)/sqrt(v2.start_point.X*v2.start_point.X+v2.start_point.Y*v2.start_point.Y); 
	if(v2.start_point.X<0) cos2=-2-cos2;

	return cos1>cos2;//降序排序
}
//得到所有加热线线段 及其总长度
int Heater::GetSegments(long double *total_length)
{
	*total_length = 0;
	Session *theSession = Session::GetSession();
	Part *workPart(theSession->Parts()->Work());

	std::vector<NXObject *>objects1=workPart->Layers()->GetAllObjectsOnLayer(int_layer);
	if(objects1.size()==0) return 0;
	for(int i=0;i<objects1.size();i++)
	{
		HeaterSegment segment;
		segment.curve=dynamic_cast<Curve *>(objects1[i]);
		if(segment.curve==NULL) continue;
		*total_length +=segment.curve->GetLength();

		if(NXFunction::IsCurve(segment.curve,3))//Line
		{
			Line* line = dynamic_cast<Line *>(segment.curve);
			if(line==NULL) continue;
			segment.p1=line->StartPoint();
			segment.p2 = line->EndPoint();
			segment.added = false;
			segment.data = 3;
			
			heater_segments.push_back(segment);
		}
		else if(NXFunction::IsCurve(segment.curve,1))//Spline
		{
			Spline *spline1(dynamic_cast<Spline *>(objects1[i]));		
			if(spline1==NULL) continue;
			std::vector<Point4d> ps=spline1->GetPoles();			
			if(ps.size()<2) continue;

			double w1 = ps[0].W; 
			Point3d p1(ps[0].X/w1,ps[0].Y/w1,0);

			double w2 = ps[ps.size()-1].W; 
			Point3d p2(ps[ps.size()-1].X/w2,ps[ps.size()-1].Y/w2,0);

			segment.p1 = p1;
			segment.p2 = p2;
			segment.added = false;
			segment.data = 1;

			heater_segments.push_back(segment);
		}
		else if(NXFunction::IsCurve(segment.curve,2))//Arc
		{
			NXOpen::Arc *arc1(dynamic_cast<NXOpen::Arc *>(objects1[i]));
			if(arc1==NULL) continue;
			Point3d o = arc1->CenterPoint();
			long double r = arc1->Radius();
			long double s = arc1->StartAngle();
			long double e = arc1->EndAngle();

			segment.p1.X = o.X + r*cos(s);
			segment.p1.Y = o.Y + r*sin(s);
			segment.p1.Z = 0;
			segment.p2.X = o.X + r*cos(e);
			segment.p2.Y = o.Y + r*sin(e);
			segment.p2.Z = 0;
			segment.added = false;
			segment.data = 2;

			heater_segments.push_back(segment);
		}
	}

	return heater_segments.size();
}

//根据一条直线，从两端搜索，获得一条完整的发热管中心线
HeaterLine Heater::GetHeaterLine(Curve *aline)
{
	Line* line = dynamic_cast<Line *>(aline);

	HeaterLine heater_line;
	heater_line.length = line->GetLength();
	heater_line.start_point = line->StartPoint();
	heater_line.end_point = line->EndPoint();
	
	heater_line.curves.push_back(aline);
	Vector3d dir(heater_line.start_point.X - heater_line.end_point.X,
				heater_line.start_point.Y - heater_line.end_point.Y,0);
	heater_line.dir = dir;
	
	//1——从end_point找
	bool is_finish = false;		
	while(is_finish==false)
	{
		is_finish = true;
		for(int i=0;i<heater_segments.size();i++)
		{
			if(heater_segments[i].added == true) continue;

			if(!NXFunction::IsCoinCide(heater_segments[i].p1,heater_line.end_point)&&
			   !NXFunction::IsCoinCide(heater_segments[i].p2,heater_line.end_point) )
			   continue;

			is_finish = false;
			heater_segments[i].added = true;
			
			heater_line.curves.push_back(heater_segments[i].curve);
			heater_line.length+=heater_segments[i].curve->GetLength();

			//端点传递和方向
			if(NXFunction::IsCoinCide(heater_segments[i].p1,heater_line.end_point))
			{
				heater_line.end_point = heater_segments[i].p2;
				heater_line.other_point = heater_segments[i].p1;
				heater_line.other = 1;

				Vector3d dir1(heater_segments[i].p1.X - heater_segments[i].p2.X,
							 heater_segments[i].p1.Y - heater_segments[i].p2.Y,0);
				heater_line.dir = dir1;
			}
			else
			{
				heater_line.end_point = heater_segments[i].p1;
				heater_line.other_point = heater_segments[i].p2;
				heater_line.other = 1;

				Vector3d dir1(heater_segments[i].p2.X - heater_segments[i].p1.X,
							 heater_segments[i].p2.Y - heater_segments[i].p1.Y,0);
				heater_line.dir = dir1;
			}
		}
	}

	//2——从start_point找
	is_finish = false;		
	while(is_finish==false)
	{
		is_finish = true;
		for(int i=0;i<heater_segments.size();i++)
		{
			if(heater_segments[i].added == true) continue;

			if(!NXFunction::IsCoinCide(heater_segments[i].p1,heater_line.start_point)&&
			   !NXFunction::IsCoinCide(heater_segments[i].p2,heater_line.start_point) )
			   continue;

			is_finish = false;
			heater_segments[i].added = true;

			heater_line.curves.push_back(heater_segments[i].curve);
			heater_line.length+=heater_segments[i].curve->GetLength();

			//端点传递和方向
			if(NXFunction::IsCoinCide(heater_segments[i].p1,heater_line.start_point))
			{
				heater_line.start_point = heater_segments[i].p2;
				heater_line.other_point = heater_segments[i].p1;
				heater_line.other = 2;

				Vector3d dir1(heater_segments[i].p1.X - heater_segments[i].p2.X,
							 heater_segments[i].p1.Y - heater_segments[i].p2.Y,0);
				heater_line.dir = dir1;
			}
			else
			{
				heater_line.start_point = heater_segments[i].p1;
				heater_line.other_point = heater_segments[i].p2;
				heater_line.other = 2;

				Vector3d dir1(heater_segments[i].p2.X - heater_segments[i].p1.X,
							 heater_segments[i].p2.Y - heater_segments[i].p1.Y,0);
				heater_line.dir = dir1;
			}
		}
	}

	heater_line.length = int((heater_line.length+0.005)*100)/100.0;//保留两位小数
	return heater_line;
}

//生成发热管实体 插入属性 命名 移动
void Heater::CreateTube(int index,
						HeaterLine heater_line,
						long double outer_D,
						long double innner_D,
						long double Z)
{
	NXFunction::CreateTube(index,heater_line.curves,6.5,0);
	SaveCode2Tube(index,heater_line.length);
	NXFunction::MoveBodyByIncrement("TUBE-"+std::to_string((long double)index),0,0,Z);
}

//属性
void Heater::SaveCode2Tube(int curve_index,long double length)
{
	Session *theSession = Session::GetSession();
    Part *workPart(theSession->Parts()->Work());

	string name_part = "TUBE-"+std::to_string((long double)curve_index);
	string code = "42201";
	long double error = 2;

	string strsql = "SELECT top 1 st_code as EXPR1 from (SELECT * from standard_table where SUBSTRING(st_code,1,4)='HR22' ) expr ";
	string strwhere = "where abs(CAST(SUBSTRING(st_code,10,4) as int)-"+std::to_string(length)+")<="+std::to_string(error);
	string str = strsql+strwhere;

	code = adoconn->GetStringValue(str.c_str(),"").GetLocaleText();
	if(code=="")
	{
		strwhere = "where CAST(SUBSTRING(st_code,10,4) as int)>"+std::to_string(length)+" order by st_code";
		str = strsql+strwhere;
		code = adoconn->GetStringValue(str.c_str(),"").GetLocaleText();
	}
	if(code=="")
	{
		code = "42201";
	}
	

	NXFunction::SetAttribute(name_part,"基本参数","name","M"+std::to_string((long double)(curve_index)));
	NXFunction::SetAttribute(name_part,"基本参数","长度",std::to_string(length));
	NXFunction::SetAttribute(name_part,"物料编码","发热管",code);
	NXFunction::SetAttribute(name_part,"物料编码","陶瓷接线盒1","HR23049000001F");
	NXFunction::SetAttribute(name_part,"物料编码","陶瓷接线盒2","HR23049000001F");
}

//生成上层陶瓷接线盒，并和管道合并
void Heater::CreateBox(int curve_index,
						Vector3d dir,
						HeaterLine heater_line,
						long double Z)
{
	NXString path_box=str_BHRT+"\\Part_Base\\TBH.prt";
	heater_line.start_point.Z = Z;
	heater_line.end_point.Z = Z;
	
	NXFunction::ImportAndRoation(path_box,heater_line.start_point,dir,1);
	if(NXFunction::UnitePart("TUBE-"+std::to_string((long double)curve_index),"TBH")==0)
		NXFunction::NumberingPart2(curve_index*2,"TBH",1);
	
	NXFunction::ImportAndRoation(path_box,heater_line.end_point,dir,1);
	if(NXFunction::UnitePart("TUBE-"+std::to_string((long double)curve_index),"TBH")==0)
		NXFunction::NumberingPart2(curve_index*2+1,"TBH",1);

	NXFunction::RemoveParameters("TUBE-"+std::to_string((long double)curve_index));
}

void Heater::CreateSwept(int curve_index,HeaterLine heater_line)
{
	Point3d p1(heater_line.start_point.X,heater_line.start_point.Y,-3.5);	
	//插入片体获得界面
	NXString path_heater=str_BHRT+"\\Part_Base\\heater.prt";
	NXFunction::ImportAndRoation(path_heater,p1,heater_line.dir,1);
	//扫描
	NXFunction::CreateGuideSwept(curve_index,"SHEET-HEATER",heater_line.curves,0,"");
	NXFunction::DeleteParts("SHEET-HEATER",1);
}

//刻字
void Heater::CreateText(int index,HeaterLine heater_line)
{
	if(heater_line.curves.size()<2) return;

	//加热器的起点终点向量的中点
	Point3d point_mid((heater_line.start_point.X+heater_line.end_point.X)/2,
					 (heater_line.start_point.Y+heater_line.end_point.Y)/2,
					 0);

	//刻字插入点
	Point3d point_insert(0,0,0);
	Vector3d vertical(0,0,0);
	if(heater_line.other==1)
	{
		vertical.X = heater_line.other_point.X - heater_line.end_point.X;
		vertical.Y = heater_line.other_point.Y - heater_line.end_point.Y;
	}
	else
	{
		vertical.X = heater_line.other_point.X - heater_line.start_point.X;
		vertical.Y = heater_line.other_point.Y - heater_line.start_point.Y;
	}
	long double unitization  = sqrt(vertical.X *vertical.X + vertical.Y *vertical.Y );
	vertical.X = vertical.X / unitization;
	vertical.Y = vertical.Y / unitization;
	point_insert.X = 32*vertical.X + point_mid.X;
	point_insert.Y = 32*vertical.Y + point_mid.Y;

	//文字的向量
	Vector3d vec_txt(heater_line.end_point.X-heater_line.start_point.X,
					heater_line.end_point.Y-heater_line.start_point.Y,0);
	long double verify = (heater_line.start_point.X-point_insert.X)*(heater_line.end_point.Y-point_insert.Y);
	verify = verify-(heater_line.start_point.Y-point_insert.Y)*(heater_line.end_point.X-point_insert.X);

	vec_txt.X *= verify;
	vec_txt.Y *= verify;

	//创建文字
	NXFunction::CreateText(point_insert,1,vec_txt,"M"+std::to_string((long double)index));
	point_insert.Z = -H;
	NXFunction::CreateText(point_insert,-1,vec_txt,"M"+std::to_string((long double)index));
}
void Heater::Crave()
{
	Session *theSession = Session::GetSession();
    Part *workPart(theSession->Parts()->Work());
	std::vector<Features::Text *> texts;

	Features::FeatureCollection::iterator it;
	Features::FeatureCollection *cols=workPart->Features();
	for(it=cols->begin();it!=cols->end();it++)
	{
		Features::Feature *feature(dynamic_cast<Features::Feature *>(*it));
		string feature_type = feature->FeatureType().GetLocaleText();
		if(feature_type!="TEXT") continue;
		Features::Text *text1(dynamic_cast<Features::Text *>(feature));
		if(text1==NULL) continue;

		texts.push_back(text1);
	}

	NXFunction::CraveOnManifold(texts);
	NXFunction::RemoveParameters("MANIFOLD");

	//删除发热管文字
    Session::UndoMarkId markId1;
    markId1 = theSession->SetUndoMark(Session::MarkVisibilityInvisible, "Delete");    
    bool notifyOnDelete1 = theSession->Preferences()->Modeling()->NotifyOnDelete();  

    theSession->UpdateManager()->ClearErrorList();    
    Session::UndoMarkId markId2;
    markId2 = theSession->SetUndoMark(Session::MarkVisibilityVisible, "Delete");
    
	std::vector<NXObject*> objs;
	for(int i=0;i<texts.size();i++)
			objs.push_back(texts[i]);
    int nErrs1  = theSession->UpdateManager()->AddToDeleteList(objs);
    
    bool notifyOnDelete2 = theSession->Preferences()->Modeling()->NotifyOnDelete();    
    int nErrs2  = theSession->UpdateManager()->DoUpdate(markId2);    
    theSession->DeleteUndoMark(markId1, NULL);
}
