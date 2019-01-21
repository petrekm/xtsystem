#define CALL_ONCE_BLOCK static int _dummyNumberOfCalls = 1; if (1==_dummyNumberOfCalls++) 

#define NUMITEMS_IMPL(NUM) static const int numItems() {return NUM; }

#define ENUM_IMPL(NAME, T1) \
  public:\
    static enumType Parse(const XTSystem::String &strName) { for (int i=0;i<numItems();i++) if (String::Compare(strName,GetNames()[i])==0) return GetValues()[i]; throw Exception(_T("Bad parsing of enum")); } \
    String ToString() const {  for (int i=0;i<numItems();i++) if (GetValues()[i] == value) return GetNames()[i]; return _T(""); } \
    NAME(): value((enumType)T1) {} \
    NAME & operator=(enumType v) { value = v; return *this;} \
    NAME(enumType v) { value = v; } \
    bool operator!= (const NAME& rhs) { return value != rhs.value; }\
    bool operator!= (const enumType& rhs) { return value != rhs; }\
    bool operator== (const NAME& rhs) { return value == rhs.value; }\
    bool operator== (const enumType& rhs) { return value == rhs; }\
    operator int() { return (int)value; } \
    int ToInt32() const {  return (int)value; } \
    int ToLong() const {  return (long)value; } \
  private: \
    enumType value; \


#define FLAGS_ENUM_IMPL(NAME, T1) \
  public:\
    static enumType Parse(const XTSystem::String &strName) { for (int i=0;i<numItems();i++) if (String::Compare(strName,GetNames()[i])==0) return GetValues()[i]; throw Exception(_T("Bad parsing of enum")); } \
    String ToString() const {  String s; int k=0; for (int i=0;i<numItems();i++) { int a = GetValues()[i]; int b = value; if ((a == b) || ((a != 0) && ((a & b)==a))) { if (k>0) s+=_T("|"); s+= GetNames()[i]; ++k;} }  return s; } \
    NAME(): value((enumType)T1) {} \
    NAME & operator=(enumType v) { value = v; return *this;} \
    NAME & operator=( int v ) { value = (enumType)v; return *this;} \
    NAME(enumType v) { value = v; } \
    NAME(long v) { value = (enumType)v; } \
    friend bool operator!= (const NAME& lhs, const NAME& rhs) { return lhs.value != rhs.value; }\
    friend bool operator!= (const NAME& lhs, const enumType& rhs) { return lhs.value != rhs; }\
    friend bool operator!= (const enumType& lhs, const NAME& rhs) { return lhs != rhs.value; }\
    friend bool operator== (const NAME& lhs, const NAME& rhs) { return lhs.value == rhs.value; }\
    friend bool operator== (const NAME& lhs, const enumType& rhs) { return lhs.value == rhs; }\
    friend bool operator== (const enumType& lhs, const NAME& rhs) { return lhs == rhs.value; }\
    friend long operator& (const NAME& lhs, const NAME& rhs) { return lhs.value & rhs.value; }\
    friend long operator| (const NAME& lhs, const NAME& rhs) { return lhs.value | rhs.value; }\
    bool ContainsFlag(const NAME& rhs) const { int a = (int)value; int b = (int)rhs.value; return (b!=0) && (a&b)==b;}\
    int ToInt32() const {  return (int)value; } \
    int ToLong() const {  return (long)value; } \
  private: \
    enumType value; \


#define enum_1(NAME,T1) \
class NAME \
{ \
  public: \
    NUMITEMS_IMPL(1) \
    typedef enum { T1 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1)}; v = std::vector<String>(args, args + numItems()); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1}; v = std::vector<enumType>(args, args + numItems()); } return v; }\
    ENUM_IMPL(NAME,T1) \
}; \


#define enum_2(NAME,T1,T2) \
class NAME \
{ \
  public: \
    NUMITEMS_IMPL(2) \
    typedef enum { T1,T2 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2)}; v = std::vector<String>(args, args + numItems()); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2}; v = std::vector<enumType>(args, args + numItems()); } return v; }\
    ENUM_IMPL(NAME,T1) \
}; \


