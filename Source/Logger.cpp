#include "StdAfx.h"
#include "Logger.h"

namespace Logger
{
    MsgHandler g_handler;


    MsgHandler::MsgHandler(VOID)
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
        return m_file.is_open();
    }


    Tags g_tags;

    BOOL Init(string p_configXML)
    {
        g_handler.Init("geordi.log");

        tinyxml2::XMLDocument doc;
        INT result = doc.LoadFile(p_configXML.c_str());
        if(result != tinyxml2::XML_NO_ERROR)
        {
            std::cout << "error opening config file for Logger" << std::endl;
            return FALSE;
        }
    

        return TRUE;
    }

    VOID LogInfo(string p_tag, string p_msg)
    {
        OutputDebugStringA((p_msg + "\n").c_str());
    }
}

