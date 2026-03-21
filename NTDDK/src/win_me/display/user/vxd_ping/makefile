TARGET=vxd_ping.exe

all:
	cl /nologo /W3 /O2 /DWIN32 /D_CONSOLE vxd_ping.c /link /out:$(TARGET)

clean:
	del /q *.obj *.exe 2>nul
