
#include "framework.h"
#include "minesweeper.h"
#include <algorithm>
#include <chrono>
#include <random>
#include <vector>

#define MAX_LOADSTRING 100
#define BOX_SIZE 25
#define MAX_SIZEY 24
#define MAX_SIZEX 30
#define MIN_MINE_AMOUNT 10
#define MIN_SIZE 10
#define PANEL_HEIGHT 40
#define FONT_SIZE 25
#define TIMER_ID 3
#define MINE -1
#define IDB_BITMAP1 130

HINSTANCE hInst;                                
WCHAR szTitle[MAX_LOADSTRING];                  
WCHAR szWindowClass[MAX_LOADSTRING];            
int sizeX = 10;
int sizeY = 10;
int windowHeight;
int windowWidth;
unsigned long long currentTime = 0;
int screenWidth;
int screenHeight;
long long start;
unsigned long long timer0 = 0;
bool count = false;
int MineAmount = MIN_MINE_AMOUNT;
int CurrentMineAmount = MineAmount;
int MineArr[MAX_SIZEX][MAX_SIZEY]; // -1 = mine
bool FlagArr[MAX_SIZEX][MAX_SIZEY];
bool Visible [MAX_SIZEX][MAX_SIZEY];
HWND BoxArr[MAX_SIZEX][MAX_SIZEY];
HWND WindowMain;
bool isDebug = false;
bool isTimerOn = false;

ATOM                MyRegisterClass(HINSTANCE hInstance);
ATOM                MyBoxClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    BoxProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    Dialog(HWND, UINT, WPARAM, LPARAM);
void                CreateBoxes(HWND);
void                newGame(HWND hWnd);
void                PaintBoxes();
void                initGame();
void                flagBox(int x, int y);
void                getPos(int*,int*,HWND);
void                RepaintPanel();
void                handleClick(int, int);
void                PaintBox(int, int);
void funcTimer(HWND hWnd, UINT unnamedParam2,UINT_PTR unnamedParam3,DWORD unnamedParam4);
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MINESWEEPER, szWindowClass, MAX_LOADSTRING);
    MyBoxClass(hInstance);
    MyRegisterClass(hInstance);

    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MINESWEEPER));

    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}


ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MINESWEEPER));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_MINESWEEPER);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

ATOM MyBoxClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = BoxProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MINESWEEPER));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_GRAYTEXT+1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_MINESWEEPER);
    wcex.lpszClassName = L"BoxClass";
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}


BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance;
   screenHeight = GetSystemMetrics(SM_CYSCREEN);
   screenWidth = GetSystemMetrics(SM_CXSCREEN);
   windowHeight = BOX_SIZE * sizeY + (sizeY)+ PANEL_HEIGHT;
   windowWidth = BOX_SIZE * sizeX + (sizeX);
   RECT rs;
   rs.top = screenHeight / 2 - windowHeight / 2;
   rs.bottom = screenHeight / 2 + windowHeight / 2;
   rs.left = screenWidth / 2 - windowWidth / 2;
   rs.right = screenWidth / 2 + windowWidth / 2;
   AdjustWindowRect(&rs,WS_OVERLAPPEDWINDOW,TRUE);

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
       rs.left, rs.top, rs.right-rs.left, rs.bottom-rs.top, nullptr, nullptr, hInstance, nullptr);
   WindowMain = hWnd;
   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case ID_GAME_NEW:
                newGame(hWnd);
                PaintBoxes();
                break;
            case ID_GAME_EXIT:
                DestroyWindow(hWnd);
                break;
            case ID_HELP_DEBUG:
            {
                HMENU hMenu = GetMenu(hWnd);
                if (isDebug)
                {
                    CheckMenuItem(hMenu, ID_HELP_DEBUG, MF_UNCHECKED);
                    isDebug = false;
                    PaintBoxes();
                }
                else
                {
                    CheckMenuItem(hMenu, ID_HELP_DEBUG, MF_CHECKED);
                    isDebug = true;
                    PaintBoxes();
                }
            }
                break;
            case ID_GAME_CUSTOMSIZE:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, Dialog);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_CREATE:
        CreateBoxes(hWnd);
        newGame(hWnd);
        PaintBoxes();
        SetTimer(hWnd, 1, 100, NULL);
        timer0 = GetTickCount64();
        break;
    case WM_TIMER:

        if (count) RepaintPanel();
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
            RepaintPanel();
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK BoxProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        int x, y;
        getPos(&x, &y, hWnd);
        PaintBox(x, y);
    }
    break;
    case WM_RBUTTONUP:
    {
        int x, y;
        getPos(&x, &y, hWnd);
        if(!Visible[x][y]) flagBox(x, y);
    }
        break;
    case WM_LBUTTONUP:
    {
        int x, y;
        getPos(&x, &y, hWnd);
        if (!(FlagArr[x][y] || Visible[x][y])) handleClick(x, y);
    }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}


INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

INT_PTR CALLBACK Dialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG: 
    {
        //https://stackoverflow.com/questions/19885770/convert-int-to-lpcwstr-by-using-wsprintf
        wchar_t text1[256];
        wchar_t text2[256];
        wchar_t text3[256];
        wsprintfW(text1, L"%d", sizeY);
        wsprintfW(text2, L"%d", sizeX);
        wsprintfW(text3, L"%d", MineAmount);
        HWND d1 = GetDlgItem(hDlg, IDC_EDIT1);
        HWND d2 = GetDlgItem(hDlg, IDC_EDIT2);
        HWND d3 = GetDlgItem(hDlg, IDC_EDIT3);
        SetWindowText(d1, text1);
        SetWindowText(d2, text2);
        SetWindowText(d3, text3);
        return (INT_PTR)TRUE;
    }
        break;
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK)
        {
            HWND d1 = GetDlgItem(hDlg, IDC_EDIT1);
            HWND d2 = GetDlgItem(hDlg, IDC_EDIT2);
            HWND d3 = GetDlgItem(hDlg, IDC_EDIT3);
            LPWSTR s1 = new TCHAR[100];
            LPWSTR s2 = new TCHAR[100];
            LPWSTR s3 = new TCHAR[100];
            GetWindowText(d1, s1, 100);
            GetWindowText(d2, s2, 100);
            GetWindowText(d3, s3, 100);
            
            sizeY       = _wtoi(s1) > MAX_SIZEY       ? MAX_SIZEY         : (_wtoi(s1) < MIN_SIZE ? MIN_SIZE : _wtoi(s1));
            sizeX       = _wtoi(s2) > MAX_SIZEX       ? MAX_SIZEY         : (_wtoi(s2) < MIN_SIZE ? MIN_SIZE : _wtoi(s2));
            MineAmount  = _wtoi(s3) < MIN_MINE_AMOUNT ? MIN_MINE_AMOUNT   : _wtoi(s3);
            CurrentMineAmount = MineAmount;
            
            EndDialog(hDlg, LOWORD(wParam));
            newGame(GetParent(hDlg));
            PaintBoxes();
            RepaintPanel();
            return (INT_PTR)TRUE;
        }
        if (LOWORD(wParam) == IDCANCEL) {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

void CreateBoxes(HWND hWnd) 
{
    for (int x = 0; x < MAX_SIZEX; x++)
        for (int y = 0; y < MAX_SIZEY; y++)
        {
            BoxArr[x][y] = CreateWindowEx(0, L"BoxClass", L"box", WS_CHILD | WS_VISIBLE,
                x * 26, y * 26 + PANEL_HEIGHT, BOX_SIZE, BOX_SIZE, hWnd, NULL, hInst, NULL);
        }         
}

void newGame(HWND hWnd) 
{
    windowHeight = BOX_SIZE * sizeY + (sizeY ) + PANEL_HEIGHT;
    windowWidth  = BOX_SIZE * sizeX + (sizeX);
    RECT rs;
    rs.top = screenHeight / 2 - windowHeight / 2;
    rs.bottom = screenHeight / 2 + windowHeight / 2;
    rs.left = screenWidth / 2 - windowWidth / 2;
    rs.right = screenWidth / 2 + windowWidth / 2;
    AdjustWindowRect(&rs, WS_OVERLAPPEDWINDOW, TRUE);
    MoveWindow(hWnd, rs.left, rs.top, rs.right - rs.left, rs.bottom - rs.top, TRUE);
    initGame();
    PaintBoxes();
    CurrentMineAmount = MineAmount;
    currentTime = 0;
    count = false;
    RepaintPanel();
}

void initGame()
{
    for (int x = 0; x < MAX_SIZEX; x++)
        for (int y = 0; y < MAX_SIZEY; y++)
        {
            MineArr[x][y] = 0;
            FlagArr[x][y] = false;
            Visible[x][y] = false;
        }
            
    std::vector<int> numbers; //random unique bomb places https://stackoverflow.com/questions/36922371/generate-different-random-numbers
    for (int i = 0; i < sizeX*sizeY; i++)
        numbers.push_back(i);
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::shuffle(numbers.begin(), numbers.end(), std::default_random_engine(seed));
    for (int i = 0; i < sizeX * sizeY - MineAmount; i++)
        numbers.pop_back();
    
    std::for_each(numbers.begin(), numbers.end(), [](int number) {
        int x = number % sizeX;
        int y = number / sizeX;
        MineArr[x][y] = MINE;
        });

    
    for(int x=0;x<sizeX;x++)
        for (int y = 0; y < sizeY; y++)
            if (MineArr[x][y] != MINE) 
            {
                int a = 0;
                if (x > 0 && MineArr[x - 1][y] == MINE) a++;
                if (x > 0 && y > 0 && MineArr[x - 1][y - 1] == MINE) a++;
                if (x > 0 && y+1 != sizeY && MineArr[x - 1][y + 1] == MINE) a++;
                if (y > 0 && MineArr[x][y - 1] == MINE) a++;
                if (y + 1 != sizeY && MineArr[x][y + 1] == MINE) a++;
                if (x + 1 != sizeX && MineArr[x + 1][y] == MINE) a++;
                if (y > 0 && x + 1 != sizeX && MineArr[x + 1][y - 1] == MINE) a++;
                if (y + 1 != sizeY && x + 1 != sizeX  && MineArr[x + 1][y + 1] == MINE) a++;
                MineArr[x][y] = a;
            }
}

void PaintFlag(int x, int y) 
{
    HDC hdc = GetDC(BoxArr[x][y]);
    HBITMAP bitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP1));
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, bitmap);
    BITMAP bmInfo;
    GetObject(bitmap, sizeof(bmInfo), &bmInfo);
    BitBlt(hdc, 0, 0, 20, 20, memDC, 0, 0, SRCCOPY);
    StretchBlt(hdc, 0, 0, 25, 25, memDC, 0, 0, bmInfo.bmWidth, bmInfo.bmHeight, SRCCOPY);
    DeleteObject(bitmap);
    ReleaseDC(BoxArr[x][y], hdc);
}

