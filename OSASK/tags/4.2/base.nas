;	"base.nas" ver.2.8
;	OSASK用のブートプログラム
;	Copyright(C) 2003 H.Kawai (川合秀実)

;	TAB = 4

[BITS 16]
[OPTIMIZE 1]
[OPTION 1]
[INSTRSET "i486p"]
[FORMAT "BIN"]

BootMdl		EQU		0
SysWorkMdl	EQU		(BootSiz + 0x0f) / 16 + BootMdl
StackMdl	EQU		0 ; (osalink0が調節してくれるので気にしない)

; <header> 30bytes.  0x0000 - 0x001d

header:

	DB	"MZ"
	DW	0, 0 ; 最終ページサイズ、ファイルページ数 (osalink0が調節してくれるので気にしない)
	DW	(relent1-relent0)/4 ; relocation entries
	DW	0x20 ; header-size / 16
	DW	StackSiz / 16 ; BSS-size / 16 (== stack-size / 16)
	DW	0xffff ; MAXALLOC
	DW	0 ; DOS_SS0 (osalink0が調節してくれるので気にしない)
	DW	StackSiz ; DOS_SP0
	DW	0 ; check-sum (ignore)
	DW	0 ; DOS_IP0 (entry-offset)
	DW	0 ; DOS_CS0 (entry-seg, relative)
	DW	0x001e ; relocation table offset
	DW	0 ; not use overlay
	DW	1 ; "I don't know."

; <relocation table image> 0x001e - 0x01ff
; org 0x001e

relent0:
; DW ofs, seg
	DW	BootMdl*16+4, 0 ; for SysWorkMdl
	DW	SysWorkMdl*16+modulelist+ 0*16+12, 0
	DW	SysWorkMdl*16+modulelist+ 1*16+12, 0
	DW	SysWorkMdl*16+modulelist+ 2*16+12, 0
	DW	SysWorkMdl*16+modulelist+ 3*16+12, 0
	DW	SysWorkMdl*16+modulelist+ 4*16+12, 0
	DW	SysWorkMdl*16+modulelist+ 5*16+12, 0
	DW	SysWorkMdl*16+modulelist+ 6*16+12, 0
	DW	SysWorkMdl*16+modulelist+ 7*16+12, 0
	DW	SysWorkMdl*16+modulelist+ 8*16+12, 0
	DW	SysWorkMdl*16+modulelist+ 9*16+12, 0
	DW	SysWorkMdl*16+modulelist+10*16+12, 0
	DW	SysWorkMdl*16+modulelist+11*16+12, 0
	DW	SysWorkMdl*16+modulelist+12*16+12, 0
	DW	SysWorkMdl*16+modulelist+13*16+12, 0
	DW	SysWorkMdl*16+modulelist+14*16+12, 0
	DW	SysWorkMdl*16+modulelist+15*16+12, 0
	DW	SysWorkMdl*16+modulelist+16*16+12, 0
	DW	SysWorkMdl*16+modulelist+17*16+12, 0
relent1:

	RESB	header+512-$

	ORG		0x0000 ; NASKは何回でもORGできる

BootBgn:
Entry:
	JMP		.entry2 ; 2バイト

	RESB	4-$ ; MASMORG(0x0004)

.SysWorkSeg:
	DW		SysWorkMdl

	RESB	8-$ ; MASMORG(0x0008)

; KHBIOS用スクリプト
	DB		0x10,0x89,0x00,0xd8,0x00,0x00
	DD		0x8000,36
	DB		0x1a
	DW		0x0000,0x0800

.entry2:
	MOV		 SI, DS
	MOV		 DS, WORD [CS:.SysWorkSeg]
	MOV		 AX, CS
	ADD		 SI,16
	CMP		 AX, SI
	#if (defined(PCAT))
		MOV		ECX,(640-4)*1024 ; for PCAT
	#elif (defined(TOWNS))
		MOV		ECX,(640+128)*1024 ; for TOWNS
	#elif (defined(NEC98))
		MOV		ECX,640*1024 ; for NEC98
	#endif
	JE		.fromdos
	MOV		 BYTE [DiskCacheReady],3
	MOV		 AX,4096/16

