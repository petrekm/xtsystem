#ifdef _MSC_VER

    #if (_MSC_VER < 1600)
    typedef __int32 int32_t;
    typedef unsigned __int32 uint32_t;
    typedef __int64 int64_t;
    typedef unsigned __int64 uint64_t;
    #else
    #include <stdint.h>
    #endif
#endif

#include <cassert>
#include <climits>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <string>
#include <sstream>
#include <cctype>
#include <math.h>
#include <cstdarg>
#include <vector>
#include <time.h>
#include <windows.h>
#include <tchar.h>

#ifdef __GNUC__
# define FMT_GCC_VERSION (__GNUC__ * 100 + __GNUC_MINOR__)
# define FMT_GCC_EXTENSION __extension__
#else
# define FMT_GCC_EXTENSION
#endif

// Compatibility with compilers other than clang.
#ifndef __has_feature
# define __has_feature(x) 0
# define __has_builtin(x) 0
#endif

#ifndef FMT_USE_INITIALIZER_LIST
# define FMT_USE_INITIALIZER_LIST \
   (__has_feature(cxx_generalized_initializers) || \
       (FMT_GCC_VERSION >= 404 && __cplusplus >= 201103) || _MSC_VER >= 1800)
#endif

#ifndef FMT_USE_VARIADIC_TEMPLATES
# define FMT_USE_VARIADIC_TEMPLATES \
   (__has_feature(cxx_variadic_templates) || \
       (FMT_GCC_VERSION >= 404 && __cplusplus >= 201103) || _MSC_VER >= 1800)
#endif

#if FMT_USE_INITIALIZER_LIST
# include <initializer_list>
#endif

// Define FMT_USE_NOEXCEPT to make format use noexcept (C++11 feature).
#if FMT_USE_NOEXCEPT || __has_feature(cxx_noexcept) || \
  (FMT_GCC_VERSION >= 408 && __cplusplus >= 201103)
# define FMT_NOEXCEPT(expr) noexcept(expr)
#else
# define FMT_NOEXCEPT(expr)
#endif

#if _MSC_VER
# pragma warning(push)
# pragma warning(disable: 4521) // 'class' : multiple copy constructors specified
#endif

namespace XTSystem_fmt {

// Fix the warning about long long on older versions of GCC
// that don't support the diagnostic pragma.
FMT_GCC_EXTENSION typedef long long LongLong;
FMT_GCC_EXTENSION typedef unsigned long long ULongLong;

namespace internal {

#if _SECURE_SCL
template <typename T>
inline stdext::checked_array_iterator<T*> CheckPtr(T *ptr, std::size_t size) {
  return stdext::checked_array_iterator<T*>(ptr, size);
}
#else
template <typename T>
inline T *CheckPtr(T *ptr, std::size_t) { return ptr; }
#endif

// A simple array for POD types with the first SIZE elements stored in
// the object itself. It supports a subset of std::vector's operations.
template <typename T, std::size_t SIZE>
class Array {
 private:
  std::size_t size_;
  std::size_t capacity_;
  T *ptr_;
  T data_[SIZE];

  void Grow(std::size_t size);

  // Do not implement!
  Array(const Array &);
  void operator=(const Array &);

 public:
  Array() : size_(0), capacity_(SIZE), ptr_(data_) {}
  ~Array() {
    if (ptr_ != data_) delete [] ptr_;
  }

  // Returns the size of this array.
  std::size_t size() const { return size_; }

  // Returns the capacity of this array.
  std::size_t capacity() const { return capacity_; }

  // Resizes the array. If T is a POD type new elements are not initialized.
  void resize(std::size_t new_size) {
    if (new_size > capacity_)
      Grow(new_size);
    size_ = new_size;
  }

  void reserve(std::size_t capacity) {
    if (capacity > capacity_)
      Grow(capacity);
  }

  void clear() { size_ = 0; }

  void push_back(const T &value) {
    if (size_ == capacity_)
      Grow(size_ + 1);
    ptr_[size_++] = value;
  }

  // Appends data to the end of the array.
  void append(const T *begin, const T *end);

  T &operator[](std::size_t index) { return ptr_[index]; }
  const T &operator[](std::size_t index) const { return ptr_[index]; }
};

template <typename T, std::size_t SIZE>
void Array<T, SIZE>::Grow(std::size_t size) {
  capacity_ = (std::max)(size, capacity_ + capacity_ / 2);
  T *p = new T[capacity_];
  std::copy(ptr_, ptr_ + size_, CheckPtr(p, capacity_));
  if (ptr_ != data_)
    delete [] ptr_;
  ptr_ = p;
}

template <typename T, std::size_t SIZE>
void Array<T, SIZE>::append(const T *begin, const T *end) {
  std::ptrdiff_t num_elements = end - begin;
  if (size_ + num_elements > capacity_)
    Grow(num_elements);
  std::copy(begin, end, CheckPtr(ptr_, capacity_) + size_);
  size_ += num_elements;
}

template <typename Char>
class CharTraits;

template <typename Char>
class BasicCharTraits {
 public:
#if _SECURE_SCL
  typedef stdext::checked_array_iterator<Char*> CharPtr;
#else
  typedef Char *CharPtr;
#endif
};

template <>
class CharTraits<char> : public BasicCharTraits<char> {
 private:
  // Conversion from wchar_t to char is not supported.
  static char ConvertChar(wchar_t);

 public:
  typedef const wchar_t *UnsupportedStrType;

  static char ConvertChar(char value) { return value; }

  template <typename T>
  static int FormatFloat(char *buffer, std::size_t size,
      const char *format, unsigned width, int precision, T value);
};

template <>
class CharTraits<wchar_t> : public BasicCharTraits<wchar_t> {
 public:
  typedef const char *UnsupportedStrType;

  static wchar_t ConvertChar(char value) { return value; }
  static wchar_t ConvertChar(wchar_t value) { return value; }

  template <typename T>
  static int FormatFloat(wchar_t *buffer, std::size_t size,
      const wchar_t *format, unsigned width, int precision, T value);
};

// Selects uint32_t if FitsIn32Bits is true, uint64_t otherwise.
template <bool FitsIn32Bits>
struct TypeSelector { typedef uint32_t Type; };

template <>
struct TypeSelector<false> { typedef uint64_t Type; };

// Checks if a number is negative - used to avoid warnings.
template <bool IsSigned>
struct SignChecker {
  template <typename T>
  static bool IsNegative(T) { return false; }
};

template <>
struct SignChecker<true> {
  template <typename T>
  static bool IsNegative(T value) { return value < 0; }
};

// Returns true if value is negative, false otherwise.
// Same as (value < 0) but doesn't produce warnings if T is an unsigned type.
template <typename T>
inline bool IsNegative(T value) {
  return SignChecker<std::numeric_limits<T>::is_signed>::IsNegative(value);
}

template <typename T>
struct IntTraits {
  // Smallest of uint32_t and uint64_t that is large enough to represent
  // all values of T.
  typedef typename
    TypeSelector<std::numeric_limits<T>::digits <= 32>::Type MainType;
};

template <typename T>
struct IsLongDouble { enum {VALUE = 0}; };

template <>
struct IsLongDouble<long double> { enum {VALUE = 1}; };



inline unsigned CountDigits(uint64_t n) {
  unsigned count = 1;
  for (;;) {
    // Integer division is slow so do it for a group of four digits instead
    // of for every digit. The idea comes from the talk by Alexandrescu
    // "Three Optimization Tips for C++". See speed-test for a comparison.
    if (n < 10) return count;
    if (n < 100) return count + 1;
    if (n < 1000) return count + 2;
    if (n < 10000) return count + 3;
    n /= 10000u;
    count += 4;
  }
}


struct Constants
{
//#define FMT_POWERS_OF_10(factor) \
//  factor * 10, \
//  factor * 100, \
//  factor * 1000, \
//  factor * 10000, \
//  factor * 100000, \
//  factor * 1000000, \
//  factor * 10000000, \
//  factor * 100000000, \
//  factor * 1000000000
//
//static const uint32_t *POWERS_OF_10_32()
//{
//
// static const uint32_t values[] = {0, FMT_POWERS_OF_10(1)};
// return values;
//
//}
//static const uint64_t *POWERS_OF_10_64()
//{
//	static const uint64_t values[] = {
//  0,
//  FMT_POWERS_OF_10(1),
//  FMT_POWERS_OF_10(fmt::ULongLong(1000000000)),
//  // Multiply several constants instead of using a single long long constants
//  // to avoid warnings about C++98 not supporting long long.
//  fmt::ULongLong(1000000000) * fmt::ULongLong(1000000000) * 10
//};	
//	return values;
//}

static const char *DIGITS()
{
	static const char values[] =
    "0001020304050607080910111213141516171819"
    "2021222324252627282930313233343536373839"
    "4041424344454647484950515253545556575859"
    "6061626364656667686970717273747576777879"
    "8081828384858687888990919293949596979899";
	return values;
}

};

template <typename Char>
class FormatterProxy;

// Formats a decimal unsigned integer value writing into buffer.
template <typename UInt, typename Char>
void FormatDecimal(Char *buffer, UInt value, unsigned num_digits) {
  --num_digits;
  while (value >= 100) {
    // Integer division is slow so do it for a group of two digits instead
    // of for every digit. The idea comes from the talk by Alexandrescu
    // "Three Optimization Tips for C++". See speed-test for a comparison.
    unsigned index = (value % 100) * 2;
    value /= 100;
    buffer[num_digits] = internal::Constants::DIGITS()[index + 1];
    buffer[num_digits - 1] = internal::Constants::DIGITS()[index];
    num_digits -= 2;
  }
  if (value < 10) {
    *buffer = static_cast<char>('0' + value);
    return;
  }
  unsigned index = static_cast<unsigned>(value * 2);
  buffer[1] = internal::Constants::DIGITS()[index + 1];
  buffer[0] = internal::Constants::DIGITS()[index];
}
}

/**
  \rst
  A string reference. It can be constructed from a C string, ``std::string``
  or as a result of a formatting operation. It is most useful as a parameter
  type to allow passing different types of strings in a function, for example::

    Formatter<> Format(StringRef format);

    Format("{}") << 42;
    Format(std::string("{}")) << 42;
    Format(Format("{{}}")) << 42;
  \endrst
 */
template <typename Char>
class BasicStringRef {
 private:
  const Char *data_;
  mutable std::size_t size_;