#define enum_3(NAME,T1,T2,T3) \
class NAME \
{ \
  public: \
    NUMITEMS_IMPL(3) \
    typedef enum { T1,T2,T3 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3)}; v = std::vector<String>(args, args + numItems()); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3}; v = std::vector<enumType>(args, args + numItems()); } return v; }\
    ENUM_IMPL(NAME,T1) \
}; \


#define enum_4(NAME,T1,T2,T3,T4) \
class NAME \
{ \
  public: \
    NUMITEMS_IMPL(4) \
    typedef enum { T1,T2,T3,T4 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3),_T(#T4)}; v = std::vector<String>(args, args + numItems()); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3,T4}; v = std::vector<enumType>(args, args + numItems()); } return v; }\
    ENUM_IMPL(NAME,T1) \
}; \


#define enum_5(NAME,T1,T2,T3,T4,T5) \
class NAME \
{ \
  public: \
    NUMITEMS_IMPL(5) \
    typedef enum { T1,T2,T3,T4,T5 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3),_T(#T4),_T(#T5)}; v = std::vector<String>(args, args + numItems()); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3,T4,T5}; v = std::vector<enumType>(args, args + numItems()); } return v; }\
    ENUM_IMPL(NAME,T1) \
}; \


#define enum_6(NAME,T1,T2,T3,T4,T5,T6) \
class NAME \
{ \
  public: \
    NUMITEMS_IMPL(6) \
    typedef enum { T1,T2,T3,T4,T5,T6 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3),_T(#T4),_T(#T5),_T(#T6)}; v = std::vector<String>(args, args + numItems()); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3,T4,T5,T6}; v = std::vector<enumType>(args, args + numItems()); } return v; }\
    ENUM_IMPL(NAME,T1) \
}; \


#define enum_7(NAME,T1,T2,T3,T4,T5,T6,T7) \
class NAME \
{ \
  public: \
    NUMITEMS_IMPL(7) \
    typedef enum { T1,T2,T3,T4,T5,T6,T7 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3),_T(#T4),_T(#T5),_T(#T6),_T(#T7)}; v = std::vector<String>(args, args + numItems()); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3,T4,T5,T6,T7}; v = std::vector<enumType>(args, args + numItems()); } return v; }\
    ENUM_IMPL(NAME,T1) \
}; \


#define enum_8(NAME,T1,T2,T3,T4,T5,T6,T7,T8) \
class NAME \
{ \
  public: \
    NUMITEMS_IMPL(8) \
    typedef enum { T1,T2,T3,T4,T5,T6,T7,T8 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3),_T(#T4),_T(#T5),_T(#T6),_T(#T7),_T(#T8)}; v = std::vector<String>(args, args + numItems()); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3,T4,T5,T6,T7,T8}; v = std::vector<enumType>(args, args + numItems()); } return v; }\
    ENUM_IMPL(NAME,T1) \
}; \


#define enum_9(NAME,T1,T2,T3,T4,T5,T6,T7,T8,T9) \
class NAME \
{ \
  public: \
    NUMITEMS_IMPL(9) \
    typedef enum { T1,T2,T3,T4,T5,T6,T7,T8,T9 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3),_T(#T4),_T(#T5),_T(#T6),_T(#T7),_T(#T8),_T(#T9)}; v = std::vector<String>(args, args + numItems()); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3,T4,T5,T6,T7,T8,T9}; v = std::vector<enumType>(args, args + numItems()); } return v; }\
    ENUM_IMPL(NAME,T1) \
}; \


#define enum_10(NAME,T1,T2,T3,T4,T5,T6,T7,T8,T9,T10) \
class NAME \
{ \
  public: \
    NUMITEMS_IMPL(10) \
    typedef enum { T1,T2,T3,T4,T5,T6,T7,T8,T9,T10 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3),_T(#T4),_T(#T5),_T(#T6),_T(#T7),_T(#T8),_T(#T9),_T(#T10)}; v = std::vector<String>(args, args + numItems()); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3,T4,T5,T6,T7,T8,T9,T10}; v = std::vector<enumType>(args, args + numItems()); } return v; }\
    ENUM_IMPL(NAME,T1) \
}; \

#define enum_16(NAME,T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14,T15,T16) \
class NAME \
{ \
  public: \
    NUMITEMS_IMPL(16) \
    typedef enum { T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14,T15,T16 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3),_T(#T4),_T(#T5),_T(#T6),_T(#T7),_T(#T8),_T(#T9),_T(#T10),_T(#T11),_T(#T12),_T(#T13),_T(#T14),_T(#T15),_T(#T16)}; v = std::vector<String>(args, args + numItems()); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14,T15,T16}; v = std::vector<enumType>(args, args + numItems()); } return v; }\
    ENUM_IMPL(NAME,T1) \
}; \

#define enum_1_manual(NAME,T1,V1) \
class NAME \
{ \
  public: \
    NUMITEMS_IMPL(1) \
    typedef enum { T1=V1 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1)}; v = std::vector<String>(args, args + numItems()); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1}; v = std::vector<enumType>(args, args + numItems()); } return v; }\
    ENUM_IMPL(NAME,T1) \
}; \


#define enum_2_manual(NAME,T1,V1,T2,V2) \
class NAME \
{ \
  public: \
    NUMITEMS_IMPL(2) \
    typedef enum { T1=V1,T2=V2 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2)}; v = std::vector<String>(args, args + numItems()); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2}; v = std::vector<enumType>(args, args + numItems()); } return v; }\
    ENUM_IMPL(NAME,T1) \
}; \


