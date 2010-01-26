#include <guigui01.h>

typedef unsigned char UCHAR;
int get32(const UCHAR *p);
void put32(UCHAR *p, int i);

#define MAXSIZ	2 * 1024 * 1024

#define	CMDLIN_IN		0
#define	CMDLIN_OUT		1
#define	CMDLIN_HEAP		2
#define	CMDLIN_MMA		3

static UCHAR cmdusg[] = {
	0x86, 0x50,
		0x88, 0x8c, 
		0x03, 'h', 'e', 'a', 'p', 0x11, '#',
		0x12, 'm', 'm', 'a', 0x11, '#',
	0x40
};

void G01Main()
{
	UCHAR *fbuf = g01_bss1a1;
	int heap_siz, mmarea, fsiz, dsize, dofs, stksiz, wrksiz, entry, bsssiz;
	int heap_adr, i;
	static UCHAR sign[4] = "Hari";

	g01_setcmdlin(cmdusg);

	/* �p�����[�^�̎擾 */
	heap_siz = g01_getcmdlin_int_s(CMDLIN_HEAP);
	mmarea   = g01_getcmdlin_int_o(CMDLIN_MMA, 0);

	/* �t�@�C���ǂݍ��� */
	g01_getcmdlin_fopen_s_0_4(CMDLIN_IN);
	fsiz = jg01_fread1f_4(MAXSIZ, fbuf);

	/* �w�b�_�m�F */
	if (get32(&fbuf[4]) != 0x24) {	/* �t�@�C������.text�X�^�[�g�A�h���X */
err_form:
		g01_putstr0_exit1("bim file format error");
	}
	if (get32(&fbuf[8]) != 0x24)	/* ���������[�h����.text�X�^�[�g�A�h���X */
		goto err_form;
	dsize  = get32(&fbuf[12]);	/* .data�Z�N�V�����T�C�Y */
	dofs   = get32(&fbuf[16]);	/* �t�@�C���̂ǂ���.data�Z�N�V���������邩 */
	stksiz = get32(&fbuf[20]);	/* �X�^�b�N�T�C�Y */
	entry  = get32(&fbuf[24]);	/* �G���g���|�C���g */
	bsssiz = get32(&fbuf[28]);	/* bss�T�C�Y */

	/* �w�b�_���� */
	heap_adr = stksiz + dsize + bsssiz;
	heap_adr = (heap_adr + 0xf) & 0xfffffff0; /* 16�o�C�g�P�ʂɐ؂�グ */
	wrksiz = heap_adr + heap_siz;
	wrksiz = (wrksiz + 0xfff) & 0xfffff000; /* 4KB�P�ʂɐ؂�グ */
	put32(&fbuf[ 0], wrksiz);
	for (i = 0; i < 4; i++)
		fbuf[4 + i] = sign[i];
	put32(&fbuf[ 8], mmarea);
	put32(&fbuf[12], stksiz);
	put32(&fbuf[16], dsize);
	put32(&fbuf[20], dofs);
	put32(&fbuf[24], 0xe9000000);
	put32(&fbuf[28], entry - 0x20);
	put32(&fbuf[32], heap_adr);

	/* �t�@�C���������� */
	g01_getcmdlin_fopen_s_3_5(CMDLIN_OUT);
	jg01_fwrite1f_5(fsiz, fbuf);
	return;
}

int get32(const UCHAR *p)
{
	return p[0] | p[1] << 8 | p[2] << 16 | p[3] << 24;
}

void put32(UCHAR *p, int i)
{
	p[0] =  i        & 0xff;
	p[1] = (i >>  8) & 0xff;
	p[2] = (i >> 16) & 0xff;
	p[3] = (i >> 24) & 0xff;
	return;
}

/*

memo

[ .bim�t�@�C���̍\�� ]

+ 0 : .text�T�C�Y
+ 4 : �t�@�C������.text�X�^�[�g�A�h���X�i0x24�j
+ 8 : ���������[�h����.text�X�^�[�g�A�h���X�i0x24�j
+12 : .data�T�C�Y
+16 : �t�@�C������.data�X�^�[�g�A�h���X
+20 : ���������[�h����.data�X�^�[�g�A�h���X
+24 : �G���g���|�C���g
+28 : bss�̈�̃o�C�g��
+36 : �R�[�h

[ .hrb�t�@�C���̍\�� ]

+ 0 : stack+.data+heap �̑傫���i4KB�̔{���j
+ 4 : �V�O�l�`�� "Hari"
+ 8 : mmarea �̑傫���i4KB�̔{���j
+12 : �X�^�b�N�����l��.data�]����
+16 : .data�̃T�C�Y
+20 : .data�̏����l�񂪃t�@�C���̂ǂ��ɂ��邩
+24 : 0xe9000000
+28 : �G���g���A�h���X-0x20
+32 : heap�̈�imalloc�̈�j�J�n�A�h���X

*/
