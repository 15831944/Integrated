#include "WireFrame.h"
#include "D:\\0_CAD\\UG\\0_Project\\BHRT\\Code\\ADOConn.hpp"
#include "D:\\0_CAD\\UG\\0_Project\\BHRT\\Code\\NXFunction.hpp"
using namespace std;
using namespace NXOpen;

WireFrame::WireFrame(long double manifold_H)
{
	string file_name=NXFunction::GetFileName();
	string host_name=NXFunction::GetHostName();
	adoconn = new ADOConn(host_name,file_name);
	adoconn->OnInitADOConn();

	blocks = adoconn->GetBlocksByAssemblyName("线架");	
	int_layer = NXFunction::GetLayerNum("wireframe");	
	g_manifold_H = manifold_H;
}

WireFrame::~WireFrame(void)
{
	adoconn->ExitConnect();
	delete adoconn;
}

void WireFrame::CreateWireFrame()
{
	NXFunction::DeleteParts("WIRELINE",0);

	if( int_layer==-1 )return;
	if( blocks.size()==0 )return;

	int count = GetSegments();
	if(count==0) return;

	count = GetWireLines();
	if(count==0) return;

	for(int i=0;i<count;i++)
	{
		CreateWireLine(wire_lines[i],i);
	}
}

int WireFrame::GetSegments()
{
	Session *theSession = Session::GetSession();
	Part *workPart(theSession->Parts()->Work());

	std::vector<NXObject *>objects1=workPart->Layers()->GetAllObjectsOnLayer(int_layer);
	if(objects1.size()==0) return 0;
	for(int i=0;i<objects1.size();i++)
	{
		WireSegment segment;
		Curve* curve =dynamic_cast<Curve *>(objects1[i]);
		if(curve==NULL) continue;		
		if(!NXFunction::IsCurve(curve,3)) continue;		
		Line* line = dynamic_cast<Line *>(curve);
		if(line==NULL) continue;

		segment.line = line;
		segment.p1=line->StartPoint();
		segment.p2 = line->EndPoint();
		segment.added = false;
			
		wire_segments.push_back(segment);
	}

	return wire_segments.size();
}

int WireFrame::GetWireLines()
{
	for(int i=0;i<blocks.size();i++)
	{
		WireLine wire_line;
		Point3d sp(blocks[i].X,blocks[i].Y,0);
		wire_line.sp = sp;
		wire_line.ep = sp;
		wire_line.length = 0;
		for(int k=0;k<wire_segments.size();k++)
		{
			for(int j=0;j<wire_segments.size();j++)
			{
				if(wire_segments[j].added == true) continue;
				if(!NXFunction::IsCoinCide(wire_segments[j].p1,wire_line.ep) &&
				   !NXFunction::IsCoinCide(wire_segments[j].p2,wire_line.ep))continue;

				wire_segments[j].added = true;
			
				wire_line.lines.push_back(wire_segments[j].line);				
				wire_line.length+=wire_segments[j].line->GetLength();

				//端点传递
				if(NXFunction::IsCoinCide(wire_segments[j].p1,wire_line.ep))
				{
					wire_line.points.push_back(wire_segments[j].p1);
					wire_line.points.push_back(wire_segments[j].p2);
					wire_line.ep = wire_segments[j].p2;
				}
				else
				{
					wire_line.points.push_back(wire_segments[j].p2);
					wire_line.points.push_back(wire_segments[j].p1);					
					wire_line.ep = wire_segments[j].p1;
				}
				
				
			}
		}
		wire_lines.push_back(wire_line);
	}
	return wire_lines.size();
}

void WireFrame::CreateWireLine(WireLine wire_line,int index)
{
	string prefix = "WIRELINE-"+std::to_string((long double)index)+"-";
	int count = wire_line.points.size();
	//线段集	
	for(unsigned int i=0;i<count;i+=2)//隔着
	{
		//生成实体
		Point3d p(wire_line.points[i].X,wire_line.points[i].Y,-g_manifold_H);
		Vector3d dir(wire_line.points[i+1].X-wire_line.points[i].X,
					wire_line.points[i+1].Y-wire_line.points[i].Y,0);
		NXFunction::ImportAndRoation(blocks[index].file_path,p,dir,1);

		std::vector<Curve*> curves;	
		curves.push_back(wire_line.lines[i/2]);

		NXFunction::CreateGuideSwept(i/2,"SHEET_WIRE_FRAME",curves,prefix,50);

		//旋转面
		Body *body = NXFunction::GetBodyByName(prefix+std::to_string((long double)i/2));
		
		if(i!=0)//不是第一段
		{
			Face* sf = NXFunction::GetFace(body,wire_line.points[i],dir);

			Vector3d d1(wire_line.points[i-1].X-wire_line.points[i-2].X,
						wire_line.points[i-1].Y-wire_line.points[i-2].Y,0);
			d1 = NXFunction::VectorUnitization(d1);

			Vector3d d2(wire_line.points[i+1].X-wire_line.points[i].X,
						wire_line.points[i+1].Y-wire_line.points[i].Y,0);
			d2 = NXFunction::VectorUnitization(d2);

			Vector3d dir_target(d1.X+d2.X,d1.Y+d2.Y,0);

			NXFunction::RotateFace(sf,d2,dir_target,wire_line.points[i]);

		}
		if(i!=count-2)//不是最后一段
		{
			Face* ef = NXFunction::GetFace(body,wire_line.points[i+1],dir);	

			Vector3d d2(wire_line.points[i+1].X-wire_line.points[i].X,
						wire_line.points[i+1].Y-wire_line.points[i].Y,0);
			d2 = NXFunction::VectorUnitization(d2);

			Vector3d d3(wire_line.points[i+3].X-wire_line.points[i+2].X,
						wire_line.points[i+3].Y-wire_line.points[i+2].Y,0);
			d3 = NXFunction::VectorUnitization(d3);

			Vector3d dir_target(d2.X+d3.X,d2.Y+d3.Y,0);

			NXFunction::RotateFace(ef,d2,dir_target,wire_line.points[i+1]);
		}
		
		//移除参数，删除片体
		NXFunction::RemoveParameters(prefix.c_str()+std::to_string((long double)i/2));
		NXFunction::RemoveParameters("SHEET_WIRE_FRAME");	
		NXFunction::DeletePart2("SHEET_WIRE_FRAME",1);

		//物料编码
		string name = prefix + std::to_string((long double)i/2);
		NXFunction::SetAttribute(name,"基本参数","线架规格",
								std::to_string((long double)wire_line.lines[i/2]->GetLength()));
		NXFunction::SetAttribute(name,"基本参数","线架备注",blocks[index].assembly_id);
		NXFunction::SetAttribute(name,"物料编码","线架","4540"+std::to_string((long double)i/2+1));
		
	}
	
}
