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
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <vector>
#include <array>
#include <cmath>
#include <algorithm>
#include <iomanip>
#include "decision_engine.h"
#include "action_queue.h"

static HWND gStatus,gScreen,gConfidence,gAction,gQueue; static HWND gFM=nullptr; static bool armed=false,paused=false; static AgentMode gMode=AgentMode::Observe; static ActionQueue gActions; static DecisionEngine gDecision; static SafetyValidator gSafety;
std::wstring nowstr(){SYSTEMTIME s;GetLocalTime(&s);wchar_t b[64];swprintf(b,64,L"%04d-%02d-%02d %02d:%02d:%02d",s.wYear,s.wMonth,s.wDay,s.wHour,s.wMinute,s.wSecond);return b;}
void dirs(){for(auto p:{L"logs",L"data",L"data\\captures",L"data\\training",L"config",L"database"})CreateDirectoryW(p,NULL);}
void logl(const std::wstring&s){std::wofstream f(L"logs\\manager.log",std::ios::app);f<<nowstr()<<L" | "<<s<<L"\n";}
void txt(HWND h,const std::wstring&s){SetWindowTextW(h,s.c_str());}
BOOL CALLBACK ep(HWND h,LPARAM p){if(!IsWindowVisible(h))return TRUE;wchar_t t[512]={};GetWindowTextW(h,t,511);std::wstring x(t);if(x.find(L"Football Manager 26")!=std::wstring::npos||x.find(L"Football Manager 2026")!=std::wstring::npos){*(HWND*)p=h;return FALSE;}return TRUE;}
HWND findFM(){HWND h=nullptr;EnumWindows(ep,(LPARAM)&h);return h;}
struct Img{int w=0,h=0;std::vector<unsigned char> bgra;};
bool grab(HWND h,Img&im){RECT r{};if(!GetClientRect(h,&r))return false;im.w=r.right;im.h=r.bottom;if(im.w<32||im.h<32)return false;HDC s=GetDC(h),m=CreateCompatibleDC(s);HBITMAP b=CreateCompatibleBitmap(s,im.w,im.h);auto o=SelectObject(m,b);BOOL ok=BitBlt(m,0,0,im.w,im.h,s,0,0,SRCCOPY|CAPTUREBLT);BITMAPINFOHEADER bi{};bi.biSize=sizeof(bi);bi.biWidth=im.w;bi.biHeight=-im.h;bi.biPlanes=1;bi.biBitCount=32;bi.biCompression=BI_RGB;im.bgra.resize((size_t)im.w*im.h*4);if(ok)ok=GetDIBits(m,b,0,im.h,im.bgra.data(),(BITMAPINFO*)&bi,DIB_RGB_COLORS);SelectObject(m,o);DeleteObject(b);DeleteDC(m);ReleaseDC(h,s);return ok;}
bool saveBmp(const Img&i,const std::wstring&p){BITMAPINFOHEADER bi{};bi.biSize=sizeof(bi);bi.biWidth=i.w;bi.biHeight=-i.h;bi.biPlanes=1;bi.biBitCount=32;bi.biCompression=BI_RGB;DWORD n=(DWORD)i.bgra.size();BITMAPFILEHEADER bf{};bf.bfType=0x4D42;bf.bfOffBits=sizeof(bf)+sizeof(bi);bf.bfSize=bf.bfOffBits+n;std::ofstream f(std::filesystem::path(p),std::ios::binary);if(!f)return false;f.write((char*)&bf,sizeof(bf));f.write((char*)&bi,sizeof(bi));f.write((char*)i.bgra.data(),n);return true;}
using Sig=std::array<float,192>; Sig signature(const Img&i){Sig s{};int k=0;for(int gy=0;gy<12;gy++)for(int gx=0;gx<16;gx++){long long sum=0,c=0;int x0=gx*i.w/16,x1=(gx+1)*i.w/16,y0=gy*i.h/12,y1=(gy+1)*i.h/12;int sx=std::max(1,(x1-x0)/8),sy=std::max(1,(y1-y0)/8);for(int y=y0;y<y1;y+=sy)for(int x=x0;x<x1;x+=sx){auto q=&i.bgra[((size_t)y*i.w+x)*4];sum+=(q[2]*299+q[1]*587+q[0]*114)/1000;c++;}s[k++]=c?float(sum)/float(c):0;}return s;}
void saveSig(const Sig&s,const std::wstring&label){CreateDirectoryW((L"data\\training\\"+label).c_str(),NULL);std::wofstream f(L"data\\training\\"+label+L"\\signatures.csv",std::ios::app);for(size_t i=0;i<s.size();i++){if(i)f<<L',';f<<s[i];}f<<L"\n";}
std::vector<Sig> loadSigs(const std::wstring&label){std::vector<Sig>v;std::wifstream f(L"data\\training\\"+label+L"\\signatures.csv");std::wstring line;while(std::getline(f,line)){std::wstringstream ss(line);Sig a{};std::wstring q;int k=0;while(std::getline(ss,q,L',')&&k<192){try{a[k++]=std::stof(q);}catch(...){}}if(k==192)v.push_back(a);}return v;}
float dist(const Sig&a,const Sig&b){double z=0;for(int i=0;i<192;i++){double d=a[i]-b[i];z+=d*d;}return (float)std::sqrt(z/192.0);}
std::pair<std::wstring,float> classify(const Img&i){auto s=signature(i);std::wstring labels[]={L"HOME",L"SQUAD",L"PLAYER_PROFILE",L"FINANCES"};std::wstring best=L"UNKNOWN";float bd=1e9f;int total=0;for(auto&l:labels){auto vv=loadSigs(l);total+=(int)vv.size();for(auto&a:vv){float d=dist(s,a);if(d<bd){bd=d;best=l;}}}if(!total)return {L"UNKNOWN",0};float conf=std::clamp(1.0f-bd/70.0f,0.0f,1.0f);if(conf<0.62f)best=L"UNKNOWN";return {best,conf};}
std::wstring stamp(){SYSTEMTIME s;GetLocalTime(&s);wchar_t b[64];swprintf(b,64,L"%04d%02d%02d_%02d%02d%02d_%03d",s.wYear,s.wMonth,s.wDay,s.wHour,s.wMinute,s.wSecond,s.wMilliseconds);return b;}
void label(HWND owner,const std::wstring&l){gFM=findFM();Img i;if(!gFM||!grab(gFM,i)){MessageBoxW(owner,L"FM26/capture unavailable.",L"Training",MB_OK|MB_ICONWARNING);return;}auto dir=L"data\\training\\"+l;CreateDirectoryW(dir.c_str(),NULL);auto p=dir+L"\\"+stamp()+L".bmp";saveBmp(i,p);saveSig(signature(i),l);txt(gAction,L"Training sample saved: "+l);logl(L"Training sample "+l);}

