; malloc, free  -  K&R��ASM���������̂��炿����Ƃ�������

[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[OPTIMIZE 1]
[OPTION 1]
[BITS 32]
[FILE 'malloc.nas']
GLOBAL _malloc
GLOBAL _free
GLOBAL __gg00_malloc_addr0
GLOBAL __gg00_malloc_base

[SECTION .data]
	ALIGNB	4
__gg00_malloc_addr0	dd	0
__gg00_malloc_base	dd	__gg00_malloc_base,0 ; ptr, size

[SECTION .text]

;void *malloc(unsigned int nbytes)

_malloc:

	push	esi
	push	ebx
	push	edx
	push	ecx
	mov	esi,__gg00_malloc_base
loop1:
	mov	ecx,ss:[esp+20] ; nbytes
;	mov	eax,__gg00_malloc_base
	mov	eax,esi
	add	ecx,8-1+8
	and	ecx,0fffffff8h
	; base�̓T�C�Y������Ȃ��̂Ő�΂Ɉ�v���Ȃ�
loop0:
	cmp	ecx,dword ds:[eax+4]	; p->size
	jbe	skip1	; find
	mov	edx,eax
	mov	eax,dword ds:[eax+0]	; p->ptr
	cmp	eax,esi
	jne	loop0

;	���̃o�[�W�����ł́Asbrk���������Ă���

	xor	eax,eax
;	cmp	eax,dword ds:[addr0]
	cmp	eax,dword ds:[esi-4]
	jne	ret0
	mov	edx,dword cs:[eax+0020h] ; ������A�X�^�e�B�b�N�f�[�^�T�C�Y
	add	edx,dword cs:[eax+0024h] ; ����������A�X�^�e�B�b�N�f�[�^�J�n�A�h���X
	add	edx,7
	mov	ecx,dword cs:[eax+0010h] ; DS�T�C�Y
	and edx,0fffffff8h
;	mov	dword ds:[__gg00_malloc_addr0],ecx
	mov	dword ds:[esi-4],ecx
	sub	ecx,edx
	mov	dword ds:[edx+4],ecx
	add	edx,8
	push	edx
	call	_free	; ���W�X�^��j�󂵂Ȃ��̂ŕۑ����Ă��Ȃ�
	pop	eax
	jmp	loop1
skip1:
	mov	ebx,dword ds:[eax+0]	; ptr (edx, eax, ebx)
	je	skip2
	mov	esi,dword ds:[eax+4]	; size
	lea	ebx,[eax+ecx]
	mov	dword ds:[eax+4],ecx
	sub	esi,ecx
	mov	ecx,dword ds:[eax+0]	; ptr
	mov	dword ds:[ebx+4],esi
	mov	dword ds:[ebx+0],ecx
skip2:
	mov	dword ds:[edx+0],ebx
	add	eax,8
ret0:
	pop	ecx
	pop	edx
	pop	ebx
	pop	esi
	ret

;void free(void *ap)

_free:

	pushad
	mov	eax,dword ss:[esp+32+4]	; ap
	sub	eax,8
	jb	skip23
	mov	esi,__gg00_malloc_base
;	mov	ebx,esi
	mov	edx,esi
	mov	ebx,dword ds:[esi+0]
	cmp	ebx,esi
	je	skip21
	cmp	eax,ebx
	jb	skip21
loop20:
	mov	edx,ebx
	mov	ebx,dword ds:[ebx+0]
	cmp	ebx,esi
	je	skip21
	cmp	eax,ebx
	ja	loop20
	; edx, eax, ebx

skip21:
	mov	ecx,edx
	mov	dword ds:[edx+0],eax
	add	ecx,dword ds:[edx+4]
	mov	dword ds:[eax+0],ebx
	cmp	eax,ecx
	jne	skip22
	mov	ecx,dword ds:[eax+4]
	mov	dword ds:[edx+0],ebx
	add	dword ds:[edx+4],ecx
	xchg	eax,edx
skip22:
	mov	ecx,eax
	add	ecx,dword ds:[eax+4]
	cmp	ecx,ebx
	jne	skip23
	mov	ecx,dword ds:[ebx+4]
	mov	ebx,dword ds:[ebx+0]
	add	dword ds:[eax+4],ecx
	mov	dword ds:[eax+0],ebx
skip23:
	popad
	ret
