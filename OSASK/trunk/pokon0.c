// "pokon0.c":�A�v���P�[�V�������E���`���[  ver.1.0
//   copyright(C) 2001 �썇�G��, �����떾
//  exe2bin0 pokon0 -s 32k

#include <guigui00.h>
#include <sysgg00.h>
// sysgg�́A��ʂ̃A�v�������p���Ă͂����Ȃ����C�u����
// �d�l�����Ȃ藬���I
#include <stdlib.h>

#define	AUTO_MALLOC		0
#define	NULL			0
#define	SYSTEM_TIMER	0x01c0
#define LIST_HEIGHT		8
#define ext_EXE			('E' | ('X' << 8) | ('E' << 16))
#define ext_BIN			('B' | ('I' << 8) | ('N' << 16))
#define	CONSOLESIZEX	40
#define	CONSOLESIZEY	15

struct FILELIST {
	char name[11];
	struct SGG_FILELIST *ptr;
};

unsigned int counter = 0, banklist[8];
struct SGG_FILELIST *file;
struct FILELIST *list, *lp;
struct LIB_TEXTBOX *selector, *console_txt;
struct LIB_WINDOW *console_win = NULL;
static unsigned char *consolebuf = NULL;
static int console_curx, console_cury, console_col;
void consoleout(char *s);
void open_console();

struct LIB_WINDOW *lib_openwindow2(struct LIB_WINDOW *window, const int slot,
	const int x_size, const int y_size, const int flags, const int base)
{
	static struct {
		int cmd;
		struct LIB_WINDOW *work_ptr;
		int slot;
		int x_size, y_size;
		char signal_length /* 1�ɂ��� */, signal_flags, dummy[2];
		int signal_base;
		int eoc;
	} command = { 0x0020, 0, 0, 0, 0, 1, 0, { 0, 0 }, 0, 0x0000 };

	if (window)
		command.work_ptr = window;
	else
		command.work_ptr = (struct LIB_WINDOW *) malloc(sizeof (struct LIB_WINDOW));
	command.slot = slot | 0x01;
	command.x_size = x_size;
	command.y_size = y_size;
	command.signal_flags = flags;
	command.signal_base = base;
	lib_execcmd(&command);
	return command.work_ptr;
}

void lib_closewindow(const int opt, struct LIB_WINDOW *window)
{
	static struct {
		int cmd, opt;
		struct LIB_WINDOW *window;
		int eoc;
	} command = { 0x0024, 0, 0, 0x0000 };

	command.opt = opt;
	command.window = window;
	lib_execcmd(&command);
	return;
}

void lib_controlwindow(const int opt, struct LIB_WINDOW *window)
{
	static struct {
		int cmd, opt;
		struct LIB_WINDOW *window;
		int eoc;
	} command = { 0x003c, 0, 0, 0x0000 };

	command.opt = opt;
	command.window = window;
	lib_execcmd(&command);
	return;
}

void put_file(const char *name, const int pos, const int col)
{
	static char charbuf16[17] = "          .     ";
	if (*name) {
		charbuf16[2] = name[0];
		charbuf16[3] = name[1];
		charbuf16[4] = name[2];
		charbuf16[5] = name[3];
		charbuf16[6] = name[4];
		charbuf16[7] = name[5];
		charbuf16[8] = name[6];
		charbuf16[9] = name[7];
	//	charbuf16[10] = '.';
		charbuf16[11] = name[ 8];
		charbuf16[12] = name[ 9];
		charbuf16[13] = name[10];
		if (col)
			lib_putstring_ASCII(0x0001, 0, pos, selector, 15,  2, charbuf16);
		else
			lib_putstring_ASCII(0x0001, 0, pos, selector,  0, 15, charbuf16);
	} else
		lib_putstring_ASCII(0x0000, 0, pos, selector, 0, 0, "                ");
	return;
}

