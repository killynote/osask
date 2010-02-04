/* "osalink1.c":OSASK LINK�v���O���� version 2.0

	�ŏ���BASE.EXE�A���̌�͊e��.BIN(.TEK)��z�肵�Ă��� */



#include <stdio.h>
#include <stdlib.h>

#define	OPTIONFILE	"OSALINK1.OPT"
#define	OUTPUTFILE	"OSASK.EXE"
#define	BUFSIZE		2 * 1024 * 1024


//�֐��̃v���g�^�C�v�錾
char tolower_hide(char moto);
const int script(char *opt, char *inp, char *out, char *helmes_buf);


void wordstore(char *p, const int w)
{
	p[0] =  w       & 0xff;
	p[1] = (w >> 8) & 0xff;
	return;
}

void dwordstore(char *p, const int d)
{
	p[0] =  d        & 0xff;
	p[1] = (d >>  8) & 0xff;
	p[2] = (d >> 16) & 0xff;
	p[3] = (d >> 24) & 0xff;
	return;
}

const int wordload(const unsigned char *p)
{
	return p[0] | p[1] << 8;
}

const int dwordload(const unsigned char *p)
{
	return p[0] | p[1] << 8 | p[2] << 16 | p[3] << 24;
}


const int main(int argc, char **argv)
{
  //�o�[�W�����\�L
  if ( argc == 2 ){
    if ( strcmp(argv[1],"-v") == 0 ){
      fprintf(stderr, "osalink1 hideyosi version 1.1\n");
      return 0;
    }
  }

	FILE *fp0, *fp1;
	int i, i2,j, k, size, totalsize = 0;
	unsigned char fname[32], name[8], c;
	unsigned char fname2[32];


	unsigned char buf0[2*1024*1024];

	unsigned char *buf = buf0, *buf1, *buf2, *buf3;

	unsigned char *optfile = OPTIONFILE, *outfile = OUTPUTFILE;

	if (argc == 4)
		return script(argv[1], argv[2], argv[3], buf0);
	if (argc >= 2)
		optfile = argv[1];
	if (argc >= 3)
		outfile = argv[2];

	// OPTIONFILE�̓ǂݍ��݂Ɗe�t�@�C���̃T�C�Y�擾
	fp0 = fopen(optfile, "r");
	if (fp0 == NULL) {
		fprintf(stderr, "Can't open \"%s\".\n", optfile);
		return 1;
	}

	for (i = 0; fgets(fname,32,fp0) != NULL; i++) {

	  //���s�R�[�h�̈���������
	  for ( i2 = 31; i2 != 0; i2--){
	    if ( fname[i2] == 0x0a ) fname[i2] = 0x0;
	    if ( fname[i2] == 0x0d ) fname[i2] = 0x0;
	  }

	  //��������E�E�E��������ăR�s�[����Ƃ��܂������E�E�E
	  strcpy (fname2,fname);

		fp1 = fopen(fname2, "rb");
		if (fp1 == NULL) {
err1:
			fclose(fp0);
			fprintf(stderr, "Can't open \"%s\".\n", fname2);
			return 1;
		}
			fprintf(stderr, "reading %s \n", fname2);

		buf1 = buf + totalsize;
		size = fread(buf + totalsize, 1, BUFSIZE - totalsize, fp1);
		fclose(fp1);
		if (size <= 0)
			goto err1;
		if (size >= BUFSIZE - totalsize)
			goto err1;
		if (i == 0) {
			buf2 = buf + (wordload(buf + 0x204) << 4) - 16 + 0x220;
			goto skip; /* BASE.EXE�͉��H���Ȃ� */
		}

		for (j = 0; j < 8; j++)
			name[j] = ' ';
		for (j = 0; j < 8 && fname[j] != '.'; j++)
			name[j] = tolower_hide(fname[j]);
		for (buf3 = buf2; *buf3; buf3 += 16) {
			c = 0;
			for (j = 0; j < 8; j++)
				c |= name[j] ^ buf3[j];
			if (c)
				continue;
			j = size;
			if (size > 20) {
			//	c = 0;
				for (k = 1; k < 16; k++)
					c |= buf1[k] ^ "\x82\xff\xff\xff\x01\x00\x00\x00OSASKCMP"[k];
				if (c == 0) {
					size -= 20;
					j = dwordload(buf1 + 16); /* �W�J��̃T�C�Y */
					for (k = 0; k < size; k++)
						buf1[k] = buf1[k + 20];
					buf3[0x0f] = 0x80; /* ���k�t���O�𗧂Ă� */
				}
			}
			dwordstore(buf3 + 0x08, j);
			wordstore(buf3 + 0x0c, (totalsize - 0x200) >> 4);
			break;
		}
skip:
		totalsize += size;
		while (totalsize & 0x0f)
			*(buf + totalsize++) = '\0'; /* �p���O���t�P�ʂ̃A���C�� */
	}
	fclose(fp0);

	/* �w�b�_���� */
	wordstore(buf + 0x02, totalsize & 0x01ff); // �ŏI�y�[�W�T�C�Y
	wordstore(buf + 0x04, (totalsize + 0x1ff) >> 9); // �t�@�C���y�[�W��
	wordstore(buf + 0x0e, (totalsize - 0x200) >> 4); // ����SS

	if (buf[0x0208] == 0x10 && buf[0x0209] == 0x89 && buf[0x020a] == 0x00) {
		/* OSASK��KHBIOS�p�X�N���v�g�𔭌� */
		size = (totalsize + (0x1ff - 0x200)) >> 9;
		buf[0x020b] =  size       & 0xff;
		buf[0x020c] = (size >> 8) & 0xff;
	}

	/* �o�� */
	fp1 = fopen(outfile, "wb");
	fwrite(buf, 1, totalsize, fp1);
	fclose(fp1);
	return 0;
}

