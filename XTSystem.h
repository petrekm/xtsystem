#pragma once 
#pragma warning(disable: 4482) 
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
#define OUT_ARG_1 << t1
#define OUT_ARG_2 << t1 << t2
#define OUT_ARG_3 << t1 << t2 << t3
#define OUT_ARG_4 << t1 << t2 << t3 << t4
#define OUT_ARG_5 << t1 << t2 << t3 << t4 << t5
#define OUT_ARG_6 << t1 << t2 << t3 << t4 << t5 << t6
#define OUT_ARG_7 << t1 << t2 << t3 << t4 << t5 << t6 << t7
#define OUT_ARG_8 << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8
#define OUT_ARG_9 << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9
#define OUT_ARG_10 << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9 << t10


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

//   2005-07-27   v1    - First public release on http://www.codeproject.com/

//#pragma once

//#include <windows.h>
#pragma comment(lib, "version.lib")  // for "VerQueryValue"

// special defines for VC5/6 (if no actual PSDK is installed):
#if _MSC_VER < 1300
typedef unsigned __int64 DWORD64, *PDWORD64;
#if defined(_WIN64)
typedef unsigned __int64 SIZE_T, *PSIZE_T;
#else
typedef unsigned long SIZE_T, *PSIZE_T;
#endif
#endif 



// The "ugly" assembler-implementation is needed for systems before XP
// If you have a new PSDK and you only compile for XP and later, then you can use 
// the "RtlCaptureContext"
// Currently there is no define which determines the PSDK-Version... 
// So we just use the compiler-version (and assumes that the PSDK is 
// the one which was installed by the VS-IDE)

// INFO: If you want, you can use the RtlCaptureContext if you only target XP and later...
//       But I currently use it in x64/IA64 environments...
//#if defined(_M_IX86) && (_WIN32_WINNT <= 0x0500) && (_MSC_VER < 1400)

#if defined(_M_IX86)
#ifdef CURRENT_THREAD_VIA_EXCEPTION
// TODO: The following is not a "good" implementation, 
// because the callstack is only valid in the "__except" block...
#define GET_CURRENT_CONTEXT(c, contextFlags) \
  do { \
    memset(&c, 0, sizeof(CONTEXT)); \
    EXCEPTION_POINTERS *pExp = NULL; \
    __try { \
      throw 0; \
    } __except( ( (pExp = GetExceptionInformation()) ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_EXECUTE_HANDLER)) {} \
    if (pExp != NULL) \
      memcpy(&c, pExp->ContextRecord, sizeof(CONTEXT)); \
      c.ContextFlags = contextFlags; \
  } while(0);
#else
// The following should be enough for walking the callstack...
#define GET_CURRENT_CONTEXT(c, contextFlags) \
  do { \
    memset(&c, 0, sizeof(CONTEXT)); \
    c.ContextFlags = contextFlags; \
    __asm    call x \
    __asm x: pop eax \
    __asm    mov c.Eip, eax \
    __asm    mov c.Ebp, ebp \
    __asm    mov c.Esp, esp \
  } while(0);
#endif

#else

// The following is defined for x86 (XP and higher), x64 and IA64:
#define GET_CURRENT_CONTEXT(c, contextFlags) \
  do { \
    memset(&c, 0, sizeof(CONTEXT)); \
    c.ContextFlags = contextFlags; \
    RtlCaptureContext(&c); \
} while(0);
#endif


// If VC7 and later, then use the shipped 'dbghelp.h'-file
#if _MSC_VER >= 1300
#pragma warning(disable:4091)
#include <dbghelp.h>
#pragma warning(default:4091)
#else
// inline the important dbghelp.h-declarations...
typedef enum {
    SymNone = 0,
    SymCoff,
    SymCv,
    SymPdb,
    SymExport,
    SymDeferred,
    SymSym,
    SymDia,
    SymVirtual,
    NumSymTypes
} SYM_TYPE;
typedef struct _IMAGEHLP_LINE64 {
    DWORD                       SizeOfStruct;           // set to sizeof(IMAGEHLP_LINE64)
    PVOID                       Key;                    // internal
    DWORD                       LineNumber;             // line number in file
    PCHAR                       FileName;               // full filename
    DWORD64                     Address;                // first instruction of line
} IMAGEHLP_LINE64, *PIMAGEHLP_LINE64;
typedef struct _IMAGEHLP_MODULE64 {
    DWORD                       SizeOfStruct;           // set to sizeof(IMAGEHLP_MODULE64)
    DWORD64                     BaseOfImage;            // base load address of module
    DWORD                       ImageSize;              // virtual size of the loaded module
    DWORD                       TimeDateStamp;          // date/time stamp from pe header
    DWORD                       CheckSum;               // checksum from the pe header
    DWORD                       NumSyms;                // number of symbols in the symbol table
    SYM_TYPE                    SymType;                // type of symbols loaded
    CHAR                        ModuleName[32];         // module name
    CHAR                        ImageName[256];         // image name
    CHAR                        LoadedImageName[256];   // symbol file name
} IMAGEHLP_MODULE64, *PIMAGEHLP_MODULE64;
typedef struct _IMAGEHLP_SYMBOL64 {
    DWORD                       SizeOfStruct;           // set to sizeof(IMAGEHLP_SYMBOL64)
    DWORD64                     Address;                // virtual address including dll base address
    DWORD                       Size;                   // estimated size of symbol, can be zero
    DWORD                       Flags;                  // info about the symbols, see the SYMF defines
    DWORD                       MaxNameLength;          // maximum size of symbol name in 'Name'
    CHAR                        Name[1];                // symbol name (null terminated string)
} IMAGEHLP_SYMBOL64, *PIMAGEHLP_SYMBOL64;
typedef enum {
    AddrMode1616,
    AddrMode1632,
    AddrModeReal,
    AddrModeFlat
} ADDRESS_MODE;
typedef struct _tagADDRESS64 {
    DWORD64       Offset;
    WORD          Segment;
    ADDRESS_MODE  Mode;
} ADDRESS64, *LPADDRESS64;
typedef struct _KDHELP64 {
    DWORD64   Thread;
    DWORD   ThCallbackStack;
    DWORD   ThCallbackBStore;
    DWORD   NextCallback;
    DWORD   FramePointer;
    DWORD64   KiCallUserMode;
    DWORD64   KeUserCallbackDispatcher;
    DWORD64   SystemRangeStart;
    DWORD64  Reserved[8];
} KDHELP64, *PKDHELP64;
typedef struct _tagSTACKFRAME64 {
    ADDRESS64   AddrPC;               // program counter
    ADDRESS64   AddrReturn;           // return address
    ADDRESS64   AddrFrame;            // frame pointer
    ADDRESS64   AddrStack;            // stack pointer
    ADDRESS64   AddrBStore;           // backing store pointer
    PVOID       FuncTableEntry;       // pointer to pdata/fpo or NULL
    DWORD64     Params[4];            // possible arguments to the function
    BOOL        Far;                  // WOW far call
    BOOL        Virtual;              // is this a virtual frame?
    DWORD64     Reserved[3];
    KDHELP64    KdHelp;
} STACKFRAME64, *LPSTACKFRAME64;
typedef
BOOL
(__stdcall *PREAD_PROCESS_MEMORY_ROUTINE64)(
    HANDLE      hProcess,
    DWORD64     qwBaseAddress,
    PVOID       lpBuffer,
    DWORD       nSize,
    LPDWORD     lpNumberOfBytesRead
    );
typedef
PVOID
(__stdcall *PFUNCTION_TABLE_ACCESS_ROUTINE64)(
    HANDLE  hProcess,
    DWORD64 AddrBase
    );
typedef
DWORD64
(__stdcall *PGET_MODULE_BASE_ROUTINE64)(
    HANDLE  hProcess,
    DWORD64 Address
    );
typedef
DWORD64
(__stdcall *PTRANSLATE_ADDRESS_ROUTINE64)(
    HANDLE    hProcess,
    HANDLE    hThread,
    LPADDRESS64 lpaddr
    );
#define SYMOPT_CASE_INSENSITIVE         0x00000001
#define SYMOPT_UNDNAME                  0x00000002
#define SYMOPT_DEFERRED_LOADS           0x00000004
#define SYMOPT_NO_CPP                   0x00000008
#define SYMOPT_LOAD_LINES               0x00000010
#define SYMOPT_OMAP_FIND_NEAREST        0x00000020
#define SYMOPT_LOAD_ANYTHING            0x00000040
#define SYMOPT_IGNORE_CVREC             0x00000080
#define SYMOPT_NO_UNQUALIFIED_LOADS     0x00000100
#define SYMOPT_FAIL_CRITICAL_ERRORS     0x00000200
#define SYMOPT_EXACT_SYMBOLS            0x00000400
#define SYMOPT_ALLOW_ABSOLUTE_SYMBOLS   0x00000800
#define SYMOPT_IGNORE_NT_SYMPATH        0x00001000
#define SYMOPT_INCLUDE_32BIT_MODULES    0x00002000
#define SYMOPT_PUBLICS_ONLY             0x00004000
#define SYMOPT_NO_PUBLICS               0x00008000
#define SYMOPT_AUTO_PUBLICS             0x00010000
#define SYMOPT_NO_IMAGE_SEARCH          0x00020000
#define SYMOPT_SECURE                   0x00040000
#define SYMOPT_DEBUG                    0x80000000
#define UNDNAME_COMPLETE                 (0x0000)  // Enable full undecoration
#define UNDNAME_NAME_ONLY                (0x1000)  // Crack only the name for primary declaration;
#endif  // _MSC_VER < 1300

// Some missing defines (for VC5/6):
#ifndef INVALID_FILE_ATTRIBUTES
#define INVALID_FILE_ATTRIBUTES ((DWORD)-1)
#endif  


// secure-CRT_functions are only available starting with VC8
#if _MSC_VER < 1400
#define strcpy_s strcpy
#define strcat_s(dst, len, src) strcat(dst, src)
#define _snprintf_s _snprintf
#define _tcscat_s _tcscat
#endif

// Normally it should be enough to use 'CONTEXT_FULL' (better would be 'CONTEXT_ALL')
#define USED_CONTEXT_FLAGS CONTEXT_FULL


namespace XTSystem { namespace Stack {
class StackWalker;


  typedef BOOL (__stdcall *PReadProcessMemoryRoutine)(
    HANDLE      hProcess,
    DWORD64     qwBaseAddress,
    PVOID       lpBuffer,
    DWORD       nSize,
    LPDWORD     lpNumberOfBytesRead,
    LPVOID      pUserData  // optional data, which was passed in "ShowCallstack"
    );

static PReadProcessMemoryRoutine &s_readMemoryFunction() {  static PReadProcessMemoryRoutine x=NULL; return x; }
static LPVOID &s_readMemoryFunction_UserData() { static LPVOID x = NULL; return x; }

class StackWalker
{
public:
  typedef enum StackWalkOptions
  {
    // No addition info will be retrived 
    // (only the address is available)
    RetrieveNone = 0,
    
    // Try to get the symbol-name
    RetrieveSymbol = 1,
    
    // Try to get the line for this symbol
    RetrieveLine = 2,
    
    // Try to retrieve the module-infos
    RetrieveModuleInfo = 4,
    
    // Also retrieve the version for the DLL/EXE
    RetrieveFileVersion = 8,
    
    // Contains all the abouve
    RetrieveVerbose = 0xF,
    
    // Generate a "good" symbol-search-path
    SymBuildPath = 0x10,
    
    // Also use the public Microsoft-Symbol-Server
    SymUseSymSrv = 0x20,
    
    // Contains all the abouve "Sym"-options
    SymAll = 0x30,
    
    // Contains all options (default)
    OptionsAll = 0x3F
  } StackWalkOptions;

 
    // #############################################################
    StackWalker(DWORD dwProcessId, HANDLE hProcess)
    {
      this->m_options = OptionsAll;
      this->m_modulesLoaded = FALSE;
      this->m_hProcess = hProcess;
      this->m_sw = new StackWalker::StackWalkerInternal(this, this->m_hProcess);
      this->m_dwProcessId = dwProcessId;
      this->m_szSymPath = NULL;
    }

  StackWalker(
    int options = OptionsAll, // 'int' is by design, to combine the enum-flags
    LPCSTR szSymPath = NULL, 
    DWORD dwProcessId = GetCurrentProcessId(), 
    HANDLE hProcess = GetCurrentProcess()
    )
    {
      this->m_options = options;
      this->m_modulesLoaded = FALSE;
      this->m_hProcess = hProcess;
      this->m_sw = new StackWalker::StackWalkerInternal(this, this->m_hProcess);
      this->m_dwProcessId = dwProcessId;
      if (szSymPath != NULL)
      {
        this->m_szSymPath = _strdup(szSymPath);
        this->m_options |= SymBuildPath;
      }
      else
        this->m_szSymPath = NULL;
    }

    virtual ~StackWalker()
    {
      if (m_szSymPath != NULL)
        free(m_szSymPath);
      m_szSymPath = NULL;
      if (this->m_sw != NULL)
        delete this->m_sw;
      this->m_sw = NULL;
    }



    BOOL LoadModules()
    {
      if (this->m_sw == NULL)
      {
        SetLastError(ERROR_DLL_INIT_FAILED);
        return FALSE;
      }
      if (m_modulesLoaded != FALSE)
        return TRUE;

      // Build the sym-path:
      char *szSymPath = NULL;
      if ( (this->m_options & SymBuildPath) != 0)
      {
        const size_t nSymPathLen = 4096;
        szSymPath = (char*) malloc(nSymPathLen);
        if (szSymPath == NULL)
        {
          SetLastError(ERROR_NOT_ENOUGH_MEMORY);
          return FALSE;
        }
        szSymPath[0] = 0;
        // Now first add the (optional) provided sympath:
        if (this->m_szSymPath != NULL)
        {
          strcat_s(szSymPath, nSymPathLen, this->m_szSymPath);
          strcat_s(szSymPath, nSymPathLen, ";");
        }

        strcat_s(szSymPath, nSymPathLen, ".;");

        const size_t nTempLen = 1024;
        char szTemp[nTempLen];
        // Now add the current directory:
        if (GetCurrentDirectoryA(nTempLen, szTemp) > 0)
        {
          szTemp[nTempLen-1] = 0;
          strcat_s(szSymPath, nSymPathLen, szTemp);
          strcat_s(szSymPath, nSymPathLen, ";");
        }

        // Now add the path for the main-module:
        if (GetModuleFileNameA(NULL, szTemp, nTempLen) > 0)
        {
          szTemp[nTempLen-1] = 0;
          for (char *p = (szTemp+strlen(szTemp)-1); p >= szTemp; --p)
          {
            // locate the rightmost path separator
            if ( (*p == '\\') || (*p == '/') || (*p == ':') )
            {
              *p = 0;
              break;
            }
          }  // for (search for path separator...)
          if (strlen(szTemp) > 0)
          {
            strcat_s(szSymPath, nSymPathLen, szTemp);
            strcat_s(szSymPath, nSymPathLen, ";");
          }
        }
        if (GetEnvironmentVariableA("_NT_SYMBOL_PATH", szTemp, nTempLen) > 0)
        {
          szTemp[nTempLen-1] = 0;
          strcat_s(szSymPath, nSymPathLen, szTemp);
          strcat_s(szSymPath, nSymPathLen, ";");
        }
        if (GetEnvironmentVariableA("_NT_ALTERNATE_SYMBOL_PATH", szTemp, nTempLen) > 0)
        {
          szTemp[nTempLen-1] = 0;
          strcat_s(szSymPath, nSymPathLen, szTemp);
          strcat_s(szSymPath, nSymPathLen, ";");
        }
        if (GetEnvironmentVariableA("SYSTEMROOT", szTemp, nTempLen) > 0)
        {
          szTemp[nTempLen-1] = 0;
          strcat_s(szSymPath, nSymPathLen, szTemp);
          strcat_s(szSymPath, nSymPathLen, ";");
          // also add the "system32"-directory:
          strcat_s(szTemp, nTempLen, "\\system32");
          strcat_s(szSymPath, nSymPathLen, szTemp);
          strcat_s(szSymPath, nSymPathLen, ";");
        }

        if ( (this->m_options & SymBuildPath) != 0)
        {
          if (GetEnvironmentVariableA("SYSTEMDRIVE", szTemp, nTempLen) > 0)
          {
            szTemp[nTempLen-1] = 0;
            strcat_s(szSymPath, nSymPathLen, "SRV*");
            strcat_s(szSymPath, nSymPathLen, szTemp);
            strcat_s(szSymPath, nSymPathLen, "\\websymbols");
            strcat_s(szSymPath, nSymPathLen, "*http://msdl.microsoft.com/download/symbols;");
          }
          else
            strcat_s(szSymPath, nSymPathLen, "SRV*c:\\websymbols*http://msdl.microsoft.com/download/symbols;");
        }
      }

      // First Init the whole stuff...
      BOOL bRet = this->m_sw->Init(szSymPath);
      if (szSymPath != NULL) free(szSymPath); szSymPath = NULL;
      if (bRet == FALSE)
      {
        this->OnDbgHelpErr("Error while initializing dbghelp.dll", 0, 0);
        SetLastError(ERROR_DLL_INIT_FAILED);
        return FALSE;
      }

      bRet = this->m_sw->LoadModules(this->m_hProcess, this->m_dwProcessId);
      if (bRet != FALSE)
        m_modulesLoaded = TRUE;
      return bRet;
    }

  BOOL ShowCallstack(
    HANDLE hThread = GetCurrentThread(), 
    const CONTEXT *context = NULL, 
    PReadProcessMemoryRoutine readMemoryFunction = NULL,
    LPVOID pUserData = NULL  // optional to identify some data in the 'readMemoryFunction'-callback
    )
  {
      CONTEXT c;;
      CallstackEntry csEntry;
      IMAGEHLP_SYMBOL64 *pSym = NULL;
      StackWalkerInternal::IMAGEHLP_MODULE64_V2 Module;
      IMAGEHLP_LINE64 Line;
      int frameNum;

      if (m_modulesLoaded == FALSE)
        this->LoadModules();  // ignore the result...

      if (this->m_sw->m_hDbhHelp == NULL)
      {
        SetLastError(ERROR_DLL_INIT_FAILED);
        return FALSE;
      }

      s_readMemoryFunction() = readMemoryFunction;
      s_readMemoryFunction_UserData() = pUserData;

      if (context == NULL)
      {
        // If no context is provided, capture the context
        if (hThread == GetCurrentThread())
        {
          GET_CURRENT_CONTEXT(c, USED_CONTEXT_FLAGS);
        }
        else
        {
          SuspendThread(hThread);
          memset(&c, 0, sizeof(CONTEXT));
          c.ContextFlags = USED_CONTEXT_FLAGS;
          if (GetThreadContext(hThread, &c) == FALSE)
          {
            ResumeThread(hThread);
            return FALSE;
          }
        }
      }
      else
        c = *context;

      // init STACKFRAME for first call
      STACKFRAME64 s; // in/out stackframe
      memset(&s, 0, sizeof(s));
      DWORD imageType;
    #ifdef _M_IX86
      // normally, call ImageNtHeader() and use machine info from PE header
      imageType = IMAGE_FILE_MACHINE_I386;
      s.AddrPC.Offset = c.Eip;
      s.AddrPC.Mode = AddrModeFlat;
      s.AddrFrame.Offset = c.Ebp;
      s.AddrFrame.Mode = AddrModeFlat;
      s.AddrStack.Offset = c.Esp;
      s.AddrStack.Mode = AddrModeFlat;
    #elif _M_X64
      imageType = IMAGE_FILE_MACHINE_AMD64;
      s.AddrPC.Offset = c.Rip;
      s.AddrPC.Mode = AddrModeFlat;
      s.AddrFrame.Offset = c.Rsp;
      s.AddrFrame.Mode = AddrModeFlat;
      s.AddrStack.Offset = c.Rsp;
      s.AddrStack.Mode = AddrModeFlat;
    #elif _M_IA64
      imageType = IMAGE_FILE_MACHINE_IA64;
      s.AddrPC.Offset = c.StIIP;
      s.AddrPC.Mode = AddrModeFlat;
      s.AddrFrame.Offset = c.IntSp;
      s.AddrFrame.Mode = AddrModeFlat;
      s.AddrBStore.Offset = c.RsBSP;
      s.AddrBStore.Mode = AddrModeFlat;
      s.AddrStack.Offset = c.IntSp;
      s.AddrStack.Mode = AddrModeFlat;
    #else
    #error "Platform not supported!"
    #endif

      pSym = (IMAGEHLP_SYMBOL64 *) malloc(sizeof(IMAGEHLP_SYMBOL64) + STACKWALK_MAX_NAMELEN);
      if (!pSym) goto cleanup;  // not enough memory...
      memset(pSym, 0, sizeof(IMAGEHLP_SYMBOL64) + STACKWALK_MAX_NAMELEN);
      pSym->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64);
      pSym->MaxNameLength = STACKWALK_MAX_NAMELEN;

      memset(&Line, 0, sizeof(Line));
      Line.SizeOfStruct = sizeof(Line);

      memset(&Module, 0, sizeof(Module));
      Module.SizeOfStruct = sizeof(Module);

      for (frameNum = 0; ; ++frameNum )
      {
        // get next stack frame (StackWalk64(), SymFunctionTableAccess64(), SymGetModuleBase64())
        // if this returns ERROR_INVALID_ADDRESS (487) or ERROR_NOACCESS (998), you can
        // assume that either you are done, or that the stack is so hosed that the next
        // deeper frame could not be found.
        // CONTEXT need not to be suplied if imageTyp is IMAGE_FILE_MACHINE_I386!
        if ( ! this->m_sw->pSW(imageType, this->m_hProcess, hThread, &s, &c, myReadProcMem, this->m_sw->pSFTA, this->m_sw->pSGMB, NULL) )
        {
          this->OnDbgHelpErr("StackWalk64", GetLastError(), s.AddrPC.Offset);
          break;
        }

        csEntry.offset = s.AddrPC.Offset;
        csEntry.name[0] = 0;
        csEntry.undName[0] = 0;
        csEntry.undFullName[0] = 0;
        csEntry.offsetFromSmybol = 0;
        csEntry.offsetFromLine = 0;
        csEntry.lineFileName[0] = 0;
        csEntry.lineNumber = 0;
        csEntry.loadedImageName[0] = 0;
        csEntry.moduleName[0] = 0;
        if (s.AddrPC.Offset == s.AddrReturn.Offset)
        {
          this->OnDbgHelpErr("StackWalk64-Endless-Callstack!", 0, s.AddrPC.Offset);
          break;
        }
        if (s.AddrPC.Offset != 0)
        {
          // we seem to have a valid PC
          // show procedure info (SymGetSymFromAddr64())
          if (this->m_sw->pSGSFA(this->m_hProcess, s.AddrPC.Offset, &(csEntry.offsetFromSmybol), pSym) != FALSE)
          {
            // TODO: Mache dies sicher...!
            strcpy_s(csEntry.name, pSym->Name);
            // UnDecorateSymbolName()
            this->m_sw->pUDSN( pSym->Name, csEntry.undName, STACKWALK_MAX_NAMELEN, UNDNAME_NAME_ONLY );
            this->m_sw->pUDSN( pSym->Name, csEntry.undFullName, STACKWALK_MAX_NAMELEN, UNDNAME_COMPLETE );
          }
          else
          {
            this->OnDbgHelpErr("SymGetSymFromAddr64", GetLastError(), s.AddrPC.Offset);
          }

          // show line number info, NT5.0-method (SymGetLineFromAddr64())
          if (this->m_sw->pSGLFA != NULL )
          { // yes, we have SymGetLineFromAddr64()
            if (this->m_sw->pSGLFA(this->m_hProcess, s.AddrPC.Offset, &(csEntry.offsetFromLine), &Line) != FALSE)
            {
              csEntry.lineNumber = Line.LineNumber;
              // TODO: Mache dies sicher...!
              strcpy_s(csEntry.lineFileName, Line.FileName);
            }
            else
            {
              this->OnDbgHelpErr("SymGetLineFromAddr64", GetLastError(), s.AddrPC.Offset);
            }
          } // yes, we have SymGetLineFromAddr64()

          // show module info (SymGetModuleInfo64())
          if (this->m_sw->GetModuleInfo(this->m_hProcess, s.AddrPC.Offset, &Module ) != FALSE)
          { // got module info OK
            switch ( Module.SymType )
            {
            case SymNone:
              csEntry.symTypeString = "-nosymbols-";
              break;
            case SymCoff:
              csEntry.symTypeString = "COFF";
              break;
            case SymCv:
              csEntry.symTypeString = "CV";
              break;
            case SymPdb:
              csEntry.symTypeString = "PDB";
              break;
            case SymExport:
              csEntry.symTypeString = "-exported-";
              break;
            case SymDeferred:
              csEntry.symTypeString = "-deferred-";
              break;
            case SymSym:
              csEntry.symTypeString = "SYM";
              break;
    #if API_VERSION_NUMBER >= 9
            case SymDia:
              csEntry.symTypeString = "DIA";
              break;
    #endif
            case 8: //SymVirtual:
              csEntry.symTypeString = "Virtual";
              break;
            default:
              //_snprintf( ty, sizeof ty, "symtype=%ld", (long) Module.SymType );
              csEntry.symTypeString = NULL;
              break;
            }

            // TODO: Mache dies sicher...!
            strcpy_s(csEntry.moduleName, Module.ModuleName);
            csEntry.baseOfImage = Module.BaseOfImage;
            strcpy_s(csEntry.loadedImageName, Module.LoadedImageName);
          } // got module info OK
          else
          {
            this->OnDbgHelpErr("SymGetModuleInfo64", GetLastError(), s.AddrPC.Offset);
          }
        } // we seem to have a valid PC

        CallstackEntryType et = nextEntry;
        if (frameNum == 0)
          et = firstEntry;
        this->OnCallstackEntry(et, csEntry);
    
        if (s.AddrReturn.Offset == 0)
        {
          this->OnCallstackEntry(lastEntry, csEntry);
          SetLastError(ERROR_SUCCESS);
          break;
        }
      } // for ( frameNum )

      cleanup:
        if (pSym) free( pSym );

      if (context == NULL)
        ResumeThread(hThread);

      return TRUE;
    }


#if _MSC_VER >= 1300
// due to some reasons, the "STACKWALK_MAX_NAMELEN" must be declared as "public" 
// in older compilers in order to use it... starting with VC7 we can declare it as "protected"
protected:
#endif
    enum { STACKWALK_MAX_NAMELEN = 1024 }; // max name length for found symbols

protected:
  // Entry for each Callstack-Entry
  typedef struct CallstackEntry
  {
    DWORD64 offset;  // if 0, we have no valid entry
    CHAR name[STACKWALK_MAX_NAMELEN];
    CHAR undName[STACKWALK_MAX_NAMELEN];
    CHAR undFullName[STACKWALK_MAX_NAMELEN];
    DWORD64 offsetFromSmybol;
    DWORD offsetFromLine;
    DWORD lineNumber;
    CHAR lineFileName[STACKWALK_MAX_NAMELEN];
    DWORD symType;
    LPCSTR symTypeString;
    CHAR moduleName[STACKWALK_MAX_NAMELEN];
    DWORD64 baseOfImage;
    CHAR loadedImageName[STACKWALK_MAX_NAMELEN];
  } CallstackEntry;

  enum CallstackEntryType {firstEntry, nextEntry, lastEntry};

  virtual void OnSymInit(LPCSTR szSearchPath, DWORD symOptions, LPCSTR szUserName)
    {
      CHAR buffer[STACKWALK_MAX_NAMELEN];
      _snprintf_s(buffer, STACKWALK_MAX_NAMELEN, "SymInit: Symbol-SearchPath: '%ws', symOptions: %d, UserName: '%ws'\n", szSearchPath, symOptions, szUserName);
      OnOutput(buffer);
      // Also display the OS-version
    #if _MSC_VER <= 1200
      OSVERSIONINFO ver;
      ZeroMemory(&ver, sizeof(OSVERSIONINFO));
      ver.dwOSVersionInfoSize = sizeof(ver);
      if (GetVersionEx(&ver) != FALSE)
      {
        _snprintf_s(buffer, STACKWALK_MAX_NAMELEN, "OS-Version: %d.%d.%d (%ws)\n", 
          ver.dwMajorVersion, ver.dwMinorVersion, ver.dwBuildNumber,
          ver.szCSDVersion);
        OnOutput(buffer);
      }
    #else
      OSVERSIONINFOEX ver;
      ZeroMemory(&ver, sizeof(OSVERSIONINFOEX));
      ver.dwOSVersionInfoSize = sizeof(ver);
#pragma warning(disable: 4996)
      if (GetVersionEx( (OSVERSIONINFO*) &ver) != FALSE)
      {
        _snprintf_s(buffer, STACKWALK_MAX_NAMELEN, "OS-Version: %d.%d.%d (%ws) 0x%x-0x%x\n", 
          ver.dwMajorVersion, ver.dwMinorVersion, ver.dwBuildNumber,
          ver.szCSDVersion, ver.wSuiteMask, ver.wProductType);
        OnOutput(buffer);
      }
#pragma warning(default: 4996)

    #endif
    }


  virtual void OnLoadModule(LPCSTR img, LPCSTR mod, DWORD64 baseAddr, DWORD size, DWORD result, LPCSTR symType, LPCSTR pdbName, ULONGLONG fileVersion)
    {
      CHAR buffer[STACKWALK_MAX_NAMELEN];
      if (fileVersion == 0)
        _snprintf_s(buffer, STACKWALK_MAX_NAMELEN, "%s:%s (%p), size: %d (result: %d), SymType: '%s', PDB: '%s'\n", img, mod, (LPVOID) baseAddr, size, result, symType, pdbName);
      else
      {
        DWORD v4 = (DWORD) fileVersion & 0xFFFF;
        DWORD v3 = (DWORD) (fileVersion>>16) & 0xFFFF;
        DWORD v2 = (DWORD) (fileVersion>>32) & 0xFFFF;
        DWORD v1 = (DWORD) (fileVersion>>48) & 0xFFFF;
        _snprintf_s(buffer, STACKWALK_MAX_NAMELEN, "%s:%s (%p), size: %d (result: %d), SymType: '%s', PDB: '%s', fileVersion: %d.%d.%d.%d\n", img, mod, (LPVOID) baseAddr, size, result, symType, pdbName, v1, v2, v3, v4);
      }
      OnOutput(buffer);
    }

  virtual void OnCallstackEntry(CallstackEntryType eType, CallstackEntry &entry)
    {
      CHAR buffer[STACKWALK_MAX_NAMELEN];
      if ( (eType != lastEntry) && (entry.offset != 0) )
      {
        if (entry.name[0] == 0)
          strcpy_s(entry.name, "(function-name not available)");
        if (entry.undName[0] != 0)
          strcpy_s(entry.name, entry.undName);
        if (entry.undFullName[0] != 0)
          strcpy_s(entry.name, entry.undFullName);
        if (entry.lineFileName[0] == 0)
        {
          strcpy_s(entry.lineFileName, "(filename not available)");
          if (entry.moduleName[0] == 0)
            strcpy_s(entry.moduleName, "(module-name not available)");
          _snprintf_s(buffer, STACKWALK_MAX_NAMELEN, "%p (%s): %s: %s\n", (LPVOID) entry.offset, entry.moduleName, entry.lineFileName, entry.name);
        }
        else
          _snprintf_s(buffer, STACKWALK_MAX_NAMELEN, "%s (%d): %s\n", entry.lineFileName, entry.lineNumber, entry.name);
        OnOutput(buffer);
      }
    }
  virtual void OnDbgHelpErr(LPCSTR szFuncName, DWORD gle, DWORD64 addr)
    {
      CHAR buffer[STACKWALK_MAX_NAMELEN];
      _snprintf_s(buffer, STACKWALK_MAX_NAMELEN, "ERROR: %s, GetLastError: %d (Address: %p)\n", szFuncName, gle, (LPVOID) addr);
      OnOutput(buffer);
    }

    virtual void OnOutput(LPCSTR buffer)
    {
      //OutputDebugStringA(buffer);
    }


  class StackWalkerInternal;
  StackWalkerInternal *m_sw;
  HANDLE m_hProcess;
  DWORD m_dwProcessId;
  BOOL m_modulesLoaded;
  LPSTR m_szSymPath;

  int m_options;

    static BOOL __stdcall myReadProcMem(
        HANDLE      hProcess,
        DWORD64     qwBaseAddress,
        PVOID       lpBuffer,
        DWORD       nSize,
        LPDWORD     lpNumberOfBytesRead
        )
    {
      if (s_readMemoryFunction() == NULL)
      {
        SIZE_T st;
        BOOL bRet = ReadProcessMemory(hProcess, (LPVOID) qwBaseAddress, lpBuffer, nSize, &st);
        *lpNumberOfBytesRead = (DWORD) st;
        //printf("ReadMemory: hProcess: %p, baseAddr: %p, buffer: %p, size: %d, read: %d, result: %d\n", hProcess, (LPVOID) qwBaseAddress, lpBuffer, nSize, (DWORD) st, (DWORD) bRet);
        return bRet;
      }
      else
      {
        return s_readMemoryFunction()(hProcess, qwBaseAddress, lpBuffer, nSize, lpNumberOfBytesRead, s_readMemoryFunction_UserData);
      }
    }
  //friend StackWalkerInternal;

    class StackWalkerInternal
    {
    public:
      StackWalkerInternal(StackWalker *parent, HANDLE hProcess)
      {
        m_parent = parent;
        m_hDbhHelp = NULL;
        pSC = NULL;
        m_hProcess = hProcess;
        m_szSymPath = NULL;
        pSFTA = NULL;
        pSGLFA = NULL;
        pSGMB = NULL;
        pSGMI = NULL;
        pSGO = NULL;
        pSGSFA = NULL;
        pSI = NULL;
        pSLM = NULL;
        pSSO = NULL;
        pSW = NULL;
        pUDSN = NULL;
        pSGSP = NULL;
      }
      ~StackWalkerInternal()
      {
        if (pSC != NULL)
          pSC(m_hProcess);  // SymCleanup
        if (m_hDbhHelp != NULL)
          FreeLibrary(m_hDbhHelp);
        m_hDbhHelp = NULL;
        m_parent = NULL;
        if(m_szSymPath != NULL)
          free(m_szSymPath);
        m_szSymPath = NULL;
      }