void PaintBox(int x, int y) 
{

    if (FlagArr[x][y]) {
        PaintFlag(x, y);
        return;
    }
    HDC hdc = GetDC(BoxArr[x][y]);
    HBRUSH brush = Visible[x][y] ? CreateSolidBrush(RGB(0xC8,0xC8,0xC8)) : (HBRUSH)(COLOR_GRAYTEXT + 1);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
    Rectangle(hdc, 0, 0, 25, 25);
    SelectObject(hdc, oldBrush);
    DeleteObject(brush);
    ReleaseDC(BoxArr[x][y], hdc);
    if (!Visible[x][y] && !isDebug) return;
    switch (MineArr[x][y])
    {
    case MINE:
    {
        HDC hdc = GetDC(BoxArr[x][y]);
        HBRUSH brush = CreateSolidBrush(RGB(0, 0, 0));
        HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
        Ellipse(hdc, 5, 5, 20, 20);
        SelectObject(hdc, oldBrush);
        DeleteObject(brush);
        ReleaseDC(BoxArr[x][y], hdc);
    }
    break;
    case 0:
        break;
    default:
        HDC hdc = GetDC(BoxArr[x][y]);
        wchar_t buffer[256];
        wsprintfW(buffer, L"%d", MineArr[x][y]);
        HFONT font = CreateFont(
            20,
            0,
            0,
            0,
            FW_BOLD,
            false,
            false,
            0,
            EASTEUROPE_CHARSET,
            OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY,
            DEFAULT_PITCH | FF_SWISS,
            _T("Arial")
        );
        HFONT oldFont = (HFONT)SelectObject(hdc, font);
        switch (MineArr[x][y]) {
        case 1:
            SetTextColor(hdc, RGB(0, 0, 0xFF));
            break;
        case 2:
            SetTextColor(hdc, RGB(0, 0xFF, 0));
            break;
        case 3:
            SetTextColor(hdc, RGB(0xFF, 0, 0));
            break;
        case 4:
            SetTextColor(hdc, RGB(216, 191, 216));
            break;
        case 5:
            SetTextColor(hdc, RGB(153, 0, 0));
            break;
        case 6:
            SetTextColor(hdc, RGB(0, 255, 255));
            break;
        case 7:
            SetTextColor(hdc, RGB(0, 0, 0));
            break;
        case 8:
            SetTextColor(hdc, RGB(102, 102, 153));
            break;
        }

        SetBkMode(hdc, TRANSPARENT);
        TextOut(hdc, 8, 4, buffer, (int)_tcslen(buffer));
        SelectObject(hdc, oldFont);
        DeleteObject(font);
        ReleaseDC(BoxArr[x][y], hdc);
        break;
    }
}

void PaintBoxes()
{
    for (int x = 0; x < MAX_SIZEX; x++)
        for (int y = 0; y < MAX_SIZEY; y++)
            PaintBox(x, y);
}

