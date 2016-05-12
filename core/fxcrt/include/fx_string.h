// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_INCLUDE_FX_STRING_H_
#define CORE_FXCRT_INCLUDE_FX_STRING_H_

#include <stdint.h>  // For intptr_t.
#include <algorithm>

#include "core/fxcrt/cfx_string_data_template.h"
#include "core/fxcrt/include/cfx_retain_ptr.h"
#include "core/fxcrt/include/fx_memory.h"
#include "core/fxcrt/include/fx_system.h"

class CFX_ByteString;
class CFX_WideString;

// An immutable string with caller-provided storage which must outlive the
// string itself. These are not necessarily nul-terminated, so that substring
// extraction (via the Mid() method) is copy-free.
class CFX_ByteStringC {
 public:
  typedef FX_CHAR value_type;

  CFX_ByteStringC() {
    m_Ptr = NULL;
    m_Length = 0;
  }

  CFX_ByteStringC(const uint8_t* ptr, FX_STRSIZE size) {
    m_Ptr = ptr;
    m_Length = size;
  }

  // Deliberately implicit to avoid calling on every string literal.
  CFX_ByteStringC(const FX_CHAR* ptr) {
    m_Ptr = (const uint8_t*)ptr;
    m_Length = ptr ? FXSYS_strlen(ptr) : 0;
  }

  // Deliberately implicit to avoid calling on every string literal.
  // |ch| must be an lvalue that outlives the the CFX_ByteStringC.
  CFX_ByteStringC(FX_CHAR& ch) {
    m_Ptr = (const uint8_t*)&ch;
    m_Length = 1;
  }

  CFX_ByteStringC(const FX_CHAR* ptr, FX_STRSIZE len) {
    m_Ptr = (const uint8_t*)ptr;
    m_Length = (len == -1) ? FXSYS_strlen(ptr) : len;
  }

  CFX_ByteStringC(const CFX_ByteStringC& src) {
    m_Ptr = src.m_Ptr;
    m_Length = src.m_Length;
  }

  CFX_ByteStringC& operator=(const FX_CHAR* src) {
    m_Ptr = (const uint8_t*)src;
    m_Length = m_Ptr ? FXSYS_strlen(src) : 0;
    return *this;
  }

  CFX_ByteStringC& operator=(const CFX_ByteStringC& src) {
    m_Ptr = src.m_Ptr;
    m_Length = src.m_Length;
    return *this;
  }

  bool operator==(const char* ptr) const {
    return FXSYS_strlen(ptr) == m_Length &&
           FXSYS_memcmp(ptr, m_Ptr, m_Length) == 0;
  }
  bool operator==(const CFX_ByteStringC& other) const {
    return other.m_Length == m_Length &&
           FXSYS_memcmp(other.m_Ptr, m_Ptr, m_Length) == 0;
  }
  bool operator!=(const char* ptr) const { return !(*this == ptr); }
  bool operator!=(const CFX_ByteStringC& other) const {
    return !(*this == other);
  }

  uint32_t GetID(FX_STRSIZE start_pos = 0) const;

  const uint8_t* raw_str() const { return m_Ptr; }
  const FX_CHAR* c_str() const {
    return reinterpret_cast<const FX_CHAR*>(m_Ptr);
  }

  FX_STRSIZE GetLength() const { return m_Length; }
  bool IsEmpty() const { return m_Length == 0; }

  uint8_t GetAt(FX_STRSIZE index) const { return m_Ptr[index]; }
  FX_CHAR CharAt(FX_STRSIZE index) const {
    return static_cast<FX_CHAR>(m_Ptr[index]);
  }

  FX_STRSIZE Find(FX_CHAR ch) const {
    const uint8_t* found =
        static_cast<const uint8_t*>(memchr(m_Ptr, ch, m_Length));
    return found ? found - m_Ptr : -1;
  }

  CFX_ByteStringC Mid(FX_STRSIZE index, FX_STRSIZE count = -1) const {
    if (index < 0) {
      index = 0;
    }
    if (index > m_Length) {
      return CFX_ByteStringC();
    }
    if (count < 0 || count > m_Length - index) {
      count = m_Length - index;
    }
    return CFX_ByteStringC(m_Ptr + index, count);
  }

