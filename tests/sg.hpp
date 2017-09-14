#pragma once
/*
sg.h - public domain

Simple graphics library.
- Provides basic keyboard/mouse handling and a highres timer.
- Assumes 32 bit BGRX pixel format (0xRRGGBB in little-endian).

-- Documentation

#define SG_DEFINE    - define this in exactly one source file
#define SG_STATIC    - (optional) make all functions static

You must also define one of these
#define SG_W32       - use W32 implementation
#define SG_X11       - use X11 implementation (TODO)

sg_init(title, w, h) - initialize and open a window with dimension w*h
sg_exit()            - cleanup.
sg_poll()            - read an event, returns 0 when no more events.
sg_paint(buf, w, h)  - draw buf consisting of w*h 32 bit pixels onto window,
will stretch to fit window size.
sg_time()            - return time in seconds since sg_init().
sg_delay(s)          - sleep for at least s seconds.
*/

#ifdef SG_STATIC
#define SGDEF static
#else
#define SGDEF extern
#endif

enum
{
	/* event types */
	SG_ev_quit = 1, SG_ev_keydown, SG_ev_keyup, SG_ev_keychar, SG_ev_mouse,

	/* keycodes ('\b', '\t', '\r', ' ', '0'..'9', 'a'..'z') */
	SG_key_esc = 27, SG_key_shift = 128, SG_key_ctrl, SG_key_alt,
	SG_key_up, SG_key_down, SG_key_left, SG_key_right, SG_key_ins, SG_key_del,
	SG_key_home, SG_key_end, SG_key_pgup, SG_key_pgdn, SG_key_f1,
	SG_key_f2, SG_key_f3, SG_key_f4, SG_key_f5, SG_key_f6, SG_key_f7,
	SG_key_f8, SG_key_f9, SG_key_f10, SG_key_f11, SG_key_f12, SG_key_mb1,
	SG_key_mb2, SG_key_mb3, SG_key_mb4, SG_key_mb5
};

typedef struct
{
	int   type;
	int   key;
	float mx;
	float my;
} sg_event;

SGDEF void   sg_init(const char *title, int w, int h);
SGDEF void   sg_exit(void);
SGDEF int    sg_poll(sg_event *ev);
SGDEF void   sg_paint(const void *buf, int w, int h);
SGDEF double sg_time(void);
SGDEF void   sg_delay(double t);

#ifdef SG_DEFINE
#ifdef SG_W32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <intrin.h>
#include <mmsystem.h>

#ifdef _MSC_VER
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "winmm.lib")
#endif

#define SG_w32_wm_exit (WM_USER+1)

#define sg_w32_fatal(s) (MessageBoxA(0, s, "Fatal", 0), ExitProcess(1))
#define sg_w32_assert(p, s) ((p) ? (void)0 : sg_w32_fatal(s))

static struct
{
	CRITICAL_SECTION cs;
	HANDLE           th;
	DWORD            tid;
	DWORD            init;
	DWORD            winrdy;
	int              winsize;
	LARGE_INTEGER    tbase;
	double           tmulf;
	WNDCLASSA        wc;
	HWND             win;
	HDC              dc;
	DWORD            qhead;
	sg_event         qdata[256];
	DWORD            qtail;
} sg_w32;

/* single-producer/single-consumer queue, only uses a compiler barriers
because x86 has acquire/release semantics on regular loads and stores */
static int sg_w32_qwrite(const sg_event *ev)
{
	DWORD head = sg_w32.qhead, tail = sg_w32.qtail, ntail = (tail + 1) & 255;
	_ReadWriteBarrier();
	if( ntail == head )
		return 0;
	sg_w32.qdata[tail] = *ev;
	_ReadWriteBarrier();
	sg_w32.qtail = ntail;
	return 1;
}

static int sg_w32_qread(sg_event *ev)
{
	DWORD head = sg_w32.qhead, tail = sg_w32.qtail;
	_ReadWriteBarrier();
	if( head == tail )
		return 0;
	*ev = sg_w32.qdata[head];
	_ReadWriteBarrier();
	sg_w32.qhead = (head + 1) & 255;
	return 1;
}

static void sg_w32_adjsize(int *w, int *h)
{
	RECT r = { 0, 0, *w, *h };
	AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW, 0);
	*w = r.right - r.left;
	*h = r.bottom - r.top;
}

