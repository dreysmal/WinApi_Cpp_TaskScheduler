#define WM_ICON WM_APP
#define ID_TRAYICON WM_USER
#include "task_scheduler.h"
task_scheduler* task_scheduler::ptr = nullptr;
int task_scheduler::count = 0;
static bool load_s = false;

task_scheduler::task_scheduler(void)
{
	ptr = this;
	tray_icon = new NOTIFYICONDATA;
}
task_scheduler::~task_scheduler()
{
	delete tray_icon;
}
void task_scheduler::Cls_OnClose(HWND hwnd)
{
	save();
	EndDialog(hwnd, 0);
}
BOOL task_scheduler::Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	icon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1));
	SetClassLong(hwnd, GCLP_HICON, LONG(icon));
	list_of_processes = GetDlgItem(hwnd, IDC_LIST1);
	list_of_logs = GetDlgItem(hwnd, IDC_LIST2);
	main_dialog = hwnd;
	memset(tray_icon, 0, sizeof(NOTIFYICONDATA));
	tray_icon->cbSize = sizeof(NOTIFYICONDATA); 
	tray_icon->hIcon = icon;
	tray_icon->hWnd = hwnd; 
	lstrcpy(tray_icon->szTip, TEXT("Task Scheduler"));
	tray_icon->uCallbackMessage = WM_ICON; 
	tray_icon->uFlags = NIF_TIP | NIF_ICON | NIF_MESSAGE | NIF_INFO;
	lstrcpy(tray_icon->szInfo, TEXT("Task Scheduler"));
	lstrcpy(tray_icon->szInfoTitle, TEXT("Task Scheduler!"));
	tray_icon->uID = ID_TRAYICON;
	load(hwnd);
	return TRUE;
}
/////////////////////////////////////////////////////////////THREAD////
DWORD WINAPI task_thread(LPVOID lp)///////////////////////////////////
{
	task_scheduler* p = (task_scheduler*)lp;
	task_info task_info_thread;
	HWND list_thread = p->list_of_processes;
	HWND list_log_thread = p->list_of_logs;
	TCHAR status_string[MAX_PATH];
	TCHAR log_string[MAX_PATH];
	lstrcpy(task_info_thread.path, p->tasks_mas[p->count].path);
	lstrcpy(task_info_thread.period, p->tasks_mas[p->count].period);
	task_info_thread.sdate = p->tasks_mas[p->count].sdate;
	task_info_thread.th_ID = p->tasks_mas[p->count].th_ID;
	task_info_thread.count = p->tasks_mas[p->count].count;
	for (size_t i = 0; i < 7; i++)
	{
		task_info_thread.days_check[i] = p->tasks_mas[p->count].days_check[i];
	}
	int timer_count = 0;
	FILETIME file_time;
	HANDLE hev = OpenEvent(EVENT_ALL_ACCESS, 0, L"{ADC3AF8E-6133-4792-9D15-94E27BC457A6}");
	//////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////DAILY////////////////////
	if (!lstrcmp(task_info_thread.period, L"Daily"))
	{
		while (true)
		{
			bool was_timer = false;
			HANDLE timer = CreateWaitableTimer(NULL, TRUE, NULL);
			if (!timer_count)
			{
				SystemTimeToFileTime(&task_info_thread.sdate, &file_time);
				LocalFileTimeToFileTime(&file_time, &file_time);
			}
			ULARGE_INTEGER tl_cur, tl_task;
			SYSTEMTIME tl_cur_systemtime;
			FILETIME tl_cur_filetime;
			GetLocalTime(&tl_cur_systemtime);
			SystemTimeToFileTime(&tl_cur_systemtime, &tl_cur_filetime);
			LocalFileTimeToFileTime(&tl_cur_filetime, &tl_cur_filetime);
			tl_cur.LowPart = tl_cur_filetime.dwLowDateTime;
			tl_cur.HighPart = tl_cur_filetime.dwHighDateTime;
			tl_task.LowPart = file_time.dwLowDateTime;
			tl_task.HighPart = file_time.dwHighDateTime;
			if(tl_cur.QuadPart < tl_task.QuadPart)
			{
				SetWaitableTimer(timer, (LARGE_INTEGER*)&file_time, 0, NULL, NULL, FALSE);
				SetEvent(hev);
				if (WaitForSingleObject(timer, INFINITE) == WAIT_OBJECT_0)
				{
					HINSTANCE shell_answer = ShellExecute(NULL, L"open", task_info_thread.path, NULL, NULL, SW_SHOW);
					int shell_answer_int = (int)shell_answer;
					switch (shell_answer_int)
					{
					case 0:
						lstrcpy(status_string, L" - Error: System has not enough memory or resurses!");
						break;
					case ERROR_BAD_FORMAT:
						lstrcpy(status_string, L" - Error: An attempt was made to load a program with an incorrect format!");
						break;
					case SE_ERR_ACCESSDENIED:
						lstrcpy(status_string, L" - Error: ACCESS DENIED!");
						break;
					case SE_ERR_DLLNOTFOUND:
						lstrcpy(status_string, L" - Error: DLL NOT FOUND!");
						break;
					case SE_ERR_ASSOCINCOMPLETE:
						lstrcpy(status_string, L" - Error: The file name association is incomplete or invalid!");
						break;
					case SE_ERR_FNF:
						lstrcpy(status_string, L" - Error: The specified file was not found!");
						break;
					case SE_ERR_NOASSOC:
						lstrcpy(status_string, L" - Error: There is no application associated with the given file name extension. !");
						break;
					case SE_ERR_OOM:
						lstrcpy(status_string, L" - Error: There was not enough memory to complete the operation!");
						break;
					case SE_ERR_PNF:
						lstrcpy(status_string, L" - Error: The specified path was not found!");
						break;
					case SE_ERR_SHARE:
						lstrcpy(status_string, L" - Error: A sharing violation occurred!");
						break;
					default:
						lstrcpy(status_string, L" - Successfully executed!");
						lstrcat(log_string, status_string);
						was_timer = true;
					}
					CancelWaitableTimer(timer);
					CloseHandle(timer);
					timer_count = 1;
					lstrcpy(log_string, task_info_thread.path);
					lstrcat(log_string, status_string);
					SendMessage(list_log_thread, LB_ADDSTRING, 0, (LPARAM)log_string);
					tl_task.QuadPart += 864000000000; //сутки
					file_time.dwLowDateTime = tl_task.LowPart;
					file_time.dwHighDateTime = tl_task.HighPart;
				}
			}
			if(!was_timer)
			{
				tl_task.QuadPart += 864000000000; //сутки
				file_time.dwLowDateTime = tl_task.LowPart;
				file_time.dwHighDateTime = tl_task.HighPart;
				timer_count = 1;
			}
		}
	}
	//////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////Weekly////////////////////
	if (!lstrcmp(task_info_thread.period, L"Weekly"))
	{
		while (true)
		{
			SYSTEMTIME ctime;
			GetLocalTime(&ctime);
			for (size_t i = 0; i < 7; i++)
			{
				if ((task_info_thread.days_check[i] == true) && (i == ctime.wDayOfWeek - 1))
				{
					HANDLE timer = CreateWaitableTimer(NULL, TRUE, NULL);
					if (!timer_count)
					{
						SystemTimeToFileTime(&task_info_thread.sdate, &file_time);
						LocalFileTimeToFileTime(&file_time, &file_time);
					}
					SetWaitableTimer(timer, (LARGE_INTEGER*)&file_time, 0, NULL, NULL, FALSE);
					SetEvent(hev);
					if (WaitForSingleObject(timer, 0) == WAIT_OBJECT_0)
					{
						HINSTANCE shell_answer = ShellExecute(NULL, L"open", task_info_thread.path, NULL, NULL, SW_SHOW);
						int shell_answer_int = (int)shell_answer;
						switch (shell_answer_int)
						{
						case 0:
							lstrcpy(status_string, L" - Error: System has not enough memory or resurses!");
							break;
						case ERROR_BAD_FORMAT:
							lstrcpy(status_string, L" - Error: An attempt was made to load a program with an incorrect format!");
							break;
						case SE_ERR_ACCESSDENIED:
							lstrcpy(status_string, L" - Error: ACCESS DENIED!");
							break;
						case SE_ERR_DLLNOTFOUND:
							lstrcpy(status_string, L" - Error: DLL NOT FOUND!");
							break;
						case SE_ERR_ASSOCINCOMPLETE:
							lstrcpy(status_string, L" - Error: The file name association is incomplete or invalid!");
							break;
						case SE_ERR_FNF:
							lstrcpy(status_string, L" - Error: The specified file was not found!");
							break;
						case SE_ERR_NOASSOC:
							lstrcpy(status_string, L" - Error: There is no application associated with the given file name extension. !");
							break;
						case SE_ERR_OOM:
							lstrcpy(status_string, L" - Error: There was not enough memory to complete the operation!");
							break;
						case SE_ERR_PNF:
							lstrcpy(status_string, L" - Error: The specified path was not found!");
							break;
						case SE_ERR_SHARE:
							lstrcpy(status_string, L" - Error: A sharing violation occurred!");
							break;
						default:
							lstrcpy(status_string, L" - Successfully executed!");
							lstrcat(log_string, status_string);
						}
						CancelWaitableTimer(timer);
						CloseHandle(timer);
						timer_count = 1;
						lstrcpy(log_string, task_info_thread.path);
						lstrcat(log_string, status_string);
						SendMessage(list_log_thread, LB_ADDSTRING, 0, (LPARAM)log_string);
						Sleep(86395000); //засыпаем на без пяти секунд сутки;
						break;
					}
				}
			}
			
		}
	}
	////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////MONTHLY////////////////////////////
	if (!lstrcmp(task_info_thread.period, L"Monthly"))
	{
		int day_if_february = 0;
		while (true)
		{
			if (day_if_february)
				task_info_thread.sdate.wDay + day_if_february;
			day_if_february = 0;
			HANDLE timer = CreateWaitableTimer(NULL, TRUE, NULL);
			if (!timer_count)
			{
				SystemTimeToFileTime(&task_info_thread.sdate, &file_time);
				LocalFileTimeToFileTime(&file_time, &file_time);
			}
			if (timer_count)
			{
				if (task_info_thread.sdate.wMonth <= 11 && task_info_thread.sdate.wMonth > 1)
					task_info_thread.sdate.wMonth++;
				else
				{
					if (task_info_thread.sdate.wMonth == 12)
					{
						task_info_thread.sdate.wYear++;
						task_info_thread.sdate.wMonth == 1;
					}
					else
					{
						if (task_info_thread.sdate.wDay > 28)
						{
							task_info_thread.sdate.wMonth++;
							day_if_february = task_info_thread.sdate.wDay - 28;
							task_info_thread.sdate.wDay = 28;
						}
					}
				}
				SystemTimeToFileTime(&task_info_thread.sdate, &file_time);
				LocalFileTimeToFileTime(&file_time, &file_time);
			}
			SetWaitableTimer(timer, (LARGE_INTEGER*)&file_time, 0, NULL, NULL, FALSE);
			SetEvent(hev);
			if (WaitForSingleObject(timer, INFINITE) == WAIT_OBJECT_0)
			{
				HINSTANCE shell_answer = ShellExecute(NULL, L"open", task_info_thread.path, NULL, NULL, SW_SHOW);
				int shell_answer_int = (int)shell_answer;
				switch (shell_answer_int)
				{
				case 0:
					lstrcpy(status_string, L" - Error: System has not enough memory or resurses!");
					break;
				case ERROR_BAD_FORMAT:
					lstrcpy(status_string, L" - Error: An attempt was made to load a program with an incorrect format!");
					break;
				case SE_ERR_ACCESSDENIED:
					lstrcpy(status_string, L" - Error: ACCESS DENIED!");
					break;
				case SE_ERR_DLLNOTFOUND:
					lstrcpy(status_string, L" - Error: DLL NOT FOUND!");
					break;
				case SE_ERR_ASSOCINCOMPLETE:
					lstrcpy(status_string, L" - Error: The file name association is incomplete or invalid!");
					break;
				case SE_ERR_FNF:
					lstrcpy(status_string, L" - Error: The specified file was not found!");
					break;
				case SE_ERR_NOASSOC:
					lstrcpy(status_string, L" - Error: There is no application associated with the given file name extension. !");
					break;
				case SE_ERR_OOM:
					lstrcpy(status_string, L" - Error: There was not enough memory to complete the operation!");
					break;
				case SE_ERR_PNF:
					lstrcpy(status_string, L" - Error: The specified path was not found!");
					break;
				case SE_ERR_SHARE:
					lstrcpy(status_string, L" - Error: A sharing violation occurred!");
					break;
				default:
					lstrcpy(status_string, L" - Successfully executed!");
					lstrcat(log_string, status_string);
				}
				CancelWaitableTimer(timer);
				CloseHandle(timer);
				timer_count = 1;
				lstrcpy(log_string, task_info_thread.path);
				lstrcat(log_string, status_string);
				SendMessage(list_log_thread, LB_ADDSTRING, 0, (LPARAM)log_string);
			}
		}
	}
	//////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////ONCE////////////////
	if (!lstrcmp(task_info_thread.period, L"Once"))
	{
		HANDLE timer = CreateWaitableTimer(NULL, TRUE, NULL);
			SystemTimeToFileTime(&task_info_thread.sdate, &file_time);
			LocalFileTimeToFileTime(&file_time, &file_time);
			SetWaitableTimer(timer, (LARGE_INTEGER*)&file_time, 0, NULL, NULL, FALSE);
			SetEvent(hev);
			if (WaitForSingleObject(timer, INFINITE) == WAIT_OBJECT_0)
			{
				HINSTANCE shell_answer = ShellExecute(NULL, L"open", task_info_thread.path, NULL, NULL, SW_SHOW);
				int shell_answer_int = (int)shell_answer;
				switch (shell_answer_int)
				{
				case 0:
					lstrcpy(status_string, L" - Error: System has not enough memory or resurses!");
					break;
				case ERROR_BAD_FORMAT:
					lstrcpy(status_string, L" - Error: An attempt was made to load a program with an incorrect format!");
					break;
				case SE_ERR_ACCESSDENIED:
					lstrcpy(status_string, L" - Error: ACCESS DENIED!");
					break;
				case SE_ERR_DLLNOTFOUND:
					lstrcpy(status_string, L" - Error: DLL NOT FOUND!");
					break;
				case SE_ERR_ASSOCINCOMPLETE:
					lstrcpy(status_string, L" - Error: The file name association is incomplete or invalid!");
					break;
				case SE_ERR_FNF:
					lstrcpy(status_string, L" - Error: The specified file was not found!");
					break;
				case SE_ERR_NOASSOC:
					lstrcpy(status_string, L" - Error: There is no application associated with the given file name extension. !");
					break;
				case SE_ERR_OOM:
					lstrcpy(status_string, L" - Error: There was not enough memory to complete the operation!");
					break;
				case SE_ERR_PNF:
					lstrcpy(status_string, L" - Error: The specified path was not found!");
					break;
				case SE_ERR_SHARE:
					lstrcpy(status_string, L" - Error: A sharing violation occurred!");
					break;
				default:
					lstrcpy(status_string, L" - Successfully executed!");
					lstrcat(log_string, status_string);
				}
				CancelWaitableTimer(timer);
				CloseHandle(timer);
				int count1 = SendMessage(list_thread, LB_GETCOUNT, 0, 0);
				for (size_t i = 0; i < count1; i++)
				{
					LRESULT task_from_list1 = SendMessage(list_thread, LB_GETITEMDATA, i, 0);
					task_info* task_from_list = (task_info*)task_from_list1;
					if(task_from_list->count == task_info_thread.count)
					{
						lstrcpy(log_string, task_from_list->path);
						lstrcat(log_string, status_string);
						SendMessage(list_thread, LB_DELETESTRING, i, 0);
						SendMessage(list_log_thread, LB_ADDSTRING, 0, (LPARAM)log_string);
						break;
					}
				}
			}
	}
	
	return 0;
}
///////////////////////////////////////////////////////////END_THREAD
//////////////////////////////////////////////////////////////////////
void task_scheduler::Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch (id)
	{
	case ID_TASKMENU_ADDTASK:
	{
		task_scheduler_second_window dlg;
		INT_PTR result = DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG2), hwnd, (DLGPROC)task_scheduler_second_window::dlgproc);
		if (result)
		{
			lstrcpy(tasks_mas[count].path, dlg.task_state_second_window.path);
			lstrcpy(tasks_mas[count].period, dlg.task_state_second_window.period);
			tasks_mas[count].stime = dlg.task_state_second_window.stime;
			tasks_mas[count].sdate = dlg.task_state_second_window.sdate;
			tasks_mas[count].sdate.wHour = tasks_mas[count].stime.wHour;
			tasks_mas[count].sdate.wMinute = tasks_mas[count].stime.wMinute;
			tasks_mas[count].sdate.wSecond = tasks_mas[count].stime.wSecond;
			tasks_mas[count].sdate.wMilliseconds = tasks_mas[count].stime.wMilliseconds;
			tasks_mas[count].count = count;
			for (size_t i = 0; i < 7; i++)
			{
				tasks_mas[count].days_check[i] = dlg.task_state_second_window.days_check[i];
			}
			SendMessage(list_of_processes, LB_ADDSTRING, 0, (LPARAM)tasks_mas[count].path);
			int count_ = SendMessage(list_of_processes, LB_GETCOUNT, 0, 0);
			
			HANDLE h = CreateThread(NULL, 0, task_thread, this, CREATE_SUSPENDED, &tasks_mas[count].th_ID);
			SendMessage(list_of_processes, LB_SETITEMDATA, count_ - 1, (LPARAM)&tasks_mas[count]);
			ResumeThread(h);
			CloseHandle(h);
			HANDLE hev;
			if (!load_s)
			{
				hev = CreateEvent(NULL, TRUE, FALSE, L"{ADC3AF8E-6133-4792-9D15-94E27BC457A6}");
			}
			else
			{
				hev = OpenEvent(EVENT_ALL_ACCESS, 0, L"{ADC3AF8E-6133-4792-9D15-94E27BC457A6}");
				ResetEvent(hev);
			}
			if (WaitForSingleObject(hev, INFINITE) == WAIT_OBJECT_0)
			{
				count++;
				load_s = true;
			}
		}
		break;
	}
	case ID_TASKMENU_EDIT:
	{
		int quan = SendMessage(list_of_processes, LB_GETCOUNT, 0, 0);
			int index = SendMessage(list_of_processes, LB_GETCURSEL, 0, 0);
			if (quan > 0)
			{
				if (index > -1)
				{
					LRESULT task_from_list1 = SendMessage(list_of_processes, LB_GETITEMDATA, index, 0);
					task_info* task_from_list = (task_info*)task_from_list1;
					task_scheduler_second_window dlg(*task_from_list);
					int count_edit = task_from_list->count;
					INT_PTR result = DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG2), hwnd, (DLGPROC)task_scheduler_second_window::dlgproc);
					if (result)
					{
						HANDLE h_th = OpenThread(THREAD_ALL_ACCESS, FALSE, task_from_list->th_ID);
						TerminateThread(h_th, 0);
						CloseHandle(h_th);
						lstrcpy(tasks_mas[count_edit].path, dlg.task_state_second_window.path);
						lstrcpy(tasks_mas[count_edit].period, dlg.task_state_second_window.period);
						tasks_mas[count_edit].stime = dlg.task_state_second_window.stime;
						tasks_mas[count_edit].sdate = dlg.task_state_second_window.sdate;
						tasks_mas[count_edit].sdate.wHour = tasks_mas[count_edit].stime.wHour;
						tasks_mas[count_edit].sdate.wMinute = tasks_mas[count_edit].stime.wMinute;
						tasks_mas[count_edit].sdate.wSecond = tasks_mas[count_edit].stime.wSecond;
						tasks_mas[count_edit].sdate.wMilliseconds = tasks_mas[count_edit].stime.wMilliseconds;
						for (size_t i = 0; i < 7; i++)
						{
							tasks_mas[count_edit].days_check[i] = dlg.task_state_second_window.days_check[i];
						}
						int count_ = SendMessage(list_of_processes, LB_GETCOUNT, 0, 0);
						HANDLE h = CreateThread(NULL, 0, task_thread, this, CREATE_SUSPENDED, &tasks_mas[count_edit].th_ID);
						SendMessage(list_of_processes, LB_SETITEMDATA, count_edit, (LPARAM)&tasks_mas[count_edit]);
						tasks_mas[count_edit].count = count_edit;
						int count_memory = count;
						count = count_edit;
						ResumeThread(h);
						Sleep(100);
						CloseHandle(h);
						count = count_memory;
					}
				}
			}
		break;
	}
	case ID_TASKMENU_DELETETASK:
	{
		int quan = SendMessage(list_of_processes, LB_GETCOUNT, 0, 0);
		int ind = SendMessage(list_of_processes, LB_GETCURSEL, 0, 0);
		if (quan > 0)
		{
			if (ind > -1)
			{
				int index = SendMessage(list_of_processes, LB_GETCURSEL, 0, 0);
				LRESULT task_from_list1 = SendMessage(list_of_processes, LB_GETITEMDATA, index, 0);
				task_info* task_from_list = (task_info*)task_from_list1;
				task_scheduler_second_window dlg(*task_from_list);
				HANDLE h_th1 = OpenThread(THREAD_ALL_ACCESS, FALSE, task_from_list->th_ID);
				SendMessage(list_of_processes, LB_DELETESTRING, index, 0);
				TerminateThread(h_th1, 0);
				CloseHandle(h_th1);
			}
		}
		break;
	}
	case ID_TASKMENU_ERASELOGLIST:
		SendMessage(list_of_logs, LB_RESETCONTENT, 0, 0);
		break;
	case ID_TASKMENU_EXIT:
		save();
		EndDialog(hwnd, 0);
		break;
	}
}
void task_scheduler::Cls_OnSize(HWND hwnd, UINT state, int cx, int cy)
{
	if (state == SIZE_MINIMIZED)
	{
		ShowWindow(hwnd, SW_HIDE); // Прячем окно
		Shell_NotifyIcon(NIM_ADD, tray_icon); // Добавляем иконку в трэй
	}
}
// обработчик пользовательского сообщения
void task_scheduler::OnTrayIcon(WPARAM wp, LPARAM lp)
{
	// WPARAM - идентификатор иконки
	// LPARAM - сообщение от мыши или клавиатурное сообщение
	if (lp == WM_LBUTTONDBLCLK)
	{
		Shell_NotifyIcon(NIM_DELETE, tray_icon); // Удаляем иконку из трэя
		ShowWindow(main_dialog, SW_NORMAL); // Восстанавливаем окно
		SetForegroundWindow(main_dialog); // устанавливаем окно на передний план
	}
}
BOOL CALLBACK task_scheduler::dlgproc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		HANDLE_MSG(hwnd, WM_CLOSE, ptr->Cls_OnClose);
		HANDLE_MSG(hwnd, WM_INITDIALOG, ptr->Cls_OnInitDialog);
		HANDLE_MSG(hwnd, WM_COMMAND, ptr->Cls_OnCommand);
		HANDLE_MSG(hwnd, WM_SIZE, ptr->Cls_OnSize);
	}
	if (message == WM_ICON)
	{
		ptr->OnTrayIcon(wParam, lParam);
		return TRUE;
	}
	return FALSE;
}
void task_scheduler::save()
{
	HKEY key;
	LONG reg_result;
	TCHAR number[10];
	TCHAR number2[10];
	TCHAR str[MAX_PATH];

	reg_result = RegCreateKeyEx( HKEY_CURRENT_USER,	L"Software\\task_scheduler", 0, 0, 0, KEY_ALL_ACCESS, 0, &key, 0);
	if (reg_result != ERROR_SUCCESS)
	{
		return;
	}
	int quantity = SendMessage(list_of_processes, LB_GETCOUNT, 0, 0);
		RegSetValueEx(key, L"Quantity", 0, REG_DWORD, (const BYTE*)&quantity, sizeof(quantity));
		if (quantity > 0)
		{
			for (size_t i = 0; i < quantity; i++)
			{
				LRESULT task_from_list1 = SendMessage(list_of_processes, LB_GETITEMDATA, i, 0);
				task_info* task_from_list = (task_info*)task_from_list1;
				wsprintf(number, L"number%d", i);
				reg_result = RegSetValueEx(key, number, 0, REG_BINARY, (const BYTE*)task_from_list, sizeof(task_info));
			}
		}
		int quantity_logs = SendMessage(list_of_logs, LB_GETCOUNT, 0, 0);
		RegSetValueEx(key, L"Quantity_logs", 0, REG_DWORD, (const BYTE*)&quantity_logs, sizeof(quantity_logs));
		if (quantity_logs > 0)
		{
			for (size_t i = 0; i < quantity_logs; i++)
			{
				SendMessage(list_of_logs, LB_GETTEXT, i, (LPARAM)str);
				wsprintf(number2, L"number2%d", i);
				reg_result = RegSetValueEx(key, number2, 0, REG_SZ, (const BYTE*)str, MAX_PATH);
			}
		}
	RegCloseKey(key);
}
void task_scheduler::load(HWND hwnd)
{
	HKEY key;
	LONG reg_result;
	TCHAR number[10];
	TCHAR number2[10];
	reg_result = RegCreateKeyEx(HKEY_CURRENT_USER, L"Software\\task_scheduler", 0, 0, 0, KEY_ALL_ACCESS, 0, &key, 0);
	if (reg_result != ERROR_SUCCESS)
	{
		MessageBox(hwnd, L"Couldn't proceed loading data or saved program data is corrupted!", L"Error", IDOK);
	}
	else
	{
		HANDLE hev;
		DWORD quantity;
		DWORD quantity_logs;
		DWORD size = sizeof(DWORD);
		task_info task_state_load;
		DWORD size_task = sizeof(task_info);
		reg_result = RegQueryValueEx(key, L"Quantity", 0, 0, (BYTE*)&quantity, &size);
		if (reg_result == ERROR_SUCCESS)
		{
			if (quantity > 0)
			{
				for (size_t i = 0; i < quantity; i++)
				{
					wsprintf(number, L"number%d", i);
					reg_result = RegQueryValueEx(key, number, 0, 0, (BYTE*)&task_state_main_window, &size_task);

					lstrcpy(tasks_mas[count].path, task_state_main_window.path);
					lstrcpy(tasks_mas[count].period, task_state_main_window.period);
					tasks_mas[count].stime = task_state_main_window.stime;
					tasks_mas[count].sdate = task_state_main_window.sdate;
					tasks_mas[count].sdate.wHour = tasks_mas[count].stime.wHour;
					tasks_mas[count].sdate.wMinute = tasks_mas[count].stime.wMinute;
					tasks_mas[count].sdate.wSecond = tasks_mas[count].stime.wSecond;
					tasks_mas[count].sdate.wMilliseconds = tasks_mas[count].stime.wMilliseconds;
					tasks_mas[count].count = count;
					for (size_t i = 0; i < 7; i++)
					{
						tasks_mas[count].days_check[i] = task_state_main_window.days_check[i];
					}
					SendMessage(list_of_processes, LB_ADDSTRING, 0, (LPARAM)tasks_mas[count].path);
					int count_ = SendMessage(list_of_processes, LB_GETCOUNT, 0, 0);
					HANDLE h = CreateThread(NULL, 0, task_thread, this, CREATE_SUSPENDED, &tasks_mas[count].th_ID);
					SendMessage(list_of_processes, LB_SETITEMDATA, count_ - 1, (LPARAM)&tasks_mas[count]);
					ResumeThread(h);
					CloseHandle(h);
					if (!load_s)
					{
						hev = CreateEvent(NULL, TRUE, FALSE, L"{ADC3AF8E-6133-4792-9D15-94E27BC457A6}");
					}
					else
					{
						ResetEvent(hev);
					}
					if (WaitForSingleObject(hev, INFINITE) == WAIT_OBJECT_0)
						count++;
					load_s = true;
				}
			}
		}
		DWORD len = MAX_PATH;
		reg_result = RegQueryValueEx(key, L"Quantity_logs", 0, 0, (BYTE*)&quantity_logs, &size);
		if (reg_result == ERROR_SUCCESS)
		{
			if (quantity_logs > 0)
			{
				for (size_t i = 0; i < quantity_logs; i++)
				{
					TCHAR str [MAX_PATH];
					ZeroMemory(str, MAX_PATH);
					wsprintf(number2, L"number2%d", i);
					reg_result = RegQueryValueEx(key, number2, 0, 0, (BYTE*)&str, &len);
					if (reg_result == ERROR_SUCCESS)
					{
						SendMessage(list_of_logs, LB_ADDSTRING, 0, (LPARAM)str);
					}
					len = MAX_PATH;
				}
			}
		}
	}
	RegCloseKey(key);
}
