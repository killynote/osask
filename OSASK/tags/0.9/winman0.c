// "winman0.c":ぐいぐい仕様ウィンドウマネージャー ver.0.5
//		copyright(C) 2000-2001 川合秀実
//  exe2bin0 winman0 -s 24k

#include <guigui00.h>
#include <sysgg00.h>
// sysggは、一般のアプリが利用してはいけないライブラリ
// 仕様もかなり流動的
#include <stdlib.h>

#define	AUTO_MALLOC			  0
#define NULL				  0
#define	MAX_WINDOWS			 16		// 16KB
#define JOBLIST_SIZE		256		// 1KB
#define	MAX_SOUNDTRACK		 16		// 0.5KB

struct DEFINESIGNAL {
	int opt, res, dev, cod, len, sig[3];
};

struct WM0_WINDOW {	// total 1KB
	struct DEFINESIGNAL defsig[29]; // 928bytes
	struct SGG_WINDOW sgg; // 68bytes
	struct DEFINESIGNAL *ds1;
	int condition, x0, y0, x1, y1, job_flag0, job_flag1;
	struct WM0_WINDOW *up, *down;
};

struct SOUNDTRACK {
	int sigbox, sigbase, slot, reserve[5];
};

struct WM0_WINDOW *window, *top = NULL, *unuse = NULL, *iactive = NULL, *pokon0 = NULL;
int *joblist, jobfree, *jobrp, *jobwp, jobnow = 0;
void (*jobfunc)(int, int);
int x2 = 0, y2, fromboot = 0, mx = 0x80000000, my, mbutton = 0;
struct SOUNDTRACK *sndtrk_buf, *sndtrk_active = NULL;

void lib_drawline(const int opt, const int reserve, const int color,
	const int x0, const int y0, const int x1, const int y1);
void init_screen(const int x, const int y);
struct WM0_WINDOW *handle2window(const int handle);
void chain_unuse(struct WM0_WINDOW *win);
struct WM0_WINDOW *get_unuse();
void mousesignal(const int header, const int dx, const int dy);
void writejob(int i);
void runjobnext();
void job_openwin0(struct WM0_WINDOW *win);
void redirect_input(struct WM0_WINDOW *win);
void job_activewin0(struct WM0_WINDOW *win);
void job_movewin0(struct WM0_WINDOW *win);
void job_movewin1(const int cmd, const int handle);
void job_movewin2();
void job_movewin3();
void job_movewin4(int sig);
void job_closewin0(struct WM0_WINDOW *win0);
void job_general0(const int cmd, const int handle);
void job_general1();
void job_general2(const int cmd, const int handle);
void job_openvgadriver(const int drv);
void job_setvgamode0(const int mode);
void job_setvgamode1(const int cmd, const int handle);
void job_setvgamode2();
void job_setvgamode3(const int sig, const int result);
void free_sndtrk(struct SOUNDTRACK *sndtrk);
struct SOUNDTRACK *alloc_sndtrk();
void send_signal2dw(const int sigbox, const int data0, const int data1);
void send_signal3dw(const int sigbox, const int data0, const int data1, const int data2);

// キー操作：
//    F9:一番下のウィンドウへ
//    F10:上から２番目のウィンドウを選択
//    F11:ウィンドウの移動
//    F12:ウィンドウクローズ