const int list_set(const int ext)
{
	int i;
	struct SGG_FILELIST *fp;
	struct FILELIST *wp1, *wp2, tmp;

	// �V�X�e���Ƀt�@�C���̃��X�g��v��
	sgg_getfilelist(256, file, 0, 0);

	// �A�v���P�[�V������LIST_HEIGHT�ȉ��̂Ƃ��̂��߂̏��u
	for (i = 0; i <= LIST_HEIGHT; i++)
		list[i].name[0] = '\0';

	// �g���qext�̂��̂����𒊏o����list��
	fp = file + 1; // �ŏ��̗v�f�̓_�~�[
	lp = list;

	while (fp->name[0]) {
		if ((fp->name[8] | (fp->name[9] << 8) | (fp->name[10] << 16)) == ext) {
			for (i = 0; i < 11; i++)
				lp->name[i] = fp->name[i];
			lp->ptr = fp;
			lp++;
		}
		fp++;
	}
	lp->name[0] = '\0';

	// sort by Koyanagi
	if (list[0].name[0] != '\0') {
		for (wp1 = list; wp1->name[0]; ++wp1) {
			for (wp2 = lp - 1; wp2 != wp1; --wp2) {
				// compare
				for (i = 0; i < 11; ++i) {
					if ((wp2 - 1)->name[i] > wp2->name[i]) {
						// swap and break
						tmp = *wp2;
						*wp2 = *(wp2 - 1);
						*(wp2 - 1) = tmp;
						break;
					} if ((wp2 - 1)->name[i] < wp2->name[i]) {
						break;
					}
				}
			}
		}
	}
	//
	lp = list;
	for (i = 0; i < LIST_HEIGHT; i++)
 		put_file(lp[i].name, i, 0);

	if (list[0].name[0] == '\0') {
		// �I���\�ȃt�@�C�����P���Ȃ�
		lib_putstring_ASCII(0x0000, 0, LIST_HEIGHT / 2 - 1, selector, 1, 0, "File not found! ");
		return -1;
	}
 	put_file(lp[0].name, 0, 1);
	return 0;
}

int *sb0, *sbp;
char sleep = 0, cursorflag = 0, cursoractive = 0;

void wait99sub()
{
	if (*sbp == 0) {
		sleep = 1;
		lib_waitsignal(0x0001, 0, 0);
	} else if (*sbp == 1) {
		lib_waitsignal(0x0000, *(sbp + 1), 0);
		sbp = sb0;
	} else if (*sbp == 0x0080) {
		// �^�X�N�I��
		int i;
		for (i = 0; banklist[i] != sbp[1]; i++);
		banklist[i] = 0;
		sgg_freememory(0x0220 /* "empty00" */ + i * 16);
		lib_waitsignal(0x0000, 2, 0);
		sbp += 2;
	} else {
		lib_waitsignal(0x0000, 1, 0);
		sbp++;
	}
	return;
}

void wait99()
{
	while (*sbp != 99)
		wait99sub();
	lib_waitsignal(0x0000, 1, 0); // 99�̕�������
	sbp++;
	return;
}

void putcursor()
{
	sleep = 0;
	lib_putstring_ASCII(0x0001, console_curx, console_cury,
		console_txt, 0, ((console_col & cursorflag) | ((console_col >> 4) & ~cursorflag)) & 0x0f, " ");
	return;
}