static const unsigned char sg_w32_vkmap[256] = {
	0, 0, 0, 0, 0, 0, 0, 0, '\b', '\t', 0, 0, 0, '\r', 0, 0,
	SG_key_shift, SG_key_ctrl, SG_key_alt, 0, 0, 0, 0, 0, 0, 0, 0,
	SG_key_esc, 0, 0, 0, 0, ' ', SG_key_pgup, SG_key_pgdn, SG_key_end,
	SG_key_home, SG_key_left, SG_key_up, SG_key_right, SG_key_down,
	0, 0, 0, 0, SG_key_ins, SG_key_del, 0, '0', '1', '2', '3', '4', '5',
	'6', '7', '8', '9', 0, 0, 0, 0, 0, 0, 0, 'a', 'b', 'c', 'd', 'e', 'f',
	'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
	'u', 'v', 'w', 'x', 'y', 'z', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, SG_key_f1, SG_key_f2, SG_key_f3, SG_key_f4, SG_key_f5,
	SG_key_f6, SG_key_f7, SG_key_f8, SG_key_f9, SG_key_f10, SG_key_f11,
	SG_key_f12
};

static LRESULT CALLBACK sg_w32_winproc(HWND hw, UINT msg, WPARAM wp, LPARAM lp)
{
	size_t w, h;
	sg_event ev = { 0 };

	switch( msg )
	{
	case WM_SYSKEYUP:
	case WM_SYSKEYDOWN:
		if( (lp&(1 << 29)) && !(lp&(1 << 31)) && wp == VK_F4 )
		{
			PostQuitMessage(0);
			return 0;
		}
	case WM_KEYUP:
	case WM_KEYDOWN:
		if( !(lp&(1 << 30)) == !!(lp&(1 << 31)) )
			return 0;
		ev.type = (lp&(1 << 31)) ? SG_ev_keyup : SG_ev_keydown;
		if( (ev.key = sg_w32_vkmap[wp & 255]) )
			goto post_event;
		return 0;
	case WM_CHAR:
		ev.type = SG_ev_keychar;
		ev.key = (int)wp;
		goto post_event;
	case WM_LBUTTONUP:   ev.key = SG_key_mb1; goto btn_up;
	case WM_MBUTTONUP:   ev.key = SG_key_mb2; goto btn_up;
	case WM_RBUTTONUP:   ev.key = SG_key_mb3; goto btn_up;
	case WM_LBUTTONDOWN: ev.key = SG_key_mb1; goto btn_down;
	case WM_MBUTTONDOWN: ev.key = SG_key_mb2; goto btn_down;
	case WM_RBUTTONDOWN: ev.key = SG_key_mb3; goto btn_down;
	case WM_MOUSEWHEEL:
		ev.key = ((int)wp >> 16) / WHEEL_DELTA > 0 ? SG_key_mb4 : SG_key_mb5;
		ev.type = SG_ev_keydown;
		sg_w32_qwrite(&ev);
		ev.type = SG_ev_keyup;
		sg_w32_qwrite(&ev);
		return 0;
	case WM_MOUSEMOVE:
		w = sg_w32.winsize & 0xffff;
		h = sg_w32.winsize >> 16;
		ev.type = SG_ev_mouse;
		ev.mx = (int)(short)LOWORD(lp) / (float)w;
		ev.my = (int)(short)HIWORD(lp) / (float)h;
		goto post_event;
	case WM_SIZE:
		_ReadWriteBarrier();
		sg_w32.winsize = (int)lp;
		return 0;
	case WM_ERASEBKGND:
		return 1;
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hw, msg, wp, lp);
btn_up:
	ev.type = SG_ev_keyup;
	goto post_event;
btn_down:
	ev.type = SG_ev_keydown;
post_event:
	sg_w32_qwrite(&ev);
	return 0;
}