 public:
  /**
    Constructs a string reference object from a C string and a size.
    If *size* is zero, which is the default, the size is computed with
    `strlen`.
   */
  BasicStringRef(const Char *s, std::size_t size = 0) : data_(s), size_(size) {}

  /**
    Constructs a string reference from an `std::string` object.
   */
  BasicStringRef(const std::basic_string<Char> &s)
  : data_(s.c_str()), size_(s.size()) {}

  /**
    Converts a string reference to an `std::string` object.
   */
  operator std::basic_string<Char>() const {
    return std::basic_string<Char>(data_, size());
  }

  /**
    Returns the pointer to a C string.
   */
  const Char *c_str() const { return data_; }

  /**
    Returns the string size.
   */
  std::size_t size() const {
    if (size_ == 0) size_ = std::char_traits<Char>::length(data_);
    return size_;
  }
};

typedef BasicStringRef<char> StringRef;
typedef BasicStringRef<wchar_t> WStringRef;

class FormatError : public std::runtime_error {
 public:
  explicit FormatError(const std::string &message)
  : std::runtime_error(message) {}
};

enum Alignment {
  ALIGN_DEFAULT, ALIGN_LEFT, ALIGN_RIGHT, ALIGN_CENTER, ALIGN_NUMERIC
};

// Flags.
enum FormatSpecFlags{ SIGN_FLAG = 1, PLUS_FLAG = 2, HASH_FLAG = 4 };

// An empty format specifier.
struct EmptySpec {};

// A type specifier.
template <char TYPE>
struct TypeSpec : EmptySpec {
  Alignment align() const { return ALIGN_DEFAULT; }
  unsigned width() const { return 0; }

  bool sign_flag() const { return false; }
  bool plus_flag() const { return false; }
  bool hash_flag() const { return false; }

  char type() const { return TYPE; }
  char fill() const { return ' '; }
};

// A width specifier.
struct WidthSpec {
  unsigned width_;
  // Fill is always wchar_t and cast to char if necessary to avoid having
  // two specialization of WidthSpec and its subclasses.
  wchar_t fill_;

  WidthSpec(unsigned width, wchar_t fill) : width_(width), fill_(fill) {}

  unsigned width() const { return width_; }
  wchar_t fill() const { return fill_; }
};

// An alignment specifier.
struct AlignSpec : WidthSpec {
  Alignment align_;

  AlignSpec(unsigned width, wchar_t fill, Alignment align = ALIGN_DEFAULT)
  : WidthSpec(width, fill), align_(align) {}

  Alignment align() const { return align_; }
};

// An alignment and type specifier.
template <char TYPE>
struct AlignTypeSpec : AlignSpec {
  AlignTypeSpec(unsigned width, wchar_t fill) : AlignSpec(width, fill) {}

  bool sign_flag() const { return false; }
  bool plus_flag() const { return false; }
  bool hash_flag() const { return false; }

  char type() const { return TYPE; }
};

// A full format specifier.
struct FormatSpec : AlignSpec {
  unsigned flags_;
  char type_;

  FormatSpec(unsigned width = 0, char type = 0, wchar_t fill = ' ')
  : AlignSpec(width, fill), flags_(0), type_(type) {}

  bool sign_flag() const { return (flags_ & FormatSpecFlags::SIGN_FLAG) != 0; }
  bool plus_flag() const { return (flags_ & FormatSpecFlags::PLUS_FLAG) != 0; }
  bool hash_flag() const { return (flags_ & FormatSpecFlags::HASH_FLAG) != 0; }

  char type() const { return type_; }
};

// An integer format specifier.
template <typename T, typename SpecT = TypeSpec<0>, typename Char = char>
class IntFormatSpec : public SpecT {
 private:
  T value_;

 public:
  IntFormatSpec(T value, const SpecT &spec = SpecT())
  : SpecT(spec), value_(value) {}

  T value() const { return value_; }
};

// A string format specifier.
template <typename T>
class StrFormatSpec : public AlignSpec {
 private:
  const T *str_;

 public:
  StrFormatSpec(const T *str, unsigned width, wchar_t fill)
  : AlignSpec(width, fill), str_(str) {}

