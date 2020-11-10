#makefile for REXXCOMP

# Ss  - allow //
# Sm  - use migration libs
# Ti+ - Debug stuff
# W1  - severe warning level
# Gm+ - Multi-thread
# C   - Compile only
# K   - errors
# Q+  - turn off logo
# Rn  - develop subsystem (no RTL)
# Ge- - make a dll
# Ge+ - make an exe

#------------------------------------------------------------------------------
# C parameters
#------------------------------------------------------------------------------
CC         = icc
OPTS       =  /c /Q+ /Kbperc /Ss /W1
MIG        =  /Sm
SUB        =  /Rn
LOPTS      =  /A:16 /NOI /nologo /BASE:0x10000
DLL        =  /Ge-
EXE        =  /Ge+
MT         =  /Gm+

OPTS       = $(OPTS) $(MT)

!IFDEF   opt
OPTS       = $(OPTS) /O+ /Ti-
!ELSE
OPTS       = $(OPTS) /O- /Ti+
LOPTS      = $(LOPTS) /DE
!ENDIF

all: rexxcomp.dll lzss.exe

#------------------------------------------------------------------------------
# COMPRESSION code
#------------------------------------------------------------------------------
lzss_dll.obj: lzss.c lzss.h
    $(CC) $(DLL) $(OPTS) $(MIG) /Folzss_dll lzss.c

lzss_exe.obj: lzss.c lzss.h
    $(CC) $(EXE) $(OPTS) $(MIG) /DFOR_EXE /Folzss_exe lzss.c

crc32dll.obj: crc32.c crc32.h
    $(CC) $(DLL) $(OPTS) /Focrc32dll crc32.c

crc32exe.obj: crc32.c crc32.h
    $(CC) $(EXE) $(OPTS) /Focrc32exe crc32.c

#------------------------------------------------------------------------------
# REXXCOMP DLL
#------------------------------------------------------------------------------

rexxcomp.obj: rexxcomp.c
    $(CC) $(DLL) $(MIG) $(OPTS) rexxcomp.c

rexxcomp.dll: rexxcomp.obj rexxcomp.def lzss_dll.obj crc32dll.obj
   link386 $(LOPTS) rexxcomp+crc32dll+lzss_dll,rexxcomp.dll,,REXX, rexxcomp.def

#------------------------------------------------------------------------------
# Standalone compression EXE
#------------------------------------------------------------------------------
main.obj: main.c main.h
    $(CC) $(EXE) $(OPTS) main.c

lzss.exe: lzss_exe.obj lzss.def crc32exe.obj main.obj
    link386 main+lzss_exe+crc32exe, lzss $(LOPTS),lzss,, lzss

