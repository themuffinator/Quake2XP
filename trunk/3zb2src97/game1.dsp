# Microsoft Developer Studio Project File - Name="3zb2bot" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=3zb2bot - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "3zb2bot.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "3zb2bot.mak" CFG="3zb2bot - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "3zb2bot - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "3zb2bot - Win32 Debug Alpha" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "3zb2bot - Win32 Release Alpha" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "3zb2bot - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "3zb2bot - x64 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "3zb2bot - x64 Debug Alpha" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "3zb2bot - x64 Release Alpha" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "3zb2bot - x64 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "3zb2bot - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\..\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\..\Debug"
# PROP Intermediate_Dir ".\Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /Zi /W3 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" PRECOMP_VC7_TOBEREMOVED /Fp".\Debug/game1.pch" /Fo".\Debug/" /Fd".\Debug/" /TC /GZ /c /GX 
# ADD CPP /nologo /MTd /Zi /W3 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" PRECOMP_VC7_TOBEREMOVED /Fp".\Debug/game1.pch" /Fo".\Debug/" /Fd".\Debug/" /TC /GZ /c /GX 
# ADD BASE MTL /nologo /D"_DEBUG" /mktyplib203 /tlb".\..\Debug\game1.tlb" /win32 
# ADD MTL /nologo /D"_DEBUG" /mktyplib203 /tlb".\..\Debug\game1.tlb" /win32 
# ADD BASE RSC /l 1033 /d "_DEBUG" 
# ADD RSC /l 1033 /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib /nologo /dll /out:"c:\Quake2\3zb2\gamex86.dll" /incremental:no /def:".\game1.def" /debug /pdb:".\..\Debug\gamex86.pdb" /pdbtype:sept /subsystem:windows /base:"0x20000000" /implib:".\..\Debug/gamex86.lib" /MACHINE:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib /nologo /dll /out:"c:\Quake2\3zb2\gamex86.dll" /incremental:no /def:".\game1.def" /debug /pdb:".\..\Debug\gamex86.pdb" /pdbtype:sept /subsystem:windows /base:"0x20000000" /implib:".\..\Debug/gamex86.lib" /MACHINE:I386

!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Debug Alpha"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\..\DebugAxp"
# PROP BASE Intermediate_Dir ".\DebugAxp"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\..\DebugAxp"
# PROP Intermediate_Dir ".\DebugAxp"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /Zi /W3 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" PRECOMP_VC7_TOBEREMOVED /Fp".\DebugAxp/game1.pch" /Fo".\DebugAxp/" /Fd".\DebugAxp/" /GZ /c /QA21164 /GX 
# ADD CPP /nologo /MTd /Zi /W3 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" PRECOMP_VC7_TOBEREMOVED /Fp".\DebugAxp/game1.pch" /Fo".\DebugAxp/" /Fd".\DebugAxp/" /GZ /c /QA21164 /GX 
# ADD BASE MTL /nologo /D"_DEBUG" /mktyplib203 /tlb".\..\DebugAxp\game1.tlb" /win32 
# ADD MTL /nologo /D"_DEBUG" /mktyplib203 /tlb".\..\DebugAxp\game1.tlb" /win32 
# ADD BASE RSC /l 1033 /d "_DEBUG" 
# ADD RSC /l 1033 /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib odbc32.lib odbccp32.lib /nologo /dll /out:"..\DebugAxp\gameaxp.dll" /incremental:no /def:".\game1.def" /debug /pdb:".\..\DebugAxp\gameaxp.pdb" /pdbtype:sept /subsystem:windows /base:"0x20000000" /implib:".\..\DebugAxp/gameaxp.lib" 
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib odbc32.lib odbccp32.lib /nologo /dll /out:"..\DebugAxp\gameaxp.dll" /incremental:no /def:".\game1.def" /debug /pdb:".\..\DebugAxp\gameaxp.pdb" /pdbtype:sept /subsystem:windows /base:"0x20000000" /implib:".\..\DebugAxp/gameaxp.lib" 

