#pragma once

#if defined PROFILE | defined _DEBUG
#define LI_ERROR(_msg) do \
{ \
    static Logger::ErrorMessenger* _pErrorMessenger = new Logger::ErrorMessenger; \
    std::string _s(_msg); \
    _pErrorMessenger->Show(_s, true, __FUNCTION__, __FILE__, __LINE__); \
} while(0)
#else
#define LI_ERROR(_msg) do { (void)sizeof(_msg); } while(0)
#endif

#if defined PROFILE | defined _DEBUG
#define LI_WARNING(_msg) do { \
    static Logger::ErrorMessenger* _pErrorMessenger = new Logger::ErrorMessenger; \
    std::string _s(_msg); \
    _pErrorMessenger->Show(_s, false, __FUNCTION__, __FILE__, __LINE__); \
} while(0)
#else
#define LI_WARNING(_msg) do { (void)sizeof(_msg); } while(0)
#endif

#if defined PROFILE | defined _DEBUG
#define LI_LOG(_tag, _msg) do { \
    Logger::Log(_tag, _msg, false, __FUNCTION__, __FILE__, __LINE__); \
} while(0)
#else
#define LI_LOG(_tag, _msg) do { (void)sizeof(_tag); (void)sizeof(_msg); } while(0)
#endif

#define LI_INFO(_msg) LI_LOG("INFO", _msg)

#define LI_LOG_WITH_TAG(_msg) LI_LOG(LI_LOGGER_TAG, _msg)

namespace Logger 
{
    class ErrorMessenger
    {
    private:
        bool m_enabled;

    public:
        ErrorMessenger(void);

        void Show(const std::string& p_errorMsg, bool p_isFatal, LPCSTR p_function, LPCSTR p_file, unsigned int p_line);

    };

    bool Init(LPCSTR p_configFile);
    void Destroy(void);
    void Log(const std::string& p_tag, const std::string& p_errorMsg, bool p_isFatal, LPCSTR p_function, LPCSTR p_file, unsigned int p_line);
    void SetDisplayFlags(const std::string& p_tag, unsigned char p_flags);

    class LogMgr
    {
        typedef std::map<std::string, unsigned char> Tags;
        typedef std::list<Logger::ErrorMessenger*> ErrorMessengerList;

    private:
        Tags m_tags;
        ErrorMessengerList m_errorMessengers;

        CRITICAL_SECTION m_tagCriticalSection;
        CRITICAL_SECTION m_messengerCriticalSection;

        void WriteToLogFile(const std::string& p_text);
        void GetOutputBuffer(std::string& p_output, const std::string& p_tag, const std::string& p_msg, LPCSTR p_function, LPCSTR p_file, unsigned int p_line);

    public:
        enum ErrorDialogResult
        {
            LOGMGR_ERROR_ABORT,
            LOGMGR_ERROR_RETRY,
            LOGMGR_ERROR_IGNORE,
        };

        LogMgr(void);
        ~LogMgr(void);

        bool Init(LPCSTR p_configFile);
        void Log(const std::string& p_tag, const std::string& p_errorMsg, bool p_isFatal, LPCSTR p_function, LPCSTR p_file, unsigned int p_line);
        void SetDisplayFlags(const std::string& p_tag, unsigned char p_flags);
        void AddErrorMessenger(ErrorMessenger* p_pMessenger);
        LogMgr::ErrorDialogResult Error(const std::string& p_errorMsg, bool p_isFatal, LPCSTR p_function, LPCSTR p_file, unsigned int p_line);

    };
}


