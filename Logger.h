#pragma once
//#include <memory>

#define TEMP_ARG_1 template<class T1>
#define TEMP_ARG_2 template<class T1, class T2>
#define TEMP_ARG_3 template<class T1, class T2, class T3>
#define TEMP_ARG_4 template<class T1, class T2, class T3, class T4>
#define TEMP_ARG_5 template<class T1, class T2, class T3, class T4, class T5>
#define TEMP_ARG_6 template<class T1, class T2, class T3, class T4, class T5, class T6>
#define TEMP_ARG_7 template<class T1, class T2, class T3, class T4, class T5, class T6, class T7>
#define TEMP_ARG_8 template<class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8>
#define TEMP_ARG_9 template<class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9>
#define TEMP_ARG_10 template<class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10>
#define TARG_1 T1 t1
#define TARG_2 T1 t1, T2 t2
#define TARG_3 T1 t1, T2 t2, T3 t3
#define TARG_4 T1 t1, T2 t2, T3 t3, T4 t4
#define TARG_5 T1 t1, T2 t2, T3 t3, T4 t4, T5 t5
#define TARG_6 T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6
#define TARG_7 T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7
#define TARG_8 T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8
#define TARG_9 T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9
#define TARG_10 T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9, T10 t10
#define TCREF_ARG_1 const T1 &t1
#define TCREF_ARG_2 const T1 &t1, const T2 &t2
#define TCREF_ARG_3 const T1 &t1, const T2 &t2, const T3 &t3
#define TCREF_ARG_4 const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4
#define TCREF_ARG_5 const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5
#define TCREF_ARG_6 const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6
#define TCREF_ARG_7 const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7
#define TCREF_ARG_8 const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7, const T8 &t8
#define TCREF_ARG_9 const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7, const T8 &t8, const T9 &t9
#define TCREF_ARG_10 const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7, const T8 &t8, const T9 &t9, const T10 &t10
#define ARG_1 t1
#define ARG_2 t1, t2
#define ARG_3 t1, t2, t3
#define ARG_4 t1, t2, t3, t4
#define ARG_5 t1, t2, t3, t4, t5
#define ARG_6 t1, t2, t3, t4, t5, t6
#define ARG_7 t1, t2, t3, t4, t5, t6, t7
#define ARG_8 t1, t2, t3, t4, t5, t6, t7, t8
#define ARG_9 t1, t2, t3, t4, t5, t6, t7, t8, t9
#define ARG_10 t1, t2, t3, t4, t5, t6, t7, t8, t9, t10
namespace XTSystem
{
    namespace Logging
    {

        enum_4(LoggingSeverity, info, warning,error,fatal)
        enum_5(LoggingCategory, basic, call, dev1, dev2, dev3)

        struct ILoggerImpl
        {
            virtual ~ILoggerImpl() {}
            virtual void Report(LoggingSeverity severity, LoggingCategory cat, const String & s) = 0;
        };

        class CLogger
        {
        public:
            CLogger():log(NULL) {  }

            //interface
            void Info(const String & s)  { if (NULL == log) return; log->Report(LoggingSeverity::info, LoggingCategory::basic, s); }
            void Warning(const String & s) { if (NULL == log) return; log->Report(LoggingSeverity::warning, LoggingCategory::basic, s); }
            void Error(const String & s) { if (NULL == log) return; log->Report(LoggingSeverity::error, LoggingCategory::basic, s); }
            void Fatal(const String & s) { if (NULL == log) return; log->Report(LoggingSeverity::fatal, LoggingCategory::basic, s); }

            void Info(LoggingCategory cat, const String & s)  { if (NULL == log) return; log->Report(LoggingSeverity::info, cat, s); }
            void Warning(LoggingCategory cat, const String & s) { if (NULL == log) return; log->Report(LoggingSeverity::warning, cat, s); }
            void Error(LoggingCategory cat, const String & s) { if (NULL == log) return; log->Report(LoggingSeverity::error, cat, s); }
            void Fatal(LoggingCategory cat, const String & s) { if (NULL == log) return; log->Report(LoggingSeverity::fatal, cat, s); }

