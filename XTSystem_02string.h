

#include <string>
#include <sstream>
#include <vector>

#ifndef _WIN32

#include <wchar.h>
#pragma warning(push)
#pragma warning(disable:4996)
#define _T(X) L"" X

#define TCHAR wchar_t
#define _tcsicmp wcsicmp
#define _tcscmp wcscmp
#define _tcslen wcslen
#endif


namespace XTSystem
{
	namespace Text
	{
		class StringBuilder;
	}
    //enum_2(StringSplitOptions,None,RemoveEmptyEntries);
    //enum StringSplitOptions
    //{
    //    None = 0,
    //    RemoveEmptyEntries = 1
    //};

    class StringSplitOptions
    {
        public: StringSplitOptions(): value(None) {}
        StringSplitOptions(int val): value(val) {}
        static const int None = 0;
        static const int RemoveEmptyEntries = 1;
        operator int() { return value;}
        private:
            int value;
    };

	typedef TCHAR Char;
class String
{
#ifdef _WIN32
	private: static std::wstring widen(const std::string& str)
	{
		const char*p = str.c_str();
		if (NULL == p) return _T("");
		const WCHAR *pwcsName;
		int nChars = MultiByteToWideChar(CP_ACP, 0, p, -1, NULL, 0);
		pwcsName = new WCHAR[nChars];
		MultiByteToWideChar(CP_ACP, 0, p, -1, (LPWSTR)pwcsName, nChars);
		std::wstring s(pwcsName);
		delete[] pwcsName;
		return s;

	}

	private: static std::string narrow(const std::wstring& str)
	{
		const TCHAR*p = str.c_str();
		if (NULL == p) return "";
		const char *pcsName;
		int nChars = WideCharToMultiByte(CP_ACP, 0, p, -1, NULL, 0, NULL, NULL);
		pcsName = new char[nChars];
		WideCharToMultiByte(CP_ACP, 0, p, -1, (LPSTR)pcsName, nChars, NULL, NULL);
		std::string s(pcsName);
		delete[] pcsName;
		return s;
	}

#else
private: static int wcsicmp(const TCHAR* cs,const TCHAR * ct)
	 {
	     while (tolower(*cs) == tolower(*ct))
	     {
		 if (*cs == 0)
		     return 0;
		 cs++;
		 ct++;
	     }
	     return tolower(*cs) - tolower(*ct);
	 }
private: static std::wstring widen( const std::string& str )
	{

            int n = str.size();
            if (n<=0) return L"";
            
			TCHAR* buf = new TCHAR[n+1];
            int rtn_val = mbstowcs(buf, str.c_str(),n);
            buf[n] = 0;
            std::wstring x;
            x = buf;
            delete buf;
            return x;

	}
private: static std::string narrow( const std::wstring& str )
{

            int n = str.size();
            if (n<=0) return "";
            
            char* buf = new char[n+1];
            int rtn_val = wcstombs(buf, str.c_str(),n);
            buf[n] = 0;
            std::string x;
            
            x = buf;
            delete buf;
            return x;

}
#endif

public:
    String():data() {}
    String(const String &s):data(s.data) {}
    String(const Char *s):data(s) {}
    String(const char *s): data()
    {
		std::string ss(s);
		data = widen(ss);
    }
    static String FromWstring(const std::wstring &s) { String ss; ss.data = s; return ss; }

    int Length() const { return (int)data.size(); }
    const Char *c_str() const { return data.c_str(); }
    const Char *GetBuffer() const{ return data.c_str(); }
    const std::wstring &GetWString() const { return data; }
	String ToString() const
	{
		return String(*this);
	}
    void Clear() { data.clear(); }

    static int Compare(const String &strA, const String &strB, bool ignoreCase=false) 
    {
        return Compare(strA.data.c_str(), strB.data.c_str(), ignoreCase);
    }
    static int Compare(const String &strA, const Char* strB, bool ignoreCase=false) 
    {
        return Compare(strA.data.c_str(), strB, ignoreCase);
    }
    static int Compare(const Char*strA, const Char* strB, bool ignoreCase=false) 
    {
        int cmp=_tcsicmp(strA, strB);
        if (ignoreCase || cmp !=0) 
        {
            return (cmp>0)?1:(cmp<0)?-1:0;
        } else
        {
            cmp = _tcscmp(strB, strA);
            return (cmp>0)?1:(cmp<0)?-1:0;
        }
    }

    int CompareTo(const String &strB) const
    {
        return Compare(data.c_str(), strB.data.c_str());
    }
    int CompareTo(const Char* strB) const
    {
        return Compare(data.c_str(), strB);
    }

