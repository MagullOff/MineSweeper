
#include "framework.h"
#include "minesweeper.h"
#include <algorithm>
#include <chrono>
#include <random>
#include <vector>

#pragma region DEFINITIONS

#define MAX_LOADSTRING 100
#define BUFSIZE 100
#define BOMB_SIZE 15
#define BOMB_MARGIN 5
#define BOX_MARGIN 1
#define BOX_TOP_MARGIN 4
#define BOX_LEFT_MARGIN 8
#define TIMER_PERIOD 100
#define TIMER_ID 1
#define BOX_SIZE 25
#define MAX_SIZEY 24
#define MAX_SIZEX 30
#define MIN_MINE_AMOUNT 10
#define MIN_SIZE 6
#define DEFAULT_SIZE 10
#define PANEL_HEIGHT 40
#define FONT_SIZE 25
#define MINE -1
#define IDB_BITMAP1 130
#define FLAG_SIZE 20

HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];
int sizeX = DEFAULT_SIZE;
int sizeY = DEFAULT_SIZE;
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
bool Visible[MAX_SIZEX][MAX_SIZEY];
HWND BoxArr[MAX_SIZEX][MAX_SIZEY];
HWND WindowMain;
bool isDebug = false;
bool isTimerOn = false;
const COLORREF colorTab[9] = {
        RGB(0, 0, 0),
        RGB(0, 0, 0xFF),
        RGB(0, 0xFF, 0),
        RGB(0xFF, 0, 0),
        RGB(255,20,147),
        RGB(153, 0, 0),
        RGB(0, 255, 255),
        RGB(249, 215, 28),
        RGB(0, 0, 0)
};

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
void                flagBox(int, int);
void                getPos(int*, int*, HWND);
void                RepaintPanel();
void                handleClick(int, int);
void                PaintBox(int, int);
void                PaintFlag(int, int);
#pragma endregion

#pragma region INITIALISATION

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

    if (!InitInstance(hInstance, nCmdShow))
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

    return (int)msg.wParam;
}


ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MINESWEEPER));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_MINESWEEPER);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

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
    wcex.hbrBackground = (HBRUSH)(COLOR_GRAYTEXT + 1);
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
    windowHeight = BOX_SIZE * sizeY + (sizeY)+PANEL_HEIGHT;
    windowWidth = BOX_SIZE * sizeX + (sizeX);
    RECT rs;
    rs.top = screenHeight / 2 - windowHeight / 2;
    rs.bottom = screenHeight / 2 + windowHeight / 2;
    rs.left = screenWidth / 2 - windowWidth / 2;
    rs.right = screenWidth / 2 + windowWidth / 2;
    AdjustWindowRect(&rs, WS_OVERLAPPEDWINDOW, TRUE);

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        rs.left, rs.top, rs.right - rs.left, rs.bottom - rs.top, nullptr, nullptr, hInstance, nullptr);
    WindowMain = hWnd;
    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

void CreateBoxes(HWND hWnd)
{
    for (int x = 0; x < MAX_SIZEX; x++)
        for (int y = 0; y < MAX_SIZEY; y++)
            BoxArr[x][y] = CreateWindowEx(0, L"BoxClass", L"box", WS_CHILD | WS_VISIBLE,
                x * (BOX_SIZE + BOX_MARGIN), y * (BOX_SIZE+BOX_MARGIN) + PANEL_HEIGHT, BOX_SIZE, BOX_SIZE, hWnd, NULL, hInst, NULL);
}
#pragma endregion

#pragma region WINDOW_CALLBACK

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
        SetTimer(hWnd, TIMER_ID, TIMER_PERIOD, NULL);
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
        if (!Visible[x][y]) flagBox(x, y);
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
#pragma endregion

#pragma region POPUPS

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
        wchar_t text1[BUFSIZE];
        wchar_t text2[BUFSIZE];
        wchar_t text3[BUFSIZE];
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
            LPWSTR s1 = new TCHAR[BUFSIZE];
            LPWSTR s2 = new TCHAR[BUFSIZE];
            LPWSTR s3 = new TCHAR[BUFSIZE];
            GetWindowText(d1, s1, BUFSIZE);
            GetWindowText(d2, s2, BUFSIZE);
            GetWindowText(d3, s3, BUFSIZE);

            sizeY = _wtoi(s1) > MAX_SIZEY ? MAX_SIZEY : (_wtoi(s1) < MIN_SIZE ? MIN_SIZE : _wtoi(s1));
            sizeX = _wtoi(s2) > MAX_SIZEX ? MAX_SIZEY : (_wtoi(s2) < MIN_SIZE ? MIN_SIZE : _wtoi(s2));
            MineAmount = _wtoi(s3) < MIN_MINE_AMOUNT ? MIN_MINE_AMOUNT : _wtoi(s3);
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