  const T *str() const { return str_; }
};

/**
  Returns an integer format specifier to format the value in base 2.
 */
IntFormatSpec<int, TypeSpec<'b'> > bin(int value);

/**
  Returns an integer format specifier to format the value in base 8.
 */
IntFormatSpec<int, TypeSpec<'o'> > oct(int value);

/**
  Returns an integer format specifier to format the value in base 16 using
  lower-case letters for the digits above 9.
 */
IntFormatSpec<int, TypeSpec<'x'> > hex(int value);

/**
  Returns an integer formatter format specifier to format in base 16 using
  upper-case letters for the digits above 9.
 */
IntFormatSpec<int, TypeSpec<'X'> > hexu(int value);

/**
  \rst
  Returns an integer format specifier to pad the formatted argument with the
  fill character to the specified width using the default (right) numeric
  alignment.

  **Example**::

    std::string s = str(Writer() << pad(hex(0xcafe), 8, '0'));
    // s == "0000cafe"

  \endrst
 */
template <char TYPE_CODE, typename Char>
IntFormatSpec<int, AlignTypeSpec<TYPE_CODE>, Char> pad(
    int value, unsigned width, Char fill = ' ');

#define DEFINE_INT_FORMATTERS(TYPE) \
inline IntFormatSpec<TYPE, TypeSpec<'b'> > bin(TYPE value) { \
  return IntFormatSpec<TYPE, TypeSpec<'b'> >(value, TypeSpec<'b'>()); \
} \
 \
inline IntFormatSpec<TYPE, TypeSpec<'o'> > oct(TYPE value) { \
  return IntFormatSpec<TYPE, TypeSpec<'o'> >(value, TypeSpec<'o'>()); \
} \
 \
inline IntFormatSpec<TYPE, TypeSpec<'x'> > hex(TYPE value) { \
  return IntFormatSpec<TYPE, TypeSpec<'x'> >(value, TypeSpec<'x'>()); \
} \
 \
inline IntFormatSpec<TYPE, TypeSpec<'X'> > hexu(TYPE value) { \
  return IntFormatSpec<TYPE, TypeSpec<'X'> >(value, TypeSpec<'X'>()); \
} \
 \
template <char TYPE_CODE> \
inline IntFormatSpec<TYPE, AlignTypeSpec<TYPE_CODE> > pad( \
    IntFormatSpec<TYPE, TypeSpec<TYPE_CODE> > f, unsigned width) { \
  return IntFormatSpec<TYPE, AlignTypeSpec<TYPE_CODE> >( \
      f.value(), AlignTypeSpec<TYPE_CODE>(width, ' ')); \
} \
 \
/* For compatibility with older compilers we provide two overloads for pad, */ \
/* one that takes a fill character and one that doesn't. In the future this */ \
/* can be replaced with one overload making the template argument Char      */ \
/* default to char (C++11). */ \
template <char TYPE_CODE, typename Char> \
inline IntFormatSpec<TYPE, AlignTypeSpec<TYPE_CODE>, Char> pad( \
    IntFormatSpec<TYPE, TypeSpec<TYPE_CODE>, Char> f, \
    unsigned width, Char fill) { \
  return IntFormatSpec<TYPE, AlignTypeSpec<TYPE_CODE>, Char>( \
      f.value(), AlignTypeSpec<TYPE_CODE>(width, fill)); \
} \
 \
inline IntFormatSpec<TYPE, AlignTypeSpec<0> > pad( \
    TYPE value, unsigned width) { \
  return IntFormatSpec<TYPE, AlignTypeSpec<0> >( \
      value, AlignTypeSpec<0>(width, ' ')); \
} \
 \
template <typename Char> \
inline IntFormatSpec<TYPE, AlignTypeSpec<0>, Char> pad( \
   TYPE value, unsigned width, Char fill) { \
 return IntFormatSpec<TYPE, AlignTypeSpec<0>, Char>( \
     value, AlignTypeSpec<0>(width, fill)); \
}

DEFINE_INT_FORMATTERS(int)
DEFINE_INT_FORMATTERS(long)
DEFINE_INT_FORMATTERS(unsigned)
DEFINE_INT_FORMATTERS(unsigned long)
DEFINE_INT_FORMATTERS(LongLong)
DEFINE_INT_FORMATTERS(ULongLong)

/**
  \rst
  Returns a string formatter that pads the formatted argument with the fill
  character to the specified width using the default (left) string alignment.

  **Example**::

    std::string s = str(Writer() << pad("abc", 8));
    // s == "abc     "

  \endrst
 */
template <typename Char>
inline StrFormatSpec<Char> pad(
    const Char *str, unsigned width, Char fill = ' ') {
  return StrFormatSpec<Char>(str, width, fill);
}

inline StrFormatSpec<wchar_t> pad(
    const wchar_t *str, unsigned width, char fill = ' ') {
  return StrFormatSpec<wchar_t>(str, width, fill);
}

template <typename Char>
class BasicFormatter;

/**
  \rst
  This template provides operations for formatting and writing data into
  a character stream. The output is stored in a memory buffer that grows
  dynamically.

  You can use one of the following typedefs for common character types:

  +---------+----------------------+
  | Type    | Definition           |
  +=========+======================+
  | Writer  | BasicWriter<char>    |
  +---------+----------------------+
  | WWriter | BasicWriter<wchar_t> |
  +---------+----------------------+

  **Example**::

     Writer out;
     out << "The answer is " << 42 << "\n";
     out.Format("({:+f}, {:+f})") << -3.14 << 3.14;

  This will write the following output to the ``out`` object:

  .. code-block:: none

     The answer is 42
     (-3.140000, +3.140000)

  The output can be converted to an ``std::string`` with ``out.str()`` or
  accessed as a C string with ``out.c_str()``.
  \endrst
 */
template <typename Char>
class BasicWriter {
 private:
  enum { INLINE_BUFFER_SIZE = 500 };
  mutable internal::Array<Char, INLINE_BUFFER_SIZE> buffer_;  // Output buffer.

  friend class BasicFormatter<Char>;

  typedef typename internal::CharTraits<Char>::CharPtr CharPtr;

#if _SECURE_SCL
  static Char *GetBase(CharPtr p) { return p.base(); }
#else
  static Char *GetBase(Char *p) { return p; }
#endif

  static CharPtr FillPadding(CharPtr buffer,
      unsigned total_size, std::size_t content_size, wchar_t fill);

  // Grows the buffer by n characters and returns a pointer to the newly
  // allocated area.
  CharPtr GrowBuffer(std::size_t n) {
    std::size_t size = buffer_.size();
    buffer_.resize(size + n);
    return internal::CheckPtr(&buffer_[size], n);
  }

  CharPtr PrepareFilledBuffer(unsigned size, const EmptySpec &, char sign) {
    CharPtr p = GrowBuffer(size);
    *p = sign;
    return p + size - 1;
  }

  CharPtr PrepareFilledBuffer(unsigned size, const AlignSpec &spec, char sign);

  // Formats an integer.
  template <typename T, typename Spec>
  void FormatInt(T value, const Spec &spec);

  // Formats a floating-point number (double or long double).
  template <typename T>
  void FormatDouble(T value, const FormatSpec &spec, int precision);

  // Formats a string.
  template <typename StringChar>
  CharPtr FormatString(
      const StringChar *s, std::size_t size, const AlignSpec &spec);

  // This method is private to disallow writing a wide string to a
  // char stream and vice versa. If you want to print a wide string
  // as a pointer as std::ostream does, cast it to const void*.
  // Do not implement!
  void operator<<(typename internal::CharTraits<Char>::UnsupportedStrType);

 public:
  /**
    Returns the number of characters written to the output buffer.
   */
  std::size_t size() const { return buffer_.size(); }

  /**
    Returns a pointer to the output buffer content. No terminating null
    character is appended.
   */
  const Char *data() const { return &buffer_[0]; }

  /**
    Returns a pointer to the output buffer content with terminating null
    character appended.
   */
  const Char *c_str() const {
    std::size_t size = buffer_.size();
    buffer_.reserve(size + 1);
    buffer_[size] = '\0';
    return &buffer_[0];
  }

  /**
    Returns the content of the output buffer as an `std::string`.
   */
  std::basic_string<Char> str() const {
    return std::basic_string<Char>(&buffer_[0], buffer_.size());
  }

  /**
    \rst
    Formats a string sending the output to the writer. Arguments are
    accepted through the returned ``BasicFormatter`` object using inserter
    operator ``<<``.

    **Example**::

       Writer out;
       out.Format("Current point:\n");
       out.Format("({:+f}, {:+f})") << -3.14 << 3.14;

    This will write the following output to the ``out`` object:

    .. code-block:: none

       Current point:
       (-3.140000, +3.140000)

    The output can be accessed using :meth:`data` or :meth:`c_str`.

    See also `Format String Syntax`_.
    \endrst
   */
  BasicFormatter<Char> Format(StringRef format);

  BasicWriter &operator<<(int value) {
    return *this << IntFormatSpec<int>(value);
  }
  BasicWriter &operator<<(unsigned value) {
    return *this << IntFormatSpec<unsigned>(value);
  }
  BasicWriter &operator<<(long value) {
    return *this << IntFormatSpec<long>(value);
  }
  BasicWriter &operator<<(unsigned long value) {
    return *this << IntFormatSpec<unsigned long>(value);
  }
  BasicWriter &operator<<(LongLong value) {
    return *this << IntFormatSpec<LongLong>(value);
  }

  /**
    Formats *value* and writes it to the stream.
   */
  BasicWriter &operator<<(ULongLong value) {
    return *this << IntFormatSpec<ULongLong>(value);
  }

  BasicWriter &operator<<(double value) {
    FormatDouble(value, FormatSpec(), -1);
    return *this;
  }

  /**
    Formats *value* using the general format for floating-point numbers
    (``'g'``) and writes it to the stream.
   */
  BasicWriter &operator<<(long double value) {
    FormatDouble(value, FormatSpec(), -1);
    return *this;
  }

  /**
   * Writes a character to the stream.
   */
  BasicWriter &operator<<(char value) {
    *GrowBuffer(1) = value;
    return *this;
  }

  BasicWriter &operator<<(wchar_t value) {
    *GrowBuffer(1) = internal::CharTraits<Char>::ConvertChar(value);
    return *this;
  }

  /**
    Writes *value* to the stream.
   */
  BasicWriter &operator<<(const XTSystem_fmt::BasicStringRef<Char> value) {
    const Char *str = value.c_str();
    std::size_t size = value.size();
    std::copy(str, str + size, GrowBuffer(size));
    return *this;
  }



  template <typename T, typename Spec, typename FillChar>
  BasicWriter &operator<<(const IntFormatSpec<T, Spec, FillChar> &spec) {
    internal::CharTraits<Char>::ConvertChar(FillChar());
    FormatInt(spec.value(), spec);
    return *this;
  }

  template <typename StringChar>
  BasicWriter &operator<<(const StrFormatSpec<StringChar> &spec) {
    const StringChar *s = spec.str();
    FormatString(s, std::char_traits<Char>::length(s), spec);
    return *this;
  }