    String& operator=(const String& inString) { data = inString.data; return *this;}
    String& operator=(const std::wstring& inString) { data = inString; return *this;}
    String& operator=(const Char* const inString) { data = inString; return *this;}
    String& operator=(const char* const inString) { std::string ss(inString); data = widen(ss); return *this;}
    String& operator+=(const String & inString) { data += inString.data; return *this; }
    String& operator+=(Char inChar) { data += inChar; return *this; }


    operator const std::wstring &() const { return data; }
    operator const Char*() const { return data.c_str(); }
    Char operator[](const int in_index) const { return data.at(in_index); }

    static String Format() { return _T("");}
    static String Format(const String& s) { return s; }
    TEMP_ARG_1 static String Format(const String &s, TCREF_ARG_1) { String res; res.data = XTSystem_fmt::Format(s.data, ARG_1); return res; }
    TEMP_ARG_2 static String Format(const String &s, TCREF_ARG_2) { String res; res.data = XTSystem_fmt::Format(s.data, ARG_2); return res; }
    TEMP_ARG_3 static String Format(const String &s, TCREF_ARG_3) { String res; res.data = XTSystem_fmt::Format(s.data, ARG_3); return res; }
    TEMP_ARG_4 static String Format(const String &s, TCREF_ARG_4) { String res; res.data = XTSystem_fmt::Format(s.data, ARG_4); return res; }
    TEMP_ARG_5 static String Format(const String &s, TCREF_ARG_5) { String res; res.data = XTSystem_fmt::Format(s.data, ARG_5); return res; }
    TEMP_ARG_6 static String Format(const String &s, TCREF_ARG_6) { String res; res.data = XTSystem_fmt::Format(s.data, ARG_6); return res; }
    TEMP_ARG_7 static String Format(const String &s, TCREF_ARG_7) { String res; res.data = XTSystem_fmt::Format(s.data, ARG_7); return res; }
    TEMP_ARG_8 static String Format(const String &s, TCREF_ARG_8) { String res; res.data = XTSystem_fmt::Format(s.data, ARG_8); return res; }
    TEMP_ARG_9 static String Format(const String &s, TCREF_ARG_9) { String res; res.data = XTSystem_fmt::Format(s.data, ARG_9); return res; }
    TEMP_ARG_10 static String Format(const String &s, TCREF_ARG_10) { String res; res.data = XTSystem_fmt::Format(s.data, ARG_10); return res; }


		private: static inline std::wstring &ltrim(std::wstring &s, const Char*chars = _T(" \t\n\r\v"))
        {
            std::wstring::size_type loc = s.find_first_not_of(chars);
            if (loc != std::wstring::npos) s = s.substr(loc); else { s = _T(""); }
            return s;
        }

		private: static inline std::wstring &rtrim(std::wstring &s, const Char*chars = _T(" \t\n\r\v"))
        {
            std::wstring::size_type loc = s.find_last_not_of(chars);
            if (loc != std::wstring::npos) s = s.substr(0, loc + 1); else { s = _T(""); }
            return s;
        }

		private: static inline std::wstring &trim(std::wstring &s)
        {
            return ltrim(rtrim(s));
        }

    public: String Trim() const
            {
                String s(*this);
                trim(s.data);
                return s;
            }


            public: String Trim(std::vector<Char> trimChars) const
            {
                String s(*this);
                trim(s.data);
                return s;
            }
    public: String TrimEnd(const Char* trimChars) const
            {
                String s(*this);
                rtrim(s.data, trimChars);
                return s;
            }
    public: String TrimEnd(std::vector<Char> trimChars) const
            {
                String s(*this);
                rtrim(s.data);
                return s;
            }
    public: String TrimStart(const Char* trimChars) const
            {
                String s(*this);;
                ltrim(s.data, trimChars);
                return s;
            }
    public: String TrimStart(std::vector<Char> trimChars) const
            {
                String s(*this);
                trim(s.data);
                return s;
            }

    public: String ToLower() const
            {
                String s(*this);
                //transform(s.data.begin(), s.data.end(),s.data.begin(),tolower);
				size_t n = s.data.size();
				for (size_t i = 0; i < n; i++) s.data[i] = tolower(s.data[i]);
                return s;
            }
    public: String ToUpper() const
            {
                String s(*this);
                //transform(s.data.begin(), s.data.end(),s.data.begin(),toupper);
				size_t n = s.data.size();
				for (size_t i = 0; i < n; i++) s.data[i] = toupper(s.data[i]);
                return s;
            }

	public: String Insert(int startIndex, const String &value) const
            {
                String s(*this);
                s.data.insert(startIndex,value.data);
                return s;
            }
            String Insert(int startIndex, const Char*value) const
            {
                String s(*this);
                s.data.insert(startIndex,value);
                return s;
            }

    public: String PadLeft(int totalWidth) const
            {
                return PadLeft(totalWidth,L' ');
            }