!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Release Alpha"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\..\ReleaseAXP"
# PROP BASE Intermediate_Dir ".\ReleaseAXP"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\..\ReleaseAXP"
# PROP Intermediate_Dir ".\ReleaseAXP"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /Z7 /W3 /Ob1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GF /Gy PRECOMP_VC7_TOBEREMOVED /Fp".\ReleaseAXP/game1.pch" /Fo".\ReleaseAXP/" /Fd".\ReleaseAXP/" /c /QA21164 /GX 
# ADD CPP /nologo /MT /Z7 /W3 /Ob1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GF /Gy PRECOMP_VC7_TOBEREMOVED /Fp".\ReleaseAXP/game1.pch" /Fo".\ReleaseAXP/" /Fd".\ReleaseAXP/" /c /QA21164 /GX 
# ADD BASE MTL /nologo /D"NDEBUG" /mktyplib203 /tlb".\..\ReleaseAXP\game1.tlb" /win32 
# ADD MTL /nologo /D"NDEBUG" /mktyplib203 /tlb".\..\ReleaseAXP\game1.tlb" /win32 
# ADD BASE RSC /l 1033 /d "NDEBUG" 
# ADD RSC /l 1033 /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib odbc32.lib odbccp32.lib /nologo /dll /out:"..\ReleaseAXP\gameaxp.dll" /incremental:no /def:".\game1.def" /pdb:".\..\ReleaseAXP\gameaxp.pdb" /pdbtype:sept /subsystem:windows /base:"0x20000000" /implib:".\..\ReleaseAXP/gameaxp.lib" 
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib odbc32.lib odbccp32.lib /nologo /dll /out:"..\ReleaseAXP\gameaxp.dll" /incremental:no /def:".\game1.def" /pdb:".\..\ReleaseAXP\gameaxp.pdb" /pdbtype:sept /subsystem:windows /base:"0x20000000" /implib:".\..\ReleaseAXP/gameaxp.lib" 

!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\..\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\..\Release"
# PROP Intermediate_Dir ".\Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /Z7 /Ob1 /Oi /Ot /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Gy PRECOMP_VC7_TOBEREMOVED /Fp".\Release/game1.pch" /Fo".\Release/" /Fd".\Release/" /TC /c /GX 
# ADD CPP /nologo /MT /Z7 /Ob1 /Oi /Ot /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Gy PRECOMP_VC7_TOBEREMOVED /Fp".\Release/game1.pch" /Fo".\Release/" /Fd".\Release/" /TC /c /GX 
# ADD BASE MTL /nologo /D"NDEBUG" /mktyplib203 /tlb".\..\Release\game1.tlb" /win32 
# ADD MTL /nologo /D"NDEBUG" /mktyplib203 /tlb".\..\Release\game1.tlb" /win32 
# ADD BASE RSC /l 1033 /d "NDEBUG" 
# ADD RSC /l 1033 /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib /nologo /dll /out:"c:\Quake2\3zb2\gamex86xp.dll" /incremental:no /def:".\game1.def" /pdb:".\..\Release\gamex86.pdb" /pdbtype:sept /subsystem:windows /base:"0x20000000" /implib:".\..\Release/gamex86.lib" /machine:ix86 /MACHINE:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib /nologo /dll /out:"c:\Quake2\3zb2\gamex86xp.dll" /incremental:no /def:".\game1.def" /pdb:".\..\Release\gamex86.pdb" /pdbtype:sept /subsystem:windows /base:"0x20000000" /implib:".\..\Release/gamex86.lib" /machine:ix86 /MACHINE:I386

!ELSEIF  "$(CFG)" == "3zb2bot - x64 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "$(PlatformName)\$(ConfigurationName)"
# PROP BASE Intermediate_Dir "$(PlatformName)\$(ConfigurationName)"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "$(PlatformName)\$(ConfigurationName)"
# PROP Intermediate_Dir "$(PlatformName)\$(ConfigurationName)"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /Zi /W3 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" PRECOMP_VC7_TOBEREMOVED /Fp".\Debug/game1.pch" /Fo".\Debug/" /Fd".\Debug/" /TC /GZ /c /GX 
# ADD CPP /nologo /MTd /Zi /W3 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" PRECOMP_VC7_TOBEREMOVED /Fp".\Debug/game1.pch" /Fo".\Debug/" /Fd".\Debug/" /TC /GZ /c /GX 
# ADD BASE MTL /nologo /D"_DEBUG" /mktyplib203 /tlb".\..\Debug\game1.tlb" /win32 
# ADD MTL /nologo /D"_DEBUG" /mktyplib203 /tlb".\..\Debug\game1.tlb" /win32 
# ADD BASE RSC /l 1033 /d "_DEBUG" 
# ADD RSC /l 1033 /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib /nologo /dll /out:"c:\Quake2\3zb2\gamex86.dll" /incremental:no /def:".\game1.def" /debug /pdb:".\..\Debug\gamex86.pdb" /pdbtype:sept /subsystem:windows /base:"0x20000000" /implib:".\..\Debug/gamex86.lib" /MACHINE:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib /nologo /dll /out:"c:\Quake2\3zb2\gamex86.dll" /incremental:no /def:".\game1.def" /debug /pdb:".\..\Debug\gamex86.pdb" /pdbtype:sept /subsystem:windows /base:"0x20000000" /implib:".\..\Debug/gamex86.lib" /MACHINE:I386

