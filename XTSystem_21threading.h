namespace XTSystem
{
    namespace Threading
    {
        class Thread
        {
        public:
            static void Sleep(int millisecondsTimeout)
            {
                ::Sleep((DWORD)millisecondsTimeout);
            }
        };
    }
}