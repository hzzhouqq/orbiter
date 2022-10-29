// ==============================================================
// Atmospheric controls implementation
// Part of the ORBITER VISUALISATION PROJECT (OVP)
// Dual licensed under GPL v3 and LGPL v3
// Copyright (C) 2014 - 2016 Jarmo Nikkanen
// ==============================================================

#include "D3D9Client.h"
#include "resource.h"
#include "D3D9Config.h"
#include "AtmoControls.h"
#include "Commctrl.h"
#include "vObject.h"
#include "vPlanet.h"
#include "Mesh.h"
#include "Scene.h"
#include <stdio.h>

using namespace oapi;

extern HINSTANCE g_hInst;
extern D3D9Client *g_client;

// ==============================================================

// Defaut c'tor to init members
ScatterParams::ScatterParams() :
	red      ( 0.650 ),  // 0.400 ... 0.700
	green    ( 0.500 ),  // 0.400 ... 0.700
	blue     ( 0.480 ),  // 0.400 ... 0.700
	rpow	 ( 4.0 ),    // -8.0 ... 8.0
	mpow	 ( 1.0 ),    // -2.0 ... 2.0
	rheight  ( 8.0 ),    // 4.0 ... 40.0 [km]
	mheight  ( 1.0 ),    // 0.0 ... 1.5
	// ----------------------------------------
	trb      ( 1.0 ),    // 0.2 ... 3.0
	// ----------------------------------------
	rayin    ( 1.0 ),    // 0.0 ... 3.0
	ray      ( 1.0 ),	 // 0.0 ... 4.0
	rphase   ( 0.3395 ), // 0.0 ... 3.5
	// ----------------------------------------
	mie      ( 0.0869 ), // 0.0 ... 8.0
	mphase   ( 0.9831 ), // 0.85 ... 0.999
	// ----------------------------------------
	miein	 ( 1.0 ),    // 0.0 ... 2.0
	aux2	 ( 0.0 ),    // 0.0 ... 2.0
	aux3	 ( 0.0 ),
	tgamma	 ( 1.0 ),
	mphaseb  ( 1.0 ),
	hazei    ( 1.7 ),
	tw_bri   ( 0.0 ),
	tw_dst	 ( 0.0 ),
	tw_haze	 ( 0.0 ),
	// ----------------------------------------
	orbalt	 ( 250e3 ),
	visalt	 ( 70e3 )
{
}

// ==============================================================

namespace AtmoControls {