            TEMP_ARG_1 void Info(const String &text, TCREF_ARG_1) { if (NULL == log) return;  Info(String::Format(text, ARG_1)); }
            TEMP_ARG_2 void Info(const String &text, TCREF_ARG_2) { if (NULL == log) return;  Info(String::Format(text, ARG_2)); }
            TEMP_ARG_3 void Info(const String &text, TCREF_ARG_3) { if (NULL == log) return;  Info(String::Format(text, ARG_3)); }
            TEMP_ARG_4 void Info(const String &text, TCREF_ARG_4) { if (NULL == log) return;  Info(String::Format(text, ARG_4)); }
            TEMP_ARG_5 void Info(const String &text, TCREF_ARG_5) { if (NULL == log) return;  Info(String::Format(text, ARG_5)); }
            TEMP_ARG_6 void Info(const String &text, TCREF_ARG_6) { if (NULL == log) return;  Info(String::Format(text, ARG_6)); }
            TEMP_ARG_7 void Info(const String &text, TCREF_ARG_7) { if (NULL == log) return;  Info(String::Format(text, ARG_7)); }
            TEMP_ARG_8 void Info(const String &text, TCREF_ARG_8) { if (NULL == log) return;  Info(String::Format(text, ARG_8)); }
            TEMP_ARG_9 void Info(const String &text, TCREF_ARG_9) { if (NULL == log) return;  Info(String::Format(text, ARG_9)); }
            TEMP_ARG_10 void Info(const String &text, TCREF_ARG_10) { if (NULL == log) return;  Info(String::Format(text, ARG_10)); }

            TEMP_ARG_1 void Warning(const String &text, TCREF_ARG_1) { if (NULL == log) return;  Warning(String::Format(text, ARG_1)); }
            TEMP_ARG_2 void Warning(const String &text, TCREF_ARG_2) { if (NULL == log) return;  Warning(String::Format(text, ARG_2)); }
            TEMP_ARG_3 void Warning(const String &text, TCREF_ARG_3) { if (NULL == log) return;  Warning(String::Format(text, ARG_3)); }
            TEMP_ARG_4 void Warning(const String &text, TCREF_ARG_4) { if (NULL == log) return;  Warning(String::Format(text, ARG_4)); }
            TEMP_ARG_5 void Warning(const String &text, TCREF_ARG_5) { if (NULL == log) return;  Warning(String::Format(text, ARG_5)); }
            TEMP_ARG_6 void Warning(const String &text, TCREF_ARG_6) { if (NULL == log) return;  Warning(String::Format(text, ARG_6)); }
            TEMP_ARG_7 void Warning(const String &text, TCREF_ARG_7) { if (NULL == log) return;  Warning(String::Format(text, ARG_7)); }
            TEMP_ARG_8 void Warning(const String &text, TCREF_ARG_8) { if (NULL == log) return;  Warning(String::Format(text, ARG_8)); }
            TEMP_ARG_9 void Warning(const String &text, TCREF_ARG_9) { if (NULL == log) return;  Warning(String::Format(text, ARG_9)); }
            TEMP_ARG_10 void Warning(const String &text, TCREF_ARG_10) { if (NULL == log) return;  Warning(String::Format(text, ARG_10)); }

            TEMP_ARG_1 void Error(const String &text, TCREF_ARG_1) { if (NULL == log) return;  Error(String::Format(text, ARG_1)); }
            TEMP_ARG_2 void Error(const String &text, TCREF_ARG_2) { if (NULL == log) return;  Error(String::Format(text, ARG_2)); }
            TEMP_ARG_3 void Error(const String &text, TCREF_ARG_3) { if (NULL == log) return;  Error(String::Format(text, ARG_3)); }
            TEMP_ARG_4 void Error(const String &text, TCREF_ARG_4) { if (NULL == log) return;  Error(String::Format(text, ARG_4)); }
            TEMP_ARG_5 void Error(const String &text, TCREF_ARG_5) { if (NULL == log) return;  Error(String::Format(text, ARG_5)); }
            TEMP_ARG_6 void Error(const String &text, TCREF_ARG_6) { if (NULL == log) return;  Error(String::Format(text, ARG_6)); }
            TEMP_ARG_7 void Error(const String &text, TCREF_ARG_7) { if (NULL == log) return;  Error(String::Format(text, ARG_7)); }
            TEMP_ARG_8 void Error(const String &text, TCREF_ARG_8) { if (NULL == log) return;  Error(String::Format(text, ARG_8)); }
            TEMP_ARG_9 void Error(const String &text, TCREF_ARG_9) { if (NULL == log) return;  Error(String::Format(text, ARG_9)); }
            TEMP_ARG_10 void Error(const String &text, TCREF_ARG_10) { if (NULL == log) return;  Error(String::Format(text, ARG_10)); }

