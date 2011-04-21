# Microsoft Developer Studio Project File - Name="the reckoning" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=the reckoning - Win32 Debug Alpha
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "the reckoning.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "the reckoning.mak" CFG="the reckoning - Win32 Debug Alpha"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "the reckoning - Win32 Debug Alpha" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "the reckoning - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "the reckoning - Win32 Release Alpha" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "the reckoning - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "the reckoning - x64 Debug Alpha" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "the reckoning - x64 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "the reckoning - x64 Release Alpha" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "the reckoning - x64 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "the reckoning - Win32 Debug Alpha"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\..\DebugAXP"
# PROP BASE Intermediate_Dir ".\DebugAXP"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\..\DebugAXP"
# PROP Intermediate_Dir ".\DebugAXP"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /Zi /W3 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" PRECOMP_VC7_TOBEREMOVED /Fp".\DebugAXP/xatrix.pch" /Fo".\DebugAXP/" /Fd".\DebugAXP/" /GZ /c /GX 
# ADD CPP /nologo /MTd /Zi /W3 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" PRECOMP_VC7_TOBEREMOVED /Fp".\DebugAXP/xatrix.pch" /Fo".\DebugAXP/" /Fd".\DebugAXP/" /GZ /c /GX 
# ADD BASE MTL /nologo /D"_DEBUG" /mktyplib203 /tlb".\..\DebugAXP\xatrix.tlb" /win32 
# ADD MTL /nologo /D"_DEBUG" /mktyplib203 /tlb".\..\DebugAXP\xatrix.tlb" /win32 
# ADD BASE RSC /l 1033 /d "_DEBUG" 
# ADD RSC /l 1033 /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib odbc32.lib odbccp32.lib winmm.lib /nologo /dll /out:".\debugAXP\gameaxp.dll" /incremental:no /def:".\xatrix.def" /debug /pdb:".\..\DebugAXP\gameaxp.pdb" /pdbtype:sept /map:".\DebugAXP\gameaxp.map" /subsystem:windows /implib:".\..\DebugAXP/gameaxp.lib" 
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib odbc32.lib odbccp32.lib winmm.lib /nologo /dll /out:".\debugAXP\gameaxp.dll" /incremental:no /def:".\xatrix.def" /debug /pdb:".\..\DebugAXP\gameaxp.pdb" /pdbtype:sept /map:".\DebugAXP\gameaxp.map" /subsystem:windows /implib:".\..\DebugAXP/gameaxp.lib" 

!ELSEIF  "$(CFG)" == "the reckoning - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\debug"
# PROP BASE Intermediate_Dir ".\debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\debug"
# PROP Intermediate_Dir ".\debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /Zi /W3 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" PRECOMP_VC7_TOBEREMOVED /Fp".\debug/xatrix.pch" /Fo".\debug/" /Fd".\debug/" /TC /GZ /c /GX 
# ADD CPP /nologo /MTd /Zi /W3 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" PRECOMP_VC7_TOBEREMOVED /Fp".\debug/xatrix.pch" /Fo".\debug/" /Fd".\debug/" /TC /GZ /c /GX 
# ADD BASE MTL /nologo /D"_DEBUG" /mktyplib203 /tlb".\debug\xatrix.tlb" /win32 
# ADD MTL /nologo /D"_DEBUG" /mktyplib203 /tlb".\debug\xatrix.tlb" /win32 
# ADD BASE RSC /l 1033 /d "_DEBUG" 
# ADD RSC /l 1033 /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib /nologo /dll /out:"c:\quake2\xatrix\gamex86xp.dll" /incremental:no /nodefaultlib:"odbc32.lib" /def:".\xatrix.def" /debug /pdb:".\debug\gamex86.pdb" /pdbtype:sept /map:".\debug\gamex86.map" /subsystem:windows /implib:".\debug/gamex86.lib" /MACHINE:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib /nologo /dll /out:"c:\quake2\xatrix\gamex86xp.dll" /incremental:no /nodefaultlib:"odbc32.lib" /def:".\xatrix.def" /debug /pdb:".\debug\gamex86.pdb" /pdbtype:sept /map:".\debug\gamex86.map" /subsystem:windows /implib:".\debug/gamex86.lib" /MACHINE:I386