void main()
{
	struct LIB_WINDOW *window;
	struct LIB_TEXTBOX *wintitle, *mode;
	char charbuf16[17];
	int cur, i, sig, bank, fmode = 0, ext;
	struct SGG_FILELIST *fp;

	lib_init(AUTO_MALLOC);
	sgg_init(AUTO_MALLOC);

	sbp = sb0 = lib_opensignalbox(256, AUTO_MALLOC, 0, 1);

	file = (struct SGG_FILELIST *) malloc(256 * sizeof (struct SGG_FILELIST));
	list = (struct FILELIST *) malloc(256 * sizeof (struct FILELIST));

	window = lib_openwindow(AUTO_MALLOC, 0x0200, 160, 40 + LIST_HEIGHT * 16);
	wintitle = lib_opentextbox(0x1000, AUTO_MALLOC,  0,  7, 1,  0,  0, window, 0x00c0, 0);
	mode     = lib_opentextbox(0x0000, AUTO_MALLOC,  0, 20, 1,  0,  0, window, 0x00c0, 0); // 256bytes
	selector = lib_opentextbox(0x0001, AUTO_MALLOC, 15, 16, 8, 16, 32, window, 0x00c0, 0); // 1.1KB

	lib_putstring_ASCII(0x0000, 0, 0, wintitle, 0, 0, "pokon10");
	lib_opentimer(SYSTEM_TIMER);
	lib_definesignal1p0(0, 0x0010 /* timer */, SYSTEM_TIMER, 0, 287);

	// �L�[�����o�^
	lib_definesignal1p0(1, 0x0100, 0x00ae /* cursor-up */,   window,  4);
//	lib_definesignal1p0(0, 0x0100, 0x00af /* cursor-down */, window,  5);
	lib_definesignal1p0(0, 0x0100, 0x00a0 /* Enter */,       window,  6);
	lib_definesignal1p0(0, 0x0100, 'F',                      window,  7);
	lib_definesignal1p0(0, 0x0100, 'f',                      window,  7);
	lib_definesignal1p0(0, 0x0100, 'R',                      window,  8);
	lib_definesignal1p0(0, 0x0100, 'r',                      window,  8);
	lib_definesignal1p0(0, 0x0100, 'S',                      window,  9);
	lib_definesignal1p0(0, 0x0100, 's',                      window,  9);
	lib_definesignal1p0(1, 0x0100, 0x00a8 /* page-up */,     window, 10);
//	lib_definesignal1p0(0, 0x0100, 0x00a9 /* page-down */,   window, 11);
	lib_definesignal1p0(1, 0x0100, 0x00ac /* cursor-left */, window, 10);
//	lib_definesignal1p0(0, 0x0100, 0x00ad /* cursor-right */,window, 11);
	lib_definesignal1p0(1, 0x0100, 0x00a6 /* Home */,        window, 12);
//	lib_definesignal1p0(0, 0x0100, 0x00a7 /* End */,         window, 13);
	lib_definesignal1p0(0, 0x0100, 0x00a4 /* Insert */,      window, 14);
	lib_definesignal1p0(0, 0x0100, 'C',                      window, 15);
	lib_definesignal1p0(0, 0x0100, 'c',                      window, 15);
	lib_definesignal1p0(0, 0x0100, 'M',                      window, 16);
	lib_definesignal1p0(0, 0x0100, 'm',                      window, 16);
	lib_definesignal0p0(0, 0, 0, 0);

	lib_putstring_ASCII(0x0000, 0, 0, mode,     0, 0, "< Run Application > ");
	cur = list_set(ext = ext_BIN);

	for (i = 0; i < 8; i++)
		banklist[i] = 0;

	for (;;) {
		do {
			sig = *sbp;
			wait99sub();
		} while (sig == 0);
  
		switch (sig) {

		case 4 /* cursor-up */:
			if (cur < 0 /* �t�@�C�����P���Ȃ� */)
				break;
			if (cur > 0) {
 				put_file(lp[cur].name, cur, 0);
				cur--;
 				put_file(lp[cur].name, cur, 1);
			} else if (lp > list) {
				lp--;
listup:
				for (i = 0; i < LIST_HEIGHT; i++) {
					if (i != cur)
						put_file(lp[i].name, i, 0);
					else
						put_file(lp[cur].name, cur, 1);
				}
			}
			break;

		case 5 /* cursor-down */:
		//	if (cur < 0 /* �t�@�C�����P���Ȃ� */)
		//		break;
		//	�t�@�C�����Ȃ��ꍇ�Acur == -1 && lp[0].name[0] == '\0'
		//	�Ȃ̂ŁA�ȉ���if���������Ȃ��B
			if (lp[cur + 1].name[0]) {
				if (cur < LIST_HEIGHT - 1) {
 					put_file(lp[cur].name, cur, 0);
					cur++;
 					put_file(lp[cur].name, cur, 1);
				} else {
					lp++;
					goto listup;
				}
			}
			break;

		case 6 /* Enter */:
			if (cur < 0 /* �t�@�C����1���Ȃ� */)
				break;
			if (ext == ext_BIN) {
				// .BIN�t�@�C�����[�h
				for (bank = 2; bank < 8; bank++) {
					if (banklist[bank] == 0)
						goto find_freebank;
				}
				if (banklist[0] == 0)
					bank = 0;
				else if (banklist[1] == 0)
					bank = 1;
				else
					break;
find_freebank:
				sgg_loadfile(0x0220 /* load to "empty00" */ + bank * 16,
					lp[cur].ptr->id /* file id */,
					99 /* finish signal */
				);
				wait99(); // finish signal������܂ő҂�
				i = *sbp++;
				lib_waitsignal(0x0000, 1, 0);
				if (i)
					break; /* switch�����甲���� */

				sgg_createtask(0x0220 /* "empty00" */ + bank * 16, 99 /* finish signal */);
				wait99(); // finish signal������܂ő҂�

				banklist[bank] = i = *sbp++;
				lib_waitsignal(0x0000, 1, 0);

				// �K����GUIGUI00�t�@�C���łȂ��ꍇ�A�^�X�N�͐������ꂸ�Ai��0�B
				if (i) {
					sgg_settasklocallevel(i,
						1 * 32 /* local level 1 (�N���E�V�X�e���������x��) */,
						16 * 64 /* gloval level 16 (��ʃA�v���P�[�V����) */,
						 2 /* Inner level */
					);
					sgg_settasklocallevel(i,
						2 * 32 /* local level 2 (�ʏ폈�����x��) */,
						16 * 64 /* gloval level 16 (��ʃA�v���P�[�V����) */,
						 2 /* Inner level */
					);
					sgg_runtask(i, 1 * 32);
				} else {
					// ���[�h�����̈�����
					sgg_freememory(0x0220 /* "empty00" */ + bank * 16);
				}
			} else if (/* ext == ext_EXE && */ (banklist[0] | banklist[1]) == 0) {
				// .EXE�t�@�C�����[�h
				sgg_loadfile(0x0220 /* load to "empty00" */,
					lp[cur].ptr->id /* file id */,
					99 /* finish signal */
				);
				wait99(); // finish signal������܂ő҂�
				i = *sbp++;
				lib_waitsignal(0x0000, 1, 0);
				if (i)
					break; /* �������s���Ń��[�h�Ɏ��s�Bswitch�����甲���� */

				i = ('K' | ('B' << 8) | ('S' << 16) | ('1' << 24));
				if (fmode)
					i = ('K' | ('B' << 8) | ('S' << 16) | ('0' << 24));

				for (fp = file + 1; fp->name[0]; fp++) {
					if ((fp->name[0] | (fp->name[1] << 8) | (fp->name[2] << 16) | (fp->name[3] << 24))
						== ('O' | ('S' << 8) | ('A' << 16) | ('S' << 24)) &&
					    (fp->name[4] | (fp->name[5] << 8) | (fp->name[6] << 16) | (fp->name[7] << 24))
						== i &&
					    (fp->name[8] | (fp->name[9] << 8) | (fp->name[10] << 16))
						== ('B' | ('I' << 8) | ('N' << 16))) {
						i = fp->id;
						break;
					}
				}
				if (fp->name[0] == 0)
					break; /* "OSASKBS0.BIN"�A"OSASKBS0.BIN"��������Ȃ����� */
				sgg_loadfile(0x0230 /* load to "empty01" */,
					i /* file id */, 99 /* finish signal */);
				wait99(); // finish signal������܂ő҂�
				i = *sbp++;
				lib_waitsignal(0x0000, 1, 0);
				if (i)
					break; /* �������s���Ń��[�h�Ɏ��s�Bswitch�����甲���� */

				for (i = 0; i < LIST_HEIGHT; i++)
					lib_putstring_ASCII(0x0000, 0, i, selector, 0, 0, "                ");
				lib_putstring_ASCII(0x0000, 0, 1, selector, 0, 0, "    Loaded.     ");
				lib_putstring_ASCII(0x0000, 0, 3, selector, 0, 0, " Change disks.  ");
				lib_putstring_ASCII(0x0000, 0, 5, selector, 0, 0, " Hit Enter key. ");

				// 6, 7, 8, 9��҂�
				while (*sbp != 6 && *sbp != 7 && *sbp != 8 && *sbp != 9)
					wait99sub();
				sig = *sbp++;
				lib_waitsignal(0x0000, 1, 0);

				if (sig == 7)
					goto signal_7;
				if (sig == 8)
					goto signal_8;

				lib_putstring_ASCII(0x0000, 0, 5, selector, 0, 0, "  Please wait.  ");
				if (sig == 6) {
					lib_putstring_ASCII(0x0000, 0, 1, selector, 0, 0, "  Formating...  ");
					lib_putstring_ASCII(0x0000, 0, 3, selector, 0, 0, "                ");
				//	lib_putstring_ASCII(0x0000, 0, 5, selector, 0, 0, "  Please wait.  ");
					i = 0x0124;
					if (fmode)
						i = 0x0118;
					sgg_format(i, 99 /* finish signal */); // format
					wait99(); // finish signal������܂ő҂�
				}
				lib_putstring_ASCII(0x0000, 0, 1, selector, 0, 0, " Writing        ");
				lib_putstring_ASCII(0x0000, 0, 3, selector, 0, 0, "   system image.");
			//	lib_putstring_ASCII(0x0000, 0, 5, selector, 0, 0, "  Please wait.  ");
				i = 0x0128;
				if (fmode)
					i = 0x011c;
				sgg_format(i, 99 /* finish signal */); // store system image
				wait99(); // finish signal������܂ő҂�

				// ���[�h�����̈�����
				sgg_freememory(0x0220 /* "empty00" */);
				sgg_freememory(0x0230 /* "empty01" */);

				lib_putstring_ASCII(0x0000, 0, 1, selector, 0, 0, "   Completed.   ");
				lib_putstring_ASCII(0x0000, 0, 3, selector, 0, 0, " Change disks.  ");
				lib_putstring_ASCII(0x0000, 0, 5, selector, 0, 0, "  Hit 'R' key.  ");

				// 7, 8��҂�
				while (*sbp != 7 && *sbp != 8)
					wait99sub();
				sig = *sbp++;
				lib_waitsignal(0x0000, 1, 0);

				sgg_format(0x0114, 99 /* finish signal */); // flush diskcache
				wait99(); // finish signal������܂ő҂�

				if (sig == 7)
					goto signal_7;
			//	if (sig == 8)
					goto signal_8;
			}
			break;

		case 7 /* to format-mode */:
		signal_7:
			lib_putstring_ASCII(0x0000, 0, 0, mode, fmode, 0, "< Load Systemimage >");
			cur = list_set(ext = ext_EXE);
			break;

		case 8 /* to run-mode */:
		signal_8:
			lib_putstring_ASCII(0x0000, 0, 0, mode,     0, 0, "< Run Application > ");
			cur = list_set(ext = ext_BIN);
			break;

		case 9 /* change format-mode */:
			if (ext == ext_EXE) {
				fmode ^= 0x01;
				lib_putstring_ASCII(0x0000, 0, 0, mode, fmode * 9, 0, "< Load Systemimage >");
			}
			break;

		case 10 /* page-up */:
			if (cur < 0 /* �t�@�C�����P���Ȃ� */)
				break;
			if (lp >= list + LIST_HEIGHT)
				lp -= LIST_HEIGHT;
			else {
				lp = list;
				cur = 0;
			}
			goto listup;

		case 11 /* page-down */:
			if (cur < 0 /* �t�@�C�����P���Ȃ� */)
				break;
			for (i = 1; lp[i].name[0] != '\0' && i < LIST_HEIGHT * 2; i++);
			if (i < LIST_HEIGHT) {
				// �S�̂�1��ʕ��ɖ����Ȃ�����
				cur = i - 1;
				goto listup;
			} else if (i < LIST_HEIGHT * 2) {
				// �c�肪1��ʕ��ɖ����Ȃ�����
				lp += i - LIST_HEIGHT;
				cur = LIST_HEIGHT - 1;
				goto listup;
			}
			lp += LIST_HEIGHT;
			goto listup;

		case 12 /* Home */:
			if (cur < 0 /* �t�@�C�����P���Ȃ� */)
				break;
			lp = list;
			cur = 0;
			goto listup;

		case 13 /* End */:
			if (cur < 0 /* �t�@�C�����P���Ȃ� */)
				break;
			lp = list;
			for (i = 0; lp[i].name[0]; i++);
			if (i < LIST_HEIGHT) {
				// �t�@�C������1��ʕ��ɖ����Ȃ�����
				cur = i - 1;
			} else {
				lp += i - LIST_HEIGHT;
				cur = LIST_HEIGHT - 1;
			}
			goto listup;

		case 14 /* Insert */:
			sgg_format(0x0114, 99 /* finish signal */); // flush diskcache
			wait99(); // finish signal������܂ő҂�
			cur = list_set(ext);
			break;

		case 15 /* open console */:
			if (console_win == NULL)
				open_console();
			break;

		case 16 /* open monitor */:
			break;

		// console�֌W

		case 256 + 0 /* VRAM�A�N�Z�X���� */:
			break;

		case 256 + 1 /* VRAM�A�N�Z�X�֎~ */:
			lib_controlwindow(0x0100, console_win);
			break;

		case 256 + 2:
		case 256 + 3:
			/* �ĕ`�� */
			lib_controlwindow(0x0203, console_win);
			break;

		case 256 + 5 /* change console title color */:
			if (*sbp++ & 0x02) {
				if (!cursoractive) {
					lib_settimer(0x0001, SYSTEM_TIMER);
					cursoractive = 1;
					cursorflag = ~0;
					putcursor();
					lib_settimertime(0x0032, SYSTEM_TIMER, 0x80000000 /* 500ms */, 0, 0);
				}
			} else {
				if (cursoractive) {
					cursoractive = 0;
					cursorflag = 0;
					putcursor();
					lib_settimer(0x0001, SYSTEM_TIMER);
				}
			}
			lib_waitsignal(0x0000, 1, 0);
			break;

		case 256 + 6 /* close console window */:
			lib_closewindow(0, console_win);
			console_win = NULL;
			if (cursoractive) {
				cursoractive = 0;
				lib_settimer(0x0001, SYSTEM_TIMER);
			}
			break;

		case 287 /* cursor blink */:
			if (sleep == 1 && cursoractive != 0) {
				cursorflag =~ cursorflag;
				putcursor();
				lib_settimertime(0x0012, SYSTEM_TIMER, 0x80000000 /* 500ms */, 0, 0);
			}
			break;

		default:
			if (256 + ' ' <= sig && sig <= 256 + 0x7f) {
				/* console�ւ�1�������� */
				if (console_win != NULL) {
					if (console_curx < CONSOLESIZEX - 1) {
						static char c[2] = { 0, 0 };
						c[0] = sig - 256;
						consoleout(c);
					}
					if (cursoractive) {
						lib_settimer(0x0001, SYSTEM_TIMER);
						cursorflag = ~0;
						putcursor();
						lib_settimertime(0x0032, SYSTEM_TIMER, 0x80000000 /* 500ms */, 0, 0);
					}
				}
			} else if (sig == 256 + 0xa0) {
				/* console�ւ�Enter���� */
				if (console_win != NULL) {
					if (cursorflag != 0 && cursoractive != 0) {
						cursorflag = 0;
						putcursor();
					}
					{
						char *p = consolebuf + console_cury * (CONSOLESIZEX + 2) + 5;
						while (*p == ' ')
							p++;
						if (*p)
							consoleout("\nBad command.\n");
					}
					consoleout("\npoko>");
					if (cursoractive) {
						lib_settimer(0x0001, SYSTEM_TIMER);
						cursorflag = ~0;
						putcursor();
						lib_settimertime(0x0032, SYSTEM_TIMER, 0x80000000 /* 500ms */, 0, 0);
					}
				}
			} else if (sig == 256 + 0xa1) {
				/* console�ւ�BackSpace���� */
				if (console_win != NULL) {
					if (cursorflag != 0 && cursoractive != 0) {
						cursorflag = 0;
						putcursor();
					}
					if (console_curx > 5) {
						console_curx--;
						consoleout(" ");
						console_curx--;
					}
					if (cursoractive) {
						lib_settimer(0x0001, SYSTEM_TIMER);
						cursorflag = ~0;
						putcursor();
						lib_settimertime(0x0032, SYSTEM_TIMER, 0x80000000 /* 500ms */, 0, 0);
					}
				}
			}
		//	break;

//		case 99:
//			lib_putstring_ASCII(0x0000, 0, 0, mode,     0, 0, "< Error 99        > ");
//			break;
		}
	}
}