void main()
{
	int *signal, *signal0, i, j;
	struct WM0_WINDOW *win;

	lib_init(AUTO_MALLOC);
        sgg_init(AUTO_MALLOC);

	signal = signal0 = lib_opensignalbox(256 * 4, AUTO_MALLOC, 0, 4); // 1KB

	// ウィンドウを確保して、全て未使用ウインドウとして登録
	window = (struct WM0_WINDOW *) malloc(MAX_WINDOWS * sizeof (struct WM0_WINDOW));
	for (i = 0; i < MAX_WINDOWS; i++)
		chain_unuse(&window[i]);

	// サウンドトラック用バッファの初期化
	sndtrk_buf = (struct SOUNDTRACK *) malloc(MAX_SOUNDTRACK * sizeof (struct SOUNDTRACK));
	for (i = 0; i < MAX_SOUNDTRACK; i++)
		free_sndtrk(&sndtrk_buf[i]);

	joblist = (int *) malloc(JOBLIST_SIZE * sizeof (int));
	*(jobrp = jobwp = joblist) = 0; // たまった仕事はない
	jobfree = JOBLIST_SIZE - 1;

	for (;;) {
		switch (signal[0]) {
		case 0x0000:
			lib_waitsignal(0x0001, 0, 0);
			break;

		case 0x0004 /* rewind */:
			lib_waitsignal(0x0000, signal[1], 0);
			signal = signal0;
			break;

		case 0x0010:
			// 初期化要請
			signal++;
			lib_waitsignal(0x0000, 1, 0);

			// ジョブリストにこの要求を入れる
			if (jobfree >= 4) {
				// 空きが十分にある
				writejob(0x0030 /* open VGA driver */);
				writejob(0x0000);
				writejob(0x0034 /* change VGA mode */);
				writejob(0x0012);
				*jobwp = 0; // ストッパー
				if (jobnow == 0)
					runjobnext();
			}
			break;

		case 0x0018:
			// from boot
			fromboot = signal[1];
			signal += 2;
			lib_waitsignal(0x0000, 2, 0);
			break;

		case 0x001c:
			// 終了要請
			goto mikannsei;
			break;

		case 0x0020:
			// ウィンドウオープン要請(handle)
			win = get_unuse();
			sgg_wm0_openwindow(&win->sgg, signal[1]);
			signal += 2;
			lib_waitsignal(0x0000, 2, 0);
			win->ds1 = win->defsig;

			// ジョブリストにこの要求を入れる
			if (jobfree >= 2) {
				// 空きが十分にある
				writejob(0x0020 /* open */);
				writejob((int) win);
				*jobwp = 0; // ストッパー
				if (jobnow == 0)
					runjobnext();
			}
			break;

		case 0x0024:
			// ウィンドウクローズ要請(handle)
			win = handle2window(signal[1]);
			signal += 2;
			lib_waitsignal(0x0000, 2, 0);
			// ジョブリストにこの要求を入れる
			if (jobfree >= 2) {
				// 空きが十分にある
				writejob(0x002c /* close */);
				writejob((int) win);
				*jobwp = 0; // ストッパー
				if (jobnow == 0)
					runjobnext();
			}
			break;

		case 0x0028:
			// ウィンドウアクティブ要請(opt, handle)
			goto mikannsei;
			break;

		case 0x002c:
			// ウィンドウ連動デバイス指定
			//  (opt,  win-handle, reserve(signalebox),
			//     default-device, default-code, len(2), 0x7f000001, signal)

			win = handle2window(signal[2]);
			if (win->ds1 < win->defsig
				+ sizeof (win->defsig) / sizeof (struct DEFINESIGNAL)) {
				struct DEFINESIGNAL *dsp = win->ds1;
				dsp->opt = signal[1];
				dsp->dev = signal[4];
				dsp->cod = signal[5];
				dsp->len = 2;
				dsp->sig[0] = signal[7];
				dsp->sig[1] = signal[8];
				if (iactive == win) {
					int sigbox = sgg_wm0_win2sbox(&win->sgg);
					sgg_wm0_definesignal2(dsp->opt, dsp->dev,
						dsp->cod, sigbox, dsp->sig[0], dsp->sig[1]);
				}
				win->ds1++;
			}
			signal += 9;
			lib_waitsignal(0x0000, 9, 0);
			break;

		case 0x0040: // open sound track (slot, tss, signal-base, reserve0, reserve1)
			// 受理したことを知らせるために、シグナルで応答する
			{
				struct SOUNDTRACK *sndtrk = alloc_sndtrk();
				sndtrk->sigbox  = signal[2 /* tss */] + 0x0240;
				sndtrk->slot    = signal[1 /* slot */];
				sndtrk->sigbase = signal[3 /* signal-base */];
				signal += 6;
				lib_waitsignal(0x0000, 6, 0);
				// ハンドル番号の対応づけを通達
				send_signal3dw(sndtrk->sigbox, sndtrk->sigbase + 0, sndtrk->slot, (int) sndtrk);
				if (sndtrk_active == NULL) {
					sndtrk_active = sndtrk;
					// アクティブシグナルを送る
					send_signal2dw(sndtrk->sigbox, sndtrk->sigbase + 8 /* enable */, sndtrk->slot);
				}
			}
			break;

		case 0x0044: // close sound track (handle)
			{
				struct SOUNDTRACK *sndtrk = (struct SOUNDTRACK *) signal[1];
				free_sndtrk(sndtrk);
				signal += 2;
				lib_waitsignal(0x0000, 2, 0);
				if (sndtrk == sndtrk_active) {
					// 違うやつをアクティブにする
					sndtrk = NULL;
					for (i = 0; i < MAX_SOUNDTRACK; i++) {
						if (sndtrk_buf[i].sigbox) {
							sndtrk = &sndtrk_buf[i];
							break;
						}
					}
					if (sndtrk_active = sndtrk) {
						// アクティブシグナルを送る
						send_signal2dw(sndtrk->sigbox, sndtrk->sigbase + 8 /* enable */, sndtrk->slot);
					}
				}
			}
			break;

		case 0x0014: // 画面モード変更完了(result)
		case 0x00c0: // 更新停止シグナル(handle)
		case 0x00c4: // 描画完了シグナル(handle)
			i = signal[0];
			j = signal[1];
			signal += 2;
			lib_waitsignal(0x0000, 2, 0);
			(*jobfunc)(i, j);
			if (jobnow == 0)
				runjobnext();
			break;

		case 0x00d0:
		case 0x00d1:
		case 0x00d2:
		case 0x00d3:
		case 0x00f0:
			i = signal[0];
			signal++;
			lib_waitsignal(0x0000, 1, 0);
			job_movewin4(i);
			break;


		case 0x0200 /* active bottom window */:
			signal++;
			lib_waitsignal(0x0000, 1, 0);
			// ジョブリストにこの要求を入れる
			if (jobfree >= 2) {
				// 空きが十分にある
				writejob(0x0024 /* active */);
				writejob((int) top->up);
				*jobwp = 0; // ストッパー
				if (jobnow == 0)
					runjobnext();
			}
			break;

		case 0x0201 /* active second window */:
			signal++;
			lib_waitsignal(0x0000, 1, 0);
			// ジョブリストにこの要求を入れる
			if (jobfree >= 2) {
				// 空きが十分にある
				writejob(0x0024 /* active */);
				writejob((int) top->down);
				*jobwp = 0; // ストッパー
				if (jobnow == 0)
					runjobnext();
			}
			break;

		case 0x0202 /* move window */:
			signal++;
			lib_waitsignal(0x0000, 1, 0);
			// ジョブリストにこの要求を入れる
			if (jobfree >= 2) {
				// 空きが十分にある
				writejob(0x0028 /* move */);
				writejob((int) top);
				*jobwp = 0; // ストッパー
				if (jobnow == 0)
					runjobnext();
			}
			break;

		case 0x0203 /* close window */:
			signal++;
			lib_waitsignal(0x0000, 1, 0);
			if (top != pokon0)
				sgg_wm0s_close(&top->sgg);
			break;

		case 0x0204 /* VGA mode 0x0012 */:
			signal++;
			lib_waitsignal(0x0000, 1, 0);

			// ジョブリストにこの要求を入れる
			if (jobfree >= 2) {
				// 空きが十分にある
				writejob(0x0034 /* change VGA mode */);
				writejob(0x0012);
				*jobwp = 0; // ストッパー
				if (jobnow == 0)
					runjobnext();
			}
			break;

		case 0x0205 /* VESA mode 0x0102 */:
			signal++;
			lib_waitsignal(0x0000, 1, 0);

			// ジョブリストにこの要求を入れる
			if (jobfree >= 2) {
				// 空きが十分にある
				writejob(0x0034 /* change VGA mode */);
				writejob(0x0102);
				*jobwp = 0; // ストッパー
				if (jobnow == 0)
					runjobnext();
			}
			break;

		case 0x73756f6d /* from mouse */:
			mousesignal(signal[1], (signed short) (signal[2] & 0xffff), (signed short) (signal[2] >> 16));
			signal += 3;
			lib_waitsignal(0x0000, 3, 0);
			break;

		default:
		mikannsei:
			lib_drawline(0x0020, 0, 0, 0, 0, 15, 15); // ここに来たことを知らせる
			signal++;
			lib_waitsignal(0x0000, 1, 0);
		}
	}
}

