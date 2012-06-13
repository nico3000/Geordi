#pragma once

#if defined PROFILE | defined _DEBUG
#define LI_ERROR(_msg) do \
{ \
    static Logger::ErrorMessenger* _pErrorMessenger = new Logger::ErrorMessenger; \
    std::string _s(_msg); \
    _pErrorMessenger->Show(_s, TRUE, __FUNCTION__, __FILE__, __LINE__); \
} while(0)
#else
#define LI_ERROR(_msg) do { (VOID)sizeof(_msg); } while(0)
#endif

#if defined PROFILE | defined _DEBUG
#define LI_WARNING(_msg) do { \
    static Logger::ErrorMessenger* _pErrorMessenger = new Logger::ErrorMessenger; \
    std::string _s(_msg); \
    _pErrorMessenger->Show(_s, FALSE, __FUNCTION__, __FILE__, __LINE__); \
} while(0)
#else
#define LI_WARNING(_msg) do { (VOID)sizeof(_msg); } while(0)
#endif

#if defined PROFILE | defined _DEBUG
#define LI_LOG(_tag, _msg) do { \
    Logger::Log(_tag, _msg, FALSE, __FUNCTION__, __FILE__, __LINE__); \
} while(0)
#else
#define LI_LOG(_tag, _msg) do { (VOID)sizeof(_tag); (VOID)sizeof(_msg); } while(0)
#endif

#define LI_LOG_WITH_TAG(_msg) LI_LOG(LI_LOGGER_TAG, _msg)

namespace Logger 
{
    class ErrorMessenger
    {
    private:
        BOOL m_enabled;

    public:
        ErrorMessenger(VOID);

        VOID Show(CONST std::string& p_errorMsg, BOOL p_isFatal, CONST CHAR* p_function, CONST CHAR* p_file, UINT p_line);

    };

    BOOL Init(CONST CHAR* p_configFile);
    VOID Destroy(VOID);
    VOID Log(CONST std::string& p_tag, CONST std::string& p_errorMsg, BOOL p_isFatal, CONST CHAR* p_function, CONST CHAR* p_file, UINT p_line);
    VOID SetDisplayFlags(CONST std::string& p_tag, UCHAR p_flags);

    class LogMgr
    {
        typedef std::map<string, UCHAR> Tags;
        typedef std::list<Logger::ErrorMessenger*> ErrorMessengerList;

    private:
        Tags m_tags;
        ErrorMessengerList m_errorMessengers;

        CRITICAL_SECTION m_tagCriticalSection;
        CRITICAL_SECTION m_messengerCriticalSection;

        VOID WriteToLogFile(CONST std::string& p_text);
        VOID GetOutputBuffer(std::string& p_output, CONST std::string& p_tag, CONST std::string& p_msg, CONST CHAR* p_function, CONST CHAR* p_file, UINT p_line);

    public:
        enum ErrorDialogResult
        {
            LOGMGR_ERROR_ABORT,
            LOGMGR_ERROR_RETRY,
            LOGMGR_ERROR_IGNORE,
        };

        LogMgr(VOID);
        ~LogMgr(VOID);

        BOOL Init(CONST CHAR* p_configFile);
        VOID Log(CONST std::string& p_tag, CONST std::string& p_errorMsg, BOOL p_isFatal, CONST CHAR* p_function, CONST CHAR* p_file, UINT p_line);
        VOID SetDisplayFlags(CONST std::string& p_tag, UCHAR p_flags);
        VOID AddErrorMessenger(ErrorMessenger* p_pMessenger);
        LogMgr::ErrorDialogResult Error(CONST std::string& p_errorMsg, BOOL p_isFatal, CONST CHAR* p_function, CONST CHAR* p_file, UINT line);

    };
}