  const uint8_t& operator[](size_t index) const { return m_Ptr[index]; }

  bool operator<(const CFX_ByteStringC& that) const {
    int result = memcmp(m_Ptr, that.m_Ptr, std::min(m_Length, that.m_Length));
    return result < 0 || (result == 0 && m_Length < that.m_Length);
  }

 protected:
  const uint8_t* m_Ptr;
  FX_STRSIZE m_Length;

 private:
  void* operator new(size_t) throw() { return NULL; }
};
inline bool operator==(const char* lhs, const CFX_ByteStringC& rhs) {
  return rhs == lhs;
}
inline bool operator!=(const char* lhs, const CFX_ByteStringC& rhs) {
  return rhs != lhs;
}
#define FXBSTR_ID(c1, c2, c3, c4)                                      \
  (((uint32_t)c1 << 24) | ((uint32_t)c2 << 16) | ((uint32_t)c3 << 8) | \
   ((uint32_t)c4))

// A mutable string with shared buffers using copy-on-write semantics that
// avoids the cost of std::string's iterator stability guarantees.
class CFX_ByteString {
 public:
  typedef FX_CHAR value_type;

  CFX_ByteString() {}
  CFX_ByteString(const CFX_ByteString& other) : m_pData(other.m_pData) {}
  CFX_ByteString(CFX_ByteString&& other) { m_pData.Swap(other.m_pData); }

  // Deliberately implicit to avoid calling on every string literal.
  CFX_ByteString(char ch);
  CFX_ByteString(const FX_CHAR* ptr)
      : CFX_ByteString(ptr, ptr ? FXSYS_strlen(ptr) : 0) {}

  CFX_ByteString(const FX_CHAR* ptr, FX_STRSIZE len);
  CFX_ByteString(const uint8_t* ptr, FX_STRSIZE len);

  // TODO(tsepez): mark constructor as explicit.
  CFX_ByteString(const CFX_ByteStringC& bstrc);
  CFX_ByteString(const CFX_ByteStringC& bstrc1, const CFX_ByteStringC& bstrc2);

  ~CFX_ByteString();

  void clear() { m_pData.Reset(); }

  static CFX_ByteString FromUnicode(const FX_WCHAR* ptr, FX_STRSIZE len = -1);
  static CFX_ByteString FromUnicode(const CFX_WideString& str);

  // Explicit conversion to C-style string.
  // Note: Any subsequent modification of |this| will invalidate the result.
  const FX_CHAR* c_str() const { return m_pData ? m_pData->m_String : ""; }

  // Explicit conversion to uint8_t*.
  // Note: Any subsequent modification of |this| will invalidate the result.
  const uint8_t* raw_str() const {
    return m_pData ? reinterpret_cast<const uint8_t*>(m_pData->m_String)
                   : nullptr;
  }

  // Explicit conversion to CFX_ByteStringC.
  // Note: Any subsequent modification of |this| will invalidate the result.
  CFX_ByteStringC AsStringC() const {
    return CFX_ByteStringC(raw_str(), GetLength());
  }

  FX_STRSIZE GetLength() const { return m_pData ? m_pData->m_nDataLength : 0; }
  bool IsEmpty() const { return !GetLength(); }

  int Compare(const CFX_ByteStringC& str) const;
  bool EqualNoCase(const CFX_ByteStringC& str) const;

  bool operator==(const char* ptr) const;
  bool operator==(const CFX_ByteStringC& str) const;
  bool operator==(const CFX_ByteString& other) const;

  bool operator!=(const char* ptr) const { return !(*this == ptr); }
  bool operator!=(const CFX_ByteStringC& str) const { return !(*this == str); }
  bool operator!=(const CFX_ByteString& other) const {
    return !(*this == other);
  }

  bool operator<(const CFX_ByteString& str) const {
    int result = FXSYS_memcmp(c_str(), str.c_str(),
                              std::min(GetLength(), str.GetLength()));
    return result < 0 || (result == 0 && GetLength() < str.GetLength());
  }