void lib_drawline(const int opt, const int reserve, const int color,
	const int x0, const int y0, const int x1, const int y1)
{
	static struct {
		int cmd, opt;
		int reserve, color;
		int x0, y0, x1, y1;
		int eoc;
	} command = { 0x0044, 0, 0, 0, 0, 0, 0, 0, 0x0000 };

	command.opt = opt;
	command.color = color;
	command.x0 = x0;
	command.y0 = y0;
 	command.x1 = x1;
	command.y1 = y1;

	lib_execcmd(&command);
	return;
}

void init_screen(const int x, const int y)
{
	lib_drawline(0x0020, 0,  6,      0,      0, x -  1, y - 29);
	lib_drawline(0x0020, 0,  8,      0, y - 28, x -  1, y - 28);
	lib_drawline(0x0020, 0, 15,      0, y - 27, x -  1, y - 27);
	lib_drawline(0x0020, 0,  8,      0, y - 26, x -  1, y -  1);
	lib_drawline(0x0020, 0, 15,      3, y - 24,     59, y - 24);
	lib_drawline(0x0020, 0, 15,      2, y - 24,      2, y -  4);
	lib_drawline(0x0020, 0,  7,      3, y -  4,     59, y -  4);
	lib_drawline(0x0020, 0,  7,     59, y - 23,     59, y -  5);
	lib_drawline(0x0020, 0,  0,      2, y -  3,     59, y -  3);
	lib_drawline(0x0020, 0,  0,     60, y - 24,     60, y -  3);
	lib_drawline(0x0020, 0,  7, x - 47, y - 24, x -  4, y - 24);
	lib_drawline(0x0020, 0,  7, x - 47, y - 23, x - 47, y -  4);
	lib_drawline(0x0020, 0, 15, x - 47, y -  3, x -  4, y -  3);
	lib_drawline(0x0020, 0, 15, x -  3, y - 24, x -  3, y -  3);
	sgg_wm0_putmouse(mx, my);
	return;
}

