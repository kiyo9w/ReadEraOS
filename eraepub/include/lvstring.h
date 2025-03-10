/** \file lvstring.h
    \brief string classes interface

    CoolReader Engine

    (c) Vadim Lopatin, 2000-2006
    Copyright (C) 2013-2020 READERA LLC

    This source code is distributed under the terms of
    GNU General Public License.

    See LICENSE file for details.
*/

#ifndef __LV_STRING_H_INCLUDED__
#define __LV_STRING_H_INCLUDED__

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <vector>
#include "erae_log.h"
#include "lvtypes.h"
#include "serialBuf.h"
#include "lStringCollection.h"
#include "dvngLig.h"

#define LCSTR(x) (UnicodeToUtf8(x).c_str())
#define UNICODE_SOFT_HYPHEN_CODE  0x00ad
#define UNICODE_ZERO_WIDTH_SPACE  0x200b
#define UNICODE_NO_BREAK_SPACE    0x00a0
#define UNICODE_HYPHEN            0x2010
#define UNICODE_NB_HYPHEN         0x2011
#define UNICODE_IDEOGRAPHIC_SPACE 0x3000
#define UNICODE_EM_QUAD_SPACE     0x2001

// must be power of 2
#define CONST_STRING_BUFFER_SIZE 4096
#define CONST_STRING_BUFFER_MASK (CONST_STRING_BUFFER_SIZE - 1)
#define CONST_STRING_BUFFER_HASH_MULT 31

// disable CJK support since it breaks usual text formatting with floating punctuation and space trunctaion turned on
#define CJK_PATCH 1

/// typed realloc with result check (size is counted in T), fatal error if failed
template <typename T> T * cr_realloc(T * ptr, size_t new_size) {
    T * newptr = reinterpret_cast<T*>(realloc(ptr, sizeof(T)*new_size));
    if (newptr) {
        return newptr;
    }
    free(ptr); // to bypass cppcheck warning
    crFatalError(-2, "realloc failed");
    return NULL;
}

/// strlen for lChar16
int lStr_len(const lChar16 * str);
/// strlen for lChar8
int lStr_len(const lChar8 * str);
/// strnlen for lChar16
int lStr_nlen(const lChar16 * str, int maxcount);
/// strnlen for lChar8
int lStr_nlen(const lChar8 * str, int maxcount);
/// strcpy for lChar16
int lStr_cpy(lChar16 * dst, const lChar16 * src);
/// strcpy for lChar16 -> lChar8
int lStr_cpy(lChar16 * dst, const lChar8 * src);
/// strcpy for lChar8
int lStr_cpy(lChar8 * dst, const lChar8 * src);
/// strncpy for lChar16
int lStr_ncpy(lChar16 * dst, const lChar16 * src, int maxcount);
/// strncpy for lChar8
int lStr_ncpy(lChar8 * dst, const lChar8 * src, int maxcount);
/// memcpy for lChar16
void lStr_memcpy(lChar16 * dst, const lChar16 * src, int count);
/// memcpy for lChar8
void lStr_memcpy(lChar8 * dst, const lChar8 * src, int count);
/// memset for lChar16
void lStr_memset(lChar16 * dst, lChar16 value, int count);
/// memset for lChar8
void lStr_memset(lChar8 * dst, lChar8 value, int count);
/// strcmp for lChar16
int lStr_cmp(const lChar16 * str1, const lChar16 * str2);
/// strcmp for lChar16 <> lChar8
int lStr_cmp(const lChar16 * str1, const lChar8 * str2);
/// strcmp for lChar8 <> lChar16
int lStr_cmp(const lChar8 * str1, const lChar16 * str2);
/// strcmp for lChar8
int lStr_cmp(const lChar8 * str1, const lChar8 * str2);
/// convert string to uppercase
void lStr_uppercase( lChar16 * str, int len );
/// convert string to lowercase
void lStr_lowercase( lChar16 * str, int len );
/// calculates CRC32 for buffer contents
lUInt32 lStr_crc32( lUInt32 prevValue, const void * buf, int size );

// converts 0..15 to 0..f
char toHexDigit( int c );
// returns 0..15 if c is hex digit, -1 otherwise
int hexDigit( int c );
// decode LEN hex digits, return decoded number, -1 if invalid
int decodeHex( const lChar16 * str, int len );
// decode LEN decimal digits, return decoded number, -1 if invalid
int decodeDecimal( const lChar16 * str, int len );


/// retrieve character properties mask array for wide c-string
void lStr_getCharProps( const lChar16 * str, int sz, lUInt16 * props );
/// retrieve character properties mask for single wide character
lUInt16 lGetCharProps( lChar16 ch );
/// find alpha sequence bounds
void lStr_findWordBounds( const lChar16 * str, int sz, int pos, int & start, int & end );