            // do we need this?
            TEMP_ARG_1 void Fatal(const String &text, TCREF_ARG_1) { if (NULL == log) return;  Fatal(String::Format(text, ARG_1)); }
            TEMP_ARG_2 void Fatal(const String &text, TCREF_ARG_2) { if (NULL == log) return;  Fatal(String::Format(text, ARG_2)); }
            TEMP_ARG_3 void Fatal(const String &text, TCREF_ARG_3) { if (NULL == log) return;  Fatal(String::Format(text, ARG_3)); }
            TEMP_ARG_4 void Fatal(const String &text, TCREF_ARG_4) { if (NULL == log) return;  Fatal(String::Format(text, ARG_4)); }
            TEMP_ARG_5 void Fatal(const String &text, TCREF_ARG_5) { if (NULL == log) return;  Fatal(String::Format(text, ARG_5)); }
            TEMP_ARG_6 void Fatal(const String &text, TCREF_ARG_6) { if (NULL == log) return;  Fatal(String::Format(text, ARG_6)); }
            TEMP_ARG_7 void Fatal(const String &text, TCREF_ARG_7) { if (NULL == log) return;  Fatal(String::Format(text, ARG_7)); }
            TEMP_ARG_8 void Fatal(const String &text, TCREF_ARG_8) { if (NULL == log) return;  Fatal(String::Format(text, ARG_8)); }
            TEMP_ARG_9 void Fatal(const String &text, TCREF_ARG_9) { if (NULL == log) return;  Fatal(String::Format(text, ARG_9)); }
            TEMP_ARG_10 void Fatal(const String &text, TCREF_ARG_10) { if (NULL == log) return; Fatal(String::Format(text, ARG_10)); }

            /// with categories
            TEMP_ARG_1 void Info(LoggingCategory cat, const String &text, TCREF_ARG_1) { if (NULL == log) return;  Info(cat, String::Format(text, ARG_1)); }
            TEMP_ARG_2 void Info(LoggingCategory cat, const String &text, TCREF_ARG_2) { if (NULL == log) return;  Info(cat, String::Format(text, ARG_2)); }
            TEMP_ARG_3 void Info(LoggingCategory cat, const String &text, TCREF_ARG_3) { if (NULL == log) return;  Info(cat, String::Format(text, ARG_3)); }
            TEMP_ARG_4 void Info(LoggingCategory cat, const String &text, TCREF_ARG_4) { if (NULL == log) return;  Info(cat, String::Format(text, ARG_4)); }
            TEMP_ARG_5 void Info(LoggingCategory cat, const String &text, TCREF_ARG_5) { if (NULL == log) return;  Info(cat, String::Format(text, ARG_5)); }
            TEMP_ARG_6 void Info(LoggingCategory cat, const String &text, TCREF_ARG_6) { if (NULL == log) return;  Info(cat, String::Format(text, ARG_6)); }
            TEMP_ARG_7 void Info(LoggingCategory cat, const String &text, TCREF_ARG_7) { if (NULL == log) return;  Info(cat, String::Format(text, ARG_7)); }
            TEMP_ARG_8 void Info(LoggingCategory cat, const String &text, TCREF_ARG_8) { if (NULL == log) return;  Info(cat, String::Format(text, ARG_8)); }
            TEMP_ARG_9 void Info(LoggingCategory cat, const String &text, TCREF_ARG_9) { if (NULL == log) return;  Info(cat, String::Format(text, ARG_9)); }
            TEMP_ARG_10 void Info(LoggingCategory cat, const String &text, TCREF_ARG_10) { if (NULL == log) return;  Info(cat, String::Format(text, ARG_10)); }

