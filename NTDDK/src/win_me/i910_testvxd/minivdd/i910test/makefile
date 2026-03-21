VXDNAME = I910TEST
SOURCES = i910test.asm i910test.c pci.c mmio.c ioctl.c log.c

ASM      = $(MASM_ROOT)\bin\ml
MYCC     = $(C32_ROOT)\bin\cl
LINK32   = $(C32_ROOT)\bin\link

VXDDIR = ..\..\..\display\minivdd

MYAFLAGS = -coff -DBLD_COFF -nologo -W2 -Zd -c -Cx -DMASM6 -Sg -DMINIVDD=1 -DWIN98
MYAFLAGS = $(MYAFLAGS) -I$(DDKROOT)\inc\win_me -I$(VXDDIR)\inc
OVERRIDE_AVXDFLAGS = $(MYAFLAGS)

MYCFLAGS = -D_X86_ -DBLD_COFF -G5fyz -W3 -Gs -nologo -Zp1 -Zl -c -Zi -DWIN98
MYCFLAGS = $(MYCFLAGS) -DWANTVXDWRAPS -DWIN32 -DMINIVDD -DIS_32 -Oi
MYCFLAGS = $(MYCFLAGS) -I$(DDKROOT)\inc\win_me -I$(VXDDIR)\inc
OVERRIDE_CVXDFLAGS = $(MYCFLAGS)

VXDLIBS = $(DDKROOT)\lib\win_me\vxdwraps.clb
!include ..\..\..\display\minivdd\mini_vdd.mk
