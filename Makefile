CC = gcc
WINDRES = windres

CFLAGS = -mwindows -O2 -lcomctl32 -lshlwapi -ldwmapi -lgdi32 -luxtheme -lshell32

all: siegexSwitcher.exe

appicon.res: appicon.rc appicon.ico
	$(WINDRES) appicon.rc -O coff -o appicon.res

siegexSwitcher.exe: siegexSwitcher.c appicon.res
	$(CC) siegexSwitcher.c appicon.res -o siegexSwitcher.exe $(CFLAGS)

clean:
	del /Q siegexSwitcher.exe appicon.res

.PHONY: all clean
