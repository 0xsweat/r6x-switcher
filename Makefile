CC = gcc
WINDRES = windres

CFLAGS = -mwindows -O2 -lcomctl32 -lshlwapi -ldwmapi -lgdi32 -luxtheme -lshell32

all: r6x-switcher.exe

appicon.res: appicon.rc appicon.ico
	$(WINDRES) appicon.rc -O coff -o appicon.res

r6x-switcher.exe: r6x-switcher.c appicon.res
	$(CC) r6x-switcher.c appicon.res -o r6x-switcher.exe $(CFLAGS)

clean:
	del /Q r6x-switcher.exe appicon.res

.PHONY: all clean