struct lstring8_chunk_t {
    friend class lString8;
    friend class lString16;
    friend struct lstring_chunk_slice_t;
public:
    lstring8_chunk_t(lChar8 * _buf8) : buf8(_buf8), size(1), len(0), nref(1) {}
    const lChar8 * data8() const { return buf8; }
private:
    lChar8* buf8; 	// Z-string
    lInt32 size;  	// 0 for free chunk
    lInt32 len;   	// Count of chars in string
    int nref;     	// Reference counter

    lstring8_chunk_t() {}

    // Chunk allocation functions
    static lstring8_chunk_t* alloc();
    static void free(lstring8_chunk_t* pChunk);
};

struct lstring16_chunk_t {
    friend class lString8;
    friend class lString16;
    friend struct lstring_chunk_slice_t;
public:
    lstring16_chunk_t(lChar16* _buf16) : buf16(_buf16), size(1), len(0), nref(1) {}
    const lChar16* data16() const { return buf16; }
private:
    lChar16* buf16; // z-string
    lInt32 size;   	// 0 for free chunk
    lInt32 len;    	// count of chars in string
    int nref;      	// reference counter

    lstring16_chunk_t() {}

    // Chunk allocation functions
    static lstring16_chunk_t* alloc();
    static void free(lstring16_chunk_t* pChunk);
};

namespace fmt {
    class decimal {
        lInt64 value;
    public:
        explicit decimal(lInt64 v) : value(v) { }
        lInt64 get() const { return value; }
    };

    class hex {
        lUInt64 value;
    public:
        explicit hex(lInt64 v) : value(v) { }
        lUInt64 get() const { return value; }
    };
}

/**
    \brief lChar8 string

    Reference counting, copy-on-write implementation of 8-bit string.
    Interface is similar to STL strings

*/
class lString8Collection;
class lString8
{
    friend class lString8Collection;
    friend class lString16Collection;
    friend const lString8 & cs8(const char * str);
public:
    // typedefs for STL compatibility
    typedef lChar8              value_type;      ///< character type
    typedef int                 size_type;       ///< size type
    typedef int                 difference_type; ///< difference type
    typedef value_type *        pointer;         ///< pointer to char type
    typedef value_type &        reference;       ///< reference to char type
    typedef const value_type *  const_pointer;   ///< pointer to const char type
    typedef const value_type &  const_reference; ///< reference to const char type

    typedef lstring8_chunk_t    lstring_chunk_t; ///< data container

    class decimal {
        lInt64 value;
    public:
        explicit decimal(lInt64 v) : value(v) { }
        lInt64 get() { return value; }
    };

    class hex {
        lUInt64 value;
    public:
        explicit hex(lInt64 v) : value(v) { }
        lUInt64 get() { return value; }
    };

private:
    lstring_chunk_t * pchunk;
    static lstring_chunk_t * EMPTY_STR_8;
    void alloc(size_type sz);
    void free();
    inline void addref() const { ++pchunk->nref; }
    inline void release() { if (--pchunk->nref==0) free(); }
    explicit lString8(lstring_chunk_t * chunk) : pchunk(chunk) { addref(); }
public:
    /// default constrictor
    explicit lString8() : pchunk(EMPTY_STR_8) { addref(); }
    /// constructor of empty string with buffer of specified size
    explicit lString8( int size ) : pchunk(EMPTY_STR_8) { addref(); reserve(size); }
    /// copy constructor
    lString8(const lString8 & str) : pchunk(str.pchunk) { addref(); }
    /// constructor from C string
    explicit lString8(const value_type * str);
    /// constructor from 16-bit C string
    explicit lString8(const lChar16 * str);
    /// constructor from string of specified length
    explicit lString8(const value_type * str, size_type count);
    /// fragment copy constructor
    explicit lString8(const lString8 & str, size_type offset, size_type count);
    /// destructor
    ~lString8() { release(); }