  const CFX_ByteString& operator=(const FX_CHAR* str);
  const CFX_ByteString& operator=(const CFX_ByteStringC& bstrc);
  const CFX_ByteString& operator=(const CFX_ByteString& stringSrc);

  const CFX_ByteString& operator+=(FX_CHAR ch);
  const CFX_ByteString& operator+=(const FX_CHAR* str);
  const CFX_ByteString& operator+=(const CFX_ByteString& str);
  const CFX_ByteString& operator+=(const CFX_ByteStringC& bstrc);

  uint8_t GetAt(FX_STRSIZE nIndex) const {
    return m_pData ? m_pData->m_String[nIndex] : 0;
  }

  uint8_t operator[](FX_STRSIZE nIndex) const {
    return m_pData ? m_pData->m_String[nIndex] : 0;
  }

  void SetAt(FX_STRSIZE nIndex, FX_CHAR ch);
  FX_STRSIZE Insert(FX_STRSIZE index, FX_CHAR ch);
  FX_STRSIZE Delete(FX_STRSIZE index, FX_STRSIZE count = 1);

  void Format(const FX_CHAR* lpszFormat, ...);
  void FormatV(const FX_CHAR* lpszFormat, va_list argList);

  void Reserve(FX_STRSIZE len);
  FX_CHAR* GetBuffer(FX_STRSIZE len);
  void ReleaseBuffer(FX_STRSIZE len = -1);

  CFX_ByteString Mid(FX_STRSIZE first) const;
  CFX_ByteString Mid(FX_STRSIZE first, FX_STRSIZE count) const;
  CFX_ByteString Left(FX_STRSIZE count) const;
  CFX_ByteString Right(FX_STRSIZE count) const;

  FX_STRSIZE Find(const CFX_ByteStringC& lpszSub, FX_STRSIZE start = 0) const;
  FX_STRSIZE Find(FX_CHAR ch, FX_STRSIZE start = 0) const;
  FX_STRSIZE ReverseFind(FX_CHAR ch) const;

  void MakeLower();
  void MakeUpper();

  void TrimRight();
  void TrimRight(FX_CHAR chTarget);
  void TrimRight(const CFX_ByteStringC& lpszTargets);

  void TrimLeft();
  void TrimLeft(FX_CHAR chTarget);
  void TrimLeft(const CFX_ByteStringC& lpszTargets);

  FX_STRSIZE Replace(const CFX_ByteStringC& lpszOld,
                     const CFX_ByteStringC& lpszNew);

  FX_STRSIZE Remove(FX_CHAR ch);

  CFX_WideString UTF8Decode() const;

  uint32_t GetID(FX_STRSIZE start_pos = 0) const;

#define FXFORMAT_SIGNED 1
#define FXFORMAT_HEX 2
#define FXFORMAT_CAPITAL 4

  static CFX_ByteString FormatInteger(int i, uint32_t flags = 0);
  static CFX_ByteString FormatFloat(FX_FLOAT f, int precision = 0);

 protected:
  using StringData = CFX_StringDataTemplate<FX_CHAR>;

  void ReallocBeforeWrite(FX_STRSIZE nNewLen);
  void AllocBeforeWrite(FX_STRSIZE nNewLen);
  void AllocCopy(CFX_ByteString& dest,
                 FX_STRSIZE nCopyLen,
                 FX_STRSIZE nCopyIndex) const;
  void AssignCopy(const FX_CHAR* pSrcData, FX_STRSIZE nSrcLen);
  void Concat(const FX_CHAR* lpszSrcData, FX_STRSIZE nSrcLen);

  CFX_RetainPtr<StringData> m_pData;
  friend class fxcrt_ByteStringConcat_Test;
};

inline bool operator==(const char* lhs, const CFX_ByteString& rhs) {
  return rhs == lhs;
}
inline bool operator==(const CFX_ByteStringC& lhs, const CFX_ByteString& rhs) {
  return rhs == lhs;
}
inline bool operator!=(const char* lhs, const CFX_ByteString& rhs) {
  return rhs != lhs;
}
inline bool operator!=(const CFX_ByteStringC& lhs, const CFX_ByteString& rhs) {
  return rhs != lhs;
}

