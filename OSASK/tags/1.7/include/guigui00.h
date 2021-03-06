#ifndef __GUIGUI00_H
#define __GUIGUI00_H

struct LIB_WORK {
	int data[256 / 4];
};

struct LIB_WINDOW {
	int data[128 / 4];
};

struct LIB_TEXTBOX {
	int data[64 / 4];
};

struct LIB_SIGHNDLREG {
	int ES, DS, FS, GS;
	int EDI, ESI, EBP, ESP;
	int EBX, EDX, ECX, EAX;
	int EIP, CS, EFLAGS;
};

struct LIB_GRAPHBOX {
	int reserve[64 / 4];
};

struct LIB_LINES1 {
	int x0, y0, dx, dy, length, color;
};

struct LIB_LINES0 {
	int x0, y0, x1, y1, dummy, color;
};

void lib_execcmd(void *EBX);
void lib_execcmd0(int cmd, ...);
const int lib_execcmd1(int ret, int cmd, ...);

void *malloc(const unsigned int nbytes);

#if 0
	/* 以下の関数はマクロで実現されている(高速化とコンパクト化のため) */
	/* 引数の型などが分かりやすいように、関数型宣言を註釈として残してある */

struct LIB_WORK *lib_init(struct LIB_WORK *work);
void lib_init_nm(struct LIB_WORK *work);
void lib_waitsignal(int opt, int signaldw, int nest);
struct LIB_WINDOW *lib_openwindow(struct LIB_WINDOW *window, int slot, int x_size, int y_size);
void lib_openwindow_nm(struct LIB_WINDOW *window, int slot, int x_size, int y_size);
struct LIB_TEXTBOX *lib_opentextbox(int opt, struct LIB_TEXTBOX *textbox, int backcolor,
	int x_size, int y_size, int x_pos, int y_pos, struct LIB_WINDOW *window, int charset,
	int init_char);
void lib_opentextbox_nm(int opt, struct LIB_TEXTBOX *textbox, int backcolor, int x_size,
	int y_size, int x_pos, int y_pos, struct LIB_WINDOW *window, int charset, int init_char);
void lib_waitsignaltime(int opt, int signaldw, int nest, unsigned int time0, unsigned int time1,
	unsigned int time2);
int *lib_opensignalbox(int bytes, int *signalbox, int eos, int rewind);
void lib_definesignal0p0(int opt, int default_assign0, int default_assign1, int default_assign2);
void lib_definesignal1p0(int opt, int default_assign0, int default_assign1,
	struct LIB_WINDOW *default_assign2, int signal);
void lib_opentimer(int slot);
void lib_closetimer(int slot);
void lib_settimertime(int opt, int slot, unsigned int time0, unsigned int time1,
	unsigned int time2);
void lib_settimer(int opt, int slot);
void lib_opensoundtrack(int slot);
void lib_controlfreq(int slot, int freq);
struct LIB_WINDOW *lib_openwindow1(struct LIB_WINDOW *window, int slot, int x_size, int y_size,
	int flags, int base);
void lib_openwindow1_nm(struct LIB_WINDOW *window, int slot, int x_size, int y_size, int flags,
	int base);
void lib_closewindow(int opt, struct LIB_WINDOW *window);
void lib_controlwindow(int opt, struct LIB_WINDOW *window);
void lib_close(int opt);
void lib_loadfontset(int opt, int slot, int len, void *font);
void lib_loadfontset0(int opt, int slot);
void lib_makecharset(int opt, int charset, int fontset, int len, int from, int base);
void lib_drawline(int opt, struct LIB_WINDOW *window, int color, int x0, int y0, int x1, int y1);
void lib_closetextbox(int opt, struct LIB_TEXTBOX *textbox);
void lib_mapmodule(int opt, int slot, int attr, int size, void *addr, int ofs);
void lib_unmapmodule(int opt, int size, void *addr);
void lib_initmodulehandle(int opt, int slot);
void lib_putblock1(struct LIB_WINDOW *win, int x, int y, int sx, int sy, int skip, void *p);
struct LIB_GRAPHBOX *lib_opengraphbox(int opt, struct LIB_GRAPHBOX *gbox, int mode, int mode_opt,
	int x_size, int y_size, int x_pos, int y_pos, struct LIB_WINDOW *window);