!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release Alpha"

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
# ADD BASE CPP /nologo /MT /W3 /Ob1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GF /Gy PRECOMP_VC7_TOBEREMOVED /Fp".\ReleaseAXP/xatrix.pch" /Fo".\ReleaseAXP/" /Fd".\ReleaseAXP/" /c /GX 
# ADD CPP /nologo /MT /W3 /Ob1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GF /Gy PRECOMP_VC7_TOBEREMOVED /Fp".\ReleaseAXP/xatrix.pch" /Fo".\ReleaseAXP/" /Fd".\ReleaseAXP/" /c /GX 
# ADD BASE MTL /nologo /D"NDEBUG" /mktyplib203 /tlb".\..\ReleaseAXP\xatrix.tlb" /win32 
# ADD MTL /nologo /D"NDEBUG" /mktyplib203 /tlb".\..\ReleaseAXP\xatrix.tlb" /win32 
# ADD BASE RSC /l 1033 /d "NDEBUG" 
# ADD RSC /l 1033 /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib odbc32.lib odbccp32.lib winmm.lib /nologo /dll /out:".\ReleaseAXP\gameaxp.dll" /incremental:no /def:".\xatrix.def" /pdb:".\..\ReleaseAXP\gameaxp.pdb" /pdbtype:sept /subsystem:windows /implib:".\..\ReleaseAXP/gameaxp.lib" 
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib odbc32.lib odbccp32.lib winmm.lib /nologo /dll /out:".\ReleaseAXP\gameaxp.dll" /incremental:no /def:".\xatrix.def" /pdb:".\..\ReleaseAXP\gameaxp.pdb" /pdbtype:sept /subsystem:windows /implib:".\..\ReleaseAXP/gameaxp.lib" 

!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\release"
# PROP BASE Intermediate_Dir ".\release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\release"
# PROP Intermediate_Dir ".\release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /Ob1 /Oi /Ot /D "WIN32" /D "NDEBUG" /D "_WINDOWS" PRECOMP_VC7_TOBEREMOVED /Fp".\release/xatrix.pch" /Fo".\release/" /Fd".\release/" /TC /c /GX 
# ADD CPP /nologo /MT /Ob1 /Oi /Ot /D "WIN32" /D "NDEBUG" /D "_WINDOWS" PRECOMP_VC7_TOBEREMOVED /Fp".\release/xatrix.pch" /Fo".\release/" /Fd".\release/" /TC /c /GX 
# ADD BASE MTL /nologo /D"NDEBUG" /mktyplib203 /tlb".\release\xatrix.tlb" /win32 
# ADD MTL /nologo /D"NDEBUG" /mktyplib203 /tlb".\release\xatrix.tlb" /win32 
# ADD BASE RSC /l 1033 /d "NDEBUG" 
# ADD RSC /l 1033 /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib /nologo /dll /out:"c:\quake2\xatrix\gamex86xp.dll" /incremental:no /def:".\xatrix.def" /pdb:".\release\gamex86.pdb" /pdbtype:sept /subsystem:windows /implib:".\release/gamex86.lib" /machine:ix86 /MACHINE:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib /nologo /dll /out:"c:\quake2\xatrix\gamex86xp.dll" /incremental:no /def:".\xatrix.def" /pdb:".\release\gamex86.pdb" /pdbtype:sept /subsystem:windows /implib:".\release/gamex86.lib" /machine:ix86 /MACHINE:I386

!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug Alpha"

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
# ADD BASE CPP /nologo /MTd /Zi /W3 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" PRECOMP_VC7_TOBEREMOVED /Fp".\DebugAXP/xatrix.pch" /Fo".\DebugAXP/" /Fd".\DebugAXP/" /GZ /c /GX 
# ADD CPP /nologo /MTd /Zi /W3 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" PRECOMP_VC7_TOBEREMOVED /Fp".\DebugAXP/xatrix.pch" /Fo".\DebugAXP/" /Fd".\DebugAXP/" /GZ /c /GX 
# ADD BASE MTL /nologo /D"_DEBUG" /mktyplib203 /tlb".\..\DebugAXP\xatrix.tlb" /win32 
# ADD MTL /nologo /D"_DEBUG" /mktyplib203 /tlb".\..\DebugAXP\xatrix.tlb" /win32 
# ADD BASE RSC /l 1033 /d "_DEBUG" 
# ADD RSC /l 1033 /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib odbc32.lib odbccp32.lib winmm.lib /nologo /dll /out:".\debugAXP\gameaxp.dll" /incremental:no /def:".\xatrix.def" /debug /pdb:".\..\DebugAXP\gameaxp.pdb" /pdbtype:sept /map:".\DebugAXP\gameaxp.map" /subsystem:windows /implib:".\..\DebugAXP/gameaxp.lib" 
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib odbc32.lib odbccp32.lib winmm.lib /nologo /dll /out:".\debugAXP\gameaxp.dll" /incremental:no /def:".\xatrix.def" /debug /pdb:".\..\DebugAXP\gameaxp.pdb" /pdbtype:sept /map:".\DebugAXP\gameaxp.map" /subsystem:windows /implib:".\..\DebugAXP/gameaxp.lib" 