void open_monitor()
/*	���j�^�[���I�[�v������ */
{



}

void consoleout(char *s)
{
	char buf[CONSOLESIZEX + 1], *bp = buf,
		*cbp = consolebuf + console_cury * (CONSOLESIZEX + 2) + console_curx;
	int curx = console_curx;
	while (*s) {
		if (*s == '\n') {
			s++;
			*bp = '\0';
			lib_putstring_ASCII(0x0001, console_curx, console_cury,
				console_txt, console_col & 0x0f, (console_col >> 4) & 0x0f, buf);
			console_cury++;
			console_curx = curx = 0;
			if (console_cury == CONSOLESIZEY) {
				/* �X�N���[������ */
				int i, j;
				bp = consolebuf;
				for (j = 0; j < CONSOLESIZEY; j++) {
					for (i = 0; i < CONSOLESIZEX + 2; i++)
						bp[i] = bp[i + (CONSOLESIZEX + 2)];
					lib_putstring_ASCII(0x0001, 0, j, console_txt,
						bp[CONSOLESIZEX + 1] & 0x0f, (bp[CONSOLESIZEX + 1] >> 4) & 0x0f, bp);
					bp += CONSOLESIZEX + 2;
				}
				console_cury = CONSOLESIZEY - 1;
			}
			bp = buf;
			cbp = consolebuf + console_cury * (CONSOLESIZEX + 2);
		} else {
			*cbp++ = *bp++ = *s++;
			consolebuf[console_cury * (CONSOLESIZEX + 2) + (CONSOLESIZEX + 1)] = console_col;
			curx++;
		}
	}
	if (bp != buf) {
		*bp = '\0';
		lib_putstring_ASCII(0x0001, console_curx, console_cury,
			console_txt, console_col & 0x0f, (console_col >> 4) & 0x0f, buf);
		console_curx = curx;
	}
	return;
}