    /// copy assignment
    lString8 & assign(const lString8 & str)
    {
        if (pchunk!=str.pchunk)
        {
            release();
            pchunk = str.pchunk;
            addref();
        }
        return *this;
    }
    /// C-string assignment
    lString8 & assign(const value_type * str);
    /// C-string fragment assignment
    lString8 & assign(const value_type * str, size_type count);
    /// string fragment assignment
    lString8 & assign(const lString8 & str, size_type offset, size_type count);
    /// C-string assignment
    lString8 & operator = (const value_type * str) { return assign(str); }
    /// string copy assignment
    lString8 & operator = (const lString8 & str) { return assign(str); }
    /// erase part of string
    lString8 & erase(size_type offset, size_type count);
    /// append C-string
    lString8 & append(const value_type * str);
    /// append C-string fragment
    lString8 & append(const value_type * str, size_type count);
    /// append string
    lString8 & append(const lString8 & str);
    /// append string fragment
    lString8 & append(const lString8 & str, size_type offset, size_type count);
    /// append repeated character
    lString8 & append(size_type count, value_type ch);
    /// append decimal number
    lString8 & appendDecimal(lInt64 v);
    /// append hex number
    lString8 & appendHex(lUInt64 v);
    /// insert C-string
    lString8 & insert(size_type p0, const value_type * str);
    /// insert C-string fragment
    lString8 & insert(size_type p0, const value_type * str, size_type count);
    /// insert string
    lString8 & insert(size_type p0, const lString8 & str);
    /// insert string fragment
    lString8 & insert(size_type p0, const lString8 & str, size_type offset, size_type count);
    /// insert repeated character
    lString8 & insert(size_type p0, size_type count, value_type ch);
    /// replace fragment with C-string
    lString8 & replace(size_type p0, size_type n0, const value_type * str);
    /// replace fragment with C-string fragment
    lString8 & replace(size_type p0, size_type n0, const value_type * str, size_type count);
    /// replace fragment with string
    lString8 & replace(size_type p0, size_type n0, const lString8 & str);
    /// replace fragment with string fragment
    lString8 & replace(size_type p0, size_type n0, const lString8 & str, size_type offset, size_type count);
    /// replace fragment with repeated character
    lString8 & replace(size_type p0, size_type n0, size_type count, value_type ch);
    /// compare with another string
    int compare(const lString8& str) const { return lStr_cmp(pchunk->buf8, str.pchunk->buf8); }
    /// compare part of string with another string
    int compare(size_type p0, size_type n0, const lString8& str) const;
    /// compare part of string with fragment of another string
    int compare(size_type p0, size_type n0, const lString8& str, size_type pos, size_type n) const;
    /// compare with C-string
    int compare(const value_type *s) const  { return lStr_cmp(pchunk->buf8, s); }
    /// compare part of string with C-string
    int compare(size_type p0, size_type n0, const value_type *s) const;
    /// compare part of string with C-string fragment
    int compare(size_type p0, size_type n0, const value_type *s, size_type pos) const;
    /// find position of substring inside string, -1 if not found
    int pos(const lString8 & subStr) const;
    /// find position of substring inside string, -1 if not found
    int pos(const char * subStr) const;
    /// find position of substring inside string starting from right, -1 if not found
    int rpos(const char * subStr) const;
    /// find position of substring inside string starting from specified position, -1 if not found
    int pos(const lString8 & subStr, int startPos) const;
    /// find position of substring inside string starting from specified position, -1 if not found
    int pos(const char * subStr, int startPos) const;

    /// substring
    lString8 substr(size_type pos, size_type n) const;
    /// substring from position to end of string
    lString8 substr(size_type pos) const { return substr(pos, length() - pos); }

    /// append single character
    lString8 & operator << (value_type ch) { return append(1, ch); }
    /// append C-string
    lString8 & operator << (const value_type * str) { return append(str); }
    /// append string
    lString8 & operator << (const lString8 & str) { return append(str); }
    /// append decimal number
    lString8 & operator << (const fmt::decimal v) { return appendDecimal(v.get()); }
    /// append hex number
    lString8 & operator << (const fmt::hex v) { return appendHex(v.get()); }

    /// returns true if string starts with specified substring
    bool startsWith ( const lString8 & substring ) const;
    /// returns true if string starts with specified substring
    bool startsWith ( const char * substring ) const;

    /// returns last character
    value_type lastChar() { return empty() ? 0 : at(length()-1); }
    /// returns first character
    value_type firstChar() { return empty() ? 0 : at(0); }

    /// calculate hash
    lUInt32 getHash() const;

    /// get character at specified position with range check
    value_type & at(size_type pos) {
        if (pos > pchunk->len)
            crFatalError();
        return modify()[pos];
    }
    /// get character at specified position without range check
    value_type operator [] ( size_type pos ) const { return pchunk->buf8[pos]; }

    /// get reference to character at specified position
    value_type & operator [] (size_type pos) { return modify()[pos]; }

    /// ensures that reference count is 1
    void  lock( size_type newsize );
    /// returns pointer to modifable string buffer
    value_type * modify() { if (pchunk->nref>1) lock(pchunk->len); return pchunk->buf8; }
    /// clear string
    void  clear() { release(); pchunk = EMPTY_STR_8; addref(); }
    /// clear string, set buffer size
    void  reset( size_type size );
    /// returns character count
    size_type   length() const { return pchunk->len; }
    /// returns buffer size
    size_type   size() const { return pchunk->size; }
    /// changes buffer size
    void  resize(size_type count = 0, value_type e = 0);
    /// reserve space for specified amount of chars
    void  reserve(size_type count = 0);
    /// returns true if string is empty
    bool  empty() const { return pchunk->len==0; }
    /// returns true if string is empty
    bool  operator !() const { return pchunk->len==0; }
    /// swaps content of two strings
    void  swap(lString8 & str)
    {
    	lstring_chunk_t * tmp = pchunk;
        pchunk=str.pchunk; str.pchunk=tmp;
    }
    /// pack string (free unused buffer space)
    lString8 & pack();