.fromdos:
	SHL		EAX,4
	ADD		EAX,0xfff;
	AND		EAX,0xff000	; 4KB単位にする
	MOV		DWORD [bootmalloc_adr1],EAX
	SUB		ECX,EAX
	MOV		DWORD [bootmalloc_fre1],ECX

	#if (defined(PCAT))
		MOV		 AH, 0x02
		INT		0x16 ; keyboard BIOS
		SHR		 AL,4
		AND		 AL,0x07
		MOV		BYTE [boot_keylock],AL
	#elif (defined(NEC98))
		MOV		CH,0xC0
		MOV		AH,0x42
		INT		0x18 ; 400ライングラフィックモード(BIOS)
		MOV		AH,0x40
		INT		0x18 ; グラフィック画面表示(BIOS)
		MOV		AH,0x0d
		INT		0x18 ; テキスト画面非表示(BIOS)
		MOV		AL,0xff
		OUT		0x0002,AL ; 割り込みマスク
		MOV		AH,0x03
		INT		0x18 ; キーボード初期化
	#endif

	MOV		AX,[CS:0x0002]
	MOV		[CFport],AX

	#if (defined(PCAT))
		#if (defined(BOCHS))
			mov	 ax, word ds:[VGA_mode]
			int	10h
		#else
		;	mov	 ax, word ds:[VGA_mode]
		;	int	10h
		#endif
	;mov dx,03d4h
	;mov ax,3213h
	;out dx,ax
	#endif

;	ここでA20を有効にする

	CLI

	#if (defined(PCAT))
		CALL	waitkbdout
		MOV		 AL,0xae	; keyboard interface enable
		OUT		0x64, AL
		CALL	waitkbdout
		MOV		 AL,0x60	; write mode command
		OUT		0x64, AL
		CALL	waitkbdout
		MOV		 AL,0x47	; 下記参照
		OUT		0x60, AL
	;		// IRQ01 enable, IRQ12 enable, system flag on, keylock enable
	;		// keyboard interface enable, mouse interface enable, scan code 1(PC format)
		CALL	waitkbdout
		MOV		 AL,0xd1	; 
		OUT		0x64, AL
		CALL	waitkbdout
		MOV		 AL,0xdf	; enable A20
		OUT		0x60, AL
		CALL	waitkbdout
		MOV		 AL,0xa8	; mouse interface enable
		OUT		0x64, AL
		CALL	waitkbdout

	;.wait03:
	;	MOV		ECX,0xaa55aa55
	;	MOV		EDX,0x55aa55aa
	;	MOV		 AX,0
	;	MOV		 FS, AX
	;	MOV		 AX,0xff00
	;	MOV		 GS, AX
	;	MOV		EAX,[FS:0]
	;	MOV		[GS:0x1000],EDX
	;	MOV		[FS:0],ECX
	;	CMP		ECX,[GS:0x1000]
	;	MOV		[FS:0],EAX
	;	JE		.wait03

	#elif (defined(NEC98))
		OUT		0xf2, AL
	#endif

;	AC = 1となるので、SPをdwordアラインしておくこと

;	CLI		; IDTが設定されるまで、割り込みを禁止する

	MOV		 AX,0xff00
	MOV		 ES, AX
	MOV		 SS, AX ; これ以降、スタック操作は禁止

	MOV		DWORD [ES:0x2000],0x102007	; 面倒なのでPDE,PTEもキャッシュさせてしまう
;	MOV		DWORD [ES:0x2004],0x103007	;   どうせ仮のものなのでどうってことはない
	MOV		EAX,0x101000
	MOV		CR3,EAX
	MOV		EAX,7
	MOV		DI,0x3000
	MOV		 CX,3*1024/4 ; 3MB
	CLD
.fillpte
	STOSD
	ADD		EAX,4096
	LOOP	.fillpte

;	to protect mode

	MOV		EAX,CR0
	OR		EAX,10000000_00000000_00000000_00001101b ; PG,TS,PE
	AND		EAX,10011111_11111010_11111111_11111011b ; CD,NW,AM,WP,EM
	MOV		CR0,EAX	; モード移行
	JMP		..$	; for pipeline-flash

;	GDTにDTを作る

;	int boot32_sel    == 2 * 8, boot32_ent == 1 * 16;
.boot32_sel	EQU		21 * 8
.boot32_ent	EQU		16 * 16 + modulelist

	MOV		 BP,0x1000+384+.boot32_sel
	MOV		 AX, WORD [.boot32_ent+12] ; boot32 seg
	SHL		EAX,4
	AND		EAX,0xffff0
	MOV		 WORD [BP+2], AX
	SHR		EAX,16
	MOV		 BYTE [BP+4], AL
	MOV		 BYTE [BP+7], AH
	MOV		 AX, WORD [.boot32_ent+ 8]
	DEC		 AX
	MOV		 WORD [BP+0], AX
	MOV		 BYTE [BP+5],10011010b	; ER
	MOV		 BYTE [BP+6],01000000b	; use32

	LGDT	[gdt0+2]

	JMP		FAR DWORD .boot32_sel:0

