#pragma once
#include <windows.h>
#include <windowsX.h>
#include <tchar.h>
#include <commctrl.h>
#include <time.h>
#include "resource.h"
#include <io.h>
#include <vector>

#pragma comment(lib,"comctl32")

struct task_info
{
	TCHAR path[MAX_PATH];
	SYSTEMTIME stime;
	SYSTEMTIME sdate;
	bool days_check[7];
	TCHAR period[50];
	DWORD th_ID; 
	int count;
};