inline CFX_ByteString operator+(const CFX_ByteStringC& str1,
                                const CFX_ByteStringC& str2) {
  return CFX_ByteString(str1, str2);
}
inline CFX_ByteString operator+(const CFX_ByteStringC& str1,
                                const FX_CHAR* str2) {
  return CFX_ByteString(str1, str2);
}
inline CFX_ByteString operator+(const FX_CHAR* str1,
                                const CFX_ByteStringC& str2) {
  return CFX_ByteString(str1, str2);
}
inline CFX_ByteString operator+(const CFX_ByteStringC& str1, FX_CHAR ch) {
  return CFX_ByteString(str1, CFX_ByteStringC(ch));
}
inline CFX_ByteString operator+(FX_CHAR ch, const CFX_ByteStringC& str2) {
  return CFX_ByteString(ch, str2);
}
inline CFX_ByteString operator+(const CFX_ByteString& str1,
                                const CFX_ByteString& str2) {
  return CFX_ByteString(str1.AsStringC(), str2.AsStringC());
}
inline CFX_ByteString operator+(const CFX_ByteString& str1, FX_CHAR ch) {
  return CFX_ByteString(str1.AsStringC(), CFX_ByteStringC(ch));
}
inline CFX_ByteString operator+(FX_CHAR ch, const CFX_ByteString& str2) {
  return CFX_ByteString(ch, str2.AsStringC());
}
inline CFX_ByteString operator+(const CFX_ByteString& str1,
                                const FX_CHAR* str2) {
  return CFX_ByteString(str1.AsStringC(), str2);
}
inline CFX_ByteString operator+(const FX_CHAR* str1,
                                const CFX_ByteString& str2) {
  return CFX_ByteString(str1, str2.AsStringC());
}
inline CFX_ByteString operator+(const CFX_ByteString& str1,
                                const CFX_ByteStringC& str2) {
  return CFX_ByteString(str1.AsStringC(), str2);
}
inline CFX_ByteString operator+(const CFX_ByteStringC& str1,
                                const CFX_ByteString& str2) {
  return CFX_ByteString(str1, str2.AsStringC());
}

class CFX_WideStringC {
 public:
  typedef FX_WCHAR value_type;

  CFX_WideStringC() {
    m_Ptr = NULL;
    m_Length = 0;
  }

  // Deliberately implicit to avoid calling on every string literal.
  CFX_WideStringC(const FX_WCHAR* ptr) {
    m_Ptr = ptr;
    m_Length = ptr ? FXSYS_wcslen(ptr) : 0;
  }

  // Deliberately implicit to avoid calling on every string literal.
  // |ch| must be an lvalue that outlives the the CFX_WideStringC.
  CFX_WideStringC(FX_WCHAR& ch) {
    m_Ptr = &ch;
    m_Length = 1;
  }

  CFX_WideStringC(const FX_WCHAR* ptr, FX_STRSIZE len) {
    m_Ptr = ptr;
    m_Length = (len == -1) ? FXSYS_wcslen(ptr) : len;
  }

  CFX_WideStringC(const CFX_WideStringC& src) {
    m_Ptr = src.m_Ptr;
    m_Length = src.m_Length;
  }

  CFX_WideStringC& operator=(const FX_WCHAR* src) {
    m_Ptr = src;
    m_Length = FXSYS_wcslen(src);
    return *this;
  }

  CFX_WideStringC& operator=(const CFX_WideStringC& src) {
    m_Ptr = src.m_Ptr;
    m_Length = src.m_Length;
    return *this;
  }

  bool operator==(const wchar_t* ptr) const {
    return FXSYS_wcslen(ptr) == m_Length && wmemcmp(ptr, m_Ptr, m_Length) == 0;
  }
  bool operator==(const CFX_WideStringC& str) const {
    return str.m_Length == m_Length && wmemcmp(str.m_Ptr, m_Ptr, m_Length) == 0;
  }
  bool operator!=(const wchar_t* ptr) const { return !(*this == ptr); }
  bool operator!=(const CFX_WideStringC& str) const { return !(*this == str); }