      BOOL Init(LPCSTR szSymPath)
      {
        if (m_parent == NULL)
          return FALSE;
        // Dynamically load the Entry-Points for dbghelp.dll:
        // First try to load the newsest one from
        TCHAR szTemp[4096];
        // But before wqe do this, we first check if the ".local" file exists
        if (GetModuleFileName(NULL, szTemp, 4096) > 0)
        {
          _tcscat_s(szTemp, _T(".local"));
          if (GetFileAttributes(szTemp) == INVALID_FILE_ATTRIBUTES)
          {
            // ".local" file does not exist, so we can try to load the dbghelp.dll from the "Debugging Tools for Windows"
            if (GetEnvironmentVariable(_T("ProgramFiles"), szTemp, 4096) > 0)
            {
              _tcscat_s(szTemp, _T("\\Debugging Tools for Windows\\dbghelp.dll"));
              // now check if the file exists:
              if (GetFileAttributes(szTemp) != INVALID_FILE_ATTRIBUTES)
              {
                m_hDbhHelp = LoadLibrary(szTemp);
              }
            }
              // Still not found? Then try to load the 64-Bit version:
            if ( (m_hDbhHelp == NULL) && (GetEnvironmentVariable(_T("ProgramFiles"), szTemp, 4096) > 0) )
            {
              _tcscat_s(szTemp, _T("\\Debugging Tools for Windows 64-Bit\\dbghelp.dll"));
              if (GetFileAttributes(szTemp) != INVALID_FILE_ATTRIBUTES)
              {
                m_hDbhHelp = LoadLibrary(szTemp);
              }
            }
          }
        }
        if (m_hDbhHelp == NULL)  // if not already loaded, try to load a default-one
          m_hDbhHelp = LoadLibrary( _T("dbghelp.dll") );
        if (m_hDbhHelp == NULL)
          return FALSE;
        pSI = (tSI) GetProcAddress(m_hDbhHelp, "SymInitialize" );
        pSC = (tSC) GetProcAddress(m_hDbhHelp, "SymCleanup" );

        pSW = (tSW) GetProcAddress(m_hDbhHelp, "StackWalk64" );
        pSGO = (tSGO) GetProcAddress(m_hDbhHelp, "SymGetOptions" );
        pSSO = (tSSO) GetProcAddress(m_hDbhHelp, "SymSetOptions" );

        pSFTA = (tSFTA) GetProcAddress(m_hDbhHelp, "SymFunctionTableAccess64" );
        pSGLFA = (tSGLFA) GetProcAddress(m_hDbhHelp, "SymGetLineFromAddr64" );
        pSGMB = (tSGMB) GetProcAddress(m_hDbhHelp, "SymGetModuleBase64" );
        pSGMI = (tSGMI) GetProcAddress(m_hDbhHelp, "SymGetModuleInfo64" );
        //pSGMI_V3 = (tSGMI_V3) GetProcAddress(m_hDbhHelp, "SymGetModuleInfo64" );
        pSGSFA = (tSGSFA) GetProcAddress(m_hDbhHelp, "SymGetSymFromAddr64" );
        pUDSN = (tUDSN) GetProcAddress(m_hDbhHelp, "UnDecorateSymbolName" );
        pSLM = (tSLM) GetProcAddress(m_hDbhHelp, "SymLoadModule64" );
        pSGSP =(tSGSP) GetProcAddress(m_hDbhHelp, "SymGetSearchPath" );

        if ( pSC == NULL || pSFTA == NULL || pSGMB == NULL || pSGMI == NULL ||
          pSGO == NULL || pSGSFA == NULL || pSI == NULL || pSSO == NULL ||
          pSW == NULL || pUDSN == NULL || pSLM == NULL )
        {
          FreeLibrary(m_hDbhHelp);
          m_hDbhHelp = NULL;
          pSC = NULL;
          return FALSE;
        }

        // SymInitialize
        if (szSymPath != NULL)
          m_szSymPath = _strdup(szSymPath);
        if (this->pSI(m_hProcess, m_szSymPath, FALSE) == FALSE)
          this->m_parent->OnDbgHelpErr("SymInitialize", GetLastError(), 0);
      
        DWORD symOptions = this->pSGO();  // SymGetOptions
        symOptions |= SYMOPT_LOAD_LINES;
        symOptions |= SYMOPT_FAIL_CRITICAL_ERRORS;
        //symOptions |= SYMOPT_NO_PROMPTS;
        // SymSetOptions
        symOptions = this->pSSO(symOptions);

        char buf[StackWalker::STACKWALK_MAX_NAMELEN] = {0};
        if (this->pSGSP != NULL)
        {
          if (this->pSGSP(m_hProcess, buf, StackWalker::STACKWALK_MAX_NAMELEN) == FALSE)
            this->m_parent->OnDbgHelpErr("SymGetSearchPath", GetLastError(), 0);
        }
        char szUserName[1024] = {0};
        DWORD dwSize = 1024;
        GetUserNameA(szUserName, &dwSize);
        this->m_parent->OnSymInit(buf, symOptions, szUserName);

        return TRUE;
      }

      StackWalker *m_parent;

      HMODULE m_hDbhHelp;
      HANDLE m_hProcess;
      LPSTR m_szSymPath;

    /*typedef struct IMAGEHLP_MODULE64_V3 {
        DWORD    SizeOfStruct;           // set to sizeof(IMAGEHLP_MODULE64)
        DWORD64  BaseOfImage;            // base load address of module
        DWORD    ImageSize;              // virtual size of the loaded module
        DWORD    TimeDateStamp;          // date/time stamp from pe header
        DWORD    CheckSum;               // checksum from the pe header
        DWORD    NumSyms;                // number of symbols in the symbol table
        SYM_TYPE SymType;                // type of symbols loaded
        CHAR     ModuleName[32];         // module name
        CHAR     ImageName[256];         // image name
        // new elements: 07-Jun-2002
        CHAR     LoadedImageName[256];   // symbol file name
        CHAR     LoadedPdbName[256];     // pdb file name
        DWORD    CVSig;                  // Signature of the CV record in the debug directories
        CHAR         CVData[MAX_PATH * 3];   // Contents of the CV record
        DWORD    PdbSig;                 // Signature of PDB
        GUID     PdbSig70;               // Signature of PDB (VC 7 and up)
        DWORD    PdbAge;                 // DBI age of pdb
        BOOL     PdbUnmatched;           // loaded an unmatched pdb
        BOOL     DbgUnmatched;           // loaded an unmatched dbg
        BOOL     LineNumbers;            // we have line number information
        BOOL     GlobalSymbols;          // we have internal symbol information
        BOOL     TypeInfo;               // we have type information
        // new elements: 17-Dec-2003
        BOOL     SourceIndexed;          // pdb supports source server
        BOOL     Publics;                // contains public symbols
    };
    */
    typedef struct IMAGEHLP_MODULE64_V2 {
        DWORD    SizeOfStruct;           // set to sizeof(IMAGEHLP_MODULE64)
        DWORD64  BaseOfImage;            // base load address of module
        DWORD    ImageSize;              // virtual size of the loaded module
        DWORD    TimeDateStamp;          // date/time stamp from pe header
        DWORD    CheckSum;               // checksum from the pe header
        DWORD    NumSyms;                // number of symbols in the symbol table
        SYM_TYPE SymType;                // type of symbols loaded
        CHAR     ModuleName[32];         // module name
        CHAR     ImageName[256];         // image name
        CHAR     LoadedImageName[256];   // symbol file name
    } IMAGEHLP_MODULE64_V2_t;


      // SymCleanup()
      typedef BOOL (__stdcall *tSC)( IN HANDLE hProcess );
      tSC pSC;

      // SymFunctionTableAccess64()
      typedef PVOID (__stdcall *tSFTA)( HANDLE hProcess, DWORD64 AddrBase );
      tSFTA pSFTA;

      // SymGetLineFromAddr64()
      typedef BOOL (__stdcall *tSGLFA)( IN HANDLE hProcess, IN DWORD64 dwAddr,
        OUT PDWORD pdwDisplacement, OUT PIMAGEHLP_LINE64 Line );
      tSGLFA pSGLFA;

      // SymGetModuleBase64()
      typedef DWORD64 (__stdcall *tSGMB)( IN HANDLE hProcess, IN DWORD64 dwAddr );
      tSGMB pSGMB;

      // SymGetModuleInfo64()
      typedef BOOL (__stdcall *tSGMI)( IN HANDLE hProcess, IN DWORD64 dwAddr, OUT IMAGEHLP_MODULE64_V2 *ModuleInfo );
      tSGMI pSGMI;

    //  // SymGetModuleInfo64()
    //  typedef BOOL (__stdcall *tSGMI_V3)( IN HANDLE hProcess, IN DWORD64 dwAddr, OUT IMAGEHLP_MODULE64_V3 *ModuleInfo );
    //  tSGMI_V3 pSGMI_V3;

      // SymGetOptions()
      typedef DWORD (__stdcall *tSGO)( VOID );
      tSGO pSGO;

      // SymGetSymFromAddr64()
      typedef BOOL (__stdcall *tSGSFA)( IN HANDLE hProcess, IN DWORD64 dwAddr,
        OUT PDWORD64 pdwDisplacement, OUT PIMAGEHLP_SYMBOL64 Symbol );
      tSGSFA pSGSFA;

      // SymInitialize()
      typedef BOOL (__stdcall *tSI)( IN HANDLE hProcess, IN PSTR UserSearchPath, IN BOOL fInvadeProcess );
      tSI pSI;

      // SymLoadModule64()
      typedef DWORD64 (__stdcall *tSLM)( IN HANDLE hProcess, IN HANDLE hFile,
        IN PSTR ImageName, IN PSTR ModuleName, IN DWORD64 BaseOfDll, IN DWORD SizeOfDll );
      tSLM pSLM;

      // SymSetOptions()
      typedef DWORD (__stdcall *tSSO)( IN DWORD SymOptions );
      tSSO pSSO;

      // StackWalk64()
      typedef BOOL (__stdcall *tSW)( 
        DWORD MachineType, 
        HANDLE hProcess,
        HANDLE hThread, 
        LPSTACKFRAME64 StackFrame, 
        PVOID ContextRecord,
        PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemoryRoutine,
        PFUNCTION_TABLE_ACCESS_ROUTINE64 FunctionTableAccessRoutine,
        PGET_MODULE_BASE_ROUTINE64 GetModuleBaseRoutine,
        PTRANSLATE_ADDRESS_ROUTINE64 TranslateAddress );
      tSW pSW;

      // UnDecorateSymbolName()
      typedef DWORD (__stdcall WINAPI *tUDSN)( PCSTR DecoratedName, PSTR UnDecoratedName,
        DWORD UndecoratedLength, DWORD Flags );
      tUDSN pUDSN;

      typedef BOOL (__stdcall WINAPI *tSGSP)(HANDLE hProcess, PSTR SearchPath, DWORD SearchPathLength);
      tSGSP pSGSP;


    private:
      // **************************************** ToolHelp32 ************************
      #define MAX_MODULE_NAME32 255
      #define TH32CS_SNAPMODULE   0x00000008
      #pragma pack( push, 8 )
      typedef struct tagMODULEENTRY32
      {
          DWORD   dwSize;
          DWORD   th32ModuleID;       // This module
          DWORD   th32ProcessID;      // owning process
          DWORD   GlblcntUsage;       // Global usage count on the module
          DWORD   ProccntUsage;       // Module usage count in th32ProcessID's context
          BYTE  * modBaseAddr;        // Base address of module in th32ProcessID's context
          DWORD   modBaseSize;        // Size in bytes of module starting at modBaseAddr
          HMODULE hModule;            // The hModule of this module in th32ProcessID's context
          char    szModule[MAX_MODULE_NAME32 + 1];
          char    szExePath[MAX_PATH];
      } MODULEENTRY32;
      typedef MODULEENTRY32 *  PMODULEENTRY32;
      typedef MODULEENTRY32 *  LPMODULEENTRY32;
      #pragma pack( pop )

      BOOL GetModuleListTH32(HANDLE hProcess, DWORD pid)
      {
        // CreateToolhelp32Snapshot()
        typedef HANDLE (__stdcall *tCT32S)(DWORD dwFlags, DWORD th32ProcessID);
        // Module32First()
        typedef BOOL (__stdcall *tM32F)(HANDLE hSnapshot, LPMODULEENTRY32 lpme);
        // Module32Next()
        typedef BOOL (__stdcall *tM32N)(HANDLE hSnapshot, LPMODULEENTRY32 lpme);

        // try both dlls...
        const TCHAR *dllname[] = { _T("kernel32.dll"), _T("tlhelp32.dll") };
        HINSTANCE hToolhelp = NULL;
        tCT32S pCT32S = NULL;
        tM32F pM32F = NULL;
        tM32N pM32N = NULL;

        HANDLE hSnap;
        MODULEENTRY32 me;
        me.dwSize = sizeof(me);
        BOOL keepGoing;
        size_t i;

        for (i = 0; i<(sizeof(dllname) / sizeof(dllname[0])); i++ )
        {
          hToolhelp = LoadLibrary( dllname[i] );
          if (hToolhelp == NULL)
            continue;
          pCT32S = (tCT32S) GetProcAddress(hToolhelp, "CreateToolhelp32Snapshot");
          pM32F = (tM32F) GetProcAddress(hToolhelp, "Module32First");
          pM32N = (tM32N) GetProcAddress(hToolhelp, "Module32Next");
          if ( (pCT32S != NULL) && (pM32F != NULL) && (pM32N != NULL) )
            break; // found the functions!
          FreeLibrary(hToolhelp);
          hToolhelp = NULL;
        }

        if (hToolhelp == NULL)
          return FALSE;

        hSnap = pCT32S( TH32CS_SNAPMODULE, pid );
        if (hSnap == (HANDLE) -1)
          return FALSE;

        keepGoing = !!pM32F( hSnap, &me );
        int cnt = 0;
        while (keepGoing)
        {
          this->LoadModule(hProcess, me.szExePath, me.szModule, (DWORD64) me.modBaseAddr, me.modBaseSize);
          cnt++;
          keepGoing = !!pM32N( hSnap, &me );
        }
        CloseHandle(hSnap);
        FreeLibrary(hToolhelp);
        if (cnt <= 0)
          return FALSE;
        return TRUE;
      }  // GetModuleListTH32

      // **************************************** PSAPI ************************
      typedef struct _MODULEINFO {
          LPVOID lpBaseOfDll;
          DWORD SizeOfImage;
          LPVOID EntryPoint;
      } MODULEINFO, *LPMODULEINFO;

      BOOL GetModuleListPSAPI(HANDLE hProcess)
      {
        // EnumProcessModules()
        typedef BOOL (__stdcall *tEPM)(HANDLE hProcess, HMODULE *lphModule, DWORD cb, LPDWORD lpcbNeeded );
        // GetModuleFileNameEx()
        typedef DWORD (__stdcall *tGMFNE)(HANDLE hProcess, HMODULE hModule, LPSTR lpFilename, DWORD nSize );
        // GetModuleBaseName()
        typedef DWORD (__stdcall *tGMBN)(HANDLE hProcess, HMODULE hModule, LPSTR lpFilename, DWORD nSize );
        // GetModuleInformation()
        typedef BOOL (__stdcall *tGMI)(HANDLE hProcess, HMODULE hModule, LPMODULEINFO pmi, DWORD nSize );

        HINSTANCE hPsapi;
        tEPM pEPM;
        tGMFNE pGMFNE;
        tGMBN pGMBN;
        tGMI pGMI;

        DWORD i;
        //ModuleEntry e;
        DWORD cbNeeded;
        MODULEINFO mi;
        HMODULE *hMods = 0;
        char *tt = NULL;
        char *tt2 = NULL;
        const SIZE_T TTBUFLEN = 8096;
        int cnt = 0;

        hPsapi = LoadLibrary( _T("psapi.dll") );
        if (hPsapi == NULL)
          return FALSE;

        pEPM = (tEPM) GetProcAddress( hPsapi, "EnumProcessModules" );
        pGMFNE = (tGMFNE) GetProcAddress( hPsapi, "GetModuleFileNameExA" );
        pGMBN = (tGMFNE) GetProcAddress( hPsapi, "GetModuleBaseNameA" );
        pGMI = (tGMI) GetProcAddress( hPsapi, "GetModuleInformation" );
        if ( (pEPM == NULL) || (pGMFNE == NULL) || (pGMBN == NULL) || (pGMI == NULL) )
        {
          // we couldn´t find all functions
          FreeLibrary(hPsapi);
          return FALSE;
        }

        hMods = (HMODULE*) malloc(sizeof(HMODULE) * (TTBUFLEN / sizeof HMODULE));
        tt = (char*) malloc(sizeof(char) * TTBUFLEN);
        tt2 = (char*) malloc(sizeof(char) * TTBUFLEN);
        if ( (hMods == NULL) || (tt == NULL) || (tt2 == NULL) )
          goto cleanup;

        if ( ! pEPM( hProcess, hMods, TTBUFLEN, &cbNeeded ) )
        {
          //_ftprintf(fLogFile, _T("%lu: EPM failed, GetLastError = %lu\n"), g_dwShowCount, gle );
          goto cleanup;
        }

        if ( cbNeeded > TTBUFLEN )
        {
          //_ftprintf(fLogFile, _T("%lu: More than %lu module handles. Huh?\n"), g_dwShowCount, lenof( hMods ) );
          goto cleanup;
        }

        for ( i = 0; i < cbNeeded / sizeof hMods[0]; i++ )
        {
          // base address, size
          pGMI(hProcess, hMods[i], &mi, sizeof mi );
          // image file name
          tt[0] = 0;
          pGMFNE(hProcess, hMods[i], tt, TTBUFLEN );
          // module name
          tt2[0] = 0;
          pGMBN(hProcess, hMods[i], tt2, TTBUFLEN );

          DWORD dwRes = this->LoadModule(hProcess, tt, tt2, (DWORD64) mi.lpBaseOfDll, mi.SizeOfImage);
          if (dwRes != ERROR_SUCCESS)
            this->m_parent->OnDbgHelpErr("LoadModule", dwRes, 0);
          cnt++;
        }

      cleanup:
        if (hPsapi != NULL) FreeLibrary(hPsapi);
        if (tt2 != NULL) free(tt2);
        if (tt != NULL) free(tt);
        if (hMods != NULL) free(hMods);

        return cnt != 0;
      }  // GetModuleListPSAPI

      DWORD LoadModule(HANDLE hProcess, LPCSTR img, LPCSTR mod, DWORD64 baseAddr, DWORD size)
      {
        CHAR *szImg = _strdup(img);
        CHAR *szMod = _strdup(mod);
        DWORD result = ERROR_SUCCESS;
        if ( (szImg == NULL) || (szMod == NULL) )
          result = ERROR_NOT_ENOUGH_MEMORY;
        else
        {
          if (pSLM(hProcess, 0, szImg, szMod, baseAddr, size) == 0)
            result = GetLastError();
        }
        ULONGLONG fileVersion = 0;
        if ( (m_parent != NULL) && (szImg != NULL) )
        {
          // try to retrive the file-version:
          if ( (this->m_parent->m_options & StackWalker::RetrieveFileVersion) != 0)
          {
            VS_FIXEDFILEINFO *fInfo = NULL;
            DWORD dwHandle;
            DWORD dwSize = GetFileVersionInfoSizeA(szImg, &dwHandle);
            if (dwSize > 0)
            {
              LPVOID vData = malloc(dwSize);
              if (vData != NULL)
              {
                if (GetFileVersionInfoA(szImg, dwHandle, dwSize, vData) != 0)
                {
                  UINT len;
                  TCHAR szSubBlock[] = _T("\\");
                  if (VerQueryValue(vData, szSubBlock, (LPVOID*) &fInfo, &len) == 0)
                    fInfo = NULL;
                  else
                  {
                    fileVersion = ((ULONGLONG)fInfo->dwFileVersionLS) + ((ULONGLONG)fInfo->dwFileVersionMS << 32);
                  }
                }
                free(vData);
              }
            }
          }

          // Retrive some additional-infos about the module
          IMAGEHLP_MODULE64_V2 Module;
          const char *szSymType = "-unknown-";
          if (this->GetModuleInfo(hProcess, baseAddr, &Module) != FALSE)
          {
            switch(Module.SymType)
            {
              case SymNone:
                szSymType = "-nosymbols-";
                break;
              case SymCoff:
                szSymType = "COFF";
                break;
              case SymCv:
                szSymType = "CV";
                break;
              case SymPdb:
                szSymType = "PDB";
                break;
              case SymExport:
                szSymType = "-exported-";
                break;
              case SymDeferred:
                szSymType = "-deferred-";
                break;
              case SymSym:
                szSymType = "SYM";
                break;
              case 8: //SymVirtual:
                szSymType = "Virtual";
                break;
              case 9: // SymDia:
                szSymType = "DIA";
                break;
            }
          }
          this->m_parent->OnLoadModule(img, mod, baseAddr, size, result, szSymType, Module.LoadedImageName, fileVersion);
        }
        if (szImg != NULL) free(szImg);
        if (szMod != NULL) free(szMod);
        return result;
      }
    public:
      BOOL LoadModules(HANDLE hProcess, DWORD dwProcessId)
      {
        // first try toolhelp32
        if (GetModuleListTH32(hProcess, dwProcessId))
          return true;
        // then try psapi
        return GetModuleListPSAPI(hProcess);
      }


      BOOL GetModuleInfo(HANDLE hProcess, DWORD64 baseAddr, IMAGEHLP_MODULE64_V2 *pModuleInfo)
      {
        if(this->pSGMI == NULL)
        {
          SetLastError(ERROR_DLL_INIT_FAILED);
          return FALSE;
        }
        // First try to use the larger ModuleInfo-Structure
    //    memset(pModuleInfo, 0, sizeof(IMAGEHLP_MODULE64_V3));
    //    pModuleInfo->SizeOfStruct = sizeof(IMAGEHLP_MODULE64_V3);
    //    if (this->pSGMI_V3 != NULL)
    //    {
    //      if (this->pSGMI_V3(hProcess, baseAddr, pModuleInfo) != FALSE)
    //        return TRUE;
    //      // check if the parameter was wrong (size is bad...)
    //      if (GetLastError() != ERROR_INVALID_PARAMETER)
    //        return FALSE;
    //    }
        // could not retrive the bigger structure, try with the smaller one (as defined in VC7.1)...
        pModuleInfo->SizeOfStruct = sizeof(IMAGEHLP_MODULE64_V2);
        void *pData = malloc(4096); // reserve enough memory, so the bug in v6.3.5.1 does not lead to memory-overwrites...
        if (pData == NULL)
        {
          SetLastError(ERROR_NOT_ENOUGH_MEMORY);
          return FALSE;
        }
        memcpy(pData, pModuleInfo, sizeof(IMAGEHLP_MODULE64_V2));
        if (this->pSGMI(hProcess, baseAddr, (IMAGEHLP_MODULE64_V2*) pData) != FALSE)
        {
          // only copy as much memory as is reserved...
          memcpy(pModuleInfo, pData, sizeof(IMAGEHLP_MODULE64_V2));
          pModuleInfo->SizeOfStruct = sizeof(IMAGEHLP_MODULE64_V2);
          free(pData);
          return TRUE;
        }
        free(pData);
        SetLastError(ERROR_DLL_INIT_FAILED);
        return FALSE;
      }
    };
};

}

}




namespace XTSystem
{
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
public:
    String():data() {}
    String(const String &s):data(s.data) {}
    String(const Char *s):data(s) {}
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

private:
        static inline std::wstring &ltrim(std::wstring &s, const Char*chars = _T(" \t\n\r\v"))
        {
            std::wstring::size_type loc = s.find_first_not_of(chars);
            if (loc != std::wstring::npos) s = s.substr(loc); else { s = _T(""); }
            return s;
        }

        static inline std::wstring &rtrim(std::wstring &s, const Char*chars = _T(" \t\n\r\v"))
        {
            std::wstring::size_type loc = s.find_last_not_of(chars);
            if (loc != std::wstring::npos) s = s.substr(0, loc + 1); else { s = _T(""); }
            return s;
        }

        static inline std::wstring &trim(std::wstring &s) 
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
                transform(s.data.begin(), s.data.end(),s.data.begin(),tolower);
                return s;
            }
    public: String ToUpper() const
            {
                String s(*this);
                transform(s.data.begin(), s.data.end(),s.data.begin(),toupper);
                return s;
            }

            String Insert(int startIndex, const String &value) const
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
                return PadLeft(totalWidth,_T(' '));
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
                return PadRight(totalWidth,_T(' '));
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
                std::replace( s.data.begin(), s.data.end(), oldChar, newChar);
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

            static String Convert_charsToString(const char*p)
            {
                if (NULL == p) return L"";
                const WCHAR *pwcsName;
                int nChars = MultiByteToWideChar(CP_ACP, 0, p, -1, NULL, 0);
                pwcsName = new WCHAR[nChars];
                MultiByteToWideChar(CP_ACP, 0, p, -1, (LPWSTR)pwcsName, nChars);
                String s(pwcsName);
                delete [] pwcsName;
                return s;
            }

			std::string Tostdstring() const
			{
				std::string strValue;
				strValue.assign(data.begin(), data.end());  // convert wstring to string
				return strValue;
			}

    friend std::basic_ostringstream<Char>&operator<<(std::basic_ostringstream<Char> &os, const String &d)
    {
        os << d.data;
        return os;
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

private:
    std::wstring data;
};


}



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



#define CALL_ONCE_BLOCK static int _dummyNumberOfCalls = 1; if (1==_dummyNumberOfCalls++) 

#define ENUM_IMPL(NAME, T1) \
  public:\
    static enumType Parse(const XTSystem::String &strName) { for (int i=0;i<numItems;i++) if (String::Compare(strName,GetNames()[i])==0) return GetValues()[i]; throw Exception(_T("Bad parsing of enum")); } \
    String ToString() const {  for (int i=0;i<numItems;i++) if (GetValues()[i] == value) return GetNames()[i]; return _T(""); } \
    NAME(): value((enumType)T1) {} \
    NAME & operator=(enumType v) { value = v; return *this;} \
    NAME(enumType v) { value = v; } \
    friend bool operator!= (const NAME& lhs, const NAME& rhs) { return lhs.value != rhs.value; }\
    friend bool operator!= (const NAME& lhs, const enumType& rhs) { return lhs.value != rhs; }\
    friend bool operator!= (const enumType& lhs, const NAME& rhs) { return lhs != rhs.value; }\
    friend bool operator== (const NAME& lhs, const NAME& rhs) { return lhs.value == rhs.value; }\
    friend bool operator== (const NAME& lhs, const enumType& rhs) { return lhs.value == rhs; }\
    friend bool operator== (const enumType& lhs, const NAME& rhs) { return lhs == rhs.value; }\
    operator int() { return (int)value; } \
    int ToInt32() const {  return (int)value; } \
    int ToLong() const {  return (long)value; } \
  private: \
    enumType value; \


#define FLAGS_ENUM_IMPL(NAME, T1) \
  public:\
    static enumType Parse(const XTSystem::String &strName) { for (int i=0;i<numItems;i++) if (String::Compare(strName,GetNames()[i])==0) return GetValues()[i]; throw Exception(_T("Bad parsing of enum")); } \
    String ToString() const {  String s; int k=0; for (int i=0;i<numItems;i++) { int a = GetValues()[i]; int b = value; if ((a == b) || ((a != 0) && ((a & b)==a))) { if (k>0) s+=_T("|"); s+= GetNames()[i]; ++k;} }  return s; } \
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
    static const int numItems = 1; \
    typedef enum { T1 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1)}; v = std::vector<String>(args, args + numItems); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1}; v = std::vector<enumType>(args, args + numItems); } return v; }\
    ENUM_IMPL(NAME,T1) \
}; \


#define enum_2(NAME,T1,T2) \
class NAME \
{ \
  public: \
    static const int numItems = 2; \
    typedef enum { T1,T2 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2)}; v = std::vector<String>(args, args + numItems); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2}; v = std::vector<enumType>(args, args + numItems); } return v; }\
    ENUM_IMPL(NAME,T1) \
}; \


#define enum_3(NAME,T1,T2,T3) \
class NAME \
{ \
  public: \
    static const int numItems = 3; \
    typedef enum { T1,T2,T3 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3)}; v = std::vector<String>(args, args + numItems); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3}; v = std::vector<enumType>(args, args + numItems); } return v; }\
    ENUM_IMPL(NAME,T1) \
}; \


#define enum_4(NAME,T1,T2,T3,T4) \
class NAME \
{ \
  public: \
    static const int numItems = 4; \
    typedef enum { T1,T2,T3,T4 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3),_T(#T4)}; v = std::vector<String>(args, args + numItems); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3,T4}; v = std::vector<enumType>(args, args + numItems); } return v; }\
    ENUM_IMPL(NAME,T1) \
}; \


#define enum_5(NAME,T1,T2,T3,T4,T5) \
class NAME \
{ \
  public: \
    static const int numItems = 5; \
    typedef enum { T1,T2,T3,T4,T5 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3),_T(#T4),_T(#T5)}; v = std::vector<String>(args, args + numItems); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3,T4,T5}; v = std::vector<enumType>(args, args + numItems); } return v; }\
    ENUM_IMPL(NAME,T1) \
}; \


#define enum_6(NAME,T1,T2,T3,T4,T5,T6) \
class NAME \
{ \
  public: \
    static const int numItems = 6; \
    typedef enum { T1,T2,T3,T4,T5,T6 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3),_T(#T4),_T(#T5),_T(#T6)}; v = std::vector<String>(args, args + numItems); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3,T4,T5,T6}; v = std::vector<enumType>(args, args + numItems); } return v; }\
    ENUM_IMPL(NAME,T1) \
}; \


#define enum_7(NAME,T1,T2,T3,T4,T5,T6,T7) \
class NAME \
{ \
  public: \
    static const int numItems = 7; \
    typedef enum { T1,T2,T3,T4,T5,T6,T7 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3),_T(#T4),_T(#T5),_T(#T6),_T(#T7)}; v = std::vector<String>(args, args + numItems); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3,T4,T5,T6,T7}; v = std::vector<enumType>(args, args + numItems); } return v; }\
    ENUM_IMPL(NAME,T1) \
}; \


#define enum_8(NAME,T1,T2,T3,T4,T5,T6,T7,T8) \
class NAME \
{ \
  public: \
    static const int numItems = 8; \
    typedef enum { T1,T2,T3,T4,T5,T6,T7,T8 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3),_T(#T4),_T(#T5),_T(#T6),_T(#T7),_T(#T8)}; v = std::vector<String>(args, args + numItems); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3,T4,T5,T6,T7,T8}; v = std::vector<enumType>(args, args + numItems); } return v; }\
    ENUM_IMPL(NAME,T1) \
}; \


#define enum_9(NAME,T1,T2,T3,T4,T5,T6,T7,T8,T9) \
class NAME \
{ \
  public: \
    static const int numItems = 9; \
    typedef enum { T1,T2,T3,T4,T5,T6,T7,T8,T9 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3),_T(#T4),_T(#T5),_T(#T6),_T(#T7),_T(#T8),_T(#T9)}; v = std::vector<String>(args, args + numItems); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3,T4,T5,T6,T7,T8,T9}; v = std::vector<enumType>(args, args + numItems); } return v; }\
    ENUM_IMPL(NAME,T1) \
}; \


#define enum_10(NAME,T1,T2,T3,T4,T5,T6,T7,T8,T9,T10) \
class NAME \
{ \
  public: \
    static const int numItems = 10; \
    typedef enum { T1,T2,T3,T4,T5,T6,T7,T8,T9,T10 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3),_T(#T4),_T(#T5),_T(#T6),_T(#T7),_T(#T8),_T(#T9),_T(#T10)}; v = std::vector<String>(args, args + numItems); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3,T4,T5,T6,T7,T8,T9,T10}; v = std::vector<enumType>(args, args + numItems); } return v; }\
    ENUM_IMPL(NAME,T1) \
}; \


#define enum_1_manual(NAME,T1,V1) \
class NAME \
{ \
  public: \
    static const int numItems = 1; \
    typedef enum { T1=V1 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1)}; v = std::vector<String>(args, args + numItems); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1}; v = std::vector<enumType>(args, args + numItems); } return v; }\
    ENUM_IMPL(NAME,T1) \
}; \


#define enum_2_manual(NAME,T1,V1,T2,V2) \
class NAME \
{ \
  public: \
    static const int numItems = 2; \
    typedef enum { T1=V1,T2=V2 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2)}; v = std::vector<String>(args, args + numItems); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2}; v = std::vector<enumType>(args, args + numItems); } return v; }\
    ENUM_IMPL(NAME,T1) \
}; \


#define enum_3_manual(NAME,T1,V1,T2,V2,T3,V3) \
class NAME \
{ \
  public: \
    static const int numItems = 3; \
    typedef enum { T1=V1,T2=V2,T3=V3 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3)}; v = std::vector<String>(args, args + numItems); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3}; v = std::vector<enumType>(args, args + numItems); } return v; }\
    ENUM_IMPL(NAME,T1) \
}; \


#define enum_4_manual(NAME,T1,V1,T2,V2,T3,V3,T4,V4) \
class NAME \
{ \
  public: \
    static const int numItems = 4; \
    typedef enum { T1=V1,T2=V2,T3=V3,T4=V4 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3),_T(#T4)}; v = std::vector<String>(args, args + numItems); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3,T4}; v = std::vector<enumType>(args, args + numItems); } return v; }\
    ENUM_IMPL(NAME,T1) \
}; \


#define enum_5_manual(NAME,T1,V1,T2,V2,T3,V3,T4,V4,T5,V5) \
class NAME \
{ \
  public: \
    static const int numItems = 5; \
    typedef enum { T1=V1,T2=V2,T3=V3,T4=V4,T5=V5 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3),_T(#T4),_T(#T5)}; v = std::vector<String>(args, args + numItems); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3,T4,T5}; v = std::vector<enumType>(args, args + numItems); } return v; }\
    ENUM_IMPL(NAME,T1) \
}; \


#define enum_6_manual(NAME,T1,V1,T2,V2,T3,V3,T4,V4,T5,V5,T6,V6) \
class NAME \
{ \
  public: \
    static const int numItems = 6; \
    typedef enum { T1=V1,T2=V2,T3=V3,T4=V4,T5=V5,T6=V6 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3),_T(#T4),_T(#T5),_T(#T6)}; v = std::vector<String>(args, args + numItems); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3,T4,T5,T6}; v = std::vector<enumType>(args, args + numItems); } return v; }\
    ENUM_IMPL(NAME,T1) \
}; \


#define enum_7_manual(NAME,T1,V1,T2,V2,T3,V3,T4,V4,T5,V5,T6,V6,T7,V7) \
class NAME \
{ \
  public: \
    static const int numItems = 7; \
    typedef enum { T1=V1,T2=V2,T3=V3,T4=V4,T5=V5,T6=V6,T7=V7 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3),_T(#T4),_T(#T5),_T(#T6),_T(#T7)}; v = std::vector<String>(args, args + numItems); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3,T4,T5,T6,T7}; v = std::vector<enumType>(args, args + numItems); } return v; }\
    ENUM_IMPL(NAME,T1) \
}; \


#define enum_8_manual(NAME,T1,V1,T2,V2,T3,V3,T4,V4,T5,V5,T6,V6,T7,V7,T8,V8) \
class NAME \
{ \
  public: \
    static const int numItems = 8; \
    typedef enum { T1=V1,T2=V2,T3=V3,T4=V4,T5=V5,T6=V6,T7=V7,T8=V8 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3),_T(#T4),_T(#T5),_T(#T6),_T(#T7),_T(#T8)}; v = std::vector<String>(args, args + numItems); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3,T4,T5,T6,T7,T8}; v = std::vector<enumType>(args, args + numItems); } return v; }\
    ENUM_IMPL(NAME,T1) \
}; \


#define enum_9_manual(NAME,T1,V1,T2,V2,T3,V3,T4,V4,T5,V5,T6,V6,T7,V7,T8,V8,T9,V9) \
class NAME \
{ \
  public: \
    static const int numItems = 9; \
    typedef enum { T1=V1,T2=V2,T3=V3,T4=V4,T5=V5,T6=V6,T7=V7,T8=V8,T9=V9 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3),_T(#T4),_T(#T5),_T(#T6),_T(#T7),_T(#T8),_T(#T9)}; v = std::vector<String>(args, args + numItems); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3,T4,T5,T6,T7,T8,T9}; v = std::vector<enumType>(args, args + numItems); } return v; }\
    ENUM_IMPL(NAME,T1) \
}; \


#define enum_10_manual(NAME,T1,V1,T2,V2,T3,V3,T4,V4,T5,V5,T6,V6,T7,V7,T8,V8,T9,V9,T10,V10) \
class NAME \
{ \
  public: \
    static const int numItems = 10; \
    typedef enum { T1=V1,T2=V2,T3=V3,T4=V4,T5=V5,T6=V6,T7=V7,T8=V8,T9=V9,T10=V10 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3),_T(#T4),_T(#T5),_T(#T6),_T(#T7),_T(#T8),_T(#T9),_T(#T10)}; v = std::vector<String>(args, args + numItems); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3,T4,T5,T6,T7,T8,T9,T10}; v = std::vector<enumType>(args, args + numItems); } return v; }\
    ENUM_IMPL(NAME,T1) \
}; \


#define flags_1(NAME,T1) \
class NAME \
{ \
  public: \
    static const int numItems = 1; \
    typedef enum { T1 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1)}; v = std::vector<String>(args, args + numItems); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1}; v = std::vector<enumType>(args, args + numItems); } return v; }\
    FLAGS_ENUM_IMPL(NAME,T1) \
}; \


#define flags_2(NAME,T1,T2) \
class NAME \
{ \
  public: \
    static const int numItems = 2; \
    typedef enum { T1,T2 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2)}; v = std::vector<String>(args, args + numItems); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2}; v = std::vector<enumType>(args, args + numItems); } return v; }\
    FLAGS_ENUM_IMPL(NAME,T1) \
}; \


