/*
 * Copyright (C) 2013-2020 READERA LLC
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * Developers: ReadEra Team (2013-2020), Tarasus (2018-2020).
 */

#ifndef _OPENREADERA_STSEARCHUTILS_H_
#define _OPENREADERA_STSEARCHUTILS_H_

#include <codecvt>
#include <string>
#include <vector>
#include <locale>
#include <algorithm>
#include <map>
#include "StUnicodeHelper.h"
#include "ore_log.h"

#define TTS_ALLOWED_ZONE_SIZE 0.2f

class Hitbox_st
{
public:
    float left_;
    float right_;
    float top_;
    float bottom_;
    std::string xpointer_;
    std::wstring text_;

    Hitbox_st() {};
    ~Hitbox_st(){};

    Hitbox_st(float left, float right, float top, float bottom, std::wstring text, std::string xpointer)
    {
        left_ = left;
        right_ = right;
        top_ = top;
        bottom_ = bottom;
        text_ = text;
        xpointer_ = xpointer;
    };

    virtual std::string getXpointer()
    {
        return xpointer_;
    }

    bool isZero()
    {
        return ( left_   == 0 &&
                 top_    == 0 &&
                 right_  == 0 &&
                 bottom_ == 0 );
    }

    std::string getCoordinatesString()
    {
        char buff[100];
        snprintf(buff, sizeof(buff), "[%f : %f] [%f : %f]", left_, top_, right_, bottom_);
        std::string res = buff;
        return res;
    }

    bool isInVerticalRange(float range_t, float range_b)
    {
        return ( (top_    > range_t && bottom_ > range_t) &&
                 (bottom_ < range_b  && top_   < range_b) );
    }
};

std::wstring uppercase(std::wstring str);
std::wstring lowercase( std::wstring str);

int pos_f_arr(std::vector<Hitbox_st> in, std::wstring subStr_in, int startPos);
int pos_f(std::wstring in, std::wstring subStr);
int pos_f(std::wstring in, std::wstring subStr, int startpos);

std::wstring stringToWstring(const std::string& t_str);
std::string wstringToString(const std::wstring& t_str);

void replaceAll(std::wstring &source, const std::wstring &from, const std::wstring &to);

std::vector<Hitbox_st> unionRects(std::vector<Hitbox_st> rects, bool glueLast = true);
std::vector<Hitbox_st> unionRectsTextCheck(std::vector<Hitbox_st> rects);
bool checkBeforePrevPage(std::vector<Hitbox_st> base, std::wstring query);
std::wstring ReplaceUnusualSpaces(std::wstring in);
std::vector<Hitbox_st> ReplaceUnusualSpaces(std::vector<Hitbox_st> in);
std::vector<std::wstring> split(const std::wstring& str, const std::wstring& delim);
std::vector<std::wstring> checkTextForDict(const std::wstring& base, std::vector<std::pair<std::wstring,std::wstring>> query_dictionary);

// std::vector<HitboxString> splitHitboxes_lossless(const HitboxString& input, const std::wstring& delim);

std::vector<std::wstring> split(const std::wstring& str, char delim);
std::vector<std::wstring> checkTextForDictSimple(const std::wstring& base, std::vector<std::wstring> query_dictionary);

#define DELIM std::wstring(L" —.,@:;'\"[]!@#$%^&*()_+-=<>*~`\\|//")
std::wstring replaceDelimWithSpace(const std::wstring& word, const std::wstring& delim);

bool checkIndic(std::wstring wstring); // implemented in indicUtils.cpp of EraEpub proj
std::wstring trim(const std::wstring& input);
std::wstring trimSpaces(const std::wstring& input);
std::wstring trimPunct(const std::wstring& input);

bool startsWith(const std::wstring& str, const std::wstring& prefix);
bool endsWith(const std::wstring& str, const std::wstring& suffix);
bool startsWith(const std::wstring& text, const std::vector<std::wstring>& parts );
bool endsWith(const std::wstring& text, const std::vector<std::wstring>& parts );
bool contains(const std::wstring& text, const std::vector<std::wstring>& parts );

//auto start = std::chrono::steady_clock::now();
//LE("%lld msec, passed since start",since(start).count());
template <class result_t   = std::chrono::milliseconds,
        class clock_t    = std::chrono::steady_clock,
        class duration_t = std::chrono::milliseconds>

auto since(std::chrono::time_point<clock_t, duration_t> const& start)
{
    return std::chrono::duration_cast<result_t>(clock_t::now() - start);
}

class HitboxSentence
{
public:
    std::wstring text = L"";
    int start = 0;
    int end   = 0;
    int type  = 0;

    HitboxSentence(int start, int end, std::wstring text)
    {
        this->start = start;
        this->end = end;
        this->text = text;
        //LE("sentence [%d -> %d] = [%ls]", start, end, text.c_str());
    };

    HitboxSentence() {};

    unsigned int length()
    {
        unsigned int len = end - start;
        if(len <= 0)
        {
            return 0;
        }
        return len;
    }

    bool empty()
    {
        return (this->length() == 0);
    }

    bool contains(int index)
    {
        return index >= start && index < end;
    }
};

int sentenceRightBound(const std::wstring& pageTexts, int start);
std::vector<HitboxSentence> generatePageSentences(std::wstring input);
HitboxSentence getFirstSentence(const std::wstring &input, const std::vector<Hitbox_st>& page_hitboxes, bool isDJVU = false);
bool checkFirst(const std::wstring& firstSentenceOfPage);
bool checkLast(const std::wstring& lastSentenceOfPage);
std::vector<Hitbox_st> sliceHitbox_st_vector(const std::vector<Hitbox_st>& v, HitboxSentence s);
std::vector<Hitbox_st> sliceHitbox_st_vector(const std::vector<Hitbox_st>& v, unsigned int start, unsigned int n);
std::wstring hitbox_stVectorToString(const std::vector<Hitbox_st>& input);
std::wstring filterStringForTTS(const std::wstring input);
std::string getCoords(const Hitbox_st& hb);

int getUnicodeRange(std::wstring text_in);
std::vector<HitboxSentence> performAllSplittings(const std::vector<Hitbox_st>& page_hitboxes , const std::vector<HitboxSentence>& sentences_in, bool isDJVU = false);
std::vector<std::vector<Hitbox_st>> convertSentencesToVectors(const std::vector<Hitbox_st>& page_hitboxes, std::vector<HitboxSentence> sentences);
int getFirstValidHitboxIndex(std::vector<Hitbox_st> sentence);
int getLastValidHitboxIndex(std::vector<Hitbox_st> sentence);
std::vector<HitboxSentence> postProcessSentences(const std::vector<HitboxSentence>& sentences_in);
std::vector<HitboxSentence> splitSentencesURLwise(const std::vector<HitboxSentence>& sentences);
std::vector<HitboxSentence> splitSentencesUnicodeWise(const std::vector<HitboxSentence>& sentences);

std::vector<Hitbox_st> filterTextViaColontitles(int page, const std::vector<Hitbox_st> &hitboxes, const std::map<int, std::pair<float, float>>& map);

float calculateUpperColontitle(const std::vector<Hitbox_st>& hitboxes);
float calculateLowerColontitle(const std::vector<Hitbox_st>& hitboxes_in);
bool hitboxComparator(Hitbox_st& a,Hitbox_st& b);

#endif //_OPENREADERA_STSEARCHUTILS_H_