  const FX_WCHAR* c_str() const { return m_Ptr; }

  FX_STRSIZE GetLength() const { return m_Length; }
  bool IsEmpty() const { return m_Length == 0; }

  FX_WCHAR GetAt(FX_STRSIZE index) const { return m_Ptr[index]; }

  CFX_WideStringC Left(FX_STRSIZE count) const {
    if (count < 1) {
      return CFX_WideStringC();
    }
    if (count > m_Length) {
      count = m_Length;
    }
    return CFX_WideStringC(m_Ptr, count);
  }

  FX_STRSIZE Find(FX_WCHAR ch) const {
    const FX_WCHAR* found =
        static_cast<const FX_WCHAR*>(wmemchr(m_Ptr, ch, m_Length));
    return found ? found - m_Ptr : -1;
  }

  CFX_WideStringC Mid(FX_STRSIZE index, FX_STRSIZE count = -1) const {
    if (index < 0) {
      index = 0;
    }
    if (index > m_Length) {
      return CFX_WideStringC();
    }
    if (count < 0 || count > m_Length - index) {
      count = m_Length - index;
    }
    return CFX_WideStringC(m_Ptr + index, count);
  }

  CFX_WideStringC Right(FX_STRSIZE count) const {
    if (count < 1) {
      return CFX_WideStringC();
    }
    if (count > m_Length) {
      count = m_Length;
    }
    return CFX_WideStringC(m_Ptr + m_Length - count, count);
  }

  const FX_WCHAR& operator[](size_t index) const { return m_Ptr[index]; }

  bool operator<(const CFX_WideStringC& that) const {
    int result = wmemcmp(m_Ptr, that.m_Ptr, std::min(m_Length, that.m_Length));
    return result < 0 || (result == 0 && m_Length < that.m_Length);
  }

 protected:
  const FX_WCHAR* m_Ptr;
  FX_STRSIZE m_Length;

 private:
  void* operator new(size_t) throw() { return NULL; }
};

inline bool operator==(const wchar_t* lhs, const CFX_WideStringC& rhs) {
  return rhs == lhs;
}
inline bool operator!=(const wchar_t* lhs, const CFX_WideStringC& rhs) {
  return rhs != lhs;
}
#define FX_WSTRC(wstr) CFX_WideStringC(wstr, FX_ArraySize(wstr) - 1)

// A mutable string with shared buffers using copy-on-write semantics that
// avoids the cost of std::string's iterator stability guarantees.
class CFX_WideString {
 public:
  typedef FX_WCHAR value_type;

  CFX_WideString() {}
  CFX_WideString(const CFX_WideString& other) : m_pData(other.m_pData) {}
  CFX_WideString(CFX_WideString&& other) { m_pData.Swap(other.m_pData); }

  // Deliberately implicit to avoid calling on every string literal.
  CFX_WideString(FX_WCHAR ch);
  CFX_WideString(const FX_WCHAR* ptr)
      : CFX_WideString(ptr, ptr ? FXSYS_wcslen(ptr) : 0) {}

  CFX_WideString(const FX_WCHAR* ptr, FX_STRSIZE len);

  // TODO(tsepez): mark constructor as explicit.
  CFX_WideString(const CFX_WideStringC& str);
  CFX_WideString(const CFX_WideStringC& str1, const CFX_WideStringC& str2);

  ~CFX_WideString();

  static CFX_WideString FromLocal(const CFX_ByteStringC& str);
  static CFX_WideString FromCodePage(const CFX_ByteStringC& str,
                                     uint16_t codepage);

  static CFX_WideString FromUTF8(const CFX_ByteStringC& str);
  static CFX_WideString FromUTF16LE(const unsigned short* str, FX_STRSIZE len);

  static FX_STRSIZE WStringLength(const unsigned short* str);

  // Explicit conversion to C-style wide string.
  // Note: Any subsequent modification of |this| will invalidate the result.
  const FX_WCHAR* c_str() const { return m_pData ? m_pData->m_String : L""; }