#define flags_3(NAME,T1,T2,T3) \
class NAME \
{ \
  public: \
    static const int numItems = 3; \
    typedef enum { T1,T2,T3 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3)}; v = std::vector<String>(args, args + numItems); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3}; v = std::vector<enumType>(args, args + numItems); } return v; }\
    FLAGS_ENUM_IMPL(NAME,T1) \
}; \


#define flags_4(NAME,T1,T2,T3,T4) \
class NAME \
{ \
  public: \
    static const int numItems = 4; \
    typedef enum { T1,T2,T3,T4 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3),_T(#T4)}; v = std::vector<String>(args, args + numItems); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3,T4}; v = std::vector<enumType>(args, args + numItems); } return v; }\
    FLAGS_ENUM_IMPL(NAME,T1) \
}; \


#define flags_5(NAME,T1,T2,T3,T4,T5) \
class NAME \
{ \
  public: \
    static const int numItems = 5; \
    typedef enum { T1,T2,T3,T4,T5 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3),_T(#T4),_T(#T5)}; v = std::vector<String>(args, args + numItems); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3,T4,T5}; v = std::vector<enumType>(args, args + numItems); } return v; }\
    FLAGS_ENUM_IMPL(NAME,T1) \
}; \


#define flags_6(NAME,T1,T2,T3,T4,T5,T6) \
class NAME \
{ \
  public: \
    static const int numItems = 6; \
    typedef enum { T1,T2,T3,T4,T5,T6 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3),_T(#T4),_T(#T5),_T(#T6)}; v = std::vector<String>(args, args + numItems); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3,T4,T5,T6}; v = std::vector<enumType>(args, args + numItems); } return v; }\
    FLAGS_ENUM_IMPL(NAME,T1) \
}; \


#define flags_7(NAME,T1,T2,T3,T4,T5,T6,T7) \
class NAME \
{ \
  public: \
    static const int numItems = 7; \
    typedef enum { T1,T2,T3,T4,T5,T6,T7 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3),_T(#T4),_T(#T5),_T(#T6),_T(#T7)}; v = std::vector<String>(args, args + numItems); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3,T4,T5,T6,T7}; v = std::vector<enumType>(args, args + numItems); } return v; }\
    FLAGS_ENUM_IMPL(NAME,T1) \
}; \


#define flags_8(NAME,T1,T2,T3,T4,T5,T6,T7,T8) \
class NAME \
{ \
  public: \
    static const int numItems = 8; \
    typedef enum { T1,T2,T3,T4,T5,T6,T7,T8 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3),_T(#T4),_T(#T5),_T(#T6),_T(#T7),_T(#T8)}; v = std::vector<String>(args, args + numItems); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3,T4,T5,T6,T7,T8}; v = std::vector<enumType>(args, args + numItems); } return v; }\
    FLAGS_ENUM_IMPL(NAME,T1) \
}; \


#define flags_9(NAME,T1,T2,T3,T4,T5,T6,T7,T8,T9) \
class NAME \
{ \
  public: \
    static const int numItems = 9; \
    typedef enum { T1,T2,T3,T4,T5,T6,T7,T8,T9 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3),_T(#T4),_T(#T5),_T(#T6),_T(#T7),_T(#T8),_T(#T9)}; v = std::vector<String>(args, args + numItems); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3,T4,T5,T6,T7,T8,T9}; v = std::vector<enumType>(args, args + numItems); } return v; }\
    FLAGS_ENUM_IMPL(NAME,T1) \
}; \


#define flags_10(NAME,T1,T2,T3,T4,T5,T6,T7,T8,T9,T10) \
class NAME \
{ \
  public: \
    static const int numItems = 10; \
    typedef enum { T1,T2,T3,T4,T5,T6,T7,T8,T9,T10 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3),_T(#T4),_T(#T5),_T(#T6),_T(#T7),_T(#T8),_T(#T9),_T(#T10)}; v = std::vector<String>(args, args + numItems); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3,T4,T5,T6,T7,T8,T9,T10}; v = std::vector<enumType>(args, args + numItems); } return v; }\
    FLAGS_ENUM_IMPL(NAME,T1) \
}; \


#define flags_1_manual(NAME,T1,V1) \
class NAME \
{ \
  public: \
    static const int numItems = 1; \
    typedef enum { T1=V1 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1)}; v = std::vector<String>(args, args + numItems); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1}; v = std::vector<enumType>(args, args + numItems); } return v; }\
    FLAGS_ENUM_IMPL(NAME,T1) \
}; \


#define flags_2_manual(NAME,T1,V1,T2,V2) \
class NAME \
{ \
  public: \
    static const int numItems = 2; \
    typedef enum { T1=V1,T2=V2 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2)}; v = std::vector<String>(args, args + numItems); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2}; v = std::vector<enumType>(args, args + numItems); } return v; }\
    FLAGS_ENUM_IMPL(NAME,T1) \
}; \


#define flags_3_manual(NAME,T1,V1,T2,V2,T3,V3) \
class NAME \
{ \
  public: \
    static const int numItems = 3; \
    typedef enum { T1=V1,T2=V2,T3=V3 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3)}; v = std::vector<String>(args, args + numItems); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3}; v = std::vector<enumType>(args, args + numItems); } return v; }\
    FLAGS_ENUM_IMPL(NAME,T1) \
}; \


#define flags_4_manual(NAME,T1,V1,T2,V2,T3,V3,T4,V4) \
class NAME \
{ \
  public: \
    static const int numItems = 4; \
    typedef enum { T1=V1,T2=V2,T3=V3,T4=V4 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3),_T(#T4)}; v = std::vector<String>(args, args + numItems); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3,T4}; v = std::vector<enumType>(args, args + numItems); } return v; }\
    FLAGS_ENUM_IMPL(NAME,T1) \
}; \


#define flags_5_manual(NAME,T1,V1,T2,V2,T3,V3,T4,V4,T5,V5) \
class NAME \
{ \
  public: \
    static const int numItems = 5; \
    typedef enum { T1=V1,T2=V2,T3=V3,T4=V4,T5=V5 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3),_T(#T4),_T(#T5)}; v = std::vector<String>(args, args + numItems); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3,T4,T5}; v = std::vector<enumType>(args, args + numItems); } return v; }\
    FLAGS_ENUM_IMPL(NAME,T1) \
}; \


#define flags_6_manual(NAME,T1,V1,T2,V2,T3,V3,T4,V4,T5,V5,T6,V6) \
class NAME \
{ \
  public: \
    static const int numItems = 6; \
    typedef enum { T1=V1,T2=V2,T3=V3,T4=V4,T5=V5,T6=V6 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3),_T(#T4),_T(#T5),_T(#T6)}; v = std::vector<String>(args, args + numItems); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3,T4,T5,T6}; v = std::vector<enumType>(args, args + numItems); } return v; }\
    FLAGS_ENUM_IMPL(NAME,T1) \
}; \


#define flags_7_manual(NAME,T1,V1,T2,V2,T3,V3,T4,V4,T5,V5,T6,V6,T7,V7) \
class NAME \
{ \
  public: \
    static const int numItems = 7; \
    typedef enum { T1=V1,T2=V2,T3=V3,T4=V4,T5=V5,T6=V6,T7=V7 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3),_T(#T4),_T(#T5),_T(#T6),_T(#T7)}; v = std::vector<String>(args, args + numItems); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3,T4,T5,T6,T7}; v = std::vector<enumType>(args, args + numItems); } return v; }\
    FLAGS_ENUM_IMPL(NAME,T1) \
}; \


#define flags_8_manual(NAME,T1,V1,T2,V2,T3,V3,T4,V4,T5,V5,T6,V6,T7,V7,T8,V8) \
class NAME \
{ \
  public: \
    static const int numItems = 8; \
    typedef enum { T1=V1,T2=V2,T3=V3,T4=V4,T5=V5,T6=V6,T7=V7,T8=V8 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3),_T(#T4),_T(#T5),_T(#T6),_T(#T7),_T(#T8)}; v = std::vector<String>(args, args + numItems); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3,T4,T5,T6,T7,T8}; v = std::vector<enumType>(args, args + numItems); } return v; }\
    FLAGS_ENUM_IMPL(NAME,T1) \
}; \


#define flags_9_manual(NAME,T1,V1,T2,V2,T3,V3,T4,V4,T5,V5,T6,V6,T7,V7,T8,V8,T9,V9) \
class NAME \
{ \
  public: \
    static const int numItems = 9; \
    typedef enum { T1=V1,T2=V2,T3=V3,T4=V4,T5=V5,T6=V6,T7=V7,T8=V8,T9=V9 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3),_T(#T4),_T(#T5),_T(#T6),_T(#T7),_T(#T8),_T(#T9)}; v = std::vector<String>(args, args + numItems); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3,T4,T5,T6,T7,T8,T9}; v = std::vector<enumType>(args, args + numItems); } return v; }\
    FLAGS_ENUM_IMPL(NAME,T1) \
}; \


#define flags_10_manual(NAME,T1,V1,T2,V2,T3,V3,T4,V4,T5,V5,T6,V6,T7,V7,T8,V8,T9,V9,T10,V10) \
class NAME \
{ \
  public: \
    static const int numItems = 10; \
    typedef enum { T1=V1,T2=V2,T3=V3,T4=V4,T5=V5,T6=V6,T7=V7,T8=V8,T9=V9,T10=V10 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3),_T(#T4),_T(#T5),_T(#T6),_T(#T7),_T(#T8),_T(#T9),_T(#T10)}; v = std::vector<String>(args, args + numItems); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {T1,T2,T3,T4,T5,T6,T7,T8,T9,T10}; v = std::vector<enumType>(args, args + numItems); } return v; }\
    FLAGS_ENUM_IMPL(NAME,T1) \
}; \



namespace XTSystem
{

    enum_4(RangeType, ClosedClosed,OpenClosed,ClosedOpen,OpenOpen)

struct Math
{
    static const double Pi() { return 3.14159265358979323846;}
    static const double E() { return 2.718281828459045;}
    static double Rad2Deg(double x) { return 180.0 * x / Pi(); }
    static double Deg2Rad(double x) { return Pi() * x / 180.0; }
    static double RadiansToDegrees(double x) { return Rad2Deg(x); }
    static double DegreesToRadians(double x) { return Deg2Rad(x); }
    static double Sqr(double x) { return x * x; }
    static double RandomUniform01() { return ((double) rand ()) / (((double) RAND_MAX) +1.0); }
    static double RandomUniformAB(double a, double b) { return (b - a) * RandomUniform01() + a; }
    static double NormalizeAngleToRange0_360(double xDEG)
    {
        double res = fmod(xDEG, 360.0);
        if (res<0) res += 360.0;
        return res;
    }

    static double NormalizeAngleToRange_180plus180(double xDEG)
    {
        double d = NormalizeAngleToRange0_360(xDEG);
        if (d>180) d -=360;
        return d;
    }
    static double NormalizeAngleToRange0_2Pi(double xRAD)
    {
        return Deg2Rad(NormalizeAngleToRange0_360(Rad2Deg(xRAD)));
    }
    static double NormalizeAngleToRange_PiPi(double xRAD)
    {
        return Deg2Rad(NormalizeAngleToRange_180plus180(Rad2Deg(xRAD)));
    }
    static double AngleDiffIn_180plus180(double xDEG, double yDEG)
    {
        double d = NormalizeAngleToRange0_360(xDEG - yDEG);
        if (d > 180) d = d - 360;
        return d;
    }
    static double AngleDiffIn_PIplusPI(double xRAD, double yRAD)
    {
        return Deg2Rad(AngleDiffIn_180plus180(Rad2Deg(xRAD), Rad2Deg(yRAD)));
    }
    template<class T>static T Abs(T x) { return (x>0)?x:-x; }
    static double Acos(double x) { return acos(x); }
    static double Asin(double x) { return asin(x); }
    static double Atan(double x) { return atan(x); }
    static double Atan2(double y,double x) { return atan2(y,x); }
    static double Ceiling(double x) { return ceil(x); }
    static double Cos(double x) { return cos(x); }
    static double Cosh(double x) { return cosh(x); }
    static double Exp(double x) { return exp(x); }
    static double FMod(double x,double y) { return fmod(x,y); }
    static double Floor(double x) { return floor(x); }
    static double Log(double x) { return log(x); }
    static double Log(double a, double base) { return log(a)/log(base); }
    static double Log10(double x) { return log10(x); }
    template<class T>static T Max(T a, T b) { return (a>b)?a:b; }
    template<class T>static T Min(T a, T b) { return (a<b)?a:b; }
    static double Pow(double x, double y) { return pow(x,y); }
    template<class T>static int Sign(T x) { return (x>0)?1:(x<0)?-1:0; }
    static double Sin(double x) { return sin(x); }
    static double Sinh(double x) { return sinh(x); }
    static double Sqrt(double x) { return sqrt(x); }
    static double Tan(double x) { return tan(x); }
    static double Tanh(double x) { return tanh(x); }
    template<class T>static T Round(T x) { return (T)((long)(x+0.5)); }
    template<class T>static T Truncate(T x) { return (T)((long)(x)); }

    template<class T>
    static bool ValueInRange(T min, T max, T value, RangeType type = RangeType::ClosedClosed)
    {
        if (min > max)
        {
            T a = min;
            min = max;
            max = a;
        }

        if (type == RangeType::ClosedClosed) return value >= min && value <= max;
        if (type == RangeType::OpenClosed) return value > min && value <= max;
        if (type == RangeType::ClosedOpen) return value >= min && value < max;
        if (type == RangeType::OpenOpen) return value > min && value < max;
        return false;
    }

    static bool AngleInRange(double minDEG, double maxDEG, double valueDEG)
    {
        if (minDEG > maxDEG) 
        {
            throw Exception(L"maxDEG must be greater than minDEG");
        }

        valueDEG = Math::NormalizeAngleToRange0_360(valueDEG);
        minDEG = Math::NormalizeAngleToRange0_360(minDEG);
        maxDEG = Math::NormalizeAngleToRange0_360(maxDEG);
        if (minDEG <= maxDEG)
        {
            return ValueInRange(minDEG, maxDEG, valueDEG);
        }
        return !ValueInRange(maxDEG, minDEG, valueDEG,RangeType::OpenOpen);
    }

    static bool AngleInRangeEx(double baseDEG, double tolLeftDEG, double tolRightDEG, double valueDEG)
    {
        if (tolLeftDEG < 0 || tolRightDEG < 0) throw Exception(L"tolLeftDEG and tolRightDEG must be positive or 0");
        if (tolRightDEG + tolLeftDEG >=360) return true;    //we covered whole circle
        return AngleInRange(baseDEG - tolLeftDEG, baseDEG + tolRightDEG, valueDEG);
    }
};

}


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



#define enum_16(NAME,T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14,T15,T16) \
class NAME \
{ \
public: \
    static const int numItems = 10; \
    typedef enum { T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11,T12,T13,T14,T15,T16 } enumType; \
    static const std::vector<String> &GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = {_T(#T1),_T(#T2),_T(#T3),_T(#T4),_T(#T5),_T(#T6),_T(#T7),_T(#T8),_T(#T9),_T(#T10),_T(#T11),_T(#T12),_T(#T13),_T(#T14),_T(#T15),_T(#T16)}; v = std::vector<String>(args, args + numItems); } return v; }\
    static const std::vector<enumType> &GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = { T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16 }; v = std::vector<enumType>(args, args + numItems); } return v; }\
    ENUM_IMPL(NAME,T1) \
}; \

namespace XTSystem
{
    enum_16(ConsoleColor, Black , DarkBlue , DarkGreen , DarkCyan , DarkRed , DarkMagenta , DarkYellow , Gray , DarkGray , Blue , Green , Cyan , Red , Magenta , Yellow , White)
    flags_4_manual(ConsoleModifiers, None, 0, Alt, 1, Shift, 2, Control, 4)

    class ConsoleKey
    {
    public:
        static const int numItems = 144;
        typedef enum
        {
            Backspace = 8,
            Tab = 9,
            Clear = 12,
            Enter = 13,
            Pause = 19,
            Escape = 27,
            Spacebar = 32,
            PageUp = 33,
            PageDown = 34,
            End = 35,
            Home = 36,
            LeftArrow = 37,
            UpArrow = 38,
            RightArrow = 39,
            DownArrow = 40,
            Select = 41,
            Print = 42,
            Execute = 43,
            PrintScreen = 44,
            Insert = 45,
            Delete = 46,
            Help = 47,
            D0 = 48,
            D1 = 49,
            D2 = 50,
            D3 = 51,
            D4 = 52,
            D5 = 53,
            D6 = 54,
            D7 = 55,
            D8 = 56,
            D9 = 57,
            A = 65,
            B = 66,
            C = 67,
            D = 68,
            E = 69,
            F = 70,
            G = 71,
            H = 72,
            I = 73,
            J = 74,
            K = 75,
            L = 76,
            M = 77,
            N = 78,
            O = 79,
            P = 80,
            Q = 81,
            R = 82,
            S = 83,
            T = 84,
            U = 85,
            V = 86,
            W = 87,
            X = 88,
            Y = 89,
            Z = 90,
            LeftWindows = 91,
            RightWindows = 92,
            Applications = 93,
            Sleep = 95,
            NumPad0 = 96,
            NumPad1 = 97,
            NumPad2 = 98,
            NumPad3 = 99,
            NumPad4 = 100,
            NumPad5 = 101,
            NumPad6 = 102,
            NumPad7 = 103,
            NumPad8 = 104,
            NumPad9 = 105,
            Multiply = 106,
            Add = 107,
            Separator = 108,
            Subtract = 109,
            Decimal = 110,
            Divide = 111,
            F1 = 112,
            F2 = 113,
            F3 = 114,
            F4 = 115,
            F5 = 116,
            F6 = 117,
            F7 = 118,
            F8 = 119,
            F9 = 120,
            F10 = 121,
            F11 = 122,
            F12 = 123,
            F13 = 124,
            F14 = 125,
            F15 = 126,
            F16 = 127,
            F17 = 128,
            F18 = 129,
            F19 = 130,
            F20 = 131,
            F21 = 132,
            F22 = 133,
            F23 = 134,
            F24 = 135,
            BrowserBack = 166,
            BrowserForward = 167,
            BrowserRefresh = 168,
            BrowserStop = 169,
            BrowserSearch = 170,
            BrowserFavorites = 171,
            BrowserHome = 172,
            VolumeMute = 173,
            VolumeDown = 174,
            VolumeUp = 175,
            MediaNext = 176,
            MediaPrevious = 177,
            MediaStop = 178,
            MediaPlay = 179,
            LaunchMail = 180,
            LaunchMediaSelect = 181,
            LaunchApp1 = 182,
            LaunchApp2 = 183,
            Oem1 = 186,
            OemPlus = 187,
            OemComma = 188,
            OemMinus = 189,
            OemPeriod = 190,
            Oem2 = 191,
            Oem3 = 192,
            Oem4 = 219,
            Oem5 = 220,
            Oem6 = 221,
            Oem7 = 222,
            Oem8 = 223,
            Oem102 = 226,
            Process = 229,
            Packet = 231,
            Attention = 246,
            CrSel = 247,
            ExSel = 248,
            EraseEndOfFile = 249,
            Play = 250,
            Zoom = 251,
            NoName = 252,
            Pa1 = 253,
            OemClear = 254,
        } enumType;

        static std::vector<String> GetNames() { static std::vector<String> v; CALL_ONCE_BLOCK{ const Char* args[] = { 
            _T("Backspace"), _T("Tab"), _T("Clear"), _T("Enter"), _T("Pause"), _T("Escape"), _T("Spacebar"), _T("PageUp"),
            _T("PageDown"), _T("End"), _T("Home"), _T("LeftArrow"), _T("UpArrow"), _T("RightArrow"), _T("DownArrow"), _T("Select"),
            _T("Print"), _T("Execute"), _T("PrintScreen"), _T("Insert"), _T("Delete"), _T("Help"),
            _T("D0"), _T("D1"), _T("D2"), _T("D3"), _T("D4"), _T("D5"), _T("D6"), _T("D7"), _T("D8"), _T("D9"),
            _T("A"), _T("B"), _T("C"), _T("D"), _T("E"), _T("F"), _T("G"), _T("H"), _T("I"), _T("J"), _T("K"), _T("L"), _T("M"), _T("N"),
            _T("O"), _T("P"), _T("Q"), _T("R"), _T("S"), _T("T"), _T("U"), _T("V"), _T("W"), _T("X"), _T("Y"), _T("Z"),
            _T("LeftWindows"), _T("RightWindows"), _T("Applications"), _T("Sleep"),
            _T("NumPad0"), _T("NumPad1"), _T("NumPad2"), _T("NumPad3"), _T("NumPad4"), _T("NumPad5"), _T("NumPad6"), _T("NumPad7"), _T("NumPad8"), _T("NumPad9"),
            _T("Multiply"), _T("Add"), _T("Separator"), _T("Subtract"), _T("Decimal"), _T("Divide"),
            _T("F1"), _T("F2"), _T("F3"), _T("F4"), _T("F5"), _T("F6"), _T("F7"), _T("F8"), _T("F9"), _T("F10"), _T("F11"), _T("F12"),
            _T("F13"), _T("F14"), _T("F15"), _T("F16"), _T("F17"), _T("F18"), _T("F19"), _T("F20"), _T("F21"), _T("F22"), _T("F23"), _T("F24"),
            _T("BrowserBack"), _T("BrowserForward"), _T("BrowserRefresh"), _T("BrowserStop"), _T("BrowserSearch"), _T("BrowserFavorites"),
            _T("BrowserHome"), _T("VolumeMute"), _T("VolumeDown"), _T("VolumeUp"), _T("MediaNext"), _T("MediaPrevious"), _T("MediaStop"),
            _T("MediaPlay"), _T("LaunchMail"), _T("LaunchMediaSelect"), _T("LaunchApp1"), _T("LaunchApp2"),
            _T("Oem1"), _T("OemPlus"), _T("OemComma"), _T("OemMinus"), _T("OemPeriod"), _T("Oem2"), _T("Oem3"), _T("Oem4"), _T("Oem5"), _T("Oem6"), _T("Oem7"), _T("Oem8"), _T("Oem102"),
            _T("Process"), _T("Packet"), _T("Attention"), _T("CrSel"), _T("ExSel"), _T("EraseEndOfFile"), _T("Play"), _T("Zoom"),
            _T("NoName"), _T("Pa1"), _T("OemClear")
        }; v = std::vector<String>(args, args + numItems); } return v; }

        static std::vector<enumType> GetValues() { static std::vector<enumType> v; CALL_ONCE_BLOCK{ const enumType args[] = {
            Backspace, Tab, Clear, Enter, Pause, Escape, Spacebar, PageUp,
            PageDown, End, Home, LeftArrow, UpArrow, RightArrow, DownArrow, Select,
            Print, Execute, PrintScreen, Insert, Delete, Help,
            D0, D1, D2, D3, D4, D5, D6, D7, D8, D9,
            A, B, C, D, E, F, G, H, I, J, K, L, M, N,
            O, P, Q, R, S, T, U, V, W, X, Y, Z,
            LeftWindows, RightWindows, Applications, Sleep,
            NumPad0, NumPad1, NumPad2, NumPad3, NumPad4, NumPad5, NumPad6, NumPad7, NumPad8, NumPad9,
            Multiply, Add, Separator, Subtract, Decimal, Divide,
            F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
            F13, F14, F15, F16, F17, F18, F19, F20, F21, F22, F23, F24,
            BrowserBack, BrowserForward, BrowserRefresh, BrowserStop, BrowserSearch, BrowserFavorites,
            BrowserHome, VolumeMute, VolumeDown, VolumeUp, MediaNext, MediaPrevious, MediaStop,
            MediaPlay, LaunchMail, LaunchMediaSelect, LaunchApp1, LaunchApp2,
            Oem1, OemPlus, OemComma, OemMinus, OemPeriod, Oem2, Oem3, Oem4, Oem5, Oem6, Oem7, Oem8, Oem102,
            Process, Packet, Attention, CrSel, ExSel, EraseEndOfFile, Play, Zoom,
            NoName, Pa1, OemClear }; v = std::vector<enumType>(args, args + numItems); } return v;
        }
        ENUM_IMPL(ConsoleKey, Backspace)
    };

    struct ConsoleKeyInfo
    {
        public: 
            ConsoleKeyInfo(Char keyChar, ConsoleKey key, bool shift, bool alt, bool control)
            {
                m_keyChar = keyChar;
                m_key = key;
                m_modifs = ((shift) ? ConsoleModifiers::Shift : 0)|
                            ((alt) ? ConsoleModifiers::Alt : 0) |
                            ((control) ? ConsoleModifiers::Control : 0);

            }
    
        //public: static bool operator !=(const ConsoleKeyInfo &a, const ConsoleKeyInfo &b)
        //{
        //}
        //public: static bool operator ==(const ConsoleKeyInfo &a, const ConsoleKeyInfo &b)
        //{
        //}
    
    public: ConsoleKey Key() const { return m_key; }     //property
    public: Char KeyChar() const { return m_keyChar; } //property
    public: ConsoleModifiers Modifiers() const { return m_modifs; } //property
        public: bool Equals(const ConsoleKeyInfo &obj)
        {
            return false;
        }
            //        public: override bool Equals(object value) {}
            //        public: override int GetHashCode();

        private: 
            Char m_keyChar;
            ConsoleKey m_key;
            ConsoleModifiers m_modifs;
    };

    class Console
    {
        private:

        class ConsoleState
        {
            HANDLE hstdin;
            HANDLE hstdout;
            CONSOLE_SCREEN_BUFFER_INFO csbi, old_csbi;

            public:
                ConsoleState()
                {
                    hstdin  = GetStdHandle( STD_INPUT_HANDLE  );
                    hstdout = GetStdHandle( STD_OUTPUT_HANDLE );
                    GetConsoleScreenBufferInfo( hstdout, &old_csbi );
                }
                ~ConsoleState()
                {
                    SetConsoleTextAttribute( hstdout,old_csbi.wAttributes);
                }

            ConsoleColor GetForeGroundColor()
            {
                GetConsoleScreenBufferInfo( hstdout, &csbi );
                return (ConsoleColor::enumType)(csbi.wAttributes & 0x0F);
            }
            ConsoleColor GetBackGroundColor()
            {
                GetConsoleScreenBufferInfo( hstdout, &csbi );
                return (ConsoleColor::enumType)(csbi.wAttributes >> 4);
            }
            void SetBackGroundColor(ConsoleColor color)
            {
                GetConsoleScreenBufferInfo( hstdout, &csbi );
                SetConsoleTextAttribute( hstdout, (csbi.wAttributes & 0x0F) | (((WORD)color) << 4));
            }
            void SetForeGroundColor(ConsoleColor color)
            {
                GetConsoleScreenBufferInfo( hstdout, &csbi );
                SetConsoleTextAttribute( hstdout, (csbi.wAttributes & 0xF0) | ((WORD)color));
            }


        };

        public:

            static ConsoleColor ForeGroundColor()
            {
                return GetConsoleState().GetForeGroundColor();
            }
            static ConsoleColor BackGroundColor()
            {
                return GetConsoleState().GetBackGroundColor();
            }

            static void ForeGroundColor(ConsoleColor color)
            {
                GetConsoleState().SetForeGroundColor(color);
            }

            static void BackGroundColor(ConsoleColor color)
            {
                GetConsoleState().SetBackGroundColor(color);
            }

            static void Write(const Char &c) { _fputtc(c,stdout); }
            static void Write(const String &s) { _fputts(s.c_str(),stdout); }
            TEMP_ARG_1 static void Write(const String &text, TCREF_ARG_1) { Write(String::Format(text,ARG_1)); }
            TEMP_ARG_2 static void Write(const String &text, TCREF_ARG_2) { Write(String::Format(text,ARG_2)); }
            TEMP_ARG_3 static void Write(const String &text, TCREF_ARG_3) { Write(String::Format(text,ARG_3)); }
            TEMP_ARG_4 static void Write(const String &text, TCREF_ARG_4) { Write(String::Format(text,ARG_4)); }
            TEMP_ARG_5 static void Write(const String &text, TCREF_ARG_5) { Write(String::Format(text,ARG_5)); }
            TEMP_ARG_6 static void Write(const String &text, TCREF_ARG_6) { Write(String::Format(text,ARG_6)); }
            TEMP_ARG_7 static void Write(const String &text, TCREF_ARG_7) { Write(String::Format(text,ARG_7)); }
            TEMP_ARG_8 static void Write(const String &text, TCREF_ARG_8) { Write(String::Format(text,ARG_8)); }
            TEMP_ARG_9 static void Write(const String &text, TCREF_ARG_9) { Write(String::Format(text,ARG_9)); }
            TEMP_ARG_10 static void Write(const String &text, TCREF_ARG_10) { Write(String::Format(text,ARG_10)); }

            static void WriteLine(const Char &c) { _fputtc(c,stdout); _fputtc(_T('\n'),stdout);}
            static void WriteLine() { _fputts(_T("\n"),stdout); }
            static void WriteLine(const String &s) { _fputts(s.c_str(),stdout); _fputtc(_T('\n'),stdout);  }
            TEMP_ARG_1 static void WriteLine(const String &text, TCREF_ARG_1) { WriteLine(String::Format(text,ARG_1)); }
            TEMP_ARG_2 static void WriteLine(const String &text, TCREF_ARG_2) { WriteLine(String::Format(text,ARG_2)); }
            TEMP_ARG_3 static void WriteLine(const String &text, TCREF_ARG_3) { WriteLine(String::Format(text,ARG_3)); }
            TEMP_ARG_4 static void WriteLine(const String &text, TCREF_ARG_4) { WriteLine(String::Format(text,ARG_4)); }
            TEMP_ARG_5 static void WriteLine(const String &text, TCREF_ARG_5) { WriteLine(String::Format(text,ARG_5)); }
            TEMP_ARG_6 static void WriteLine(const String &text, TCREF_ARG_6) { WriteLine(String::Format(text,ARG_6)); }
            TEMP_ARG_7 static void WriteLine(const String &text, TCREF_ARG_7) { WriteLine(String::Format(text,ARG_7)); }
            TEMP_ARG_8 static void WriteLine(const String &text, TCREF_ARG_8) { WriteLine(String::Format(text,ARG_8)); }
            TEMP_ARG_9 static void WriteLine(const String &text, TCREF_ARG_9) { WriteLine(String::Format(text,ARG_9)); }
            TEMP_ARG_10 static void WriteLine(const String &text, TCREF_ARG_10) { WriteLine(String::Format(text,ARG_10)); }
            static String ReadLine()
            {
                Char A[2000];
                _fgetts( A, sizeof(A), stdin);
                String line = A;
                line = line.TrimEnd(_T("\r\n"));
                return line;
            }

            //bool KeyAvailable() const   // property
            //{
            //    int i = _kbhit();
            //    return i!=0;
            //}

            //static ConsoleKeyInfo ReadKey()
            //{
            //    return ReadKey(false);
            //}
            //static ConsoleKeyInfo ReadKey(bool intercept)
            //{
            //    int i = _getch();
            //    char c = (char)i;
            //    return ConsoleKeyInfo();
            //}
    private:
        static ConsoleState& GetConsoleState()
        {
            static ConsoleState state;
            return state;
        }
    };
}

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

namespace XTSystem
{
    namespace Text
    {
        class StringBuilder 
        {
        public: 
            StringBuilder() {}
            void AppendLine(const String &line) 
            { 
                data.push_back(line); 
            }
            String ToString() const
            {
                String s = _T("");
				int size = (int)data.size();
                for (int i=0;i<size;i++) s+= data[i];
                return s;
            }
            void Clear()
            {
                data.clear();
            }
        private:
            std::vector<String> data;
        };
    }
}


namespace XTSystem
{
	namespace IO
	{
		class StreamReader
		{
		public:
			StreamReader() : isopened(false)
			{

			}

			StreamReader(const String &path) : isopened(false)
			{
				Open(path);
			}

			void Open(const String &path)
			{
				isopened = false;
#pragma warning(disable: 4996)
				fin = _tfopen(path.c_str(), _T("r"));
#pragma warning(default: 4996)
				if (!fin) throw Exception(_T("Can't open file"));
				isopened = true;
			}

		public: ~StreamReader()
		{
			Close();
		}

		private: bool GetLine(String &line, bool &endOfFile)
		{
			Char buffer[2000];
			int i = 1;
			Char *ptr(NULL);
			line = _T("");
			endOfFile = false;
			for (;;) //while (true) 
			{
				ptr = _fgetts(buffer, 2000, fin);
				if (NULL == ptr)
				{
					if (i == 1)
					{
						return false;
					}
					endOfFile = true;
					return true;
				}

				line += String(buffer);
				int len = line.Length();
				if (len > 0)
				{
					if (line[len - 1] == _T('\n'))
					{
						line = line.TrimEnd(_T("\r\n"));
						return true;
					}
				}

				i++;
			}
			return false;
		}

		public: String ReadLine()
		{
			String s;
			bool endOfFile(false);
			bool bOk = GetLine(s, endOfFile);
			if (!bOk) throw Exception(_T("System.IO.IOException"));
			return s;
		}
		public: String ReadToEnd()
		{
			return _T("");
		}


		public: void Close()
		{
			if (isopened) { fclose(fin); fin = NULL; isopened = false; }
		}
		private: FILE *fin;
				 bool isopened;
				 friend class File;
		};





		class StreamWriter
		{
		public: StreamWriter(const String &path) :autoflush(false), isopened(false)
		{
			Open(path);
		}

		public: StreamWriter() :autoflush(false), isopened(false)
		{
		}

				void Open(const String &path)
				{
					if (isopened) throw Exception(_T("File already opened"));
					autoflush = false;
					isopened = false;
#pragma warning(disable: 4996)
					fout = _tfopen(path.c_str(), _T("w"));
#pragma warning(default: 4996)
					if (!fout) throw Exception(_T("Can't open file"));
					isopened = true;
				}

		public: ~StreamWriter()
		{
			Close();
		}
		public: void WriteLine(const String &text)
		{
			if (!isopened)   throw Exception(_T("File not opened"));
			int numchars = _fputts(text.c_str(), fout);
			numchars = _fputts(_T("\n"), fout);
			(numchars);
			if (autoflush) Flush();
		}
				void Flush()
				{
					fflush(fout);
				}

