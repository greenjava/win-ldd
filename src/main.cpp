#include <stdio.h>
#include <iostream>
#include <vector>
#include <map>
#include <string>

#include <windows.h>
#include <Dbghelp.h>

typedef std::map<std::string, std::string> DepMapType;

//------------------------------------------------------------------------------

const DepMapType getDependencies(const HMODULE hMod)
{
    DepMapType depsMap;

    IMAGE_DOS_HEADER* pDosHeader = (IMAGE_DOS_HEADER*)hMod;
    IMAGE_OPTIONAL_HEADER * pOptHeader = (IMAGE_OPTIONAL_HEADER*)((BYTE*)hMod + pDosHeader->e_lfanew +24);
    IMAGE_IMPORT_DESCRIPTOR* pImportDesc = (IMAGE_IMPORT_DESCRIPTOR*) ((BYTE*)hMod + pOptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

    while ( pImportDesc->FirstThunk )
    {
        char* dllName = (char*) ((BYTE*)hMod + pImportDesc->Name);

        std::string dllPath = "";
        HMODULE hModDep = ::LoadLibraryEx(dllName, NULL, DONT_RESOLVE_DLL_REFERENCES);
        if(hModDep != 0)
        {
            LPTSTR  strDLLPath = new TCHAR[_MAX_PATH];
            ::GetModuleFileName(hModDep, strDLLPath, _MAX_PATH);
            dllPath = std::string(strDLLPath);
            delete strDLLPath;
        }
        FreeLibrary(hModDep);
        depsMap[std::string(dllName)] = dllPath;

        pImportDesc++;
    }

    return depsMap;
}

//------------------------------------------------------------------------------

int printDependencies(const std::string libPath)
{
    HMODULE hMod = ::LoadLibraryEx(libPath.c_str(), NULL, DONT_RESOLVE_DLL_REFERENCES);
    if(hMod == 0)
    {
        // Retrieves the last error message.
        DWORD   lastError = GetLastError();
        char    buffer[1024];
        FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, 0, lastError, 0, buffer, 1024, 0 );
        std::cerr << "\t" << buffer << std::endl;
        return -1;
    }
    const DepMapType& depMap = getDependencies(hMod);
    FreeLibrary(hMod);
    DepMapType::const_iterator iter;
    for(iter = depMap.begin(); iter != depMap.end(); ++iter)
    {
        std::cout << "\t" << iter->first << " => " << iter->second << std::endl;
    }

    return 0;
}

//------------------------------------------------------------------------------

int printUsage()
{
    std::cout << "ldd for Windows, Version 1.0" << std::endl;
    std::cout << "usage:\n ldd FILE..." << std::endl;
    return -1;
}

//------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    if(argc <= 1)
    {
        return printUsage();
    }
    int res = 0;
    for(int i = 1; i < argc; ++i)
    {
        std::cout << argv[i] << std::endl;
        res = printDependencies(argv[i]);
    }
    return res;
}

//------------------------------------------------------------------------------

