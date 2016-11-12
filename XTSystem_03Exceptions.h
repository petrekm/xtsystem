

namespace XTSystem
{
    namespace Stack 
    {
        class ExceptionStackWalker : public Stack::StackWalker
        {
        public:
          ExceptionStackWalker() : StackWalker(),state(0) {}
        protected:
          virtual void OnOutput(LPCSTR szText)
          { 
              //stacktrace += L"xxx\n";
              //return;

                String line = String::Convert_charsToString(szText);

                switch (state)
                {
                case 0:
                    //if (line.Contains(L"StackWalker::ShowCallstack")) state=1;
                    if (line.Contains(L"dbghelp")) state=1;
                    break;
                case 1:
                    state = 2;
                    break;
                case 2:
                    if (line.Contains(L": wmain")||line.Contains(L": LocalThread")) state=3;
                    stacktrace += line;
                    break;
                default:
                    break;
                }

          }

          int state;
          XTSystem::String stacktrace;
        public:
            const XTSystem::String StackTrace() { return stacktrace; }
        };

    }
}

namespace XTSystem
{
    template<class T> void safe_delete(T*&p) { if (NULL != p) { delete p; p=NULL; } }
    template<class T> void safe_delete_array(T*&p) { if (NULL != p) { delete []p; p=NULL; } }

    class Exception
    {
        public:
            Exception(const String &message)
            {
                Message = message;
#ifdef XTSYSTEM_EXCEPTIONS_WITH_CALLSTACK
                Stack::ExceptionStackWalker sw;
                sw.ShowCallstack();
                StackTrace = sw.StackTrace();
#endif
            }
            Exception(const Char *message)
            {
                Message = message;
#ifdef XTSYSTEM_EXCEPTIONS_WITH_CALLSTACK
                Stack::ExceptionStackWalker sw;
                sw.ShowCallstack();
                StackTrace = sw.StackTrace();
#endif
            }

            String Message;
            String StackTrace;
            String ToString() const { return Message; }

    };
}