  void Write(const std::basic_string<Char> &s, const FormatSpec &spec) {
    FormatString(s.data(), s.size(), spec);
  }

  void Clear() {
    buffer_.clear();
  }
};

template <typename Char>
template <typename StringChar>
typename BasicWriter<Char>::CharPtr BasicWriter<Char>::FormatString(
    const StringChar *s, std::size_t size, const AlignSpec &spec) {
  CharPtr out = CharPtr();
  if (spec.width() > size) {
    out = GrowBuffer(spec.width());
    Char fill = static_cast<Char>(spec.fill());
    if (spec.align() == ALIGN_RIGHT) {
      std::fill_n(out, spec.width() - size, fill);
      out += spec.width() - size;
    } else if (spec.align() == ALIGN_CENTER) {
      out = FillPadding(out, spec.width(), size, fill);
    } else {
      std::fill_n(out + size, spec.width() - size, fill);
    }
  } else {
    out = GrowBuffer(size);
  }
  std::copy(s, s + size, out);
  return out;
}



template <typename Char>
template <typename T, typename Spec>
void BasicWriter<Char>::FormatInt(T value, const Spec &spec) {
  unsigned size = 0;
  char sign = 0;
  typedef typename internal::IntTraits<T>::MainType UnsignedType;
  UnsignedType abs_value = value;
  if (internal::IsNegative(value)) {
    sign = '-';
    ++size;
    abs_value = 0 - abs_value;
  } else if (spec.sign_flag()) {
    sign = spec.plus_flag() ? '+' : ' ';
    ++size;
  }
  switch (spec.type()) {
  case 0: case 'd': {
    unsigned num_digits = internal::CountDigits(abs_value);
    CharPtr p =
        PrepareFilledBuffer(size + num_digits, spec, sign) + 1 - num_digits;
    internal::FormatDecimal(GetBase(p), abs_value, num_digits);
    break;
  }
  case 'x': case 'X': {
    UnsignedType n = abs_value;
    bool print_prefix = spec.hash_flag();
    if (print_prefix) size += 2;
    do {
      ++size;
    } while ((n >>= 4) != 0);
    Char *p = GetBase(PrepareFilledBuffer(size, spec, sign));
    n = abs_value;
    const char *digits = spec.type() == 'x' ?
        "0123456789abcdef" : "0123456789ABCDEF";
    do {
      *p-- = digits[n & 0xf];
    } while ((n >>= 4) != 0);
    if (print_prefix) {
      *p-- = spec.type();
      *p = '0';
    }
    break;
  }
  case 'b': case 'B': {
    UnsignedType n = abs_value;
    bool print_prefix = spec.hash_flag();
    if (print_prefix) size += 2;
    do {
      ++size;
    } while ((n >>= 1) != 0);
    Char *p = GetBase(PrepareFilledBuffer(size, spec, sign));
    n = abs_value;
    do {
      *p-- = '0' + (n & 1);
    } while ((n >>= 1) != 0);
    if (print_prefix) {
      *p-- = spec.type();
      *p = '0';
    }
    break;
  }
  case 'o': {
    UnsignedType n = abs_value;
    bool print_prefix = spec.hash_flag();
    if (print_prefix) ++size;
    do {
      ++size;
    } while ((n >>= 3) != 0);
    Char *p = GetBase(PrepareFilledBuffer(size, spec, sign));
    n = abs_value;
    do {
      *p-- = '0' + (n & 7);
    } while ((n >>= 3) != 0);
    if (print_prefix)
      *p = '0';
    break;
  }
  default:
    internal::Internal::ReportUnknownType(spec.type(), "integer");
    break;
  }
}

template <typename Char>
BasicFormatter<Char> BasicWriter<Char>::Format(StringRef format) {
  BasicFormatter<Char> f(*this, format.c_str());
  return f;
}

typedef BasicWriter<char> Writer;
typedef BasicWriter<wchar_t> WWriter;

// The default formatting function.
template <typename Char, typename T>
void Format(BasicWriter<Char> &w, const FormatSpec &spec, const T &value) {
  std::basic_ostringstream<Char> os;
  os << value.ToString();
  w.Write(os.str(), spec);
}

namespace internal {
// Formats an argument of a custom type, such as a user-defined class.
template <typename Char, typename T>
void FormatCustomArg(
    BasicWriter<Char> &w, const void *arg, const FormatSpec &spec) {
  Format(w, spec, *static_cast<const T*>(arg));
}
}

/**
  \rst
  The :cpp:class:`fmt::BasicFormatter` template provides string formatting
  functionality similar to Python's `str.format
  <http://docs.python.org/3/library/stdtypes.html#str.format>`__.
  The class provides operator<< for feeding formatting arguments and all
  the output is sent to a :cpp:class:`fmt::Writer` object.
  \endrst
 */
template <typename Char>
class BasicFormatter {
 private:
  BasicWriter<Char> *writer_;

  enum Type {
    // Numeric types should go first.
    INT, UINT, LONG, ULONG, LONG_LONG, ULONG_LONG, DOUBLE, LONG_DOUBLE,
    LAST_NUMERIC_TYPE = LONG_DOUBLE,
    CHAR, STRING, WSTRING, POINTER, CUSTOM
  };

  typedef void (*FormatFunc)(
      BasicWriter<Char> &w, const void *arg, const FormatSpec &spec);

  // A format argument.
  class Arg {
   private:
    // This method is private to disallow formatting of arbitrary pointers.
    // If you want to output a pointer cast it to const void*. Do not implement!
    template <typename T>
    Arg(const T *value);

    // This method is private to disallow formatting of arbitrary pointers.
    // If you want to output a pointer cast it to void*. Do not implement!
    template <typename T>
    Arg(T *value);

    struct StringValue {
      const Char *value;
      std::size_t size;
    };

    struct CustomValue {
      const void *value;
      FormatFunc format;
    };

   public:
    Type type;
    union {
      int int_value;
      unsigned uint_value;
      double double_value;
      long long_value;
      unsigned long ulong_value;
      LongLong long_long_value;
      ULongLong ulong_long_value;
      long double long_double_value;
      const void *pointer_value;
      StringValue string;
      CustomValue custom;
    };
    mutable BasicFormatter *formatter;

    Arg(short value) : type(INT), int_value(value), formatter(0) {}
    Arg(unsigned short value) : type(UINT), int_value(value), formatter(0) {}
    Arg(int value) : type(INT), int_value(value), formatter(0) {}
    Arg(unsigned value) : type(UINT), uint_value(value), formatter(0) {}
    Arg(long value) : type(LONG), long_value(value), formatter(0) {}
    Arg(unsigned long value) : type(ULONG), ulong_value(value), formatter(0) {}
    Arg(LongLong value)
    : type(LONG_LONG), long_long_value(value), formatter(0) {}
    Arg(ULongLong value)
    : type(ULONG_LONG), ulong_long_value(value), formatter(0) {}
    Arg(float value) : type(DOUBLE), double_value(value), formatter(0) {}
    Arg(double value) : type(DOUBLE), double_value(value), formatter(0) {}
    Arg(long double value)
    : type(LONG_DOUBLE), long_double_value(value), formatter(0) {}
    Arg(char value) : type(CHAR), int_value(value), formatter(0) {}
    Arg(wchar_t value)
    : type(CHAR), int_value(internal::CharTraits<Char>::ConvertChar(value)),
      formatter(0) {}

    Arg(const Char *value) : type(STRING), formatter(0) {
      string.value = value;
      string.size = 0;
    }

    Arg(Char *value) : type(STRING), formatter(0) {
      string.value = value;
      string.size = 0;
    }

    Arg(const void *value)
    : type(POINTER), pointer_value(value), formatter(0) {}

    Arg(void *value) : type(POINTER), pointer_value(value), formatter(0) {}

    Arg(const std::basic_string<Char> &value) : type(STRING), formatter(0) {
      string.value = value.c_str();
      string.size = value.size();
    }

    Arg(BasicStringRef<Char> value) : type(STRING), formatter(0) {
      string.value = value.c_str();
      string.size = value.size();
    }

    template <typename T>
    Arg(const T &value) : type(CUSTOM), formatter(0) {
      custom.value = &value;
      custom.format = &internal::FormatCustomArg<Char, T>;
    }

    ~Arg() FMT_NOEXCEPT(false) {
      // Format is called here to make sure that a referred object is
      // still alive, for example:
      //
      //   Print("{0}") << std::string("test");
      //
      // Here an Arg object refers to a temporary std::string which is
      // destroyed at the end of the statement. Since the string object is
      // constructed before the Arg object, it will be destroyed after,
      // so it will be alive in the Arg's destructor where Format is called.
      // Note that the string object will not necessarily be alive when
      // the destructor of BasicFormatter is called.
      if (formatter)
        formatter->CompleteFormatting();
    }
  };