  // Explicit conversion to CFX_WideStringC.
  // Note: Any subsequent modification of |this| will invalidate the result.
  CFX_WideStringC AsStringC() const {
    return CFX_WideStringC(c_str(), GetLength());
  }

  void clear() { m_pData.Reset(); }

  FX_STRSIZE GetLength() const { return m_pData ? m_pData->m_nDataLength : 0; }
  bool IsEmpty() const { return !GetLength(); }

  const CFX_WideString& operator=(const FX_WCHAR* str);
  const CFX_WideString& operator=(const CFX_WideString& stringSrc);
  const CFX_WideString& operator=(const CFX_WideStringC& stringSrc);

  const CFX_WideString& operator+=(const FX_WCHAR* str);
  const CFX_WideString& operator+=(FX_WCHAR ch);
  const CFX_WideString& operator+=(const CFX_WideString& str);
  const CFX_WideString& operator+=(const CFX_WideStringC& str);

  bool operator==(const wchar_t* ptr) const;
  bool operator==(const CFX_WideStringC& str) const;
  bool operator==(const CFX_WideString& other) const;

  bool operator!=(const wchar_t* ptr) const { return !(*this == ptr); }
  bool operator!=(const CFX_WideStringC& str) const { return !(*this == str); }
  bool operator!=(const CFX_WideString& other) const {
    return !(*this == other);
  }

  bool operator<(const CFX_WideString& str) const {
    int result =
        wmemcmp(c_str(), str.c_str(), std::min(GetLength(), str.GetLength()));
    return result < 0 || (result == 0 && GetLength() < str.GetLength());
  }

  FX_WCHAR GetAt(FX_STRSIZE nIndex) const {
    return m_pData ? m_pData->m_String[nIndex] : 0;
  }

  FX_WCHAR operator[](FX_STRSIZE nIndex) const {
    return m_pData ? m_pData->m_String[nIndex] : 0;
  }

  void SetAt(FX_STRSIZE nIndex, FX_WCHAR ch);

  int Compare(const FX_WCHAR* str) const;
  int Compare(const CFX_WideString& str) const;
  int CompareNoCase(const FX_WCHAR* str) const;


  CFX_WideString Mid(FX_STRSIZE first) const;
  CFX_WideString Mid(FX_STRSIZE first, FX_STRSIZE count) const;
  CFX_WideString Left(FX_STRSIZE count) const;
  CFX_WideString Right(FX_STRSIZE count) const;

  FX_STRSIZE Insert(FX_STRSIZE index, FX_WCHAR ch);
  FX_STRSIZE Delete(FX_STRSIZE index, FX_STRSIZE count = 1);

  void Format(const FX_WCHAR* lpszFormat, ...);
  void FormatV(const FX_WCHAR* lpszFormat, va_list argList);

  void MakeLower();
  void MakeUpper();

  void TrimRight();
  void TrimRight(FX_WCHAR chTarget);
  void TrimRight(const CFX_WideStringC& pTargets);

  void TrimLeft();
  void TrimLeft(FX_WCHAR chTarget);
  void TrimLeft(const CFX_WideStringC& pTargets);

  void Reserve(FX_STRSIZE len);
  FX_WCHAR* GetBuffer(FX_STRSIZE len);
  void ReleaseBuffer(FX_STRSIZE len = -1);

  int GetInteger() const;
  FX_FLOAT GetFloat() const;

  FX_STRSIZE Find(const CFX_WideStringC& pSub, FX_STRSIZE start = 0) const;
  FX_STRSIZE Find(FX_WCHAR ch, FX_STRSIZE start = 0) const;
  FX_STRSIZE Replace(const CFX_WideStringC& pOld, const CFX_WideStringC& pNew);
  FX_STRSIZE Remove(FX_WCHAR ch);

  CFX_ByteString UTF8Encode() const;
  CFX_ByteString UTF16LE_Encode() const;

 protected:
  using StringData = CFX_StringDataTemplate<FX_WCHAR>;

