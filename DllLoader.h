#pragma once

#include "windows.h"

class DllLoader {
private:
    HMODULE m_libHandle;
public:

    DllLoader() {

        m_libHandle = NULL;
    }

    ~DllLoader() {

        if(m_libHandle != NULL) {

            FreeLibrary(m_libHandle);
        }

    }

    bool load(std::string fn) {
        
        m_libHandle = LoadLibraryA(fn.c_str());

        if(m_libHandle == NULL) {
        
            std::cout << "DllLoader: failed to load code... Error: " << GetLastError() << std::endl;

            return false;
        }

        return true;
    }

    template<typename... T_ARGS>
    bool call(std::string fName, T_ARGS... args) {

	
        using FUNC = void(*)(T_ARGS... args);

        FUNC func = (FUNC) GetProcAddress(m_libHandle, fName.c_str());
        
        if(func == NULL) {
            
            std::cout << "DllLoader: function retrieval failed .. Error: " << GetLastError() << std::endl;
            return false;
        }

        func(args...);

        return true;

    }

};