struct WM0_WINDOW *handle2window(const int handle)
{
	// topの中から探してもいい
	int i;
	for (i = 0; i < MAX_WINDOWS; i++) {
		if (window[i].sgg.handle == handle)
			return &(window[i]);
	}
	return NULL;
}

void chain_unuse(struct WM0_WINDOW *win)
{
	// unuseは一番上
	// winは一番下に追加
	win->sgg.handle = 0; // handle2windowが間違って検出しないために
	if (unuse) {
		struct WM0_WINDOW *bottom;
		bottom = unuse->up;
		win->down = unuse;
		win->up = bottom;
		unuse->up = win;
		bottom->down = win;
	} else {
		unuse = win;
		win->up = win;
		win->down = win;
	}
	return;
}

struct WM0_WINDOW *get_unuse()
{
	// 一番上からとってくる
	struct WM0_WINDOW *win = unuse;
	struct WM0_WINDOW *bottom = unuse->up;
	unuse = unuse->down;
	unuse->up = bottom;
	bottom->down = unuse;
	if (win == unuse)
		unuse = NULL;
	return win;
}

void mousesignal(const int header, const int dx, const int dy)
{
	if ((header >> 28) == 0x0 /* normal mode */) {
		// マウス状態変更
		int ox = mx, oy = my;
		mx += dx;
		my += dy;
		if (mx < 0)
			mx = 0;
		if (mx >= x2)
			mx = x2 - 1;
		if (my < 0)
			my = 0;
		if (my >= y2 - 15)
			my = y2 - 16;
		if (mx != ox || my != oy)
			sgg_wm0_movemouse(mx, my);
		if (mbutton != (header & 0x07)) {
			// マウスのボタン状態が変化
			// bit0:left
			// bit1:right
			// bit2:middle
			// bit3:always 1
			// bit4:reserve(dx bit8)
			// bit5:reserve(dy bit8)
			if ((mbutton & 0x01) == 0x00 && (header & 0x01) == 0x01) {
				// 左ボタンが押された

				struct WM0_WINDOW *win;

				// どのwindowをクリックしたのかを検出
				if (win = top) {
					do {
						if (win->x0 <= mx && mx < win->x1 && win->y0 <= my && my < win->y1)
							goto found_window;
						win = win->down;
					} while (win != top);
					win = NULL;
				}
				if (win != NULL) {
		found_window:
					if (win != top) {
						// ジョブリストにウィンドウアクティブ要求を入れる
						if (jobfree >= 2) {
							// 空きが十分にある
							writejob(0x0024 /* active */);
							writejob((int) win);
							*jobwp = 0; // ストッパー
						}
					}
				}
			}
			mbutton = header & 0x07;
		}
	} else if ((header >> 28) == 0xa /* extmode byte2 */) {
		// マウスリセット
		mbutton = 0;
		sgg_wm0_enablemouse();
	} else {
		// mikannsei
	}
	// 溜まったジョブがあれば、実行する
	if (jobnow == 0)
		runjobnext();
	return;
}

void writejob(int i)
{
	*jobwp++ = i;
	jobfree--;
	if (jobwp == joblist + JOBLIST_SIZE)
		jobwp = joblist;
	return;
}

const int readjob()
{
	int i = *jobrp++;
	jobfree++;
	if (jobrp == joblist + JOBLIST_SIZE)
		jobrp = joblist;
	return i;
}

