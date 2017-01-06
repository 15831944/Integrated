#include "PartControl.h"
#include "ADOConn.hpp"
#include "NXFunction.hpp"
using namespace std;
using namespace NXOpen;

PartControl::PartControl(Components block,int part_type,int index)
{
	adoconn=new ADOConn();
	adoconn->OnInitADOConn();
	PartComponent=block;
}

PartControl::~PartControl(void)
{
	adoconn->ExitConnect();
	delete adoconn;
}

///提交生成
void PartControl::Commit()
{
}