!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug"

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
# ADD BASE CPP /nologo /MTd /Zi /W3 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" PRECOMP_VC7_TOBEREMOVED /Fp".\debug/xatrix.pch" /Fo".\debug/" /Fd".\debug/" /TC /GZ /c /GX 
# ADD CPP /nologo /MTd /Zi /W3 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" PRECOMP_VC7_TOBEREMOVED /Fp".\debug/xatrix.pch" /Fo".\debug/" /Fd".\debug/" /TC /GZ /c /GX 
# ADD BASE MTL /nologo /D"_DEBUG" /mktyplib203 /tlb".\debug\xatrix.tlb" /win32 
# ADD MTL /nologo /D"_DEBUG" /mktyplib203 /tlb".\debug\xatrix.tlb" /win32 
# ADD BASE RSC /l 1033 /d "_DEBUG" 
# ADD RSC /l 1033 /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib /nologo /dll /out:"c:\quake2\xatrix\gamex86xp.dll" /incremental:no /nodefaultlib:"odbc32.lib" /def:".\xatrix.def" /debug /pdb:".\debug\gamex86.pdb" /pdbtype:sept /map:".\debug\gamex86.map" /subsystem:windows /implib:".\debug/gamex86.lib" /MACHINE:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib /nologo /dll /out:"c:\quake2\xatrix\gamex86xp.dll" /incremental:no /nodefaultlib:"odbc32.lib" /def:".\xatrix.def" /debug /pdb:".\debug\gamex86.pdb" /pdbtype:sept /map:".\debug\gamex86.map" /subsystem:windows /implib:".\debug/gamex86.lib" /MACHINE:I386

!ELSEIF  "$(CFG)" == "the reckoning - x64 Release Alpha"

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
# ADD BASE CPP /nologo /MT /W3 /Ob1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GF /Gy PRECOMP_VC7_TOBEREMOVED /Fp".\ReleaseAXP/xatrix.pch" /Fo".\ReleaseAXP/" /Fd".\ReleaseAXP/" /c /GX 
# ADD CPP /nologo /MT /W3 /Ob1 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GF /Gy PRECOMP_VC7_TOBEREMOVED /Fp".\ReleaseAXP/xatrix.pch" /Fo".\ReleaseAXP/" /Fd".\ReleaseAXP/" /c /GX 
# ADD BASE MTL /nologo /D"NDEBUG" /mktyplib203 /tlb".\..\ReleaseAXP\xatrix.tlb" /win32 
# ADD MTL /nologo /D"NDEBUG" /mktyplib203 /tlb".\..\ReleaseAXP\xatrix.tlb" /win32 
# ADD BASE RSC /l 1033 /d "NDEBUG" 
# ADD RSC /l 1033 /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib odbc32.lib odbccp32.lib winmm.lib /nologo /dll /out:".\ReleaseAXP\gameaxp.dll" /incremental:no /def:".\xatrix.def" /pdb:".\..\ReleaseAXP\gameaxp.pdb" /pdbtype:sept /subsystem:windows /implib:".\..\ReleaseAXP/gameaxp.lib" 
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib odbc32.lib odbccp32.lib winmm.lib /nologo /dll /out:".\ReleaseAXP\gameaxp.dll" /incremental:no /def:".\xatrix.def" /pdb:".\..\ReleaseAXP\gameaxp.pdb" /pdbtype:sept /subsystem:windows /implib:".\..\ReleaseAXP/gameaxp.lib" 