            String PadLeft(int totalWidth, Char paddingChar) const
            {
                String s(*this);
                int num = totalWidth-Length();
                if (num>0) s.data.insert(0,num,paddingChar);
                return s;
            }

            String PadRight(int totalWidth) const
            {
                return PadRight(totalWidth,L' ');
            }

            String PadRight(int totalWidth, Char paddingChar) const
            {
                String s(*this);
                int num = totalWidth-Length();
                if (num>0) s.data.insert(Length(),num,paddingChar);
                return s;
            }



    public: 
        //std::vector<String> Split(params char[] separator)  const;
        //std::vector<String> Split(char[] separator, int count)  const;
        //std::vector<String> Split(char[] separator, StringSplitOptions options)  const;
        //std::vector<String> Split(string[] separator, StringSplitOptions options)  const
        std::vector<String> Split(const String &separator, StringSplitOptions options) const
        {
            std::vector<String> result;
            int separatorLength = separator.Length();
            int start = 0;
            int end = IndexOf(separator);
            while (end != -1)
            {
                if (end-start >0 || options == StringSplitOptions::None)
                {
                    result.push_back(Substring(start, end - start));
                }
                start = end + separatorLength;
                end = IndexOf(separator,start);
            }
            if (end == -1)
            {
                if (start < Length() || options == StringSplitOptions::None) result.push_back(Substring(start));
            }
            return result;
        }

        //std::vector<String> Split(char[] separator, int count, StringSplitOptions options);
        //std::vector<String> Split(string[] separator, int count, StringSplitOptions options)

    public: 
        
            int IndexOf(Char value, int startIndex) const
            {
                std::wstring::size_type loc = data.find(value, startIndex );
                if( loc == std::wstring::npos ) return -1;
                return (int)loc;
            }

            int IndexOf(const Char *needle, int startIndex) const
            {
                std::wstring::size_type loc = data.find(needle, startIndex );
                if( loc == std::wstring::npos ) return -1;
                return (int)loc;
            }

            int IndexOf(const String  &needle, int startIndex) const { return IndexOf(needle.c_str(), startIndex); }
            int IndexOf(Char value)  const { return IndexOf(value, 0); }
            int IndexOf(const Char *needle) const { return IndexOf(needle,0); }
            int IndexOf(const String  &needle) const { return IndexOf(needle, 0); }
            
        
            int LastIndexOf(Char value, int startIndex) const
            {
                std::wstring::size_type loc = data.rfind(value, startIndex );
                if( loc == std::wstring::npos ) return -1;
                return (int)loc;
            }

            int LastIndexOf(const Char *needle, int startIndex) const
            {
                std::wstring::size_type loc = data.rfind(needle, startIndex );
                if( loc == std::wstring::npos ) return -1;
                return (int)loc;
            }

            int LastIndexOf(const String  &needle, int startIndex) const { return LastIndexOf(needle.c_str(), startIndex); }
            int LastIndexOf(Char value) const { return LastIndexOf(value, Length()-1); }
            int LastIndexOf(const Char *needle) const { return LastIndexOf(needle,Length()-1); }
            int LastIndexOf(const String  &needle) const { return LastIndexOf(needle, Length()-1); }
            
    public: bool StartsWith(const Char *value) const
            {
                int index=IndexOf(value);
                return index==0;
            }
            bool StartsWith(const String &value) const { return StartsWith(value.c_str()); }

            bool Contains(const Char *value) const { int index=IndexOf(value); return index!=-1; }
            bool Contains(const String &value) const { return Contains(value.c_str()); }
            bool Contains(const Char value) const { int index=IndexOf(value); return index!=-1; }
            
            bool EndsWith(const Char *value) const
            {
                int len = (int)_tcslen(value);
                int index=LastIndexOf(value);
                return index==Length()-len;
            }
            bool EndsWith(const String &value) const
            { 
                int len = value.Length();
                int index = LastIndexOf(value.c_str());
                return index==Length()-len;
            }
            bool EndsWith(Char value) const
			{
				return (Length()>=1 && data[Length()-1]==value);
			}
            bool StartsWith(Char value) const
			{
				return (Length()>0 && data[0]==value);
			}

    public: String Substring(int startIndex) const
            {
                String res;
                res.data = data.substr(startIndex);
                return res;
            }

            String Substring(int startIndex, int length) const
            {
                String res;
                res.data = data.substr(startIndex, length);
                return res;
            }
            
    public: std::vector<Char> ToCharArray()
            {
                std::vector<Char> arr;
                arr.reserve(Length());
                for (int i=0;i<Length();i++) arr.push_back(data[i]);
                return arr;
            }

