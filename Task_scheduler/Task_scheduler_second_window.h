#pragma once
#include "Header.h"
class task_scheduler_second_window
{
public:
	task_scheduler_second_window();
	task_scheduler_second_window(task_info task_state);
	bool constr_with_parametr;
public:
	static BOOL CALLBACK dlgproc(HWND hwnd, UINT mes, WPARAM wp, LPARAM lp);
	static task_scheduler_second_window* ptr;
	BOOL Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	void Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
	void Cls_OnClose(HWND hwnd);
	HWND open_button, h_period, h_path, cancel, time_picker, date_picker;
	HWND days[7];
	bool new_task_parametrs_check = false;
	task_info task_state_second_window;
};