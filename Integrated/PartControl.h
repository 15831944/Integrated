#pragma once
#include <iostream>
#include <fstream>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <tchar.h>
#include <malloc.h>
#include <string.h>
#include <NXOpen/Session.hxx>
#include <NXOpen/UI.hxx>

using namespace std;
using namespace NXOpen;
class ADOConn;
struct Components;
enum PartType{
		Normal = 1 //Ò»°ãÁã¼þ
	};
class PartControl
{
public:
	PartControl(Components block,int part_type,int index);
	~PartControl(void);

	void Commit();
public:
	ADOConn *adoconn;
	Components PartComponent;

	string AssemblyID;
	string ComponentName;
	string AssemblyName;
};

