/*
 * Rainbow Six Siege X Server Switcher
 * Copyright (c) 2025 0xsweat
 * Licensed under the MIT License. See LICENSE file in the project root for full license information.
 */
// This application allows users to switch their matchmaking server configuration for Rainbow Six Siege.

#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <dwmapi.h>
#include <uxtheme.h>
#include <io.h>
#include <shlobj.h>  // For SHGetFolderPath

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "uxtheme.lib")
#pragma comment(lib, "shell32.lib")

#define MAX_PATH_LEN 512
#define MAX_CONFIGS 32

// Color scheme
const COLORREF COLOR_BG = RGB(32, 32, 32);
const COLORREF COLOR_FG = RGB(255, 255, 255);
const COLORREF COLOR_CTRL = RGB(45, 45, 48);
const COLORREF COLOR_BTN = RGB(0, 122, 204);
const COLORREF COLOR_BTN_HOVER = RGB(28, 151, 234);
const COLORREF COLOR_BTN_PRESS = RGB(0, 102, 184);
const COLORREF COLOR_BORDER = RGB(67, 67, 70);

// Global resources
HFONT g_hFont = NULL;
HFONT g_hFontSmall = NULL;
HBRUSH g_hBrushBg = NULL;
HBRUSH g_hBrushCtrl = NULL;

// Server list
const char *servers[] = {
    "default",
    "playfab/australiaeast",
    "playfab/brazilsouth",
    "playfab/centralus",
    "playfab/eastasia",
    "playfab/eastus",
    "playfab/japaneast",
    "playfab/northeurope",
    "playfab/southafricanorth",
    "playfab/southcentralus",
    "playfab/southeastasia",
    "playfab/uaenorth",
    "playfab/westeurope",
    "playfab/westus"
};

// Config files
char configs[MAX_CONFIGS][MAX_PATH_LEN];
int config_count = 0;

void CreateResources() {
    // Create fonts
    LOGFONT lf = {0};
    lf.lfHeight = -20;
    lf.lfWeight = FW_SEMIBOLD;
    lf.lfQuality = CLEARTYPE_QUALITY;
    strcpy(lf.lfFaceName, "Segoe UI");
    g_hFont = CreateFontIndirect(&lf);
    
    lf.lfHeight = -16;
    lf.lfWeight = FW_NORMAL;
    g_hFontSmall = CreateFontIndirect(&lf);
    
    // Create brushes
    g_hBrushBg = CreateSolidBrush(COLOR_BG);
    g_hBrushCtrl = CreateSolidBrush(COLOR_CTRL);
}

void CleanupResources() {
    if (g_hFont) DeleteObject(g_hFont);
    if (g_hFontSmall) DeleteObject(g_hFontSmall);
    if (g_hBrushBg) DeleteObject(g_hBrushBg);
    if (g_hBrushCtrl) DeleteObject(g_hBrushCtrl);
}

void EnableDarkMode(HWND hwnd) {
    BOOL dark = TRUE;
    DwmSetWindowAttribute(hwnd, 19, &dark, sizeof(dark));
    DWORD color = 0xFF202020;
    DwmSetWindowAttribute(hwnd, 35, &color, sizeof(color));
}

void DrawRoundRect(HDC hdc, RECT rect, COLORREF color, int radius) {
    HBRUSH brush = CreateSolidBrush(color);
    HPEN pen = CreatePen(PS_SOLID, 1, color);
    HGDIOBJ oldBrush = SelectObject(hdc, brush);
    HGDIOBJ oldPen = SelectObject(hdc, pen);
    
    RoundRect(hdc, rect.left, rect.top, rect.right, rect.bottom, radius, radius);
    
    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);
    DeleteObject(pen);
    DeleteObject(brush);
}

// Button subclass for hover effects
typedef struct {
    BOOL hover;
    BOOL press;
} ButtonData;