void runjobnext()
{
	do {
		if ((jobnow = *jobrp) == 0)
			return;

		readjob(); // から読み
		switch (jobnow) {

		case 0x0020 /* open new window */:
			job_openwin0((struct WM0_WINDOW *) readjob());
			break;

		case 0x0024 /* active window */:
			job_activewin0((struct WM0_WINDOW *) readjob());
			break;

		case 0x0028 /* move window */:
			job_movewin0((struct WM0_WINDOW *) readjob());
			break;

		case 0x002c /* close window */:
			job_closewin0((struct WM0_WINDOW *) readjob());
			break;

		case 0x0030 /* open VGA driver */:
			job_openvgadriver(readjob());
			break;

		case 0x0034 /* set VGA mode */:
			job_setvgamode0(readjob());

		}
	} while (jobnow == 0);
	return;
}

struct WM0_WINDOW *job_win;
int job_count, job_int0;

const int overrapwin(const struct WM0_WINDOW *win0, const struct WM0_WINDOW *win1)
{
	return win0->x0 < win1->x1 && win1->x0 < win0->x1 && win0->y0 < win1->y1 && win1->y0 < win0->y1;
}

void job_openwin0(struct WM0_WINDOW *win)
{
	int xsize = sgg_wm0_winsizex(&win->sgg);
	int ysize = sgg_wm0_winsizey(&win->sgg);

	// まず、座標を決める
	if (top == NULL) {
		// for pokon0
		win->x0 = 16;
		win->y0 = 32;
		pokon0 = win;
	} else {
		win->x0 = (x2 - xsize) / 2;
		win->x0 &= ~0x07; // オープン位置を8の倍数になるように調整
		win->y0 = (y2 - ysize) / 2;
	}

	// 各種パラメーターの初期化
	win->x1 = win->x0 + xsize;
	win->y1 = win->y0 + ysize;
	win->job_flag0 = 0x81000000; // override-disable & must-redraw

	// 入力アクティブを変更
	redirect_input(win); // この関数は、ウィンドウ制御はしない
	iactive = win;

	// 接続
	if (top == NULL)
		win->up = win->down = top = win;
	else {
		struct WM0_WINDOW *bottom;
		win->up = bottom = top->up;
		win->down = top;
		top->up = win;
		bottom->down = win;
		top = win;
	}

	sgg_wm0s_movewindow(&win->sgg, win->x0, win->y0);
	sgg_wm0s_setstatus(&win->sgg, win->condition = 0x03 /* bit0:表示, bit1:入力 */);
	job_general1();
	return;
}

void redirect_input(struct WM0_WINDOW *win)
{
	// キー入力シグナルの対応づけを初期化
	sgg_wm0_definesignal0(255, 0x0100, 0);

	// winman0のキー操作を登録(F9〜F12)
	sgg_wm0_definesignal2(3, 0x0100, 0x0089 /* F9 */,
		0x3240 /* winman0 signalbox */, 0x7f000001, 0x0200);
	sgg_wm0_definesignal2(1, 0x0100, 0x0081 /* F1 */,
		0x3240 /* winman0 signalbox */, 0x7f000001, 0x0204);

	if (win) {
		struct DEFINESIGNAL *dsp;
		int sigbox = sgg_wm0_win2sbox(&win->sgg);
		for (dsp = win->defsig; dsp < win->ds1; dsp++) {
			if (dsp->len == 2)
				sgg_wm0_definesignal2(dsp->opt, dsp->dev,
					dsp->cod, sigbox, dsp->sig[0], dsp->sig[1]);
		}
	}
	return;
}

void job_activewin0(struct WM0_WINDOW *win)
{
	int x0, y0, x1, y1;
	struct WM0_WINDOW *win_up, *win_down;

	if (top == win) {
		// topとiactiveは常に等しい
		jobnow = 0;
		return;
	}

	// winをリストから一度切り離す
	win_up = win->up;
	win_down = win->down;
	win_up->down = win_down;
	win_down->up = win_up;

	// 再接続
	win->up = win_down = top->up;
	win->down = top;
	top->up = win;
	win_down->down = win;
	top = win;

	// 入力アクティブを変更
	redirect_input(win); // この関数は、ウィンドウ制御はしない
	iactive = win;

	job_general1();
	return;
}

void job_movewin0(struct WM0_WINDOW *win)
{
	// winは、必ずtopで、かつ、iactiveであること

	job_win = top /* win */;

	// キー入力シグナルの対応づけを初期化
	sgg_wm0_definesignal0(255, 0x0100, 0);

	// とりあえず、全てのウィンドウの画面更新権を一時的に剥奪する
	job_count = 0;
	win = top;
	jobfunc = &job_movewin1;
	do {
		win->job_flag0 = 0x01;
		if (win->condition & 0x01) {
			job_count++; // disable
			sgg_wm0s_accessdisable(&win->sgg);
		}
		win = win->down;
	} while (win != top);

//	以下は成立しない(最低一つは出力アクティブなウィンドウが存在するから)
//	if (job_count == 0) {
//		job_movewin2(); // すぐにウィンドウ枠表示へ
//	}

	return;
}