void lib_flushgraphbox(int opt, struct LIB_WINDOW *win, int x, int y, int sx, int sy, int skip,
	void *p);
void lib_drawline0(int opt, struct LIB_GRAPHBOX *gbox, int color, int x0, int y0, int x1, int y1);
void lib_drawlines0(int opt, struct LIB_GRAPHBOX *gbox, int x0, int y0, int xsize, int ysize,
	int lines, const struct LIB_LINES1 *ofs);
void lib_convlines(int opt, int lines, struct LIB_LINES0 *src, struct LIB_LINES1 *dest);
void lib_initmodulehandle0(int opt, int slot);
void lib_putblock02001(struct LIB_GRAPHBOX *gbox, void *buf, int vx0, int vy0);
struct LIB_GRAPHBOX *lib_opengraphbox2(int opt, struct LIB_GRAPHBOX *gbox, int mode, int mode_opt,
	int x_bsize, int y_bsize, int x_vsize, int y_vsize, int x_pos, int y_pos,
	struct LIB_WINDOW *window);
void lib_putblock03001(struct LIB_GRAPHBOX *gbox, void *buf, int vx0, int vy0, void *tbuf,
	int tbuf_skip, int tcol0);

#endif

void lib_putstring_ASCII(int opt, int x_pos, int y_pos, struct LIB_TEXTBOX *textbox, int color,
	int backcolor, const char *str);
void lib_definesignalhandler(void (*lib_signalhandler)(struct LIB_SIGHNDLREG *));
const int lib_readCSb(int offset);
const int lib_readCSd(int offset);
const int lib_readmodulesize(int slot);
void lib_initmodulehandle1(int slot, int num, int sig);
void lib_steppath0(int opt, int slot, const char *name, int sig);

#define	lib_init(work) \
	(struct LIB_WORK *) lib_execcmd1(1 * 4 + 12, 0x0004, \
	(work) ? (void *) (work) : malloc(sizeof (struct LIB_WORK)), 0x0000)

#define	lib_init_nm(work) \
	lib_execcmd0(0x0004, (void *) (work), 0x0000)

#define	lib_waitsignal(opt, signaldw, nest) \
	lib_execcmd0(0x0018, (int) (opt), (int) (signaldw), (int) (nest), 0x0000)

#define	lib_openwindow(window, slot, x_size, y_size) \
	(struct LIB_WINDOW *) lib_execcmd1(1 * 4 + 12, 0x0020, \
	(window) ? (void *) (window) : malloc(sizeof (struct LIB_WINDOW)), \
	(int) (slot), (int) (x_size), (int) (y_size), 0x0000)

#define	lib_openwindow_nm(window, slot, x_size, y_size) \
	lib_execcmd0(0x0020, (void *) (window), (int) (slot), (int) (x_size), \
	(int) (y_size), 0x0000)

#define	lib_opentextbox(opt, textbox, backcolor, x_size, y_size, x_pos, y_pos, window, charset, init_char) \
	(struct LIB_TEXTBOX *) lib_execcmd1(2 * 4 + 12, 0x0028, (int) (opt), \
	(textbox) ? (void *) (textbox) : malloc(sizeof (struct LIB_TEXTBOX) + 8 * (x_size) * (y_size)), \
	(int) (backcolor), (int) (x_size), (int) (y_size), (int) (x_pos), \
	(int) (y_pos), (void *) (window), (int) (charset), (int) (init_char), \
	0x0000)

#define	lib_opentextbox_nm(opt, textbox, backcolor, x_size, y_size, x_pos, y_pos, window, charset, init_char) \
	lib_execcmd0(0x0028, (int) (opt), (void *) (textbox), (int) (backcolor), \
	(int) (x_size), (int) (y_size), (int) (x_pos), (int) (y_pos), \
	(void *) (window), (int) (charset), (int) (init_char), 0x0000)

#define	lib_waitsignaltime(opt, signaldw, nest, time0, time1, time2) \
	lib_execcmd0(0x0018, (int) (opt), (int) (signaldw), (int) (nest), \
	(int) (time0), (int) (time1), (int) (time2), 0x0000)