  enum { NUM_INLINE_ARGS = 10 };
  internal::Array<const Arg*, NUM_INLINE_ARGS> args_;  // Format arguments.

  const Char *format_;  // Format string.
  int num_open_braces_;
  int next_arg_index_;

  friend class internal::FormatterProxy<Char>;

  // Forbid copying from a temporary as in the following example:
  //   fmt::Formatter<> f = Format("test"); // not allowed
  // This is done because BasicFormatter objects should normally exist
  // only as temporaries returned by one of the formatting functions.
  // Do not implement.
  BasicFormatter(const BasicFormatter &);
  BasicFormatter& operator=(const BasicFormatter &);

  void ReportError(const Char *s, StringRef message) const;

  unsigned ParseUInt(const Char *&s) const;

  // Parses argument index and returns an argument with this index.
  const Arg &ParseArgIndex(const Char *&s);

  void CheckSign(const Char *&s, const Arg &arg);

  // Parses the format string and performs the actual formatting,
  // writing the output to writer_.
  void DoFormat();

  // Formats an integer.
  // TODO: remove
  template <typename T>
  void FormatInt(T value, const FormatSpec &spec) {
    *writer_ << IntFormatSpec<T, FormatSpec>(value, spec);
  }

  struct Proxy {
    BasicWriter<Char> *writer;
    const Char *format;

    Proxy(BasicWriter<Char> *w, const Char *fmt) : writer(w), format(fmt) {}
  };

 protected:
  const Char *TakeFormatString() {
    const Char *format = this->format_;
    this->format_ = 0;
    return format;
  }

  void CompleteFormatting() {
    if (!format_) return;
    DoFormat();
  }

 public:
  // Constructs a formatter with a writer to be used for output and a format
  // string.
  BasicFormatter(BasicWriter<Char> &w, const Char *format = 0)
  : writer_(&w), format_(format) {}

#if FMT_USE_INITIALIZER_LIST
  // Constructs a formatter with formatting arguments.
  BasicFormatter(BasicWriter<Char> &w,
      const Char *format, std::initializer_list<Arg> args)
  : writer_(&w), format_(format) {
    // TODO: don't copy arguments
    args_.reserve(args.size());
    for (const Arg &arg: args)
      args_.push_back(&arg);
  }
#endif

  // Performs formatting if the format string is non-null. The format string
  // can be null if its ownership has been transferred to another formatter.
  ~BasicFormatter() {
    CompleteFormatting();
  }

  BasicFormatter(BasicFormatter &f) : writer_(f.writer_), format_(f.format_) {
    f.format_ = 0;
  }

  // Feeds an argument to a formatter.
  BasicFormatter &operator<<(const Arg &arg) {
    arg.formatter = this;
    args_.push_back(&arg);
    return *this;
  }

  operator internal::FormatterProxy<Char>() {
    return internal::FormatterProxy<Char>(this);
  }

  operator StringRef() {
    CompleteFormatting();
    return StringRef(writer_->c_str(), writer_->size());
  }
};

template <typename Char>
inline std::basic_string<Char> str(const BasicWriter<Char> &f) {
  return f.str();
}

template <typename Char>
inline const Char *c_str(const BasicWriter<Char> &f) { return f.c_str(); }

namespace internal {

template <typename Char>
class FormatterProxy {
 private:
  BasicFormatter<Char> *formatter_;

 public:
  explicit FormatterProxy(BasicFormatter<Char> *f) : formatter_(f) {}

  BasicWriter<Char> *Format() {
    formatter_->CompleteFormatting();
    return formatter_->writer_;
  }
};
}

/**
  Returns the content of the output buffer as an `std::string`.
 */
inline std::string str(internal::FormatterProxy<char> p) {
  return p.Format()->str();
}

/**
  Returns a pointer to the output buffer content with terminating null
  character appended.
 */
inline const char *c_str(internal::FormatterProxy<char> p) {
  return p.Format()->c_str();
}

inline std::wstring str(internal::FormatterProxy<wchar_t> p) {
  return p.Format()->str();
}

inline const wchar_t *c_str(internal::FormatterProxy<wchar_t> p) {
  return p.Format()->c_str();
}

/**
  A formatting action that does nothing.
 */
class NoAction {
 public:
  /** Does nothing. */
  template <typename Char>
  void operator()(const BasicWriter<Char> &) const {}
};

/**
  \rst
  A formatter with an action performed when formatting is complete.
  Objects of this class normally exist only as temporaries returned
  by one of the formatting functions. You can use this class to create
  your own functions similar to :cpp:func:`fmt::Format()`.

  **Example**::

    struct PrintError {
      void operator()(const fmt::Writer &w) const {
        fmt::Print("Error: {}\n") << w.str();
      }
    };

    // Formats an error message and prints it to stdout.
    fmt::Formatter<PrintError> ReportError(const char *format) {
      fmt::Formatter f<PrintError>(format);
      return f;
    }

    ReportError("File not found: {}") << path;
  \endrst
 */
template <typename Action = NoAction, typename Char = char>
class Formatter : private Action, public BasicFormatter<Char> {
 private:
  BasicWriter<Char> writer_;
  bool inactive_;

  // Forbid copying other than from a temporary. Do not implement.
  Formatter(const Formatter &);
  Formatter& operator=(const Formatter &);

 public:
  /**
    \rst
    Constructs a formatter with a format string and an action.
    The action should be an unary function object that takes a const
    reference to :cpp:class:`fmt::BasicWriter` as an argument.
    See :cpp:class:`fmt::NoAction` and :cpp:class:`fmt::Write` for
    examples of action classes.
    \endrst
  */
  explicit Formatter(BasicStringRef<Char> format, Action a = Action())
  : Action(a), BasicFormatter<Char>(writer_, format.c_str()),
    inactive_(false) {
  }

  Formatter(Formatter &f)
  : Action(f), BasicFormatter<Char>(writer_, f.TakeFormatString()),
    inactive_(false) {
    f.inactive_ = true;
  }

  /**
    Performs the actual formatting, invokes the action and destroys the object.
   */
  ~Formatter() FMT_NOEXCEPT(false) {
    if (!inactive_) {
      this->CompleteFormatting();
      (*this)(writer_);
    }
  }
};

/**
  Fast integer formatter.
 */
class FormatInt {
 private:
  // Buffer should be large enough to hold all digits (digits10 + 1),
  // a sign and a null character.
  enum {BUFFER_SIZE = std::numeric_limits<ULongLong>::digits10 + 3};
  mutable char buffer_[BUFFER_SIZE];
  char *str_;

  // Formats value in reverse and returns the number of digits.
  char *FormatDecimal(ULongLong value) {
    char *buffer_end = buffer_ + BUFFER_SIZE - 1;
    while (value >= 100) {
      // Integer division is slow so do it for a group of two digits instead
      // of for every digit. The idea comes from the talk by Alexandrescu
      // "Three Optimization Tips for C++". See speed-test for a comparison.
      unsigned index = (value % 100) * 2;
      value /= 100;
      *--buffer_end = internal::Constants::DIGITS()[index + 1];
      *--buffer_end = internal::Constants::DIGITS()[index];
    }
    if (value < 10) {
      *--buffer_end = static_cast<char>('0' + value);
      return buffer_end;
    }
    unsigned index = static_cast<unsigned>(value * 2);
    *--buffer_end = internal::Constants::DIGITS()[index + 1];
    *--buffer_end = internal::Constants::DIGITS()[index];
    return buffer_end;
  }
  
  void FormatSigned(LongLong value) {
    ULongLong abs_value = value;
    bool negative = value < 0;
    if (negative)
      abs_value = 0 - value;
    str_ = FormatDecimal(abs_value);
    if (negative)
      *--str_ = '-';
  }

 public:
  explicit FormatInt(int value) { FormatSigned(value); }
  explicit FormatInt(long value) { FormatSigned(value); }
  explicit FormatInt(LongLong value) { FormatSigned(value); }
  explicit FormatInt(unsigned value) : str_(FormatDecimal(value)) {}
  explicit FormatInt(unsigned long value) : str_(FormatDecimal(value)) {}
  explicit FormatInt(ULongLong value) : str_(FormatDecimal(value)) {}

  /**
    Returns the number of characters written to the output buffer.
   */
  std::size_t size() const { return buffer_ - str_ + BUFFER_SIZE - 1; }

