[INSTRSET "i486p"]
[BITS 32]

	DB		'G',0x01
	DB		0x08, 0x9a, 0x00
;	CALL	[ESI]
	ORG		2
	DB		0x55, 0x16, 0x00
	INC		EAX
	CMP		AL,0x7f
	JB		0

;	DB		'G',0x01, 0x00
;
;	ORG		0
;	MOV		AL,0x20
;do:
;	CALL	[ESI]
;	DB		0x50, 0x15, 0x60, 0x03
;	INC		EAX
;	CMP		AL,0x7f
;	JB		do

;	�w�b�_3
;	EAX=7f ECX=5f -> �����3.5�܂�4
;	DB		0x50, 0x15, 0xbb, 0x01, 0x30
;	LOOP	-2
; = 14

;	�w�b�_3
;	EAX=20 -> �����1.5�܂�2
;	DB		0x50, 0x15, 0x60, 0x03
;	INC		EAX
;	CMP		AL,0x7f
;	JB		do
; = 14