!ELSEIF  "$(CFG)" == "the reckoning - x64 Release"

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
# ADD BASE CPP /nologo /MT /Ob1 /Oi /Ot /D "WIN32" /D "NDEBUG" /D "_WINDOWS" PRECOMP_VC7_TOBEREMOVED /Fp".\release/xatrix.pch" /Fo".\release/" /Fd".\release/" /TC /c /GX 
# ADD CPP /nologo /MT /Ob1 /Oi /Ot /D "WIN32" /D "NDEBUG" /D "_WINDOWS" PRECOMP_VC7_TOBEREMOVED /Fp".\release/xatrix.pch" /Fo".\release/" /Fd".\release/" /TC /c /GX 
# ADD BASE MTL /nologo /D"NDEBUG" /mktyplib203 /tlb".\release\xatrix.tlb" /win32 
# ADD MTL /nologo /D"NDEBUG" /mktyplib203 /tlb".\release\xatrix.tlb" /win32 
# ADD BASE RSC /l 1033 /d "NDEBUG" 
# ADD RSC /l 1033 /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib /nologo /dll /out:"c:\quake2\xatrix\gamex86xp.dll" /incremental:no /def:".\xatrix.def" /pdb:".\release\gamex86.pdb" /pdbtype:sept /subsystem:windows /implib:".\release/gamex86.lib" /MACHINE:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib /nologo /dll /out:"c:\quake2\xatrix\gamex86xp.dll" /incremental:no /def:".\xatrix.def" /pdb:".\release\gamex86.pdb" /pdbtype:sept /subsystem:windows /implib:".\release/gamex86.lib" /MACHINE:I386

!ENDIF

# Begin Target

# Name "the reckoning - Win32 Debug Alpha"
# Name "the reckoning - Win32 Debug"
# Name "the reckoning - Win32 Release Alpha"
# Name "the reckoning - Win32 Release"
# Name "the reckoning - x64 Debug Alpha"
# Name "the reckoning - x64 Debug"
# Name "the reckoning - x64 Release Alpha"
# Name "the reckoning - x64 Release"
# Begin Group "Source Files"

# PROP Default_Filter "*.c"
# Begin Source File

SOURCE=g_ai.c

!IF  "$(CFG)" == "the reckoning - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=g_chase.c

!IF  "$(CFG)" == "the reckoning - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=g_cmds.c

!IF  "$(CFG)" == "the reckoning - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=g_combat.c

!IF  "$(CFG)" == "the reckoning - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=g_func.c

!IF  "$(CFG)" == "the reckoning - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=g_items.c

!IF  "$(CFG)" == "the reckoning - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=g_main.c

!IF  "$(CFG)" == "the reckoning - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=g_misc.c

!IF  "$(CFG)" == "the reckoning - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=g_monster.c

!IF  "$(CFG)" == "the reckoning - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=g_phys.c

!IF  "$(CFG)" == "the reckoning - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=g_save.c

!IF  "$(CFG)" == "the reckoning - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=g_spawn.c

!IF  "$(CFG)" == "the reckoning - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=g_svcmds.c

!IF  "$(CFG)" == "the reckoning - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=g_target.c

!IF  "$(CFG)" == "the reckoning - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=g_trigger.c

!IF  "$(CFG)" == "the reckoning - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=g_turret.c

!IF  "$(CFG)" == "the reckoning - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=g_utils.c

!IF  "$(CFG)" == "the reckoning - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=g_weapon.c

!IF  "$(CFG)" == "the reckoning - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=m_actor.c

!IF  "$(CFG)" == "the reckoning - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=m_berserk.c

!IF  "$(CFG)" == "the reckoning - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=m_boss2.c

!IF  "$(CFG)" == "the reckoning - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=m_boss3.c

!IF  "$(CFG)" == "the reckoning - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=m_boss31.c

!IF  "$(CFG)" == "the reckoning - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=m_boss32.c

!IF  "$(CFG)" == "the reckoning - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=m_boss5.c

!IF  "$(CFG)" == "the reckoning - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=m_brain.c

!IF  "$(CFG)" == "the reckoning - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=m_chick.c

!IF  "$(CFG)" == "the reckoning - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=m_fixbot.c