            std::vector<Char> ToCharArray(int startIndex, int length)
            {
                std::vector<Char> arr;
                arr.reserve(length);
                if (startIndex<0) startIndex=0;
                if (startIndex>=Length()) return arr;
                for (int i=startIndex; (i < Length()) && (i<startIndex+length) ;i++) arr.push_back(data[i]);
                return arr;
            }


    public: 
            String Replace(const Char *oldValue, const Char *newValue)
            {
                std::wstring::size_type pos = 0;
                std::wstring::size_type match_pos;
                std::wstring result;
                std::wstring sOldValue(oldValue);
                std::wstring sNewValue(newValue);

                while ((match_pos = data.find(sOldValue, pos)) != std::wstring::npos) 
                {
                    result += data.substr(pos, match_pos - pos);
                    result += sNewValue;
                    pos = match_pos + sOldValue.size();
                }
                result += data.substr(pos, std::wstring::npos);

                String s;
                s.data = result;
                return s;
            }

            String Replace(const String &oldValue, const String &newValue) { return Replace(oldValue.c_str(), newValue.c_str()); }
            String Replace(const Char*oldValue, const String &newValue) { return Replace(oldValue, newValue.c_str()); }
            String Replace(const String &oldValue, const Char*newValue) { return Replace(oldValue.c_str(), newValue); }

            String Replace(Char oldChar, Char newChar)
            {
                String s(*this);
                //std::replace( s.data.begin(), s.data.end(), oldChar, newChar);
				size_t n = s.data.size();
				for (size_t i = 0; i < n; i++)
				{
					if (s.data[i] == oldChar) s.data[i] = newChar;
				}
                return s;
            }


            String Remove(int startIndex)
            {
                String s(*this);
                s.data.erase(startIndex);     
                return s;
            }
            String Remove(int startIndex, int count)
            {
                String s(*this);
                s.data.erase(startIndex, count);     
                return s;
            }

			std::string Tostdstring() const
			{
				return narrow(data);
            }

	std::wstring Tostdwstring() const
	{
		return data;
	}


    friend String operator+(const String& A,const String& B)
    {
        String res;
        res.data = A.data + B.data;
        return res;
    }
    friend String operator+(const String& A,const Char& B)
    {
        String res;
        res.data = A.data + B;
        return res;
    }

    friend bool operator== (const String& lhs, const String& rhs) { return lhs.data.compare(rhs.data)==0; }
    friend bool operator== (const Char*   lhs, const String& rhs) { return rhs.data.compare(lhs)==0; }
    friend bool operator== (const String& lhs, const Char*   rhs) { return lhs.data.compare(rhs)==0; }
    friend bool operator!= (const String& lhs, const String& rhs) { return lhs.data.compare(rhs.data)!=0; }

    friend bool operator!= (const Char*   lhs, const String& rhs) { return rhs.data.compare(lhs)!=0; }
    friend bool operator!= (const String& lhs, const Char*   rhs) { return lhs.data.compare(rhs)!=0; }
    friend bool operator<  (const String& lhs, const String& rhs) { return lhs.data.compare(rhs.data)<0; }
    friend bool operator<  (const Char*   lhs, const String& rhs) { return rhs.data.compare(lhs)>0; }
    friend bool operator<  (const String& lhs, const Char*   rhs) { return lhs.data.compare(rhs)<0; }

    friend bool operator<= (const String& lhs, const String& rhs) { return lhs.data.compare(rhs.data)<=0; }
    friend bool operator<= (const Char*   lhs, const String& rhs) { return rhs.data.compare(lhs)>=0; }
    friend bool operator<= (const String& lhs, const Char*   rhs) { return lhs.data.compare(rhs)<=0; }
    friend bool operator>  (const String& lhs, const String& rhs) { return lhs.data.compare(rhs.data)>0; }
    friend bool operator>  (const Char*   lhs, const String& rhs) { return rhs.data.compare(lhs)<0; }
    friend bool operator>  (const String& lhs, const Char*   rhs) { return lhs.data.compare(rhs)>0; }
    friend bool operator>= (const String& lhs, const String& rhs) { return lhs.data.compare(rhs.data)>=0; }
    friend bool operator>= (const Char*   lhs, const String& rhs) { return rhs.data.compare(lhs)<=0; }
    friend bool operator>= (const String& lhs, const Char*   rhs) { return lhs.data.compare(rhs)>=0; }


	//friend std::basic_ostringstream<Char>&operator<<(std::basic_ostringstream<Char> &os, const String &d)
	//{
	//	os << d.data;
	//	return os;
	//}

private:
    std::wstring data;

	void _reserve(size_t capacity)
	{
		data.reserve(capacity);
	}
	friend class Text::StringBuilder;
};


}

#ifndef _WIN32
	#pragma warning(pop)
#endif