				TEMP_ARG_1 void WriteLine(const String &text, TCREF_ARG_1) { WriteLine(String::Format(text, ARG_1)); }
				TEMP_ARG_2 void WriteLine(const String &text, TCREF_ARG_2) { WriteLine(String::Format(text, ARG_2)); }
				TEMP_ARG_3 void WriteLine(const String &text, TCREF_ARG_3) { WriteLine(String::Format(text, ARG_3)); }
				TEMP_ARG_4 void WriteLine(const String &text, TCREF_ARG_4) { WriteLine(String::Format(text, ARG_4)); }
				TEMP_ARG_5 void WriteLine(const String &text, TCREF_ARG_5) { WriteLine(String::Format(text, ARG_5)); }
				TEMP_ARG_6 void WriteLine(const String &text, TCREF_ARG_6) { WriteLine(String::Format(text, ARG_6)); }
				TEMP_ARG_7 void WriteLine(const String &text, TCREF_ARG_7) { WriteLine(String::Format(text, ARG_7)); }
				TEMP_ARG_8 void WriteLine(const String &text, TCREF_ARG_8) { WriteLine(String::Format(text, ARG_8)); }
				TEMP_ARG_9 void WriteLine(const String &text, TCREF_ARG_9) { WriteLine(String::Format(text, ARG_9)); }
				TEMP_ARG_10 void WriteLine(const String &text, TCREF_ARG_10) { WriteLine(String::Format(text, ARG_10)); }
		public: void Write(const String &text)
		{
			if (!isopened)   throw Exception(_T("File not opened"));

			int numchars = _fputts(text.c_str(), fout);
			(numchars);
			if (autoflush) Flush();
		}
				TEMP_ARG_1 void Write(const String &text, TCREF_ARG_1) { Write(String::Format(text, ARG_1)); }
				TEMP_ARG_2 void Write(const String &text, TCREF_ARG_2) { Write(String::Format(text, ARG_2)); }
				TEMP_ARG_3 void Write(const String &text, TCREF_ARG_3) { Write(String::Format(text, ARG_3)); }
				TEMP_ARG_4 void Write(const String &text, TCREF_ARG_4) { Write(String::Format(text, ARG_4)); }
				TEMP_ARG_5 void Write(const String &text, TCREF_ARG_5) { Write(String::Format(text, ARG_5)); }
				TEMP_ARG_6 void Write(const String &text, TCREF_ARG_6) { Write(String::Format(text, ARG_6)); }
				TEMP_ARG_7 void Write(const String &text, TCREF_ARG_7) { Write(String::Format(text, ARG_7)); }
				TEMP_ARG_8 void Write(const String &text, TCREF_ARG_8) { Write(String::Format(text, ARG_8)); }
				TEMP_ARG_9 void Write(const String &text, TCREF_ARG_9) { Write(String::Format(text, ARG_9)); }
				TEMP_ARG_10 void Write(const String &text, TCREF_ARG_10) { Write(String::Format(text, ARG_10)); }


		public: void Close()
		{
			if (isopened) { fclose(fout); fout = NULL; isopened = false; }
		}
		public: bool AutoFlush() { return autoflush; }
		public: void AutoFlush(bool value) { autoflush = value; }
		private: FILE *fout;
				 bool isopened;
				 bool autoflush;
		};

		class BinaryWriter
		{
		public: BinaryWriter(const String &path)
		{
			Open(path);
		}

		public: BinaryWriter()
		{
			autoflush = false;
			isopened = false;
		}

				void Open(const String &path)
				{
					autoflush = false;
					isopened = false;
#pragma warning(disable: 4996)
					fout = _tfopen(path.c_str(), _T("wb"));
#pragma warning(default: 4996)
					if (!fout) throw Exception(_T("Can't open file"));
					isopened = true;
				}

		public: ~BinaryWriter()
		{
			Close();
		}

				void Flush()
				{
					fflush(fout);
				}

				/*
				public: void Write(const Char *text)
				{
				if (!isopened)   throw Exception(_T("File not opened"));
				int len = (int)_tcslen(text);

				if (len < 256)
				{
				byte byte_len = (byte)len;
				fwrite(&byte_len, sizeof(byte), 1, fout);
				}
				else if (len < 65536)
				{
				unsigned short ulen = (unsigned short)(len);
				fwrite(&ulen, sizeof(unsigned short), 1, fout);
				}
				else
				{
				fwrite(&len, sizeof(int), 1, fout);
				}
				fwrite(text, sizeof(Char)*len, 1, fout);
				if (autoflush) Flush();
				}

				public: void Write(const std::vector<byte>&vec)
				{
				if (!isopened)   throw Exception(_T("File not opened"));
				int len = (int)vec.size();
				byte *arr = new byte[len];
				for (int i = 0; i < len; i++)arr[i] = vec[i];
				fwrite(&len, sizeof(int), 1, fout);
				fwrite(arr, sizeof(byte)*len, 1, fout);
				delete[]arr;
				if (autoflush) Flush();
				}

				public: void Write(const String &text)
				{
				if (!isopened)   throw Exception(_T("File not opened"));
				fwrite(text.c_str(), sizeof(Char)*text.Length(), 1, fout);
				if (autoflush) Flush();
				}
				*/
		public: template<class T> void Write(const T &data)
		{
			if (!isopened)   throw Exception(_T("File not opened"));
			fwrite(&data, sizeof(T), 1, fout);
			if (autoflush) Flush();
		}

		public: template<class T>void Write(T *data, int length)
		{
			if (!isopened)   throw Exception(_T("File not opened"));
			fwrite(data, sizeof(T), length, fout);
			if (autoflush) Flush();
		}
				//	public: void Write(const unsigned char &data)
				//	{
				//		if (!isopened)   throw Exception(_T("File not opened"));
				//		fwrite(&data, sizeof(unsigned char), 1, fout);
				//		if (autoflush) Flush();
				//	}

				//	public: void Write(const long &data)
				//	{
				//		if (!isopened)   throw Exception(_T("File not opened"));
				//		fwrite(&data, sizeof(long), 1, fout);
				//		if (autoflush) Flush();
				//	}
				//public: void Write(const bool &data)
				//{
				//	if (!isopened)   throw Exception(_T("File not opened"));
				//	fwrite(&data, sizeof(bool), 1, fout);
				//	if (autoflush) Flush();
				//}
				//public: void Write(const double &data)
				//	{
				//		if (!isopened)   throw Exception(_T("File not opened"));
				//		fwrite(&data, sizeof(double), 1, fout);
				//		if (autoflush) Flush();
				//	}

				//TEMP_ARG_1 void Write(const String &text, TCREF_ARG_1) { Write(String::Format(text, ARG_1)); }



		public: void Close()
		{
			if (isopened) { fclose(fout); fout = NULL; isopened = false; }
		}
		public: bool AutoFlush() { return autoflush; }
		public: void AutoFlush(bool value) { autoflush = value; }
		private: FILE *fout;
				 bool isopened;
				 bool autoflush;
		};

		class Path
		{

		public: static bool HasExtension(const String &path)
		{
			String ext = GetExtension(path);
			return ext.Length() > 1;
		}
		public: static bool IsPathRooted(const String &path)
		{
			if (path.StartsWith(Path::DirectorySeparatorChar()) || path.StartsWith(Path::AltDirectorySeparatorChar())) return true;
			if (path.Length() >= 2 && path[1] == Path::VolumeSeparatorChar()) return true;
			//TODO:  && path[0] should be some letter a-z A-Z

			return false;
		}
		private: static bool IsFullPathWithDrive(const String &path)
		{
			if (path.Length() >= 2 && path[1] == Path::VolumeSeparatorChar()) return true;
			//TODO:  && path[0] should be some letter a-z A-Z

			return false;
		}
				 //public: static char[] GetInvalidFileNameChars();
				 //public: static char[] GetInvalidPathChars();
		public: static Char AltDirectorySeparatorChar() { return _T('/'); }
		public: static Char DirectorySeparatorChar() { return _T('\\'); }
		public: static Char PathSeparator() { return _T(';'); }
		public: static Char VolumeSeparatorChar() { return _T(':'); }
		public: static std::vector<Char> InvalidPathChars()
		{
			static int call = 0;
			static std::vector<Char> v;
			if (call == 0)
			{
				Char chars[] = { 34, 60, 62, 124, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
					17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 };
				v = std::vector<Char>(chars, chars + sizeof(chars) / sizeof(chars[0]));
			}
			++call;
			return v;
		};

				//public: static String ChangeExtension(const String &path, String extension);
				//public: static String Combine(params String[] paths);
		public: static String Combine(const String &path1, const String &path2)
		{
			bool bPath1EndsWithSlash = path1.EndsWith(Path::DirectorySeparatorChar()) || path1.EndsWith(Path::AltDirectorySeparatorChar());
			bool bPath2StartsWithSlash = path2.StartsWith(Path::DirectorySeparatorChar()) || path2.StartsWith(Path::AltDirectorySeparatorChar());
			bool bPath2IsFullPathWithDrive = Path::IsFullPathWithDrive(path2);

			if (path1.Length() == 0) return path2;
			if (path2.Length() == 0) return path1;

			if (bPath2IsFullPathWithDrive) return path2;
			if (!bPath1EndsWithSlash && !bPath2StartsWithSlash) return path1 + Path::DirectorySeparatorChar() + path2;
			if (bPath1EndsWithSlash && !bPath2StartsWithSlash) return path1 + path2;
			if (!bPath1EndsWithSlash && bPath2StartsWithSlash) return path1 + path2;
			if (bPath1EndsWithSlash && bPath2StartsWithSlash)
			{
				return path1.Substring(0, path1.Length() - 1) + Path::DirectorySeparatorChar() + path2.Substring(1);
			}
			return path2;
		}
		public: static String Combine(const String &path1, const String &path2, const String &path3)
		{
			return Combine(Combine(path1, path2), path3);
		}
		public: static String Combine(const String &path1, const String &path2, const String &path3, const String &path4)
		{
			return Combine(Combine(Combine(path1, path2), path3), path4);
		}

		public: static String GetDirectoryName(const String &path)
		{
			String ret;
			int k1 = Math::Max(path.LastIndexOf(DirectorySeparatorChar()), path.LastIndexOf(AltDirectorySeparatorChar()));
			if (k1 >= 0)
			{
				ret = path.Substring(0, k1);
				if (ret.Length() <= 2)  // 'C:\'
				{
					ret.Clear();
				}
			}
			return ret;
		}

		public: static String GetExtension(const String &path)
		{
			String filename = GetFileName(path);
			int i = filename.LastIndexOf(L'.');
			String ext;
			if (i >= 0) ext = filename.Substring(i);
			if (ext.Length() > 1) return ext;
			return L"";
		}

		public: static String GetFileName(const String &path)
		{
			int k = Math::Max(path.LastIndexOf(DirectorySeparatorChar()), path.LastIndexOf(AltDirectorySeparatorChar()));
			if (k >= 0) return path.Substring(k + 1);
			return path;
		}



		public: static String GetFileNameWithoutExtension(const String &path)
		{
			String filename = GetFileName(path);
			int i = filename.LastIndexOf(L'.');

			if (i < filename.Length() - 1)
			{
				filename = filename.Substring(0, i);
			}
			return filename;
		}

				//public: static String GetFullPath(const String &path);
				//public: static String GetPathRoot(const String &path);
				//public: static String GetRandomFileName();
				//public: static String GetTempFileName();
				//public: static String GetTempPath();
		};


		class File
		{

		public: static std::vector<String> ReadAllLines(const String &path)
		{
			std::vector<String> v;
			StreamReader sr(path);
			for (;;) //while (true) 
			{
				String line;
				bool endOfFile(false);
				bool bOk = sr.GetLine(line, endOfFile);

				if (!bOk)
				{
					if (v.size() == 0) throw Exception(_T("System.IO.IOException"));

					return v;
				}
				v.push_back(line);
				if (endOfFile) return v;
			}
			return v;
		}

		public: static String ReadAllText(const String&path)
		{
#pragma warning(disable: 4996)
			FILE *f = fopen(path.Tostdstring().c_str(), "rb");
#pragma warning(default: 4996)

			fseek(f, 0, SEEK_END);
			long fsize = ftell(f);
			fseek(f, 0, SEEK_SET);  //same as rewind(f);

			char *buf = new char[fsize + 1];
			fread(buf, fsize, 1, f);
			fclose(f);
			buf[fsize] = 0;
			String s = String::Convert_charsToString(buf);
			delete[]buf;

			return s;

		}
				/*
						static String ReadAllText(const String& path)
						{
						std::ifstream t(path);
						std::string str;

						t.seekg(0, std::ios::end);
						str.reserve(t.tellg());
						t.seekg(0, std::ios::beg);

						str.assign((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());                  // close file handle

						return str;
						}
						*/

		public:
			static void WriteAllText(const String &path, const String& contents)
			{
				StreamWriter wr(path);
				wr.Write(contents);
				wr.Close();
			}

			//static void WriteAllLines(const String& path, const Collections::Generic::Arr<String>& lines)     //, System.Text.Encoding encoding);
			//{

			//	StreamWriter wr(path);

			//	auto e = lines.cend();
			//	auto b = lines.cbegin();

			//	for (auto it = b; it != e; it++)
			//	{
			//		wr.WriteLine(*it);
			//	}

			//	wr.Close();
			//}

			static void WriteAllLines(const String& path, const std::vector<String>& lines)     //, System.Text.Encoding encoding);
			{
				StreamWriter wr(path);
				for (int i = 0; i < lines.size(); i++)
				{
					wr.WriteLine(lines[i]);
				}
				wr.Close();

			}

		};
	}
}
namespace XTSystem 
{
	  class Random 
	  {
	  private:
			int MBIG;
			int MSEED;
			int MZ;
			
      
			int inext;
			int inextp;
			int SeedArray[56];
    
	  void InitConsts()
	  {
			MBIG =  Int32::MaxValue();
			MSEED = 161803398;
			MZ = 0;

			inext = 0;
			inextp = 0;
	  }
	  public:
	 Random()
	 {
		 MakeRandom(GetTickCount());
	 }
    
	 Random(int Seed)
	 {
		 MakeRandom(Seed);
	 }

	  private: void MakeRandom(int Seed) 
	  {
		  InitConsts();
        int ii;
        int mj, mk;
    
        int subtraction = (Seed == Int32::MinValue()) ? Int32::MaxValue() : Math::Abs(Seed);
        mj = MSEED - subtraction;
        SeedArray[55]=mj;
        mk=1;
        for (int i=1; i<55; i++) 
		{  //Apparently the range [1..55] is special (Knuth) and so we're wasting the 0'th position.
          ii = (21*i)%55;
          SeedArray[ii]=mk;
          mk = mj - mk;
          if (mk<0) mk+=MBIG;
          mj=SeedArray[ii];
        }
        for (int k=1; k<5; k++) {
          for (int i=1; i<56; i++) {
        SeedArray[i] -= SeedArray[1+(i+30)%55];
        if (SeedArray[i]<0) SeedArray[i]+=MBIG;
          }
        }
        inext=0;
        inextp = 21;
        Seed = 1;
      }
    
	  protected: virtual double Sample() 
	  {
          //Including this division at the end gives us significantly improved
          //random number distribution.
          return (InternalSample()*(1.0/MBIG));
      }
    
	  private: int InternalSample() 
	  {
          int retVal;
          int locINext = inext;
          int locINextp = inextp;
 
          if (++locINext >=56) locINext=1;
          if (++locINextp>= 56) locINextp = 1;
          
          retVal = SeedArray[locINext]-SeedArray[locINextp];
 
          if (retVal == MBIG) retVal--;          
          if (retVal<0) retVal+=MBIG;
          
          SeedArray[locINext]=retVal;
 
          inext = locINext;
          inextp = locINextp;
                    
          return retVal;
      }

	  public: virtual int Next() 
	  {
        return InternalSample();
      }
    
	  private: double GetSampleForLargeRange() 
			   {
 
          int result = InternalSample();
          // Note we can't use addition here. The distribution will be bad if we do that.
          bool negative = (InternalSample()%2 == 0) ? true : false;  // decide the sign based on second sample
          if( negative) {
              result = -result;
          }
          double d = result;
          d += (Int32::MaxValue() - 1); // get a number in range [0 .. 2 * Int32MaxValue - 1)
          d /= 2*(unsigned int)Int32::MaxValue() - 1  ;              
          return d;
      }
 
 
	  public: virtual int Next(int minValue, int maxValue) 
	  {
          if (minValue>maxValue) {
              //throw new ArgumentOutOfRangeException("minValue",Environment.GetResourceString("Argument_MinMaxValue", "minValue", "maxValue"));
          }
          //Contract.EndContractBlock();
          
          long range = (long)maxValue-minValue;
          if( range <= (long)Int32::MaxValue()) {  
              return ((int)(Sample() * range) + minValue);
          }          
          else { 
              return (int)((long)(GetSampleForLargeRange() * range) + minValue);
          }
      }
    
    
	  public: virtual int Next(int maxValue) 
	  {
          if (maxValue<0) 
		  {
              //throw new ArgumentOutOfRangeException("maxValue", Environment.GetResourceString("ArgumentOutOfRange_MustBePositive", "maxValue"));
          }
          //Contract.EndContractBlock();
          return (int)(Sample()*maxValue);
      }
    
    
	  public: virtual double NextDouble() 
	  {
        return Sample();
      }
    
    
	  //public: virtual void NextBytes(byte [] buffer)
	  //{
   //     if (buffer==null) throw new ArgumentNullException("buffer");
   //     //Contract.EndContractBlock();
   //     for (int i=0; i<buffer.Length; i++) {
   //       buffer[i]=(byte)(InternalSample()%(Byte.MaxValue+1)); 
   //     }
   //   }
    };
 

} 

/*
namespace XTSystem
{
    class MT19937
    {
        // Period parameters
        private:
        const ulong    N                = 624;
        const ulong    M                = 397;
        const ulong    MATRIX_A        = 0x9908B0DFUL;        // constant vector a
        const ulong UPPER_MASK        = 0x80000000UL;        // most significant w-r bits
        const ulong LOWER_MASK        = 0X7FFFFFFFUL;        // least significant r bits
        const uint    DEFAULT_SEED    = 4357;

        static ulong [] mt            = new ulong[N+1];    // the array for the state vector
        static ulong    mti            = N + 1;            // mti==N+1 means mt[N] is not initialized

        public: MT19937()
        {
            ulong [] init = new ulong[4];
            init[0]= 0x123;
            init[1]= 0x234;
            init[2]= 0x345;
            init[3] =0x456;
            ulong length = 4;
            init_by_array(init, length);
        }

        // initializes mt[N] with a seed
        void init_genrand(ulong s)
        {
            mt[0]= s & 0xffffffffUL;
            for (mti=1; mti < N; mti++)
            {
                mt[mti] = (1812433253UL * (mt[mti-1] ^ (mt[mti-1] >> 30)) + mti);
                // See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier.
                // In the previous versions, MSBs of the seed affect
                // only MSBs of the array mt[].
                // 2002/01/09 modified by Makoto Matsumoto
                mt[mti] &= 0xffffffffUL;
                // for >32 bit machines
            }
        }


        // initialize by an array with array-length
        // init_key is the array for initializing keys
        // key_length is its length
        public void init_by_array(ulong[] init_key, ulong key_length)
        {
            ulong i, j, k;
            init_genrand(19650218UL);
            i=1; j=0;
            k = ( N > key_length ? N : key_length);
            for (; k > 0; k--)
            {
                mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1664525UL))
                + init_key[j] + j;        // non linear
                mt[i] &= 0xffffffffUL;    // for WORDSIZE > 32 machines
                i++; j++;
                if (i>=N) { mt[0] = mt[N-1]; i=1; }
                if (j>=key_length) j=0;
            }
            for (k = N - 1; k > 0; k--)
            {
                mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1566083941UL))
                - i;                    // non linear
                mt[i] &= 0xffffffffUL;    // for WORDSIZE > 32 machines
                i++;
                if (i>=N) { mt[0] = mt[N-1]; i=1; }
            }
            mt[0] = 0x80000000UL;        // MSB is 1; assuring non-zero initial array
        }

        // generates a random number on [0,0x7fffffff]-interval
        public long genrand_int31()
        {
            return (long)(genrand_int32()>>1);
        }
        // generates a random number on [0,1]-real-interval
        public double genrand_real1()
        {
            return (double)genrand_int32()*(1.0/4294967295.0); // divided by 2^32-1
        }
        // generates a random number on [0,1)-real-interval
        public double genrand_real2()
        {
            return (double)genrand_int32()*(1.0/4294967296.0); // divided by 2^32
        }
        // generates a random number on (0,1)-real-interval
        public double genrand_real3()
        {
            return (((double)genrand_int32()) + 0.5)*(1.0/4294967296.0); // divided by 2^32
        }
        // generates a random number on [0,1) with 53-bit resolution
        public double genrand_res53()
        {
            ulong a = genrand_int32() >>5;
            ulong b = genrand_int32()>>6;
            return(double)(a*67108864.0+b)*(1.0/9007199254740992.0);
        }
        // These real versions are due to Isaku Wada, 2002/01/09 added

        // generates a random number on [0,0xffffffff]-interval
        public ulong genrand_int32()
        {
            ulong y = 0;
            ulong [] mag01 = new ulong[2];
            mag01[0]    = 0x0UL;
            mag01[1]    = MATRIX_A;
            // mag01[x] = x * MATRIX_A  for x=0,1

            if (mti >= N)
            {
                // generate N words at one time
                ulong kk;

                if (mti == N+1)   // if init_genrand() has not been called,
                    init_genrand(5489UL); // a default initial seed is used

                for (kk=0; kk < N - M; kk++)
                {
                    y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
                    mt[kk] = mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1UL];
                }
                for (;kk<N-1;kk++)
                {
                    y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
                    //mt[kk] = mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1UL];
                    mt[kk] = mt[kk - 227] ^ (y >> 1) ^ mag01[y & 0x1UL];
                }
                y = (mt[N-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
                mt[N-1] = mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1UL];

                mti = 0;
            }

            y = mt[mti++];

            // Tempering
            y ^= (y >> 11);
            y ^= (y << 7) & 0x9d2c5680UL;
            y ^= (y << 15) & 0xefc60000UL;
            y ^= (y >> 18);

            return y;
        }

        public int RandomRange(int lo, int hi)
        {
            return (Math.Abs((int)genrand_int32() % (hi - lo + 1)) + lo);
        }
        //public int RollDice(int face, int number_of_dice)
        //{
        //    int roll = 0;
        //    for(int loop=0; loop < number_of_dice; loop++)
        //    {
        //        roll += (RandomRange(1,face));
        //    }
        //    return roll;
        //}
        //public int D6(int die_count)    { return RollDice(6,die_count); }

    };
}


*/

namespace XTSystem
{
    namespace Diagnostics
    {
        class StopwatchObsolete
        {
        public: 
            //static const long Frequency;
            //static const bool IsHighResolution;
            StopwatchObsolete() { Reset(); }

            double ElapsedSeconds() const { return  ((double)(stop-start))/CLOCKS_PER_SEC; }

            //TimeSpan Elapsed()  { return (long) (1000*(stop-start)/CLOCKS_PER_SEC); }

            long ElapsedMilliseconds() const { return (long) ((1000*(stop-start))/CLOCKS_PER_SEC); }
            long ElapsedTicks() const { return (long) (stop-start); }
            bool IsRunning() const { return bIsRunning; }

            static long GetTimestamp() { return (long)clock(); }

            void Reset()
            {
                bIsRunning = false;
                start = 0;
                stop = 0;
            }

            void Restart()
            {
                bIsRunning = false;
                Start();
            }

            void Start()
            {
                if (!bIsRunning)
                {
                    bIsRunning = true;
                    start = stop = clock();
                }
            }

            static StopwatchObsolete StartNew()
            {
                StopwatchObsolete sw;
                sw.Start();
                return sw;
            }

            void Stop()
            {
                stop = clock();
                bIsRunning = false;

            }
        private: 
            bool bIsRunning;
            clock_t start, stop;

        };


        class Stopwatch
        {
        public: 
            typedef long long unsigned int TimeStamp;
            //static const long Frequency;
            //static const bool IsHighResolution;

            Stopwatch() 
            { 
                Reset(); 
            }
            static Stopwatch StartNew()
            {
                Stopwatch sw;
                sw.Start();
                return sw;
            }

            double ElapsedSeconds() const { return (ElapsedMicroseconds(sum)/1000000.0); }

            //TimeSpan Elapsed() 
            //{
            //
            //return (long) (1000*(stop-start)/CLOCKS_PER_SEC);
            //}

            long ElapsedMilliseconds() const { return (long)(ElapsedMicroseconds(sum)/1000); }
            //long ElapsedTicks() const { return (long) (stop-start); }
            bool IsRunning() const { return bIsRunning; }

            static TimeStamp GetTimestamp() { return GetCurrentTimestamp(); }

            void Reset()
            {
                bIsRunning = false;
                start = 0;
                stop = 0;
                sum = 0;
            }

            void Restart()
            {
                Reset();
                Start();
            }

            void Start()
            {
                if (!bIsRunning)
                {
                    bIsRunning = true;
                    start = stop = GetCurrentTimestamp();
                }
            }

            void Stop()
            {
                stop = GetCurrentTimestamp();
                sum += stop - start;
                bIsRunning = false;

            }
        private: 
            bool bIsRunning;
            TimeStamp start, stop,sum;

            static TimeStamp GetCurrentTimestamp()
            {
                LARGE_INTEGER res;
                QueryPerformanceCounter(&res);
                return res.QuadPart;
            }

            static TimeStamp InitalizeFrequency()
            {
                LARGE_INTEGER res;
                QueryPerformanceFrequency((LARGE_INTEGER *)&res);
                return res.QuadPart;
            }

            static TimeStamp ElapsedMicroseconds(TimeStamp deltaTimeStamp) 
            { 
                return ((deltaTimeStamp)* 1000000 / InitalizeFrequency()); 
            }
        };
    }
}


namespace XTSystem
{
	class Environment
	{
	public: 
		static String CurrentDirectory()
		{
			Char buff[2000];
			GetCurrentDirectory(2000,buff);
			return String(buff);
		}

		static String OSVersion()
		{
			OSVERSIONINFO osvi;
			//BOOL bIsWindowsXPorLater;
			ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
			osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
#pragma warning(disable: 4996)
			GetVersionEx(&osvi);
#pragma warning(default: 4996)
			return String::Format(_T("Microsoft Windows NT {0}.{1}.{2} {3}"), osvi.dwMajorVersion,osvi.dwMinorVersion, osvi.dwBuildNumber, String(osvi.szCSDVersion));
			//return String(_T("Microsoft Windows NT 6.1.7601 Service Pack 1"));
		}

		static int ProcessorCount()
		{
			SYSTEM_INFO sysinfo;
			GetSystemInfo( &sysinfo );
			return sysinfo.dwNumberOfProcessors;
		}

		static String MachineName()
		{
			DWORD len;
			Char buff[200];
			GetComputerName(buff,&len);
			return String(buff);

		}

		static String NewLine()
		{
			return _T("\r\n");
		}
		
	};
}

#undef GetCurrentDirectory
#undef SetCurrentDirectory
#undef CreateDirectory

namespace XTSystem
{
    namespace IO
    {
        class Directory
        {
            public: static std::vector<String> GetDirectories(const String &path, const String &searchPattern)
                    {
                        return GetFilesSelected(path, searchPattern, &IsDirectoryPriv);
                    }
            public: static std::vector<String> GetDirectories(const String &path)
                    {
                        return GetDirectories(path, _T("*"));
                    }
            public: static std::vector<String> GetFiles(const String &path, const String &searchPattern)
                    {
                        return GetFilesSelected(path, searchPattern, &IsFileOrLinkPriv);
                    }
            public: static std::vector<String> GetFiles(const String &path)
                {
                    return GetFiles(path, _T("*"));
                }

			public: static bool Exists(const String &path)
			{
				DWORD ftyp = GetFileAttributesW(path.c_str());
				if (ftyp == INVALID_FILE_ATTRIBUTES)
					return false;  //something is wrong with your path!

				if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
					return true;   // this is a directory!

				return false;    // this is not a directory!
			}

			public: static String GetCurrentDirectory_()
			{
				TCHAR path[MAX_PATH + 1] = L"";
				DWORD len = GetCurrentDirectoryW(MAX_PATH, path);

				String value(path);
				return value;
			}

			public: static void SetCurrentDirectory_(const String &path)
			{
				BOOL ok = SetCurrentDirectoryW(path.c_str());
				if (ok != TRUE)
				{
					throw Exception(L"Can't set current directory");
				}

			}
			//public: static String GetDirectoryRoot(const String &path)
			//{

			//}

			public: static /*DirectoryInfo*/ void CreateDirectory_(const String &path)
			{
				BOOL ok = CreateDirectoryW(path.c_str(), NULL);
				if (ok != TRUE)
				{
					throw Exception(L"Can't set current directory");
				}
			}

			public: static void Delete(const String &path)
			{
				BOOL ok = RemoveDirectory(path.c_str());
			}
			//public: static void Delete(const String &path, bool recursive)
			//{

			//}

/*			private: bool deleteDirectory(std::string& directoryname, int flags)
			{
				if (directoryname.at(directoryname.size() - 1) != '\\') directoryname += '\\';

				if ((flags & CONTENTS) == CONTENTS)
				{
					WIN32_FIND_DATAA fdata;
					HANDLE dhandle;
					//BUG 1: Adding a extra \ to the directory name..
					directoryname += "*";
					dhandle = FindFirstFileA(directoryname.c_str(), &fdata);
					//BUG 2: Not checking for invalid file handle return from FindFirstFileA
					if (dhandle != INVALID_HANDLE_VALUE)
					{
						// Loop through all the files in the main directory and delete files & make a list of directories
						while (true)
						{
							if (FindNextFileA(dhandle, &fdata))
							{
								std::string     filename = fdata.cFileName;
								if (filename.compare("..") != 0)
								{
									//BUG 3: caused by BUG 1 - Removing too many characters from string.. removing 1 instead of 2
									std::string filelocation = directoryname.substr(0, directoryname.size() - 1) + filename;

									// If we've encountered a directory then recall this function for that specific folder.

									//BUG 4: not really a bug, but spurious function call - we know its a directory from FindData already, use it.
									if ((fdata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
										DeleteFileA(filelocation.c_str());
									else
										deleteDirectory(filelocation, DIRECTORY_AND_CONTENTS);
								}
							}
							else if (GetLastError() == ERROR_NO_MORE_FILES)    break;
						}
						directoryname = directoryname.substr(0, directoryname.size() - 2);
						//BUG 5: Not closing the FileFind with FindClose - OS keeps handles to directory open.  MAIN BUG
						FindClose(dhandle);
					}
				}
				if ((flags & DIRECTORY) == DIRECTORY)
				{
					HANDLE DirectoryHandle;
					DirectoryHandle = CreateFileA(directoryname.c_str(),
						FILE_LIST_DIRECTORY,
						FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
						NULL,
						OPEN_EXISTING,
						FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
						NULL);
					//BUG 6: Not checking CreateFileA for invalid handle return.
					if (DirectoryHandle != INVALID_HANDLE_VALUE)
					{

						bool DeletionResult = (RemoveDirectoryA(directoryname.c_str()) != 0) ? true : false;
						CloseHandle(DirectoryHandle);
						return DeletionResult;
					}
					else
					{
						return true;
					}
				}

				return true;
			}
*/

			//public: static DirectoryInfo CreateDirectory(const String &path, DirectorySecurity directorySecurity);


            private: static bool IsFileOrLinkPriv(WIN32_FIND_DATA *ffd) { return (ffd->dwFileAttributes & ~FILE_ATTRIBUTE_DIRECTORY)!=0;}
            private: static bool IsDirectoryPriv(WIN32_FIND_DATA *ffd) { return (ffd->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)!=0;}
            private: static std::vector<String> GetFilesSelected(const String &path, const String &searchPattern, bool (*predicate)(WIN32_FIND_DATA *))
                     {
                         std::vector<String> vec;

                         String searchSpecification = IO::Path::Combine(path, searchPattern);

                         WIN32_FIND_DATA ffd;
                         //LARGE_INTEGER filesize;
                         HANDLE hFind = INVALID_HANDLE_VALUE;
                         //DWORD dwError=0;

                         if (searchSpecification.Length()>=MAX_PATH) return vec;    // error

                         hFind = FindFirstFile(searchSpecification.c_str(), &ffd);

                         if (INVALID_HANDLE_VALUE == hFind) return vec;

                         do
                         {
                             if (predicate(&ffd)) vec.push_back(IO::Path::Combine(path,String(ffd.cFileName)));
                         } while (FindNextFile(hFind, &ffd) != 0);

                         return vec;
                     }
        };

/*		
        class DirectoryObsolete
        {
            //public: static DateTime GetCreationTime(const String &path);
            //public: static DateTime GetCreationTimeUtc(const String &path);
            //public: static DateTime GetLastAccessTime(const String &path);
            //public: static DateTime GetLastAccessTimeUtc(const String &path);
            //public: static DateTime GetLastWriteTime(const String &path);
            //public: static DateTime GetLastWriteTimeUtc(const String &path);
            //public: static DirectoryInfo CreateDirectory(const String &path);
            //public: static DirectoryInfo CreateDirectory(const String &path, DirectorySecurity directorySecurity);
            //public: static DirectoryInfo GetParent(const String &path);
            //public: static DirectorySecurity GetAccessControl(const String &path);
            //public: static DirectorySecurity GetAccessControl(const String &path, AccessControlSections includeSections);
            //public: static IEnumerable<String> EnumerateDirectories(const String &path);
            //public: static IEnumerable<String> EnumerateDirectories(const String &path, const String &searchPattern);
            //public: static IEnumerable<String> EnumerateDirectories(const String &path, const String &searchPattern, SearchOption searchOption);
            //public: static IEnumerable<String> EnumerateFileSystemEntries(const String &path);
            //public: static IEnumerable<String> EnumerateFileSystemEntries(const String &path, const String &searchPattern);
            //public: static IEnumerable<String> EnumerateFileSystemEntries(const String &path, const String &searchPattern, SearchOption searchOption);
            //public: static IEnumerable<String> EnumerateFiles(const String &path);
            //public: static IEnumerable<String> EnumerateFiles(const String &path, const String &searchPattern);
            //public: static IEnumerable<String> EnumerateFiles(const String &path, const String &searchPattern, SearchOption searchOption);
            //public: static bool Exists(const String &path);
            //public: static String GetCurrentDirectory();
            //public: static String GetDirectoryRoot(const String &path);

			private: static std::vector<String> GetFilesSelected(const String &path, bool (*predicate)(dirent::wdirent*))
					{
						    dirent::WDIR *dir = dirent::wopendir (path.c_str());
							if (dir == NULL) throw Exception(_T("Invalid path!"));

							std::vector<String> vec;
							struct dirent::wdirent *ent;
							while ((ent = dirent::wreaddir(dir)) != NULL) 
							{
								if (predicate(ent))	vec.push_back(Path::Combine(path,String(ent->d_name)));
							}
							dirent::wclosedir(dir);
							return vec;
					}

			private: static bool IsFileOrLinkPriv(dirent::wdirent* ent) { return ent->d_type==DT_REG || ent->d_type==DT_LNK; }
			private: static bool IsDirectoryPriv(dirent::wdirent* ent) { return ent->d_type==DT_DIR;}

            public: static std::vector<String> GetDirectories(const String &path)
					{
						return GetFilesSelected(path, &IsDirectoryPriv);
					}
            public: static std::vector<String> GetDirectories(const String &path, const String &searchPattern)
					{
						return GetFilesSelected(path, &IsDirectoryPriv);
					}
            //public: static String[] GetDirectories(const String &path, const String &searchPattern, SearchOption searchOption);
            //public: static String[] GetFileSystemEntries(const String &path);
            //public: static String[] GetFileSystemEntries(const String &path, const String &searchPattern);
            //public: static String[] GetFileSystemEntries(const String &path, const String &searchPattern, SearchOption searchOption);
		
			public: static std::vector<String> GetFiles(const String &path)
					{
						return GetFilesSelected(path, &IsFileOrLinkPriv);
					}

			//public: static String[] GetFiles(const String &path, const String &searchPattern);
            //public: static String[] GetFiles(const String &path, const String &searchPattern, SearchOption searchOption);
            //public: static String[] GetLogicalDrives();
            //public: static void Delete(const String &path);
            //public: static void Delete(const String &path, bool recursive);
            //public: static void Move(String sourceDirName, String destDirName);
            //public: static void SetAccessControl(const String &path, DirectorySecurity directorySecurity);
            //public: static void SetCreationTime(const String &path, DateTime creationTime);
            //public: static void SetCreationTimeUtc(const String &path, DateTime creationTimeUtc);
            //public: static void SetCurrentDirectory(const String &path);
            //public: static void SetLastAccessTime(const String &path, DateTime lastAccessTime);
            //public: static void SetLastAccessTimeUtc(const String &path, DateTime lastAccessTimeUtc);
            //public: static void SetLastWriteTime(const String &path, DateTime lastWriteTime);
            //public: static void SetLastWriteTimeUtc(const String &path, DateTime lastWriteTimeUtc);

        };
*/
    }
}


#define GetCurrentDirectory GetCurrentDirectoryW
#define SetCurrentDirectory SetCurrentDirectoryW
#define CreateDirectory CreateDirectoryW

#include <map>
#include <list>

template <typename T, std::size_t N>char (&sizeof_array( T(&)[N] ))[N];    // declared, undefined
#define ARR_SIZE(x) sizeof(sizeof_array(x))
#define Array_Init(TYPE, A, ...) TYPE A##_init[] = __VA_ARGS__ ;  Arr<TYPE> A(A##_init,ARR_SIZE(A##_init));

namespace XTSystem
{
	namespace Collections
	{
			namespace Generic
			{
				template<class TValue>
				class List
				{
						typedef std::list<TValue> T_List;
						T_List m_vecValues;