	struct sSlider {
		double min, max;
		HWND hWnd;
		int id;
		int dsp;
		int style;
	};


ScatterParams defs;
ScatterParams *param = NULL;

sSlider Slider[ATM_SLIDER_COUNT];

DWORD atmmode = 0;
DWORD dwCmd = NULL;
HWND hDlg = NULL;
vPlanet *vObj = NULL;

// ==============================================================

HWND CreateToolTip(int toolID, HWND hDlg, PTSTR pszText)
{
    if (!toolID || !hDlg || !pszText) return NULL;
    
    // Get the window of the tool.
    HWND hwndTool = GetDlgItem(hDlg, toolID);
    // Create the tooltip. g_hInst is the global instance handle.
    HWND hwndTip = CreateWindowEx(NULL, TOOLTIPS_CLASS, NULL, WS_POPUP |TTS_ALWAYSTIP | TTS_BALLOON, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hDlg, NULL, g_hInst, NULL);
    
    if (!hwndTool || !hwndTip) return NULL;
                                                          
    // Associate the tooltip with the tool.
    TOOLINFO toolInfo = { 0 };
    toolInfo.cbSize = sizeof(toolInfo);
    toolInfo.hwnd = hDlg;
    toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
    toolInfo.uId = (UINT_PTR)hwndTool;
    toolInfo.lpszText = pszText;
    SendMessage(hwndTip, TTM_ADDTOOL, 0, (LPARAM)&toolInfo);

    return hwndTip;
}

// ==============================================================

void Create()
{
	vObj = NULL;
	hDlg = NULL;

	dwCmd = oapiRegisterCustomCmd("D3D9 Atmospheric Controls", "This dialog allows to control various atmospheric parameters and effects", OpenDlgClbk, NULL);
	
	memset(Slider,0,sizeof(Slider));
	
	// ATTENTION: Order of ScatterParams must match with slider indexes

	Slider[0].id = IDC_ATM_TW_DST;
	Slider[1].id = IDC_ATM_TW_HAZE;
	Slider[2].id = IDC_ATM_TW_BRI;
	Slider[3].id = IDC_ATM_RPOW;
	Slider[4].id = IDC_ATM_IN;
	Slider[5].id = IDC_ATM_RAY;
	Slider[6].id = IDC_ATM_RPHASE;
	Slider[7].id = IDC_ATM_MIE;
	Slider[8].id = IDC_ATM_MPHASE;
	Slider[9].id = IDC_ATM_HEIGHT;
	Slider[10].id = IDC_ATM_AUX2;
	Slider[11].id = IDC_ATM_M_HEIGHT;
	Slider[12].id = IDC_ATM_MPOW;
	Slider[13].id = IDC_ATM_TRB;
	Slider[14].id = IDC_ATM_MIEIN;
	Slider[15].id = IDC_ATM_AUX3;
	Slider[16].id = IDC_ATM_TRGAMMA;
	Slider[17].id = IDC_ATM_AUX4;
	Slider[18].id = IDC_ATM_AUX5;
	
	Slider[0].dsp = IDC_ATD_TW_DST;
	Slider[1].dsp = IDC_ATD_TW_HAZE;
	Slider[2].dsp = IDC_ATD_TW_BRI;
	Slider[3].dsp = IDC_ATD_RPOW;
	Slider[4].dsp = IDC_ATD_IN;
	Slider[5].dsp = IDC_ATD_RAY;
	Slider[6].dsp = IDC_ATD_RPHASE;
	Slider[7].dsp = IDC_ATD_MIE;
	Slider[8].dsp = IDC_ATD_MPHASE;
	Slider[9].dsp = IDC_ATD_HEIGHT;
	Slider[10].dsp = IDC_ATD_AUX2;
	Slider[11].dsp = IDC_ATD_M_HEIGHT;
	Slider[12].dsp = IDC_ATD_MPOW;
	Slider[13].dsp = IDC_ATD_TRB;
	Slider[14].dsp = IDC_ATD_MIEIN;
	Slider[15].dsp = IDC_ATD_AUX3;
	Slider[16].dsp = IDC_ATD_TRGAMMA;
	Slider[17].dsp = IDC_ATD_AUX4;
	Slider[18].dsp = IDC_ATD_AUX5;
}

// ==============================================================

bool IsActive()
{
	return (hDlg!=NULL);
}

// ==============================================================

bool Visualize()
{
	if (!hDlg) return false;
	return SendDlgItemMessage(hDlg, IDC_ATM_DISPLAY, BM_GETCHECK, 0, 0) == BST_CHECKED;
}

// ==============================================================

void Release()
{
	if (dwCmd) oapiUnregisterCustomCmd(dwCmd);
	dwCmd = NULL;
}

// ==============================================================

void OpenDlgClbk(void *context)
{
	HWND l_hDlg = oapiOpenDialog(g_hInst, IDD_D3D9SCATTER, WndProc);

	if (l_hDlg) hDlg = l_hDlg; // otherwise open already
	else return;

	Scene *scene = g_client->GetScene();
	
	if (scene) {
		OBJHANDLE hBody = scene->GetCameraProxyBody();
		if (hBody) vObj = static_cast<vPlanet *>(scene->GetVisObject(hBody));
	}

	if (vObj) param = vObj->GetAtmoParams(atmmode);
	else      param = &defs;

	for (int i=0;i<ATM_SLIDER_COUNT;i++) Slider[i].hWnd = GetDlgItem(hDlg, Slider[i].id);

	// Style flags
	// 1 = unit in [km] 
	// 2 = same for orbital and surface setup
	// 4 = call vPlanet::UpdateAtmoConfig() on change
	// 8 = x^2 "linearization"
	// 16 = x^0.5 "linearization"
	// 32 = x^4 "linearization"
	

	ConfigSlider(IDC_ATM_TW_DST,    0.01, 0.5);
	ConfigSlider(IDC_ATM_TW_HAZE,   0.01, 2.0);
	ConfigSlider(IDC_ATM_TW_BRI,    0.01, 4.0);
	ConfigSlider(IDC_ATM_RPOW,     -8.0, 8.0);
	ConfigSlider(IDC_ATM_MPOW,     -8.0, 8.0);
	ConfigSlider(IDC_ATM_HEIGHT,    3.0, 400.0, 1|2|4|8);
	ConfigSlider(IDC_ATM_M_HEIGHT,  0.2, 10.0, 1|2|4|8);
	// -------------------------------------------------------
	ConfigSlider(IDC_ATM_TRB,	   0.5, 4.0);
	ConfigSlider(IDC_ATM_TRGAMMA,  0.3, 1.5);
	// -------------------------------------------------------
	ConfigSlider(IDC_ATM_RAY,      0.01, 20.0, 32);
	ConfigSlider(IDC_ATM_IN,       0.1, 10.0, 8);
	ConfigSlider(IDC_ATM_RPHASE,   -0.75, 0.75);
	// -------------------------------------------------------
	ConfigSlider(IDC_ATM_MIE,      0.2, 100.0, 8);
	ConfigSlider(IDC_ATM_MPHASE,   0.2, 0.9999, 16);
	ConfigSlider(IDC_ATM_MIEIN,	   0.01, 20.0, 32);
	// -------------------------------------------------------
	ConfigSlider(IDC_ATM_AUX2,	   0.2, 2.0);		//Clouds intensity
	ConfigSlider(IDC_ATM_AUX3,	   0.01, 4.0, 8);	//HDR-B
	ConfigSlider(IDC_ATM_AUX4,		0.02, 6.0);		// Mie Phase-B
	ConfigSlider(IDC_ATM_AUX5,		0.001, 1.0, 8);	// Sun Glare Intensity	
	// -------------------------------------------------------
	CreateToolTip(IDC_ATM_TW_DST,	hDlg, "Light travel behind terminator");
	CreateToolTip(IDC_ATM_TW_HAZE,	hDlg, "Ambient terrain haze intensity");
	CreateToolTip(IDC_ATM_TW_BRI,	hDlg, "Sky brightness during twilight");
	CreateToolTip(IDC_ATM_RPOW,		hDlg, "Main control for atmospheric rayleigh color composition (4.0 for the Earth)");
	CreateToolTip(IDC_ATM_MPOW,		hDlg, "Main control for atmospheric mie color composition");
	CreateToolTip(IDC_ATM_HEIGHT,	hDlg, "Atmosphere Ray scale height (7km - 10km for the Earth)");
	CreateToolTip(IDC_ATM_M_HEIGHT,	hDlg, "Atmosphere Mie scale height (0km - 2km for the Earth)");
	// -------------------------------------------------------
	CreateToolTip(IDC_ATM_TRB,		hDlg, "Terrain/Ocean brightness control (default 1.0)");
	CreateToolTip(IDC_ATM_TRGAMMA,	hDlg, "Terrain/Ocean gamma control value (default 1.0)");
	// -------------------------------------------------------
	CreateToolTip(IDC_ATM_RAY,		hDlg, "Overall control for rayleigh scattering (i.e. Haze stickness, atmosphere transparency, optical depth");
	CreateToolTip(IDC_ATM_IN,		hDlg, "Controls in-scatter out-scatter ratio");
	CreateToolTip(IDC_ATM_RPHASE,	hDlg, "Controls a directional dependency of in-scattered sunlight (Most visible when camera, planet and the sun are aligned)");
	// -------------------------------------------------------
	CreateToolTip(IDC_ATM_MIE,		hDlg, "Overall scale factor for mie scattering. (Mie-particle density)");
	CreateToolTip(IDC_ATM_MPHASE,	hDlg, "Directional strength of Henyey-Greenstein phase function");
	CreateToolTip(IDC_ATM_MIEIN,	hDlg, "Light scattering from mie-particles");
	// -------------------------------------------------------
	CreateToolTip(IDC_ATM_AUX2,		hDlg, "Sun visibility through clouds");
	CreateToolTip(IDC_ATM_AUX3,		hDlg, "'HDR' Exposure factor");
	CreateToolTip(IDC_ATM_AUX4,		hDlg, "Omnidirectional mie scattering scale factor");
	CreateToolTip(IDC_ATM_AUX5,		hDlg, "Sun 'glare' intensity");
	
	SendDlgItemMessageA(hDlg, IDC_ATM_MODE, CB_RESETCONTENT, 0, 0);
	SendDlgItemMessageA(hDlg, IDC_ATM_MODE, CB_ADDSTRING, 0, (LPARAM)"Auto");
	SendDlgItemMessageA(hDlg, IDC_ATM_MODE, CB_ADDSTRING, 0, (LPARAM)"Surface");
	SendDlgItemMessageA(hDlg, IDC_ATM_MODE, CB_ADDSTRING, 0, (LPARAM)"Low Orbit");
	SendDlgItemMessageA(hDlg, IDC_ATM_MODE, CB_ADDSTRING, 0, (LPARAM)"High Orbit");
	SendDlgItemMessageA(hDlg, IDC_ATM_MODE, CB_SETCURSEL, atmmode, 0);

	SetTimer(hDlg, 0, 200, NULL);

	UpdateSliders();
}

// ==============================================================

double GetValue(int id)
{
	for (int i=0;i<ATM_SLIDER_COUNT;i++) if (Slider[i].id==id) return param->data[i];
	LogErr("Invalid Slider ID in AtmoControls");
	return 0.0;
}

// ==============================================================

void ConfigSlider(int id, double min, double max, int style)
{
	for (int i=0;i<ATM_SLIDER_COUNT;i++) if (Slider[i].id==id) {
		Slider[i].max = max;
		Slider[i].min = min;
		Slider[i].style = style;
		UpdateSlider(id);
		return;
	}
	LogErr("Invalid Slider ID in AtmoControls");
}

// ==============================================================

void SetSlider(int id, WORD pos)
{
	if (!vObj) return;
	for (int i=0;i<ATM_SLIDER_COUNT;i++) if (Slider[i].id==id) {
		double x = (1000.0-double(pos))/1000.0;

		if (Slider[i].style & 8) x = x * x;
		if (Slider[i].style & 16) x = sqrt(x);
		if (Slider[i].style & 32) x = pow(x, 4.0);

		double v = Slider[i].min*(1.0-x) + Slider[i].max*x;
		
		if (Slider[i].style&2) {
			vObj->GetAtmoParams(1)->data[i] = v;
			vObj->GetAtmoParams(2)->data[i] = v;
			vObj->GetAtmoParams(3)->data[i] = v;
		}
		else {
			param->data[i] = v;
		}

		vObj->UpdateAtmoConfig(); 

		UpdateSlider(id, false);

		return;
	}
	LogErr("Invalid Slider ID in AtmoControls");
}

// ==============================================================

void UpdateSliders()
{
	for (int i=0;i<ATM_SLIDER_COUNT;i++) UpdateSlider(Slider[i].id);
}

// ==============================================================

void UpdateSlider(int id, bool bSetPos)
{
	char buf[32];

	if (!param) return;

	for (int i=0;i<ATM_SLIDER_COUNT;i++) if (Slider[i].id==id) {

		double val = param->data[i];
		
		SendDlgItemMessage(hDlg, id, TBM_SETRANGEMAX, 1, 1000);
		SendDlgItemMessage(hDlg, id, TBM_SETRANGEMIN, 1, 0);
		SendDlgItemMessage(hDlg, id, TBM_SETTICFREQ,  1, 0);

		if (bSetPos) {
			double x = (val - Slider[i].min)/(Slider[i].max-Slider[i].min);
			if (Slider[i].style & 8) x = sqrt(x);
			if (Slider[i].style & 16) x = x * x;
			if (Slider[i].style & 32) x = pow(x, 0.25);
			DWORD dpos = 1000 - DWORD(x*1000.0);
			SendDlgItemMessage(hDlg, id, TBM_SETPOS,  1, dpos);
		}

		if (Slider[i].style&1) sprintf_s(buf,"%.1lf k", val);
		else				   sprintf_s(buf,"%.3lf", val);
		
		SetWindowTextA(GetDlgItem(hDlg, Slider[i].dsp), buf);
		return;
	}
	LogErr("Invalid Slider ID in AtmoControls");
}

// ==============================================================

vPlanet * GetVisual()
{
	return vObj;
}

// ==============================================================

void SetVisual(vObject *vo)
{
	if (!vo) {
		vObj = NULL;
		param = &defs;
		return;
	}

	if (!hDlg || !dwCmd) return;
	
	OBJHANDLE hObj = vo->GetObjectA();

	if (oapiGetObjectType(hObj)!=OBJTP_PLANET) {
		LogErr("Invalid Object Type in AtmoControls");
		vObj = NULL;
		param = &defs;
		return;
	}

	vObj = static_cast<vPlanet *>(vo);

	if (vObj) param = vObj->GetAtmoParams(0);
	else	  param = &defs;

	UpdateSliders();
}


// ==============================================================
// Dialog message handler

INT_PTR CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static bool bOrbOld = false;