    /// remove spaces from begin and end of string
    lString8 & trim();
    /// convert to integer
    int atoi() const;
    /// convert to 64 bit integer
    lInt64 atoi64() const;

    /// returns C-string
    const value_type * c_str() const { return pchunk->buf8; }

    /// append string
    lString8 & operator += ( lString8 s ) { return append(s); }
    /// append C-string
    lString8 & operator += ( const value_type * s ) { return append(s); }
    /// append single character
    lString8 & operator += ( value_type ch ) { return append(1, ch); }
    /// append decimal
    lString8 & operator += ( fmt::decimal v ) { return appendDecimal(v.get()); }
    /// append hex
    lString8 & operator += ( fmt::hex v ) { return appendHex(v.get()); }

    /// returns true if string ends with specified substring
    bool endsWith( const lChar8 * substring ) const;

    /// constructs string representation of integer
    static lString8 itoa( int i );
    /// constructs string representation of unsigned integer
    static lString8 itoa( unsigned int i );
    // constructs string representation of 64 bit integer
    static lString8 itoa( lInt64 n );

    static const lString8 empty_str;
};

/**
    \brief Wide character (lChar16) string. 

   Reference counting, copy-on-write implementation.
   Interface is similar to STL strings.

*/
class lString16Collection;
class lString16
{
    friend const lString16 & cs16(const char * str);
    friend const lString16 & cs16(const lChar16 * str);
    friend class lString16Collection;
public:
    // typedefs for STL compatibility
    typedef lChar16             value_type;
    typedef int                 size_type;
    typedef int                 difference_type;
    typedef value_type *        pointer;
    typedef value_type &        reference;
    typedef const value_type *  const_pointer;
    typedef const value_type &  const_reference;

    typedef lstring16_chunk_t    lstring_chunk_t; ///< data container

private:
    lstring_chunk_t * pchunk;
    static lstring_chunk_t * EMPTY_STR_16;
    void alloc(size_type sz);
    void free();
    inline void addref() const { ++pchunk->nref; }
    inline void release() { if (--pchunk->nref==0) free(); }
    lString16 LigatureCheck(lString16 text) ;
    lString16 LigatureCheck(lString16 text, lChar16 lam, lChar16 alef, lChar16 lig);
    lString16 PrettyLetters(lString16 text) ;

    //Indic scripts related functions
    bool CheckDvng();
    bool CheckBangla();
    bool CheckMalay();
    bool CheckKannada();
    bool CheckTamil();
    bool CheckTelugu();
    bool CheckGujarati();
    bool CheckOriya();
    lString16 processDvngText();
    lString16 processBanglaText();
    lString16 processMalayText();
    lString16 processKannadaText();
    lString16 processTamilText();
    lString16 processTeluguText();
    lString16 processGujaratiText();
    lString16 processOriyaText();
public:
    explicit lString16(lstring_chunk_t * chunk) : pchunk(chunk) { addref(); }
    /// empty string constructor
    explicit lString16() : pchunk(EMPTY_STR_16) { addref(); }
    /// copy constructor
    lString16(const lString16 & str) : pchunk(str.pchunk) { addref(); }
    /// constructor from wide c-string
    lString16(const value_type * str);
    /// constructor from 8bit c-string (ASCII only)
    explicit lString16(const lChar8 * str);
    /// constructor from 8bit (ASCII only) character array fragment
    explicit lString16(const lChar8 * str, size_type count);
    /// constructor from wide character array fragment
    explicit lString16(const value_type * str, size_type count);
    /// constructor from another string substring
    explicit lString16(const lString16 & str, size_type offset, size_type count);
    /// desctructor
    ~lString16() { release(); }

    /// assignment from string
    lString16 & assign(const lString16 & str)
    {
        if (pchunk!=str.pchunk)
        {
            release();
            pchunk = str.pchunk;
            addref();
        }
        return *this;
    }
    /// assignment from c-string
    lString16 & assign(const value_type * str);
    /// assignment from 8bit c-string (ASCII only)
    lString16 & assign(const lChar8 * str);
    /// assignment from character array fragment
    lString16 & assign(const value_type * str, size_type count);
    /// assignment from 8-bit character array fragment (ASCII only)
    lString16 & assign(const lChar8 * str, size_type count);
    /// assignment from string fragment
    lString16 & assign(const lString16 & str, size_type offset, size_type count);
    /// assignment from c-string
    lString16 & operator = (const value_type * str) { return assign(str); }
    /// assignment from string 8bit ASCII only
    lString16 & operator = (const lChar8 * str) { return assign(str); }
    /// assignment from string
    lString16 & operator = (const lString16 & str) { return assign(str); }
    lString16 & erase(size_type offset, size_type count);
    lString16 & erase(size_type offset) { return erase(offset,1);};

