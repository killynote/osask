#//include ../include/rules.mak

TOOLPATH = ../z_tools/

INCPATH = $(TOOLPATH)win32/

TARGET = osalink1
MODE = exe


LD = $(TOOLPATH)ld.exe
NASK = $(TOOLPATH)nask.exe
GAS2NASK = $(TOOLPATH)gas2nask.exe
CC1 = $(TOOLPATH)cc1.exe
SJISCONV = $(TOOLPATH)sjisconv.exe
DEL = del



all : osalink1.exe


$(TARGET).$(MODE) : $(TARGET).obj
	$(LD) -s -Bdynamic --stack 0x1000000 -o $@ -Map $(TARGET).map $< $(INCPATH)w32clibc.lib $(INCPATH)golibc.lib $(INCPATH)libmingw.lib

#		18MB(apl) + 16MB(sys)




$(TARGET).obj : $(TARGET).nas
	$(NASK) $< $(TARGET).obj



$(TARGET).nas : $(TARGET).gas
	$(GAS2NASK) -a $< $@


$(TARGET).gas : $(TARGET).ca
	$(CC1) -I$(INCPATH) -Dmain=main0 -Os -Wall -quiet -o $@ $<



$(TARGET).ca : $(TARGET).c
	$(SJISCONV) -s $< $@





clean :
	-$(DEL) *.ca
	-$(DEL) *.gas
	-$(DEL) *.nas
	-$(DEL) *.map
	-$(DEL) *.obj
	-$(DEL) *.exe