char *skipspace(char *s, char *s1)
{
retry:
	while (s < s1 && *s <= ' ')
		s++;
	if (s + 1 < s1 && *s == '/' && *(s + 1) == '*') {
		s += 2;
		do {
			while (s < s1 && *s != '*')
				s++;
			if (s + 1 >= s1)
				return s1;
			s += 2;
		} while (*(s - 1) != '/');
		goto retry;
	}
	return s;
}

char *checktoken(char *s, char *s1)
/* ������ĂԑO��skipspace���Ă������� */
{
	char *s0 = s;
	if (s < s1) {
		do {
			char c = *s;
			if (c <= ' ')
				break;
			if (c == ',')
				break;
			if (c == '(')
				break;
			if (c == ')')
				break;
			if (c == '{')
				break;
			if (c == '}')
				break;
			if (c == ';')
				break;
			if (c == '\x22')
				break;
			s++;
		} while (s < s1);
		if (s == s0)
			s++;
	}
	return s;
}

int getnum(char *s, char *s1)
/* 10�i����16�i���̂�, ���������̒l�͈���Ȃ� */
/* �G���[����-1��Ԃ� */
/* s��skipspace()���Ă������� */
{
	int i = 0, base = 10, c;
	if (s >= s1)
		goto err;
	if (s + 2 < s1 && *s == '0' && tolower_hide(*(s + 1)) == 'x') {
		s += 2;
		base = 16;
	}
	do {
		c = tolower_hide(*s++);
		if (c < '0')
			goto err;
		c -= '0';
		if (c <= 9)
			goto skip;
		if (c < 'a' - '0')
			goto err;
		c -= 'a' - '0' - 10;
		if (c >= base)
			goto err;
	skip:
		i = i * base + c;
	} while (s < s1);
	return i;
err:
	return -1;
}

const char cmptoken(char *s, char *s1, char *t)
{
	do {
		if (s >= s1)
			break;
		if (*s++ != *t)
			break;
		t++;
	} while (*t);
	return *t;
}