            TEMP_ARG_1 void Warning(LoggingCategory cat, const String &text, TCREF_ARG_1) { if (NULL == log) return;  Warning(cat, String::Format(text, ARG_1)); }
            TEMP_ARG_2 void Warning(LoggingCategory cat, const String &text, TCREF_ARG_2) { if (NULL == log) return;  Warning(cat, String::Format(text, ARG_2)); }
            TEMP_ARG_3 void Warning(LoggingCategory cat, const String &text, TCREF_ARG_3) { if (NULL == log) return;  Warning(cat, String::Format(text, ARG_3)); }
            TEMP_ARG_4 void Warning(LoggingCategory cat, const String &text, TCREF_ARG_4) { if (NULL == log) return;  Warning(cat, String::Format(text, ARG_4)); }
            TEMP_ARG_5 void Warning(LoggingCategory cat, const String &text, TCREF_ARG_5) { if (NULL == log) return;  Warning(cat, String::Format(text, ARG_5)); }
            TEMP_ARG_6 void Warning(LoggingCategory cat, const String &text, TCREF_ARG_6) { if (NULL == log) return;  Warning(cat, String::Format(text, ARG_6)); }
            TEMP_ARG_7 void Warning(LoggingCategory cat, const String &text, TCREF_ARG_7) { if (NULL == log) return;  Warning(cat, String::Format(text, ARG_7)); }
            TEMP_ARG_8 void Warning(LoggingCategory cat, const String &text, TCREF_ARG_8) { if (NULL == log) return;  Warning(cat, String::Format(text, ARG_8)); }
            TEMP_ARG_9 void Warning(LoggingCategory cat, const String &text, TCREF_ARG_9) { if (NULL == log) return;  Warning(cat, String::Format(text, ARG_9)); }
            TEMP_ARG_10 void Warning(LoggingCategory cat, const String &text, TCREF_ARG_10) { if (NULL == log) return;  Warning(cat, String::Format(text, ARG_10)); }

            TEMP_ARG_1 void Error(LoggingCategory cat, const String &text, TCREF_ARG_1) { if (NULL == log) return;  Error(cat, String::Format(text, ARG_1)); }
            TEMP_ARG_2 void Error(LoggingCategory cat, const String &text, TCREF_ARG_2) { if (NULL == log) return;  Error(cat, String::Format(text, ARG_2)); }
            TEMP_ARG_3 void Error(LoggingCategory cat, const String &text, TCREF_ARG_3) { if (NULL == log) return;  Error(cat, String::Format(text, ARG_3)); }
            TEMP_ARG_4 void Error(LoggingCategory cat, const String &text, TCREF_ARG_4) { if (NULL == log) return;  Error(cat, String::Format(text, ARG_4)); }
            TEMP_ARG_5 void Error(LoggingCategory cat, const String &text, TCREF_ARG_5) { if (NULL == log) return;  Error(cat, String::Format(text, ARG_5)); }
            TEMP_ARG_6 void Error(LoggingCategory cat, const String &text, TCREF_ARG_6) { if (NULL == log) return;  Error(cat, String::Format(text, ARG_6)); }
            TEMP_ARG_7 void Error(LoggingCategory cat, const String &text, TCREF_ARG_7) { if (NULL == log) return;  Error(cat, String::Format(text, ARG_7)); }
            TEMP_ARG_8 void Error(LoggingCategory cat, const String &text, TCREF_ARG_8) { if (NULL == log) return;  Error(cat, String::Format(text, ARG_8)); }
            TEMP_ARG_9 void Error(LoggingCategory cat, const String &text, TCREF_ARG_9) { if (NULL == log) return;  Error(cat, String::Format(text, ARG_9)); }
            TEMP_ARG_10 void Error(LoggingCategory cat, const String &text, TCREF_ARG_10) { if (NULL == log) return;  Error(cat, String::Format(text, ARG_10)); }

            // do we need this?
            TEMP_ARG_1 void Fatal(LoggingCategory cat, const String &text, TCREF_ARG_1) { if (NULL == log) return;  Fatal(cat, String::Format(text, ARG_1)); }
            TEMP_ARG_2 void Fatal(LoggingCategory cat, const String &text, TCREF_ARG_2) { if (NULL == log) return;  Fatal(cat, String::Format(text, ARG_2)); }
            TEMP_ARG_3 void Fatal(LoggingCategory cat, const String &text, TCREF_ARG_3) { if (NULL == log) return;  Fatal(cat, String::Format(text, ARG_3)); }
            TEMP_ARG_4 void Fatal(LoggingCategory cat, const String &text, TCREF_ARG_4) { if (NULL == log) return;  Fatal(cat, String::Format(text, ARG_4)); }
            TEMP_ARG_5 void Fatal(LoggingCategory cat, const String &text, TCREF_ARG_5) { if (NULL == log) return;  Fatal(cat, String::Format(text, ARG_5)); }
            TEMP_ARG_6 void Fatal(LoggingCategory cat, const String &text, TCREF_ARG_6) { if (NULL == log) return;  Fatal(cat, String::Format(text, ARG_6)); }
            TEMP_ARG_7 void Fatal(LoggingCategory cat, const String &text, TCREF_ARG_7) { if (NULL == log) return;  Fatal(cat, String::Format(text, ARG_7)); }
            TEMP_ARG_8 void Fatal(LoggingCategory cat, const String &text, TCREF_ARG_8) { if (NULL == log) return;  Fatal(cat, String::Format(text, ARG_8)); }
            TEMP_ARG_9 void Fatal(LoggingCategory cat, const String &text, TCREF_ARG_9) { if (NULL == log) return;  Fatal(cat, String::Format(text, ARG_9)); }
            TEMP_ARG_10 void Fatal(LoggingCategory cat, const String &text, TCREF_ARG_10) { if (NULL == log) return; Fatal(cat, String::Format(text, ARG_10)); }