#define	lib_opensignalbox(bytes, signalbox, eos, rewind) \
	(int *) lib_execcmd1(2 * 4 + 12, 0x0060, (int) (bytes), \
	(signalbox) ? (void *) (signalbox) : malloc(bytes), (int) (eos), \
	(int) (rewind), 0x0000)

#define	lib_opensignalbox_nm(bytes, signalbox, eos, rewind) \
	lib_execcmd0(0x0060, (int) (bytes), (void *) (signalbox), (int) (eos), \
	(int) (rewind), 0x0000)

#define	lib_definesignal0p0(opt, default_assign0, default_assign1, default_assign2) \
	lib_execcmd0(0x0068, (int) (opt), (int) (default_assign0), \
	(int) (default_assign1), (int) (default_assign2), 0, 0, 0x0000)

#define	lib_definesignal1p0(opt, default_assign0, default_assign1, default_assign2, signal) \
	lib_execcmd0(0x0068, (int) (opt), (int) (default_assign0), \
	(int) (default_assign1), (int) (default_assign2), 1, (int) (signal), \
	0, 0x0000)

#define	lib_opentimer(slot) \
	lib_execcmd0(0x0070, (int) (slot), 0x0000)

#define	lib_closetimer(slot) \
	lib_execcmd0(0x0074, (int) (slot), 0x0000)

#define	lib_settimertime(opt, slot, time0, time1, time2) \
	lib_execcmd0(0x0078, (int) (opt), (int) (slot), (int) (time0), \
	(int) (time1), (int) (time2), 0x0000)

#define	lib_settimer(opt, slot) \
	lib_execcmd0(0x0078, (int) (opt), (int) (slot), 0x0000)

#define	lib_opensoundtrack(slot) \
	lib_execcmd0(0x0080, (int) (slot), 0, 0x0000)

#define	lib_controlfreq(slot, freq) \
	lib_execcmd0(0x008c, (int) (slot), (int) (freq), 0x0000)

#define	lib_openwindow1(window, slot, x_size, y_size, flags, base) \
	(struct LIB_WINDOW *) lib_execcmd1(1 * 4 + 12, 0x0020, \
	(window) ? (void *) (window) : malloc(sizeof (struct LIB_WINDOW)), \
	(int) (slot) | 0x01, (int) (x_size), (int) (y_size), \
	0x01 | (int) (flags) << 8, (int) (base), 0x0000)

#define	lib_openwindow1_nm(window, slot, x_size, y_size, flags, base) \
	lib_execcmd0(0x0020, (void *) (window), (int) (slot) | 0x01, \
	(int) (x_size), (int) (y_size), 0x01 | (int) (flags) << 8, (int) (base), \
	0x0000)

#define	lib_closewindow(opt, window) \
	lib_execcmd0(0x0024, (int) (opt), (void *) (window), 0x0000)

#define	lib_controlwindow(opt, window) \
	lib_execcmd0(0x003c, (int) (opt), (void *) (window), 0x0000)

#define	lib_close(opt) \
	lib_execcmd0(0x0008, (int) (opt), 0x0000)

#define	lib_loadfontset(opt, slot, len, font) \
	lib_execcmd0(0x00e0, (int) (opt), (int) (slot), (int) (len), (int) (font), \
	0x000c, 0x0000)

#define	lib_loadfontset0(opt, slot) \
	lib_execcmd0(0x00e0, (int) (opt), (int) (slot), 0x0000)

#define	lib_makecharset(opt, charset, fontset, len, from, base) \
	lib_execcmd0(0x00e8, (int) (opt), (int) (charset), (int) (fontset), \
	(int) (len), (int) (from), (int) (base), 0x0000)

#define	lib_drawline(opt, window, color, x0, y0, x1, y1) \
	lib_execcmd0(0x0044, (int) (opt), (void *) (window), (int) (color), \
	(int) (x0), (int) (y0), (int) (x1), (int) (y1), 0x0000)

#define	lib_closetextbox(opt, textbox) \
	lib_execcmd0(0x002c, (int) (opt), (int) (textbox), 0x0000)