    lString16 & append(int val);
    lString16 & append(const value_type * str);
    lString16 & append(const value_type * str, size_type count);
    lString16 & append(const lChar8 * str);
    lString16 & append(const lChar8 * str, size_type count);
    lString16 & append(const lString16 & str);
    lString16 & append(const lString16 & str, size_type offset, size_type count);
    lString16 & append(size_type count, value_type ch);
    /// append decimal number
    lString16 & appendDecimal(lInt64 v);
    /// append hex number
    lString16 & appendHex(lUInt64 v);
    lString16 & insert(size_type p0, const value_type * str);
    lString16 & insert(size_type p0, const value_type * str, size_type count);
    lString16 & insert(size_type p0, const lString16 & str);
    lString16 & insert(size_type p0, const lString16 & str, size_type offset, size_type count);
    lString16 & insert(size_type p0, size_type count, value_type ch);
    lString16 & replace(size_type p0, size_type n0, const value_type * str);
    lString16 & replace(size_type p0, size_type n0, const value_type * str, size_type count);
    lString16 & replace(size_type p0, size_type n0, const lString16 & str);
    lString16 & replace(size_type p0, size_type n0, const lString16 & str, size_type offset, size_type count);
    /// replace range of string with character ch repeated count times
    lString16 & replace(size_type p0, size_type n0, size_type count, value_type ch);
    /// make string uppercase
    lString16 & uppercase();
    /// make string lowercase
    lString16 & lowercase();
    /// compare with another string
    int compare(const lString16& str) const { return lStr_cmp(pchunk->buf16, str.pchunk->buf16); }
    /// compare subrange with another string
    int compare(size_type p0, size_type n0, const lString16& str) const;
    /// compare subrange with substring of another string
    int compare(size_type p0, size_type n0, const lString16& str, size_type pos, size_type n) const;
    int compare(const value_type *s) const  { return lStr_cmp(pchunk->buf16, s); }
    int compare(const lChar8 *s) const  { return lStr_cmp(pchunk->buf16, s); }
    int compare(size_type p0, size_type n0, const value_type *s) const;
    int compare(size_type p0, size_type n0, const value_type *s, size_type pos) const;

    /// split string into two strings using delimiter
    bool split2( const lString16 & delim, lString16 & value1, lString16 & value2 );
    /// split string into two strings using delimiter
    bool split2( const lChar16 * delim, lString16 & value1, lString16 & value2 );
    /// split string into two strings using delimiter
    bool split2( const lChar8 * delim, lString16 & value1, lString16 & value2 );

    /// returns n characters beginning with pos
    lString16 substr(size_type pos, size_type n) const;
    /// returns part of string from specified position to end of string
    lString16 substr(size_type pos) const { return substr(pos, length()-pos); }
    /// replaces first found occurence of pattern
    bool replace(const lString16 & findStr, const lString16 & replaceStr);
    /// replaces first found occurence of "$N" pattern with string, where N=index
    bool replaceParam(int index, const lString16 & replaceStr);
    /// replaces first found occurence of "$N" pattern with itoa of integer, where N=index
    bool replaceIntParam(int index, int replaceNumber);

    /// find position of substring inside string, -1 if not found
    int pos(lString16 subStr) const;
    /// find position of substring inside string starting from specified position, -1 if not found
    int pos(const lString16 & subStr, int start) const;
    /// find position of substring inside string, -1 if not found
    int pos(const lChar16 * subStr) const;
    /// find position of substring inside string (8bit ASCII only), -1 if not found
    int pos(const lChar8 * subStr) const;
    /// find position of substring inside string starting from specified position, -1 if not found
    int pos(const lChar16 * subStr, int start) const;
    /// find position of substring inside string (8bit ASCII only) starting from specified position, -1 if not found
    int pos(const lChar8 * subStr, int start) const;

    /// find position of substring inside string, right to left, return -1 if not found
    int rpos(lString16 subStr) const;
    int rpos(const lChar8 * subStr) const;

    /// append single character
    lString16 & operator << (value_type ch) { return append(1, ch); }
    /// append c-string
    lString16 & operator << (const value_type * str) { return append(str); }
    /// append 8-bit c-string (ASCII only)
    lString16 & operator << (const lChar8 * str) { return append(str); }
    /// append string
    lString16 & operator << (const lString16 & str) { return append(str); }
    /// append decimal number
    lString16 & operator << (const fmt::decimal v) { return appendDecimal(v.get()); }
    /// append hex number
    lString16 & operator << (const fmt::hex v) { return appendHex(v.get()); }