  /**
    Returns a pointer to the output buffer content. No terminating null
    character is appended.
   */
  const char *data() const { return str_; }

  /**
    Returns a pointer to the output buffer content with terminating null
    character appended.
   */
  const char *c_str() const {
    buffer_[BUFFER_SIZE - 1] = '\0';
    return str_;
  }

  /**
    Returns the content of the output buffer as an `std::string`.
   */
  std::string str() const { return std::string(str_, size()); }
};

// Formats a decimal integer value writing into buffer and returns
// a pointer to the end of the formatted string. This function doesn't
// write a terminating null character.
template <typename T>
inline void FormatDec(char *&buffer, T value) {
  typename internal::IntTraits<T>::MainType abs_value = value;
  if (internal::IsNegative(value)) {
    *buffer++ = '-';
    abs_value = 0 - abs_value;
  }
  if (abs_value < 100) {
    if (abs_value < 10) {
      *buffer++ = static_cast<char>('0' + abs_value);
      return;
    }
    unsigned index = static_cast<unsigned>(abs_value * 2);
    *buffer++ = internal::DIGITS[index];
    *buffer++ = internal::DIGITS[index + 1];
    return;
  }
  unsigned num_digits = internal::CountDigits(abs_value);
  internal::FormatDecimal(buffer, abs_value, num_digits);
  buffer += num_digits;
}

/**
  \rst
  Formats a string similarly to Python's `str.format
  <http://docs.python.org/3/library/stdtypes.html#str.format>`__.
  Returns a temporary formatter object that accepts arguments via
  operator ``<<``.

  *format* is a format string that contains literal text and replacement
  fields surrounded by braces ``{}``. The formatter object replaces the
  fields with formatted arguments and stores the output in a memory buffer.
  The content of the buffer can be converted to ``std::string`` with
  :cpp:func:`fmt::str()` or accessed as a C string with
  :cpp:func:`fmt::c_str()`.

  **Example**::

    std::string message = str(Format("The answer is {}") << 42);

  See also `Format String Syntax`_.
  \endrst
*/
inline Formatter<> Format(StringRef format) {
  Formatter<> f(format);
  return f;
}

inline Formatter<NoAction, wchar_t> Format(WStringRef format) {
  Formatter<NoAction, wchar_t> f(format);
  return f;
}

/** A formatting action that writes formatted output to stdout. */
class Write {
 public:
  /** Writes the output to stdout. */
  void operator()(const BasicWriter<char> &w) const {
    std::fwrite(w.data(), 1, w.size(), stdout);
  }
};

// Formats a string and prints it to stdout.
// Example:
//   Print("Elapsed time: {0:.2f} seconds") << 1.23;
inline Formatter<Write> Print(StringRef format) {
  Formatter<Write> f(format);
  return f;
}

enum Color {BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE};

/** A formatting action that writes colored output to stdout. */
class ColorWriter {
 private:
  Color color_;

 public:
  explicit ColorWriter(Color c) : color_(c) {}

  /** Writes the colored output to stdout. */
  //void operator()(const BasicWriter<char> &w) const;


	void operator()(const XTSystem_fmt::BasicWriter<char> &w) const {
	  char escape[] = "\x1b[30m";
	  char reset_color[] = "\x1b[0m";
	  escape[3] = '0' + static_cast<char>(color_);
	  std::fputs(escape, stdout);
	  std::fwrite(w.data(), 1, w.size(), stdout);
	  std::fputs(reset_color, stdout);
	}

};

// Formats a string and prints it to stdout with the given color.
// Example:
//   PrintColored(fmt::RED, "Elapsed time: {0:.2f} seconds") << 1.23;
inline Formatter<ColorWriter> PrintColored(Color c, StringRef format) {
  Formatter<ColorWriter> f(format, ColorWriter(c));
  return f;
}

#if FMT_USE_INITIALIZER_LIST && FMT_USE_VARIADIC_TEMPLATES
template<typename... Args>
std::string Format(const StringRef &format, const Args & ... args) {
  Writer w;
  BasicFormatter<char> f(w, format.c_str(), { args... });
  return XTSystem_fmt::str(f);
}

template<typename... Args>
std::wstring Format(const WStringRef &format, const Args & ... args) {
  WWriter w;
  BasicFormatter<wchar_t> f(w, format.c_str(), { args... });
  return XTSystem_fmt::str(f);
}
#endif  // FMT_USE_INITIALIZER_LIST && FMT_USE_VARIADIC_TEMPLATES

}

namespace XTSystem_fmt {

struct Ext
{
static inline int SignBit(double value) 
{
  if (value < 0) return 1;
  if (value == value) return 0;
  int dec = 0, sign = 0;
  char buffer[2];  // The buffer size must be >= 2 or _ecvt_s will fail.
  _ecvt_s(buffer, sizeof(buffer), value, 0, &dec, &sign);
  return sign;
}

static inline int IsInf(double x) { return !_finite(x); }

static inline int FMT_SNPRINTF(char *buffer, size_t size, const char *format, ...) {
  va_list args;
  va_start(args, format);
  int result = vsnprintf_s(buffer, size, _TRUNCATE, format, args);
  va_end(args);
  return result;
}



};

}

template <typename T>
int XTSystem_fmt::internal::CharTraits<char>::FormatFloat(
    char *buffer, std::size_t size, const char *format,
    unsigned width, int precision, T value) {
  if (width == 0) {
    return precision < 0 ?
        Ext::FMT_SNPRINTF(buffer, size, format, value) :
        Ext::FMT_SNPRINTF(buffer, size, format, precision, value);
  }
  return precision < 0 ?
      Ext::FMT_SNPRINTF(buffer, size, format, width, value) :
      Ext::FMT_SNPRINTF(buffer, size, format, width, precision, value);
}

template <typename T>
int XTSystem_fmt::internal::CharTraits<wchar_t>::FormatFloat(
    wchar_t *buffer, std::size_t size, const wchar_t *format,
    unsigned width, int precision, T value) {
  if (width == 0) {
    return precision < 0 ?
        swprintf(buffer, size, format, value) :
        swprintf(buffer, size, format, precision, value);
  }
  return precision < 0 ?
      swprintf(buffer, size, format, width, value) :
      swprintf(buffer, size, format, width, precision, value);
}

namespace XTSystem_fmt
{
	namespace internal
	{
		struct Internal
		{
			static void ReportUnknownType(char code, const char *type)
			{
			  if (std::isprint(static_cast<unsigned char>(code))) {
				throw XTSystem_fmt::FormatError(XTSystem_fmt::str(
					XTSystem_fmt::Format("unknown format code '{}' for {}") << code << type));
			  }
			  throw XTSystem_fmt::FormatError(
				  XTSystem_fmt::str(XTSystem_fmt::Format("unknown format code '\\x{:02x}' for {}")
					<< static_cast<unsigned>(code) << type));
			}

		};
	}
}




// Fills the padding around the content and returns the pointer to the
// content area.
template <typename Char>
typename XTSystem_fmt::BasicWriter<Char>::CharPtr
  XTSystem_fmt::BasicWriter<Char>::FillPadding(CharPtr buffer,
    unsigned total_size, std::size_t content_size, wchar_t fill) {
  std::size_t padding = total_size - content_size;
  std::size_t left_padding = padding / 2;
  Char fill_char = static_cast<Char>(fill);
  std::fill_n(buffer, left_padding, fill_char);
  buffer += left_padding;
  CharPtr content = buffer;
  std::fill_n(buffer + content_size, padding - left_padding, fill_char);
  return content;
}

template <typename Char>
typename XTSystem_fmt::BasicWriter<Char>::CharPtr
  XTSystem_fmt::BasicWriter<Char>::PrepareFilledBuffer(
    unsigned size, const AlignSpec &spec, char sign) {
  unsigned width = spec.width();
  if (width <= size) {
    CharPtr p = GrowBuffer(size);
    *p = sign;
    return p + size - 1;
  }
  CharPtr p = GrowBuffer(width);
  CharPtr end = p + width;
  Alignment align = spec.align();
  // TODO: error if fill is not convertible to Char
  Char fill = static_cast<Char>(spec.fill());
  if (align == ALIGN_LEFT) {
    *p = sign;
    p += size;
    std::fill(p, end, fill);
  } else if (align == ALIGN_CENTER) {
    p = FillPadding(p, width, size, fill);
    *p = sign;
    p += size;
  } else {
    if (align == ALIGN_NUMERIC) {
      if (sign) {
        *p++ = sign;
        --size;
      }
    } else {
      *(end - size) = sign;
    }
    std::fill(p, end - size, fill);
    p = end;
  }
  return p - 1;
}