#define	lib_mapmodule(opt, slot, attr, size, addr, ofs) \
	lib_execcmd0(0x00c0, (int) (opt), (int) (slot), (int) (size), \
	(void *) (addr), 0x000c, (int) ((ofs) | (attr)), 0x0000)

#define	lib_unmapmodule(opt, size, addr) \
	lib_execcmd0(0x00c4, (int) (opt), (int) (size), (void *) (addr), 0x000c, \
	0x0000)

#define	lib_initmodulehandle(opt, slot) \
	lib_execcmd0(0x00a0, (int) (opt), (int) (slot), 0x0000)

#define	lib_putblock1(win, x, y, sx, sy, skip, p) \
	lib_execcmd0(0x004c, 1, (void *) (win), (int) (x), (int) (y), (int) (sx), \
	(int) (sy), (int) (skip), (void *) (p), 0x000c, 0x0000)

#define	lib_opengraphbox(opt, graphbox, mode, mode_opt, x_size, y_size, x_pos, y_pos, window) \
	(struct LIB_GRAPHBOX *) lib_execcmd1(2 * 4 + 12, 0x0030, (int) (opt), \
	(graphbox) ? (void *) (graphbox) : malloc(sizeof (struct LIB_GRAPHBOX) + (x_size) * (y_size)), \
	(int) (mode), (int) (mode_opt), (int) (x_size), (int) (y_size), \
	(int) (x_pos), (int) (y_pos), (void *) (window), 0x0000)

#define	lib_flushgraphbox(opt, win, x, y, sx, sy, skip, p) \
	lib_execcmd0(0x004c, opt, (void *) (win), (int) (x), (int) (y), (int) (sx), \
	(int) (sy), (int) (skip), (void *) (p), 0x000c, 0x0000)

#define	lib_drawline0(opt, gbox, color, x0, y0, x1, y1) \
	lib_execcmd0(0x0054, (int) (opt), (void *) (gbox), (int) (color), \
	(int) (x0), (int) (y0), (int) (x1), (int) (y1), 0x0000)

#define lib_drawlines0(opt, gbox, x0, y0, xsize, ysize, lines, ofs) \
	lib_execcmd0(0x0108, (int) (opt), (void *) (gbox), (int) (x0), \
	(int) (y0), (int) (xsize), (int) (ysize), (int) (lines), (void *) (ofs), \
	0x000c, 0x0000)

#define	lib_convlines(opt, lines, src, dest) \
	lib_execcmd0(0x010c, (int) (opt), (int) (lines), \
	(struct LIB_LINES0 *) (src), 0x000c, (struct LIB_LINES1 *) (dest), \
	0x000c, 0x0000)

#define	lib_initmodulehandle0(opt, slot) \
	lib_execcmd0(0x00a0, (int) (opt), (int) (slot), 0x0000)

#define lib_putblock02001(gbox, buf, vx0, vy0) \
	lib_execcmd0(0x0058, 0x2001, (void *) (gbox), (void *) (buf), \
	(int) (vx0), (int) (vy0), 0x0000)

#define	lib_opengraphbox2(opt, graphbox, mode, mode_opt, x_bsize, y_bsize, \
	x_vsize, y_vsize, x_pos, y_pos, window) \
	(struct LIB_GRAPHBOX *) lib_execcmd1(2 * 4 + 12, 0x0030, (int) (opt), \
	(graphbox) ? (void *) (graphbox) : malloc(sizeof (struct LIB_GRAPHBOX) + (x_bsize) * (y_bsize)), \
	(int) (mode), (int) (mode_opt), (int) (x_bsize), (int) (y_bsize), \
	(int) (x_vsize), (int) (y_vsize), (int) (x_pos), (int) (y_pos), \
	(void *) (window), 0x0000)

#define lib_putblock03001(gbox, buf, vx0, vy0, tbuf, tbuf_skip, tcol0) \
	lib_execcmd0(0x0058, 0x3001, (void *) (gbox), (void *) (buf), \
	(int) (vx0), (int) (vy0), (void *) (tbuf), (int) (tbuf_skip), \
	(int) (tcol0), 0x0000)

#endif