    /// returns true if string starts with specified substring
    bool startsWith (const lString16 & substring) const;
    /// returns true if string starts with specified substring
    bool startsWith (const lChar16 * substring) const;
    /// returns true if string starts with specified substring (8bit ASCII only)
    bool startsWith (const lChar8 * substring) const;
    /// returns true if string ends with specified substring
    bool endsWith(const lChar16 * substring) const;
    /// returns true if string ends with specified substring (8-bit ASCII only)
    bool endsWith(const lChar8 * substring) const;
    /// returns true if string ends with specified substring
    bool endsWith (const lString16 & substring) const;
    /// returns true if string starts with specified substring, case insensitive
    bool startsWithNoCase (const lString16 & substring) const;

    /// returns last character
    value_type lastChar() { return empty() ? 0 : at(length()-1); }
    /// returns first character
    value_type firstChar() { return empty() ? 0 : at(0); }

    /// calculates hash for string
    lUInt32 getHash() const;

    /// returns character at specified position, with index bounds checking, fatal error if fails
    value_type & at( size_type pos )
    {
        if ((unsigned)pos > (unsigned)pchunk->len)
        {
            char buf[80];
            sprintf(buf,"lString16 at():: index out of bounds: %u > %u",(unsigned)pos, (unsigned)pchunk->len);
            crFatalError(404, buf);
        }
        return modify()[pos];
    }

    /// returns character at specified position, without index bounds checking
    value_type operator [] (size_type pos) const {
    	return pchunk->buf16[pos];
    }

    /// returns reference to specified character position (lvalue)
    value_type & operator [] (size_type pos) { return modify()[pos]; }
    /// resizes string, copies if several references exist
    void  lock(size_type newsize);
    /// returns writable pointer to string buffer
    value_type * modify() { if (pchunk->nref>1) lock(pchunk->len); return pchunk->buf16; }
    /// clears string contents
    void  clear() { release(); pchunk = EMPTY_STR_16; addref(); }
    /// resets string, allocates space for specified amount of characters
    void  reset (size_type size);
    /// returns string length, in characters
    size_type length() const { return pchunk->len; }
    size_type size() const { return pchunk->size; }
    /// resizes string buffer, appends with specified character if buffer is being extended
    void  resize(size_type count = 0, value_type e = 0);
    /// ensures string buffer can hold at least count characters
    void  reserve(size_type count = 0);
    /// erase all extra characters from end of string after size
    void  limit( size_type size );
    /// returns true if string is empty
    bool  empty() const { return pchunk->len==0; }
    /// swaps two string variables contents
    void  swap( lString16 & str ) { lstring_chunk_t * tmp = pchunk;
                pchunk=str.pchunk; str.pchunk=tmp; }
    /// trims all unused space at end of string (sets size to length)
    lString16 & pack();
    /// trims non alpha at beginning and end of string
    lString16 & trimNonAlpha();
    /// trims spaces at beginning and end of string
    lString16 & trim();
    /// trims duplicate space characters inside string and (optionally) at end and beginning of string
    lString16 & trimDoubleSpaces( bool allowStartSpace, bool allowEndSpace, bool removeEolHyphens=false );
    /// converts to integer
    int atoi() const;
    /// converts to integer, returns true if success
    bool atoi( int &n ) const;
    /// converts to 64 bit integer, returns true if success
    bool atoi( lInt64 &n ) const;
    /// returns constant c-string pointer
    const value_type * c_str() const { return pchunk->buf16; }
    /// returns constant c-string pointer, same as c_str()
    //const value_type * data() const { return pchunk->buf16; }
    /// appends string
    lString16 & operator += ( lString16 s ) { return append(s); }
    /// appends c-string
    lString16 & operator += ( const value_type * s ) { return append(s); }
    /// append C-string
    lString16 & operator += ( const lChar8 * s ) { return append(s); }
    /// appends single character
    lString16 & operator += ( value_type ch ) { return append(1, ch); }
    /// append decimal
    lString16 & operator += ( fmt::decimal v ) { return appendDecimal(v.get()); }
    /// append hex
    lString16 & operator += ( fmt::hex v ) { return appendHex(v.get()); }

    /// constructs string representation of integer
    static lString16 itoa( int i );
    /// constructs string representation of unsigned integer
    static lString16 itoa( unsigned int i );
    /// constructs string representation of 64 bit integer
    static lString16 itoa( lInt64 i );
    /// constructs string representation of unsigned 64 bit integer
    static lString16 itoa( lUInt64 i );

    /// empty string global instance
    static const lString16 empty_str;

    //removes UFFD (unicode questionmark) from the end of the line.
    lString16 TrimEndQuestionChar(lString16 & str);
    //converting all different spaces to one type space
    lString16 ReplaceUnusualSpaces();
    //check lString16 if it contains only digits. True if yes.
    bool DigitsOnly();
    //convert chars in rtl string depending on their word location
    lString16 PrepareRTL();
    //check whether string contains rtl characters
    bool CheckRTL();
    //split string to array of words, punct signs ands spaces
    std::vector<lString16> ArabicSplitToWords();