!ELSEIF  "$(CFG)" == "3zb2bot - x64 Debug Alpha"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "$(PlatformName)\$(ConfigurationName)"
# PROP BASE Intermediate_Dir "$(PlatformName)\$(ConfigurationName)"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "$(PlatformName)\$(ConfigurationName)"
# PROP Intermediate_Dir "$(PlatformName)\$(ConfigurationName)"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /Zi /W3 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" PRECOMP_VC7_TOBEREMOVED /Fp".\DebugAxp/game1.pch" /Fo".\DebugAxp/" /Fd".\DebugAxp/" /GZ /c /QA21164 /GX 
# ADD CPP /nologo /MTd /Zi /W3 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" PRECOMP_VC7_TOBEREMOVED /Fp".\DebugAxp/game1.pch" /Fo".\DebugAxp/" /Fd".\DebugAxp/" /GZ /c /QA21164 /GX 
# ADD BASE MTL /nologo /D"_DEBUG" /mktyplib203 /tlb".\..\DebugAxp\game1.tlb" /win32 
# ADD MTL /nologo /D"_DEBUG" /mktyplib203 /tlb".\..\DebugAxp\game1.tlb" /win32 
# ADD BASE RSC /l 1033 /d "_DEBUG" 
# ADD RSC /l 1033 /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib odbc32.lib odbccp32.lib /nologo /dll /out:"..\DebugAxp\gameaxp.dll" /incremental:no /def:".\game1.def" /debug /pdb:".\..\DebugAxp\gameaxp.pdb" /pdbtype:sept /subsystem:windows /base:"0x20000000" /implib:".\..\DebugAxp/gameaxp.lib" 
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib odbc32.lib odbccp32.lib /nologo /dll /out:"..\DebugAxp\gameaxp.dll" /incremental:no /def:".\game1.def" /debug /pdb:".\..\DebugAxp\gameaxp.pdb" /pdbtype:sept /subsystem:windows /base:"0x20000000" /implib:".\..\DebugAxp/gameaxp.lib" 

!ELSEIF  "$(CFG)" == "3zb2bot - x64 Release Alpha"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "$(PlatformName)\$(ConfigurationName)"
# PROP BASE Intermediate_Dir "$(PlatformName)\$(ConfigurationName)"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "$(PlatformName)\$(ConfigurationName)"
# PROP Intermediate_Dir "$(PlatformName)\$(ConfigurationName)"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /Z7 /W3 /Ob1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GF /Gy PRECOMP_VC7_TOBEREMOVED /Fp".\ReleaseAXP/game1.pch" /Fo".\ReleaseAXP/" /Fd".\ReleaseAXP/" /c /QA21164 /GX 
# ADD CPP /nologo /MT /Z7 /W3 /Ob1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GF /Gy PRECOMP_VC7_TOBEREMOVED /Fp".\ReleaseAXP/game1.pch" /Fo".\ReleaseAXP/" /Fd".\ReleaseAXP/" /c /QA21164 /GX 
# ADD BASE MTL /nologo /D"NDEBUG" /mktyplib203 /tlb".\..\ReleaseAXP\game1.tlb" /win32 
# ADD MTL /nologo /D"NDEBUG" /mktyplib203 /tlb".\..\ReleaseAXP\game1.tlb" /win32 
# ADD BASE RSC /l 1033 /d "NDEBUG" 
# ADD RSC /l 1033 /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib odbc32.lib odbccp32.lib /nologo /dll /out:"..\ReleaseAXP\gameaxp.dll" /incremental:no /def:".\game1.def" /pdb:".\..\ReleaseAXP\gameaxp.pdb" /pdbtype:sept /subsystem:windows /base:"0x20000000" /implib:".\..\ReleaseAXP/gameaxp.lib" 
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib odbc32.lib odbccp32.lib /nologo /dll /out:"..\ReleaseAXP\gameaxp.dll" /incremental:no /def:".\game1.def" /pdb:".\..\ReleaseAXP\gameaxp.pdb" /pdbtype:sept /subsystem:windows /base:"0x20000000" /implib:".\..\ReleaseAXP/gameaxp.lib" 