#pragma endregion

#pragma region UTILS
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

void PaintBoxes()
{
    for (int x = 0; x < MAX_SIZEX; x++)
        for (int y = 0; y < MAX_SIZEY; y++)
            PaintBox(x, y);
}

void makeVisible(int x, int y)
{
    Visible[x][y] = true;
    PaintBox(x, y);
}

#pragma endregion

#pragma region PAINT

void RepaintPanel()
{
    TCHAR buf[BUFSIZE];
    TCHAR buf1[BUFSIZE];

    HDC hdc = GetDC(WindowMain);
    HFONT panelFont = CreateFont(
        FONT_SIZE,
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

    Rectangle(hdc, 1, 1, windowWidth - 2, PANEL_HEIGHT - 2);
    if (count) currentTime = GetTickCount64() - timer0;
    wsprintfW(buf, L"%04d", CurrentMineAmount);
    wsprintfW(buf1, L"%04d.%d", int(currentTime / 1000), int(currentTime / 100) - int((currentTime / 1000) * 10));
    HFONT font = panelFont;
    HFONT oldFont = (HFONT)SelectObject(hdc, font);
    RECT rc;
    GetClientRect(WindowMain, &rc);
    SetTextColor(hdc, RGB(0xFF, 0, 0));
    TextOut(hdc, windowWidth / 4 - 25, PANEL_HEIGHT / 2 - FONT_SIZE / 2, buf1, (int)_tcslen(buf1));
    TextOut(hdc, 3 * windowWidth / 4 - 25, PANEL_HEIGHT / 2 - FONT_SIZE / 2, buf, (int)_tcslen(buf));
    SelectObject(hdc, oldFont);
    DeleteObject(font);
    ReleaseDC(WindowMain, hdc);
}

void DrawMine(int x, int y)
{
    HDC hdc = GetDC(BoxArr[x][y]);
    HBRUSH brush = CreateSolidBrush(RGB(0, 0, 0));
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
    Ellipse(hdc, BOMB_MARGIN, BOMB_MARGIN, BOMB_MARGIN + BOMB_SIZE, BOMB_MARGIN + BOMB_SIZE);
    SelectObject(hdc, oldBrush);
    DeleteObject(brush);
    ReleaseDC(BoxArr[x][y], hdc);
}

void DrawBackGround(int x, int y)
{
    HDC hdc = GetDC(BoxArr[x][y]);
    HBRUSH brush = Visible[x][y] ? CreateSolidBrush(RGB(0xC8, 0xC8, 0xC8)) : (HBRUSH)(COLOR_GRAYTEXT + 1);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, brush);
    Rectangle(hdc, 0, 0, BOX_SIZE, BOX_SIZE);
    SelectObject(hdc, oldBrush);
    DeleteObject(brush);
    ReleaseDC(BoxArr[x][y], hdc);
}

void DrawNumber(int x, int y)
{
    HDC hdc = GetDC(BoxArr[x][y]);
    HFONT boxFont = CreateFont(
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
    wchar_t buffer[BUFSIZE];
    wsprintfW(buffer, L"%d", MineArr[x][y]);
    HFONT font = boxFont;
    HFONT oldFont = (HFONT)SelectObject(hdc, font);
    SetTextColor(hdc, colorTab[MineArr[x][y]]);
    SetBkMode(hdc, TRANSPARENT);
    TextOut(hdc, BOX_LEFT_MARGIN, BOX_TOP_MARGIN, buffer, (int)_tcslen(buffer));
    SelectObject(hdc, oldFont);
    DeleteObject(font);
    ReleaseDC(BoxArr[x][y], hdc);
}

void PaintBox(int x, int y)
{
    if (FlagArr[x][y])
        return PaintFlag(x, y);
        
    DrawBackGround(x, y);
    if (!Visible[x][y] && !isDebug) return;
    if (MineArr[x][y] == 0) return;
    if (MineArr[x][y] == MINE) DrawMine(x, y);
    else DrawNumber(x, y);
}

void PaintFlag(int x, int y)
{
    DrawBackGround(x, y);
    HDC hdc = GetDC(BoxArr[x][y]);
    HBITMAP bitmap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP1));
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, bitmap);
    // https://forum.dobreprogramy.pl/t/winapi-wyswietlanie-bitmapy/471038
    BITMAP bmInfo;
    GetObject(bitmap, sizeof(bmInfo), &bmInfo);
    BitBlt(hdc, 1, 1, FLAG_SIZE, FLAG_SIZE, memDC, 0, 0, SRCCOPY);
    StretchBlt(hdc, 1, 1, BOX_SIZE-2, BOX_SIZE-2, memDC, 0, 0, bmInfo.bmWidth, bmInfo.bmHeight, SRCCOPY);
    DeleteObject(bitmap);
    ReleaseDC(BoxArr[x][y], hdc);
}

