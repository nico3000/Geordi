#pragma once

#define LI_TAG_ERROR(_tag, _msg) Logger::LogError(_tag, _msg, __FUNCTION__, __FILE__, __LINE__)
#define LI_TAG_WARNING(_tag, _msg) Logger::LogWarning(_tag, _msg, __FUNCTION__, __FILE__, __LINE__)
#define LI_TAG_INFO(_tag, _msg) Logger::LogInfo(_tag, _msg)

#define LI_ERROR(_msg) LI_TAG_ERROR(LOGGER_TAG, _msg);
#define LI_WARNING(_msg) LI_TAG_WARNING(LOGGER_TAG, _msg);
#define LI_INFO(_msg) LI_TAG_INFO(LOGGER_TAG, _msg);

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
        BOOL m_initialized;

    public:
        MsgHandler(VOID);
        ~MsgHandler(VOID);

        BOOL Init(string p_outFilename);
        VOID LogInfo(string p_tag, string p_msg);
        VOID LogWarning(string p_tag, string p_msg, string p_function, string p_file, INT p_line);
        VOID LogError(string p_tag, string p_msg, string p_function, string p_file, INT p_line);

        BOOL IsInitialized(VOID) { return m_initialized; }

    };

    typedef std::hash_map<string, Tag> Tags;

    BOOL Init(HWND hWnd, string p_configXML, string p_configName);
    VOID LogErrorFallback(string p_msg, string p_function, string p_file, INT p_line);
    VOID LogInfo(string p_tag, string p_msg);
    VOID LogWarning(string p_tag, string p_msg, string p_function, string p_file, INT p_line);
    VOID LogError(string p_tag, string p_msg, string p_function, string p_file, INT p_line);
    VOID ShowStatus(wstring p_text);
}