std::wstring exeDir(){wchar_t b[MAX_PATH]={};GetModuleFileNameW(NULL,b,MAX_PATH);return std::filesystem::path(b).parent_path().wstring();}
std::wstring escj(const std::wstring&s){std::wstring o;for(auto c:s){if(c==L'\\'||c==L'\"'){o+=L'\\';o+=c;}else if(c==L'\n')o+=L"\\n";else o+=c;}return o;}
void exportDiag(HWND owner){
 dirs(); CreateDirectoryW(L"diagnostics",NULL);
 gFM=findFM(); if(!gFM){MessageBoxW(owner,L"Open Football Manager 2026 first, then run the diagnostic again.",L"FM26 Diagnostic",MB_OK|MB_ICONWARNING);return;}
 RECT cr{},wr{};GetClientRect(gFM,&cr);GetWindowRect(gFM,&wr);wchar_t title[512]={};GetWindowTextW(gFM,title,511);
 DWORD pid=0;GetWindowThreadProcessId(gFM,&pid);UINT dpi=96;auto gd=(UINT(WINAPI*)(HWND))GetProcAddress(GetModuleHandleW(L"user32.dll"),"GetDpiForWindow");if(gd)dpi=gd(gFM);
 Img im;bool cap=grab(gFM,im);std::wstring shot=L"diagnostics\\fm26_current_screen.bmp";if(cap)saveBmp(im,shot);
 std::wofstream j(L"diagnostics\\system.json");j<<L"{\n  \"version\": \"0.6\",\n  \"mode\": \"READ_ONLY\",\n  \"fm26_detected\": true,\n  \"window_title\": \""<<escj(title)<<L"\",\n  \"pid\": "<<pid<<L",\n  \"client_width\": "<<(cr.right-cr.left)<<L",\n  \"client_height\": "<<(cr.bottom-cr.top)<<L",\n  \"window_width\": "<<(wr.right-wr.left)<<L",\n  \"window_height\": "<<(wr.bottom-wr.top)<<L",\n  \"dpi\": "<<dpi<<L",\n  \"dpi_scale_percent\": "<<(dpi*100/96)<<L",\n  \"capture_ok\": "<<(cap?L"true":L"false")<<L"\n}\n";
 std::wofstream r(L"diagnostics\\README_DIAGNOSTIC.txt");r<<L"FM26 AI MANAGER V0.6 DIAGNOSTIC\n\nThis build is READ-ONLY. It does not control mouse/keyboard and does not modify the FM26 save.\n\nFiles:\n- system.json: detected FM26/window/DPI information\n- fm26_current_screen.bmp: current FM26 client capture\n- ../logs/manager.log: program log\n\nFor screen training, use the four TRAIN buttons while the corresponding FM26 screen is open.\n";
 txt(gAction,L"Diagnostic exported to the diagnostics folder.");logl(L"Diagnostic exported");
 MessageBoxW(owner,L"Diagnostic complete. Open the program folder and send the 'diagnostics' folder (and 'logs' folder if present) back in ChatGPT.",L"FM26 Diagnostic",MB_OK|MB_ICONINFORMATION);
}
LRESULT CALLBACK WndProc(HWND h,UINT m,WPARAM w,LPARAM l){switch(m){case WM_CREATE:{dirs();CreateWindowW(L"STATIC",L"FM26 AI MANAGER — Portable V0.6 Diagnostic",WS_CHILD|WS_VISIBLE,20,16,650,26,h,0,0,0);CreateWindowW(L"STATIC",L"FM26:",WS_CHILD|WS_VISIBLE,20,55,70,22,h,0,0,0);gStatus=CreateWindowW(L"STATIC",L"Searching...",WS_CHILD|WS_VISIBLE,95,55,500,22,h,0,0,0);CreateWindowW(L"STATIC",L"SCREEN:",WS_CHILD|WS_VISIBLE,20,88,70,22,h,0,0,0);gScreen=CreateWindowW(L"STATIC",L"UNKNOWN",WS_CHILD|WS_VISIBLE,95,88,250,22,h,0,0,0);CreateWindowW(L"STATIC",L"CONFIDENCE:",WS_CHILD|WS_VISIBLE,360,88,100,22,h,0,0,0);gConfidence=CreateWindowW(L"STATIC",L"0%",WS_CHILD|WS_VISIBLE,465,88,100,22,h,0,0,0);gAction=CreateWindowW(L"STATIC",L"Waiting for FM26...",WS_CHILD|WS_VISIBLE,20,125,630,42,h,0,0,0);CreateWindowW(L"BUTTON",L"START VISION",WS_CHILD|WS_VISIBLE,20,180,125,38,h,(HMENU)1,0,0);CreateWindowW(L"BUTTON",L"PAUSE",WS_CHILD|WS_VISIBLE,155,180,90,38,h,(HMENU)2,0,0);CreateWindowW(L"BUTTON",L"EMERGENCY STOP",WS_CHILD|WS_VISIBLE,255,180,150,38,h,(HMENU)3,0,0);CreateWindowW(L"BUTTON",L"EXPORT DIAGNOSTIC",WS_CHILD|WS_VISIBLE,415,180,165,38,h,(HMENU)4,0,0);CreateWindowW(L"STATIC",L"TRAIN CURRENT FM26 SCREEN (read-only):",WS_CHILD|WS_VISIBLE,20,240,350,22,h,0,0,0);CreateWindowW(L"BUTTON",L"HOME / PORTAL",WS_CHILD|WS_VISIBLE,20,272,135,36,h,(HMENU)10,0,0);CreateWindowW(L"BUTTON",L"SQUAD",WS_CHILD|WS_VISIBLE,165,272,105,36,h,(HMENU)11,0,0);CreateWindowW(L"BUTTON",L"PLAYER PROFILE",WS_CHILD|WS_VISIBLE,280,272,140,36,h,(HMENU)12,0,0);CreateWindowW(L"BUTTON",L"FINANCES",WS_CHILD|WS_VISIBLE,430,272,110,36,h,(HMENU)13,0,0);CreateWindowW(L"STATIC",L"Safety gate: V0.6 is diagnostic/read-only. Mouse, keyboard and save control are disabled.",WS_CHILD|WS_VISIBLE,20,330,640,44,h,0,0,0);gQueue=CreateWindowW(L"STATIC",L"ACTION QUEUE: empty",WS_CHILD|WS_VISIBLE,20,385,700,90,h,0,0,0);SetTimer(h,1,1200,NULL);logl(L"V0.6 started");return 0;}case WM_TIMER:{gFM=findFM();if(!gFM){txt(gStatus,L"NOT DETECTED");txt(gScreen,L"UNKNOWN");txt(gConfidence,L"0%");return 0;}txt(gStatus,L"DETECTED — read-only");if(armed&&!paused){Img i;if(grab(gFM,i)){auto r=classify(i);txt(gScreen,r.first);wchar_t b[32];swprintf(b,32,L"%.0f%%",r.second*100);txt(gConfidence,b);txt(gAction,r.first==L"UNKNOWN"?L"Screen not confidently recognized. Training samples may be needed.":L"Screen recognized: "+r.first);
ClubState cs; cs.fmDetected=true; cs.screen=r.first; cs.screenConfidence=r.second; cs.paused=paused;
auto objs=gDecision.objectives(cs); gActions.clear();
if(!objs.empty()){ ProposedAction a; a.id=L"inspect"; a.description=L"Analyse current screen for objective: "+objs.front().description; a.requiredScreen=r.first; a.risk=Risk::Low; a.confidence=r.second; a.changesSave=false; gActions.push(a); }
auto qa=gActions.peek(); if(qa){auto sr=gSafety.validate(cs,*qa,gMode); txt(gQueue,L"ACTION QUEUE\r\n1. "+qa->description+L"\r\nVALIDATION: "+(sr.allowed?L"PASS — DRY-RUN ONLY":L"BLOCKED — "+sr.reason));} else txt(gQueue,L"ACTION QUEUE: empty");}}return 0;}case WM_COMMAND:{int id=LOWORD(w);if(id==1){armed=true;paused=false;txt(gAction,L"Vision active. Read-only.");}else if(id==2){paused=!paused;txt(gAction,paused?L"Vision paused.":L"Vision resumed.");}else if(id==3){armed=false;paused=false;txt(gAction,L"EMERGENCY STOP — agent idle.");}else if(id==4){exportDiag(h);}else if(id>=10&&id<=13){std::wstring a[]={L"HOME",L"SQUAD",L"PLAYER_PROFILE",L"FINANCES"};label(h,a[id-10]);}return 0;}case WM_DESTROY:KillTimer(h,1);PostQuitMessage(0);return 0;}return DefWindowProcW(h,m,w,l);}
int WINAPI wWinMain(HINSTANCE x,HINSTANCE,LPWSTR,int n){auto dpi=(BOOL(WINAPI*)())GetProcAddress(GetModuleHandleW(L"user32.dll"),"SetProcessDPIAware");if(dpi)dpi();WNDCLASSW c{};c.lpfnWndProc=WndProc;c.hInstance=x;c.lpszClassName=L"FM26AIV03";c.hCursor=LoadCursor(NULL,IDC_ARROW);c.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);RegisterClassW(&c);HWND h=CreateWindowW(c.lpszClassName,L"FM26 AI Manager — Portable V0.6",WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX,CW_USEDEFAULT,CW_USEDEFAULT,760,560,0,0,x,0);ShowWindow(h,n);UpdateWindow(h);MSG q;while(GetMessageW(&q,0,0,0)){TranslateMessage(&q);DispatchMessageW(&q);}return 0;}