  void ReallocBeforeWrite(FX_STRSIZE nLen);
  void AllocBeforeWrite(FX_STRSIZE nLen);
  void AllocCopy(CFX_WideString& dest,
                 FX_STRSIZE nCopyLen,
                 FX_STRSIZE nCopyIndex) const;
  void AssignCopy(const FX_WCHAR* pSrcData, FX_STRSIZE nSrcLen);
  void Concat(const FX_WCHAR* lpszSrcData, FX_STRSIZE nSrcLen);

  CFX_RetainPtr<StringData> m_pData;
  friend class fxcrt_WideStringConcatInPlace_Test;
};

inline CFX_WideString operator+(const CFX_WideStringC& str1,
                                const CFX_WideStringC& str2) {
  return CFX_WideString(str1, str2);
}
inline CFX_WideString operator+(const CFX_WideStringC& str1,
                                const FX_WCHAR* str2) {
  return CFX_WideString(str1, str2);
}
inline CFX_WideString operator+(const FX_WCHAR* str1,
                                const CFX_WideStringC& str2) {
  return CFX_WideString(str1, str2);
}
inline CFX_WideString operator+(const CFX_WideStringC& str1, FX_WCHAR ch) {
  return CFX_WideString(str1, CFX_WideStringC(ch));
}
inline CFX_WideString operator+(FX_WCHAR ch, const CFX_WideStringC& str2) {
  return CFX_WideString(ch, str2);
}
inline CFX_WideString operator+(const CFX_WideString& str1,
                                const CFX_WideString& str2) {
  return CFX_WideString(str1.AsStringC(), str2.AsStringC());
}
inline CFX_WideString operator+(const CFX_WideString& str1, FX_WCHAR ch) {
  return CFX_WideString(str1.AsStringC(), CFX_WideStringC(ch));
}
inline CFX_WideString operator+(FX_WCHAR ch, const CFX_WideString& str2) {
  return CFX_WideString(ch, str2.AsStringC());
}
inline CFX_WideString operator+(const CFX_WideString& str1,
                                const FX_WCHAR* str2) {
  return CFX_WideString(str1.AsStringC(), str2);
}
inline CFX_WideString operator+(const FX_WCHAR* str1,
                                const CFX_WideString& str2) {
  return CFX_WideString(str1, str2.AsStringC());
}
inline CFX_WideString operator+(const CFX_WideString& str1,
                                const CFX_WideStringC& str2) {
  return CFX_WideString(str1.AsStringC(), str2);
}
inline CFX_WideString operator+(const CFX_WideStringC& str1,
                                const CFX_WideString& str2) {
  return CFX_WideString(str1, str2.AsStringC());
}
inline bool operator==(const wchar_t* lhs, const CFX_WideString& rhs) {
  return rhs == lhs;
}
inline bool operator==(const CFX_WideStringC& lhs, const CFX_WideString& rhs) {
  return rhs == lhs;
}
inline bool operator!=(const wchar_t* lhs, const CFX_WideString& rhs) {
  return rhs != lhs;
}
inline bool operator!=(const CFX_WideStringC& lhs, const CFX_WideString& rhs) {
  return rhs != lhs;
}

CFX_ByteString FX_UTF8Encode(const FX_WCHAR* pwsStr, FX_STRSIZE len);
inline CFX_ByteString FX_UTF8Encode(const CFX_WideStringC& wsStr) {
  return FX_UTF8Encode(wsStr.c_str(), wsStr.GetLength());
}
inline CFX_ByteString FX_UTF8Encode(const CFX_WideString& wsStr) {
  return FX_UTF8Encode(wsStr.c_str(), wsStr.GetLength());
}

FX_FLOAT FX_atof(const CFX_ByteStringC& str);
inline FX_FLOAT FX_atof(const CFX_WideStringC& wsStr) {
  return FX_atof(FX_UTF8Encode(wsStr.c_str(), wsStr.GetLength()).c_str());
}
void FX_atonum(const CFX_ByteStringC& str, FX_BOOL& bInteger, void* pData);
FX_STRSIZE FX_ftoa(FX_FLOAT f, FX_CHAR* buf);

#endif  // CORE_FXCRT_INCLUDE_FX_STRING_H_