#define enum_3_manual(NAME,T1,V1,T2,V2,T3,V3) \
class NAME \
{ \
  public: \
    NUMITEMS_IMPL(3) \
    typedef enum { T1=V1,T2=V2,T3=V3 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3)}; v = std::vector<String>(args, args + numItems()); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3}; v = std::vector<enumType>(args, args + numItems()); } return v; }\
    ENUM_IMPL(NAME,T1) \
}; \


#define enum_4_manual(NAME,T1,V1,T2,V2,T3,V3,T4,V4) \
class NAME \
{ \
  public: \
    NUMITEMS_IMPL(4) \
    typedef enum { T1=V1,T2=V2,T3=V3,T4=V4 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3),_T(#T4)}; v = std::vector<String>(args, args + numItems()); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3,T4}; v = std::vector<enumType>(args, args + numItems()); } return v; }\
    ENUM_IMPL(NAME,T1) \
}; \


#define enum_5_manual(NAME,T1,V1,T2,V2,T3,V3,T4,V4,T5,V5) \
class NAME \
{ \
  public: \
    NUMITEMS_IMPL(5) \
    typedef enum { T1=V1,T2=V2,T3=V3,T4=V4,T5=V5 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3),_T(#T4),_T(#T5)}; v = std::vector<String>(args, args + numItems()); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3,T4,T5}; v = std::vector<enumType>(args, args + numItems()); } return v; }\
    ENUM_IMPL(NAME,T1) \
}; \


#define enum_6_manual(NAME,T1,V1,T2,V2,T3,V3,T4,V4,T5,V5,T6,V6) \
class NAME \
{ \
  public: \
    NUMITEMS_IMPL(6) \
    typedef enum { T1=V1,T2=V2,T3=V3,T4=V4,T5=V5,T6=V6 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3),_T(#T4),_T(#T5),_T(#T6)}; v = std::vector<String>(args, args + numItems()); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3,T4,T5,T6}; v = std::vector<enumType>(args, args + numItems()); } return v; }\
    ENUM_IMPL(NAME,T1) \
}; \


#define enum_7_manual(NAME,T1,V1,T2,V2,T3,V3,T4,V4,T5,V5,T6,V6,T7,V7) \
class NAME \
{ \
  public: \
    NUMITEMS_IMPL(7) \
    typedef enum { T1=V1,T2=V2,T3=V3,T4=V4,T5=V5,T6=V6,T7=V7 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3),_T(#T4),_T(#T5),_T(#T6),_T(#T7)}; v = std::vector<String>(args, args + numItems()); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3,T4,T5,T6,T7}; v = std::vector<enumType>(args, args + numItems()); } return v; }\
    ENUM_IMPL(NAME,T1) \
}; \


#define enum_8_manual(NAME,T1,V1,T2,V2,T3,V3,T4,V4,T5,V5,T6,V6,T7,V7,T8,V8) \
class NAME \
{ \
  public: \
    NUMITEMS_IMPL(8) \
    typedef enum { T1=V1,T2=V2,T3=V3,T4=V4,T5=V5,T6=V6,T7=V7,T8=V8 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3),_T(#T4),_T(#T5),_T(#T6),_T(#T7),_T(#T8)}; v = std::vector<String>(args, args + numItems()); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3,T4,T5,T6,T7,T8}; v = std::vector<enumType>(args, args + numItems()); } return v; }\
    ENUM_IMPL(NAME,T1) \
}; \


#define enum_9_manual(NAME,T1,V1,T2,V2,T3,V3,T4,V4,T5,V5,T6,V6,T7,V7,T8,V8,T9,V9) \
class NAME \
{ \
  public: \
    NUMITEMS_IMPL(9) \
    typedef enum { T1=V1,T2=V2,T3=V3,T4=V4,T5=V5,T6=V6,T7=V7,T8=V8,T9=V9 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3),_T(#T4),_T(#T5),_T(#T6),_T(#T7),_T(#T8),_T(#T9)}; v = std::vector<String>(args, args + numItems()); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3,T4,T5,T6,T7,T8,T9}; v = std::vector<enumType>(args, args + numItems()); } return v; }\
    ENUM_IMPL(NAME,T1) \
}; \


#define enum_10_manual(NAME,T1,V1,T2,V2,T3,V3,T4,V4,T5,V5,T6,V6,T7,V7,T8,V8,T9,V9,T10,V10) \
class NAME \
{ \
  public: \
    NUMITEMS_IMPL(10) \
    typedef enum { T1=V1,T2=V2,T3=V3,T4=V4,T5=V5,T6=V6,T7=V7,T8=V8,T9=V9,T10=V10 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3),_T(#T4),_T(#T5),_T(#T6),_T(#T7),_T(#T8),_T(#T9),_T(#T10)}; v = std::vector<String>(args, args + numItems()); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3,T4,T5,T6,T7,T8,T9,T10}; v = std::vector<enumType>(args, args + numItems()); } return v; }\
    ENUM_IMPL(NAME,T1) \
}; \


#define flags_1(NAME,T1) \
class NAME \
{ \
  public: \
    NUMITEMS_IMPL(1) \
    typedef enum { T1 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1)}; v = std::vector<String>(args, args + numItems()); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1}; v = std::vector<enumType>(args, args + numItems()); } return v; }\
    FLAGS_ENUM_IMPL(NAME,T1) \
}; \


#define flags_2(NAME,T1,T2) \
class NAME \
{ \
  public: \
    NUMITEMS_IMPL(2) \
    typedef enum { T1,T2 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2)}; v = std::vector<String>(args, args + numItems()); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2}; v = std::vector<enumType>(args, args + numItems()); } return v; }\
    FLAGS_ENUM_IMPL(NAME,T1) \
}; \