static DWORD WINAPI sg_w32_winthrd(void *arg)
{
	char *title = (char *)((INT_PTR *)arg)[0];
	int w = (int)(((INT_PTR *)arg)[1] & 0xffff);
	int h = (int)(((INT_PTR *)arg)[1] >> 16);
	MSG msg;
	sg_event ev;

	sg_w32.wc.style = CS_VREDRAW | CS_HREDRAW | CS_OWNDC;
	sg_w32.wc.lpfnWndProc = sg_w32_winproc;
	sg_w32.wc.hInstance = GetModuleHandleW(0);
	sg_w32.wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	sg_w32.wc.hCursor = LoadCursor(0, IDC_ARROW);
	sg_w32.wc.lpszClassName = "sg";
	if( !RegisterClassA(&sg_w32.wc) )
		sg_w32_fatal("RegisterClass");

	sg_w32_adjsize(&w, &h);
	sg_w32.win = CreateWindowA("sg", title, WS_OVERLAPPEDWINDOW | WS_VISIBLE,
							   CW_USEDEFAULT, CW_USEDEFAULT, w, h, 0, 0,
							   sg_w32.wc.hInstance, 0);
	sg_w32_assert(sg_w32.win, "CreateWindow");
	sg_w32.dc = GetDC(sg_w32.win);
	sg_w32_assert(sg_w32.dc, "GetDC");

	_ReadWriteBarrier();
	sg_w32.winrdy = 1;

	while( GetMessage(&msg, 0, 0, 0) > 0 )
	{
		if( msg.message == SG_w32_wm_exit )
			break;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	ev.type = SG_ev_quit;
	sg_w32_qwrite(&ev);

	EnterCriticalSection(&sg_w32.cs);
	ReleaseDC(sg_w32.win, sg_w32.dc);
	DestroyWindow(sg_w32.win);
	UnregisterClassA("sg", sg_w32.wc.hInstance);
	sg_w32.win = 0;
	sg_w32.dc = 0;
	sg_w32.winrdy = 0;
	sg_w32.winsize = 0;
	LeaveCriticalSection(&sg_w32.cs);

	return 0;
}

SGDEF void sg_init(const char *title, int w, int h)
{
	INT_PTR arg[2] = { (INT_PTR)title, (h << 16) | w };
	LARGE_INTEGER freq;

	InitializeCriticalSection(&sg_w32.cs);
	sg_w32.th = CreateThread(0, 0, sg_w32_winthrd, arg, 0, &sg_w32.tid);
	sg_w32_assert(sg_w32.th, "CreateThread");
	timeBeginPeriod(1);
	QueryPerformanceFrequency(&freq);
	sg_w32.tmulf = 1.0 / (double)freq.QuadPart;
	while( !sg_w32.winrdy )
	{
		_ReadWriteBarrier();
		_mm_pause();
	}
	QueryPerformanceCounter(&sg_w32.tbase);
	sg_w32.init = 1;
}

SGDEF void sg_exit(void)
{
	if( !sg_w32.init )
		return;
	PostThreadMessage(sg_w32.tid, SG_w32_wm_exit, 0, 0);
	WaitForSingleObject(sg_w32.th, INFINITE);
	CloseHandle(sg_w32.th);
	DeleteCriticalSection(&sg_w32.cs);
	sg_w32.qhead = sg_w32.qtail = 0;
	sg_w32.init = 0;
}

SGDEF int sg_poll(sg_event *ev)
{
	if( !sg_w32.init )
		return 0;
	return sg_w32_qread(ev);
}

SGDEF void sg_paint(const void *buf, int w, int h)
{
	static BITMAPINFO bmi = { sizeof(BITMAPINFOHEADER), 0, 0, 1, 32, BI_RGB };
	int dw = sg_w32.winsize, dh = dw >> 16;
	_ReadWriteBarrier();
	dw &= 0xffff;
	bmi.bmiHeader.biWidth = w;
	bmi.bmiHeader.biHeight = -h;
	if( sg_w32.init && TryEnterCriticalSection(&sg_w32.cs) )
	{
		StretchDIBits(sg_w32.dc, 0, 0, dw, dh, 0, 0, w, h, buf, &bmi,
					  DIB_RGB_COLORS, SRCCOPY);
		ValidateRect(sg_w32.win, 0);
		LeaveCriticalSection(&sg_w32.cs);
	}
}

SGDEF double sg_time(void)
{
	LARGE_INTEGER t;
	QueryPerformanceCounter(&t);
	return (double)(t.QuadPart - sg_w32.tbase.QuadPart) * sg_w32.tmulf;
}

SGDEF void sg_delay(double t)
{
	if( t > 0.0 )
		Sleep((DWORD)(t*1000.0));
}

#else
#error must define SG_W32
#endif
#endif