LRESULT CALLBACK ButtonProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, 
                           UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
    ButtonData* data = (ButtonData*)dwRefData;
    
    switch (msg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rc;
        GetClientRect(hwnd, &rc);
        
        // Determine color
        COLORREF color = COLOR_BTN;
        if (data->press) color = COLOR_BTN_PRESS;
        else if (data->hover) color = COLOR_BTN_HOVER;
        
        // Draw button
        DrawRoundRect(hdc, rc, color, 8);
        
        // Draw text
        char text[128];
        GetWindowText(hwnd, text, sizeof(text));
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, COLOR_FG);
        SelectObject(hdc, g_hFont);
        DrawText(hdc, text, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_MOUSEMOVE:
        if (!data->hover) {
            data->hover = TRUE;
            InvalidateRect(hwnd, NULL, FALSE);
            
            TRACKMOUSEEVENT tme = {sizeof(tme), TME_LEAVE, hwnd, 0};
            TrackMouseEvent(&tme);
        }
        break;
    case WM_MOUSELEAVE:
        data->hover = FALSE;
        InvalidateRect(hwnd, NULL, FALSE);
        break;
    case WM_LBUTTONDOWN:
        data->press = TRUE;
        InvalidateRect(hwnd, NULL, FALSE);
        SetCapture(hwnd);
        break;
    case WM_LBUTTONUP:
        if (data->press) {
            data->press = FALSE;
            InvalidateRect(hwnd, NULL, FALSE);
            ReleaseCapture();
            
            RECT rc;
            POINT pt;
            GetCursorPos(&pt);
            ScreenToClient(hwnd, &pt);
            GetClientRect(hwnd, &rc);
            
            if (PtInRect(&rc, pt)) {
                SendMessage(GetParent(hwnd), WM_COMMAND, GetDlgCtrlID(hwnd), (LPARAM)hwnd);
            }
        }
        break;
    case WM_ERASEBKGND:
        return 1;
    case WM_NCDESTROY:
        RemoveWindowSubclass(hwnd, ButtonProc, uIdSubclass);
        free(data);
        break;
    }
    return DefSubclassProc(hwnd, msg, wParam, lParam);
}

HWND CreateButton(HWND parent, const char* text, int x, int y, int w, int h, int id) {
    HWND btn = CreateWindow("BUTTON", text, WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,
                           x, y, w, h, parent, (HMENU)(UINT_PTR)id, NULL, NULL);
    
    ButtonData* data = (ButtonData*)calloc(1, sizeof(ButtonData));
    SetWindowSubclass(btn, ButtonProc, 0, (DWORD_PTR)data);
    
    return btn;
}

void load_configs() {
    char search_path[MAX_PATH_LEN];
    char documents_path[MAX_PATH_LEN];
    struct _finddata_t c_file;
    intptr_t hFile;

    config_count = 0;

    // Get user's Documents folder
    HRESULT hr = SHGetFolderPath(NULL, CSIDL_MYDOCUMENTS, NULL, SHGFP_TYPE_CURRENT, documents_path);
    if (FAILED(hr)) {
        return;
    }

    // Build path to Rainbow Six Siege config folder
    snprintf(search_path, sizeof(search_path), "%s\\My Games\\Rainbow Six - Siege\\*", documents_path);

    if ((hFile = _findfirst(search_path, &c_file)) == -1L) return;

    do {
        if ((c_file.attrib & _A_SUBDIR) && c_file.name[0] != '.') {
            char ini_path[MAX_PATH_LEN];
            snprintf(ini_path, sizeof(ini_path), "%s\\My Games\\Rainbow Six - Siege\\%s\\GameSettings.ini",
                     documents_path, c_file.name);
            if (_access(ini_path, 0) == 0 && config_count < MAX_CONFIGS) {
                strncpy(configs[config_count++], ini_path, MAX_PATH_LEN);
            }
        }
    } while (_findnext(hFile, &c_file) == 0);
    _findclose(hFile);
}

char* get_current_server() {
    static char server[128] = "";
    if (config_count == 0) return server;
    
    FILE *f = fopen(configs[0], "r");
    if (!f) return server;
    
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "DataCenterHint=", 15) == 0) {
            char *p = line + 15;
            char *end = strchr(p, '\n');
            if (end) *end = 0;
            strcpy(server, p);
            break;
        }
    }
    fclose(f);
    return server;
}