#define flags_3(NAME,T1,T2,T3) \
class NAME \
{ \
  public: \
    NUMITEMS_IMPL(3) \
    typedef enum { T1,T2,T3 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3)}; v = std::vector<String>(args, args + numItems()); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3}; v = std::vector<enumType>(args, args + numItems()); } return v; }\
    FLAGS_ENUM_IMPL(NAME,T1) \
}; \


#define flags_4(NAME,T1,T2,T3,T4) \
class NAME \
{ \
  public: \
    NUMITEMS_IMPL(4) \
    typedef enum { T1,T2,T3,T4 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3),_T(#T4)}; v = std::vector<String>(args, args + numItems()); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3,T4}; v = std::vector<enumType>(args, args + numItems()); } return v; }\
    FLAGS_ENUM_IMPL(NAME,T1) \
}; \


#define flags_5(NAME,T1,T2,T3,T4,T5) \
class NAME \
{ \
  public: \
    NUMITEMS_IMPL(5) \
    typedef enum { T1,T2,T3,T4,T5 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3),_T(#T4),_T(#T5)}; v = std::vector<String>(args, args + numItems()); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3,T4,T5}; v = std::vector<enumType>(args, args + numItems()); } return v; }\
    FLAGS_ENUM_IMPL(NAME,T1) \
}; \


#define flags_6(NAME,T1,T2,T3,T4,T5,T6) \
class NAME \
{ \
  public: \
    NUMITEMS_IMPL(6) \
    typedef enum { T1,T2,T3,T4,T5,T6 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3),_T(#T4),_T(#T5),_T(#T6)}; v = std::vector<String>(args, args + numItems()); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3,T4,T5,T6}; v = std::vector<enumType>(args, args + numItems()); } return v; }\
    FLAGS_ENUM_IMPL(NAME,T1) \
}; \


#define flags_7(NAME,T1,T2,T3,T4,T5,T6,T7) \
class NAME \
{ \
  public: \
    NUMITEMS_IMPL(7) \
    typedef enum { T1,T2,T3,T4,T5,T6,T7 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3),_T(#T4),_T(#T5),_T(#T6),_T(#T7)}; v = std::vector<String>(args, args + numItems()); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3,T4,T5,T6,T7}; v = std::vector<enumType>(args, args + numItems()); } return v; }\
    FLAGS_ENUM_IMPL(NAME,T1) \
}; \


#define flags_8(NAME,T1,T2,T3,T4,T5,T6,T7,T8) \
class NAME \
{ \
  public: \
    NUMITEMS_IMPL(8) \
    typedef enum { T1,T2,T3,T4,T5,T6,T7,T8 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3),_T(#T4),_T(#T5),_T(#T6),_T(#T7),_T(#T8)}; v = std::vector<String>(args, args + numItems()); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3,T4,T5,T6,T7,T8}; v = std::vector<enumType>(args, args + numItems()); } return v; }\
    FLAGS_ENUM_IMPL(NAME,T1) \
}; \


#define flags_9(NAME,T1,T2,T3,T4,T5,T6,T7,T8,T9) \
class NAME \
{ \
  public: \
    NUMITEMS_IMPL(9) \
    typedef enum { T1,T2,T3,T4,T5,T6,T7,T8,T9 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3),_T(#T4),_T(#T5),_T(#T6),_T(#T7),_T(#T8),_T(#T9)}; v = std::vector<String>(args, args + numItems()); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3,T4,T5,T6,T7,T8,T9}; v = std::vector<enumType>(args, args + numItems()); } return v; }\
    FLAGS_ENUM_IMPL(NAME,T1) \
}; \


#define flags_10(NAME,T1,T2,T3,T4,T5,T6,T7,T8,T9,T10) \
class NAME \
{ \
  public: \
    NUMITEMS_IMPL(10) \
    typedef enum { T1,T2,T3,T4,T5,T6,T7,T8,T9,T10 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3),_T(#T4),_T(#T5),_T(#T6),_T(#T7),_T(#T8),_T(#T9),_T(#T10)}; v = std::vector<String>(args, args + numItems()); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3,T4,T5,T6,T7,T8,T9,T10}; v = std::vector<enumType>(args, args + numItems()); } return v; }\
    FLAGS_ENUM_IMPL(NAME,T1) \
}; \


#define flags_1_manual(NAME,T1,V1) \
class NAME \
{ \
  public: \
    NUMITEMS_IMPL(1) \
    typedef enum { T1=V1 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1)}; v = std::vector<String>(args, args + numItems()); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1}; v = std::vector<enumType>(args, args + numItems()); } return v; }\
    FLAGS_ENUM_IMPL(NAME,T1) \
}; \


#define flags_2_manual(NAME,T1,V1,T2,V2) \
class NAME \
{ \
  public: \
    NUMITEMS_IMPL(2) \
    typedef enum { T1=V1,T2=V2 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2)}; v = std::vector<String>(args, args + numItems()); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2}; v = std::vector<enumType>(args, args + numItems()); } return v; }\
    FLAGS_ENUM_IMPL(NAME,T1) \
}; \