						public: List(int capacity)
								{
									m_vecValues.reserve(capacity);
								}
						public: List()
						{

						}
						public: void Add(const TValue &val)
								{
									m_vecValues.push_back(val);
								}

						public: int BinarySearch(const TValue &item)
						{
							return -1;
						}
						public: int Count() const
							{
								return (int)m_vecValues.size();
							}
						public: int Capacity() const
							{
								return (int)m_vecValues.capacity();
							}
						public: TValue &operator[](const int in_index)
								{
									if (in_index<0 || in_index>=(int)m_vecValues.size())
										throw Exception(_T("Out of bounds"));
									return m_vecValues.at(in_index); 
								}
	

						
						public: typedef typename T_List::iterator iterator;
						public: typedef typename T_List::const_iterator const_iterator;
						public: iterator begin() { return m_vecValues.begin(); }
						public: iterator end() { return m_vecValues.end(); }
				};


				template<class TValue>
				class Arr
				{
						typedef std::vector<TValue> T_Arr;
						T_Arr m_vecValues;

						public: Arr(int length)
								{
									m_vecValues.reserve(length);
									for (int i=0;i<length;++i) m_vecValues.push_back(TValue());
								}

						public: Arr(const TValue*arr, int len)
						{
							m_vecValues = T_Arr(arr,arr+len);
						}

						public: int Length() const
							{
								return (int)m_vecValues.size();
							}

						public: TValue &operator[](const int in_index)
								{
									if (in_index<0 || in_index>=(int)m_vecValues.size())
										throw Exception(_T("Out of bounds"));
									return m_vecValues.at(in_index); 
								}
						public: const TValue &operator[](const int in_index) const
								{
									if (in_index<0 || in_index>=(int)m_vecValues.size())
										throw Exception(_T("Out of bounds"));
									return m_vecValues.at(in_index); 
								}
	

						
						public: typedef typename T_Arr::iterator iterator;
						public: typedef typename T_Arr::const_iterator const_iterator;
						public: iterator begin() { return m_vecValues.begin(); }
						public: iterator end() { return m_vecValues.end(); }
				};


				template<class TKey, class TValue>
				class Dictionary
				{
                    private:
                            typedef std::map<TKey, TValue> T_mapType;
                            T_mapType m_map;
                            std::list<TKey*> m_lstKeys;
                            std::list<TValue*> m_lstValues;

					public: int Count()
							{
								
							}
					public: void Clear()
							{
							}
					public: void Add(const TKey &key, const TValue &value)
							{
								m_map.insert(std::make_pair(key,value));
							}

					public: std::list<TKey> Keys()
						    {
                                T_mapType::const_iterator b = m_map.begin();
                                T_mapType::const_iterator e = m_map.end();
                                std::list<TKey> lst;
                                for (T_mapType::const_iterator it = b; it != e; it++) lst.push_back(it->first);
							    return lst;
						    }

                    public: std::list<TValue> Values()
                            {
                                T_mapType::const_iterator b = m_map.begin();
                                T_mapType::const_iterator e = m_map.end();
                                std::list<TValue> lst;
                                for (T_mapType::const_iterator it = b; it != e; it++) lst.push_back(it->second);
                                return lst;
                            }

                            public: TValue &operator[](const TKey &key)
                            {
                                T_mapType::iterator it = m_map.find(key);
                                if (it == m_map.end()) throw Exception(_T("Item not in dictionary"));
                                return it->second;
                            }
                            public: const TValue &operator[](const TKey &key) const
                            {
                                T_mapType::const_iterator it = m_map.find(key);
                                if (it == m_map.end()) throw Exception(_T("Item not in dictionary"));
                                return it->second;
                            }

				    };
			}
	}
}


namespace Microsoft
{

	namespace Win32
	{
		enum RegistryValueKind
		{
			None = -1,
			Unknown = 0,
			String_reg = 1,
			ExpandString = 2,
			Binary = 3,
			DWord = 4,
			MultiString = 7,
			QWord = 11,
		};

		union convert_BYTE_DWORD
		{
			DWORD dword_value;
			BYTE bytes[sizeof(DWORD)];
		};

		class RegistryObject
		{
			public:
				RegistryObject(const RegistryObject &ro):length(ro.length),kind(ro.kind)
				{
					data = new BYTE[length];
					memcpy(data,ro.data,length*sizeof(BYTE));
				}
			~RegistryObject() { if (NULL!=data) delete []data; }
			static RegistryObject Create(DWORD _type, BYTE *_data, DWORD _length)
			{
				if (_type==REG_DWORD) return RegistryObject(RegistryValueKind::DWord, _data, _length); 
				//if (_type==REG_QWORD) return RegistryObject(RegistryValueKind::QWord, _data,sizeof(QWORD)); 
				if (_type==REG_SZ) return RegistryObject(RegistryValueKind::String_reg,_data,_length); 
				if (_type==REG_MULTI_SZ) return RegistryObject(RegistryValueKind::MultiString,_data,_length); 
				if (_type==REG_BINARY) return RegistryObject(RegistryValueKind::Binary,_data,_length); 
				if (_type==REG_EXPAND_SZ) return RegistryObject(RegistryValueKind::ExpandString,_data,_length); 


				throw XTSystem::Exception(_T("Wrong registry type"));
			}
			operator int()
			{
				if (kind == RegistryValueKind::DWord) 
				{
					return (int)convertToDWORD();
				}
				throw XTSystem::Exception(_T("can't convert to int"));
			}
			private: RegistryObject(RegistryValueKind _kind, BYTE *_data, DWORD _length): kind(_kind), length(_length), data(_data) 
			{	
			}

			RegistryValueKind kind;
			DWORD length;
			BYTE *data;
		public: 
			RegistryValueKind Kind() const { return kind; }
			XTSystem::String ToString() const
			{
				if (kind == RegistryValueKind::DWord) 
				{ 
					DWORD d = convertToDWORD();
					if ((int)d<0)
					{
						return XTSystem::String::Format(_T("{0}"),(int)d);
					}
					return XTSystem::String::Format(_T("{0}"),d);
				}
				if (kind == RegistryValueKind::String_reg) 
				{ 
					XTSystem::Char *str = (XTSystem::Char *) (data);
					return XTSystem::String::Format(_T("{0}"),str);
				}
					
				return _T("");
			}
		private: DWORD convertToDWORD() const
				 {
					convert_BYTE_DWORD convert;
					for (int i=0;i<sizeof(DWORD);++i) convert.bytes[i] = data[i];
					return convert.dword_value;
				 }

		};

		class RegistryKey
		{
			public: ~RegistryKey()
					{
						Close();
					}
			private: RegistryKey(HKEY _hKey, const XTSystem::String &_name, const XTSystem::String &_relativePathToParent, bool _writable = false)
			{
				hKey = _hKey;
				name = _name;
				relativePathToParent = _relativePathToParent;
				writable = _writable;
				//GetInfo();
			}

			HKEY hKey;
			XTSystem::String name;
			XTSystem::String relativePathToParent;
			bool writable;
			//public: SafeRegistryHandle Handle() { get; }
			public: XTSystem::String Name() { return name; }
			public: XTSystem::String RegistryKeyPath() { return relativePathToParent; }
			public: int SubKeyCount() 
					{ 
						DWORD dwNumberOfSubKeys(0);
						DWORD retCode = RegQueryInfoKey(hKey, NULL, NULL, NULL, &dwNumberOfSubKeys, 
														NULL, NULL, NULL, NULL, NULL, NULL, NULL);
						if (retCode != ERROR_SUCCESS) throw XTSystem::Exception(_T("can't get number of subkeys"));
						return dwNumberOfSubKeys;
					}
			public: int ValueCount() 
					{ 
						DWORD dwNumberOfValues(0);
						DWORD retCode = RegQueryInfoKey(hKey, NULL, NULL, NULL, NULL, NULL, &dwNumberOfValues,
														NULL, NULL, NULL, NULL, NULL);
						if (retCode != ERROR_SUCCESS) throw XTSystem::Exception(_T("can't get number of values"));
						return dwNumberOfValues;
					}

			//public: RegistryView View { get; }

			public: void Close()
					{
						if (NULL!=hKey) RegCloseKey(hKey);
						hKey = NULL;
					}
			public:
				RegistryKey *OpenSubKey(const XTSystem::String &path, bool writable = false)
				{
					HKEY hSubKey;
					REGSAM mode = (writable)?KEY_WRITE|KEY_READ:KEY_READ;
					if( RegOpenKeyEx( hKey,path.c_str(), 0, mode, &hSubKey) == ERROR_SUCCESS)
				    {
						XTSystem::String name = GetNameFromRegistryPath(path);
						XTSystem::String relpathtoparrent = relativePathToParent + _T('\\') + path;
						return new RegistryKey(hSubKey,name,relpathtoparrent,writable);
				    }
					throw XTSystem::Exception(_T("Can't find registry key"));
					return NULL;
				}

			public:
				RegistryKey *CreateSubKey(const XTSystem::String &subkey)
				{
					if (!writable) throw XTSystem::Exception(_T("registry key is not writable"));

					HKEY hSubKey;
					LONG retcode = RegCreateKeyEx(hKey, subkey.c_str(), 0L, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hSubKey, NULL );
					if (retcode != ERROR_SUCCESS)
					{
						throw XTSystem::Exception(_T("Key already exist"));
					}
		
					XTSystem::String name = GetNameFromRegistryPath(subkey);
					XTSystem::String relpathtoparrent = relativePathToParent + _T('\\') + subkey;
					return new RegistryKey(hSubKey,name,relpathtoparrent, true);
				}
			public:

				void SetValue(const XTSystem::String &name, const XTSystem::Char*value)
				{
					if (!writable) throw XTSystem::Exception(_T("registry key is not writable"));

					LONG RetCode = RegSetValueEx (hKey, name.c_str(), NULL, REG_SZ, (LPBYTE)value, (DWORD)((_tcslen(value)+1)*sizeof(XTSystem::Char)));
					if (RetCode != ERROR_SUCCESS) throw XTSystem::Exception(_T("can't write key"));

				}
				void SetValue(const XTSystem::String &name, const XTSystem::String &value)
				{	
					if (!writable) throw XTSystem::Exception(_T("registry key is not writable"));

					LONG RetCode = RegSetValueEx (hKey, name.c_str(), NULL, REG_SZ, (LPBYTE)value.c_str(), (DWORD)((value.Length()+1)*sizeof(XTSystem::Char)));
					if (RetCode != ERROR_SUCCESS) throw XTSystem::Exception(_T("can't write key"));

				}

                void SetValue(const XTSystem::String &name, XTSystem::Char value)
                {
                    if (!writable) throw XTSystem::Exception(_T("registry key is not writable"));
                    SetValue(name, XTSystem::String()+value);
                }

				void SetValue(const XTSystem::String &name, bool value)
				{
					if (!writable) throw XTSystem::Exception(_T("registry key is not writable"));
					SetValue(name, XTSystem::Bool::ToString(value));
				}
				void SetValue(const XTSystem::String &name, double value)
				{
					if (!writable) throw XTSystem::Exception(_T("registry key is not writable"));
					double abs_value = XTSystem::Math::Abs(value);
					if (abs_value>1e-4 && abs_value <1e10)
					{
						SetValue(name, XTSystem::String::Format(_T("{0:15.14f}"),value));
					} else
					{
						SetValue(name, XTSystem::String::Format(_T("{0:15.14E}"),value));
					}
				}

				void SetValue(const XTSystem::String &name, short data)
				{
					if (!writable) throw XTSystem::Exception(_T("registry key is not writable"));
					SetValue(name, XTSystem::String::Format(_T("{0}"),data));
				}

				void SetValue(const XTSystem::String &name, int value)
				{
					if (!writable) throw XTSystem::Exception(_T("registry key is not writable"));
					DWORD data = (DWORD)value;
					//LPBYTE bytes = (LPBYTE)&data;
					LONG RetCode = RegSetValueEx(hKey, name.c_str(), NULL, REG_DWORD, (LPBYTE)&data, sizeof(data));
					if (RetCode != ERROR_SUCCESS) throw XTSystem::Exception(_T("can't write key"));
				}

				void SetValue(const XTSystem::String &name, long data)
				{
					if (!writable) throw XTSystem::Exception(_T("registry key is not writable"));
					SetValue(name, XTSystem::String::Format(_T("{0}"),data));
				}

				void SetValue(const XTSystem::String &name, long long data)
				{
					if (!writable) throw XTSystem::Exception(_T("registry key is not writable"));
					SetValue(name, XTSystem::String::Format(_T("{0}"),data));
				}

				void SetValue(const XTSystem::String &name, unsigned short data)
				{
					if (!writable) throw XTSystem::Exception(_T("registry key is not writable"));
					SetValue(name, XTSystem::String::Format(_T("{0}"),data));
				}

				void SetValue(const XTSystem::String &name, unsigned int data)
				{
					if (!writable) throw XTSystem::Exception(_T("registry key is not writable"));
					SetValue(name, XTSystem::String::Format(_T("{0}"),data));
				}

				void SetValue(const XTSystem::String &name, unsigned long data)
				{
					if (!writable) throw XTSystem::Exception(_T("registry key is not writable"));
					SetValue(name, XTSystem::String::Format(_T("{0}"),data));
				}


				void SetValue(const XTSystem::String &name, const XTSystem::Collections::Generic::Arr<BYTE> &data)
				{
					if (!writable) throw XTSystem::Exception(_T("registry key is not writable"));
					BYTE *arr=new BYTE[data.Length()];
					for (int i=0;i<data.Length();i++) arr[i]=data[i];

					LONG RetCode = RegSetValueEx(hKey, name.c_str(), NULL, REG_BINARY, arr, data.Length());
					XTSystem::safe_delete_array(arr);
					if (RetCode != ERROR_SUCCESS) throw XTSystem::Exception(_T("can't write key"));
				}

				void SetValue(const XTSystem::String &name, const BYTE*data, int length)
				{
					if (!writable) throw XTSystem::Exception(_T("registry key is not writable"));
					LONG RetCode = RegSetValueEx(hKey, name.c_str(), NULL, REG_BINARY, data, length);
					if (RetCode != ERROR_SUCCESS) throw XTSystem::Exception(_T("can't write key"));
				}

				private: XTSystem::Char *accumulate(const XTSystem::String*data, int length, int *pSize)
						 {
							int size = 1;
							for (int i=0;i<length; ++i) size += data[i].Length()+1;
							XTSystem::Char *arr = new XTSystem::Char[size];
							XTSystem::Char *arr1 = arr;
							for (int i=0;i<length; ++i)
							{
								#pragma warning(disable: 4996)
								_tcsncpy(arr1, data[i].c_str(), data[i].Length());
								#pragma warning(default: 4996)
								arr1 += data[i].Length();
								*arr1 = _T('\0'); ++arr1;
							}
							*arr1 = _T('\0');
							*pSize = size;
							return arr;
						 }

				private: XTSystem::Char *accumulate(const XTSystem::Collections::Generic::Arr<XTSystem::String> &data, int *pSize)
						 {
							int size = 1;
							for (int i=0;i<data.Length(); ++i) size += data[i].Length()+1;
							XTSystem::Char *arr = new XTSystem::Char[size];
							XTSystem::Char *arr1 = arr;
							for (int i=0;i<data.Length(); ++i)
							{
								#pragma warning(disable: 4996)
								_tcsncpy(arr1, data[i].c_str(), data[i].Length());
								#pragma warning(default: 4996)
								arr1 += data[i].Length();
								*arr1 = _T('\0'); ++arr1;
							}
							*arr1 = _T('\0');
							*pSize = size;
							return arr;
						 }
						 
				public: void SetValue(const XTSystem::String &name, const XTSystem::String*data, int length)
				{
					if (!writable) throw XTSystem::Exception(_T("registry key is not writable"));
					int size = 0;
					XTSystem::Char *arr = accumulate(data, length, &size);
					LONG RetCode = RegSetValueEx(hKey, name.c_str(), NULL, REG_MULTI_SZ, (LPBYTE)arr, (DWORD)((size)*sizeof(XTSystem::Char)));
					XTSystem::safe_delete_array(arr);
					if (RetCode != ERROR_SUCCESS) throw XTSystem::Exception(_T("can't write key"));
				}

				void SetValue(const XTSystem::String &name, const XTSystem::Collections::Generic::Arr<XTSystem::String> &data)
				{
					if (!writable) throw XTSystem::Exception(_T("registry key is not writable"));
					int size = 0;
					XTSystem::Char *arr = accumulate(data, &size);
					LONG RetCode = RegSetValueEx(hKey, name.c_str(), NULL, REG_MULTI_SZ, (LPBYTE)arr, (DWORD)((size)*sizeof(XTSystem::Char)));
					XTSystem::safe_delete_array(arr);
					if (RetCode != ERROR_SUCCESS) throw XTSystem::Exception(_T("can't write key"));
				}

				public: XTSystem::Collections::Generic::Arr<XTSystem::String> GetSubKeyNames()
					{
						int numSubkeys = SubKeyCount(); 
						XTSystem::Collections::Generic::Arr<XTSystem::String> arr(numSubkeys);
						if (0 == numSubkeys) return arr;

						TCHAR bufSubKeyName[MAX_KEY_LENGTH];
						for (int i=0; i<numSubkeys; i++) 
						{ 
							DWORD dwSizeOfSubKeyName = MAX_KEY_LENGTH;
							DWORD retCode1 = RegEnumKeyEx(hKey, i, bufSubKeyName, &dwSizeOfSubKeyName, NULL, NULL, NULL, NULL); 
							if (retCode1 == ERROR_SUCCESS) arr[i] = bufSubKeyName;
						}

						return arr;

					}
				public: XTSystem::Collections::Generic::Arr<XTSystem::String> GetValueNames()
					{
						int numValues = ValueCount(); 
						XTSystem::Collections::Generic::Arr<XTSystem::String> arr(numValues);
						if (0 == numValues) return arr;

						TCHAR bufValueName[MAX_VALUE_NAME]; 
						for (int i=0; i<numValues; i++) 
						{ 
							DWORD dwSizeOfValueName = MAX_VALUE_NAME; 
							DWORD retCode1 = RegEnumValue(hKey, i, bufValueName, &dwSizeOfValueName, NULL, NULL, NULL, NULL);
 							if (retCode1 == ERROR_SUCCESS) arr[i] = bufValueName;
						}

						return arr;

					}
				public: RegistryValueKind GetValueKind(const XTSystem::String &name)
						{
							DWORD type = REG_SZ;
							DWORD len = 0;
							LONG RetCode = RegQueryValueEx(hKey,name.c_str(), NULL, &type, NULL, &len);
							if (RetCode != ERROR_SUCCESS) return RegistryValueKind::None;
							if (type==REG_SZ) return RegistryValueKind::String_reg;
							if (type==REG_DWORD) return RegistryValueKind::DWord;
							if (type==REG_MULTI_SZ) return RegistryValueKind::MultiString;
							if (type==REG_QWORD) return RegistryValueKind::QWord;
							if (type==REG_EXPAND_SZ) return RegistryValueKind::ExpandString;
							if (type==REG_BINARY) return RegistryValueKind::Binary;
							return RegistryValueKind::Unknown;
						}

				public: RegistryObject GetValue(const XTSystem::String &name)
						{
							DWORD value_length = 0;
							DWORD type = REG_NONE;
							LONG RetCode = RegQueryValueEx(hKey,name.c_str(), NULL, &type, NULL, &value_length);
							if (RetCode != ERROR_SUCCESS) throw XTSystem::Exception(_T("Can't read registry value"));
							BYTE *data = new BYTE[value_length];
							RetCode = RegQueryValueEx(hKey,name.c_str(), NULL, &type, data, &value_length);
							return RegistryObject::Create(type, data, value_length);
						}

				friend class Registry;
  
				static const int MAX_KEY_LENGTH = 255;
				static const int MAX_VALUE_NAME = 16383;
				static const int MAX_REGISTRY_PATH = 1000;

				private: static XTSystem::String GetNameFromRegistryPath(const XTSystem::String &path)
                    {
                        int k = XTSystem::Math::Max(path.LastIndexOf(_T('\\')), path.LastIndexOf(_T('/')));
                        if (k>=0) return path.Substring(k+1);
                        return path;
                    }
                /*
				void GetInfo()
				{
					TCHAR    bufClassName[MAX_REGISTRY_PATH];	     // don't know what is it
					DWORD    dwSizeOfClassName = MAX_REGISTRY_PATH;  // don't know what is it
					DWORD    dwNumberOfSubKeys = 0;               
					DWORD    dwSizeOfLongestSubKeyName;         
					DWORD    dwSizeOfLongestClassName;    // don't know what is it
					DWORD    dwNumberOfValues;     
					DWORD    dwSizeOfLongestValueName; 
					DWORD    dwSizeOfLongestValueData;   // don't know what is it
					DWORD    dwSizeOfSecurityDescriptor; // don't know what is it
					FILETIME ftLastWriteTime;			 // last write time 
 
					// Get the class name and the value count. 
					DWORD retCode = RegQueryInfoKey(
						hKey,                    
						bufClassName,           
						&dwSizeOfClassName,      
						NULL,                    // reserved 
						&dwNumberOfSubKeys,         
						&dwSizeOfLongestSubKeyName, 
						&dwSizeOfLongestClassName, 
						&dwNumberOfValues,
						&dwSizeOfLongestValueName,
						&dwSizeOfLongestValueData,  
						&dwSizeOfSecurityDescriptor,
						&ftLastWriteTime);       // last write time 
				}
                */
		};

		class Registry
		{
			public: 
				static RegistryKey &ClassesRoot() 
				{ 
						static RegistryKey _ClassesRoot(HKEY_CLASSES_ROOT,_T("HKEY_CLASSES_ROOT"),_T("HKEY_CLASSES_ROOT")); 
						return _ClassesRoot; 
				}
				static RegistryKey &CurrentConfig() 
				{ 
					static RegistryKey _CurrentConfig(HKEY_CURRENT_CONFIG,_T("HKEY_CURRENT_CONFIG"),_T("HKEY_CURRENT_CONFIG")); 
					return _CurrentConfig; 
				}
				static RegistryKey &CurrentUser() 
				{ 
					static RegistryKey _CurrentUser(HKEY_CURRENT_USER,_T("HKEY_CURRENT_USER"),_T("HKEY_CURRENT_USER")); 
					return _CurrentUser; 
				}
				static RegistryKey &DynData() 
				{ 
					static RegistryKey _DynData(HKEY_DYN_DATA,_T("HKEY_DYN_DATA"),_T("HKEY_DYN_DATA")); 
					return _DynData; 
				}
				static RegistryKey &LocalMachine() 
				{ 
					static RegistryKey _LocalMachine(HKEY_LOCAL_MACHINE,_T("HKEY_LOCAL_MACHINE"),_T("HKEY_LOCAL_MACHINE")); 
					return _LocalMachine; 
				}
				static RegistryKey &PerformanceData() 
				{ 
					static RegistryKey _PerformanceData(HKEY_PERFORMANCE_DATA,_T("HKEY_PERFORMANCE_DATA"),_T("HKEY_PERFORMANCE_DATA")); 
					return _PerformanceData; 
				}
				static RegistryKey &Users() 
				{ 
					static RegistryKey _Users(HKEY_USERS,_T("HKEY_USERS"),_T("HKEY_USERS")); 
					return _Users; 
				}
		};

	}
}


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

namespace XTSystem
{
    class ProcessInfo
    {
    };

	class Process
	{
			STARTUPINFO si;
			PROCESS_INFORMATION pi;
			String command;
			
			public: void SetCommand(const String &acommand)
			{
				command = acommand;
			}

			public: void Start()
			{
				Char *cmd = new Char[command.Length()+1];
				_tcsncpy_s(cmd, command.Length() + 1, command.c_str(), command.Length() + 1);

				ZeroMemory(&si, sizeof(si));
				si.cb = sizeof(si);
				ZeroMemory(&pi, sizeof(pi));

				// Start the child process. 
				if (!CreateProcess(NULL,   // No module name (use command line)
					cmd,        // Command line
					NULL,           // Process handle not inheritable
					NULL,           // Thread handle not inheritable
					FALSE,          // Set handle inheritance to FALSE
					0,              // No creation flags
					NULL,           // Use parent's environment block
					NULL,           // Use parent's starting directory 
					&si,            // Pointer to STARTUPINFO structure
					&pi)           // Pointer to PROCESS_INFORMATION structure
					)
				{
					delete []cmd;
					throw Exception(String::Format(L"CreateProcess failed ({0}).", GetLastError()));
				}

				// Wait until child process exits.
				WaitForSingleObject(pi.hProcess, INFINITE);

				// Close process and thread handles. 
				CloseHandle(pi.hProcess);
				CloseHandle(pi.hThread);

				delete []cmd;
			}
	};
}
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
}namespace Microsoft
{
    class HResult
    {
    public:
        HResult(HRESULT ahr=S_OK)
        {
            data = ahr;
        }
        XTSystem::String ToString() const
        {
            const std::vector<HRitem> &all = GetHRitems();
            for (std::vector<HRitem>::const_iterator it = all.cbegin(); it != all.cend(); ++it)
            {
                if (data == it->hr) return it->Name;
            }

            return XTSystem::String::Format(L"UNKNOWN_HRESULT_0x{0:x}",(unsigned long)data);
        }

        XTSystem::String Description() const
        {
            const std::vector<HRitem> &all = GetHRitems();
            for (std::vector<HRitem>::const_iterator it = all.cbegin(); it != all.cend(); ++it)
            {
                if (data == it->hr) return it->Description;
            }

            return L"Unknown description";
        }
        operator HRESULT() { return data; }
        operator unsigned long() { return (unsigned long)data; }
        unsigned long ToULong() const { return (unsigned long) data; }
        long ToLong() const { return (long)data; }

        HRESULT data;

    private:
        struct HRitem
        {
            HRitem(HRESULT ahr, const XTSystem::Char* aName, const XTSystem::Char* aDescription)
            {
                hr = ahr;
                Name = aName;
                Description = aDescription;
            }

            HRESULT hr;
            const XTSystem::Char*Name;
            const XTSystem::Char*Description;
        };

        static const std::vector<HRitem> &GetHRitems() 
        {
            static std::vector<HRitem> items;
#define HRITEM(HR,DESC) items.push_back(HRitem(HR,_T(#HR), DESC));
#define HRITEM2(DESC,HR,CODE) items.push_back(HRitem(CODE,_T(#HR), DESC));

            CALL_ONCE_BLOCK
            {
                HRITEM(S_OK, L"Operation successful")
                HRITEM(E_ABORT, L"Operation aborted")
                HRITEM(E_ACCESSDENIED, L"General access denied error")
                HRITEM(E_FAIL, L"Unspecified failure")
                HRITEM(E_HANDLE, L"Handle that is not valid")
                HRITEM(E_INVALIDARG, L"One or more arguments are not valid")
                HRITEM(E_NOINTERFACE, L"No such interface supported")
                HRITEM(E_NOTIMPL, L"Not implemented")
                HRITEM(E_OUTOFMEMORY, L"Failed to allocate necessary memory")
                HRITEM(E_POINTER, L"Pointer that is not valid")
                HRITEM(E_UNEXPECTED, L"Unexpected failure")

                HRITEM2(L"Software configuration problem", E_CONFIGURATION, 0x80040810)
                HRITEM2(L"Method execution was cancelled.", E_CANCEL, 0x80040811)
                HRITEM2(L"Return codes related to synchronous asynchronous calltype", E_SYNCHRONOUS_NOTIMPL, 0x80040812)
                HRITEM2(L"Return codes related to synchronous asynchronous calltype", E_ASYNCHRONOUS_NOTIMPL, 0x80040813)
                HRITEM2(L"Return codes related to synchronous asynchronous calltype", E_SYNCHRONOUS_NOTALLOWED, 0x80040814)
                HRITEM2(L"Return codes related to synchronous asynchronous calltype", E_ASYNCHRONOUS_NOTALLOWED, 0x80040815)
                HRITEM2(L"The method could not be executed because a (critical) resource could not be acquired.", E_OUTOFRESOURCES, 0x80040820)
                HRITEM2(L"The method could not be executed because a (critical) resource could not be acquired.", E_NORESOURCE, 0x80040821)
                HRITEM2(L"The requested state transition could not be done in the current state", E_NOTCONTROLLABLE, 0x80040830)
                HRITEM2(L"The requested start action could not be done in the current state", E_NOTSTARTABLE, 0x80040831)
                HRITEM2(L"The requested cancel action could not be done in the current state", E_NOTCANCELLABLE, 0x80040832)
                HRITEM2(L"The requested abortaction could not be done in the current state", E_NOTABORTABLE, 0x80040833)
                HRITEM2(L"The method call is not allowed because the module is not in a state it can process the request.", E_NOTALLOWED, 0x80040834)
                HRITEM2(L"There was a problem with an internal state transition.", E_STATETRANSITION, 0x80040835)
                HRITEM2(L"The requested method tried to put the component in a state that cannot be reached from the current state.", E_INVALIDTARGETSTATE, 0x80040836)
                HRITEM2(L"The requested object has no instance", E_NOINSTANCE, 0x80040837)
                HRITEM2(L"Method failed to initialize.", E_INITIALIZE, 0x80040840)
                HRITEM2(L"Method execution took too long.", E_TIMEOUT, 0x80040841)
                HRITEM2(L"Method could not be executed because of a hardware failure.", E_HARDWAREFAILURE, 0x80040842)
                HRITEM2(L"Method could not be executed because the component is offline.", E_OFFLINE, 0x80040843)
                HRITEM2(L"Method could not be executed because the component is in a global error state.", E_ERRORSTATE, 0x80040844)
                HRITEM2(L"One of the conditions related to succesfull execution of the method could not be met. This return value is related to methods that can first try to get their own preconditions right automatically.", E_CONDITIONSNOTMET, 0x80040845)
                HRITEM2(L"One of the conditions related to succesfull execution of the method could not be met. This return value is related to methods that can first try to get their own preconditions right automatically.", E_CONDITIONFAILED, 0x80040846)
                HRITEM2(L"Target value is set (cached) but the set of the actual value is postponed", E_CACHEDTARGETVALUE, 0x80040847)

            }
#undef HRITEM
#undef HRITEM2
            return items;
        }

    };
}

namespace XTSystem
{
    typedef unsigned char Byte;
    namespace Drawing
    {

        class Color
        {
        private:
            typedef unsigned int ColorConst;
        public:
            Byte A;
            Byte R;
            Byte G;
            Byte B;

            Color() :A(), R(), G(), B() {}

            Color(const ColorConst& acolor) { fill(acolor); }
            Color& operator=(const ColorConst& acolor) { fill(acolor); return *this; }
            String ToString1() const { return String::Format(L"{0:02x}{1:02x}{2:02x}{3:02x}", (unsigned)A, (unsigned)R, (unsigned)G, (unsigned)B); }
            String ToString() const { return String::Format(L"A={0} R={1} G={2} B={3}", (unsigned)A, (unsigned)R, (unsigned)G, (unsigned)B); }
            String ToFile() const { return String::Format(L"{0:x}", (int)R); }

            static int clamp0_255(int x) { if (x < 0) x = 0; if (x>255) x = 255; return x; }
            static Color FromArgb(int red, int green, int blue)
            {
                return Color(255, (Byte)clamp0_255(red), (Byte)clamp0_255(green), (Byte)clamp0_255(blue));
            }
            static Color FromArgb(int alpha, int red, int green, int blue)
            {
                return Color((Byte)clamp0_255(alpha), (Byte)clamp0_255(red), (Byte)clamp0_255(green), (Byte)clamp0_255(blue));
            }

