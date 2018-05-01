#include "task_scheduler_second_window.h"
task_scheduler_second_window* task_scheduler_second_window::ptr = nullptr;

task_scheduler_second_window::task_scheduler_second_window(void)
{
	ptr = this;
	constr_with_parametr = false;
}
task_scheduler_second_window::task_scheduler_second_window(task_info task_state)
{
	ptr = this;
	lstrcpy(this->task_state_second_window.path, task_state.path);
	lstrcpy(this->task_state_second_window.period, task_state.period);
	this->task_state_second_window.stime = task_state.stime;
	this->task_state_second_window.sdate = task_state.sdate;
	this->task_state_second_window.th_ID = task_state.th_ID;
	for (size_t i = 0; i < 7; i++)
	{
		this->task_state_second_window.days_check[i] = task_state.days_check[i];
	}
	constr_with_parametr = true;
}
void task_scheduler_second_window::Cls_OnClose(HWND hwnd)
{
	EndDialog(hwnd, 0);
}
BOOL task_scheduler_second_window::Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	open_button = GetDlgItem(hwnd, IDC_obzor);
	h_period	= GetDlgItem(hwnd, IDC_COMBO1);
	h_path		= GetDlgItem(hwnd, IDC_EDIT1);
	time_picker = GetDlgItem(hwnd, IDC_DATETIMEPICKER2);
	date_picker = GetDlgItem(hwnd, IDC_DATETIMEPICKER1);
	cancel		= GetDlgItem(hwnd, ID_cancel);
	for (size_t i = 0; i < 7; i++)
	{
		days[i]= GetDlgItem(hwnd, IDC_CHECK1 + i);
	}	
	if (constr_with_parametr)
	{
		SetWindowText(h_path,	 this->task_state_second_window.path);
		SetWindowText(h_period,	 this->task_state_second_window.period);
		SendMessage(time_picker, DTM_SETSYSTEMTIME, 0, (LPARAM)&this->task_state_second_window.stime);
		SendMessage(date_picker, DTM_SETSYSTEMTIME, 0, (LPARAM)&this->task_state_second_window.sdate);
		for (size_t i = 0; i < 7; i++)
		{
			if (this->task_state_second_window.days_check[i] == true)
			{
				SendMessage(days[i], BM_SETCHECK, BST_CHECKED, 0);
			}
		}
	}
	else
	{
		SetWindowText(h_period, L"Once");
	}
	SendMessage(h_period, CB_ADDSTRING, NULL, LPARAM(TEXT("Daily")));
	SendMessage(h_period, CB_ADDSTRING, NULL, LPARAM(TEXT("Weekly")));
	SendMessage(h_period, CB_ADDSTRING, NULL, LPARAM(TEXT("Monthly")));
	SendMessage(h_period, CB_ADDSTRING, NULL, LPARAM(TEXT("Once")));
	return TRUE;
}
void task_scheduler_second_window::Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	if (id == IDC_COMBO1 && codeNotify == CBN_SELCHANGE)
	{
		int index = SendMessage(h_period, CB_GETCURSEL, 0, 0);
		if (index == 0)
		{
			for (size_t i = 0; i < 7; i++)
			{
				SendMessage(days[i], BM_SETCHECK, WPARAM(BST_CHECKED), 0);
				EnableWindow(days[i], FALSE);
			}
		}
		else
			if (index == 3 || index == 2)
			{
				for (size_t i = 0; i < 7; i++)
				{
					SendMessage(days[i], BM_SETCHECK, WPARAM(BST_UNCHECKED), 0);
					EnableWindow(days[i], FALSE);
				}
			}
			else
			{
				for (size_t i = 0; i < 7; i++)
				{
					SendMessage(days[i], BM_SETCHECK, WPARAM(BST_UNCHECKED), 0);
					EnableWindow(days[i], TRUE);
				}
			}
	}
	switch (id)
	{
	case IDC_obzor:
	{
		TCHAR FullPath[MAX_PATH] = { 0 };
		OPENFILENAME open = { sizeof(OPENFILENAME) };
		open.hwndOwner = hwnd;
		open.lpstrFilter = TEXT("Executable Files(*.exe)\0*.exe\0All Files(*.*)\0*.*\0");
		open.lpstrFile = FullPath;
		open.nMaxFile = MAX_PATH;
		open.lpstrInitialDir = TEXT("C:\\");
		open.Flags = OFN_CREATEPROMPT | OFN_PATHMUSTEXIST;
		GetOpenFileName(&open);
		SetWindowText(h_path, open.lpstrFile);
		break;
	}
	case IDOK:
	{
		SendMessage(time_picker, DTM_GETSYSTEMTIME, 0, (LPARAM)&task_state_second_window.stime);
		SendMessage(date_picker, DTM_GETSYSTEMTIME, 0, (LPARAM)&task_state_second_window.sdate);
		GetWindowText(h_path, task_state_second_window.path, MAX_PATH);
		GetWindowText(h_period, task_state_second_window.period, 50);
		for (size_t i = 0; i < 7; i++)
		{
			SendMessage(days[i], BM_GETCHECK, 0, 0) == BST_CHECKED ? task_state_second_window.days_check[i] = true : task_state_second_window.days_check[i] = false;
		}
		///checks:
		////path
		if (lstrcmp(task_state_second_window.path, L""))
		{
			new_task_parametrs_check = true;
		}
		else
		{
			new_task_parametrs_check = false;
			MessageBox(hwnd, L"Please choose path to exe file", L"Warning!", ID_OK);
			break;
		}
		if (!lstrcmp(task_state_second_window.period, L"Weekly"))
		{
			for (size_t i = 0; i < 7; i++)
			{
				if ((task_state_second_window.days_check[i] == true) && (task_state_second_window.sdate.wDayOfWeek - 1 == i))
				{
					new_task_parametrs_check = true;
					break;
				}
				else
				{
					new_task_parametrs_check = false;
				}
			}
			if (new_task_parametrs_check == false)
			{
				MessageBox(hwnd, L"Wrong day checked!", L"Warning!", ID_OK);
				break;
			}
		}
		if (new_task_parametrs_check == true)
		{
			EndDialog(hwnd, 1);
		}
		else
		{
			EndDialog(hwnd, 0);
		}
	}
	break;

	case ID_cancel:
	{
		EndDialog(hwnd, 0);
		break;
	}
	break;
	
	}
}
BOOL CALLBACK task_scheduler_second_window::dlgproc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		HANDLE_MSG(hwnd, WM_CLOSE, ptr->Cls_OnClose);
		HANDLE_MSG(hwnd, WM_INITDIALOG, ptr->Cls_OnInitDialog);
		HANDLE_MSG(hwnd, WM_COMMAND, ptr->Cls_OnCommand);
	}
	return FALSE;
}