#include "StdAfx.h"
#include "Logger.h"

#define LOG_ERROR_FALLBACK(_msg) Logger::LogErrorFallback(_msg, __FUNCTION__, __FILE__, __LINE__)
#define LOGGER_TAG "logger"

namespace Logger
{
    MsgHandler g_handler;
    Tags g_tags;


    MsgHandler::MsgHandler(VOID):
    m_initialized(FALSE)
    {
    }


    MsgHandler::~MsgHandler(VOID)
    {
        if(m_file.is_open())
        {
            m_file.close();
        }
    }

    BOOL MsgHandler::Init(string p_outFilename)
    {
        m_file.open(p_outFilename);
        return m_initialized = m_file.is_open();
    }


    VOID MsgHandler::LogInfo(string p_tag, string p_msg)
    {
        std::ostringstream str;
        str << "INFO " << p_tag << " ->" << std::endl << " " << p_msg << std::endl;
        if(g_tags.find(p_tag) != g_tags.end())
        {
            if(g_tags[p_tag].toFile)
            {
                m_file << str.str();
            }
        }
        else if(p_tag.compare("logger") != 0)
        {
            LI_WARNING("unknown tag: " + p_tag);
        }
        OutputDebugStringA(str.str().c_str());
    }


    VOID MsgHandler::LogWarning(string p_tag, string p_msg, string p_function, string p_file, INT p_line)
    {
        if(g_tags.find(p_tag) != g_tags.end())
        {
            if(!g_tags[p_tag].ignoreWarnings)
            {
                std::ostringstream str;
                str << "WARNING, Tag: " << p_tag << ", function: " << p_function << " ->" << std::endl << " " << p_file << "(" << p_line << ") : " << p_msg << std::endl;
                if(g_tags[p_tag].toFile)
                {
                    m_file << str.str();
                }
                if(g_tags[p_tag].toDebug)
                {
                    OutputDebugStringA(str.str().c_str());
                }
            }
        }
        else if(p_tag.compare("logger") != 0)
        {
            LogWarning("logger", "unknown tag: " + p_tag, p_function, p_file, p_line);
        }
    }


    VOID MsgHandler::LogError(string p_tag, string p_msg, string p_function, string p_file, INT p_line)
    {
        if(g_tags.find(p_tag) != g_tags.end())
        {
            if(!g_tags[p_tag].ignoreErrors)
            {
                std::ostringstream str;
                str << "ERROR, Tag: " << p_tag << ", function: " << p_function << " ->" << std::endl << " " << p_file << "(" << p_line << ") : " << p_msg << std::endl;
                if(g_tags[p_tag].toFile)
                {
                    m_file << str.str();
                }
                if(g_tags[p_tag].toDebug)
                {
                    OutputDebugStringA(str.str().c_str());
                }
            }
        }
        else if(p_tag.compare("logger") != 0)
        {
            LogWarning("logger", "unknown tag: " + p_tag, p_function, p_file, p_line);
        }
    }


    
    BOOL Init(string p_configXML, string p_configName)
    {
        tinyxml2::XMLDocument doc;
        INT result = doc.LoadFile(p_configXML.c_str());
        if(result != tinyxml2::XML_NO_ERROR)
        {
            LOG_ERROR_FALLBACK("error opening config file for Logger\n");
            return FALSE;
        }
        tinyxml2::XMLElement* pLoggingElem = doc.FirstChildElement("logging");
        if(pLoggingElem == NULL)
        {
            LOG_ERROR_FALLBACK("didn't find \"logging\" element");
            return FALSE;
        }
        tinyxml2::XMLElement* pConfigElem = pLoggingElem->FirstChildElement("config");
        while(pConfigElem != NULL && pConfigElem->Attribute("name", p_configName.c_str()) == NULL)
        {
            pConfigElem = pConfigElem->NextSiblingElement("config");
        }
        if(pConfigElem == NULL)
        {
            LOG_ERROR_FALLBACK("didn't find config \"" + p_configName + "\"");
            return FALSE;
        }

        tinyxml2::XMLElement* pFileElem = pConfigElem->FirstChildElement("file");
        if(pFileElem == NULL || !g_handler.Init(pFileElem->Attribute("name")))
        {
            LOG_ERROR_FALLBACK("error opening output file");
        }

        tinyxml2::XMLElement* pTagElem = pConfigElem->FirstChildElement("tag");
        while(pTagElem != NULL)
        {
            Tag tag;
            tag.ignoreErrors = FALSE;
            tag.ignoreWarnings = FALSE;
            tag.toDebug = pTagElem->BoolAttribute("toDebug");
            tag.toFile = pTagElem->BoolAttribute("toFile");
            g_tags[pTagElem->Attribute("name")] = tag;

            pTagElem = pTagElem->NextSiblingElement("tag");

            //OutputDebugStringA(pTagElem->Attribute("name"));
        }

        LogInfo("logger", "Initialization successful");
        
        return TRUE;
    }


    VOID LogErrorFallback(string p_msg, string p_function, string p_file, INT p_line)
    {
        std::ostringstream str;
        str << p_file << "(" << p_line << ") : " << p_function << "; " << p_msg << std::endl;
        OutputDebugStringA(str.str().c_str());
    }


    VOID LogInfo(string p_tag, string p_msg)
    {
        if(g_handler.IsInitialized())
        {
            g_handler.LogInfo(p_tag, p_msg);
        }
    }


    VOID LogWarning(string p_tag, string p_msg, string p_function, string p_file, INT p_line)
    {
        if(g_handler.IsInitialized())
        {
            g_handler.LogWarning(p_tag, p_msg, p_function, p_file, p_line);
        }
    }


    VOID LogError(string p_tag, string p_msg, string p_function, string p_file, INT p_line)
    {
        if(g_handler.IsInitialized())
        {
            g_handler.LogError(p_tag, p_msg, p_function, p_file, p_line);
        }
    }
}