void job_movewin1(const int cmd, const int handle)
{
	// 0x00c0しかこない
	if (--job_count == 0)
		job_movewin2();
	return;
}

int job_movewin_x, job_movewin_y;

void job_movewin2()
{
	// シグナルを宣言(0x00d0〜0x00d3, 0x00f0)
	sgg_wm0_definesignal2(3, 0x0100, 0x00ac /* left */,
		0x3240 /* winman0 signalbox */, 0x7f000001, 0x00d0);
	sgg_wm0_definesignal2(0, 0x0100, 0x00a0 /* Enter */,
		0x3240 /* winman0 signalbox */, 0x7f000001, 0x00f0);

	// 枠を描く
	job_movewin_x = job_win->x0;
	job_movewin_y = job_win->y0;
	job_movewin3();
	return;
}

void job_movewin3()
{
	int x0, y0, x1, y1;
	struct WM0_WINDOW *win = job_win;

	x0 = job_movewin_x;
	y0 = job_movewin_y;
	x1 = x0 + win->x1 - win->x0 - 1;
	y1 = y0 + win->y1 - win->y0 - 1;

	lib_drawline(0x00e0, 0, 9, x0,     y0,     x1 - 3, y0 + 2);
	lib_drawline(0x00e0, 0, 9, x0 + 3, y1 - 2, x1,     y1    );
	lib_drawline(0x00e0, 0, 9, x0,     y0 + 3, x0 + 2, y1    );
	lib_drawline(0x00e0, 0, 9, x1 - 2, y0,     x1,     y1 - 3);
	return;
}

void job_movewin4(int sig)
{
	struct WM0_WINDOW *win0 = job_win;

	int x0 = job_movewin_x;
	int y0 = job_movewin_y;
	int xsize = win0->x1 - win0->x0;
	int ysize = win0->y1 - win0->y0;

	if (sig == 0x00d0 && x0 >= 8)
		x0 -= 8;
	if (sig == 0x00d1 && x0 + xsize <= x2 - 8)
		x0 += 8;
	if (sig == 0x00d2 && y0 >= 8)
		y0 -= 8;
	if (sig == 0x00d3 && y0 + ysize <= y2 - 36)
		y0 += 8;
	if ((x0 - job_movewin_x) | (y0 - job_movewin_y)) {
		job_movewin3();
		job_movewin_x = x0;
		job_movewin_y = y0;
		job_movewin3();
	}

	if (sig == 0x00f0) {
		job_movewin3();
		struct WM0_WINDOW *win1;
		win0->job_flag0 = 0x81000000; // override-disable & must-redraw
		win1 = win0 /* top */->down;
		do {
			int flag0 = 0x01000000; // override-disable
			if (overrapwin(win0, win1))
				flag0 = 0x81000000; // override-disable & must-redraw
			win1->job_flag0 = flag0;
		} while ((win1 = win1->down) != win0);

		// ウィンドウを消す
		lib_drawline(0x0020, 0, 6, win0->x0, win0->y0, win0->x1 - 1, win0->y1 - 1);

		win0->x0 = x0;
		win0->y0 = y0;
		win0->x1 = x0 + xsize;
		win0->y1 = y0 + ysize;
		sgg_wm0s_movewindow(&win0->sgg, win0->x0, win0->y0);
		redirect_input(win0);
		job_general1();
	}
	return;
}

void job_closewin0(struct WM0_WINDOW *win0)
// このウィンドウは既にaccessdisableになっていることが前提
{
	struct WM0_WINDOW *win1, *win_up, *win_down;

	// winをリストから切り離す
	win_up = win0->up;
	win_down = win0->down;
	win_up->down = win_down;
	win_down->up = win_up;	

	job_count = 1;

	if (win0 == top) {
		top = win_down;
		if (win_up == win0) {
			top = NULL;
			redirect_input(0);
			iactive = NULL;
			goto no_window;
		}
		redirect_input(top);
		iactive = top;
	}

	win1 = top;
	do {
		int flag0 = 0;
		if (overrapwin(win0, win1)) {
			flag0 = 0x81000000;
			job_count++;
			sgg_wm0s_accessdisable(&win1->sgg);
		}
		win1->job_flag0 = flag0;
	} while ((win1 = win1->down) != top);

no_window:

	// ウィンドウを消す
	lib_drawline(0x0020, 0, 6, win0->x0, win0->y0, win0->x1 - 1, win0->y1 - 1);
	chain_unuse(win0);
	job_general0(0, 0);
	return;
}