void FlagMinesAccept(int x = -1, int y = -1)
{
        for (int i = 0; i < sizeX; i++)
            for (int j = 0; j < sizeY; j++)
                if (!(i == x && j == y) && MineArr[i][j] == MINE)
                {
                    PaintFlag(i, j);
                    FlagArr[i][j] = true;
                }
}
#pragma endregion

#pragma region GAME_MECHANICKS
void unCover(int x, int y)
{
    if (x < 0 || y < 0 || x >= sizeX || y >= sizeY || Visible[x][y]) return;
    switch (MineArr[x][y]) {
    case MINE:
        break;
    case 0:
        makeVisible(x, y);
        unCover(x + 1, y);
        unCover(x + 1, y + 1);
        unCover(x + 1, y - 1);
        unCover(x - 1, y - 1);
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

    for (int i = 0; i < sizeX * sizeY; i++)
        numbers.push_back(i);

    unsigned seed = (unsigned)std::chrono::system_clock::now().time_since_epoch().count();
    std::shuffle(numbers.begin(), numbers.end(), std::default_random_engine(seed));
    for (int i = 0; i < sizeX * sizeY - MineAmount; i++)
        numbers.pop_back();

    std::for_each(numbers.begin(), numbers.end(), [](int number) {
        int x = number % sizeX;
        int y = number / sizeX;
        MineArr[x][y] = MINE;
        });


    for (int x = 0; x < sizeX; x++)
        for (int y = 0; y < sizeY; y++)
            if (MineArr[x][y] != MINE)
            {
                int a = 0;
                if (x > 0 && MineArr[x - 1][y] == MINE) a++;
                if (x > 0 && y > 0 && MineArr[x - 1][y - 1] == MINE) a++;
                if (x > 0 && y + 1 != sizeY && MineArr[x - 1][y + 1] == MINE) a++;
                if (y > 0 && MineArr[x][y - 1] == MINE) a++;
                if (y + 1 != sizeY && MineArr[x][y + 1] == MINE) a++;
                if (x + 1 != sizeX && MineArr[x + 1][y] == MINE) a++;
                if (y > 0 && x + 1 != sizeX && MineArr[x + 1][y - 1] == MINE) a++;
                if (y + 1 != sizeY && x + 1 != sizeX && MineArr[x + 1][y + 1] == MINE) a++;
                MineArr[x][y] = a;
            }

    CurrentMineAmount = MineAmount;
    currentTime = 0;
    count = false;
}

void newGame(HWND hWnd)
{
    windowHeight = BOX_SIZE * sizeY + (sizeY)+PANEL_HEIGHT;
    windowWidth = BOX_SIZE * sizeX + (sizeX);
    RECT rs;
    rs.top = screenHeight / 2 - windowHeight / 2;
    rs.bottom = screenHeight / 2 + windowHeight / 2;
    rs.left = screenWidth / 2 - windowWidth / 2;
    rs.right = screenWidth / 2 + windowWidth / 2;
    AdjustWindowRect(&rs, WS_OVERLAPPEDWINDOW, TRUE);
    MoveWindow(hWnd, rs.left, rs.top, rs.right - rs.left, rs.bottom - rs.top, TRUE);
    initGame();
    PaintBoxes();
    
    RepaintPanel();
}

#pragma endregion

#pragma region MESSAGE_BOX
void BOOM()
{
    int msgboxID = MessageBox(
        NULL,
        (LPCWSTR)L"BOOM!",
        (LPCWSTR)L"MineSweeper",
        MB_ICONERROR | MB_OK
    );
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
#pragma endregion

#pragma region CLICK_HANDLER

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
        FlagMinesAccept(x, y);
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
        
        for (int i = 0; i < sizeX; i++)
            for (int j = 0; j < sizeY; j++)
                makeVisible(i, j);
        FlagMinesAccept();
        WinPrompt();
    }
}

void  flagBox(int x, int y)
{
    if (!count)
    {
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

    if (checkWin())
    {
        count = false;
        
        for (int i = 0; i < sizeX; i++)
            for (int j = 0; j < sizeY; j++)
                makeVisible(i, j);
        FlagMinesAccept();
        WinPrompt();
    }
}

#pragma endregion
