#pragma once
#include "Header.h"
#include "Task_scheduler_second_window.h"
#include <vector>
using namespace std;
class task_scheduler
{
public:
	task_scheduler();
public:
	~task_scheduler();
	static BOOL CALLBACK dlgproc(HWND hwnd, UINT mes, WPARAM wp, LPARAM lp);
	static task_scheduler* ptr;
	BOOL Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
	void Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
	void Cls_OnClose(HWND hwnd);
	void Cls_OnSize(HWND hwnd, UINT state, int cx, int cy);
	void OnTrayIcon(WPARAM wp, LPARAM lp); // обработчик пользовательского сообщения
	HICON icon;
	PNOTIFYICONDATA tray_icon;
	HWND list_of_processes;
	HWND list_of_logs;
	HWND main_dialog;
	task_info task_state_main_window;
	static int count;
	void save();
	void load(HWND);
	task_info tasks_mas[64];
};