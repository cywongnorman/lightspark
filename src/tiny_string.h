/**************************************************************************
    Lightspark, a free flash player implementation

    Copyright (C) 2012-2013  Alessandro Pignotti (a.pignotti@sssup.it)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

#ifndef TINY_STRING_H
#define TINY_STRING_H 1

#include <cstring>
#include <cstdint>
#include <ostream>
#include <list>
/* for utf8 handling */
#include <glib.h>
#include "compat.h"

/* forward declare for tiny_string conversion */
namespace Glib { class ustring; }
typedef unsigned char xmlChar;

namespace lightspark
{

/* Iterates over utf8 characters */
class CharIterator /*: public forward_iterator<uint32_t>*/
{
friend class tiny_string;
private:
	char* buf_ptr;
public:
	CharIterator(char* buf) : buf_ptr(buf) {}
	/* Return the utf8-character value */
	uint32_t operator*() const
	{
		return g_utf8_get_char(buf_ptr);
	}
	CharIterator& operator++() //prefix
	{
		buf_ptr = g_utf8_next_char(buf_ptr);
		return *this;
	}
	CharIterator operator++(int) // postfix
	{
		CharIterator result = *this;
		++(*this);
		return result;
	}
	bool operator==(const CharIterator& o) const
	{
		return buf_ptr == o.buf_ptr;
	}
	bool operator!=(const CharIterator& o) const { return !(*this == o); }
	/* returns true if the current character is a digit */
	bool isdigit() const
	{
		return g_unichar_isdigit(g_utf8_get_char(buf_ptr));
	}
	/* return the value of the current character interpreted as decimal digit,
	 * i.e. '7' -> 7
	 */
	int32_t digit_value() const
	{
		return g_unichar_digit_value(g_utf8_get_char(buf_ptr));
	}
};

/*
 * String class.
 * The string can contain '\0's, so don't use raw_buf().
 * Use len() to determine actual size.
 */
class DLL_PUBLIC tiny_string
{
friend std::ostream& operator<<(std::ostream& s, const tiny_string& r);
private:
	enum TYPE { READONLY=0, STATIC, DYNAMIC };
	/*must be at least 6 bytes for tiny_string(uint32_t c) constructor */
	#define STATIC_SIZE 64
	char _buf_static[STATIC_SIZE];
	char* buf;
	/*
	   stringSize includes the trailing \0
	*/
	uint32_t stringSize;
	uint32_t numchars;
	TYPE type;
#ifdef MEMORY_USAGE_PROFILING
	//Implemented in memory_support.cpp
	DLL_PUBLIC void reportMemoryChange(int32_t change) const;
#else
	//NOP
	void reportMemoryChange(int32_t change) const {}
#endif
	//TODO: use static buffer again if reassigning to short string
	void makePrivateCopy(const char* s);
	void createBuffer(uint32_t s);
	void resizeBuffer(uint32_t s);
	void resetToStatic();
	void init();
	bool isASCII:1;
	bool hasNull:1;
public:
	static const uint32_t npos = (uint32_t)(-1);

	tiny_string():_buf_static(),buf(_buf_static),stringSize(1),numchars(0),type(STATIC),isASCII(true),hasNull(false){buf[0]=0;}
	/* construct from utf character */
	static tiny_string fromChar(uint32_t c);
	tiny_string(const char* s,bool copy=false);
	tiny_string(const tiny_string& r);
	tiny_string(const std::string& r);
	tiny_string(const Glib::ustring& r);
	tiny_string(std::istream& in, int len);
	~tiny_string();
	tiny_string& operator=(const tiny_string& s);
	tiny_string& operator=(const std::string& s);
	tiny_string& operator=(const char* s);
	tiny_string& operator=(const Glib::ustring& s);
	tiny_string& operator+=(const char* s);
	tiny_string& operator+=(const tiny_string& r);
	tiny_string& operator+=(const std::string& s);
	tiny_string& operator+=(const Glib::ustring& s);
	tiny_string& operator+=(uint32_t c);
	const tiny_string operator+(const tiny_string& r) const;
	const tiny_string operator+(const char* s) const;
	const tiny_string operator+(const std::string& r) const;
	const tiny_string operator+(const Glib::ustring& r) const;
	bool operator<(const tiny_string& r) const;
	bool operator>(const tiny_string& r) const;
	bool operator==(const tiny_string& r) const;
	bool operator==(const std::string& r) const;
	bool operator!=(const std::string& r) const;
	bool operator!=(const tiny_string& r) const;
	bool operator==(const char* r) const;
	bool operator==(const xmlChar* r) const;
	bool operator!=(const char* r) const;
	bool operator==(const Glib::ustring&) const;
	bool operator!=(const Glib::ustring&) const;
	inline const char* raw_buf() const
	{
		return buf;
	}
	inline bool empty() const
	{
		return stringSize == 1;
	}
	inline void clear()
	{
		resetToStatic();
		numchars = 0;
		isASCII = true;
		hasNull = false;
	}
	
	/* returns the length in bytes, not counting the trailing \0 */
	inline uint32_t numBytes() const
	{
		return stringSize-1;
	}
	/* returns the length in utf-8 characters, not counting the trailing \0 */
	inline uint32_t numChars() const
	{
		return numchars;
	}
	
	/* start and len are indices of utf8-characters */
	tiny_string substr(uint32_t start, uint32_t len) const;
	tiny_string substr(uint32_t start, const CharIterator& end) const;
	/* start and len are indices of bytes */
	tiny_string substr_bytes(uint32_t start, uint32_t len) const;
	/* finds the first occurence of char in the utf-8 string
	 * Return NULL if not found, else ptr to beginning of first occurence of c */
	char* strchr(char c) const;
	char* strchrr(char c) const;
	/*explicit*/ operator std::string() const;
	operator Glib::ustring() const;
	bool startsWith(const char* o) const;
        bool endsWith(const char* o) const;
	/* idx is an index of utf-8 characters */
	uint32_t charAt(uint32_t idx) const;
	/* start is an index of characters.
	 * returns index of character */
	uint32_t find(const tiny_string& needle, uint32_t start = 0) const;
	uint32_t rfind(const tiny_string& needle, uint32_t start = npos) const;
	tiny_string& replace(uint32_t pos1, uint32_t n1, const tiny_string& o);
	tiny_string& replace_bytes(uint32_t bytestart, uint32_t bytenum, const tiny_string& o);
	tiny_string lowercase() const;
	tiny_string uppercase() const;
	/* like strcasecmp(s1.raw_buf(),s2.raw_buf()) but for unicode */
	int strcasecmp(tiny_string& s2) const;
	/* split string at each occurrence of delimiter character */
	std::list<tiny_string> split(uint32_t delimiter) const;
	/* Convert from byte offset to UTF-8 character index */
	uint32_t bytePosToIndex(uint32_t bytepos) const;
	/* iterate over utf8 characters */
	CharIterator begin();
	CharIterator begin() const;
	CharIterator end();
	CharIterator end() const;
	int compare(const tiny_string& r) const;
};

};
#endif /* TINY_STRING_H */