void update_configs(const char *server) {
    for (int i = 0; i < config_count; i++) {
        FILE *fin = fopen(configs[i], "r");
        if (!fin) continue;
        
        char tmp[MAX_PATH_LEN];
        snprintf(tmp, sizeof(tmp), "%s.tmp", configs[i]);
        FILE *fout = fopen(tmp, "w");
        if (!fout) {
            fclose(fin);
            continue;
        }
        
        char line[512];
        int found = 0;
        while (fgets(line, sizeof(line), fin)) {
            if (strncmp(line, "DataCenterHint=", 15) == 0) {
                fprintf(fout, "DataCenterHint=%s\n", server);
                found = 1;
            } else {
                fputs(line, fout);
            }
        }
        if (!found) fprintf(fout, "DataCenterHint=%s\n", server);
        
        fclose(fin);
        fclose(fout);
        remove(configs[i]);
        rename(tmp, configs[i]);
    }
}

const char* format_server_name(const char* server) {
    static char formatted[128];
    
    if (strcmp(server, "default") == 0) return "Auto Select (Lowest Ping)";
    
    const char* name = server;
    if (strncmp(server, "playfab/", 8) == 0) name = server + 8;
    
    // Format known regions
    struct { const char* key; const char* display; } regions[] = {
        {"australiaeast", "Australia East"},
        {"brazilsouth", "Brazil South"},
        {"centralus", "Central US"},
        {"eastasia", "East Asia"},
        {"eastus", "East US"},
        {"japaneast", "Japan East"},
        {"northeurope", "North Europe"},
        {"southafricanorth", "South Africa North"},
        {"southcentralus", "South Central US"},
        {"southeastasia", "Southeast Asia"},
        {"uaenorth", "UAE North"},
        {"westeurope", "West Europe"},
        {"westus", "West US"}
    };
    
    for (int i = 0; i < sizeof(regions)/sizeof(regions[0]); i++) {
        if (strcmp(name, regions[i].key) == 0) {
            return regions[i].display;
        }
    }
    
    strcpy(formatted, name);
    return formatted;
}

// Combobox custom draw
LRESULT CALLBACK ComboProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam,
                          UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
    if (msg == WM_PAINT) {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rc;
        GetClientRect(hwnd, &rc);
        
        // Background
        FillRect(hdc, &rc, g_hBrushCtrl);
        
        // Border
        HPEN pen = CreatePen(PS_SOLID, 1, COLOR_BORDER);
        SelectObject(hdc, pen);
        SelectObject(hdc, GetStockObject(NULL_BRUSH));
        Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);
        DeleteObject(pen);
        
        // Text
        int sel = SendMessage(hwnd, CB_GETCURSEL, 0, 0);
        if (sel >= 0) {
            char text[256];
            SendMessage(hwnd, CB_GETLBTEXT, sel, (LPARAM)text);
            
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, COLOR_FG);
            SelectObject(hdc, g_hFontSmall);
            
            RECT textRc = rc;
            textRc.left += 10;
            textRc.right -= 30;
            textRc.top += 5;     // Add top padding
            textRc.bottom -= 5;  // Add bottom padding
            DrawText(hdc, text, -1, &textRc, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        }
        
        // Arrow
        int cx = rc.right - 15;
        int cy = (rc.top + rc.bottom) / 2;
        
        POINT arrow[3] = {{cx-5, cy-2}, {cx+5, cy-2}, {cx, cy+3}};
        
        HBRUSH brush = CreateSolidBrush(COLOR_FG);
        SelectObject(hdc, brush);
        Polygon(hdc, arrow, 3);
        DeleteObject(brush);
        
        EndPaint(hwnd, &ps);
        return 0;
    }
    return DefSubclassProc(hwnd, msg, wParam, lParam);
}

