#include "ELog.h"
#pragma warning(disable: 6387)

static HANDLE hSdt = nullptr;
static HWND hWnd = nullptr;
static std::mutex mtx;

bool WINAPI ELoggerInit(const char* consoleTitle, const int nWidth, const int nHeight)
{
    if (!AllocConsole()) return false;
    hSdt = GetStdHandle(STD_OUTPUT_HANDLE);
    if (!hSdt) return false;

    const HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    if (!hStdin) return false;

    DWORD mode;
    if (!GetConsoleMode(hStdin, &mode)) return false;
    mode &= ~ENABLE_QUICK_EDIT_MODE;
    mode &= ~ENABLE_INSERT_MODE;
    mode &= ~ENABLE_MOUSE_INPUT;
    mode &= ~ENABLE_WINDOW_INPUT;
    if (!SetConsoleMode(hStdin, mode))
        return false;

    CONSOLE_FONT_INFOEX cfi;
    cfi.cbSize = sizeof cfi;
    cfi.nFont = 0;
    cfi.dwFontSize.X = 0;
    cfi.dwFontSize.Y = 15;
    cfi.FontFamily = FF_DONTCARE;
    cfi.FontWeight = FW_NORMAL;
    wcscpy_s(cfi.FaceName, L"YaHei Consolas Hybrid");
    SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &cfi);
    return true;
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
        char date[30] = {0};
        sprintf_s(date, " [ %02d:%02d:%02d ] ",
                  st.wHour,
                  st.wMinute,
                  st.wSecond
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

static void PrintNoDate(const char* format, const char* name, const unsigned char color, ...)
{
    mtx.lock();
    if (format != nullptr && hSdt != nullptr)
    {
        SYSTEMTIME st;
        st = {0};
        GetLocalTime(&st);

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
        Print(buffer, "<信息> ", 绿色);
        va_end(argList);
        delete[]buffer;
    }
}

void WINAPI ELoggerWarning(const char* format)
{
    if (format != nullptr)
    {
        va_list argList;
        va_start(argList, format);
        size_t length = _vscprintf(format, argList) + 1;
        auto buffer = new char[length];
        vsprintf_s(buffer, length, format, argList);
        Print(buffer, "<警告> ", 黄色);
        va_end(argList);
        delete[]buffer;
    }
}

void WINAPI ELoggerFailure(const char* format)
{
    if (format != nullptr)
    {
        va_list argList;
        va_start(argList, format);
        size_t length = _vscprintf(format, argList) + 1;
        auto buffer = new char[length];
        vsprintf_s(buffer, length, format, argList);
        Print(buffer, "<错误> ", 红色);
        va_end(argList);
        delete[]buffer;
    }
}

void WINAPI ELoggerDebug(const char* format)
{
    if (format != nullptr)
    {
        va_list argList;
        va_start(argList, format);
        size_t length = _vscprintf(format, argList) + 1;
        auto buffer = new char[length];
        vsprintf_s(buffer, length, format, argList);
        Print(buffer, "<调试> ", 蓝色);
        va_end(argList);
        delete[]buffer;
    }
}

void WINAPI ELoggerCustom(const char* type, const char* format, const unsigned char color)
{
    if (format != nullptr)
    {
        va_list argList;
        va_start(argList, format);
        size_t length = _vscprintf(format, argList) + 1;
        auto buffer = new char[length];
        vsprintf_s(buffer, length, format, argList);

        size_t byteBufferLength = _vscprintf(type, argList) + 1;
        auto typeBuffer = new char[byteBufferLength];
        sprintf(typeBuffer, "<%s> ", type);

        Print(buffer, typeBuffer, color);
        va_end(argList);
        delete[]buffer;
    }
}

void WINAPI ELoggerEmpty()
{
    const char* buffer = nullptr;
    PrintNoDate("", "", 青色);
    delete[]buffer;
}
