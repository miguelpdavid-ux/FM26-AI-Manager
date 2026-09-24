#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <dwmapi.h>
#include <string>
#include <fstream>
#include <filesystem>
#include <vector>
#include <cstdint>
#include <algorithm>

static HWND gFM=nullptr;
static HWND hConn,hSave,hVision,hAI,hMode,hActivity,hMetric1,hMetric2,hMetric3;
static bool running=true, emergency=false;
static int controlLevel=25;
static uint64_t captures=0,lastHash=0,changes=0;
static HFONT fontTitle,fontHead,fontBody,fontSmall;
static HBRUSH bgBrush, panelBrush;
static COLORREF BG=RGB(15,18,24), PANEL=RGB(23,27,35), TEXT=RGB(235,239,245), MUTED=RGB(151,160,174), GREEN=RGB(80,210,145), BLUE=RGB(92,150,255), RED=RGB(245,91,91);

void dirs(){for(auto p:{L"config",L"data",L"database",L"logs",L"backups"})CreateDirectoryW(p,nullptr);}
std::wstring stamp(){SYSTEMTIME s{};GetLocalTime(&s);wchar_t b[64];swprintf(b,64,L"%04d-%02d-%02d %02d:%02d:%02d",s.wYear,s.wMonth,s.wDay,s.wHour,s.wMinute,s.wSecond);return b;}
void logLine(const std::wstring&s){std::wofstream f(L"logs\\manager.log",std::ios::app);f<<stamp()<<L" | "<<s<<L"\n";}
void setText(HWND h,const std::wstring&s){SetWindowTextW(h,s.c_str());}
void saveMode(){std::wofstream f(L"config\\control.ini");f<<controlLevel;}
void loadMode(){std::wifstream f(L"config\\control.ini");int v=25;if(f>>v && (v==25||v==60||v==100))controlLevel=v;}
BOOL CALLBACK enumFM(HWND h,LPARAM p){if(!IsWindowVisible(h))return TRUE;wchar_t t[512]{};GetWindowTextW(h,t,511);std::wstring s=t;if(s.find(L"Football Manager 2026")!=std::wstring::npos||s.find(L"Football Manager 26")!=std::wstring::npos){*(HWND*)p=h;return FALSE;}return TRUE;}
HWND findFM(){HWND h=nullptr;EnumWindows(enumFM,(LPARAM)&h);return h;}
struct Img{int w=0,h=0;std::vector<unsigned char> px;};
bool grab(HWND h,Img&im){RECT r{};if(!GetClientRect(h,&r))return false;im.w=r.right;im.h=r.bottom;if(im.w<64||im.h<64)return false;HDC s=GetDC(h),m=CreateCompatibleDC(s);HBITMAP b=CreateCompatibleBitmap(s,im.w,im.h);HGDIOBJ old=SelectObject(m,b);BOOL ok=BitBlt(m,0,0,im.w,im.h,s,0,0,SRCCOPY|CAPTUREBLT);BITMAPINFOHEADER bi{};bi.biSize=sizeof(bi);bi.biWidth=im.w;bi.biHeight=-im.h;bi.biPlanes=1;bi.biBitCount=32;bi.biCompression=BI_RGB;im.px.resize((size_t)im.w*im.h*4);if(ok)ok=GetDIBits(m,b,0,im.h,im.px.data(),(BITMAPINFO*)&bi,DIB_RGB_COLORS);SelectObject(m,old);DeleteObject(b);DeleteDC(m);ReleaseDC(h,s);return ok;}
uint64_t hashImg(const Img&i){uint64_t h=1469598103934665603ULL;size_t step=std::max<size_t>(4,i.px.size()/4096);for(size_t n=0;n<i.px.size();n+=step){h^=i.px[n];h*=1099511628211ULL;}return h;}
std::wstring modeName(){return controlLevel==25?L"25%  ASSIST":controlLevel==60?L"60%  COPILOT":L"100%  AUTONOMOUS";}
void chooseMode(int v){controlLevel=v; emergency=false; saveMode(); setText(hMode,modeName()); setText(hAI,L"● ACTIVE"); setText(hActivity,L"Control policy changed. Rebuilding decision permissions for the active save."); logLine(L"Control level changed to "+std::to_wstring(v)+L"%"); InvalidateRect(GetParent(hMode),nullptr,FALSE);}
void style(HWND h,HFONT f){SendMessageW(h,WM_SETFONT,(WPARAM)f,TRUE);}
HWND label(HWND p,const wchar_t*t,int x,int y,int w,int h,HFONT f=0){HWND c=CreateWindowW(L"STATIC",t,WS_CHILD|WS_VISIBLE,x,y,w,h,p,0,0,0);style(c,f?f:fontBody);return c;}
HWND button(HWND p,const wchar_t*t,int x,int y,int w,int h,int id){HWND c=CreateWindowW(L"BUTTON",t,WS_CHILD|WS_VISIBLE|BS_OWNERDRAW,x,y,w,h,p,(HMENU)(INT_PTR)id,0,0);style(c,fontBody);return c;}
void updateSession(){gFM=findFM();if(!gFM){setText(hConn,L"● WAITING FOR FM26");setText(hSave,L"No active save session detected");setText(hVision,L"● IDLE");return;}setText(hConn,L"● CONNECTED");wchar_t title[512]{};GetWindowTextW(gFM,title,511);setText(hSave,L"Active FM26 session detected automatically");if(!running||emergency){setText(hVision,L"● PAUSED");return;}Img im;if(grab(gFM,im)){captures++;uint64_t hh=hashImg(im);if(lastHash&&hh!=lastHash)changes++;lastHash=hh;setText(hVision,L"● ACTIVE");setText(hMetric1,L"CAPTURES   "+std::to_wstring(captures));setText(hMetric2,L"STATE CHANGES   "+std::to_wstring(changes));setText(hMetric3,L"FRAME   "+std::to_wstring(im.w)+L" × "+std::to_wstring(im.h));setText(hActivity,L"Observing current save continuously. Context refreshes when the FM26 screen changes.");}}
LRESULT CALLBACK WndProc(HWND h,UINT m,WPARAM w,LPARAM l){switch(m){
case WM_CREATE:{dirs();loadMode();fontTitle=CreateFontW(28,0,0,0,FW_BOLD,0,0,0,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Segoe UI");fontHead=CreateFontW(18,0,0,0,FW_SEMIBOLD,0,0,0,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Segoe UI");fontBody=CreateFontW(16,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Segoe UI");fontSmall=CreateFontW(14,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Segoe UI");bgBrush=CreateSolidBrush(BG);panelBrush=CreateSolidBrush(PANEL);
label(h,L"FM26 AI MANAGER",28,22,330,42,fontTitle);label(h,L"V1 • PORTABLE CONTROL CENTER",30,61,310,22,fontSmall);hConn=label(h,L"● WAITING FOR FM26",770,30,260,28,fontHead);hSave=label(h,L"No active save session detected",770,62,300,22,fontSmall);
label(h,L"AI CONTROL",30,120,200,28,fontHead);button(h,L"25%  ASSIST",30,158,190,52,25);button(h,L"60%  COPILOT",232,158,190,52,60);button(h,L"100%  AUTONOMOUS",434,158,220,52,100);button(h,L"EMERGENCY STOP",790,158,230,52,999);
hMode=label(h,modeName().c_str(),30,230,300,28,fontHead);label(h,L"Active policy",30,260,200,20,fontSmall);
label(h,L"SYSTEM STATUS",30,320,220,28,fontHead);label(h,L"FM26",30,360,130,24,fontBody);label(h,L"VISION",30,397,130,24,fontBody);label(h,L"DECISION AI",30,434,130,24,fontBody);label(h,L"SAVE GUARD",30,471,130,24,fontBody);hVision=label(h,L"● IDLE",175,397,180,24,fontBody);hAI=label(h,L"● ACTIVE",175,434,180,24,fontBody);label(h,L"● ACTIVE",175,471,180,24,fontBody);
label(h,L"LIVE INTELLIGENCE",420,320,260,28,fontHead);hMetric1=label(h,L"CAPTURES   0",420,360,270,26,fontBody);hMetric2=label(h,L"STATE CHANGES   0",420,397,270,26,fontBody);hMetric3=label(h,L"FRAME   —",420,434,270,26,fontBody);label(h,L"SAVE CONTEXT",420,485,180,24,fontHead);label(h,L"Automatic • no club profile is hard-coded",420,518,360,24,fontSmall);
label(h,L"AI ACTIVITY",30,570,180,28,fontHead);hActivity=label(h,L"Starting manager core…",30,608,980,56,fontBody);label(h,L"High-risk actions remain safety-gated until their screen executor is verified.",30,690,780,22,fontSmall);SetTimer(h,1,500,nullptr);logLine(L"V1 control center started");return 0;}
case WM_TIMER:updateSession();return 0;
case WM_COMMAND:{int id=LOWORD(w);if(id==25||id==60||id==100){chooseMode(id);return 0;}if(id==999){emergency=true;running=false;setText(hAI,L"● STOPPED");setText(hVision,L"● STOPPED");setText(hActivity,L"EMERGENCY STOP engaged. All agent control is disabled.");logLine(L"Emergency stop");return 0;}return 0;}
case WM_DRAWITEM:{auto*d=(DRAWITEMSTRUCT*)l;int id=(int)d->CtlID;bool active=(id==controlLevel&&!emergency);COLORREF fill=id==999?RGB(72,28,32):(active?RGB(42,83,150):RGB(35,41,52));HBRUSH br=CreateSolidBrush(fill);FillRect(d->hDC,&d->rcItem,br);DeleteObject(br);SetBkMode(d->hDC,TRANSPARENT);SetTextColor(d->hDC,TEXT);wchar_t t[128]{};GetWindowTextW(d->hwndItem,t,127);DrawTextW(d->hDC,t,-1,&d->rcItem,DT_CENTER|DT_VCENTER|DT_SINGLELINE);return TRUE;}
case WM_CTLCOLORSTATIC:{HDC dc=(HDC)w;SetBkColor(dc,BG);SetTextColor(dc,TEXT);return (LRESULT)bgBrush;}
case WM_ERASEBKGND:{RECT r;GetClientRect(h,&r);FillRect((HDC)w,&r,bgBrush);return 1;}
case WM_DESTROY:KillTimer(h,1);DeleteObject(fontTitle);DeleteObject(fontHead);DeleteObject(fontBody);DeleteObject(fontSmall);DeleteObject(bgBrush);DeleteObject(panelBrush);PostQuitMessage(0);return 0;}
return DefWindowProcW(h,m,w,l);}
int WINAPI wWinMain(HINSTANCE inst,HINSTANCE,LPWSTR,int show){auto dpi=(BOOL(WINAPI*)())GetProcAddress(GetModuleHandleW(L"user32.dll"),"SetProcessDPIAware");if(dpi)dpi();WNDCLASSW wc{};wc.lpfnWndProc=WndProc;wc.hInstance=inst;wc.lpszClassName=L"FM26AIV1";wc.hCursor=LoadCursor(nullptr,IDC_ARROW);wc.hbrBackground=(HBRUSH)GetStockObject(BLACK_BRUSH);RegisterClassW(&wc);HWND h=CreateWindowW(wc.lpszClassName,L"FM26 AI Manager V1",WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX,CW_USEDEFAULT,CW_USEDEFAULT,1080,780,nullptr,nullptr,inst,nullptr);ShowWindow(h,show);UpdateWindow(h);MSG msg{};while(GetMessageW(&msg,nullptr,0,0)){TranslateMessage(&msg);DispatchMessageW(&msg);}return 0;}