!IF  "$(CFG)" == "the reckoning - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=m_flash.c

!IF  "$(CFG)" == "the reckoning - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=m_flipper.c

!IF  "$(CFG)" == "the reckoning - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=m_float.c

!IF  "$(CFG)" == "the reckoning - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=m_flyer.c

!IF  "$(CFG)" == "the reckoning - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=m_gekk.c

!IF  "$(CFG)" == "the reckoning - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=m_gladb.c

!IF  "$(CFG)" == "the reckoning - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=m_gladiator.c

!IF  "$(CFG)" == "the reckoning - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=m_gunner.c

!IF  "$(CFG)" == "the reckoning - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=m_hover.c

!IF  "$(CFG)" == "the reckoning - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=m_infantry.c

!IF  "$(CFG)" == "the reckoning - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=m_insane.c

!IF  "$(CFG)" == "the reckoning - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=m_medic.c

!IF  "$(CFG)" == "the reckoning - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=m_move.c

!IF  "$(CFG)" == "the reckoning - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=m_mutant.c

!IF  "$(CFG)" == "the reckoning - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=m_parasite.c

!IF  "$(CFG)" == "the reckoning - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=m_soldier.c

!IF  "$(CFG)" == "the reckoning - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=m_supertank.c

!IF  "$(CFG)" == "the reckoning - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=m_tank.c

!IF  "$(CFG)" == "the reckoning - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=p_client.c

!IF  "$(CFG)" == "the reckoning - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=p_hud.c

!IF  "$(CFG)" == "the reckoning - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=p_trail.c

!IF  "$(CFG)" == "the reckoning - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=p_view.c

!IF  "$(CFG)" == "the reckoning - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=p_weapon.c

!IF  "$(CFG)" == "the reckoning - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# Begin Source File

SOURCE=q_shared.c

!IF  "$(CFG)" == "the reckoning - Win32 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - Win32 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug Alpha"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "C_ONLY" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Debug"

# ADD CPP /nologo /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /GZ /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release Alpha"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "C_ONLY" /GX 
!ELSEIF  "$(CFG)" == "the reckoning - x64 Release"

# ADD CPP /nologo /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /GX 
!ENDIF

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "*.h"
# Begin Source File

SOURCE=g_local.h
# End Source File
# Begin Source File

SOURCE=game.h
# End Source File
# Begin Source File

SOURCE=m_actor.h
# End Source File
# Begin Source File

SOURCE=m_berserk.h
# End Source File
# Begin Source File

SOURCE=m_boss2.h
# End Source File
# Begin Source File

SOURCE=m_boss31.h
# End Source File
# Begin Source File

SOURCE=m_boss32.h
# End Source File
# Begin Source File

SOURCE=m_brain.h
# End Source File
# Begin Source File

SOURCE=m_chick.h
# End Source File
# Begin Source File

SOURCE=m_fixbot.h
# End Source File
# Begin Source File

SOURCE=m_flipper.h
# End Source File
# Begin Source File

SOURCE=m_float.h
# End Source File
# Begin Source File

SOURCE=m_flyer.h
# End Source File
# Begin Source File

SOURCE=m_gekk.h
# End Source File
# Begin Source File

SOURCE=m_gladiator.h
# End Source File
# Begin Source File

SOURCE=m_gunner.h
# End Source File
# Begin Source File

SOURCE=m_hover.h
# End Source File
# Begin Source File

SOURCE=m_infantry.h
# End Source File
# Begin Source File

SOURCE=m_insane.h
# End Source File
# Begin Source File

SOURCE=m_medic.h
# End Source File
# Begin Source File

SOURCE=m_mutant.h
# End Source File
# Begin Source File

SOURCE=m_parasite.h
# End Source File
# Begin Source File

SOURCE=m_player.h
# End Source File
# Begin Source File

SOURCE=m_rider.h
# End Source File
# Begin Source File

SOURCE=m_soldier.h
# End Source File
# Begin Source File

SOURCE=m_soldierh.h
# End Source File
# Begin Source File

SOURCE=m_supertank.h
# End Source File
# Begin Source File

SOURCE=m_tank.h
# End Source File
# Begin Source File

SOURCE=q_shared.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "*.def,*.res"
# Begin Source File

SOURCE=xatrix.def
# End Source File
# End Group
# End Target
# End Project