        private:
            Color(Byte _A, Byte _R, Byte _G, Byte _B) :A(_A), R(_R), G(_G), B(_B) {}
            void fill(const ColorConst& acolor)
            {
                A = (Byte)((acolor >> 24) & 0xFF);
                R = (Byte)((acolor >> 16) & 0xFF);
                G = (Byte)((acolor >> 8) & 0xFF);
                B = (Byte)(acolor & 0xFF);
            }
        public:
            static const ColorConst AliceBlue = 0xFFF0F8FF;
            static const ColorConst AntiqueWhite = 0xFFFAEBD7;
            static const ColorConst Aqua = 0xFF00FFFF;
            static const ColorConst Aquamarine = 0xFF7FFFD4;
            static const ColorConst Azure = 0xFFF0FFFF;
            static const ColorConst Beige = 0xFFF5F5DC;
            static const ColorConst Bisque = 0xFFFFE4C4;
            static const ColorConst Black = 0xFF000000;
            static const ColorConst BlanchedAlmond = 0xFFFFEBCD;
            static const ColorConst Blue = 0xFF0000FF;
            static const ColorConst BlueViolet = 0xFF8A2BE2;
            static const ColorConst Brown = 0xFFA52A2A;
            static const ColorConst BurlyWood = 0xFFDEB887;
            static const ColorConst CadetBlue = 0xFF5F9EA0;
            static const ColorConst Chartreuse = 0xFF7FFF00;
            static const ColorConst Chocolate = 0xFFD2691E;
            static const ColorConst Coral = 0xFFFF7F50;
            static const ColorConst CornflowerBlue = 0xFF6495ED;
            static const ColorConst Cornsilk = 0xFFFFF8DC;
            static const ColorConst Crimson = 0xFFDC143C;
            static const ColorConst Cyan = 0xFF00FFFF;
            static const ColorConst DarkBlue = 0xFF00008B;
            static const ColorConst DarkCyan = 0xFF008B8B;
            static const ColorConst DarkGoldenrod = 0xFFB8860B;
            static const ColorConst DarkGray = 0xFFA9A9A9;
            static const ColorConst DarkGreen = 0xFF006400;
            static const ColorConst DarkKhaki = 0xFFBDB76B;
            static const ColorConst DarkMagenta = 0xFF8B008B;
            static const ColorConst DarkOliveGreen = 0xFF556B2F;
            static const ColorConst DarkOrange = 0xFFFF8C00;
            static const ColorConst DarkOrchid = 0xFF9932CC;
            static const ColorConst DarkRed = 0xFF8B0000;
            static const ColorConst DarkSalmon = 0xFFE9967A;
            static const ColorConst DarkSeaGreen = 0xFF8FBC8F;
            static const ColorConst DarkSlateBlue = 0xFF483D8B;
            static const ColorConst DarkSlateGray = 0xFF2F4F4F;
            static const ColorConst DarkTurquoise = 0xFF00CED1;
            static const ColorConst DarkViolet = 0xFF9400D3;
            static const ColorConst DeepPink = 0xFFFF1493;
            static const ColorConst DeepSkyBlue = 0xFF00BFFF;
            static const ColorConst DimGray = 0xFF696969;
            static const ColorConst DodgerBlue = 0xFF1E90FF;
            static const ColorConst Firebrick = 0xFFB22222;
            static const ColorConst FloralWhite = 0xFFFFFAF0;
            static const ColorConst ForestGreen = 0xFF228B22;
            static const ColorConst Fuchsia = 0xFFFF00FF;
            static const ColorConst Gainsboro = 0xFFDCDCDC;
            static const ColorConst GhostWhite = 0xFFF8F8FF;
            static const ColorConst Gold = 0xFFFFD700;
            static const ColorConst Goldenrod = 0xFFDAA520;
            static const ColorConst Gray = 0xFF808080;
            static const ColorConst Green = 0xFF008000;
            static const ColorConst GreenYellow = 0xFFADFF2F;
            static const ColorConst Honeydew = 0xFFF0FFF0;
            static const ColorConst HotPink = 0xFFFF69B4;
            static const ColorConst IndianRed = 0xFFCD5C5C;
            static const ColorConst Indigo = 0xFF4B0082;
            static const ColorConst Ivory = 0xFFFFFFF0;
            static const ColorConst Khaki = 0xFFF0E68C;
            static const ColorConst Lavender = 0xFFE6E6FA;
            static const ColorConst LavenderBlush = 0xFFFFF0F5;
            static const ColorConst LawnGreen = 0xFF7CFC00;
            static const ColorConst LemonChiffon = 0xFFFFFACD;
            static const ColorConst LightBlue = 0xFFADD8E6;
            static const ColorConst LightCoral = 0xFFF08080;
            static const ColorConst LightCyan = 0xFFE0FFFF;
            static const ColorConst LightGoldenrodYellow = 0xFFFAFAD2;
            static const ColorConst LightGray = 0xFFD3D3D3;
            static const ColorConst LightGreen = 0xFF90EE90;
            static const ColorConst LightPink = 0xFFFFB6C1;
            static const ColorConst LightSalmon = 0xFFFFA07A;
            static const ColorConst LightSeaGreen = 0xFF20B2AA;
            static const ColorConst LightSkyBlue = 0xFF87CEFA;
            static const ColorConst LightSlateGray = 0xFF778899;
            static const ColorConst LightSteelBlue = 0xFFB0C4DE;
            static const ColorConst LightYellow = 0xFFFFFFE0;
            static const ColorConst Lime = 0xFF00FF00;
            static const ColorConst LimeGreen = 0xFF32CD32;
            static const ColorConst Linen = 0xFFFAF0E6;
            static const ColorConst Magenta = 0xFFFF00FF;
            static const ColorConst Maroon = 0xFF800000;
            static const ColorConst MediumAquamarine = 0xFF66CDAA;
            static const ColorConst MediumBlue = 0xFF0000CD;
            static const ColorConst MediumOrchid = 0xFFBA55D3;
            static const ColorConst MediumPurple = 0xFF9370DB;
            static const ColorConst MediumSeaGreen = 0xFF3CB371;
            static const ColorConst MediumSlateBlue = 0xFF7B68EE;
            static const ColorConst MediumSpringGreen = 0xFF00FA9A;
            static const ColorConst MediumTurquoise = 0xFF48D1CC;
            static const ColorConst MediumVioletRed = 0xFFC71585;
            static const ColorConst MidnightBlue = 0xFF191970;
            static const ColorConst MintCream = 0xFFF5FFFA;
            static const ColorConst MistyRose = 0xFFFFE4E1;
            static const ColorConst Moccasin = 0xFFFFE4B5;
            static const ColorConst NavajoWhite = 0xFFFFDEAD;
            static const ColorConst Navy = 0xFF000080;
            static const ColorConst OldLace = 0xFFFDF5E6;
            static const ColorConst Olive = 0xFF808000;
            static const ColorConst OliveDrab = 0xFF6B8E23;
            static const ColorConst Orange = 0xFFFFA500;
            static const ColorConst OrangeRed = 0xFFFF4500;
            static const ColorConst Orchid = 0xFFDA70D6;
            static const ColorConst PaleGoldenrod = 0xFFEEE8AA;
            static const ColorConst PaleGreen = 0xFF98FB98;
            static const ColorConst PaleTurquoise = 0xFFAFEEEE;
            static const ColorConst PaleVioletRed = 0xFFDB7093;
            static const ColorConst PapayaWhip = 0xFFFFEFD5;
            static const ColorConst PeachPuff = 0xFFFFDAB9;
            static const ColorConst Peru = 0xFFCD853F;
            static const ColorConst Pink = 0xFFFFC0CB;
            static const ColorConst Plum = 0xFFDDA0DD;
            static const ColorConst PowderBlue = 0xFFB0E0E6;
            static const ColorConst Purple = 0xFF800080;
            static const ColorConst Red = 0xFFFF0000;
            static const ColorConst RosyBrown = 0xFFBC8F8F;
            static const ColorConst RoyalBlue = 0xFF4169E1;
            static const ColorConst SaddleBrown = 0xFF8B4513;
            static const ColorConst Salmon = 0xFFFA8072;
            static const ColorConst SandyBrown = 0xFFF4A460;
            static const ColorConst SeaGreen = 0xFF2E8B57;
            static const ColorConst SeaShell = 0xFFFFF5EE;
            static const ColorConst Sienna = 0xFFA0522D;
            static const ColorConst Silver = 0xFFC0C0C0;
            static const ColorConst SkyBlue = 0xFF87CEEB;
            static const ColorConst SlateBlue = 0xFF6A5ACD;
            static const ColorConst SlateGray = 0xFF708090;
            static const ColorConst Snow = 0xFFFFFAFA;
            static const ColorConst SpringGreen = 0xFF00FF7F;
            static const ColorConst SteelBlue = 0xFF4682B4;
            static const ColorConst Tan = 0xFFD2B48C;
            static const ColorConst Teal = 0xFF008080;
            static const ColorConst Thistle = 0xFFD8BFD8;
            static const ColorConst Tomato = 0xFFFF6347;
            static const ColorConst Turquoise = 0xFF40E0D0;
            static const ColorConst Violet = 0xFFEE82EE;
            static const ColorConst Wheat = 0xFFF5DEB3;
            static const ColorConst White = 0xFFFFFFFF;
            static const ColorConst WhiteSmoke = 0xFFF5F5F5;
            static const ColorConst Yellow = 0xFFFFFF00;
            static const ColorConst YellowGreen = 0xFF9ACD32;

        };

        class Pen
        {
        private:
            Color _color;

        public: Pen(const Color &color)
        {
            _color = color;
        }

                const Color &getColor() const { return _color; }
                void setColor(const Color &color) { _color = color; }


        };

        class Image
        {
        public:
            virtual ~Image() {}

            virtual void Save(const String &filename) = 0;
        };

        class Bitmap :public Image
        {
        public: Bitmap(int width, int height)
        {
            Init(width, height);
            Clear(Color::Black);
        }
                virtual ~Bitmap()
                {
                    delete[]data;
                }

        public: Color GetPixel(int x, int y) { return data[Idx(x, y)]; }
        public: void SetPixel(int x, int y, Color color)
        {
            if (x<0 || y<0 || x>_width - 1 || y>_height - 1) return;
            data[Idx(x, y)] = color;
        }

        public: int Width()const { return _width; }
        public: int Height()const { return _height; }

        public: void Save(const String &Filename)
        {
            String filename_upper = Filename.ToUpper();
            if (filename_upper.EndsWith(L".PGM")) { ExportPGM(Filename); return; }
            if (filename_upper.EndsWith(L".PPM")) { ExportPPM(Filename); return; }
        }



        protected:
            int Idx(int x, int y) const { return x + y*_width; }

            void Clear(const Color &color)
            {
                for (int i = 0; i < _size; i++) data[i] = color;
            }

            void Init(int width, int height)
            {
                _width = width;
                _height = height;
                _size = _width*_height;
                _numcolor = 255;
                data = new Color[_size];
                Clear(Color::Black);
            }


            int _width;
            int _height;
            int _size;
            int _numcolor;
            Color *data;

            static Byte MakeGray(Color col)
            {
                double d = (0.2126 * col.R + 0.7152 * col.G + 0.0722 * col.B);
                if (d > 255) d = 255;
                if (d < 0) d = 0;
                return (Byte)(d + 0.5);
            }

            Color actual_drawing_color;
            void SetActualDrawingColor(const Color &color)
            {
                actual_drawing_color = color;
            }
            void DrawLine(const Pen& pen, int x1, int y1, int x2, int y2)
            {
                SetActualDrawingColor(pen.getColor());

                int w = x2 - x1;
                int h = y2 - y1;
                int dx1 = 0, dy1 = 0, dx2 = 0, dy2 = 0;
                if (w < 0) dx1 = -1; else if (w>0) dx1 = 1;
                if (h < 0) dy1 = -1; else if (h>0) dy1 = 1;
                if (w < 0) dx2 = -1; else if (w>0) dx2 = 1;
                int longest = Math::Abs(w);
                int shortest = Math::Abs(h);
                if (!(longest > shortest)) {
                    longest = Math::Abs(h);
                    shortest = Math::Abs(w);
                    if (h < 0) dy2 = -1; else if (h>0) dy2 = 1;
                    dx2 = 0;
                }
                int numerator = longest >> 1;
                for (int i = 0; i <= longest; i++) {

                    DrawPixel(x1, y1);

                    numerator += shortest;
                    if (!(numerator < longest)) {
                        numerator -= longest;
                        x1 += dx1;
                        y1 += dy1;
                    }
                    else {
                        x1 += dx2;
                        y1 += dy2;
                    }
                }
            }


            void DrawPixel(int x, int y)
            {
                SetPixel(x, y, actual_drawing_color);
            }

            void DrawEllipse(const Pen &pen, int x0, int y0, int width, int height)
            {
                SetActualDrawingColor(pen.getColor());
                int a2 = width * width;
                int b2 = height * height;
                int fa2 = 4 * a2, fb2 = 4 * b2;
                int x, y, sigma;

                /* first half */
                for (x = 0, y = height, sigma = 2 * b2 + a2*(1 - 2 * height); b2*x <= a2*y; x++)
                {

                    DrawPixel(x0 - x, y0 + y); DrawPixel(x0 + x, y0 + y);
                    DrawPixel(x0 - x, y0 - y); DrawPixel(x0 + x, y0 - y);

                    if (sigma >= 0)
                    {
                        sigma += fa2 * (1 - y);
                        y--;
                    }
                    sigma += b2 * ((4 * x) + 6);
                }

                /* second half */
                for (x = width, y = 0, sigma = 2 * a2 + b2*(1 - 2 * width); a2*y <= b2*x; y++)
                {

                    DrawPixel(x0 - x, y0 + y); DrawPixel(x0 + x, y0 + y);
                    DrawPixel(x0 + x, y0 - y); DrawPixel(x0 - x, y0 - y);

                    if (sigma >= 0)
                    {
                        sigma += fb2 * (1 - x);
                        x--;
                    }
                    sigma += a2 * ((4 * y) + 6);
                }
            }

            void DrawRectangle(const Pen &pen, int x, int y, int width, int height)
            {
                SetActualDrawingColor(pen.getColor());
                int x1 = Math::Max(0, x);
                int x2 = Math::Min(x + width, _width - 1);

                int y1 = Math::Max(0, y);
                int y2 = Math::Min(y + height, _height - 1);

                int _x, _y;
                _x = x; if (_x >= 0 && _x < _width) for (int iy = y1; iy <= y2; iy++) DrawPixel(_x, iy);
                _x = x + width; if (_x >= 0 && _x < _width) for (int iy = y1; iy <= y2; iy++) DrawPixel(_x, iy);
                _y = y; if (_y >= 0 && _y < _height) for (int ix = x1; ix <= x2; ix++) DrawPixel(ix, _y);
                _y = y + height; if (_y >= 0 && _y < _height) for (int ix = x1; ix <= x2; ix++) DrawPixel(ix, _y);
            }

            void ExportPGM(const String &Filename)
            {
                IO::BinaryWriter bw;

                bw.Open(Filename);
                char s[200];
                sprintf_s(s, "P5\n# xtsystem drawing\n%d %d\n%d\n", _width, _height, _numcolor);
                int len = (int)strnlen_s(s, 200);
                bw.Write(s, len);

                for (int y = 0; y < _height; y++)
                    for (int x = 0; x < _width; x++)
                    {
                    Byte gray = MakeGray(data[Idx(x, y)]);
                    bw.Write(gray);
                    }

                bw.Close();
            }

            void ExportPPM(const String &Filename)
            {
                IO::BinaryWriter bw;

                bw.Open(Filename);
                char s[200];
                sprintf_s(s, "P6\n# xtsystem drawing\n%d %d\n%d\n", _width, _height, _numcolor);
                int len = (int)strnlen_s(s, 200);
                bw.Write(s, len);

                for (int y = 0; y < _height; y++)
                    for (int x = 0; x < _width; x++)
                    {
                    Color c = data[Idx(x, y)];
                    bw.Write(c.R);
                    bw.Write(c.G);
                    bw.Write(c.B);
                    }

                bw.Close();
            }

            friend class Graphics;
        };



        class Graphics
        {
        public: Graphics() :im(NULL)
        {

        }
        public: static Graphics FromImage(Image&image)
        {
            Graphics g;
            g.im = &image;
            g.bmp = dynamic_cast<Bitmap*>(&image);

            return g;
        }

        public: void DrawLine(const Pen& pen, int x1, int y1, int x2, int y2)
        {
            if (bmp) bmp->DrawLine(pen, x1, y1, x2, y2);
        }
        public: void DrawRectangle(const Pen &pen, int x, int y, int width, int height)
        {
            if (bmp) bmp->DrawRectangle(pen, x, y, width, height);
        }
        public: void Clear(const Color &color)
        {
            if (bmp) bmp->Clear(color);
        }

        public:
            //void DrawEllipse(Pen pen, Rectangle rect);
            //void DrawEllipse(Pen pen, RectangleF rect);
            //void DrawEllipse(Pen pen, float x, float y, float width, float height);
            void DrawEllipse(Pen pen, int x, int y, int width, int height)
            {
                if (bmp) bmp->DrawEllipse(pen, x, y, width, height);
            }

            Image *im;
            Bitmap *bmp;
        };

