#include "ELog.h"
#pragma warning(disable: 6387)

static HANDLE  hSdt = nullptr;
static HWND    hWnd = nullptr;
static std::mutex mtx;

bool WINAPI ELoggerInit(const char* consoleTitle, const int nWidth, const int nHeight)
{
	if (!AllocConsole()) return false;
	hSdt = GetStdHandle(STD_OUTPUT_HANDLE);
    if (!hSdt) return false;

	// 获取控制台窗口句柄
	hWnd = GetConsoleWindow();
	if (!hWnd) return false;

	// 设置中文输出不乱码
	setlocale(LC_CTYPE, "");

    LONG style = GetWindowLong(hWnd, GWL_STYLE);
	style = style & ~WS_MINIMIZEBOX;
	style = style & ~WS_MAXIMIZEBOX;
	SetWindowLong(hWnd, GWL_STYLE, style);
	SetWindowLong(hWnd, GWL_EXSTYLE, WS_EX_TOOLWINDOW);

	// 关闭调试用控制台退出按钮
	DeleteMenu(GetSystemMenu(hWnd, FALSE), SC_CLOSE, MF_BYCOMMAND);
	
	// 设置调试控制台标题为空
	SetConsoleTitleA(consoleTitle);
	
	// 移动调试控制台到右下角
	MoveWindow(
		hWnd,
		GetSystemMetrics(SM_CXFULLSCREEN) - nWidth,
		GetSystemMetrics(SM_CYFULLSCREEN) - nHeight,
		nWidth,
		nHeight + GetSystemMetrics(SM_CYSIZE),
		true
	);

	// 移除快速编辑模式,插入模式
    const HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
	if (!hStdin) return false;

	DWORD mode;
    if (!GetConsoleMode(hStdin, &mode)) return false;
	mode &= ~ENABLE_QUICK_EDIT_MODE;
	mode &= ~ENABLE_INSERT_MODE;
	mode &= ~ENABLE_MOUSE_INPUT;
	return SetConsoleMode(hStdin, mode);
}

bool WINAPI ELoggerFree()
{
	return FreeConsole();
}

static void ConsoleWrite(const char* content, const size_t len)
{
	WriteConsoleA(hSdt, content, len, nullptr, nullptr);
}

static void Print(const char* format, const char* name, const unsigned char color, ...)
{
	mtx.lock();
	if (format != nullptr && hSdt != nullptr)
	{
		SYSTEMTIME st;
        st = {0};
        GetLocalTime(&st);
		char date[30] = { 0 };
		sprintf_s(date, "%d-%02d-%02d %02d:%02d:%02d.%03d",
			st.wYear,
			st.wMonth,
			st.wDay,
			st.wHour,
			st.wMinute,
			st.wSecond,
			st.wMilliseconds
		);
		SetConsoleTextAttribute(hSdt, color);
		ConsoleWrite(date, strlen(date));

		SetConsoleTextAttribute(hSdt, color);
		ConsoleWrite(name, strlen(name));

		va_list argList;
		va_start(argList, format);
		size_t length = static_cast<size_t>(_vscprintf(format, argList)) + 1;
        auto buffer = new char[length];
		vsprintf_s(buffer, length, format, argList);
		SetConsoleTextAttribute(hSdt, color);
		ConsoleWrite(buffer, length);
		va_end(argList);
		ConsoleWrite("\n", 1);
		delete[]buffer;
	}
	mtx.unlock();
}

void WINAPI ELoggerSucc(const char* format)
{
	if (format != nullptr)
	{
		va_list argList;
		va_start(argList, format);
		size_t length = _vscprintf(format, argList) + 1;
        auto buffer = new char[length];
		vsprintf_s(buffer, length, format, argList);
		Print(buffer, " < 信息 > ", 白色);
		va_end(argList);
		delete[]buffer;
	}
}

void WINAPI ELoggerWarning(const char* format)
{
	if (format != nullptr) {
		va_list argList;
		va_start(argList, format);
		size_t length = _vscprintf(format, argList) + 1;
        auto buffer = new char[length];
		vsprintf_s(buffer, length, format, argList);
		Print(buffer, " < 警告 > ", 黄色);
		va_end(argList);
		delete[]buffer;
	}
}

void WINAPI ELoggerFailure(const char* format)
{
	if (format != nullptr) {

		va_list argList;
		va_start(argList, format);
		size_t length = _vscprintf(format, argList) + 1;
        auto buffer = new char[length];
		vsprintf_s(buffer, length, format, argList);
		Print(buffer, " < 错误 > ", 红色);
		va_end(argList);
		delete[]buffer;
	}
}

void WINAPI ELoggerDebug(const char* format)
{
	if (format != nullptr) {

		va_list argList;
		va_start(argList, format);
		size_t length = _vscprintf(format, argList) + 1;
		auto buffer = new char[length];
		vsprintf_s(buffer, length, format, argList);
		Print(buffer, " < 调试 > ", 蓝色);
		va_end(argList);
		delete[]buffer;
	}
}