    lString16 ReversePrettyLetters();

    lString16 replaceAllPunctuation(const lString16 & replaceStr);

    lString16 AddtoAllPunctuation(const lString16 &addStrBefore, const lString16 &addStrAfter);

    bool CheckIfHasPunctuation();

    //Indic scripts related functions
    lString16 processIndicText();
    lString16 processIndicText(bool *indic);
    lString16 restoreIndicText();

    void checkCJK();
    bool checkCJ_bool();
    bool checkK_bool();

    bool operator > (const lString16& op2) const {return compare(op2) > 0;};
    bool operator < (const lString16& op2) const {return compare(op2) < 0;};
};

/// calculates hash for wide string
inline lUInt32 getHash( const lString16 & s ) { return s.getHash(); }
/// calculates hash for string
inline lUInt32 getHash( const lString8 & s ) { return s.getHash(); }
/// get reference to atomic constant string for string literal
/// e.g. cs8("abc") -- fast and memory effective replacement of lString8("abc")
const lString8 & cs8(const char * str);
/// get reference to atomic constant wide string for string literal
/// e.g. cs16("abc") -- fast and memory effective replacement of lString16("abc")
const lString16 & cs16(const char * str);
/// get reference to atomic constant wide string for string literal
/// e.g. cs16(L"abc") -- fast and memory effective replacement of lString16(L"abc")
const lString16 & cs16(const lChar16 * str);
/// calculates hash for wide c-string
lUInt32 calcStringHash( const lChar16 * s );

class SerialBuf;

/// returns true if two wide strings are equal
inline bool operator == (const lString16& s1, const lString16& s2 ) { return s1.compare(s2)==0; }
inline bool operator == (const lString16& s1, const lChar16 * s2 )  { return s1.compare(s2)==0; }
inline bool operator == (const lString16& s1, const lChar8 * s2 )   { return s1.compare(s2)==0; }
inline bool operator == (const lChar16 * s1, const lString16& s2 )  { return s2.compare(s1)==0; }
inline bool operator == (const lString16& s1, const lString8& s2 )  { return lStr_cmp(s2.c_str(), s1.c_str())==0; }
inline bool operator == (const lString8& s1, const lString16& s2 )  { return lStr_cmp(s2.c_str(), s1.c_str())==0; }
inline bool operator == (const lString8& s1, const lString8& s2 )   { return s1.compare(s2)==0; }
inline bool operator == (const lString8& s1, const lChar8 * s2 )    { return s1.compare(s2)==0; }
inline bool operator == (const lChar8 * s1, const lString8& s2 )    { return s2.compare(s1)==0; }

inline bool operator != (const lString16& s1, const lString16& s2 ) { return s1.compare(s2)!=0; }
inline bool operator != (const lString16& s1, const lChar16 * s2 )  { return s1.compare(s2)!=0; }
inline bool operator != (const lString16& s1, const lChar8 * s2 )   { return s1.compare(s2)!=0; }
inline bool operator != (const lChar16 * s1, const lString16& s2 )  { return s2.compare(s1)!=0; }
inline bool operator != (const lChar8 * s1, const lString16& s2 )   { return s2.compare(s1)!=0; }
inline bool operator != (const lString8& s1, const lString8& s2 )   { return s1.compare(s2)!=0; }
inline bool operator != (const lString8& s1, const lChar8 * s2 )    { return s1.compare(s2)!=0; }
inline bool operator != (const lChar8 * s1, const lString8& s2 )    { return s2.compare(s1)!=0; }

inline lString16 operator + (const lString16 &s1, const lString16 &s2) { lString16 s(s1); s.append(s2); return s; }
inline lString16 operator + (const lString16 &s1, const lChar16 * s2)  { lString16 s(s1); s.append(s2); return s; }
inline lString16 operator + (const lString16 &s1, const lChar8 * s2)   { lString16 s(s1); s.append(s2); return s; }
inline lString16 operator + (const lChar16 * s1,  const lString16 &s2) { lString16 s(s1); s.append(s2); return s; }
inline lString16 operator + (const lChar8 * s1,  const lString16 &s2)  { lString16 s(s1); s.append(s2); return s; }
inline lString16 operator + (const lString16 &s1, fmt::decimal v)      { lString16 s(s1); s.appendDecimal(v.get()); return s; }
inline lString16 operator + (const lString16 &s1, fmt::hex v)          { lString16 s(s1); s.appendHex(v.get()); return s; }
inline lString8  operator + (const lString8 &s1, const lString8 &s2)   { lString8 s(s1);  s.append(s2); return s; }
inline lString8  operator + (const lString8 &s1, const lChar8 * s2)    { lString8 s(s1);  s.append(s2); return s; }
inline lString8  operator + (const lString8 &s1, fmt::decimal v)       { lString8 s(s1);  s.appendDecimal(v.get()); return s; }
inline lString8  operator + (const lString8 &s1, fmt::hex v)           { lString8 s(s1);  s.appendHex(v.get()); return s; }