            void BindImplementation(ILoggerImpl &ilog) { log = &ilog; }
            void UnbindImplementation(){ log = NULL; }

        private:
            ILoggerImpl *log;
            
        };


        class RAII_LogEntryExit
        {
        public:
            RAII_LogEntryExit(const XTSystem::Char *asFunction,const XTSystem::Char *asFilename, int anLine, int anCall=-1, DWORD anThreadID=-1, const XTSystem::Char*asData=NULL ) 
            {
                Set(NULL,XTSystem::String(asFunction),XTSystem::String(asFilename),anLine, anCall, anThreadID, asData);
            }
            RAII_LogEntryExit(CLogger &ilog, const XTSystem::Char *asFunction,const XTSystem::Char *asFilename, int anLine, int anCall=-1, DWORD anThreadID=-1, const XTSystem::Char*asData=NULL ) 
            {
                Set(&ilog,XTSystem::String(asFunction),XTSystem::String(asFilename),anLine, anCall, anThreadID, asData);
            }


            ~RAII_LogEntryExit() 
            {
                if (m_pIILogger)
                {
                    
                    m_pIILogger->Info(LoggingCategory::call, XTSystem::String::Format(L"{0}[-] {1} [{2}:{3}]", m_padding, m_sFunction, m_nCall, m_nThreadID));
                } else
                {
                }
                DecreaseLevel(m_nThreadID);
            }
            int Level() const { return m_level;}
            XTSystem::String Padding() const { return m_padding; }
            XTSystem::String Padding1() const { return m_padding1; }


        private:
            XTSystem::String m_sFunction;
            XTSystem::String m_sFilename;
            const XTSystem::Char*Data;
            int m_nLine;
            int m_nCall;
            DWORD m_nThreadID;
            CLogger *m_pIILogger;
            XTSystem::String m_padding;
            XTSystem::String m_padding1;
            int m_level;

            void Set(CLogger *pilog, const XTSystem::String &sFunction,const XTSystem::String &sFilename, int anLine, int anCall, DWORD nThreadID, const XTSystem::Char*asData)
            {
                m_sFunction = sFunction;
                m_sFilename = sFilename;
                m_nLine = anLine;
                m_nCall = anCall;
                m_nThreadID = nThreadID;
                m_pIILogger = pilog;
                Data = asData;
                m_level = IncreaseLevel(m_nThreadID);
                m_padding = String(L"").PadLeft(m_level * Indention());
                m_padding1 = String(L"").PadLeft((m_level+1) * Indention());

                if (m_pIILogger)
                {
                    m_pIILogger->Info(LoggingCategory::call, XTSystem::String::Format(L"{0}[+] {1}({2}) [call={3}:thread={4}]",m_padding, m_sFunction, (Data)?(Data):L"", m_nCall, m_nThreadID));
                } else
                {
                }
            }

            static int IncreaseLevel(DWORD thread)
            {
                std::map<DWORD,int> &paddingLevels = PaddingLevels();
                std::map<DWORD,int>::iterator it= paddingLevels.find(thread);
                if (it==paddingLevels.end())
                {
                    paddingLevels.insert(std::make_pair(thread,-1));
                    it = paddingLevels.find(thread);
                }
                (it->second)++;
                return (it->second);
            }

            static void DecreaseLevel(DWORD thread)
            {
                std::map<DWORD,int> &paddingLevels = PaddingLevels();
                std::map<DWORD,int>::iterator it= paddingLevels.find(thread);
                if (it==paddingLevels.end())
                {
                    paddingLevels.insert(std::make_pair(thread,0));
                    it = paddingLevels.find(thread);
                } 
                if ((it->second)>0) (it->second)--;
            }

            static std::map<DWORD,int> &PaddingLevels()
            {
                static std::map<DWORD,int> _paddingLevels;
                return _paddingLevels;
            }
            static int Indention()
            {
                return 2;
            }
        };

}
}

