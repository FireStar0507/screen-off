// 息屏静音工具 by.LU
// GitHub: FireStar0507/screen-off

#include <windows.h>
#include <shellapi.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>

// ---------- 全局变量 ----------
HWND g_hWndBlack = NULL;
bool g_bIsOff = false;
float g_originalVolume = 1.0f;
UINT g_hotkeyId = 1;
HANDLE g_hMutex = NULL;
IAudioEndpointVolume* g_pVolume = NULL;

// ---------- 音量控制 (COM) ----------
bool InitVolumeControl() {
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr)) return false;

    IMMDeviceEnumerator* pEnumerator = NULL;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL,
                          __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);
    if (FAILED(hr)) {
        CoUninitialize();
        return false;
    }

    IMMDevice* pDevice = NULL;
    hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
    pEnumerator->Release();
    if (FAILED(hr)) {
        CoUninitialize();
        return false;
    }

    hr = pDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL,
                           (void**)&g_pVolume);
    pDevice->Release();
    if (FAILED(hr)) {
        CoUninitialize();
        return false;
    }
    return true;
}

void SetSystemVolume(float volume) {
    if (g_pVolume) g_pVolume->SetMasterVolumeLevelScalar(volume, NULL);
}

float GetSystemVolume() {
    if (g_pVolume) {
        float vol;
        g_pVolume->GetMasterVolumeLevelScalar(&vol);
        return vol;
    }
    return 1.0f;
}

// ---------- 单实例检测 ----------
bool CheckSingleInstance() {
    g_hMutex = CreateMutexW(NULL, TRUE, L"Global\\ScreenOffTool_Mutex");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        // 已有实例，激活其隐藏窗口（可有可无，此处仅作演示）
        HWND hWnd = FindWindowW(L"ScreenOffMainClass", NULL);
        if (hWnd) {
            if (IsIconic(hWnd)) ShowWindow(hWnd, SW_RESTORE);
            SetForegroundWindow(hWnd);
        }
        CloseHandle(g_hMutex);
        return false;
    }
    return true;
}

// ---------- 黑色窗口过程 ----------
LRESULT CALLBACK BlackWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_KEYDOWN:
            if (wParam == VK_ESCAPE) {
                HWND hMain = FindWindowW(L"ScreenOffMainClass", NULL);
                if (hMain) PostMessage(hMain, WM_HOTKEY, g_hotkeyId, 0);
                return 0;
            }
            break;
        case WM_DESTROY:
            // 不调用 PostQuitMessage，只销毁自身
            break;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

// ---------- 创建/销毁黑色窗口 ----------
void CreateBlackWindow() {
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = BlackWndProc;
    wc.hInstance = GetModuleHandleW(NULL);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));
    wc.lpszClassName = L"BlackScreenClass";
    RegisterClassExW(&wc);

    g_hWndBlack = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_NOACTIVATE,
        L"BlackScreenClass",
        NULL,
        WS_POPUP | WS_VISIBLE,
        0, 0,
        GetSystemMetrics(SM_CXSCREEN),
        GetSystemMetrics(SM_CYSCREEN),
        NULL, NULL, GetModuleHandleW(NULL), NULL
    );
}

void DestroyBlackWindow() {
    if (g_hWndBlack) {
        DestroyWindow(g_hWndBlack);
        g_hWndBlack = NULL;
    }
}

void ToggleBlackScreen(bool show) {
    if (show && !g_hWndBlack) CreateBlackWindow();
    else if (!show && g_hWndBlack) DestroyBlackWindow();
}

// ---------- 主窗口过程 ----------
LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            if (!RegisterHotKey(hwnd, g_hotkeyId, MOD_CONTROL | MOD_ALT, 'O'))
                MessageBoxW(hwnd, L"注册热键失败！", L"错误", MB_ICONERROR);

            NOTIFYICONDATAW nid = {};
            nid.cbSize = sizeof(NOTIFYICONDATAW);
            nid.hWnd = hwnd;
            nid.uID = 100;
            nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
            nid.uCallbackMessage = WM_APP + 1;
            nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
            wcscpy_s(nid.szTip, L"息屏静音工具 by.LU");
            Shell_NotifyIconW(NIM_ADD, &nid);
            break;
        }
        case WM_DESTROY: {
            NOTIFYICONDATAW nid = {};
            nid.cbSize = sizeof(NOTIFYICONDATAW);
            nid.hWnd = hwnd;
            nid.uID = 100;
            Shell_NotifyIconW(NIM_DELETE, &nid);

            if (g_bIsOff) {
                ToggleBlackScreen(false);
                SetSystemVolume(g_originalVolume);
                g_bIsOff = false;
            }
            if (g_pVolume) { g_pVolume->Release(); g_pVolume = NULL; }
            CoUninitialize();
            if (g_hMutex) { CloseHandle(g_hMutex); g_hMutex = NULL; }
            PostQuitMessage(0);
            break;
        }
        case WM_HOTKEY: {
            if (wParam == g_hotkeyId) {
                if (!g_bIsOff) {
                    g_originalVolume = GetSystemVolume();
                    SetSystemVolume(0.0f);
                    ToggleBlackScreen(true);
                    g_bIsOff = true;
                } else {
                    ToggleBlackScreen(false);
                    SetSystemVolume(g_originalVolume);
                    g_bIsOff = false;
                }
            }
            break;
        }
        case WM_APP + 1: {
            if (lParam == WM_LBUTTONDBLCLK) {
                SendMessage(hwnd, WM_HOTKEY, g_hotkeyId, 0);
            } else if (lParam == WM_RBUTTONUP) {
                HMENU hMenu = CreatePopupMenu();
                AppendMenuW(hMenu, MF_STRING, 1, L"切换息屏");
                AppendMenuW(hMenu, MF_STRING, 2, L"退出");
                POINT pt;
                GetCursorPos(&pt);
                SetForegroundWindow(hwnd);
                int cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_NONOTIFY,
                                         pt.x, pt.y, 0, hwnd, NULL);
                DestroyMenu(hMenu);
                if (cmd == 1) SendMessage(hwnd, WM_HOTKEY, g_hotkeyId, 0);
                else if (cmd == 2) DestroyWindow(hwnd);
            }
            break;
        }
        default:
            return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
    return 0;
}

// ---------- WinMain ----------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow) {
    // 防多开
    if (!CheckSingleInstance()) return 0;

    // 初始化音量控制
    if (!InitVolumeControl()) {
        MessageBoxW(NULL, L"音量控制初始化失败！", L"错误", MB_ICONERROR);
        return 1;
    }

    // 注册主窗口类
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = MainWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"ScreenOffMainClass";
    if (!RegisterClassExW(&wc)) {
        MessageBoxW(NULL, L"注册主窗口类失败", L"错误", MB_ICONERROR);
        return 1;
    }

    // 创建隐藏窗口（用于接收消息）
    HWND hWnd = CreateWindowExW(
        0,
        L"ScreenOffMainClass",
        L"ScreenOff",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 200, 200,
        NULL, NULL, hInstance, NULL
    );
    if (!hWnd) {
        MessageBoxW(NULL, L"创建主窗口失败", L"错误", MB_ICONERROR);
        return 1;
    }

    ShowWindow(hWnd, SW_HIDE);

    // 消息循环
    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return 0;
}