/// fast 16-bit string character appender
template <int BUFSIZE> class lStringBuf16 {
    lString16 & str;
    lChar16 buf[BUFSIZE];
    int pos;
	lStringBuf16 & operator = (lStringBuf16 & v)
	{
        CR_UNUSED(v);
		// not available
		return *this;
	}
public:
    lStringBuf16( lString16 & s )
    : str(s), pos(0)
    {
    }
    inline void append( lChar16 ch )
    {
        buf[ pos++ ] = ch;
        if ( pos==BUFSIZE )
            flush();
    }
    inline lStringBuf16& operator << ( lChar16 ch )
    {
        buf[ pos++ ] = ch;
        if ( pos==BUFSIZE )
            flush();
        return *this;
    }
    inline void flush()
    {
        str.append( buf, pos );
        pos = 0;
    }
    ~lStringBuf16( )
    {
        flush();
    }
};

/// fast 8-bit string character appender
template <int BUFSIZE> class lStringBuf8 {
    lString8 & str;
    lChar8 buf[BUFSIZE];
    int pos;
public:
    lStringBuf8( lString8 & s )
    : str(s), pos(0)
    {
    }
    inline void append( lChar8 ch )
    {
        buf[ pos++ ] = ch;
        if ( pos==BUFSIZE )
            flush();
    }
    inline lStringBuf8& operator << ( lChar8 ch )
    {
        buf[ pos++ ] = ch;
        if ( pos==BUFSIZE )
            flush();
        return *this;
    }
    inline void flush()
    {
        str.append( buf, pos );
        pos = 0;
    }
    ~lStringBuf8( )
    {
        flush();
    }
};

lString8  UnicodeToTranslit( const lString16 & str );
/// converts wide unicode string to local 8-bit encoding
lString8  UnicodeToLocal( const lString16 & str );
/// converts wide unicode string to utf-8 string
lString8  UnicodeToUtf8( const lString16 & str );
/// converts wide unicode string to utf-8 string
lString8 UnicodeToUtf8(const lChar16 * s, int count);
/// converts unicode string to 8-bit string using specified conversion table
lString8  UnicodeTo8Bit( const lString16 & str, const lChar8 * * table );
/// converts 8-bit string to unicode string using specified conversion table for upper 128 characters
lString16 ByteToUnicode( const lString8 & str, const lChar16 * table );
/// converts 8-bit string in local encoding to wide unicode string
lString16 LocalToUnicode( const lString8 & str );
/// converts utf-8 string to wide unicode string
lString16 Utf8ToUnicode( const lString8 & str );
/// converts utf-8 c-string to wide unicode string
lString16 Utf8ToUnicode( const char * s );
/// converts utf-8 string fragment to wide unicode string
lString16 Utf8ToUnicode( const char * s, int sz );
/// converts utf-8 string fragment to wide unicode string
void Utf8ToUnicode(const lUInt8 * src,  int &srclen, lChar16 * dst, int &dstlen);
/// decodes path like "file%20name" to "file name"
lString16 DecodeHTMLUrlString( lString16 s );
/// truncates string by specified size, appends ... if truncated, prefers to wrap whole words
void limitStringSize(lString16 & str, int maxSize);

int TrimDoubleSpaces(lChar16 * buf, int len,  bool allowStartSpace, bool allowEndSpace, bool removeEolHyphens);

lString16 metaSanitize(lString16 in);

bool splitIntegerList( lString16 s, lString16 delim, int & value1, int & value2 );

bool isDevanagari_consonant(lChar16 ch);
bool isBangla_consonant(lChar16 ch);

LigMap parseFontIndexes(lString16 path, int language);

class ExternalLigMap
{
private:
public:
    lString16 face;
    lString16 path;
    int type = 0;

    ExternalLigMap(){};
    ExternalLigMap(lString16 face, int type, lString16 path)
    {
        this->path = path;
        this->face = face;
        this->type = type;
    }

    bool isNull()
    {
        return path.empty() || face.empty() || type == 0;
    }
};

class LigMapManager
{
private:
    std::vector<ExternalLigMap> maps;

    ExternalLigMap find(lString16 face);
public:
    void addMap(lString16 face, int type, lString16 path);

    bool initMap(lString16 face);
};

extern LigMapManager gligMapManager;

lString16 trim(const lString16& input);
lString16 trimSpaces(const lString16& input);

#endif // __LV_STRING_H_INCLUDED__
