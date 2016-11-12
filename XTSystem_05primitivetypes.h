
namespace XTSystem
{
    class Bool
    {
        public: static bool TryParse(const String& text, bool* pOutValue)
            {
                if (String::Compare(text, _T("true"),true)==0)
                {
                    *pOutValue=true;
                    return true;
                }
                if (String::Compare(text, _T("false"),true)==0)
                {
                    *pOutValue=false;
                    return true;
                }
                return false;
            }
        public: static bool Parse(const String &s)
            {
                bool bValue(false);
                bool bOk = TryParse(s, &bValue);
                if (!bOk)
                {
                    throw Exception(_T("Invalid conversion"));
                }
                return bValue;
            }
        public: static String ToString(bool b)
            {
                return (b)?_T("True"):_T("False");
            }
        public: Bool(bool d = false) :data(d)
            {
            }
        public: String ToString() const
            {
                return ToString(data);
            }

            String Format(const String &TwoWordsdelimitedBySlash) const
            {
                std::vector<String> items = TwoWordsdelimitedBySlash.Split(L"/", StringSplitOptions::RemoveEmptyEntries);
                if (items.size() < 2) return ToString();
                return data ? items[0] : items[1];
            }

        public: operator bool() const { return data; }
        private:
            bool data;
    };


    class Double
    {
        public: static double Epsilon() { return DBL_EPSILON;  }
        public: static double MaxValue() { return DBL_MAX; }
        public: static double MinValue() { return  DBL_MIN; }
    //    public: static double Epsilon() { return 4.94066e-324;  }
    //    public: static double MaxValue() { return 1.79769e+308; }
    //    public: static double MinValue() { return  -1.79769e+308; }
            //public: static double NaN() { return  0.0 / 0.0;}
            //public: static double NegativeInfinity() { return  -1.0 / 0.0; }
            //public: static double PositiveInfinity() { return  1.0 / 0.0; }

        public: static bool TryParse(const String& text, double* pOutValue)
            {
                const Char* ptr = text.c_str();
                Char* endptr;
                *pOutValue = _tcstod(ptr, &endptr);
                return (*ptr && endptr - ptr == text.Length());
            }
        public: static double Parse(const String &s)
            {
                double dValue(0);
                bool bOk = TryParse(s, &dValue);
                if (!bOk)
                {
                    throw Exception(_T("Invalid conversion"));
                }
                return dValue;
            }
        public: static String ToString(double d)
            {
                return String::Format(_T("{0}"), d);
            }
        public:    Double(double d=0.0):data(d)
            {

            }
        public: String ToString() const
            {
                return ToString(data);
            }
        public: String ToString(String sFormat) const
            {
                return String::Format(sFormat,data);
            }
        public: operator double () const { return data; }
        private:
            double data;
    };


    class Long
    {

        public: static long MaxValue() { return LONG_MAX; }
        public: static long MinValue() { return LONG_MIN; }

    //public: static long MaxValue() { return 9223372036854775807L; }
    //public: static long MinValue() { return -9223372036854775808L; }
        public: static bool TryParse(const String& text, long* pOutValue)
            {
                const Char* ptr = text.c_str();
                Char* endptr;
                *pOutValue = _tcstol(ptr, &endptr,10);
                return (*ptr && endptr - ptr == (int)text.Length());
            }
        public: static long Parse(const String &s)
            {
                long lValue(0);
                bool bOk = TryParse(s, &lValue);
                if (!bOk)
                {
                    throw Exception(_T("Invalid conversion"));
                }
                return lValue;
            }
        public: static String ToString(long d)
            {
                return String::Format(_T("{0}"), d);
            }
        public:    Long(long d = 0) :data(d)
            {
            }
        public: String ToString() const
            {
                return ToString(data);
            }
        public: String ToString(String sFormat) const
            {
                return String::Format(sFormat, data);
            }
        public: operator long() const { return data; }
        private:
            long data;

    };


    class Int32
    {
        public: static int MaxValue() { return INT_MAX; }
        public: static int MinValue() { return INT_MIN; }
    //public: static int MaxValue() { return 2147483647; }
    //public: static int MinValue() { return -2147483648; }
        public: static bool TryParse(const String& text, int* pOutValue)
            {
                const Char* ptr = text.c_str();
                Char* endptr;
                *pOutValue = _tcstol(ptr, &endptr,10);
                return (*ptr && endptr - ptr == (int)text.Length());
            }

        public: static int Parse(const String &s)
            {
                int lValue(0);
                bool bOk = TryParse(s, &lValue);
                if (!bOk)
                {
                    throw Exception(_T("Invalid conversion"));
                }
                return lValue;
            }
        public: static String ToString(long d)
            {
                return String::Format(_T("{0}"), d);
            }
        public:    Int32(int d = 0) :data(d)
            {
            }
        public: String ToString() const
            {
                return ToString(data);
            }
        public: String ToString(String sFormat) const
            {
                return String::Format(sFormat, data);
            }
        public: operator int() const { return data; }
        private:
            int data;
    };



}