template <typename Char>
template <typename T>
void XTSystem_fmt::BasicWriter<Char>::FormatDouble(
    T value, const FormatSpec &spec, int precision) {
  // Check type.
  char type = spec.type();
  bool upper = false;
  switch (type) {
  case 0:
    type = 'g';
    break;
  case 'e': case 'f': case 'g':
    break;
  case 'F':
#ifdef _MSC_VER
    // MSVC's printf doesn't support 'F'.
    type = 'f';
#endif
    // Fall through.
  case 'E': case 'G':
    upper = true;
    break;
  default:
    internal::Internal::ReportUnknownType(type, "double");
    break;
  }

  char sign = 0;
  // Use SignBit instead of value < 0 because the latter is always
  // false for NaN.
  if (Ext::SignBit(static_cast<double>(value))) {
    sign = '-';
    value = -value;
  } else if (spec.sign_flag()) {
    sign = spec.plus_flag() ? '+' : ' ';
  }

  if (value != value) {
    // Format NaN ourselves because sprintf's output is not consistent
    // across platforms.
    std::size_t size = 4;
    const char *nan = upper ? " NAN" : " nan";
    if (!sign) {
      --size;
      ++nan;
    }
    CharPtr out = FormatString(nan, size, spec);
    if (sign)
      *out = sign;
    return;
  }

  if (Ext::IsInf(static_cast<double>(value))) {
    // Format infinity ourselves because sprintf's output is not consistent
    // across platforms.
    std::size_t size = 4;
    const char *inf = upper ? " INF" : " inf";
    if (!sign) {
      --size;
      ++inf;
    }
    CharPtr out = FormatString(inf, size, spec);
    if (sign)
      *out = sign;
    return;
  }

  std::size_t offset = buffer_.size();
  unsigned width = spec.width();
  if (sign) {
    buffer_.reserve(buffer_.size() + (std::max)(width, 1u));
    if (width > 0)
      --width;
    ++offset;
  }

  // Build format string.
  enum { MAX_FORMAT_SIZE = 10}; // longest format: %#-*.*Lg
  Char format[MAX_FORMAT_SIZE];
  Char *format_ptr = format;
  *format_ptr++ = '%';
  unsigned width_for_sprintf = width;
  if (spec.hash_flag())
    *format_ptr++ = '#';
  if (spec.align() == ALIGN_CENTER) {
    width_for_sprintf = 0;
  } else {
    if (spec.align() == ALIGN_LEFT)
      *format_ptr++ = '-';
    if (width != 0)
      *format_ptr++ = '*';
  }
  if (precision >= 0) {
    *format_ptr++ = '.';
    *format_ptr++ = '*';
  }
  if (internal::IsLongDouble<T>::VALUE)
    *format_ptr++ = 'L';
  *format_ptr++ = type;
  *format_ptr = '\0';

  // Format using snprintf.
  Char fill = static_cast<Char>(spec.fill());
  for (;;) {
    std::size_t size = buffer_.capacity() - offset;
    Char *start = &buffer_[offset];
    int n = internal::CharTraits<Char>::FormatFloat(
        start, size, format, width_for_sprintf, precision, value);
    if (n >= 0 && offset + n < buffer_.capacity()) {
      if (sign) {
        if ((spec.align() != ALIGN_RIGHT && spec.align() != ALIGN_DEFAULT) ||
            *start != ' ') {
          *(start - 1) = sign;
          sign = 0;
        } else {
          *(start - 1) = fill;
        }
        ++n;
      }
      if (spec.align() == ALIGN_CENTER &&
          spec.width() > static_cast<unsigned>(n)) {
        unsigned width = spec.width();
        CharPtr p = GrowBuffer(width);
        std::copy(p, p + n, p + (width - n) / 2);
        FillPadding(p, spec.width(), n, fill);
        return;
      }
      if (spec.fill() != ' ' || sign) {
        while (*start == ' ')
          *start++ = fill;
        if (sign)
          *(start - 1) = sign;
      }
      GrowBuffer(n);
      return;
    }
    buffer_.reserve(n >= 0 ? offset + n + 1 : 2 * buffer_.capacity());
  }
}

// Throws Exception(message) if format contains '}', otherwise throws
// FormatError reporting unmatched '{'. The idea is that unmatched '{'
// should override other errors.
template <typename Char>
void XTSystem_fmt::BasicFormatter<Char>::ReportError(
    const Char *s, StringRef message) const {
  for (int num_open_braces = num_open_braces_; *s; ++s) {
    if (*s == '{') {
      ++num_open_braces;
    } else if (*s == '}') {
      if (--num_open_braces == 0)
        throw XTSystem_fmt::FormatError(message);
    }
  }
  throw XTSystem_fmt::FormatError("unmatched '{' in format");
}

// Parses an unsigned integer advancing s to the end of the parsed input.
// This function assumes that the first character of s is a digit.
template <typename Char>
unsigned XTSystem_fmt::BasicFormatter<Char>::ParseUInt(const Char *&s) const {
  assert('0' <= *s && *s <= '9');
  unsigned value = 0;
  do {
    unsigned new_value = value * 10 + (*s++ - '0');
    if (new_value < value)  // Check if value wrapped around.
      ReportError(s, "number is too big in format");
    value = new_value;
  } while ('0' <= *s && *s <= '9');
  return value;
}

template <typename Char>
inline const typename XTSystem_fmt::BasicFormatter<Char>::Arg
    &XTSystem_fmt::BasicFormatter<Char>::ParseArgIndex(const Char *&s) {
  unsigned arg_index = 0;
  if (*s < '0' || *s > '9') {
    if (*s != '}' && *s != ':')
      ReportError(s, "invalid argument index in format string");
    if (next_arg_index_ < 0) {
      ReportError(s,
          "cannot switch from manual to automatic argument indexing");
    }
    arg_index = next_arg_index_++;
  } else {
    if (next_arg_index_ > 0) {
      ReportError(s,
          "cannot switch from automatic to manual argument indexing");
    }
    next_arg_index_ = -1;
    arg_index = ParseUInt(s);
  }
  if (arg_index >= args_.size())
    ReportError(s, "argument index is out of range in format");
  return *args_[arg_index];
}

template <typename Char>
void XTSystem_fmt::BasicFormatter<Char>::CheckSign(const Char *&s, const Arg &arg) {
  char sign = static_cast<char>(*s);
  if (arg.type > LAST_NUMERIC_TYPE) {
    ReportError(s,
        Format("format specifier '{}' requires numeric argument") << sign);
  }
  if (arg.type == UINT || arg.type == ULONG || arg.type == ULONG_LONG) {
    ReportError(s,
        Format("format specifier '{}' requires signed argument") << sign);
  }
  ++s;
}

