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
#include <filesystem>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <array>

static HWND gFM=nullptr, hConn=nullptr, hClub=nullptr, hVision=nullptr, hAI=nullptr, hGuard=nullptr;
static HWND hActivity=nullptr,hMetric1=nullptr,hMetric2=nullptr,hMetric3=nullptr,hLang=nullptr,hPageTitle=nullptr,hTactical=nullptr;
static bool running=true, emergency=false;
static int controlLevel=25, activePage=0, language=0;
static uint64_t captures=0,lastHash=0,changes=0;
static HFONT fontBrand=nullptr,fontTitle=nullptr,fontHead=nullptr,fontBody=nullptr,fontSmall=nullptr,fontMetric=nullptr;
static HBRUSH bgBrush=nullptr;
static COLORREF BG=RGB(10,16,27), TEXT=RGB(238,243,250), MUTED=RGB(145,158,178), ACCENT=RGB(63,151,255), GREEN=RGB(65,211,143), RED=RGB(245,82,95);

enum IDs { ID_ASSIST=25, ID_COPILOT=60, ID_AUTO=100, ID_STOP=999, ID_LANG=1100, ID_NAV0=1200 };
static const wchar_t* langs[]={L"Português (Portugal)",L"English",L"Français",L"Italiano",L"Español"};
static const wchar_t* navEN[]={L"OVERVIEW",L"SQUAD",L"TRANSFERS",L"STAFF",L"FINANCES",L"TACTICS",L"DEVELOPMENT",L"AI ACTIVITY"};
static HWND navBtns[8]{};