bool checkWin()
{
    int cnt = 0;
    int cnt2 = 0;
    int invis = 0;
    for (int x = 0; x < sizeX; x++)
        for (int y = 0; y < sizeY; y++)
            if (MineArr[x][y] == MINE && FlagArr[x][y]) cnt++;
    
    for (int x = 0; x < sizeX; x++)
        for (int y = 0; y < sizeY; y++)
        {
            if (!Visible[x][y]) invis++;
            if (!Visible[x][y] && MineArr[x][y] == MINE) cnt2++;
        }
            

    return cnt == MineAmount || (invis == MineAmount && cnt2 == MineAmount);
}

void WinPrompt()
{
    int msgboxID = MessageBox(
        NULL,
        (LPCWSTR)L"WIN!",
        (LPCWSTR)L"MineSweeper",
        MB_OK
    );
}

void makeVisible(int x, int y)
{
    Visible[x][y] = true;
    PaintBox(x, y);
}

void  flagBox(int x, int y)
{
    if (!count) {
        timer0 = GetTickCount64();
        count = true;
    }

    if (FlagArr[x][y]) 
    {
        CurrentMineAmount++;
        FlagArr[x][y] = false;
    }
    else
    {
        CurrentMineAmount--;
        FlagArr[x][y] = true;
    }

    PaintBox(x, y);
    RepaintPanel();

    if (checkWin()) {
        count = false;
        WinPrompt();
        for (int i = 0; i < sizeX; i++)
            for (int j = 0; j < sizeY; j++)
                makeVisible(i, j);
    }
}

void getPos(int* x1, int* y1, HWND hWnd) 
{
    for (int x = 0; x < MAX_SIZEX; x++)
        for (int y = 0; y < MAX_SIZEY; y++)
            if (hWnd == BoxArr[x][y]) 
            {
                *x1 = x;
                *y1 = y;
                return;
            }
}

void RepaintPanel()
{
    const int bufSize = 100;
    TCHAR buf[bufSize];
    TCHAR buf1[bufSize];

    HDC hdc = GetDC(WindowMain);
    Rectangle(hdc, 1, 1, windowWidth-2, PANEL_HEIGHT-2);
    if(count) currentTime = GetTickCount64() - timer0;
    wsprintfW(buf, L"%04d", CurrentMineAmount);
    wsprintfW(buf1, L"%04d.%d", int(currentTime/1000),  int(currentTime / 100) -int((currentTime / 1000)*10));
    HFONT font = CreateFont(
        25,
        0,
        0,
        0,
        FW_BOLD,
        false,
        false,
        0,
        EASTEUROPE_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_SWISS,
        _T("Arial")
    );
    HFONT oldFont = (HFONT)SelectObject(hdc, font);
    RECT rc;
    GetClientRect(WindowMain, &rc);
    SetTextColor(hdc, RGB(0xFF, 0, 0));
    TextOut(hdc, windowWidth / 4 - 25, PANEL_HEIGHT / 2 - FONT_SIZE / 2, buf1, (int)_tcslen(buf1));
    TextOut(hdc, 3 * windowWidth / 4 - 25, PANEL_HEIGHT / 2 - FONT_SIZE / 2, buf, (int)_tcslen(buf));
    SelectObject(hdc, oldFont);
    DeleteObject(font);
    ReleaseDC(WindowMain,hdc);
}

void BOOM()
{
    int msgboxID = MessageBox(
        NULL,
        (LPCWSTR)L"BOOM!",
        (LPCWSTR)L"MineSweeper",
        MB_ICONERROR | MB_OK
    );
}

void unCover(int x, int y)
{
    if (x < 0 || y < 0 || x >= sizeX || y >= sizeY || Visible[x][y]) return;
    switch (MineArr[x][y]) {
    case MINE:
        break;
    case 0:
        makeVisible(x,y);
        unCover(x + 1, y);
        unCover(x + 1, y+1);
        unCover(x + 1, y -1);
        unCover(x - 1, y -1);
        unCover(x - 1, y + 1);
        unCover(x, y + 1);
        unCover(x, y - 1);
        unCover(x - 1, y);
        break;
    default:
        makeVisible(x, y);
        return;
    }
}

void handleClick(int x, int y)
{
    if (!count) {
        timer0 = GetTickCount64();
        count = true;
    }
    switch (MineArr[x][y]) {
    case MINE:
        count = false;
        for (int i = 0; i < sizeX; i++)
            for (int j = 0; j < sizeY; j++)
                makeVisible(i, j);
        BOOM();
        break;
    case 0:
        unCover(x, y);
        break;
    default:
        makeVisible(x, y);
        break;
    }
    if (checkWin()) {
        count = false;
        WinPrompt();
        for (int i = 0; i < sizeX; i++)
            for (int j = 0; j < sizeY; j++)
                makeVisible(i, j);
    }
}