#if (defined(PCAT))
waitkbdout:
	IN		 AL,0x64
	TEST	 AL,0x02
	IN		 AL,0x60 ; から読み(受信バッファが悪さをしないように)
	JNZ		waitkbdout
	RETN
#endif

BootSiz		EQU		$ - BootBgn

			RESB	(16 - ($ % 16)) % 16

			ORG		0x0000

SysWorkBgn:

VESA_busdevfnc	DD	0 ; 80 bus dev-func ofs-reg
VESAPNP_00		DD	0
VESAPNP_08		DD	0
VESAPNP_2c		DD	0

VGA_mode		DW	12h	; +0x10
				DW	0
#if (defined(BOCHS) | defined(VMWARE))
to_winman0		DD	1 ; +0x14
#else
to_winman0		DD	0 ; +0x14
#endif

CFport			DD	0	; +0x18
eflags			DD	0

		;	align	16

modulelist:
	DD	"syswork ",SysWorkSiz,SysWorkMdl	;  0 * 16
	DD	"init    ",0,BootMdl	;  1 * 16
	DD	"vgadrv0 ",0,BootMdl	;  2 * 16
	DD	"keymos0 ",0,BootMdl	;  3 * 16
	DD	"timerdrv",0,BootMdl	;  4 * 16
	DD	"tapi0   ",0,BootMdl	;  5 * 16
	DD	"decode0 ",0,BootMdl	;  6 * 16
	DD	"fdcdrv0 ",0,BootMdl	;  7 * 16
	DD	"pioneer0",0,BootMdl	;  8 * 16
	DD	"winman0 ",0,BootMdl	;  9 * 16
	DD	"pokon0  ",0,BootMdl	; 10 * 16
	DD	"vesa8   ",0,BootMdl	; 11 * 16
	DD	"ankfont0",0,BootMdl	; 12 * 16 256バイトだけ必要
	DD	"papi0   ",0,BootMdl	; 13 * 16
	DD	"vesa16  ",0,BootMdl	; 14 * 16
	DD	"vesa32  ",0,BootMdl	; 15 * 16
	DD	"boot32  ",0,BootMdl	; 16 * 16 これは必ず非圧縮
	DD	"fdimage ",0,BootMdl	; 17 * 16

loaded_modules	EQU		($ - modulelist) / 16

alloclist:
#if (defined(PCAT))
	DD	"pdepte  ",4096*70,-1	; 0
#else
	DD	"pdepte  ",4096*67,-1	; 0
#endif
	DD	"idtgdt  ",4096,-1	; 48+463entry
	DD	"freeslot",0,0
	DD	"stack   ",4096*4,-1
	DD	"keydata ",4096,-1	; 4
	DD	"gapidata",4096*176,-1   ; 起動最低限しか用意しない
	DD	"timerdat",4096*2,-1
	DD	"tapiwork",0,0 ; for 31 tasks(init, idle, winman0, pokon0).
	DD	"decodata",4096*6,-1	; 8
	DD	"fdcwork ",4096,-1
	DD	"papiwork",4096*8,-1

	DD	0

bootmalloc_adr0		DD	0x1000	; (+0x01d4)
bootmalloc_fre0		DD	0	; (+0x01d8)
bootmalloc_adr1		DD	0	; LastMdlを指す (+0x01dc)
bootmalloc_fre1		DD	0	; 636KB - bootmalloc_adr1 (+0x01e0)

FD_motor_init		DB	01ch ; motor on
FD_cache_init		DB	1 ; must init
DiskCacheReady		DB	0
	; bit0 : cache enable, bit1:boot from OSASK boot-sector
boot_keylock		DB	0
FD_debug			DD	-1

bootmalloc_adr2		DD	1024 * 1024
bootmalloc_fre2		DD	0
bootmalloc_adr3		DD	16 * 1024 * 1024
bootmalloc_fre3		DD	0
bootlinear			DD	0x480000 ; 4.5MB
bootlinear_EDI		DD	0
bmodule_size		DD	0
bmodule_paddr		DD	0
bmodule_laddr		DD	0
DiskCacheLength		DD	0
gdt0				DD	(4096 - 384 - 1)*0x10000, 0x100180
mscount				DD	0, 0

SysWorkSiz	EQU		$ - SysWorkBgn

			RESB	(16 - ($ % 16)) % 16

StackSiz	EQU		256

END
