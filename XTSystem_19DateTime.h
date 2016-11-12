
namespace XTSystem
{
    class DateTime
    {
        private:
            SYSTEMTIME st;

        public:
            int Year() const { return (int)st.wYear; }
            int Month() const { return (int)st.wMonth; }
            int Day() const { return (int)st.wDay; }
            int Hour() const { return (int)st.wHour; }
            int Minute() const { return (int)st.wMinute; }
            int Second() const { return (int)st.wSecond; }
            int Millisecond() const { return (int)st.wMilliseconds; }

            static DateTime Now()
            {
                DateTime date;
                GetLocalTime(&date.st);
                //GetSystemTime(&date.st);

                return date;
            }
    };

}
