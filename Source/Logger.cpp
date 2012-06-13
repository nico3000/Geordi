#include "StdAfx.h"
#include "Logger.h"


#define LOGMGR_TAG_ERROR "ERROR"
#define LOGMGR_TAG_WARNING "WARNING"


namespace Logger
{
    static LogMgr* g_pLogMgr = NULL;


    ErrorMessenger::ErrorMessenger(VOID):
    m_enabled(TRUE)
    {
        g_pLogMgr->AddErrorMessenger(this);
    }

    VOID ErrorMessenger::Show(CONST std::string& p_errorMsg, BOOL p_isFatal, CONST CHAR* p_function, CONST CHAR* p_file, UINT p_line)
    {
        if(m_enabled)
        {
            switch(g_pLogMgr->Error(p_errorMsg, p_isFatal, p_function, p_file, p_line))
            {
            case LogMgr::LOGMGR_ERROR_ABORT: break; // TODO
            case LogMgr::LOGMGR_ERROR_IGNORE: m_enabled = FALSE;
            case LogMgr::LOGMGR_ERROR_RETRY: break; // do nothing
            }

        }
    }


    BOOL Init(CONST CHAR* p_configFile)
    {
        g_pLogMgr = new LogMgr;
        if(g_pLogMgr == NULL)
        {
            return FALSE;
        }
        return g_pLogMgr->Init(p_configFile);
    }


    VOID Destroy(VOID)
    {
        SAFE_DELETE(g_pLogMgr);
    }


    VOID Log(CONST std::string& p_tag, CONST std::string& p_msg, BOOL p_isFatal, CONST CHAR* p_function, CONST CHAR* p_file, UINT p_line)
    {
        g_pLogMgr->Log(p_tag, p_msg, p_isFatal, p_function, p_file, p_line);
    }


    VOID SetDisplayFlags(CONST std::string& p_tag, UCHAR p_flags)
    {
        g_pLogMgr->SetDisplayFlags(p_tag, p_flags);
    }


    CONST UCHAR LOGFLAG_WRITE_TO_LOG_FILE = 1 << 0;
    CONST UCHAR LOGFLAG_WRITE_TO_DEBUGGER = 1 << 1;
#define LOGMGR_FILE "LostIsland.log"

    LogMgr::LogMgr(VOID)
    {

    }


    LogMgr::~LogMgr(VOID)
    {
        for each(ErrorMessenger* pErrorMessenger in m_errorMessengers)
        {
            delete pErrorMessenger;
        }
        m_errorMessengers.clear();
    }


    BOOL LogMgr::Init(CONST CHAR* p_configFile)
    {
        tinyxml2::XMLDocument doc;
        INT result = doc.LoadFile(p_configFile);
        if(result != tinyxml2::XML_NO_ERROR)
        {
            OutputDebugStringA((std::string("error opening ") + p_configFile + '\n').c_str());
            return FALSE;
        }
        tinyxml2::XMLElement* pLoggingElem = doc.FirstChildElement("Logging");
        if(pLoggingElem == NULL)
        {
            OutputDebugStringA((std::string("no \"Logging\" element in ") + p_configFile + '\n').c_str());
            return FALSE;
        }
        
        tinyxml2::XMLElement* pTagElem = pLoggingElem->FirstChildElement("Log");
        while(pTagElem != NULL)
        {
            UCHAR flags = 0;
            if(pTagElem->BoolAttribute("file"))
            {
                flags |= LOGFLAG_WRITE_TO_LOG_FILE;
            }
            if(pTagElem->BoolAttribute("debugger"))
            {
                flags |= LOGFLAG_WRITE_TO_DEBUGGER;
            }
            this->SetDisplayFlags(pTagElem->Attribute("tag"), flags);

            pTagElem = pTagElem->NextSiblingElement("Log");
        }
        
        std::ofstream file(LOGMGR_FILE, std::ios::trunc);
        file.close();
        
        LI_LOG("Logger", "Initialization successful");
                
        //g_sbHWnd = CreateStatusWindowA(WS_CHILD | WS_VISIBLE, "Teststatus", hWnd, 0);
        
        return TRUE;
    }


    VOID LogMgr::Log(CONST std::string& p_tag, CONST std::string& p_msg, BOOL p_isFatal, CONST CHAR* p_function, CONST CHAR* p_file, UINT p_line)
    {
        std::string out;
        this->GetOutputBuffer(out, p_tag, p_msg, p_function, p_file, p_line);

        UCHAR flags = m_tags.find(p_tag) == m_tags.end() ? (LOGFLAG_WRITE_TO_DEBUGGER | LOGFLAG_WRITE_TO_LOG_FILE) : m_tags[p_tag];
        if(flags & LOGFLAG_WRITE_TO_DEBUGGER)
        {
            OutputDebugStringA((out + '\n').c_str());
        }
        if(flags & LOGFLAG_WRITE_TO_LOG_FILE)
        {
            this->WriteToLogFile(out + '\n');
        }
    }


    VOID LogMgr::SetDisplayFlags(CONST std::string& p_tag, UCHAR p_flags)
    {
        m_tags[p_tag] = p_flags;
    }


    VOID LogMgr::AddErrorMessenger(ErrorMessenger* p_pMessenger)
    {
        m_errorMessengers.push_back(p_pMessenger);
    }


    LogMgr::ErrorDialogResult LogMgr::Error(CONST std::string& p_errorMsg, BOOL p_isFatal, CONST CHAR* p_function, CONST CHAR* p_file, UINT p_line)
    {
        std::string out;
        this->GetOutputBuffer(out, p_isFatal ? LOGMGR_TAG_ERROR : LOGMGR_TAG_WARNING, p_errorMsg, p_function, p_file, p_line);
        switch(MessageBoxA(NULL, out.c_str(), LOGMGR_TAG_ERROR, MB_ABORTRETRYIGNORE | (p_isFatal ? MB_ICONERROR : MB_ICONWARNING)))
        {
        case IDABORT: return LOGMGR_ERROR_ABORT;
        case IDRETRY: return LOGMGR_ERROR_RETRY;
        case IDIGNORE: return LOGMGR_ERROR_IGNORE;
        }
        return LOGMGR_ERROR_RETRY; // unreachable... i hope...
    }


    VOID LogMgr::GetOutputBuffer(std::string& p_output, CONST std::string& p_tag, CONST std::string& p_msg, CONST CHAR* p_function, CONST CHAR* p_file, UINT p_line)
    {
        std::ostringstream str(p_output);
        str << "[" << p_tag << "] " << p_msg;
        if(p_tag.compare(LOGMGR_TAG_ERROR) == 0 || p_tag.compare(LOGMGR_TAG_WARNING) == 0)
        {
            str << std::endl << "Function: " << p_function << std::endl << p_file << std::endl << "Line: " << p_line;
        }
        p_output = str.str();
    }


    VOID LogMgr::WriteToLogFile(CONST std::string& p_text)
    {
        std::ofstream file(LOGMGR_FILE, std::ios::app);
        file << p_text;
        file.close();
    }
}

