#pragma once

namespace Logger 
{
    struct Tag {
        BOOL toFile;
        BOOL toDebug;
        BOOL ignoreWarnings;
        BOOL ignoreErrors;
    };

    class MsgHandler
    {
    private:
        string m_fileName;
        std::ofstream m_file;

    public:
        MsgHandler(VOID);
        ~MsgHandler(VOID);

        BOOL Init(string p_outFilename);
    };

    typedef std::hash_map<string, Tag> Tags;

    extern Tags g_tags;

    BOOL Init(string p_configXML);
    VOID LogInfo(string p_tag, string p_msg);
    VOID LogWarning(string p_tag, string p_msg, string p_function, string p_file, INT p_line);
    VOID LogError(string p_tag, string p_msg, string p_function, string p_file, INT p_line);
}