	switch (uMsg) {

	case WM_INITDIALOG:
	{
		vObject *vPl = NULL;
		OBJHANDLE hProxy = g_client->GetScene()->GetCameraProxyBody();
		if (hProxy) vPl = g_client->GetScene()->GetVisObject(hProxy);
		SetVisual(vPl);
		return true;
	}

	case WM_TIMER:
	{
		if (vObj) {	
			char title[256];
			sprintf_s(title, 256, "Atmospheric Controls [%s]", vObj->GetName());

			param = vObj->GetAtmoParams(atmmode);

			if (vObj->GetAtmoMode() == 1) sprintf_s(title, 256, "Atmospheric Controls [%s] [Surface]", vObj->GetName());
			if (vObj->GetAtmoMode() == 2) sprintf_s(title, 256, "Atmospheric Controls [%s] [LowOrbit]", vObj->GetName());
			if (vObj->GetAtmoMode() == 3) sprintf_s(title, 256, "Atmospheric Controls [%s] [HighOrbit]", vObj->GetName());
		
			SetWindowText(GetDlgItem(hWnd, IDC_ATM_COPYTO), "Copy to All");

			SetWindowTextA(hDlg, title);
			UpdateSliders();
		}
		
	}

	case WM_VSCROLL:
	{
		if (LOWORD(wParam)==TB_THUMBTRACK) {
			WORD pos = HIWORD(wParam);
			for (int i=0;i<ATM_SLIDER_COUNT;i++) if (Slider[i].hWnd==HWND(lParam)) {
				SetSlider(Slider[i].id, pos);
				return true;
			}
		}
		return false;
	}

	case WM_COMMAND:

		switch (LOWORD(wParam))
		{
			case IDCANCEL:  
			case IDOK:
				oapiCloseDialog(hWnd);
				hDlg = NULL;
				return TRUE;

			case IDC_ATM_LOAD:
				if (vObj) {
					vObj->LoadAtmoConfig();
					UpdateSliders();
				}
				break;

			case IDC_ATM_SAVE:
				if (vObj) vObj->SaveAtmoConfig();
				break;

			case IDC_ATM_COPYTO:
				if (vObj) {
					if (vObj->GetAtmoParams(1) != param) memcpy(vObj->GetAtmoParams(1), param, sizeof(ScatterParams));			
				}
				break;

			case IDC_ATM_COPYLOW:
				if (vObj) {
					if (vObj->GetAtmoParams(2) != param) memcpy(vObj->GetAtmoParams(2), param, sizeof(ScatterParams));
				}
				break;

			case IDC_ATM_COPYHIGH:
				if (vObj) {
					if (vObj->GetAtmoParams(3) != param) memcpy(vObj->GetAtmoParams(3), param, sizeof(ScatterParams));
				}
				break;

			case IDC_ATM_MODE:
				if (HIWORD(wParam)==CBN_SELCHANGE) {
					atmmode = DWORD(SendDlgItemMessage(hWnd, IDC_ATM_MODE, CB_GETCURSEL, 0, 0));
				}
				break;

			default: 
				//LogErr("LOWORD(%hu), HIWORD(0x%hX)",LOWORD(wParam),HIWORD(wParam));
				break;
		}
		break;
	}

	return oapiDefDialogProc(hWnd, uMsg, wParam, lParam);;
}

} //namespace