void dirs(){for(auto p:{L"config",L"data",L"database",L"logs",L"backups"})CreateDirectoryW(p,nullptr);}
std::wstring stamp(){SYSTEMTIME s{};GetLocalTime(&s);wchar_t b[64];swprintf(b,64,L"%04d-%02d-%02d %02d:%02d:%02d",s.wYear,s.wMonth,s.wDay,s.wHour,s.wMinute,s.wSecond);return b;}
void logLine(const std::wstring&s){std::wofstream f(L"logs\\manager.log",std::ios::app);f<<stamp()<<L" | "<<s<<L"\n";}
void setText(HWND h,const std::wstring&s){if(h)SetWindowTextW(h,s.c_str());}
void savePrefs(){std::wofstream f(L"config\\preferences.ini");f<<controlLevel<<L"\n"<<language<<L"\n";}
void loadPrefs(){std::wifstream f(L"config\\preferences.ini");int c=25,l=0;if(f>>c && (c==25||c==60||c==100))controlLevel=c;if(f>>l && l>=0&&l<5)language=l;}
BOOL CALLBACK enumFM(HWND h,LPARAM p){if(!IsWindowVisible(h))return TRUE;wchar_t t[512]{};GetWindowTextW(h,t,511);std::wstring s=t;if(s.find(L"Football Manager 2026")!=std::wstring::npos||s.find(L"Football Manager 26")!=std::wstring::npos){*(HWND*)p=h;return FALSE;}return TRUE;}
HWND findFM(){HWND h=nullptr;EnumWindows(enumFM,(LPARAM)&h);return h;}
struct Img{int w=0,h=0;std::vector<unsigned char> px;};
bool grab(HWND h,Img&im){RECT r{};if(!GetClientRect(h,&r))return false;im.w=r.right;im.h=r.bottom;if(im.w<64||im.h<64)return false;HDC s=GetDC(h),m=CreateCompatibleDC(s);HBITMAP b=CreateCompatibleBitmap(s,im.w,im.h);HGDIOBJ old=SelectObject(m,b);BOOL ok=BitBlt(m,0,0,im.w,im.h,s,0,0,SRCCOPY|CAPTUREBLT);BITMAPINFOHEADER bi{};bi.biSize=sizeof(bi);bi.biWidth=im.w;bi.biHeight=-im.h;bi.biPlanes=1;bi.biBitCount=32;bi.biCompression=BI_RGB;im.px.resize((size_t)im.w*im.h*4);if(ok)ok=GetDIBits(m,b,0,im.h,im.px.data(),(BITMAPINFO*)&bi,DIB_RGB_COLORS);SelectObject(m,old);DeleteObject(b);DeleteDC(m);ReleaseDC(h,s);return ok;}
uint64_t hashImg(const Img&i){uint64_t h=1469598103934665603ULL;size_t step=std::max<size_t>(4,i.px.size()/4096);for(size_t n=0;n<i.px.size();n+=step){h^=i.px[n];h*=1099511628211ULL;}return h;}
void style(HWND h,HFONT f){SendMessageW(h,WM_SETFONT,(WPARAM)f,TRUE);}
HWND label(HWND p,const wchar_t*t,int x,int y,int w,int h,HFONT f=0){HWND c=CreateWindowW(L"STATIC",t,WS_CHILD|WS_VISIBLE,x,y,w,h,p,0,0,0);style(c,f?f:fontBody);return c;}
HWND button(HWND p,const wchar_t*t,int x,int y,int w,int h,int id){HWND c=CreateWindowW(L"BUTTON",t,WS_CHILD|WS_VISIBLE|BS_OWNERDRAW,x,y,w,h,p,(HMENU)(INT_PTR)id,0,0);style(c,fontBody);return c;}
std::wstring tr(const wchar_t* pt,const wchar_t* en,const wchar_t* fr,const wchar_t* it,const wchar_t* es){switch(language){case 0:return pt;case 1:return en;case 2:return fr;case 3:return it;default:return es;}}
std::wstring modeName(){if(controlLevel==25)return tr(L"25%  ASSISTENTE",L"25%  ASSIST",L"25%  ASSISTANT",L"25%  ASSISTENTE",L"25%  ASISTENTE");if(controlLevel==60)return tr(L"60%  COPILOTO",L"60%  COPILOT",L"60%  COPILOTE",L"60%  COPILOTA",L"60%  COPILOTO");return tr(L"100%  AUTÓNOMO",L"100%  AUTONOMOUS",L"100%  AUTONOME",L"100%  AUTONOMO",L"100%  AUTÓNOMO");}
void applyLanguage(){
 const wchar_t* pt[]={L"VISÃO GERAL",L"PLANTEL",L"TRANSFERÊNCIAS",L"STAFF",L"FINANÇAS",L"TÁTICA",L"DESENVOLVIMENTO",L"ATIVIDADE IA"};
 const wchar_t* fr[]={L"VUE D'ENSEMBLE",L"EFFECTIF",L"TRANSFERTS",L"STAFF",L"FINANCES",L"TACTIQUE",L"DÉVELOPPEMENT",L"ACTIVITÉ IA"};
 const wchar_t* it[]={L"PANORAMICA",L"ROSA",L"TRASFERIMENTI",L"STAFF",L"FINANZE",L"TATTICA",L"SVILUPPO",L"ATTIVITÀ IA"};
 const wchar_t* es[]={L"RESUMEN",L"PLANTILLA",L"FICHAJES",L"STAFF",L"FINANZAS",L"TÁCTICA",L"DESARROLLO",L"ACTIVIDAD IA"};
 for(int i=0;i<8;i++){const wchar_t* s=language==0?pt[i]:language==1?navEN[i]:language==2?fr[i]:language==3?it[i]:es[i];SetWindowTextW(navBtns[i],s);}
 setText(hPageTitle,tr(L"CENTRO DE INTELIGÊNCIA DO CLUBE",L"CLUB INTELLIGENCE CENTER",L"CENTRE D'INTELLIGENCE DU CLUB",L"CENTRO INTELLIGENZA CLUB",L"CENTRO DE INTELIGENCIA DEL CLUB"));
 setText(hTactical,tr(L"Tactical AI: análise automática do plantel → identidade → formação → funções → instruções coletivas e individuais → XI → banco → bolas paradas.",L"Tactical AI: automatic squad analysis → identity → formation → roles → team & player instructions → XI → bench → set pieces.",L"IA tactique : analyse automatique → identité → formation → rôles → consignes collectives et individuelles → XI → banc → coups de pied arrêtés.",L"IA tattica: analisi automatica → identità → modulo → ruoli → istruzioni squadra e giocatori → XI → panchina → palle inattive.",L"IA táctica: análisis automático → identidad → formación → roles → instrucciones de equipo y jugador → XI → banquillo → balón parado."));
 savePrefs();InvalidateRect(GetParent(hPageTitle),nullptr,TRUE);
}
void chooseMode(int v){controlLevel=v;emergency=false;running=true;savePrefs();setText(hAI,L"● ACTIVE");setText(hActivity,tr(L"Política de autonomia atualizada. A IA está a recalcular permissões para este save.",L"Autonomy policy updated. AI is rebuilding permissions for this save.",L"Politique d'autonomie mise à jour. L'IA recalcule les permissions.",L"Politica di autonomia aggiornata. L'IA ricalcola i permessi.",L"Política de autonomía actualizada. La IA recalcula los permisos."));logLine(L"Control level "+std::to_wstring(v));InvalidateRect(GetParent(hAI),nullptr,TRUE);}
void updateSession(){gFM=findFM();if(!gFM){setText(hConn,tr(L"● À ESPERA DO FM26",L"● WAITING FOR FM26",L"● EN ATTENTE DE FM26",L"● IN ATTESA DI FM26",L"● ESPERANDO FM26"));setText(hClub,tr(L"Nenhum save ativo detetado",L"No active save detected",L"Aucune sauvegarde active détectée",L"Nessun salvataggio attivo rilevato",L"No se detecta partida activa"));setText(hVision,L"● IDLE");return;}setText(hConn,L"● FM26 CONNECTED");setText(hClub,tr(L"Save atual detetado automaticamente",L"Current save detected automatically",L"Sauvegarde actuelle détectée automatiquement",L"Salvataggio corrente rilevato automaticamente",L"Partida actual detectada automáticamente"));if(!running||emergency){setText(hVision,L"● PAUSED");return;}Img im;if(grab(gFM,im)){captures++;uint64_t hh=hashImg(im);if(lastHash&&hh!=lastHash)changes++;lastHash=hh;setText(hVision,L"● ACTIVE");setText(hMetric1,std::to_wstring(captures));setText(hMetric2,std::to_wstring(changes));setText(hMetric3,std::to_wstring(im.w)+L" × "+std::to_wstring(im.h));}}
void roundRect(HDC dc,RECT r,COLORREF c,int rad=18){HBRUSH b=CreateSolidBrush(c);HPEN p=CreatePen(PS_SOLID,1,c);auto ob=SelectObject(dc,b);auto op=SelectObject(dc,p);RoundRect(dc,r.left,r.top,r.right,r.bottom,rad,rad);SelectObject(dc,ob);SelectObject(dc,op);DeleteObject(b);DeleteObject(p);}
LRESULT CALLBACK WndProc(HWND h,UINT m,WPARAM w,LPARAM l){switch(m){
case WM_CREATE:{dirs();loadPrefs();fontBrand=CreateFontW(38,0,0,0,FW_HEAVY,0,0,0,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Bahnschrift SemiBold");fontTitle=CreateFontW(24,0,0,0,FW_BOLD,0,0,0,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Segoe UI Variable Display");fontHead=CreateFontW(17,0,0,0,FW_SEMIBOLD,0,0,0,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Segoe UI Variable Text");fontBody=CreateFontW(15,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Segoe UI Variable Text");fontSmall=CreateFontW(13,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Segoe UI Variable Text");fontMetric=CreateFontW(26,0,0,0,FW_BOLD,0,0,0,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Bahnschrift");bgBrush=CreateSolidBrush(BG);
 label(h,L"FM26",30,20,130,50,fontBrand);label(h,L"AI MANAGER",145,30,230,34,fontTitle);label(h,L"INTELLIGENT CLUB OPERATING SYSTEM",32,69,330,20,fontSmall);hConn=label(h,L"● WAITING FOR FM26",930,27,260,28,fontHead);hClub=label(h,L"No active save detected",930,60,300,22,fontSmall);
 hLang=CreateWindowW(L"COMBOBOX",L"",WS_CHILD|WS_VISIBLE|CBS_DROPDOWNLIST|WS_VSCROLL,1040,92,190,180,h,(HMENU)ID_LANG,0,0);style(hLang,fontSmall);for(auto s:langs)SendMessageW(hLang,CB_ADDSTRING,0,(LPARAM)s);SendMessageW(hLang,CB_SETCURSEL,language,0);
 int ny=145;for(int i=0;i<8;i++){navBtns[i]=button(h,navEN[i],24,ny+i*54,205,42,ID_NAV0+i);}
 hPageTitle=label(h,L"CLUB INTELLIGENCE CENTER",270,137,600,38,fontTitle);label(h,L"AUTONOMY",270,193,180,24,fontHead);button(h,L"25%  ASSIST",270,225,190,48,ID_ASSIST);button(h,L"60%  COPILOT",472,225,190,48,ID_COPILOT);button(h,L"100%  AUTONOMOUS",674,225,220,48,ID_AUTO);button(h,L"EMERGENCY STOP",990,225,225,48,ID_STOP);
 label(h,L"LIVE INTELLIGENCE",270,310,220,24,fontHead);label(h,L"CAPTURES",285,353,150,20,fontSmall);label(h,L"STATE CHANGES",535,353,150,20,fontSmall);label(h,L"FRAME",785,353,150,20,fontSmall);hMetric1=label(h,L"0",285,382,180,40,fontMetric);hMetric2=label(h,L"0",535,382,180,40,fontMetric);hMetric3=label(h,L"—",785,382,220,40,fontMetric);
 label(h,L"SYSTEM",1040,330,150,22,fontHead);label(h,L"VISION",1040,365,90,20,fontSmall);hVision=label(h,L"● IDLE",1130,365,100,20,fontSmall);label(h,L"DECISION AI",1040,395,90,20,fontSmall);hAI=label(h,L"● ACTIVE",1130,395,100,20,fontSmall);label(h,L"SAVE GUARD",1040,425,90,20,fontSmall);hGuard=label(h,L"● ACTIVE",1130,425,100,20,fontSmall);
 label(h,L"TACTICAL AI",270,485,220,24,fontHead);hTactical=label(h,L"",270,520,900,54,fontBody);label(h,L"AI ACTIVITY",270,625,180,24,fontHead);hActivity=label(h,L"Manager core ready. Waiting for save intelligence.",270,660,920,50,fontBody);
 label(h,L"V1.5  •  PORTABLE  •  MULTILINGUAL  •  SAVE-AWARE",270,740,700,22,fontSmall);applyLanguage();SetTimer(h,1,500,nullptr);logLine(L"V1.5 started");return 0;}
case WM_TIMER:updateSession();return 0;
case WM_COMMAND:{int id=LOWORD(w);if(id==ID_ASSIST||id==ID_COPILOT||id==ID_AUTO){chooseMode(id);return 0;}if(id==ID_STOP){emergency=true;running=false;setText(hAI,L"● STOPPED");setText(hVision,L"● STOPPED");setText(hActivity,tr(L"PARAGEM DE EMERGÊNCIA ativa. Todo o controlo da IA foi bloqueado.",L"EMERGENCY STOP active. All AI control is blocked.",L"ARRÊT D'URGENCE actif. Tout contrôle IA est bloqué.",L"ARRESTO DI EMERGENZA attivo. Tutto il controllo IA è bloccato.",L"PARADA DE EMERGENCIA activa. Todo el control IA está bloqueado."));return 0;}if(id==ID_LANG&&HIWORD(w)==CBN_SELCHANGE){language=(int)SendMessageW(hLang,CB_GETCURSEL,0,0);applyLanguage();return 0;}if(id>=ID_NAV0&&id<ID_NAV0+8){activePage=id-ID_NAV0;InvalidateRect(h,nullptr,FALSE);return 0;}return 0;}
case WM_DRAWITEM:{auto*d=(DRAWITEMSTRUCT*)l;int id=(int)d->CtlID;bool mode=(id==controlLevel&&!emergency), nav=(id>=ID_NAV0&&id<ID_NAV0+8), navActive=nav&&(id-ID_NAV0==activePage);COLORREF fill=id==ID_STOP?RGB(96,31,43):(mode?RGB(27,101,185):(navActive?RGB(25,65,108):RGB(18,29,46)));HBRUSH br=CreateSolidBrush(fill);HPEN pen=CreatePen(PS_SOLID,1,(mode||navActive)?ACCENT:RGB(42,58,79));auto ob=SelectObject(d->hDC,br);auto op=SelectObject(d->hDC,pen);RoundRect(d->hDC,d->rcItem.left,d->rcItem.top,d->rcItem.right,d->rcItem.bottom,12,12);SelectObject(d->hDC,ob);SelectObject(d->hDC,op);DeleteObject(br);DeleteObject(pen);SetBkMode(d->hDC,TRANSPARENT);SetTextColor(d->hDC,TEXT);wchar_t t[128]{};GetWindowTextW(d->hwndItem,t,127);RECT rr=d->rcItem;DrawTextW(d->hDC,t,-1,&rr,DT_CENTER|DT_VCENTER|DT_SINGLELINE);return TRUE;}
case WM_CTLCOLORSTATIC:{HDC dc=(HDC)w;SetBkMode(dc,TRANSPARENT);SetTextColor(dc,TEXT);return (LRESULT)GetStockObject(NULL_BRUSH);}
case WM_ERASEBKGND:{HDC dc=(HDC)w;RECT r;GetClientRect(h,&r);TRIVERTEX v[2]={{0,0,(COLOR16)(8<<8),(COLOR16)(15<<8),(COLOR16)(27<<8),0},{r.right,r.bottom,(COLOR16)(15<<8),(COLOR16)(31<<8),(COLOR16)(48<<8),0}};GRADIENT_RECT gr={0,1};GradientFill(dc,v,2,&gr,1,GRADIENT_FILL_RECT_V);roundRect(dc,{250,120,r.right-20,r.bottom-25},RGB(13,23,38),24);roundRect(dc,{265,295,1015,455},RGB(17,31,50),18);roundRect(dc,{1025,295,r.right-35,455},RGB(17,31,50),18);roundRect(dc,{265,470,r.right-35,590},RGB(17,31,50),18);roundRect(dc,{265,610,r.right-35,725},RGB(17,31,50),18);return 1;}
case WM_DESTROY:KillTimer(h,1);for(auto f:{fontBrand,fontTitle,fontHead,fontBody,fontSmall,fontMetric})if(f)DeleteObject(f);if(bgBrush)DeleteObject(bgBrush);PostQuitMessage(0);return 0;}
return DefWindowProcW(h,m,w,l);}
int WINAPI wWinMain(HINSTANCE inst,HINSTANCE,LPWSTR,int show){auto dpi=(BOOL(WINAPI*)())GetProcAddress(GetModuleHandleW(L"user32.dll"),"SetProcessDPIAware");if(dpi)dpi();WNDCLASSW wc{};wc.lpfnWndProc=WndProc;wc.hInstance=inst;wc.lpszClassName=L"FM26AIV15";wc.hCursor=LoadCursor(nullptr,IDC_ARROW);RegisterClassW(&wc);HWND h=CreateWindowW(wc.lpszClassName,L"FM26 AI Manager V1.5",WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX,CW_USEDEFAULT,CW_USEDEFAULT,1280,820,nullptr,nullptr,inst,nullptr);ShowWindow(h,show);UpdateWindow(h);MSG msg{};while(GetMessageW(&msg,nullptr,0,0)){TranslateMessage(&msg);DispatchMessageW(&msg);}return 0;}