// Callback for EnumChildWindows
BOOL CALLBACK SetFontCallback(HWND child, LPARAM lParam) {
    int id = GetDlgCtrlID(child);
    SendMessage(child, WM_SETFONT, (WPARAM)(id == 0 ? g_hFont : g_hFontSmall), TRUE);
    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static HWND hCombo, hList, hButton, hCurrent;
    
    switch (msg) {
    case WM_CREATE: {
        // Title
        CreateWindow("STATIC", "Server Configuration", WS_VISIBLE | WS_CHILD | SS_CENTER,
                    0, 20, 700, 40, hwnd, NULL, NULL, NULL);
        
        // Server dropdown label
        CreateWindow("STATIC", "Select Server Region", WS_VISIBLE | WS_CHILD,
                    40, 75, 300, 25, hwnd, NULL, NULL, NULL);
        
        hCombo = CreateWindow("COMBOBOX", NULL, 
                             WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | WS_VSCROLL | CBS_OWNERDRAWFIXED | CBS_HASSTRINGS,
                             40, 105, 620, 400, hwnd, (HMENU)(UINT_PTR)1, NULL, NULL);
        SetWindowSubclass(hCombo, ComboProc, 0, 0);
        SendMessage(hCombo, CB_SETITEMHEIGHT, -1, 30); // Set combo height
        SendMessage(hCombo, CB_SETITEMHEIGHT, 0, 25);  // Set dropdown item height
        
        // Add servers
        for (int i = 0; i < sizeof(servers)/sizeof(servers[0]); i++) {
            SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)format_server_name(servers[i]));
        }
        SendMessage(hCombo, CB_SETCURSEL, 0, 0);
        
        // Config list label
        char configLabel[256];
        snprintf(configLabel, sizeof(configLabel), "Configuration Files (%d found)", config_count);
        CreateWindow("STATIC", configLabel, WS_VISIBLE | WS_CHILD,
                40, 155, 400, 25, hwnd, NULL, NULL, NULL);

        hList = CreateWindow(WC_LISTBOX, NULL,
                            WS_VISIBLE | WS_CHILD | WS_VSCROLL | WS_BORDER,
                            40, 185, 620, 140, hwnd, (HMENU)(UINT_PTR)2, NULL, NULL);
        SendMessage(hList, LB_SETITEMHEIGHT, 0, 22); // Set listbox item height
        
        // Add config files
        for (int i = 0; i < config_count; i++) {
            // Find the last '\' (before GameSettings.ini)
            char *last = strrchr(configs[i], '\\');
            if (last) {
                // Temporarily terminate at last '\' to get the folder path
                *last = '\0';
                // Now find the previous '\' for the folder name
                char *prev = strrchr(configs[i], '\\');
                if (prev) {
                    SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)(prev + 1));
                } else {
                    SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)configs[i]);
                }
                *last = '\\'; // Restore original string
            } else {
                SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)configs[i]);
            }
        }
        
        // Apply button
        hButton = CreateButton(hwnd, "Apply Changes", 275, 345, 150, 40, 3);
        
        // Current server
        char text[256];
        char* current = get_current_server();
        snprintf(text, sizeof(text), "Current: %s", 
                current[0] ? format_server_name(current) : "Not Set");
        hCurrent = CreateWindow("STATIC", text, WS_VISIBLE | WS_CHILD | SS_CENTER,
                               40, 400, 620, 25, hwnd, (HMENU)(UINT_PTR)4, NULL, NULL);
        
        // Set fonts
        EnumChildWindows(hwnd, SetFontCallback, 0);
        
        break;
    }
    case WM_COMMAND:
        if (HIWORD(wParam) == CBN_DROPDOWN && LOWORD(wParam) == 1) {
            // Apply dark theme to dropdown when it opens
            COMBOBOXINFO cbi = {0};
            cbi.cbSize = sizeof(COMBOBOXINFO);
            if (GetComboBoxInfo(hCombo, &cbi)) {
                if (cbi.hwndList) {
                    SetWindowTheme(cbi.hwndList, L"DarkMode_Explorer", NULL);
                }
            }
        }
        
        if (LOWORD(wParam) == 3) { // Apply button
            int sel = SendMessage(hCombo, CB_GETCURSEL, 0, 0);
            if (sel >= 0) {
                update_configs(servers[sel]);
                
                char text[256];
                snprintf(text, sizeof(text), "Current: %s", format_server_name(servers[sel]));
                SetWindowText(hCurrent, text);
                
                MessageBox(
                    hwnd,
                    "Server settings updated successfully!\n"
                    "Don't forget to restart siege for the changes to take effect!",
                    "Success",
                    MB_OK | MB_ICONINFORMATION
                );
            }
        }
        break;
    case WM_CTLCOLORSTATIC:
        SetBkColor((HDC)wParam, COLOR_BG);
        SetTextColor((HDC)wParam, COLOR_FG);
        return (LRESULT)g_hBrushBg;
    case WM_CTLCOLORLISTBOX: {
        HDC hdc = (HDC)wParam;
        HWND hControl = (HWND)lParam;
        
        // Check if this is the combobox dropdown
        COMBOBOXINFO cbi = {0};
        cbi.cbSize = sizeof(COMBOBOXINFO);
        if (GetComboBoxInfo(hCombo, &cbi) && hControl == cbi.hwndList) {
            // This is the dropdown list
            SetBkColor(hdc, COLOR_CTRL);
            SetTextColor(hdc, COLOR_FG);
            return (LRESULT)g_hBrushCtrl;
        }
        
        // For other listboxes
        SetBkColor(hdc, COLOR_CTRL);
        SetTextColor(hdc, COLOR_FG);
        return (LRESULT)g_hBrushCtrl;
    }
    case WM_DRAWITEM: {
        LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;
        if (dis->CtlType == ODT_COMBOBOX && dis->CtlID == 1) {
            // Draw combobox dropdown items
            if (dis->itemID != -1) {
                // Get item text
                char text[256];
                SendMessage(hCombo, CB_GETLBTEXT, dis->itemID, (LPARAM)text);
                
                // Set colors based on selection state
                COLORREF bgColor = (dis->itemState & ODS_SELECTED) ? RGB(62, 62, 66) : COLOR_CTRL;
                COLORREF textColor = COLOR_FG;
                
                // Fill background
                HBRUSH brush = CreateSolidBrush(bgColor);
                FillRect(dis->hDC, &dis->rcItem, brush);
                DeleteObject(brush);
                
                // Draw text
                SetBkMode(dis->hDC, TRANSPARENT);
                SetTextColor(dis->hDC, textColor);
                SelectObject(dis->hDC, g_hFontSmall);
                
                RECT textRect = dis->rcItem;
                textRect.left += 5;
                DrawText(dis->hDC, text, -1, &textRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
                
                // Draw focus rectangle if needed
                if (dis->itemState & ODS_FOCUS) {
                    DrawFocusRect(dis->hDC, &dis->rcItem);
                }
            }
            return TRUE;
        }
        break;
    }
    case WM_CTLCOLORBTN:
        return (LRESULT)g_hBrushCtrl;
    case WM_ERASEBKGND: {
        RECT rc;
        GetClientRect(hwnd, &rc);
        FillRect((HDC)wParam, &rc, g_hBrushBg);
        return 1;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPSTR lpCmd, int nShow) {
    load_configs();
    CreateResources();
    
    if (config_count == 0) {
        MessageBox(NULL, 
            "No configuration files found in Documents\\My Games\\Rainbow Six - Siege.\n\nThe application will now exit.",
            "Error", MB_OK | MB_ICONERROR);
        return 1;
    }
    
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(hInstance, "IDI_APPICON");
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = "ServerConfig";
    wc.hbrBackground = g_hBrushBg;
    RegisterClass(&wc);
    
    HWND hwnd = CreateWindow("ServerConfig", "Rainbow Six Siege X Server Config Switcher (https://github.com/0xsweat/r6x-switcher)",
                           WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                           CW_USEDEFAULT, CW_USEDEFAULT, 700, 500,
                           NULL, NULL, hInstance, NULL);
    
    EnableDarkMode(hwnd);
    ShowWindow(hwnd, nShow);
    
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    CleanupResources();
    return 0;
}