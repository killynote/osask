include ../z_tools/command.txt

TARGET     = golibc
STACKSIZE  =
MALLOCSIZE =
MMAREA     =
OBJS       = abs.obj atof.obj atoi.obj errno.obj frexp.obj ldexp.obj \
	memchr.obj memcmp.obj memmove.obj memset.obj qsort.obj rand.obj \
	sprintf.obj strcat.obj strcmp.obj strcpy.obj strcspn.obj strdup.obj \
	strlen.obj strncat.obj strncmp.obj strncpy.obj strpbrk.obj strrchr.obj \
	strspn.obj strstr.obj strtol.obj strtoul.obj strtoul0.obj vsprintf.obj

# 以上5つはソースごとに書き換える
# OBJSを書き換えると分割コンパイル対応

INCPATH  = .


# 以上の項目はあなたのディレクトリ構成にあわせて書き換える

ALL :
	$(MAKE) $(TARGET).lib

%.ca : %.c Makefile
	$(SJISCONV) in:$*.c out:$*.ca

%.gas : %.ca Makefile
	$(CC1)  -I$(INCPATH) -Os -quiet -W -Wall -Werror -o $*.gas $*.ca

%.nas : %.gas Makefile
	$(GAS2NASK) $*.gas $*.nas

%.obj : %.nas Makefile
	$(NASK) $*.nas $*.obj

%.ias : %.ask Makefile
	$(CPP0) -o $*.ias $*.ask

%.3as : %.ias Makefile
	$(ASKA) $*.ias $*.3as

%.nas : %.3as Makefile
	$(NASKCNV) $*.3as $*.nas

%.lst : %.nas Makefile
	$(NASK) $*.nas $*.obj $*.lst

$(TARGET).bim : $(OBJS) Makefile
	$(OBJ2BIM) @$(RULEFILE) out:$(TARGET).bim stack:$(STACKSIZE) map:$(TARGET).map $(OBJS)

%.bin : %.bim Makefile
	$(BIM2BIN) in:$*.bim out:$*.org malloc:$(MALLOCSIZE) mmarea:$(MMAREA)
	$(BIM2BIN) in:$*.org out:$*.bin -osacmp -tek0

$(TARGET).lib : $(OBJS) Makefile
	$(GOLIB) out=$@ $(OBJS)

clean :
	-$(DELE) *.obj
	-$(DELE) $(TARGET).bim
	-$(DELE) $(TARGET).map
	-$(DELE) $(TARGET).org
	-$(DELE) $(TARGET).lib