// ウィンドウ消去の場合、消したいウィンドウと重なっているものを全て"must-redraw"にしたあと
// ウィンドウを消し、あとは自動に任せる

// ウィンドウオープンの場合、追加して、後は自動に任せる

// ウィンドウ移動の場合、移動元と重なっているものを全て"must-redraw"にしたあと
// もといた場所を消し、あとは自動に任せる

void job_general0(const int cmd, const int handle)
{
	jobfunc = &job_general0;
	if (--job_count)
		return;
	job_general1();
	return;
}

void job_general1()
// condition.bit 0 ... 0:accessdisable 1:accessenable
// condition.bit 1 ... 0:inputdisable(not active) 1:inputenable(active)

// job_flag0.bit 0 ... new condition.bit 0(auto-set)
// job_flag0.bit 1 ... new condition.bit 1(auto-set)
// job_flag0.bit24 ... 0:normal 1:override-accessdisabled
// job_flag0.bit31 ... 0:normal 1:must-redraw
{
	struct WM0_WINDOW *win0, *win1, *bottom, *top_ = top /* 高速化、コンパクト化のため */;
	int flag0;

	if (top_ == NULL) {
		jobnow = 0;
	//	jobfunc = NULL;
		return;
	}

	// accessenable & not input active
	win0 = top_;
	do {
		flag0 = win0->job_flag0;
		flag0 |=  0x01; // accessenable
		flag0 &= ~0x02; // not-input-active
		if ((win0->condition & 0x01) == 0)
			flag0 |= 0x01000000; // override-disable
		win0->job_flag0 = flag0;
	} while ((win0 = win0->down) != top_);

	// 上から見ていって、重なっているものはaccessdisable
	win0 = top_;
	do {
		for (win1 = win0->down; win1 != top_; win1 = win1->down) {
			if (win1->job_flag0 & 0x01) { // このif文がなくても実行結果は変わらないが、高速化のため
				if (overrapwin(win0, win1))
					win1->job_flag0 &= ~0x01; // accessdisable
			}
		}
	} while ((win0 = win0->down) != top_);

	// topはjob_flag0.bit 1 = 1;
	top_->job_flag0 |= 0x02; // input-active

	// 下から見ていって、conditionが変化していたらjob_flag0.bit31 = 1;
	//   job_flag0.bit31 == 1なら自分に重なっている上のやつ全てもjob_flag0.bit31 = 1;
	win0 = bottom = top_->up; // 一番上の上は、一番下
	do {
		flag0 = win0->job_flag0;
		if (win0->condition != (flag0 & 0x03) || (flag0 & 0x01000000) != 0)
			win0->job_flag0 = (flag0 |= 0x80000000);
		if (flag0 & 0x80000000) {
			for (win1 = win0->up; win1 != bottom; win1 = win1->up) {
				if ((win1->job_flag0 & 0x80000000) == 0) { // このif文がなくても実行結果は変わらないが、高速化のため
					if (overrapwin(win0, win1))
						win1->job_flag0 |= 0x80000000; // must-redraw
				}
			}
		}
	} while ((win0 = win0->down) != bottom);

	// redrawを予定しているウィンドウで、他のredraw予定ウィンドウとオーバーラップしているものは、
	// 全てoverride-accessdisableにする
	job_count = 1;
	win0 = top_;
	do {
		if ((win0->job_flag0 & 0x81000000) == 0x80000000) {
			for (win1 = win0->down; win1 != win0; win1 = win1->down) {
				if (win1->job_flag0 & 0x80000000) {
					if (overrapwin(win0, win1)) {
						sgg_wm0s_accessdisable(&win0->sgg);
						win0->job_flag0 |= 0x01000000;
						job_count++;
						if ((win1->job_flag0 & 0x81000000) == 0x80000000) {
							sgg_wm0s_accessdisable(&win1->sgg);
							win1->job_flag0 |= 0x01000000;
							job_count++;
						}
					}
				}
			}
		}
	} while ((win0 = win0->down) != top_);
	job_win = top_;
	job_general2(0, 0);
	return;
}