!ELSEIF  "$(CFG)" == "3zb2bot - x64 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "$(PlatformName)\$(ConfigurationName)"
# PROP BASE Intermediate_Dir "$(PlatformName)\$(ConfigurationName)"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "$(PlatformName)\$(ConfigurationName)"
# PROP Intermediate_Dir "$(PlatformName)\$(ConfigurationName)"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /Z7 /Ob1 /Oi /Ot /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Gy PRECOMP_VC7_TOBEREMOVED /Fp".\Release/game1.pch" /Fo".\Release/" /Fd".\Release/" /TC /c /GX 
# ADD CPP /nologo /MT /Z7 /Ob1 /Oi /Ot /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Gy PRECOMP_VC7_TOBEREMOVED /Fp".\Release/game1.pch" /Fo".\Release/" /Fd".\Release/" /TC /c /GX 
# ADD BASE MTL /nologo /D"NDEBUG" /mktyplib203 /tlb".\..\Release\game1.tlb" /win32 
# ADD MTL /nologo /D"NDEBUG" /mktyplib203 /tlb".\..\Release\game1.tlb" /win32 
# ADD BASE RSC /l 1033 /d "NDEBUG" 
# ADD RSC /l 1033 /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib /nologo /dll /out:"c:\Quake2\3zb2\gamex86xp.dll" /incremental:no /def:".\game1.def" /pdb:".\..\Release\gamex86.pdb" /pdbtype:sept /subsystem:windows /base:"0x20000000" /implib:".\..\Release/gamex86.lib" /MACHINE:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib /nologo /dll /out:"c:\Quake2\3zb2\gamex86xp.dll" /incremental:no /def:".\game1.def" /pdb:".\..\Release\gamex86.pdb" /pdbtype:sept /subsystem:windows /base:"0x20000000" /implib:".\..\Release/gamex86.lib" /MACHINE:I386

!ENDIF

# Begin Target

# Name "3zb2bot - Win32 Debug"
# Name "3zb2bot - Win32 Debug Alpha"
# Name "3zb2bot - Win32 Release Alpha"
# Name "3zb2bot - Win32 Release"
# Name "3zb2bot - x64 Debug"
# Name "3zb2bot - x64 Debug Alpha"
# Name "3zb2bot - x64 Release Alpha"
# Name "3zb2bot - x64 Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=bot.c

!IF  "$(CFG)" == "3zb2bot - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=bot_fire.c

!IF  "$(CFG)" == "3zb2bot - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=bot_func.c

!IF  "$(CFG)" == "3zb2bot - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=bot_za.c

!IF  "$(CFG)" == "3zb2bot - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=g_chase.c

!IF  "$(CFG)" == "3zb2bot - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=g_cmds.c

!IF  "$(CFG)" == "3zb2bot - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=g_combat.c

!IF  "$(CFG)" == "3zb2bot - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=g_ctf.c

!IF  "$(CFG)" == "3zb2bot - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=g_func.c

!IF  "$(CFG)" == "3zb2bot - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=g_items.c

!IF  "$(CFG)" == "3zb2bot - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=g_main.c

!IF  "$(CFG)" == "3zb2bot - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=g_misc.c

!IF  "$(CFG)" == "3zb2bot - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=g_monster.c

!IF  "$(CFG)" == "3zb2bot - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=g_phys.c

!IF  "$(CFG)" == "3zb2bot - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=g_save.c

!IF  "$(CFG)" == "3zb2bot - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=g_spawn.c

!IF  "$(CFG)" == "3zb2bot - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=g_svcmds.c

!IF  "$(CFG)" == "3zb2bot - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=g_target.c

!IF  "$(CFG)" == "3zb2bot - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=g_trigger.c

!IF  "$(CFG)" == "3zb2bot - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=g_utils.c

!IF  "$(CFG)" == "3zb2bot - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=g_weapon.c

!IF  "$(CFG)" == "3zb2bot - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=game1.def
# End Source File
# Begin Source File

SOURCE=m_move.c

!IF  "$(CFG)" == "3zb2bot - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=p_client.c

!IF  "$(CFG)" == "3zb2bot - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=p_hud.c

!IF  "$(CFG)" == "3zb2bot - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=p_menu.c

!IF  "$(CFG)" == "3zb2bot - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=p_trail.c

!IF  "$(CFG)" == "3zb2bot - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=p_view.c

!IF  "$(CFG)" == "3zb2bot - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=p_weapon.c

!IF  "$(CFG)" == "3zb2bot - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=q_shared.c

!IF  "$(CFG)" == "3zb2bot - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "BUILDING_REF_GL" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "3zb2bot - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=bot.h
# End Source File
# Begin Source File

SOURCE=botstr.h
# End Source File
# Begin Source File

SOURCE=g_local.h
# End Source File
# Begin Source File

SOURCE=game.h
# End Source File
# Begin Source File

SOURCE=q_shared.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project