template <typename Char>
void XTSystem_fmt::BasicFormatter<Char>::DoFormat() {
  const Char *start = format_;
  format_ = 0;
  next_arg_index_ = 0;
  const Char *s = start;
  BasicWriter<Char> &writer = *writer_;
  while (*s) {
    Char c = *s++;
    if (c != '{' && c != '}') continue;
    if (*s == c) {
      writer.buffer_.append(start, s);
      start = ++s;
      continue;
    }
    if (c == '}')
      throw FormatError("unmatched '}' in format");
    num_open_braces_= 1;
    writer.buffer_.append(start, s - 1);

    const Arg &arg = ParseArgIndex(s);

    FormatSpec spec;
    int precision = -1;
    if (*s == ':') {
      ++s;

      // Parse fill and alignment.
      if (Char c = *s) {
        const Char *p = s + 1;
        spec.align_ = ALIGN_DEFAULT;
        do {
          switch (*p) {
          case '<':
            spec.align_ = ALIGN_LEFT;
            break;
          case '>':
            spec.align_ = ALIGN_RIGHT;
            break;
          case '=':
            spec.align_ = ALIGN_NUMERIC;
            break;
          case '^':
            spec.align_ = ALIGN_CENTER;
            break;
          }
          if (spec.align_ != ALIGN_DEFAULT) {
            if (p != s) {
              if (c == '}') break;
              if (c == '{')
                ReportError(s, "invalid fill character '{'");
              s += 2;
              spec.fill_ = c;
            } else ++s;
            if (spec.align_ == ALIGN_NUMERIC && arg.type > LAST_NUMERIC_TYPE)
              ReportError(s, "format specifier '=' requires numeric argument");
            break;
          }
        } while (--p >= s);
      }

      // Parse sign.
      switch (*s) {
      case '+':
        CheckSign(s, arg);
        spec.flags_ |= FormatSpecFlags::SIGN_FLAG | FormatSpecFlags::PLUS_FLAG;
        break;
      case '-':
        CheckSign(s, arg);
        break;
      case ' ':
        CheckSign(s, arg);
        spec.flags_ |= FormatSpecFlags::SIGN_FLAG;
        break;
      }

      if (*s == '#') {
        if (arg.type > LAST_NUMERIC_TYPE)
          ReportError(s, "format specifier '#' requires numeric argument");
        spec.flags_ |= FormatSpecFlags::HASH_FLAG;
        ++s;
      }

      // Parse width and zero flag.
      if ('0' <= *s && *s <= '9') {
        if (*s == '0') {
          if (arg.type > LAST_NUMERIC_TYPE)
            ReportError(s, "format specifier '0' requires numeric argument");
          spec.align_ = ALIGN_NUMERIC;
          spec.fill_ = '0';
        }
        // Zero may be parsed again as a part of the width, but it is simpler
        // and more efficient than checking if the next char is a digit.
        unsigned value = ParseUInt(s);
        if (value > INT_MAX)
          ReportError(s, "number is too big in format");
        spec.width_ = value;
      }

      // Parse precision.
      if (*s == '.') {
        ++s;
        precision = 0;
        if ('0' <= *s && *s <= '9') {
          unsigned value = ParseUInt(s);
          if (value > INT_MAX)
            ReportError(s, "number is too big in format");
          precision = value;
        } else if (*s == '{') {
          ++s;
          ++num_open_braces_;
          const Arg &precision_arg = ParseArgIndex(s);
          ULongLong value = 0;
          switch (precision_arg.type) {
          case INT:
            if (precision_arg.int_value < 0)
              ReportError(s, "negative precision in format");
            value = precision_arg.int_value;
            break;
          case UINT:
            value = precision_arg.uint_value;
            break;
          case LONG:
            if (precision_arg.long_value < 0)
              ReportError(s, "negative precision in format");
            value = precision_arg.long_value;
            break;
          case ULONG:
            value = precision_arg.ulong_value;
            break;
          case LONG_LONG:
            if (precision_arg.long_long_value < 0)
              ReportError(s, "negative precision in format");
            value = precision_arg.long_long_value;
            break;
          case ULONG_LONG:
            value = precision_arg.ulong_long_value;
            break;
          default:
            ReportError(s, "precision is not integer");
          }
          if (value > INT_MAX)
            ReportError(s, "number is too big in format");
          precision = static_cast<int>(value);
          if (*s++ != '}')
            throw FormatError("unmatched '{' in format");
          --num_open_braces_;
        } else {
          ReportError(s, "missing precision in format");
        }
        if (arg.type != DOUBLE && arg.type != LONG_DOUBLE) {
          ReportError(s,
              "precision specifier requires floating-point argument");
        }
      }

      // Parse type.
      if (*s != '}' && *s)
        spec.type_ = static_cast<char>(*s++);
    }

    if (*s++ != '}')
      throw FormatError("unmatched '{' in format");
    start = s;

    // Format argument.
    switch (arg.type) {
    case INT:
      FormatInt(arg.int_value, spec);
      break;
    case UINT:
      FormatInt(arg.uint_value, spec);
      break;
    case LONG:
      FormatInt(arg.long_value, spec);
      break;
    case ULONG:
      FormatInt(arg.ulong_value, spec);
      break;
    case LONG_LONG:
      FormatInt(arg.long_long_value, spec);
      break;
    case ULONG_LONG:
      FormatInt(arg.ulong_long_value, spec);
      break;
    case DOUBLE:
      writer.FormatDouble(arg.double_value, spec, precision);
      break;
    case LONG_DOUBLE:
      writer.FormatDouble(arg.long_double_value, spec, precision);
      break;
    case CHAR: {
      if (spec.type_ && spec.type_ != 'c')
        internal::Internal::ReportUnknownType(spec.type_, "char");
      typedef typename BasicWriter<Char>::CharPtr CharPtr;
      CharPtr out = CharPtr();
      if (spec.width_ > 1) {
        Char fill = static_cast<Char>(spec.fill());
        out = writer.GrowBuffer(spec.width_);
        if (spec.align_ == ALIGN_RIGHT) {
          std::fill_n(out, spec.width_ - 1, fill);
          out += spec.width_ - 1;
        } else if (spec.align_ == ALIGN_CENTER) {
          out = writer.FillPadding(out, spec.width_, 1, fill);
        } else {
          std::fill_n(out + 1, spec.width_ - 1, fill);
        }
      } else {
        out = writer.GrowBuffer(1);
      }
      *out = static_cast<Char>(arg.int_value);
      break;
    }
    case STRING: {
      if (spec.type_ && spec.type_ != 's')
        internal::Internal::ReportUnknownType(spec.type_, "string");
      const Char *str = arg.string.value;
      std::size_t size = arg.string.size;
      if (size == 0) {
        if (!str)
          throw FormatError("string pointer is null");
        if (*str)
          size = std::char_traits<Char>::length(str);
      }
      writer.FormatString(str, size, spec);
      break;
    }
    case POINTER:
      if (spec.type_ && spec.type_ != 'p')
        internal::Internal::ReportUnknownType(spec.type_, "pointer");
      spec.flags_= FormatSpecFlags::HASH_FLAG;
      spec.type_ = 'x';
      FormatInt(reinterpret_cast<uintptr_t>(arg.pointer_value), spec);
      break;
    case CUSTOM:
      if (spec.type_)
        internal::Internal::ReportUnknownType(spec.type_, "object");
      arg.custom.format(writer, arg.custom.value, spec);
      break;
    default:
      assert(false);
      break;
    }
  }
  writer.buffer_.append(start, s);
}





// Explicit instantiations for char.

template void XTSystem_fmt::BasicWriter<char>::FormatDouble<double>(
    double value, const FormatSpec &spec, int precision);

template void XTSystem_fmt::BasicWriter<char>::FormatDouble<long double>(
    long double value, const FormatSpec &spec, int precision);

template XTSystem_fmt::BasicWriter<char>::CharPtr
  XTSystem_fmt::BasicWriter<char>::FillPadding(CharPtr buffer,
    unsigned total_size, std::size_t content_size, wchar_t fill);

template XTSystem_fmt::BasicWriter<char>::CharPtr
  XTSystem_fmt::BasicWriter<char>::PrepareFilledBuffer(
    unsigned size, const AlignSpec &spec, char sign);

template void XTSystem_fmt::BasicFormatter<char>::ReportError(
    const char *s, StringRef message) const;

template unsigned XTSystem_fmt::BasicFormatter<char>::ParseUInt(const char *&s) const;

template const XTSystem_fmt::BasicFormatter<char>::Arg
    &XTSystem_fmt::BasicFormatter<char>::ParseArgIndex(const char *&s);

template void XTSystem_fmt::BasicFormatter<char>::CheckSign(
    const char *&s, const Arg &arg);

template void XTSystem_fmt::BasicFormatter<char>::DoFormat();

// Explicit instantiations for wchar_t.

template void XTSystem_fmt::BasicWriter<wchar_t>::FormatDouble<double>(
    double value, const FormatSpec &spec, int precision);

template void XTSystem_fmt::BasicWriter<wchar_t>::FormatDouble<long double>(
    long double value, const FormatSpec &spec, int precision);

template XTSystem_fmt::BasicWriter<wchar_t>::CharPtr
  XTSystem_fmt::BasicWriter<wchar_t>::FillPadding(CharPtr buffer,
      unsigned total_size, std::size_t content_size, wchar_t fill);

template XTSystem_fmt::BasicWriter<wchar_t>::CharPtr
  XTSystem_fmt::BasicWriter<wchar_t>::PrepareFilledBuffer(
    unsigned size, const AlignSpec &spec, char sign);

template void XTSystem_fmt::BasicFormatter<wchar_t>::ReportError(
    const wchar_t *s, StringRef message) const;

template unsigned XTSystem_fmt::BasicFormatter<wchar_t>::ParseUInt(
    const wchar_t *&s) const;

template const XTSystem_fmt::BasicFormatter<wchar_t>::Arg
    &XTSystem_fmt::BasicFormatter<wchar_t>::ParseArgIndex(const wchar_t *&s);

template void XTSystem_fmt::BasicFormatter<wchar_t>::CheckSign(
    const wchar_t *&s, const Arg &arg);

template void XTSystem_fmt::BasicFormatter<wchar_t>::DoFormat();


#if _MSC_VER
# pragma warning(pop)
#endif