void open_console()
/*	�R���\�[�����I�[�v������ */
/*	�J�[�\���_�ł̂��߂ɁAsetmode���E�� */
/*	�J�[�\���_�ł̂��߂̃^�C�}�[���C�l�[�u���ɂ��� */
{
	struct LIB_TEXTBOX *console_tit;
	int i, j;
	char *bp;
	console_win = lib_openwindow2(AUTO_MALLOC, 0x0210, CONSOLESIZEX * 8, CONSOLESIZEY * 16, 0x0d, 256);
	console_tit = lib_opentextbox(0x1000, AUTO_MALLOC,  0, 16,  1,  0,  0, console_win, 0x00c0, 0);
	console_txt = lib_opentextbox(0x0001, AUTO_MALLOC,  0, CONSOLESIZEX, CONSOLESIZEY,  0,  0, console_win, 0x00c0, 0); // 5KB
	lib_putstring_ASCII(0x0000, 0, 0, console_tit, 0, 0, "pokon10 console");
	if (consolebuf == NULL)
		consolebuf = (char *) malloc((CONSOLESIZEX + 2) * (CONSOLESIZEY + 1));
	bp = consolebuf;
	for (j = 0; j < CONSOLESIZEY + 1; j++) {
		for (i = 0; i < CONSOLESIZEX; i++) {
			*bp++ = ' ';
		}
		bp[0] = '\0';
		bp[1] = 0;
		bp += 2;
	}
	lib_definesignal1p0(0x5f, 0x0100, ' ',              console_win, 256 + ' ');
	lib_definesignal1p0(1,    0x0100, 0xa0 /* Enter */, console_win, 256 + 0xa0);
	lib_definesignal0p0(0, 0, 0, 0);
	console_curx = console_cury = 0;
	console_col = 15;
	consoleout("Heppoko-shell \"poko\" version 1.7\n    Copyright (C) 2001 H.Kawai(Kawaido)\n");
	consoleout("\npoko>");
	if (cursoractive) {
		lib_settimer(0x0001, SYSTEM_TIMER);
		cursorflag = ~0;
		putcursor();
		lib_settimertime(0x0032, SYSTEM_TIMER, 0x80000000 /* 500ms */, 0, 0);
	}
	return;
}