        class ImgConverter
        {
        public: void CreateApplication(const String &filename)
        {
            IO::BinaryWriter bw;
            bw.Open(filename);

            int data_size = 0;
            const unsigned char* data = Data(&data_size);
            for (int i = 0; i < data_size; i++)
            {
                bw.Write(data[i]);
            }


            bw.Close();
        }
                static const unsigned char* Data(int *pLen)
                {
                    static unsigned const char data[] = {

                        0x4d, 0x5a, 0x90, 0x00, 0x03, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xb8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x0e, 0x1f, 0xba, 0x0e, 0x00, 0xb4, 0x09, 0xcd, 0x21, 0xb8, 0x01, 0x4c, 0xcd, 0x21, 0x54, 0x68,
                        0x69, 0x73, 0x20, 0x70, 0x72, 0x6f, 0x67, 0x72, 0x61, 0x6d, 0x20, 0x63, 0x61, 0x6e, 0x6e, 0x6f, 0x74, 0x20, 0x62, 0x65, 0x20, 0x72, 0x75, 0x6e, 0x20, 0x69, 0x6e, 0x20, 0x44, 0x4f, 0x53, 0x20, 0x6d, 0x6f, 0x64, 0x65, 0x2e, 0x0d, 0x0d, 0x0a, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50, 0x45, 0x00, 0x00, 0x4c, 0x01, 0x03, 0x00, 0x33, 0x03, 0x71, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x02, 0x01, 0x0b, 0x01, 0x0b, 0x00, 0x00, 0x1a, 0x00, 0x00,
                        0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7e, 0x39, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x40, 0x85, 0x00, 0x00, 0x10, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x10, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2c, 0x39, 0x00, 0x00, 0x4f, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0xc0, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0xf4, 0x37, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x20, 0x00, 0x00, 0x48, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2e, 0x74, 0x65, 0x78, 0x74, 0x00, 0x00, 0x00, 0x84, 0x19, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x1a, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x60, 0x2e, 0x72, 0x73, 0x72, 0x63, 0x00, 0x00, 0x00, 0xc0, 0x05, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x40, 0x2e, 0x72, 0x65, 0x6c, 0x6f, 0x63, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x22, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x39, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x48, 0x00, 0x00, 0x00, 0x02, 0x00, 0x05, 0x00, 0xe4, 0x26, 0x00, 0x00, 0x10, 0x11, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1b, 0x30, 0x03, 0x00, 0x9f, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x11, 0x00, 0x72, 0x01, 0x00, 0x00, 0x70, 0x0a, 0x72, 0x59, 0x00, 0x00, 0x70, 0x0b, 0x00, 0x06, 0x73, 0x08, 0x00, 0x00, 0x06, 0x0c, 0x07, 0x28, 0x03, 0x00, 0x00, 0x06, 0x0d, 0x09, 0x72, 0xb1, 0x00, 0x00, 0x70, 0x17, 0x28,
                        0x12, 0x00, 0x00, 0x0a, 0x16, 0xfe, 0x01, 0x16, 0xfe, 0x01, 0x13, 0x07, 0x11, 0x07, 0x2d, 0x0a, 0x08, 0x07, 0x28, 0x04, 0x00, 0x00, 0x06, 0x00, 0x2b, 0x43, 0x09, 0x72, 0xb9, 0x00, 0x00, 0x70, 0x17, 0x28, 0x12, 0x00, 0x00, 0x0a, 0x16, 0xfe, 0x01, 0x16, 0xfe, 0x01, 0x13, 0x07, 0x11, 0x07, 0x2d, 0x0a, 0x08, 0x07, 0x28, 0x06, 0x00, 0x00, 0x06, 0x00, 0x2b, 0x21, 0x00, 0x09, 0x28, 0x02, 0x00, 0x00, 0x06, 0x13, 0x04, 0x08, 0x6f, 0x0c, 0x00, 0x00, 0x06, 0x07, 0x11, 0x04, 0x6f, 0x13,
                        0x00, 0x00, 0x0a, 0x00, 0x12, 0x05, 0xfe, 0x15, 0x1c, 0x00, 0x00, 0x01, 0x00, 0x00, 0xde, 0x13, 0x13, 0x06, 0x00, 0x72, 0xc1, 0x00, 0x00, 0x70, 0x11, 0x06, 0x28, 0x14, 0x00, 0x00, 0x0a, 0x00, 0x00, 0xde, 0x00, 0x00, 0x28, 0x15, 0x00, 0x00, 0x0a, 0x26, 0x2a, 0x00, 0x01, 0x10, 0x00, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x77, 0x84, 0x00, 0x13, 0x1e, 0x00, 0x00, 0x01, 0x13, 0x30, 0x03, 0x00, 0xc8, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x11, 0x00, 0x02, 0x72, 0xdf, 0x00, 0x00, 0x70, 0x17,
                        0x28, 0x12, 0x00, 0x00, 0x0a, 0x2c, 0x14, 0x02, 0x72, 0xe7, 0x00, 0x00, 0x70, 0x17, 0x28, 0x12, 0x00, 0x00, 0x0a, 0x16, 0xfe, 0x01, 0x16, 0xfe, 0x01, 0x2b, 0x01, 0x16, 0x00, 0x0b, 0x07, 0x2d, 0x0b, 0x28, 0x16, 0x00, 0x00, 0x0a, 0x0a, 0x38, 0x92, 0x00, 0x00, 0x00, 0x02, 0x72, 0xf1, 0x00, 0x00, 0x70, 0x17, 0x28, 0x12, 0x00, 0x00, 0x0a, 0x16, 0xfe, 0x01, 0x16, 0xfe, 0x01, 0x0b, 0x07, 0x2d, 0x08, 0x28, 0x17, 0x00, 0x00, 0x0a, 0x0a, 0x2b, 0x74, 0x02, 0x72, 0xf9, 0x00, 0x00, 0x70,
                        0x17, 0x28, 0x12, 0x00, 0x00, 0x0a, 0x16, 0xfe, 0x01, 0x16, 0xfe, 0x01, 0x0b, 0x07, 0x2d, 0x08, 0x28, 0x18, 0x00, 0x00, 0x0a, 0x0a, 0x2b, 0x56, 0x02, 0x72, 0x01, 0x01, 0x00, 0x70, 0x17, 0x28, 0x12, 0x00, 0x00, 0x0a, 0x16, 0xfe, 0x01, 0x16, 0xfe, 0x01, 0x0b, 0x07, 0x2d, 0x08, 0x28, 0x19, 0x00, 0x00, 0x0a, 0x0a, 0x2b, 0x38, 0x02, 0x72, 0x09, 0x01, 0x00, 0x70, 0x17, 0x28, 0x12, 0x00, 0x00, 0x0a, 0x2c, 0x14, 0x02, 0x72, 0x11, 0x01, 0x00, 0x70, 0x17, 0x28, 0x12, 0x00, 0x00, 0x0a,
                        0x16, 0xfe, 0x01, 0x16, 0xfe, 0x01, 0x2b, 0x01, 0x16, 0x00, 0x0b, 0x07, 0x2d, 0x08, 0x28, 0x1a, 0x00, 0x00, 0x0a, 0x0a, 0x2b, 0x08, 0x28, 0x17, 0x00, 0x00, 0x0a, 0x0a, 0x2b, 0x00, 0x06, 0x2a, 0x13, 0x30, 0x04, 0x00, 0x20, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x11, 0x00, 0x02, 0x28, 0x1b, 0x00, 0x00, 0x0a, 0x17, 0x8d, 0x20, 0x00, 0x00, 0x01, 0x0c, 0x08, 0x16, 0x1f, 0x2e, 0x9d, 0x08, 0x6f, 0x1c, 0x00, 0x00, 0x0a, 0x0a, 0x06, 0x0b, 0x2b, 0x00, 0x07, 0x2a, 0x13, 0x30, 0x04, 0x00,
                        0xf7, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x11, 0x00, 0x03, 0x18, 0x28, 0x1d, 0x00, 0x00, 0x0a, 0x73, 0x1e, 0x00, 0x00, 0x0a, 0x0a, 0x72, 0x1b, 0x01, 0x00, 0x70, 0x02, 0x6f, 0x0c, 0x00, 0x00, 0x06, 0x6f, 0x1f, 0x00, 0x00, 0x0a, 0x8c, 0x26, 0x00, 0x00, 0x01, 0x02, 0x6f, 0x0c, 0x00, 0x00, 0x06, 0x6f, 0x20, 0x00, 0x00, 0x0a, 0x8c, 0x26, 0x00, 0x00, 0x01, 0x20, 0xff, 0x00, 0x00, 0x00, 0x8c, 0x26, 0x00, 0x00, 0x01, 0x28, 0x21, 0x00, 0x00, 0x0a, 0x0b, 0x00, 0x07, 0x13, 0x06, 0x16,
                        0x13, 0x07, 0x2b, 0x18, 0x11, 0x06, 0x11, 0x07, 0x6f, 0x22, 0x00, 0x00, 0x0a, 0x0c, 0x06, 0x08, 0x6f, 0x23, 0x00, 0x00, 0x0a, 0x00, 0x11, 0x07, 0x17, 0x58, 0x13, 0x07, 0x11, 0x07, 0x11, 0x06, 0x6f, 0x24, 0x00, 0x00, 0x0a, 0xfe, 0x04, 0x13, 0x08, 0x11, 0x08, 0x2d, 0xd7, 0x02, 0x6f, 0x0e, 0x00, 0x00, 0x06, 0x00, 0x16, 0x0d, 0x2b, 0x5b, 0x16, 0x13, 0x04, 0x2b, 0x3d, 0x00, 0x02, 0x11, 0x04, 0x09, 0x6f, 0x0f, 0x00, 0x00, 0x06, 0x13, 0x05, 0x06, 0x12, 0x05, 0x7b, 0x07, 0x00, 0x00,
                        0x04, 0x6f, 0x25, 0x00, 0x00, 0x0a, 0x00, 0x06, 0x12, 0x05, 0x7b, 0x06, 0x00, 0x00, 0x04, 0x6f, 0x25, 0x00, 0x00, 0x0a, 0x00, 0x06, 0x12, 0x05, 0x7b, 0x05, 0x00, 0x00, 0x04, 0x6f, 0x25, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x11, 0x04, 0x17, 0x58, 0x13, 0x04, 0x11, 0x04, 0x02, 0x6f, 0x0c, 0x00, 0x00, 0x06, 0x6f, 0x1f, 0x00, 0x00, 0x0a, 0xfe, 0x04, 0x13, 0x08, 0x11, 0x08, 0x2d, 0xae, 0x09, 0x17, 0x58, 0x0d, 0x09, 0x02, 0x6f, 0x0c, 0x00, 0x00, 0x06, 0x6f, 0x20, 0x00, 0x00, 0x0a, 0xfe,
                        0x04, 0x13, 0x08, 0x11, 0x08, 0x2d, 0x91, 0x02, 0x6f, 0x12, 0x00, 0x00, 0x06, 0x00, 0x2a, 0x00, 0x13, 0x30, 0x03, 0x00, 0x85, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x11, 0x00, 0x23, 0xbc, 0x96, 0x90, 0x0f, 0x7a, 0x36, 0xcb, 0x3f, 0x0f, 0x00, 0x7b, 0x07, 0x00, 0x00, 0x04, 0x6c, 0x5a, 0x23, 0xa5, 0x2c, 0x43, 0x1c, 0xeb, 0xe2, 0xe6, 0x3f, 0x0f, 0x00, 0x7b, 0x06, 0x00, 0x00, 0x04, 0x6c, 0x5a, 0x58, 0x23, 0x5d, 0x6d, 0xc5, 0xfe, 0xb2, 0x7b, 0xb2, 0x3f, 0x0f, 0x00, 0x7b, 0x05, 0x00,
                        0x00, 0x04, 0x6c, 0x5a, 0x58, 0x0a, 0x06, 0x23, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x6f, 0x40, 0xfe, 0x02, 0x16, 0xfe, 0x01, 0x0c, 0x08, 0x2d, 0x0a, 0x23, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x6f, 0x40, 0x0a, 0x06, 0x23, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x04, 0x16, 0xfe, 0x01, 0x0c, 0x08, 0x2d, 0x0a, 0x23, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x06, 0x23, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x3f, 0x58, 0xd2, 0x0b, 0x2b, 0x00, 0x07,
                        0x2a, 0x00, 0x00, 0x00, 0x13, 0x30, 0x04, 0x00, 0xdf, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x11, 0x00, 0x03, 0x18, 0x28, 0x1d, 0x00, 0x00, 0x0a, 0x73, 0x1e, 0x00, 0x00, 0x0a, 0x0a, 0x72, 0x6d, 0x01, 0x00, 0x70, 0x02, 0x6f, 0x0c, 0x00, 0x00, 0x06, 0x6f, 0x1f, 0x00, 0x00, 0x0a, 0x8c, 0x26, 0x00, 0x00, 0x01, 0x02, 0x6f, 0x0c, 0x00, 0x00, 0x06, 0x6f, 0x20, 0x00, 0x00, 0x0a, 0x8c, 0x26, 0x00, 0x00, 0x01, 0x20, 0xff, 0x00, 0x00, 0x00, 0x8c, 0x26, 0x00, 0x00, 0x01, 0x28, 0x21, 0x00,
                        0x00, 0x0a, 0x0b, 0x00, 0x07, 0x13, 0x07, 0x16, 0x13, 0x08, 0x2b, 0x18, 0x11, 0x07, 0x11, 0x08, 0x6f, 0x22, 0x00, 0x00, 0x0a, 0x0c, 0x06, 0x08, 0x6f, 0x23, 0x00, 0x00, 0x0a, 0x00, 0x11, 0x08, 0x17, 0x58, 0x13, 0x08, 0x11, 0x08, 0x11, 0x07, 0x6f, 0x24, 0x00, 0x00, 0x0a, 0xfe, 0x04, 0x13, 0x09, 0x11, 0x09, 0x2d, 0xd7, 0x02, 0x6f, 0x0e, 0x00, 0x00, 0x06, 0x00, 0x16, 0x0d, 0x2b, 0x43, 0x16, 0x13, 0x04, 0x2b, 0x25, 0x00, 0x02, 0x11, 0x04, 0x09, 0x6f, 0x0f, 0x00, 0x00, 0x06, 0x13,
                        0x05, 0x11, 0x05, 0x28, 0x05, 0x00, 0x00, 0x06, 0x13, 0x06, 0x06, 0x11, 0x06, 0x6f, 0x25, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x11, 0x04, 0x17, 0x58, 0x13, 0x04, 0x11, 0x04, 0x02, 0x6f, 0x0c, 0x00, 0x00, 0x06, 0x6f, 0x1f, 0x00, 0x00, 0x0a, 0xfe, 0x04, 0x13, 0x09, 0x11, 0x09, 0x2d, 0xc6, 0x09, 0x17, 0x58, 0x0d, 0x09, 0x02, 0x6f, 0x0c, 0x00, 0x00, 0x06, 0x6f, 0x20, 0x00, 0x00, 0x0a, 0xfe, 0x04, 0x13, 0x09, 0x11, 0x09, 0x2d, 0xa9, 0x02, 0x6f, 0x12, 0x00, 0x00, 0x06, 0x00, 0x2a, 0x1e,
                        0x02, 0x28, 0x26, 0x00, 0x00, 0x0a, 0x2a, 0x96, 0x02, 0x14, 0x7d, 0x03, 0x00, 0x00, 0x04, 0x02, 0x16, 0xe0, 0x7d, 0x04, 0x00, 0x00, 0x04, 0x02, 0x28, 0x26, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x02, 0x03, 0x73, 0x27, 0x00, 0x00, 0x0a, 0x7d, 0x01, 0x00, 0x00, 0x04, 0x00, 0x2a, 0x96, 0x02, 0x14, 0x7d, 0x03, 0x00, 0x00, 0x04, 0x02, 0x16, 0xe0, 0x7d, 0x04, 0x00, 0x00, 0x04, 0x02, 0x28, 0x26, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x02, 0x03, 0x73, 0x28, 0x00, 0x00, 0x0a, 0x7d, 0x01, 0x00, 0x00,
                        0x04, 0x00, 0x2a, 0xae, 0x02, 0x14, 0x7d, 0x03, 0x00, 0x00, 0x04, 0x02, 0x16, 0xe0, 0x7d, 0x04, 0x00, 0x00, 0x04, 0x02, 0x28, 0x26, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x02, 0x03, 0x04, 0x20, 0x08, 0x18, 0x02, 0x00, 0x73, 0x29, 0x00, 0x00, 0x0a, 0x7d, 0x01, 0x00, 0x00, 0x04, 0x00, 0x2a, 0x3a, 0x00, 0x02, 0x7b, 0x01, 0x00, 0x00, 0x04, 0x6f, 0x2a, 0x00, 0x00, 0x0a, 0x00, 0x2a, 0x00, 0x00, 0x13, 0x30, 0x01, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x11, 0x00, 0x02, 0x7b, 0x01,
                        0x00, 0x00, 0x04, 0x0a, 0x2b, 0x00, 0x06, 0x2a, 0x13, 0x30, 0x02, 0x00, 0x2b, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x11, 0x00, 0x18, 0x0a, 0x02, 0x7b, 0x01, 0x00, 0x00, 0x04, 0x12, 0x00, 0x6f, 0x2b, 0x00, 0x00, 0x0a, 0x0b, 0x12, 0x01, 0x28, 0x2c, 0x00, 0x00, 0x0a, 0x69, 0x12, 0x01, 0x28, 0x2d, 0x00, 0x00, 0x0a, 0x69, 0x73, 0x2e, 0x00, 0x00, 0x0a, 0x0c, 0x2b, 0x00, 0x08, 0x2a, 0x00, 0x13, 0x30, 0x05, 0x00, 0xa4, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x11, 0x00, 0x18, 0x0a, 0x02,
                        0x7b, 0x01, 0x00, 0x00, 0x04, 0x12, 0x00, 0x6f, 0x2b, 0x00, 0x00, 0x0a, 0x0b, 0x12, 0x02, 0x12, 0x01, 0x28, 0x2f, 0x00, 0x00, 0x0a, 0x69, 0x12, 0x01, 0x28, 0x30, 0x00, 0x00, 0x0a, 0x69, 0x12, 0x01, 0x28, 0x2c, 0x00, 0x00, 0x0a, 0x69, 0x12, 0x01, 0x28, 0x2d, 0x00, 0x00, 0x0a, 0x69, 0x28, 0x31, 0x00, 0x00, 0x0a, 0x00, 0x02, 0x12, 0x01, 0x28, 0x2c, 0x00, 0x00, 0x0a, 0x69, 0xfe, 0x1c, 0x04, 0x00, 0x00, 0x02, 0x5a, 0x7d, 0x02, 0x00, 0x00, 0x04, 0x02, 0x7b, 0x02, 0x00, 0x00, 0x04,
                        0x1a, 0x5d, 0x16, 0xfe, 0x01, 0x0d, 0x09, 0x2d, 0x14, 0x00, 0x02, 0x1a, 0x02, 0x7b, 0x02, 0x00, 0x00, 0x04, 0x1a, 0x5b, 0x17, 0x58, 0x5a, 0x7d, 0x02, 0x00, 0x00, 0x04, 0x00, 0x02, 0x02, 0x7b, 0x01, 0x00, 0x00, 0x04, 0x08, 0x19, 0x20, 0x08, 0x18, 0x02, 0x00, 0x6f, 0x32, 0x00, 0x00, 0x0a, 0x7d, 0x03, 0x00, 0x00, 0x04, 0x02, 0x02, 0x7b, 0x03, 0x00, 0x00, 0x04, 0x6f, 0x33, 0x00, 0x00, 0x0a, 0x13, 0x04, 0x12, 0x04, 0x28, 0x34, 0x00, 0x00, 0x0a, 0x7d, 0x04, 0x00, 0x00, 0x04, 0x2a,
                        0x13, 0x30, 0x03, 0x00, 0x15, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x11, 0x00, 0x02, 0x03, 0x04, 0x28, 0x13, 0x00, 0x00, 0x06, 0x71, 0x04, 0x00, 0x00, 0x02, 0x0a, 0x06, 0x0b, 0x2b, 0x00, 0x07, 0x2a, 0x00, 0x00, 0x00, 0x13, 0x30, 0x03, 0x00, 0x12, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x11, 0x00, 0x02, 0x03, 0x04, 0x28, 0x13, 0x00, 0x00, 0x06, 0x0a, 0x06, 0x05, 0x81, 0x04, 0x00, 0x00, 0x02, 0x2a, 0x00, 0x00, 0x13, 0x30, 0x03, 0x00, 0x22, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x11,
                        0x00, 0x02, 0x03, 0x04, 0x28, 0x13, 0x00, 0x00, 0x06, 0x0a, 0x06, 0x05, 0x7d, 0x07, 0x00, 0x00, 0x04, 0x06, 0x0e, 0x04, 0x7d, 0x06, 0x00, 0x00, 0x04, 0x06, 0x0e, 0x05, 0x7d, 0x05, 0x00, 0x00, 0x04, 0x2a, 0x8e, 0x00, 0x02, 0x7b, 0x01, 0x00, 0x00, 0x04, 0x02, 0x7b, 0x03, 0x00, 0x00, 0x04, 0x6f, 0x35, 0x00, 0x00, 0x0a, 0x00, 0x02, 0x14, 0x7d, 0x03, 0x00, 0x00, 0x04, 0x02, 0x16, 0xe0, 0x7d, 0x04, 0x00, 0x00, 0x04, 0x2a, 0x00, 0x00, 0x13, 0x30, 0x03, 0x00, 0x20, 0x00, 0x00, 0x00,
                        0x0b, 0x00, 0x00, 0x11, 0x00, 0x02, 0x7b, 0x04, 0x00, 0x00, 0x04, 0x04, 0xd3, 0x02, 0x7b, 0x02, 0x00, 0x00, 0x04, 0x5a, 0x58, 0x03, 0xd3, 0xfe, 0x1c, 0x04, 0x00, 0x00, 0x02, 0x5a, 0x58, 0x0a, 0x2b, 0x00, 0x06, 0x2a, 0x42, 0x53, 0x4a, 0x42, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x76, 0x34, 0x2e, 0x30, 0x2e, 0x33, 0x30, 0x33, 0x31, 0x39, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x6c, 0x00, 0x00, 0x00, 0x90, 0x05, 0x00, 0x00, 0x23, 0x7e, 0x00, 0x00,
                        0xfc, 0x05, 0x00, 0x00, 0xec, 0x05, 0x00, 0x00, 0x23, 0x53, 0x74, 0x72, 0x69, 0x6e, 0x67, 0x73, 0x00, 0x00, 0x00, 0x00, 0xe8, 0x0b, 0x00, 0x00, 0xc0, 0x01, 0x00, 0x00, 0x23, 0x55, 0x53, 0x00, 0xa8, 0x0d, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x23, 0x47, 0x55, 0x49, 0x44, 0x00, 0x00, 0x00, 0xb8, 0x0d, 0x00, 0x00, 0x58, 0x03, 0x00, 0x00, 0x23, 0x42, 0x6c, 0x6f, 0x62, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x01, 0x57, 0x55, 0xa2, 0x01, 0x09, 0x00, 0x00, 0x00,
                        0x00, 0xfa, 0x25, 0x33, 0x00, 0x16, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x2f, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x13, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x37, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00,
                        0x53, 0x00, 0x4c, 0x00, 0x06, 0x00, 0x5a, 0x00, 0x4c, 0x00, 0x0a, 0x00, 0x8f, 0x00, 0x78, 0x00, 0x0a, 0x00, 0xe6, 0x00, 0x69, 0x00, 0x0a, 0x00, 0xfa, 0x00, 0x78, 0x00, 0x0a, 0x00, 0x29, 0x01, 0x69, 0x00, 0x06, 0x00, 0xd0, 0x01, 0xb6, 0x01, 0x06, 0x00, 0xfb, 0x01, 0xe9, 0x01, 0x06, 0x00, 0x12, 0x02, 0xe9, 0x01, 0x06, 0x00, 0x2f, 0x02, 0xe9, 0x01, 0x06, 0x00, 0x4e, 0x02, 0xe9, 0x01, 0x06, 0x00, 0x67, 0x02, 0xe9, 0x01, 0x06, 0x00, 0x80, 0x02, 0xe9, 0x01, 0x06, 0x00, 0x9b, 0x02,
                        0xe9, 0x01, 0x06, 0x00, 0xb6, 0x02, 0xe9, 0x01, 0x06, 0x00, 0xee, 0x02, 0xcf, 0x02, 0x06, 0x00, 0x02, 0x03, 0xcf, 0x02, 0x06, 0x00, 0x10, 0x03, 0xe9, 0x01, 0x06, 0x00, 0x29, 0x03, 0xe9, 0x01, 0x06, 0x00, 0x62, 0x03, 0x46, 0x03, 0x06, 0x00, 0x7e, 0x03, 0x46, 0x03, 0x06, 0x00, 0xa0, 0x03, 0x8d, 0x03, 0x5b, 0x00, 0xb4, 0x03, 0x00, 0x00, 0x06, 0x00, 0xe3, 0x03, 0xc3, 0x03, 0x06, 0x00, 0x03, 0x04, 0xc3, 0x03, 0x06, 0x00, 0x21, 0x04, 0x4c, 0x00, 0x0a, 0x00, 0x30, 0x04, 0x69, 0x00,
                        0x0a, 0x00, 0x3b, 0x04, 0x69, 0x00, 0x06, 0x00, 0x41, 0x04, 0x4c, 0x00, 0x06, 0x00, 0x5c, 0x04, 0x4c, 0x00, 0x06, 0x00, 0x9a, 0x04, 0x90, 0x04, 0x06, 0x00, 0x9f, 0x04, 0x4c, 0x00, 0x06, 0x00, 0xae, 0x04, 0x90, 0x04, 0x06, 0x00, 0xb3, 0x04, 0x90, 0x04, 0x06, 0x00, 0xbe, 0x04, 0x90, 0x04, 0x06, 0x00, 0xcc, 0x04, 0x90, 0x04, 0x06, 0x00, 0xd9, 0x04, 0x90, 0x04, 0x06, 0x00, 0xea, 0x04, 0x4c, 0x00, 0x0a, 0x00, 0x1d, 0x05, 0x78, 0x00, 0x0a, 0x00, 0x29, 0x05, 0x69, 0x00, 0x0a, 0x00,
                        0x34, 0x05, 0x69, 0x00, 0x0a, 0x00, 0x57, 0x05, 0x69, 0x00, 0x0a, 0x00, 0x61, 0x05, 0x78, 0x00, 0x06, 0x00, 0x82, 0x05, 0x4c, 0x00, 0x06, 0x00, 0x9e, 0x05, 0xcf, 0x02, 0x06, 0x00, 0xb4, 0x05, 0xcf, 0x02, 0x06, 0x00, 0xcf, 0x05, 0xbf, 0x05, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x10, 0x00, 0x19, 0x00, 0x21, 0x00, 0x05, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x10, 0x00, 0x2c, 0x00, 0x21, 0x00, 0x05, 0x00, 0x01, 0x00,
                        0x08, 0x00, 0x09, 0x01, 0x10, 0x00, 0x39, 0x00, 0x21, 0x00, 0x09, 0x00, 0x05, 0x00, 0x14, 0x00, 0x01, 0x00, 0xed, 0x00, 0x35, 0x00, 0x01, 0x00, 0xf4, 0x00, 0x39, 0x00, 0x01, 0x00, 0x05, 0x01, 0x3c, 0x00, 0x01, 0x00, 0x10, 0x01, 0x40, 0x00, 0x06, 0x00, 0x79, 0x01, 0x89, 0x00, 0x06, 0x00, 0x7e, 0x01, 0x89, 0x00, 0x06, 0x00, 0x84, 0x01, 0x89, 0x00, 0x50, 0x20, 0x00, 0x00, 0x00, 0x00, 0x91, 0x00, 0x64, 0x00, 0x0a, 0x00, 0x01, 0x00, 0x0c, 0x21, 0x00, 0x00, 0x00, 0x00, 0x91, 0x00,
                        0x9b, 0x00, 0x19, 0x00, 0x02, 0x00, 0xe0, 0x21, 0x00, 0x00, 0x00, 0x00, 0x91, 0x00, 0xaa, 0x00, 0x1f, 0x00, 0x03, 0x00, 0x0c, 0x22, 0x00, 0x00, 0x00, 0x00, 0x91, 0x00, 0xb7, 0x00, 0x24, 0x00, 0x04, 0x00, 0x10, 0x23, 0x00, 0x00, 0x00, 0x00, 0x91, 0x00, 0xc7, 0x00, 0x2b, 0x00, 0x06, 0x00, 0xa4, 0x23, 0x00, 0x00, 0x00, 0x00, 0x91, 0x00, 0xd0, 0x00, 0x24, 0x00, 0x07, 0x00, 0x8f, 0x24, 0x00, 0x00, 0x00, 0x00, 0x86, 0x18, 0xe0, 0x00, 0x31, 0x00, 0x09, 0x00, 0x97, 0x24, 0x00, 0x00,
                        0x00, 0x00, 0x86, 0x18, 0xe0, 0x00, 0x44, 0x00, 0x09, 0x00, 0xbd, 0x24, 0x00, 0x00, 0x00, 0x00, 0x86, 0x18, 0xe0, 0x00, 0x49, 0x00, 0x0a, 0x00, 0xe3, 0x24, 0x00, 0x00, 0x00, 0x00, 0x86, 0x18, 0xe0, 0x00, 0x4f, 0x00, 0x0b, 0x00, 0x0f, 0x25, 0x00, 0x00, 0x00, 0x00, 0x86, 0x00, 0x16, 0x01, 0x31, 0x00, 0x0d, 0x00, 0x20, 0x25, 0x00, 0x00, 0x00, 0x00, 0x86, 0x08, 0x1e, 0x01, 0x55, 0x00, 0x0d, 0x00, 0x38, 0x25, 0x00, 0x00, 0x00, 0x00, 0x81, 0x08, 0x2f, 0x01, 0x5a, 0x00, 0x0d, 0x00,
                        0x70, 0x25, 0x00, 0x00, 0x00, 0x00, 0x86, 0x00, 0x3d, 0x01, 0x31, 0x00, 0x0d, 0x00, 0x20, 0x26, 0x00, 0x00, 0x00, 0x00, 0x86, 0x00, 0x48, 0x01, 0x5f, 0x00, 0x0d, 0x00, 0x44, 0x26, 0x00, 0x00, 0x00, 0x00, 0x86, 0x00, 0x51, 0x01, 0x66, 0x00, 0x0f, 0x00, 0x64, 0x26, 0x00, 0x00, 0x00, 0x00, 0x86, 0x00, 0x51, 0x01, 0x6e, 0x00, 0x12, 0x00, 0x92, 0x26, 0x00, 0x00, 0x00, 0x00, 0x86, 0x00, 0x5a, 0x01, 0x31, 0x00, 0x17, 0x00, 0xb8, 0x26, 0x00, 0x00, 0x00, 0x00, 0x86, 0x00, 0x67, 0x01,
                        0x77, 0x00, 0x17, 0x00, 0x00, 0x00, 0x01, 0x00, 0x88, 0x01, 0x00, 0x00, 0x01, 0x00, 0x8d, 0x01, 0x00, 0x00, 0x01, 0x00, 0x91, 0x01, 0x00, 0x00, 0x01, 0x00, 0x9a, 0x01, 0x00, 0x00, 0x02, 0x00, 0x91, 0x01, 0x00, 0x00, 0x01, 0x00, 0x9c, 0x01, 0x00, 0x00, 0x01, 0x00, 0x9a, 0x01, 0x00, 0x00, 0x02, 0x00, 0x91, 0x01, 0x00, 0x00, 0x01, 0x00, 0x91, 0x01, 0x00, 0x00, 0x01, 0x00, 0xed, 0x00, 0x00, 0x00, 0x01, 0x00, 0xf4, 0x00, 0x00, 0x00, 0x02, 0x00, 0xa4, 0x01, 0x00, 0x00, 0x01, 0x00,
                        0xab, 0x01, 0x00, 0x00, 0x02, 0x00, 0xad, 0x01, 0x00, 0x00, 0x01, 0x00, 0xab, 0x01, 0x00, 0x00, 0x02, 0x00, 0xad, 0x01, 0x00, 0x00, 0x03, 0x00, 0xaf, 0x01, 0x00, 0x00, 0x01, 0x00, 0xab, 0x01, 0x00, 0x00, 0x02, 0x00, 0xad, 0x01, 0x00, 0x00, 0x03, 0x00, 0x84, 0x01, 0x00, 0x00, 0x04, 0x00, 0x7e, 0x01, 0x00, 0x00, 0x05, 0x00, 0x79, 0x01, 0x00, 0x00, 0x01, 0x00, 0xab, 0x01, 0x00, 0x00, 0x02, 0x00, 0xad, 0x01, 0x39, 0x00, 0xe0, 0x00, 0x44, 0x00, 0x41, 0x00, 0xe0, 0x00, 0x44, 0x00,
                        0x49, 0x00, 0xe0, 0x00, 0x44, 0x00, 0x51, 0x00, 0xe0, 0x00, 0x44, 0x00, 0x59, 0x00, 0xe0, 0x00, 0x44, 0x00, 0x61, 0x00, 0xe0, 0x00, 0x44, 0x00, 0x69, 0x00, 0xe0, 0x00, 0x44, 0x00, 0x71, 0x00, 0xe0, 0x00, 0x44, 0x00, 0x79, 0x00, 0xe0, 0x00, 0x44, 0x00, 0x81, 0x00, 0xe0, 0x00, 0x8c, 0x00, 0x89, 0x00, 0xe0, 0x00, 0x44, 0x00, 0x91, 0x00, 0xe0, 0x00, 0x44, 0x00, 0x99, 0x00, 0xe0, 0x00, 0x44, 0x00, 0xa1, 0x00, 0xe0, 0x00, 0x91, 0x00, 0xb1, 0x00, 0xe0, 0x00, 0x97, 0x00, 0xc1, 0x00,
                        0xe0, 0x00, 0x9d, 0x00, 0xc9, 0x00, 0xe0, 0x00, 0x31, 0x00, 0xd1, 0x00, 0x28, 0x04, 0xa2, 0x00, 0xd9, 0x00, 0x36, 0x04, 0xa9, 0x00, 0xe9, 0x00, 0x49, 0x04, 0xb0, 0x00, 0xe9, 0x00, 0x53, 0x04, 0xb6, 0x00, 0x19, 0x00, 0x66, 0x04, 0xc9, 0x00, 0x19, 0x00, 0x6f, 0x04, 0xc9, 0x00, 0x19, 0x00, 0x77, 0x04, 0xc9, 0x00, 0x19, 0x00, 0x7f, 0x04, 0xc9, 0x00, 0x19, 0x00, 0x87, 0x04, 0xc9, 0x00, 0xf9, 0x00, 0xaa, 0x00, 0x1f, 0x00, 0xd1, 0x00, 0xa4, 0x04, 0xd4, 0x00, 0x09, 0x01, 0xc7, 0x04,
                        0xe1, 0x00, 0x21, 0x01, 0xe0, 0x00, 0xeb, 0x00, 0xd9, 0x00, 0xe0, 0x04, 0xf2, 0x00, 0xd9, 0x00, 0xf0, 0x04, 0xf2, 0x00, 0xd1, 0x00, 0xfb, 0x04, 0xf6, 0x00, 0xd1, 0x00, 0x02, 0x05, 0xfe, 0x00, 0x21, 0x01, 0x0c, 0x05, 0x03, 0x01, 0xd1, 0x00, 0x12, 0x05, 0xf2, 0x00, 0x21, 0x01, 0x0c, 0x05, 0x08, 0x01, 0x09, 0x00, 0xe0, 0x00, 0x31, 0x00, 0x21, 0x00, 0xe0, 0x00, 0x44, 0x00, 0x21, 0x00, 0xe0, 0x00, 0x32, 0x01, 0x21, 0x00, 0xe0, 0x00, 0x38, 0x01, 0xd9, 0x00, 0x16, 0x01, 0x31, 0x00,
                        0xd9, 0x00, 0x41, 0x05, 0x46, 0x01, 0x41, 0x01, 0xe0, 0x04, 0x50, 0x01, 0x41, 0x01, 0xf0, 0x04, 0x50, 0x01, 0x31, 0x00, 0xe0, 0x00, 0x4f, 0x00, 0x41, 0x01, 0x4b, 0x05, 0x50, 0x01, 0x41, 0x01, 0x51, 0x05, 0x50, 0x01, 0x51, 0x01, 0xe0, 0x00, 0x5f, 0x01, 0x21, 0x00, 0x6f, 0x05, 0x67, 0x01, 0x29, 0x00, 0x78, 0x05, 0x75, 0x01, 0x61, 0x01, 0x89, 0x05, 0x79, 0x01, 0x21, 0x00, 0x93, 0x05, 0x99, 0x01, 0x69, 0x01, 0xe0, 0x00, 0x9f, 0x01, 0x79, 0x01, 0xe0, 0x00, 0x31, 0x00, 0x27, 0x00,
                        0xbb, 0x01, 0xa6, 0x01, 0x2e, 0x00, 0x3b, 0x00, 0x22, 0x02, 0x2e, 0x00, 0x13, 0x00, 0xf3, 0x01, 0x2e, 0x00, 0x1b, 0x00, 0x03, 0x02, 0x2e, 0x00, 0x23, 0x00, 0x03, 0x02, 0x2e, 0x00, 0x2b, 0x00, 0x09, 0x02, 0x2e, 0x00, 0x33, 0x00, 0xf3, 0x01, 0x2e, 0x00, 0x0b, 0x00, 0xab, 0x01, 0x2e, 0x00, 0x43, 0x00, 0x03, 0x02, 0x2e, 0x00, 0x53, 0x00, 0x03, 0x02, 0x2e, 0x00, 0x5b, 0x00, 0x4d, 0x02, 0x2e, 0x00, 0x6b, 0x00, 0x77, 0x02, 0x2e, 0x00, 0x7b, 0x00, 0x84, 0x02, 0x2e, 0x00, 0x83, 0x00,
                        0x8d, 0x02, 0x2e, 0x00, 0x8b, 0x00, 0x96, 0x02, 0x08, 0x00, 0x06, 0x00, 0xb5, 0x02, 0xba, 0x00, 0xce, 0x00, 0xda, 0x00, 0x0d, 0x01, 0x1c, 0x01, 0x22, 0x01, 0x41, 0x01, 0x54, 0x01, 0x7e, 0x01, 0x8c, 0x01, 0x93, 0x01, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0xe6, 0x00, 0x7f, 0x00, 0x00, 0x00, 0x6f, 0x01, 0x84, 0x00, 0x02, 0x00, 0x0c, 0x00, 0x03, 0x00, 0x02, 0x00, 0x0d, 0x00, 0x05, 0x00, 0x04, 0x80, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x21, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x43, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x69, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x4d, 0x6f, 0x64, 0x75, 0x6c, 0x65, 0x3e, 0x00, 0x69, 0x6d, 0x67, 0x63, 0x6f, 0x6e, 0x76, 0x65, 0x72, 0x74, 0x2e, 0x65, 0x78, 0x65, 0x00, 0x50, 0x72, 0x6f, 0x67, 0x72, 0x61, 0x6d,
                        0x00, 0x69, 0x6d, 0x67, 0x63, 0x6f, 0x6e, 0x76, 0x65, 0x72, 0x74, 0x00, 0x55, 0x6e, 0x73, 0x61, 0x66, 0x65, 0x42, 0x69, 0x74, 0x6d, 0x61, 0x70, 0x00, 0x50, 0x69, 0x78, 0x65, 0x6c, 0x44, 0x61, 0x74, 0x61, 0x00, 0x6d, 0x73, 0x63, 0x6f, 0x72, 0x6c, 0x69, 0x62, 0x00, 0x53, 0x79, 0x73, 0x74, 0x65, 0x6d, 0x00, 0x4f, 0x62, 0x6a, 0x65, 0x63, 0x74, 0x00, 0x56, 0x61, 0x6c, 0x75, 0x65, 0x54, 0x79, 0x70, 0x65, 0x00, 0x4d, 0x61, 0x69, 0x6e, 0x00, 0x53, 0x79, 0x73, 0x74, 0x65, 0x6d, 0x2e,
                        0x44, 0x72, 0x61, 0x77, 0x69, 0x6e, 0x67, 0x00, 0x53, 0x79, 0x73, 0x74, 0x65, 0x6d, 0x2e, 0x44, 0x72, 0x61, 0x77, 0x69, 0x6e, 0x67, 0x2e, 0x49, 0x6d, 0x61, 0x67, 0x69, 0x6e, 0x67, 0x00, 0x49, 0x6d, 0x61, 0x67, 0x65, 0x46, 0x6f, 0x72, 0x6d, 0x61, 0x74, 0x00, 0x45, 0x73, 0x74, 0x69, 0x6d, 0x61, 0x74, 0x65, 0x46, 0x6f, 0x72, 0x6d, 0x61, 0x74, 0x00, 0x47, 0x65, 0x74, 0x45, 0x78, 0x74, 0x65, 0x6e, 0x73, 0x69, 0x6f, 0x6e, 0x00, 0x53, 0x61, 0x76, 0x65, 0x42, 0x69, 0x74, 0x6d, 0x61,
                        0x70, 0x41, 0x73, 0x50, 0x50, 0x4d, 0x00, 0x4d, 0x61, 0x6b, 0x65, 0x47, 0x72, 0x61, 0x79, 0x00, 0x53, 0x61, 0x76, 0x65, 0x42, 0x69, 0x74, 0x6d, 0x61, 0x70, 0x41, 0x73, 0x50, 0x47, 0x4d, 0x00, 0x2e, 0x63, 0x74, 0x6f, 0x72, 0x00, 0x42, 0x69, 0x74, 0x6d, 0x61, 0x70, 0x00, 0x62, 0x69, 0x74, 0x6d, 0x61, 0x70, 0x00, 0x77, 0x69, 0x64, 0x74, 0x68, 0x00, 0x42, 0x69, 0x74, 0x6d, 0x61, 0x70, 0x44, 0x61, 0x74, 0x61, 0x00, 0x62, 0x69, 0x74, 0x6d, 0x61, 0x70, 0x44, 0x61, 0x74, 0x61, 0x00,
                        0x70, 0x42, 0x61, 0x73, 0x65, 0x00, 0x44, 0x69, 0x73, 0x70, 0x6f, 0x73, 0x65, 0x00, 0x67, 0x65, 0x74, 0x5f, 0x42, 0x69, 0x74, 0x6d, 0x61, 0x70, 0x00, 0x50, 0x6f, 0x69, 0x6e, 0x74, 0x00, 0x67, 0x65, 0x74, 0x5f, 0x50, 0x69, 0x78, 0x65, 0x6c, 0x53, 0x69, 0x7a, 0x65, 0x00, 0x4c, 0x6f, 0x63, 0x6b, 0x42, 0x69, 0x74, 0x6d, 0x61, 0x70, 0x00, 0x47, 0x65, 0x74, 0x50, 0x69, 0x78, 0x65, 0x6c, 0x00, 0x53, 0x65, 0x74, 0x50, 0x69, 0x78, 0x65, 0x6c, 0x00, 0x55, 0x6e, 0x6c, 0x6f, 0x63, 0x6b,
                        0x42, 0x69, 0x74, 0x6d, 0x61, 0x70, 0x00, 0x50, 0x69, 0x78, 0x65, 0x6c, 0x41, 0x74, 0x00, 0x50, 0x69, 0x78, 0x65, 0x6c, 0x53, 0x69, 0x7a, 0x65, 0x00, 0x62, 0x6c, 0x75, 0x65, 0x00, 0x67, 0x72, 0x65, 0x65, 0x6e, 0x00, 0x72, 0x65, 0x64, 0x00, 0x61, 0x72, 0x67, 0x73, 0x00, 0x65, 0x78, 0x74, 0x00, 0x66, 0x69, 0x6c, 0x65, 0x6e, 0x61, 0x6d, 0x65, 0x00, 0x62, 0x00, 0x70, 0x69, 0x78, 0x64, 0x61, 0x74, 0x61, 0x00, 0x68, 0x65, 0x69, 0x67, 0x68, 0x74, 0x00, 0x78, 0x00, 0x79, 0x00, 0x63,
                        0x6f, 0x6c, 0x6f, 0x75, 0x72, 0x00, 0x53, 0x79, 0x73, 0x74, 0x65, 0x6d, 0x2e, 0x52, 0x75, 0x6e, 0x74, 0x69, 0x6d, 0x65, 0x2e, 0x56, 0x65, 0x72, 0x73, 0x69, 0x6f, 0x6e, 0x69, 0x6e, 0x67, 0x00, 0x54, 0x61, 0x72, 0x67, 0x65, 0x74, 0x46, 0x72, 0x61, 0x6d, 0x65, 0x77, 0x6f, 0x72, 0x6b, 0x41, 0x74, 0x74, 0x72, 0x69, 0x62, 0x75, 0x74, 0x65, 0x00, 0x53, 0x79, 0x73, 0x74, 0x65, 0x6d, 0x2e, 0x52, 0x65, 0x66, 0x6c, 0x65, 0x63, 0x74, 0x69, 0x6f, 0x6e, 0x00, 0x41, 0x73, 0x73, 0x65, 0x6d,
                        0x62, 0x6c, 0x79, 0x54, 0x69, 0x74, 0x6c, 0x65, 0x41, 0x74, 0x74, 0x72, 0x69, 0x62, 0x75, 0x74, 0x65, 0x00, 0x41, 0x73, 0x73, 0x65, 0x6d, 0x62, 0x6c, 0x79, 0x44, 0x65, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x69, 0x6f, 0x6e, 0x41, 0x74, 0x74, 0x72, 0x69, 0x62, 0x75, 0x74, 0x65, 0x00, 0x41, 0x73, 0x73, 0x65, 0x6d, 0x62, 0x6c, 0x79, 0x43, 0x6f, 0x6e, 0x66, 0x69, 0x67, 0x75, 0x72, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x41, 0x74, 0x74, 0x72, 0x69, 0x62, 0x75, 0x74, 0x65, 0x00, 0x41, 0x73,
                        0x73, 0x65, 0x6d, 0x62, 0x6c, 0x79, 0x43, 0x6f, 0x6d, 0x70, 0x61, 0x6e, 0x79, 0x41, 0x74, 0x74, 0x72, 0x69, 0x62, 0x75, 0x74, 0x65, 0x00, 0x41, 0x73, 0x73, 0x65, 0x6d, 0x62, 0x6c, 0x79, 0x50, 0x72, 0x6f, 0x64, 0x75, 0x63, 0x74, 0x41, 0x74, 0x74, 0x72, 0x69, 0x62, 0x75, 0x74, 0x65, 0x00, 0x41, 0x73, 0x73, 0x65, 0x6d, 0x62, 0x6c, 0x79, 0x43, 0x6f, 0x70, 0x79, 0x72, 0x69, 0x67, 0x68, 0x74, 0x41, 0x74, 0x74, 0x72, 0x69, 0x62, 0x75, 0x74, 0x65, 0x00, 0x41, 0x73, 0x73, 0x65, 0x6d,
                        0x62, 0x6c, 0x79, 0x54, 0x72, 0x61, 0x64, 0x65, 0x6d, 0x61, 0x72, 0x6b, 0x41, 0x74, 0x74, 0x72, 0x69, 0x62, 0x75, 0x74, 0x65, 0x00, 0x41, 0x73, 0x73, 0x65, 0x6d, 0x62, 0x6c, 0x79, 0x43, 0x75, 0x6c, 0x74, 0x75, 0x72, 0x65, 0x41, 0x74, 0x74, 0x72, 0x69, 0x62, 0x75, 0x74, 0x65, 0x00, 0x53, 0x79, 0x73, 0x74, 0x65, 0x6d, 0x2e, 0x52, 0x75, 0x6e, 0x74, 0x69, 0x6d, 0x65, 0x2e, 0x49, 0x6e, 0x74, 0x65, 0x72, 0x6f, 0x70, 0x53, 0x65, 0x72, 0x76, 0x69, 0x63, 0x65, 0x73, 0x00, 0x43, 0x6f,
                        0x6d, 0x56, 0x69, 0x73, 0x69, 0x62, 0x6c, 0x65, 0x41, 0x74, 0x74, 0x72, 0x69, 0x62, 0x75, 0x74, 0x65, 0x00, 0x47, 0x75, 0x69, 0x64, 0x41, 0x74, 0x74, 0x72, 0x69, 0x62, 0x75, 0x74, 0x65, 0x00, 0x41, 0x73, 0x73, 0x65, 0x6d, 0x62, 0x6c, 0x79, 0x56, 0x65, 0x72, 0x73, 0x69, 0x6f, 0x6e, 0x41, 0x74, 0x74, 0x72, 0x69, 0x62, 0x75, 0x74, 0x65, 0x00, 0x41, 0x73, 0x73, 0x65, 0x6d, 0x62, 0x6c, 0x79, 0x46, 0x69, 0x6c, 0x65, 0x56, 0x65, 0x72, 0x73, 0x69, 0x6f, 0x6e, 0x41, 0x74, 0x74, 0x72,
                        0x69, 0x62, 0x75, 0x74, 0x65, 0x00, 0x53, 0x79, 0x73, 0x74, 0x65, 0x6d, 0x2e, 0x53, 0x65, 0x63, 0x75, 0x72, 0x69, 0x74, 0x79, 0x2e, 0x50, 0x65, 0x72, 0x6d, 0x69, 0x73, 0x73, 0x69, 0x6f, 0x6e, 0x73, 0x00, 0x53, 0x65, 0x63, 0x75, 0x72, 0x69, 0x74, 0x79, 0x50, 0x65, 0x72, 0x6d, 0x69, 0x73, 0x73, 0x69, 0x6f, 0x6e, 0x41, 0x74, 0x74, 0x72, 0x69, 0x62, 0x75, 0x74, 0x65, 0x00, 0x53, 0x65, 0x63, 0x75, 0x72, 0x69, 0x74, 0x79, 0x41, 0x63, 0x74, 0x69, 0x6f, 0x6e, 0x00, 0x53, 0x79, 0x73,
                        0x74, 0x65, 0x6d, 0x2e, 0x44, 0x69, 0x61, 0x67, 0x6e, 0x6f, 0x73, 0x74, 0x69, 0x63, 0x73, 0x00, 0x44, 0x65, 0x62, 0x75, 0x67, 0x67, 0x61, 0x62, 0x6c, 0x65, 0x41, 0x74, 0x74, 0x72, 0x69, 0x62, 0x75, 0x74, 0x65, 0x00, 0x44, 0x65, 0x62, 0x75, 0x67, 0x67, 0x69, 0x6e, 0x67, 0x4d, 0x6f, 0x64, 0x65, 0x73, 0x00, 0x53, 0x79, 0x73, 0x74, 0x65, 0x6d, 0x2e, 0x52, 0x75, 0x6e, 0x74, 0x69, 0x6d, 0x65, 0x2e, 0x43, 0x6f, 0x6d, 0x70, 0x69, 0x6c, 0x65, 0x72, 0x53, 0x65, 0x72, 0x76, 0x69, 0x63,
                        0x65, 0x73, 0x00, 0x43, 0x6f, 0x6d, 0x70, 0x69, 0x6c, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x52, 0x65, 0x6c, 0x61, 0x78, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x73, 0x41, 0x74, 0x74, 0x72, 0x69, 0x62, 0x75, 0x74, 0x65, 0x00, 0x52, 0x75, 0x6e, 0x74, 0x69, 0x6d, 0x65, 0x43, 0x6f, 0x6d, 0x70, 0x61, 0x74, 0x69, 0x62, 0x69, 0x6c, 0x69, 0x74, 0x79, 0x41, 0x74, 0x74, 0x72, 0x69, 0x62, 0x75, 0x74, 0x65, 0x00, 0x53, 0x74, 0x72, 0x69, 0x6e, 0x67, 0x00, 0x43, 0x6f, 0x6d, 0x70, 0x61, 0x72, 0x65, 0x00,
                        0x49, 0x6d, 0x61, 0x67, 0x65, 0x00, 0x53, 0x61, 0x76, 0x65, 0x00, 0x43, 0x6f, 0x6c, 0x6f, 0x72, 0x00, 0x43, 0x6f, 0x6e, 0x73, 0x6f, 0x6c, 0x65, 0x00, 0x57, 0x72, 0x69, 0x74, 0x65, 0x4c, 0x69, 0x6e, 0x65, 0x00, 0x52, 0x65, 0x61, 0x64, 0x4c, 0x69, 0x6e, 0x65, 0x00, 0x45, 0x78, 0x63, 0x65, 0x70, 0x74, 0x69, 0x6f, 0x6e, 0x00, 0x67, 0x65, 0x74, 0x5f, 0x4a, 0x70, 0x65, 0x67, 0x00, 0x67, 0x65, 0x74, 0x5f, 0x50, 0x6e, 0x67, 0x00, 0x67, 0x65, 0x74, 0x5f, 0x42, 0x6d, 0x70, 0x00, 0x67,
                        0x65, 0x74, 0x5f, 0x47, 0x69, 0x66, 0x00, 0x67, 0x65, 0x74, 0x5f, 0x54, 0x69, 0x66, 0x66, 0x00, 0x53, 0x79, 0x73, 0x74, 0x65, 0x6d, 0x2e, 0x49, 0x4f, 0x00, 0x50, 0x61, 0x74, 0x68, 0x00, 0x43, 0x68, 0x61, 0x72, 0x00, 0x54, 0x72, 0x69, 0x6d, 0x53, 0x74, 0x61, 0x72, 0x74, 0x00, 0x46, 0x69, 0x6c, 0x65, 0x00, 0x46, 0x69, 0x6c, 0x65, 0x53, 0x74, 0x72, 0x65, 0x61, 0x6d, 0x00, 0x46, 0x69, 0x6c, 0x65, 0x4d, 0x6f, 0x64, 0x65, 0x00, 0x4f, 0x70, 0x65, 0x6e, 0x00, 0x42, 0x69, 0x6e, 0x61,
                        0x72, 0x79, 0x57, 0x72, 0x69, 0x74, 0x65, 0x72, 0x00, 0x53, 0x74, 0x72, 0x65, 0x61, 0x6d, 0x00, 0x67, 0x65, 0x74, 0x5f, 0x57, 0x69, 0x64, 0x74, 0x68, 0x00, 0x49, 0x6e, 0x74, 0x33, 0x32, 0x00, 0x67, 0x65, 0x74, 0x5f, 0x48, 0x65, 0x69, 0x67, 0x68, 0x74, 0x00, 0x46, 0x6f, 0x72, 0x6d, 0x61, 0x74, 0x00, 0x67, 0x65, 0x74, 0x5f, 0x43, 0x68, 0x61, 0x72, 0x73, 0x00, 0x57, 0x72, 0x69, 0x74, 0x65, 0x00, 0x67, 0x65, 0x74, 0x5f, 0x4c, 0x65, 0x6e, 0x67, 0x74, 0x68, 0x00, 0x50, 0x69, 0x78,
                        0x65, 0x6c, 0x46, 0x6f, 0x72, 0x6d, 0x61, 0x74, 0x00, 0x52, 0x65, 0x63, 0x74, 0x61, 0x6e, 0x67, 0x6c, 0x65, 0x46, 0x00, 0x47, 0x72, 0x61, 0x70, 0x68, 0x69, 0x63, 0x73, 0x55, 0x6e, 0x69, 0x74, 0x00, 0x47, 0x65, 0x74, 0x42, 0x6f, 0x75, 0x6e, 0x64, 0x73, 0x00, 0x67, 0x65, 0x74, 0x5f, 0x58, 0x00, 0x67, 0x65, 0x74, 0x5f, 0x59, 0x00, 0x52, 0x65, 0x63, 0x74, 0x61, 0x6e, 0x67, 0x6c, 0x65, 0x00, 0x49, 0x6d, 0x61, 0x67, 0x65, 0x4c, 0x6f, 0x63, 0x6b, 0x4d, 0x6f, 0x64, 0x65, 0x00, 0x4c,
                        0x6f, 0x63, 0x6b, 0x42, 0x69, 0x74, 0x73, 0x00, 0x67, 0x65, 0x74, 0x5f, 0x53, 0x63, 0x61, 0x6e, 0x30, 0x00, 0x49, 0x6e, 0x74, 0x50, 0x74, 0x72, 0x00, 0x54, 0x6f, 0x50, 0x6f, 0x69, 0x6e, 0x74, 0x65, 0x72, 0x00, 0x55, 0x6e, 0x6c, 0x6f, 0x63, 0x6b, 0x42, 0x69, 0x74, 0x73, 0x00, 0x53, 0x74, 0x72, 0x75, 0x63, 0x74, 0x4c, 0x61, 0x79, 0x6f, 0x75, 0x74, 0x41, 0x74, 0x74, 0x72, 0x69, 0x62, 0x75, 0x74, 0x65, 0x00, 0x4c, 0x61, 0x79, 0x6f, 0x75, 0x74, 0x4b, 0x69, 0x6e, 0x64, 0x00, 0x53,
                        0x79, 0x73, 0x74, 0x65, 0x6d, 0x2e, 0x53, 0x65, 0x63, 0x75, 0x72, 0x69, 0x74, 0x79, 0x00, 0x55, 0x6e, 0x76, 0x65, 0x72, 0x69, 0x66, 0x69, 0x61, 0x62, 0x6c, 0x65, 0x43, 0x6f, 0x64, 0x65, 0x41, 0x74, 0x74, 0x72, 0x69, 0x62, 0x75, 0x74, 0x65, 0x00, 0x00, 0x00, 0x00, 0x00, 0x57, 0x63, 0x00, 0x3a, 0x00, 0x5c, 0x00, 0x77, 0x00, 0x6f, 0x00, 0x72, 0x00, 0x6b, 0x00, 0x5f, 0x00, 0x6e, 0x00, 0x65, 0x00, 0x77, 0x00, 0x5c, 0x00, 0x47, 0x00, 0x49, 0x00, 0x54, 0x00, 0x5c, 0x00, 0x74, 0x00,
                        0x65, 0x00, 0x73, 0x00, 0x74, 0x00, 0x70, 0x00, 0x6e, 0x00, 0x67, 0x00, 0x5c, 0x00, 0x63, 0x00, 0x79, 0x00, 0x6c, 0x00, 0x69, 0x00, 0x6e, 0x00, 0x64, 0x00, 0x65, 0x00, 0x72, 0x00, 0x5f, 0x00, 0x73, 0x00, 0x68, 0x00, 0x61, 0x00, 0x64, 0x00, 0x65, 0x00, 0x64, 0x00, 0x2e, 0x00, 0x70, 0x00, 0x6e, 0x00, 0x67, 0x00, 0x00, 0x57, 0x63, 0x00, 0x3a, 0x00, 0x5c, 0x00, 0x77, 0x00, 0x6f, 0x00, 0x72, 0x00, 0x6b, 0x00, 0x5f, 0x00, 0x6e, 0x00, 0x65, 0x00, 0x77, 0x00, 0x5c, 0x00, 0x47, 0x00,
                        0x49, 0x00, 0x54, 0x00, 0x5c, 0x00, 0x74, 0x00, 0x65, 0x00, 0x73, 0x00, 0x74, 0x00, 0x70, 0x00, 0x6e, 0x00, 0x67, 0x00, 0x5c, 0x00, 0x63, 0x00, 0x79, 0x00, 0x6c, 0x00, 0x69, 0x00, 0x6e, 0x00, 0x64, 0x00, 0x65, 0x00, 0x72, 0x00, 0x5f, 0x00, 0x73, 0x00, 0x68, 0x00, 0x61, 0x00, 0x64, 0x00, 0x65, 0x00, 0x64, 0x00, 0x2e, 0x00, 0x70, 0x00, 0x70, 0x00, 0x6d, 0x00, 0x00, 0x07, 0x70, 0x00, 0x70, 0x00, 0x6d, 0x00, 0x00, 0x07, 0x70, 0x00, 0x67, 0x00, 0x6d, 0x00, 0x00, 0x1d, 0x45, 0x00,
                        0x78, 0x00, 0x63, 0x00, 0x65, 0x00, 0x70, 0x00, 0x74, 0x00, 0x69, 0x00, 0x6f, 0x00, 0x6e, 0x00, 0x3a, 0x00, 0x20, 0x00, 0x7b, 0x00, 0x30, 0x00, 0x7d, 0x00, 0x00, 0x07, 0x6a, 0x00, 0x70, 0x00, 0x67, 0x00, 0x00, 0x09, 0x6a, 0x00, 0x70, 0x00, 0x65, 0x00, 0x67, 0x00, 0x00, 0x07, 0x70, 0x00, 0x6e, 0x00, 0x67, 0x00, 0x00, 0x07, 0x62, 0x00, 0x6d, 0x00, 0x70, 0x00, 0x00, 0x07, 0x67, 0x00, 0x69, 0x00, 0x66, 0x00, 0x00, 0x07, 0x74, 0x00, 0x69, 0x00, 0x66, 0x00, 0x00, 0x09, 0x74, 0x00,
                        0x69, 0x00, 0x66, 0x00, 0x66, 0x00, 0x00, 0x51, 0x50, 0x00, 0x36, 0x00, 0x0a, 0x00, 0x23, 0x00, 0x20, 0x00, 0x69, 0x00, 0x6d, 0x00, 0x67, 0x00, 0x63, 0x00, 0x6f, 0x00, 0x6e, 0x00, 0x76, 0x00, 0x65, 0x00, 0x72, 0x00, 0x74, 0x00, 0x20, 0x00, 0x56, 0x00, 0x65, 0x00, 0x72, 0x00, 0x73, 0x00, 0x69, 0x00, 0x6f, 0x00, 0x6e, 0x00, 0x20, 0x00, 0x31, 0x00, 0x2e, 0x00, 0x31, 0x00, 0x0a, 0x00, 0x7b, 0x00, 0x30, 0x00, 0x7d, 0x00, 0x20, 0x00, 0x7b, 0x00, 0x31, 0x00, 0x7d, 0x00, 0x0a, 0x00,
                        0x7b, 0x00, 0x32, 0x00, 0x7d, 0x00, 0x0a, 0x00, 0x00, 0x51, 0x50, 0x00, 0x35, 0x00, 0x0a, 0x00, 0x23, 0x00, 0x20, 0x00, 0x69, 0x00, 0x6d, 0x00, 0x67, 0x00, 0x63, 0x00, 0x6f, 0x00, 0x6e, 0x00, 0x76, 0x00, 0x65, 0x00, 0x72, 0x00, 0x74, 0x00, 0x20, 0x00, 0x56, 0x00, 0x65, 0x00, 0x72, 0x00, 0x73, 0x00, 0x69, 0x00, 0x6f, 0x00, 0x6e, 0x00, 0x20, 0x00, 0x31, 0x00, 0x2e, 0x00, 0x31, 0x00, 0x0a, 0x00, 0x7b, 0x00, 0x30, 0x00, 0x7d, 0x00, 0x20, 0x00, 0x7b, 0x00, 0x31, 0x00, 0x7d, 0x00,
                        0x0a, 0x00, 0x7b, 0x00, 0x32, 0x00, 0x7d, 0x00, 0x0a, 0x00, 0x00, 0x00, 0xb6, 0xc3, 0xb4, 0x62, 0xe4, 0xf3, 0x28, 0x47, 0x84, 0xbe, 0x9b, 0x5e, 0x3e, 0x3a, 0x48, 0xbe, 0x00, 0x08, 0xb7, 0x7a, 0x5c, 0x56, 0x19, 0x34, 0xe0, 0x89, 0x05, 0x00, 0x01, 0x01, 0x1d, 0x0e, 0x08, 0xb0, 0x3f, 0x5f, 0x7f, 0x11, 0xd5, 0x0a, 0x3a, 0x05, 0x00, 0x01, 0x12, 0x0d, 0x0e, 0x04, 0x00, 0x01, 0x0e, 0x0e, 0x06, 0x00, 0x02, 0x01, 0x12, 0x0c, 0x0e, 0x05, 0x00, 0x01, 0x05, 0x11, 0x10, 0x03, 0x20, 0x00,
                        0x01, 0x03, 0x06, 0x12, 0x11, 0x02, 0x06, 0x08, 0x03, 0x06, 0x12, 0x15, 0x03, 0x06, 0x0f, 0x05, 0x04, 0x20, 0x01, 0x01, 0x0e, 0x05, 0x20, 0x01, 0x01, 0x12, 0x11, 0x05, 0x20, 0x02, 0x01, 0x08, 0x08, 0x04, 0x20, 0x00, 0x12, 0x11, 0x04, 0x20, 0x00, 0x11, 0x19, 0x06, 0x20, 0x02, 0x11, 0x10, 0x08, 0x08, 0x07, 0x20, 0x03, 0x01, 0x08, 0x08, 0x11, 0x10, 0x08, 0x20, 0x05, 0x01, 0x08, 0x08, 0x05, 0x05, 0x05, 0x07, 0x20, 0x02, 0x0f, 0x11, 0x10, 0x08, 0x08, 0x04, 0x28, 0x00, 0x12, 0x11,
                        0x04, 0x28, 0x00, 0x11, 0x19, 0x02, 0x06, 0x05, 0x04, 0x20, 0x01, 0x01, 0x02, 0x05, 0x20, 0x01, 0x01, 0x11, 0x55, 0x05, 0x20, 0x01, 0x01, 0x11, 0x5d, 0x04, 0x20, 0x01, 0x01, 0x08, 0x06, 0x00, 0x03, 0x08, 0x0e, 0x0e, 0x02, 0x06, 0x20, 0x02, 0x01, 0x0e, 0x12, 0x0d, 0x05, 0x00, 0x02, 0x01, 0x0e, 0x1c, 0x03, 0x00, 0x00, 0x0e, 0x0e, 0x07, 0x08, 0x0e, 0x0e, 0x12, 0x0c, 0x0e, 0x12, 0x0d, 0x11, 0x71, 0x12, 0x79, 0x02, 0x04, 0x00, 0x00, 0x12, 0x0d, 0x05, 0x07, 0x02, 0x12, 0x0d, 0x02,
                        0x05, 0x20, 0x01, 0x0e, 0x1d, 0x03, 0x06, 0x07, 0x03, 0x0e, 0x0e, 0x1d, 0x03, 0x09, 0x00, 0x02, 0x12, 0x80, 0x89, 0x0e, 0x11, 0x80, 0x8d, 0x06, 0x20, 0x01, 0x01, 0x12, 0x80, 0x95, 0x03, 0x20, 0x00, 0x08, 0x07, 0x00, 0x04, 0x0e, 0x0e, 0x1c, 0x1c, 0x1c, 0x04, 0x20, 0x01, 0x03, 0x08, 0x04, 0x20, 0x01, 0x01, 0x03, 0x04, 0x20, 0x01, 0x01, 0x05, 0x0e, 0x07, 0x09, 0x12, 0x80, 0x91, 0x0e, 0x03, 0x08, 0x08, 0x11, 0x10, 0x0e, 0x08, 0x02, 0x05, 0x07, 0x03, 0x0d, 0x05, 0x02, 0x0f, 0x07,
                        0x0a, 0x12, 0x80, 0x91, 0x0e, 0x03, 0x08, 0x08, 0x11, 0x10, 0x05, 0x0e, 0x08, 0x02, 0x05, 0x20, 0x01, 0x01, 0x12, 0x6d, 0x08, 0x20, 0x03, 0x01, 0x08, 0x08, 0x11, 0x80, 0x9d, 0x04, 0x07, 0x01, 0x12, 0x11, 0x09, 0x20, 0x01, 0x11, 0x80, 0xa1, 0x10, 0x11, 0x80, 0xa5, 0x03, 0x20, 0x00, 0x0c, 0x0a, 0x07, 0x03, 0x11, 0x80, 0xa5, 0x11, 0x80, 0xa1, 0x11, 0x19, 0x07, 0x20, 0x04, 0x01, 0x08, 0x08, 0x08, 0x08, 0x0d, 0x20, 0x03, 0x12, 0x15, 0x11, 0x80, 0xa9, 0x11, 0x80, 0xad, 0x11, 0x80,
                        0x9d, 0x03, 0x20, 0x00, 0x18, 0x04, 0x20, 0x00, 0x0f, 0x01, 0x0d, 0x07, 0x05, 0x11, 0x80, 0xa5, 0x11, 0x80, 0xa1, 0x11, 0x80, 0xa9, 0x02, 0x18, 0x06, 0x07, 0x02, 0x11, 0x10, 0x11, 0x10, 0x05, 0x07, 0x01, 0x0f, 0x11, 0x10, 0x05, 0x20, 0x01, 0x01, 0x12, 0x15, 0x06, 0x20, 0x01, 0x01, 0x11, 0x80, 0xb9, 0x04, 0x01, 0x00, 0x00, 0x00, 0x47, 0x01, 0x00, 0x1a, 0x2e, 0x4e, 0x45, 0x54, 0x46, 0x72, 0x61, 0x6d, 0x65, 0x77, 0x6f, 0x72, 0x6b, 0x2c, 0x56, 0x65, 0x72, 0x73, 0x69, 0x6f, 0x6e,
                        0x3d, 0x76, 0x34, 0x2e, 0x30, 0x01, 0x00, 0x54, 0x0e, 0x14, 0x46, 0x72, 0x61, 0x6d, 0x65, 0x77, 0x6f, 0x72, 0x6b, 0x44, 0x69, 0x73, 0x70, 0x6c, 0x61, 0x79, 0x4e, 0x61, 0x6d, 0x65, 0x10, 0x2e, 0x4e, 0x45, 0x54, 0x20, 0x46, 0x72, 0x61, 0x6d, 0x65, 0x77, 0x6f, 0x72, 0x6b, 0x20, 0x34, 0x0f, 0x01, 0x00, 0x0a, 0x69, 0x6d, 0x67, 0x63, 0x6f, 0x6e, 0x76, 0x65, 0x72, 0x74, 0x00, 0x00, 0x05, 0x01, 0x00, 0x00, 0x00, 0x00, 0x18, 0x01, 0x00, 0x13, 0x46, 0x45, 0x49, 0x20, 0x45, 0x6c, 0x65,
                        0x63, 0x74, 0x72, 0x6f, 0x6e, 0x20, 0x4f, 0x70, 0x74, 0x69, 0x63, 0x73, 0x00, 0x00, 0x2a, 0x01, 0x00, 0x25, 0x43, 0x6f, 0x70, 0x79, 0x72, 0x69, 0x67, 0x68, 0x74, 0x20, 0xc2, 0xa9, 0x20, 0x46, 0x45, 0x49, 0x20, 0x45, 0x6c, 0x65, 0x63, 0x74, 0x72, 0x6f, 0x6e, 0x20, 0x4f, 0x70, 0x74, 0x69, 0x63, 0x73, 0x20, 0x32, 0x30, 0x31, 0x34, 0x00, 0x00, 0x29, 0x01, 0x00, 0x24, 0x64, 0x65, 0x32, 0x36, 0x64, 0x64, 0x36, 0x36, 0x2d, 0x38, 0x38, 0x35, 0x31, 0x2d, 0x34, 0x32, 0x35, 0x39, 0x2d,
                        0x61, 0x33, 0x66, 0x38, 0x2d, 0x33, 0x33, 0x65, 0x64, 0x30, 0x30, 0x36, 0x66, 0x34, 0x63, 0x39, 0x35, 0x00, 0x00, 0x0c, 0x01, 0x00, 0x07, 0x31, 0x2e, 0x30, 0x2e, 0x30, 0x2e, 0x30, 0x00, 0x00, 0x08, 0x01, 0x00, 0x07, 0x01, 0x00, 0x00, 0x00, 0x00, 0x08, 0x01, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x01, 0x00, 0x01, 0x00, 0x54, 0x02, 0x16, 0x57, 0x72, 0x61, 0x70, 0x4e, 0x6f, 0x6e, 0x45, 0x78, 0x63, 0x65, 0x70, 0x74, 0x69, 0x6f, 0x6e, 0x54, 0x68, 0x72, 0x6f, 0x77, 0x73,
                        0x01, 0x80, 0x9e, 0x2e, 0x01, 0x80, 0x84, 0x53, 0x79, 0x73, 0x74, 0x65, 0x6d, 0x2e, 0x53, 0x65, 0x63, 0x75, 0x72, 0x69, 0x74, 0x79, 0x2e, 0x50, 0x65, 0x72, 0x6d, 0x69, 0x73, 0x73, 0x69, 0x6f, 0x6e, 0x73, 0x2e, 0x53, 0x65, 0x63, 0x75, 0x72, 0x69, 0x74, 0x79, 0x50, 0x65, 0x72, 0x6d, 0x69, 0x73, 0x73, 0x69, 0x6f, 0x6e, 0x41, 0x74, 0x74, 0x72, 0x69, 0x62, 0x75, 0x74, 0x65, 0x2c, 0x20, 0x6d, 0x73, 0x63, 0x6f, 0x72, 0x6c, 0x69, 0x62, 0x2c, 0x20, 0x56, 0x65, 0x72, 0x73, 0x69, 0x6f,
                        0x6e, 0x3d, 0x34, 0x2e, 0x30, 0x2e, 0x30, 0x2e, 0x30, 0x2c, 0x20, 0x43, 0x75, 0x6c, 0x74, 0x75, 0x72, 0x65, 0x3d, 0x6e, 0x65, 0x75, 0x74, 0x72, 0x61, 0x6c, 0x2c, 0x20, 0x50, 0x75, 0x62, 0x6c, 0x69, 0x63, 0x4b, 0x65, 0x79, 0x54, 0x6f, 0x6b, 0x65, 0x6e, 0x3d, 0x62, 0x37, 0x37, 0x61, 0x35, 0x63, 0x35, 0x36, 0x31, 0x39, 0x33, 0x34, 0x65, 0x30, 0x38, 0x39, 0x15, 0x01, 0x54, 0x02, 0x10, 0x53, 0x6b, 0x69, 0x70, 0x56, 0x65, 0x72, 0x69, 0x66, 0x69, 0x63, 0x61, 0x74, 0x69, 0x6f, 0x6e,
                        0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x33, 0x03, 0x71, 0x54, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x1c, 0x01, 0x00, 0x00, 0x10, 0x38, 0x00, 0x00, 0x10, 0x1a, 0x00, 0x00, 0x52, 0x53, 0x44, 0x53, 0x4e, 0x3b, 0x28, 0x94, 0x5a, 0x6b, 0x61, 0x42, 0x83, 0x82, 0xe7, 0x39, 0x67, 0x66, 0x7c, 0x8f, 0x0e, 0x00, 0x00, 0x00, 0x63, 0x3a, 0x5c, 0x77, 0x6f, 0x72, 0x6b, 0x5f, 0x6e, 0x65, 0x77, 0x5c, 0x69, 0x6d, 0x67, 0x63, 0x6f, 0x6e, 0x76, 0x65, 0x72, 0x74, 0x5c, 0x69,
                        0x6d, 0x67, 0x63, 0x6f, 0x6e, 0x76, 0x65, 0x72, 0x74, 0x5c, 0x6f, 0x62, 0x6a, 0x5c, 0x44, 0x65, 0x62, 0x75, 0x67, 0x5c, 0x69, 0x6d, 0x67, 0x63, 0x6f, 0x6e, 0x76, 0x65, 0x72, 0x74, 0x2e, 0x70, 0x64, 0x62, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x54, 0x39, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6e, 0x39, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x39, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5f, 0x43, 0x6f, 0x72, 0x45, 0x78, 0x65, 0x4d, 0x61, 0x69, 0x6e, 0x00, 0x6d, 0x73, 0x63, 0x6f, 0x72, 0x65, 0x65, 0x2e, 0x64, 0x6c, 0x6c, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x25,
                        0x00, 0x20, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x10, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x80, 0x18, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00, 0x80,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x50, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x68, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x90, 0x00, 0x00, 0x00, 0xa0, 0x40, 0x00, 0x00, 0x30, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xd0, 0x43, 0x00, 0x00, 0xea, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x03, 0x34, 0x00, 0x00, 0x00, 0x56, 0x00, 0x53, 0x00, 0x5f, 0x00, 0x56, 0x00, 0x45, 0x00, 0x52, 0x00, 0x53, 0x00, 0x49, 0x00, 0x4f, 0x00, 0x4e, 0x00, 0x5f, 0x00, 0x49, 0x00, 0x4e, 0x00,
                        0x46, 0x00, 0x4f, 0x00, 0x00, 0x00, 0x00, 0x00, 0xbd, 0x04, 0xef, 0xfe, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x44, 0x00, 0x00, 0x00, 0x01, 0x00, 0x56, 0x00, 0x61, 0x00, 0x72, 0x00, 0x46, 0x00, 0x69, 0x00, 0x6c, 0x00, 0x65, 0x00,
                        0x49, 0x00, 0x6e, 0x00, 0x66, 0x00, 0x6f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x24, 0x00, 0x04, 0x00, 0x00, 0x00, 0x54, 0x00, 0x72, 0x00, 0x61, 0x00, 0x6e, 0x00, 0x73, 0x00, 0x6c, 0x00, 0x61, 0x00, 0x74, 0x00, 0x69, 0x00, 0x6f, 0x00, 0x6e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xb0, 0x04, 0x90, 0x02, 0x00, 0x00, 0x01, 0x00, 0x53, 0x00, 0x74, 0x00, 0x72, 0x00, 0x69, 0x00, 0x6e, 0x00, 0x67, 0x00, 0x46, 0x00, 0x69, 0x00, 0x6c, 0x00, 0x65, 0x00, 0x49, 0x00, 0x6e, 0x00, 0x66, 0x00,
                        0x6f, 0x00, 0x00, 0x00, 0x6c, 0x02, 0x00, 0x00, 0x01, 0x00, 0x30, 0x00, 0x30, 0x00, 0x30, 0x00, 0x30, 0x00, 0x30, 0x00, 0x34, 0x00, 0x62, 0x00, 0x30, 0x00, 0x00, 0x00, 0x48, 0x00, 0x14, 0x00, 0x01, 0x00, 0x43, 0x00, 0x6f, 0x00, 0x6d, 0x00, 0x70, 0x00, 0x61, 0x00, 0x6e, 0x00, 0x79, 0x00, 0x4e, 0x00, 0x61, 0x00, 0x6d, 0x00, 0x65, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46, 0x00, 0x45, 0x00, 0x49, 0x00, 0x20, 0x00, 0x45, 0x00, 0x6c, 0x00, 0x65, 0x00, 0x63, 0x00, 0x74, 0x00, 0x72, 0x00,
                        0x6f, 0x00, 0x6e, 0x00, 0x20, 0x00, 0x4f, 0x00, 0x70, 0x00, 0x74, 0x00, 0x69, 0x00, 0x63, 0x00, 0x73, 0x00, 0x00, 0x00, 0x40, 0x00, 0x0b, 0x00, 0x01, 0x00, 0x46, 0x00, 0x69, 0x00, 0x6c, 0x00, 0x65, 0x00, 0x44, 0x00, 0x65, 0x00, 0x73, 0x00, 0x63, 0x00, 0x72, 0x00, 0x69, 0x00, 0x70, 0x00, 0x74, 0x00, 0x69, 0x00, 0x6f, 0x00, 0x6e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x69, 0x00, 0x6d, 0x00, 0x67, 0x00, 0x63, 0x00, 0x6f, 0x00, 0x6e, 0x00, 0x76, 0x00, 0x65, 0x00, 0x72, 0x00, 0x74, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x08, 0x00, 0x01, 0x00, 0x46, 0x00, 0x69, 0x00, 0x6c, 0x00, 0x65, 0x00, 0x56, 0x00, 0x65, 0x00, 0x72, 0x00, 0x73, 0x00, 0x69, 0x00, 0x6f, 0x00, 0x6e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x31, 0x00, 0x2e, 0x00, 0x30, 0x00, 0x2e, 0x00, 0x30, 0x00, 0x2e, 0x00, 0x30, 0x00, 0x00, 0x00, 0x40, 0x00, 0x0f, 0x00, 0x01, 0x00, 0x49, 0x00, 0x6e, 0x00, 0x74, 0x00, 0x65, 0x00, 0x72, 0x00, 0x6e, 0x00, 0x61, 0x00, 0x6c, 0x00, 0x4e, 0x00, 0x61, 0x00, 0x6d, 0x00,
                        0x65, 0x00, 0x00, 0x00, 0x69, 0x00, 0x6d, 0x00, 0x67, 0x00, 0x63, 0x00, 0x6f, 0x00, 0x6e, 0x00, 0x76, 0x00, 0x65, 0x00, 0x72, 0x00, 0x74, 0x00, 0x2e, 0x00, 0x65, 0x00, 0x78, 0x00, 0x65, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 0x25, 0x00, 0x01, 0x00, 0x4c, 0x00, 0x65, 0x00, 0x67, 0x00, 0x61, 0x00, 0x6c, 0x00, 0x43, 0x00, 0x6f, 0x00, 0x70, 0x00, 0x79, 0x00, 0x72, 0x00, 0x69, 0x00, 0x67, 0x00, 0x68, 0x00, 0x74, 0x00, 0x00, 0x00, 0x43, 0x00, 0x6f, 0x00, 0x70, 0x00, 0x79, 0x00,
                        0x72, 0x00, 0x69, 0x00, 0x67, 0x00, 0x68, 0x00, 0x74, 0x00, 0x20, 0x00, 0xa9, 0x00, 0x20, 0x00, 0x46, 0x00, 0x45, 0x00, 0x49, 0x00, 0x20, 0x00, 0x45, 0x00, 0x6c, 0x00, 0x65, 0x00, 0x63, 0x00, 0x74, 0x00, 0x72, 0x00, 0x6f, 0x00, 0x6e, 0x00, 0x20, 0x00, 0x4f, 0x00, 0x70, 0x00, 0x74, 0x00, 0x69, 0x00, 0x63, 0x00, 0x73, 0x00, 0x20, 0x00, 0x32, 0x00, 0x30, 0x00, 0x31, 0x00, 0x34, 0x00, 0x00, 0x00, 0x00, 0x00, 0x48, 0x00, 0x0f, 0x00, 0x01, 0x00, 0x4f, 0x00, 0x72, 0x00, 0x69, 0x00,
                        0x67, 0x00, 0x69, 0x00, 0x6e, 0x00, 0x61, 0x00, 0x6c, 0x00, 0x46, 0x00, 0x69, 0x00, 0x6c, 0x00, 0x65, 0x00, 0x6e, 0x00, 0x61, 0x00, 0x6d, 0x00, 0x65, 0x00, 0x00, 0x00, 0x69, 0x00, 0x6d, 0x00, 0x67, 0x00, 0x63, 0x00, 0x6f, 0x00, 0x6e, 0x00, 0x76, 0x00, 0x65, 0x00, 0x72, 0x00, 0x74, 0x00, 0x2e, 0x00, 0x65, 0x00, 0x78, 0x00, 0x65, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0x0b, 0x00, 0x01, 0x00, 0x50, 0x00, 0x72, 0x00, 0x6f, 0x00, 0x64, 0x00, 0x75, 0x00, 0x63, 0x00, 0x74, 0x00,
                        0x4e, 0x00, 0x61, 0x00, 0x6d, 0x00, 0x65, 0x00, 0x00, 0x00, 0x00, 0x00, 0x69, 0x00, 0x6d, 0x00, 0x67, 0x00, 0x63, 0x00, 0x6f, 0x00, 0x6e, 0x00, 0x76, 0x00, 0x65, 0x00, 0x72, 0x00, 0x74, 0x00, 0x00, 0x00, 0x00, 0x00, 0x34, 0x00, 0x08, 0x00, 0x01, 0x00, 0x50, 0x00, 0x72, 0x00, 0x6f, 0x00, 0x64, 0x00, 0x75, 0x00, 0x63, 0x00, 0x74, 0x00, 0x56, 0x00, 0x65, 0x00, 0x72, 0x00, 0x73, 0x00, 0x69, 0x00, 0x6f, 0x00, 0x6e, 0x00, 0x00, 0x00, 0x31, 0x00, 0x2e, 0x00, 0x30, 0x00, 0x2e, 0x00,
                        0x30, 0x00, 0x2e, 0x00, 0x30, 0x00, 0x00, 0x00, 0x38, 0x00, 0x08, 0x00, 0x01, 0x00, 0x41, 0x00, 0x73, 0x00, 0x73, 0x00, 0x65, 0x00, 0x6d, 0x00, 0x62, 0x00, 0x6c, 0x00, 0x79, 0x00, 0x20, 0x00, 0x56, 0x00, 0x65, 0x00, 0x72, 0x00, 0x73, 0x00, 0x69, 0x00, 0x6f, 0x00, 0x6e, 0x00, 0x00, 0x00, 0x31, 0x00, 0x2e, 0x00, 0x30, 0x00, 0x2e, 0x00, 0x30, 0x00, 0x2e, 0x00, 0x30, 0x00, 0x00, 0x00, 0xef, 0xbb, 0xbf, 0x3c, 0x3f, 0x78, 0x6d, 0x6c, 0x20, 0x76, 0x65, 0x72, 0x73, 0x69, 0x6f, 0x6e,
                        0x3d, 0x22, 0x31, 0x2e, 0x30, 0x22, 0x20, 0x65, 0x6e, 0x63, 0x6f, 0x64, 0x69, 0x6e, 0x67, 0x3d, 0x22, 0x55, 0x54, 0x46, 0x2d, 0x38, 0x22, 0x20, 0x73, 0x74, 0x61, 0x6e, 0x64, 0x61, 0x6c, 0x6f, 0x6e, 0x65, 0x3d, 0x22, 0x79, 0x65, 0x73, 0x22, 0x3f, 0x3e, 0x0d, 0x0a, 0x3c, 0x61, 0x73, 0x73, 0x65, 0x6d, 0x62, 0x6c, 0x79, 0x20, 0x78, 0x6d, 0x6c, 0x6e, 0x73, 0x3d, 0x22, 0x75, 0x72, 0x6e, 0x3a, 0x73, 0x63, 0x68, 0x65, 0x6d, 0x61, 0x73, 0x2d, 0x6d, 0x69, 0x63, 0x72, 0x6f, 0x73, 0x6f,
                        0x66, 0x74, 0x2d, 0x63, 0x6f, 0x6d, 0x3a, 0x61, 0x73, 0x6d, 0x2e, 0x76, 0x31, 0x22, 0x20, 0x6d, 0x61, 0x6e, 0x69, 0x66, 0x65, 0x73, 0x74, 0x56, 0x65, 0x72, 0x73, 0x69, 0x6f, 0x6e, 0x3d, 0x22, 0x31, 0x2e, 0x30, 0x22, 0x3e, 0x0d, 0x0a, 0x20, 0x20, 0x3c, 0x61, 0x73, 0x73, 0x65, 0x6d, 0x62, 0x6c, 0x79, 0x49, 0x64, 0x65, 0x6e, 0x74, 0x69, 0x74, 0x79, 0x20, 0x76, 0x65, 0x72, 0x73, 0x69, 0x6f, 0x6e, 0x3d, 0x22, 0x31, 0x2e, 0x30, 0x2e, 0x30, 0x2e, 0x30, 0x22, 0x20, 0x6e, 0x61, 0x6d,
                        0x65, 0x3d, 0x22, 0x4d, 0x79, 0x41, 0x70, 0x70, 0x6c, 0x69, 0x63, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x2e, 0x61, 0x70, 0x70, 0x22, 0x2f, 0x3e, 0x0d, 0x0a, 0x20, 0x20, 0x3c, 0x74, 0x72, 0x75, 0x73, 0x74, 0x49, 0x6e, 0x66, 0x6f, 0x20, 0x78, 0x6d, 0x6c, 0x6e, 0x73, 0x3d, 0x22, 0x75, 0x72, 0x6e, 0x3a, 0x73, 0x63, 0x68, 0x65, 0x6d, 0x61, 0x73, 0x2d, 0x6d, 0x69, 0x63, 0x72, 0x6f, 0x73, 0x6f, 0x66, 0x74, 0x2d, 0x63, 0x6f, 0x6d, 0x3a, 0x61, 0x73, 0x6d, 0x2e, 0x76, 0x32, 0x22, 0x3e, 0x0d,
                        0x0a, 0x20, 0x20, 0x20, 0x20, 0x3c, 0x73, 0x65, 0x63, 0x75, 0x72, 0x69, 0x74, 0x79, 0x3e, 0x0d, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x3c, 0x72, 0x65, 0x71, 0x75, 0x65, 0x73, 0x74, 0x65, 0x64, 0x50, 0x72, 0x69, 0x76, 0x69, 0x6c, 0x65, 0x67, 0x65, 0x73, 0x20, 0x78, 0x6d, 0x6c, 0x6e, 0x73, 0x3d, 0x22, 0x75, 0x72, 0x6e, 0x3a, 0x73, 0x63, 0x68, 0x65, 0x6d, 0x61, 0x73, 0x2d, 0x6d, 0x69, 0x63, 0x72, 0x6f, 0x73, 0x6f, 0x66, 0x74, 0x2d, 0x63, 0x6f, 0x6d, 0x3a, 0x61, 0x73, 0x6d,
                        0x2e, 0x76, 0x33, 0x22, 0x3e, 0x0d, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x3c, 0x72, 0x65, 0x71, 0x75, 0x65, 0x73, 0x74, 0x65, 0x64, 0x45, 0x78, 0x65, 0x63, 0x75, 0x74, 0x69, 0x6f, 0x6e, 0x4c, 0x65, 0x76, 0x65, 0x6c, 0x20, 0x6c, 0x65, 0x76, 0x65, 0x6c, 0x3d, 0x22, 0x61, 0x73, 0x49, 0x6e, 0x76, 0x6f, 0x6b, 0x65, 0x72, 0x22, 0x20, 0x75, 0x69, 0x41, 0x63, 0x63, 0x65, 0x73, 0x73, 0x3d, 0x22, 0x66, 0x61, 0x6c, 0x73, 0x65, 0x22, 0x2f, 0x3e, 0x0d, 0x0a, 0x20, 0x20,
                        0x20, 0x20, 0x20, 0x20, 0x3c, 0x2f, 0x72, 0x65, 0x71, 0x75, 0x65, 0x73, 0x74, 0x65, 0x64, 0x50, 0x72, 0x69, 0x76, 0x69, 0x6c, 0x65, 0x67, 0x65, 0x73, 0x3e, 0x0d, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x3c, 0x2f, 0x73, 0x65, 0x63, 0x75, 0x72, 0x69, 0x74, 0x79, 0x3e, 0x0d, 0x0a, 0x20, 0x20, 0x3c, 0x2f, 0x74, 0x72, 0x75, 0x73, 0x74, 0x49, 0x6e, 0x66, 0x6f, 0x3e, 0x0d, 0x0a, 0x3c, 0x2f, 0x61, 0x73, 0x73, 0x65, 0x6d, 0x62, 0x6c, 0x79, 0x3e, 0x0d, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x80, 0x39, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

                    *pLen = ARR_SIZE(data);
                    return data;
                }
        };
    }

}


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

#undef OUT_ARG_1
#undef OUT_ARG_2
#undef OUT_ARG_3
#undef OUT_ARG_4
#undef OUT_ARG_5
#undef OUT_ARG_6
#undef OUT_ARG_7
#undef OUT_ARG_8
#undef OUT_ARG_9
#undef OUT_ARG_10

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



//  syntax like in c#  
//#define foreach for
//#define in :

// for quick conversions from String to for c-functions  accepting (const char *)  
// Example:    
//      String s = "/home/images/image.pgm";    
//      fopen(s.sss(), "w");
//


#define sss() Tostdstring().c_str()

#pragma warning(default: 4482) 