#define LOG_ENTRY_EXIT_EX(LOGGER,...) static int __methodCall_##LOGGER=0; ++__methodCall_##LOGGER; \
    XTSystem::Logging::RAII_LogEntryExit __RAII_LogEntryExit_##LOGGER(LOGGER, __FUNCTIONW__,__FILEW__,__LINE__, __methodCall_##LOGGER, GetCurrentThreadId(), String::Format(__VA_ARGS__));

#define LOG_ENTRY_EXIT(...) LOG_ENTRY_EXIT_EX(Log,String::Format(__VA_ARGS__))

#define METHOD_LEVEL_EX(LOGGER) __RAII_LogEntryExit_##LOGGER.Level()
#define METHOD_PADDING_EX(LOGGER) __RAII_LogEntryExit_##LOGGER.Padding()
#define PAD_EX(LOGGER) __RAII_LogEntryExit_##LOGGER.Padding1()

#define METHOD_LEVEL METHOD_LEVEL_EX(Log)
#define METHOD_PADDING METHOD_PADDING_EX(Log)
#define PAD PAD_EX(Log)

#undef TEMP_ARG_1
#undef TEMP_ARG_2
#undef TEMP_ARG_3
#undef TEMP_ARG_4
#undef TEMP_ARG_5
#undef TEMP_ARG_6
#undef TEMP_ARG_7
#undef TEMP_ARG_8
#undef TEMP_ARG_9
#undef TEMP_ARG_10
#undef TARG_1
#undef TARG_2
#undef TARG_3
#undef TARG_4
#undef TARG_5
#undef TARG_6
#undef TARG_7
#undef TARG_8
#undef TARG_9
#undef TARG_10
#undef TCREF_ARG_1
#undef TCREF_ARG_2
#undef TCREF_ARG_3
#undef TCREF_ARG_4
#undef TCREF_ARG_5
#undef TCREF_ARG_6
#undef TCREF_ARG_7
#undef TCREF_ARG_8
#undef TCREF_ARG_9
#undef TCREF_ARG_10
#undef ARG_1
#undef ARG_2
#undef ARG_3
#undef ARG_4
#undef ARG_5
#undef ARG_6
#undef ARG_7
#undef ARG_8
#undef ARG_9
#undef ARG_10


namespace XTSystem
{
    namespace Logging
    {
        class CFileLogger :public ILoggerImpl
        {
            private: String folder;
            private: String filename;
            private: String filenamePath;
            private: IO::StreamWriter writer;

            public: String Folder() { return folder; }
                    String FilenamePath() { return filenamePath; }
                    String Filename() { return filename; }

            public: CFileLogger(const String &_folder = L"C:\\", const String &_filename = L"<default>")
            {
                    folder = _folder;
                    filename = _filename;
                    if (filename == L"<default>") filename = GenerateFilename();
                    filenamePath = IO::Path::Combine(folder, filename);
                    //if (IO::Directory::IsFileWritable(filenamePath));

                    writer.Open(filenamePath);
            }


            public: void Report(LoggingSeverity severity, LoggingCategory cat, const String & s)
            {
                DateTime now = DateTime::Now();
                String time = String::Format(L"{0}:{1}:{2}.{3}", now.Hour(), now.Minute(), now.Second(), now.Millisecond());
                    
                writer.WriteLine(String::Format(L"{0},{1},{2},{3}", time, severity.ToString(), cat.ToString(), s));
                //writer.Flush();
            }

            public: static String GenerateFilename(const String &prefix = L"log", const String &extension = L".csv")
            {
                DateTime now = DateTime::Now();
                return String::Format(L"{0}_{1:04d}_{2:02d}_{3:02d}_{4:02d}_{5:02d}_{6:02d}{7}", prefix, now.Year(), now.Month(), now.Day(), now.Hour(), now.Minute(), now.Second(), extension);
            }


        };

        class CConsoleLogger :public ILoggerImpl
        {

            public: void Report(LoggingSeverity severity, LoggingCategory cat, const String & s)
            {
                DateTime now = DateTime::Now();
                String time = String::Format(L"{0:02d}:{1:02d}:{2:02d}.{3:03d}", now.Hour(), now.Minute(), now.Second(), now.Millisecond());
                Console::WriteLine(String::Format(L"{0} [{1:7}, {2:7}]  {3}", time, severity.ToString(), cat.ToString(), s));
            }

        };
    }
}