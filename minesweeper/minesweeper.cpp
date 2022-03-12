
#include "framework.h"
#include "minesweeper.h"

#define MAX_LOADSTRING 100
#define BOX_SIZE 25
#define MAX_SIZEY 24
#define MAX_SIZEX 30
#define MIN_MINE_AMOUNT 10
#define PANEL_HEIGHT 40
#define FONT_SIZE 25

HINSTANCE hInst;                                
WCHAR szTitle[MAX_LOADSTRING];                  
WCHAR szWindowClass[MAX_LOADSTRING];            
int sizeX = 10;
int sizeY = 10;
int windowHeight;
int windowWidth;
int screenWidth;
int screenHeight;
int MineAmount = MIN_MINE_AMOUNT;
int MineArr[MAX_SIZEX][MAX_SIZEY]; // -1 = mine
HWND BoxArr[MAX_SIZEX][MAX_SIZEY];
HWND Panel;

ATOM                MyRegisterClass(HINSTANCE hInstance);
ATOM                MyBoxClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    BoxProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    Dialog(HWND, UINT, WPARAM, LPARAM);
void                CreateBoxes(HWND);

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
    wcex.hbrBackground = (HBRUSH)(COLOR_GRAYTEXT);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_MINESWEEPER);
    wcex.lpszClassName = L"BoxClass";
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Przechowuj dojście wystąpienia w naszej zmiennej globalnej
   screenHeight = GetSystemMetrics(SM_CYSCREEN);
   screenWidth = GetSystemMetrics(SM_CXSCREEN);
   windowHeight = BOX_SIZE * sizeY + (sizeY - 1)+ PANEL_HEIGHT;
   windowWidth = BOX_SIZE * sizeX + (sizeX - 1);
   RECT rs;
   rs.top = screenHeight / 2 - windowHeight / 2;
   rs.bottom = screenHeight / 2 + windowHeight / 2;
   rs.left = screenWidth / 2 - windowWidth / 2;
   rs.right = screenWidth / 2 + windowWidth / 2;
   AdjustWindowRect(&rs,WS_OVERLAPPEDWINDOW,TRUE);

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
       rs.left, rs.top, rs.right-rs.left, rs.bottom-rs.top, nullptr, nullptr, hInstance, nullptr);

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
            case ID_GAME_EXIT:
                DestroyWindow(hWnd);
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
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Tutaj dodaj kod rysujący używający elementu hdc...
            EndPaint(hWnd, &ps);
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
            
            sizeY       = _wtoi(s1) > MAX_SIZEY       ? MAX_SIZEY         : _wtoi(s1);
            sizeX       = _wtoi(s2) > MAX_SIZEX       ? MAX_SIZEY         : _wtoi(s2);
            MineAmount  = _wtoi(s3) < MIN_MINE_AMOUNT ? MIN_MINE_AMOUNT   : _wtoi(s3);

            //newgame
            EndDialog(hDlg, LOWORD(wParam));
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
            if (BoxArr[x][y]) DestroyWindow(BoxArr[x][y]);

    Panel = CreateWindowEx(0, szWindowClass, L"panel", WS_CHILD | WS_VISIBLE,
        0, 0, PANEL_HEIGHT, windowWidth, hWnd, NULL, hInst, NULL);

    for (int x = 0; x < sizeX; x++)
        for (int y = 0; y < sizeY; y++)
            BoxArr[x][y] = CreateWindowEx(0, L"BoxClass", L"box", WS_CHILD | WS_VISIBLE,
                x * 26, y * 26 + PANEL_HEIGHT, BOX_SIZE, BOX_SIZE, hWnd, NULL, hInst, NULL);
}