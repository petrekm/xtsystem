namespace XTSystem
{

    class Convert
    {
    public:
        template<typename T> static String ToString(T defaultvalue) { return _T(""); }
        template<> static String ToString(long defaultvalue) { return String::Format(_T("{0}"),defaultvalue); }
        template<> static String ToString(double defaultvalue) { return String::Format(_T("{0}"),defaultvalue); }
        template<> static String ToString(const String &defaultvalue) { return defaultvalue; }
        template<> static String ToString(bool defaultvalue) { return (defaultvalue)?_T("True"):_T("False"); }

		template<class T>static Char ToChar(const T &o) { return Character::Parse(o.ToString()); }
		template<>static Char ToChar(const Char &o) { return o; }

		template<class T>static bool ToBoolean(const T &o) { return Bool::Parse(o.ToString()); }
		template<>static bool ToBoolean(const bool &o) { return o; }

		template<class T>static double ToDouble(const T &o) { return Double::Parse(o.ToString()); }
		template<>static double ToDouble(const double &o) { return o; }

		template<class T>static float ToSingle(const T &o) { return Single::Parse(o.ToString()); }
		template<>static float ToSingle(const float &o) { return o; }

		template<class T>static short ToInt16(const T &o) { return Int16::Parse(o.ToString()); }
		template<>static short ToInt16(const short &o) { return o; }

		template<class T>static int ToInt32(const T &o) { return Int32::Parse(o.ToString()); }
		template<>static int ToInt32(const int &o) { return o; }

		template<class T>static long long ToInt64(const T &o) { return Int64::Parse(o.ToString()); }
		template<>static long long ToInt64(const long long &o) { return o; }

		template<class T>static unsigned short ToUInt16(const T &o) { return UInt16::Parse(o.ToString()); }
		template<>static unsigned short ToUInt16(const short & o) { return o; }

		template<class T>static unsigned int ToUInt32(const T &o) { return UInt32::Parse(o.ToString()); }
		template<>static unsigned int ToUInt32(const unsigned int &o) { return o; }

		template<class T>static unsigned long long ToUInt64(const T &o) { return UInt64::Parse(o.ToString()); }
		template<>static unsigned long long ToUInt64(const long long &o) { return o; }

/*

#include "base64.h"
#include <iostream>

static const std::string base64_chars = 
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";


static inline bool is_base64(unsigned char c) {
  return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len) {
  std::string ret;
  int i = 0;
  int j = 0;
  unsigned char char_array_3[3];
  unsigned char char_array_4[4];

  while (in_len--) {
    char_array_3[i++] = *(bytes_to_encode++);
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for(i = 0; (i <4) ; i++)
        ret += base64_chars[char_array_4[i]];
      i = 0;
    }
  }

  if (i)
  {
    for(j = i; j < 3; j++)
      char_array_3[j] = '\0';

    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    char_array_4[3] = char_array_3[2] & 0x3f;

    for (j = 0; (j < i + 1); j++)
      ret += base64_chars[char_array_4[j]];

    while((i++ < 3))
      ret += '=';

  }

  return ret;

}

std::string base64_decode(std::string const& encoded_string) {
  int in_len = encoded_string.size();
  int i = 0;
  int j = 0;
  int in_ = 0;
  unsigned char char_array_4[4], char_array_3[3];
  std::string ret;

  while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
    char_array_4[i++] = encoded_string[in_]; in_++;
    if (i ==4) {
      for (i = 0; i <4; i++)
        char_array_4[i] = base64_chars.find(char_array_4[i]);

      char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

      for (i = 0; (i < 3); i++)
        ret += char_array_3[i];
      i = 0;
    }
  }

  if (i) {
    for (j = i; j <4; j++)
      char_array_4[j] = 0;

    for (j = 0; j <4; j++)
      char_array_4[j] = base64_chars.find(char_array_4[j]);

    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

    for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
  }

  return ret;
}

#include <string>

std::string base64_encode(unsigned char const* , unsigned int len);
std::string base64_decode(std::string const& s);




#include "base64.h"
#include <iostream>

int main() {
  const std::string s = "ADP GmbH\nAnalyse Design & Programmierung\nGesellschaft mit beschränkter Haftung" ;

  std::string encoded = base64_encode(reinterpret_cast<const unsigned char*>(s.c_str()), s.length());
  std::string decoded = base64_decode(encoded);

  std::cout << "encoded: " << encoded << std::endl;
  std::cout << "decoded: " << decoded << std::endl;

  return 0;
}


*/
    
 
    };
}