#define flags_3_manual(NAME,T1,V1,T2,V2,T3,V3) \
class NAME \
{ \
  public: \
    NUMITEMS_IMPL(3) \
    typedef enum { T1=V1,T2=V2,T3=V3 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3)}; v = std::vector<String>(args, args + numItems()); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3}; v = std::vector<enumType>(args, args + numItems()); } return v; }\
    FLAGS_ENUM_IMPL(NAME,T1) \
}; \


#define flags_4_manual(NAME,T1,V1,T2,V2,T3,V3,T4,V4) \
class NAME \
{ \
  public: \
    NUMITEMS_IMPL(4) \
    typedef enum { T1=V1,T2=V2,T3=V3,T4=V4 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3),_T(#T4)}; v = std::vector<String>(args, args + numItems()); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3,T4}; v = std::vector<enumType>(args, args + numItems()); } return v; }\
    FLAGS_ENUM_IMPL(NAME,T1) \
}; \


#define flags_5_manual(NAME,T1,V1,T2,V2,T3,V3,T4,V4,T5,V5) \
class NAME \
{ \
  public: \
    NUMITEMS_IMPL(5) \
    typedef enum { T1=V1,T2=V2,T3=V3,T4=V4,T5=V5 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3),_T(#T4),_T(#T5)}; v = std::vector<String>(args, args + numItems()); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3,T4,T5}; v = std::vector<enumType>(args, args + numItems()); } return v; }\
    FLAGS_ENUM_IMPL(NAME,T1) \
}; \


#define flags_6_manual(NAME,T1,V1,T2,V2,T3,V3,T4,V4,T5,V5,T6,V6) \
class NAME \
{ \
  public: \
    NUMITEMS_IMPL(6) \
    typedef enum { T1=V1,T2=V2,T3=V3,T4=V4,T5=V5,T6=V6 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3),_T(#T4),_T(#T5),_T(#T6)}; v = std::vector<String>(args, args + numItems()); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3,T4,T5,T6}; v = std::vector<enumType>(args, args + numItems()); } return v; }\
    FLAGS_ENUM_IMPL(NAME,T1) \
}; \


#define flags_7_manual(NAME,T1,V1,T2,V2,T3,V3,T4,V4,T5,V5,T6,V6,T7,V7) \
class NAME \
{ \
  public: \
    NUMITEMS_IMPL(7) \
    typedef enum { T1=V1,T2=V2,T3=V3,T4=V4,T5=V5,T6=V6,T7=V7 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3),_T(#T4),_T(#T5),_T(#T6),_T(#T7)}; v = std::vector<String>(args, args + numItems()); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3,T4,T5,T6,T7}; v = std::vector<enumType>(args, args + numItems()); } return v; }\
    FLAGS_ENUM_IMPL(NAME,T1) \
}; \


#define flags_8_manual(NAME,T1,V1,T2,V2,T3,V3,T4,V4,T5,V5,T6,V6,T7,V7,T8,V8) \
class NAME \
{ \
  public: \
    NUMITEMS_IMPL(8) \
    typedef enum { T1=V1,T2=V2,T3=V3,T4=V4,T5=V5,T6=V6,T7=V7,T8=V8 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3),_T(#T4),_T(#T5),_T(#T6),_T(#T7),_T(#T8)}; v = std::vector<String>(args, args + numItems()); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3,T4,T5,T6,T7,T8}; v = std::vector<enumType>(args, args + numItems()); } return v; }\
    FLAGS_ENUM_IMPL(NAME,T1) \
}; \


#define flags_9_manual(NAME,T1,V1,T2,V2,T3,V3,T4,V4,T5,V5,T6,V6,T7,V7,T8,V8,T9,V9) \
class NAME \
{ \
  public: \
    NUMITEMS_IMPL(9) \
    typedef enum { T1=V1,T2=V2,T3=V3,T4=V4,T5=V5,T6=V6,T7=V7,T8=V8,T9=V9 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3),_T(#T4),_T(#T5),_T(#T6),_T(#T7),_T(#T8),_T(#T9)}; v = std::vector<String>(args, args + numItems()); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3,T4,T5,T6,T7,T8,T9}; v = std::vector<enumType>(args, args + numItems()); } return v; }\
    FLAGS_ENUM_IMPL(NAME,T1) \
}; \


#define flags_10_manual(NAME,T1,V1,T2,V2,T3,V3,T4,V4,T5,V5,T6,V6,T7,V7,T8,V8,T9,V9,T10,V10) \
class NAME \
{ \
  public: \
    NUMITEMS_IMPL(10) \
    typedef enum { T1=V1,T2=V2,T3=V3,T4=V4,T5=V5,T6=V6,T7=V7,T8=V8,T9=V9,T10=V10 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3),_T(#T4),_T(#T5),_T(#T6),_T(#T7),_T(#T8),_T(#T9),_T(#T10)}; v = std::vector<String>(args, args + numItems()); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3,T4,T5,T6,T7,T8,T9,T10}; v = std::vector<enumType>(args, args + numItems()); } return v; }\
    FLAGS_ENUM_IMPL(NAME,T1) \
}; \