void job_general2(const int cmd, const int handle)
{
	struct WM0_WINDOW *win = job_win;
	int flag0;

	jobfunc = &job_general2;

	if (--job_count)
		return;

	if (win == top && win->job_flag0 == 0) {
		// １周目でwin->job_flag0が0になることはない
		// 全ての作業が完了
		jobnow = 0;
	//	jobfunc = NULL;
		return;
	}

	do {
		win = win->up;
		flag0 = win->job_flag0;
		win->job_flag0 = 0;
		if ((flag0 & 0x01000001) == 0x01000001 && x2 != 0)
			sgg_wm0s_accessenable(&win->sgg);
	} while ((flag0 & 0x80000000) == 0);

	job_win = win;
	if ((flag0 & 0x01000001) == 0x00000000) {
		sgg_wm0s_accessdisable(&win->sgg);
		job_count = 1;
	}
	if ((flag0 & 0x03) != win->condition)
		sgg_wm0s_setstatus(&win->sgg, win->condition = (flag0 & 0x03));
	sgg_wm0s_redraw(&win->sgg);
	job_count++;
	return;
}

void free_sndtrk(struct SOUNDTRACK *sndtrk)
{
	sndtrk->sigbox = 0;
	return;
}

struct SOUNDTRACK *alloc_sndtrk()
{
	int i;
	for (i = 0; i < MAX_SOUNDTRACK; i++) {
		if (sndtrk_buf[i].sigbox == 0)
			return &sndtrk_buf[i];
	}
	return NULL;
}

void send_signal2dw(const int sigbox, const int data0, const int data1)
{
	static struct {
		int cmd, opt;
		int data[3];
		int eoc;
	} command = { 0x0020, 0x80000000 + 3, { 0 }, 0x0000 };

	command.data[0] = sigbox | 2;
	command.data[1] = data0;
	command.data[2] = data1;

	sgg_execcmd(&command);
	return;
}

void send_signal3dw(const int sigbox, const int data0, const int data1, const int data2)
{
	static struct {
		int cmd, opt;
		int data[4];
		int eoc;
	} command = { 0x0020, 0x80000000 + 4, { 0 }, 0x0000 };

	command.data[0] = sigbox | 3;
	command.data[1] = data0;
	command.data[2] = data1;
	command.data[3] = data2;

	sgg_execcmd(&command);
	return;
}

void job_openvgadriver(const int drv)
{
	// closeする時にすべてのウィンドウはdisableになっているはずなので、
	// ここではdisableにはしない
	sgg_wm0_gapicmd_0010_0000();
	jobnow = 0;
	return;
}

void job_setvgamode0(const int mode)
{
	struct WM0_WINDOW *win;
	job_int0 = mode;

	if (mx != 0x80000000) {
		sgg_wm0_removemouse();
	} else
		mx = my = 1;

	// とりあえず、全てのウィンドウの画面更新権を一時的に剥奪する
	job_count = 0;
	jobfunc = &job_setvgamode1;
	if (win = top) {
		do {
			win->job_flag0 = 0x81000000; // override-accessdisabled & must-redraw
			if ((win->condition & 0x01) != 0 && x2 != 0) {
				job_count++; // disable
				sgg_wm0s_accessdisable(&win->sgg);
			}
		} while ((win = win->down) != top);
	}

	if (job_count == 0) {
		job_setvgamode2(); // すぐにディスプレーモード変更へ
	}

	return;
}

void job_setvgamode1(const int cmd, const int handle)
{
	// 0x00c0しかこない
	if (--job_count == 0)
		job_setvgamode2();
	return;
}

void job_setvgamode2()
{
	if (fromboot & 0x0001) {
		// 普通の方法が使えない
		// (仮想86モードでのVGAモード切り換えがうまく行かない)
		x2 = 640;
		y2 = 480;
		sgg_wm0_gapicmd_001c_0020(); // 画面モード設定(640x480)
		sgg_wm0_gapicmd_001c_0004(); // ハードウェア初期化
		init_screen(x2, y2);
		job_general1();
		return;
	}

	int mode = job_int0;
	switch (mode) {
	case 0x0012:
		x2 = 640;
		y2 = 480;
		break;

	case 0x0102:
		x2 = 800;
		y2 = 600;

	}
	jobfunc = &job_setvgamode3;
	sgg_wm0_setvideomode(mode, 0x0014);
	return;
}

void job_setvgamode3(const int sig, const int result)
{
	// 0x0014しかこない
	if (result == 0) {
		sgg_wm0_gapicmd_001c_0004(); // ハードウェア初期化
		init_screen(x2, y2);
		job_general1();
		return;
	}

	// VESAのノンサポートなどにより、画面モード切り換え失敗
	x2 = 640;
	y2 = 480;
//	jobfunc = &job_setvgamode3;
	sgg_wm0_setvideomode(0x0012 /* VGA */, 0x0014);
	return;
}