char *get2prm(char *s, char *scr1, int *p0, int *p1)
/* "(p0, p1," */
{
	char *s1;

	s = skipspace(s, scr1);
	if (s >= scr1)
		goto err;
	s1 = checktoken(s, scr1);
	if (cmptoken(s, s1, "(") != 0)
		goto err;
	s = skipspace(s1, scr1);
	if (s >= scr1)
		goto err;
	s1 = checktoken(s, scr1);
	if ((*p0 = getnum(s, s1)) < 0)
		goto err;
	s = skipspace(s1, scr1);
	if (s >= scr1)
		goto err;
	s1 = checktoken(s, scr1);
	if (cmptoken(s, s1, ",") != 0)
		goto err;
	s = skipspace(s1, scr1);
	if (s >= scr1)
		goto err;
	s1 = checktoken(s, scr1);
	if ((*p1 = getnum(s, s1)) < 0)
		goto err;
	s = skipspace(s1, scr1);
	if (s >= scr1)
		goto err;
	s1 = checktoken(s, scr1);
	if (cmptoken(s, s1, ",") != 0)
		goto err;
	s = skipspace(s1, scr1);
	if (s >= scr1)
		goto err;
	return s;

err:
	return NULL;
}

char *get3prm(char *s, char *scr1, int *p0, int *p1, int *p2)
{
	char *s1;

	s = get2prm(s, scr1, p0, p1);
	if (s == NULL)
		goto err;
	s1 = checktoken(s, scr1);
	if ((*p2 = getnum(s, s1)) < 0)
		goto err;
	s = skipspace(s1, scr1);
	if (s >= scr1)
		goto err;
	s1 = checktoken(s, scr1);
	if (cmptoken(s, s1, ")") != 0)
		goto err;
	s = skipspace(s1, scr1);
	if (s >= scr1)
		goto err;
	s1 = checktoken(s, scr1);
	if (cmptoken(s, s1, ";") != 0)
		goto err;
	return s1;

err:
	return NULL;
}

const int script(char *opt, char *inp, char *out, char *helmes_buf)
/* �X�N���v�g��4KB�𒴂����玀�ɂ܂� */
{
	unsigned char *buf = helmes_buf + 4 * 1024, *scr0 = helmes_buf, *scr1, *s, *s1;
	FILE *fp;
	int size, memofs, filofs;

	fp = fopen(opt, "r");
	if (fp == NULL)
		goto err;
	scr1 = scr0 + fread(scr0, 1, 4 * 1024, fp);
	fclose(fp);

	s = scr0;
	for (;;) {
		s = skipspace(s, scr1);
		if (s >= scr1)
			break;
		s1 = checktoken(s, scr1);
		if (cmptoken(s, s1, ";") == 0)
			continue;
		if (cmptoken(s, s1, "load") == 0) {
			s = get3prm(s1, scr1, &size, &memofs, &filofs);
			if (s == NULL)
				goto err;
			fp = fopen(inp, "rb");
			if (fp == NULL)
				goto err;
			fseek(fp, filofs, SEEK_SET);
			if (fread(buf + memofs, 1, size, fp) != size)
				goto err;
			fclose(fp);
			continue;
		}
		if (cmptoken(s, s1, "store") == 0) {
			s = get3prm(s1, scr1, &size, &memofs, &filofs);
			if (s == NULL)
				goto err;
			fp = fopen(out, "wb");
			if (fp == NULL)
				goto err;
			fseek(fp, filofs, SEEK_SET);
			if (fwrite(buf + memofs, 1, size, fp) != size)
				goto err;
			fclose(fp);
			continue;
		}
		if (cmptoken(s, s1, "fill") == 0) {
			s = get3prm(s1, scr1, &size, &memofs, &filofs);
			if (s == NULL)
				goto err;
			while (size > 0) {
				buf[memofs++] = (char) filofs;
				size--;
			}
			continue;
		}

#if 0
		if (cmptoken(s, s1, "if") == 0) {

			s = skipspace(s1, scr1);
			if (s >= scr1)
				goto err;
			s1 = checktoken(s, scr1);
			if (cmptoken(s, s1, "(") != 0) {
				goto err;
			s = skipspace(s1, scr1);
			if (s >= scr1)
				goto err;

		}
#endif

err:
		return 1;
	}
	return 0;
}




char tolower_hide(char moto){
    if ( (moto > 0x41) && (moto < 0x5a ) ){
        return moto + 0x20;
    }
    else{
       return moto;
    }
}
