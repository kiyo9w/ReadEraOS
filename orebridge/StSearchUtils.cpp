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

typedef unsigned int uint;

#include "StSearchUtils.h"
#include <map>
#include <utility>
#include <vector>
#include <openreadera.h>

std::wstring uppercase(std::wstring str)
{
    for ( int i=0; i<str.length(); i++ ) {
        wchar_t ch = str[i];
        if ( ch>='a' && ch<='z' ) {
            str[i] = ch - 0x20;
        } else if ( ch>=0xE0 && ch<=0xFF ) {
            str[i] = ch - 0x20;
        } else if ( ch>=0x430 && ch<=0x44F ) {// cyrillic
            str[i] = ch - 0x20;
        } else if( ch == 0x451) { //cyrillic "Ё"
            str[i] = 0x401;
        } else if ( ch>=0x3b0 && ch<=0x3cF ) {
            str[i] = ch - 0x20;
        } else if ( (ch >> 8)==0x1F ) { // greek
            wchar_t n = ch & 255;
            if (n<0x70) {
                str[i] = ch | 8;
            } else if (n<0x80) {

            } else if (n<0xF0) {
                str[i] = ch | 8;
            }
        } else if(ch >= 0x561 && ch <= 0x586) { // armenian
            str[i] = ch - 0x30;
        } else if ((ch >= 0x10D0 && ch <= 0x10F5)|| ch == 0x10F7 || ch == 0x10FD ) {  // georgian
            str[i] = ch - 0x30;
        }
    }
    return str;
}

std::wstring lowercase( std::wstring str)
{
    for ( int i=0; i<str.length(); i++ ) {
        wchar_t ch = str[i];
        if ( ch>='A' && ch<='Z' ) {
            str[i] = ch + 0x20;
        } else if ( ch>=0xC0 && ch<=0xDF ) {
            str[i] = ch + 0x20;
        } else if ( ch>=0x410 && ch<=0x42F ) {  //cyrillic
            str[i] = ch + 0x20;
        } else if( ch == 0x401) { //cyrillic "Ё"
            str[i] = 0x451;
        } else if ( ch>=0x390 && ch<=0x3aF ) { // Greek
            str[i] = ch + 0x20;
        } else if ( (ch >> 8)==0x1F ) { // greek
            wchar_t n = ch & 255;
            if (n<0x70) {
                str[i] = ch & (~8);
            } else if (n<0x80) {

            } else if (n<0xF0) {
                str[i] = ch & (~8);
            }
        }
        else if(ch >= 0x531 && ch <= 0x556) {  // armenian
            str[i] = ch + 0x30;
        } else if ((ch >= 0x10A0 && ch <= 0x10C5)|| ch == 0x10C7 || ch == 0x10CD ) {  // georgian
            str[i] = ch + 0x30;
        }
    }
    return str;
}

int pos_f_arr(std::vector<Hitbox_st> in, std::wstring subStr_in, int startPos)
{
    if (startPos > in.size()-1)
    {
        return -1;
    }
    if (subStr_in.length() > in.size() - startPos)
    {
        return -1;
    }
    int s_len = subStr_in.length();
    int diff_len = in.size() - s_len;
    for (uint i = startPos; i <= diff_len; i++)
    {
        int flg = 1;
        for (uint j = 0; j < s_len; j++)
            if (lowercase(in.at(i + j).text_).at(0) != subStr_in.at(j))
            {
                flg = 0;
                break;
            }
        if (flg)
        {
            return i;
        }
    }
    return -1;
}

int pos_f(std::wstring in, std::wstring subStr)
{
    //return in.find(subStr);

    if (subStr.length() > in.length())
    {
        return -1;
    }
    int s_len = subStr.length();
    int diff_len = in.length() - s_len;
    for (int i = 0; i <= diff_len; i++)
    {
        int flg = 1;
        for (int j = 0; j < s_len; j++)
            if (in.at(i + j) != subStr.at(j))
            {
                flg = 0;
                break;
            }
        if (flg)
        {
            return i;
        }
    }
    return -1;
}

int pos_f(std::wstring in, std::wstring subStr, int startpos)
{
    //return in.find(subStr);

    if (subStr.length() > in.length())
    {
        return -1;
    }
    int s_len = subStr.length();
    int diff_len = in.length() - s_len;
    for (int i = startpos; i <= diff_len; i++)
    {
        int flg = 1;
        for (int j = 0; j < s_len; j++)
            if (in.at(i + j) != subStr.at(j))
            {
                flg = 0;
                break;
            }
        if (flg)
        {
            return i;
        }
    }
    return -1;
}

std::wstring stringToWstring(const std::string& t_str)
{
    if(t_str.empty())
    {
        return std::wstring();
    }
    //setup converter
    typedef std::codecvt_utf8<wchar_t> convert_type;
    char errstr[7] = "error\0";
    wchar_t werrstr[7] = L"error\0";
    std::wstring_convert<convert_type, wchar_t> converter(errstr,werrstr);

    //use converter (.to_bytes: wstr->str, .from_bytes: str->wstr)
    std::wstring res = converter.from_bytes(t_str);
    if(res == werrstr)
    {
        return std::wstring();
    }
    return res;
}

std::string wstringToString(const std::wstring& t_str)
{
    if(t_str.empty())
    {
        return std::string();
    }
    //setup converter
    typedef std::codecvt_utf8<wchar_t> convert_type;
    char errstr[7] = "error\0";
    std::wstring_convert<convert_type, wchar_t> converter(errstr);

    //use converter (.to_bytes: wstr->str, .from_bytes: str->wstr)
    std::string res = converter.to_bytes(t_str);
    if(res == errstr)
    {
        return std::string();
    }
    return res;
}

void replaceAll(std::wstring &source, const std::wstring &from, const std::wstring &to)
{
    std::wstring newString;
    newString.reserve(source.length());  // avoids a few memory allocations

    std::wstring::size_type lastPos = 0;
    std::wstring::size_type findPos;

    while ((findPos = pos_f(source, from, lastPos)) != -1)
    {
        newString.append(source, lastPos, findPos - lastPos);
        newString += to;
        lastPos = findPos + from.length();
    }

    // Care for the rest after last occurrence
    newString += source.substr(lastPos);

    source.swap(newString);
}

std::vector<Hitbox_st> unionRects(std::vector<Hitbox_st> rects, bool glueLast)
{
    std::vector<Hitbox_st> result;
    if (rects.empty())
    {
        return result;
    }

    Hitbox_st curr = rects.at(0);
    int max = rects.size();
    if (!glueLast)
    {
        max--;
    }
    for (int i = 0; i < max; i++)
    {
        Hitbox_st rect = rects.at(i);
        if (curr.right_ >= rect.left_ && curr.top_ == rect.top_ && curr.bottom_ == rect.bottom_)
        {
            curr.right_ = rect.right_;
            continue;
        }
        result.push_back(curr);
        curr = rect;
    }
    result.push_back(curr);
    if (!glueLast)
    {
        result.push_back(rects.at(rects.size()-1));
    }
    return result;
}

std::vector<Hitbox_st> unionRectsTextCheck(std::vector<Hitbox_st> rects)
{
    std::vector<Hitbox_st> result;
    if (rects.empty())
    {
        return result;
    }

    Hitbox_st curr = rects.at(0);
    for (int i = 0; i < rects.size(); i++)
    {
        Hitbox_st rect = rects.at(i);
        if (curr.right_ >= rect.left_ &&
            curr.top_ == rect.top_ &&
            curr.bottom_ == rect.bottom_ &&
            curr.text_ == rect.text_)
        {
            curr.right_ = rect.right_;
            continue;
        }
        result.push_back(curr);
        curr = rect;
    }
    result.push_back(curr);
    return result;
}

std::wstring ReplaceUnusualSpaces(std::wstring in)
{
    if(in.length()<=0)
    {
        return in;
    }
    std::replace( in.begin(), in.end(), 0x0009, 0x0020);
    std::replace( in.begin(), in.end(), 0x00A0, 0x0020);
    std::replace( in.begin(), in.end(), 0x180E, 0x0020);
    std::replace( in.begin(), in.end(), 0x2000, 0x0020);
    std::replace( in.begin(), in.end(), 0x2001, 0x0020);
    std::replace( in.begin(), in.end(), 0x2002, 0x0020);
    std::replace( in.begin(), in.end(), 0x2003, 0x0020);
    std::replace( in.begin(), in.end(), 0x2004, 0x0020);
    std::replace( in.begin(), in.end(), 0x2005, 0x0020);
    std::replace( in.begin(), in.end(), 0x2006, 0x0020);
    std::replace( in.begin(), in.end(), 0x2007, 0x0020);
    std::replace( in.begin(), in.end(), 0x2008, 0x0020);
    std::replace( in.begin(), in.end(), 0x2009, 0x0020);
    std::replace( in.begin(), in.end(), 0x200A, 0x0020);
    std::replace( in.begin(), in.end(), 0x200B, 0x0020);
    std::replace( in.begin(), in.end(), 0x202F, 0x0020);
    std::replace( in.begin(), in.end(), 0x205F, 0x0020);
    std::replace( in.begin(), in.end(), 0x3000, 0x0020);
    std::replace( in.begin(), in.end(), 0xFEFF, 0x0020);
    return in;
}

std::vector<Hitbox_st> ReplaceUnusualSpaces(std::vector<Hitbox_st> in)
{
    for (int i = 0; i < in.size() ; i++)
    {
        Hitbox_st* curr = &in[i];
        curr->text_ = ReplaceUnusualSpaces(curr->text_);
    }
    return in;
}

bool checkBeforePrevPage(std::vector<Hitbox_st> base, std::wstring query)
{
    int qlen = query.length();
    std::vector<Hitbox_st> subset(&base[0], &base[qlen]);
    std::wstring chStr = query.substr(qlen-1,1);
    return (pos_f_arr(subset, chStr, 0) != -1);
}

std::vector<std::wstring> split(const std::wstring& str, char delim) {
    std::vector<std::wstring> strings;
    size_t start;
    size_t end = 0;
    while ((start = str.find_first_not_of(delim, end)) != std::wstring::npos) {
        end = str.find(delim, start);
        strings.push_back(str.substr(start, end - start));
    }
    return strings;
}

std::vector<std::wstring> split(const std::wstring& str, const std::wstring& delim) {
    std::vector<std::wstring> strings;
    size_t start;
    size_t end = 0;
    while ((start = str.find_first_not_of(delim, end)) != std::wstring::npos) {
        end = str.find_first_of(delim, start);
        strings.push_back(str.substr(start, end - start));
    }
    return strings;
}

bool sortedVectorHasInt(std::vector<int> vec, int search)
{
    for (int val : vec)
    {
        if( val > search)  { return false; }
        if( val < search)  { continue; }
        if( val == search) { return true; }
    }
    //vec empty
    return false;
}

bool FindInShortmap_fast(std::wstring query, std::map<std::wstring,std::vector<std::wstring>> shortmap)
{
    std::vector<std::wstring> result;
    std::wstring shortword = query.substr(0,3);
    LE("FindInShortmap_fast [%ls] -> [%ls]",shortword.c_str(), query.c_str());
    auto it_2 = shortmap.find(shortword);
    if(it_2 != shortmap.end())
    {
        LE("FindInShortmap_fast found [%ls] from short [%ls]",it_2->first.c_str(),shortword.c_str());

        std::vector<std::wstring> words = it_2->second;
        for (const auto& word : words)
        {
            if(word.rfind(query,0) == 0) //starts with
            {
                LE("found word [%ls] that starts with [%ls] -> [%ls]",word.c_str(),shortword.c_str(),query.c_str());
                return true;
            }
        }
    }
    return false;
}

std::vector<int> isComboPresent(const std::wstring& combo, std::map<std::wstring,std::vector<int>> map, std::map<std::wstring,std::vector<std::wstring>> shortmap)
{
    //LE("isComboPresent: combo [%ls]",combo.c_str());

    std::vector<std::wstring> wordCollection = split(combo, ' ');
    std::vector<int> result;

    std::vector<std::vector<int>> vectors;
    for (int i = 0; i < wordCollection.size(); i++)
    {
        std::wstring word = wordCollection.at(i);
        auto it = map.find(word);
        if(it != map.end())
        {
            vectors.push_back(it->second);
        }
        else if(!vectors.empty() && i == wordCollection.size() - 1)
        {
            //LE("isComboPresent: FindInShortmap_fast [%ls]",word.c_str());
            if (FindInShortmap_fast(word, shortmap))
            {
                //LE("isComboPresent: FindInShortmap_fast FOUND");
                auto pre_vector = vectors.back();
                std::vector<int> last_vec;
                for (int item : pre_vector)
                {
                    last_vec.push_back(item + 1);
                }
                vectors.push_back(last_vec);
            }
            //else
            //{
            //    LE("isComboPresent: FindInShortmap_fast NOT FOUND");
            //}
        }
        else
        {
            //one of the words not found
            LE("isComboPresent: one of the words (%ls) not found",word.c_str());
            return result;
        }
    }
    if(vectors.size() < wordCollection.size())
    {
        LE("isComboPresent: COMBO SEARCH FAILED");
        return result;
    }
    //all the words are present in map, now need to find out if they're ordered

    std::vector<int> vec0 = vectors.at(0);
    for (int i = 0; i < vec0.size(); i++)
    {
        int val = vec0.at(i);
        bool flag = true;
        for (int j = 1; j < vectors.size(); j++)
        {
            if (!sortedVectorHasInt(vectors.at(j), val + j))
            {
                flag = false;
                break;
            }
        }
        if (!flag)
        {
            continue;
        }
        else
        {
            result.push_back(val);
        }
    }
    LE("isComboPresent: result = %s", result.empty() ? "false" : "true");
    return result;
}

std::vector<std::wstring> checkTextForDict(const std::wstring& base, std::vector<std::pair<std::wstring,std::wstring>> query_dictionary)
{
    if (base.empty())
    {
        return std::vector<std::wstring>();
    }
    std::map<std::wstring,std::vector<int>> textmap;
    std::map<std::wstring,std::vector<std::wstring>> shortmap;

    //LE("START p");
    std::vector<std::wstring> textCollection = split(base, DELIM); //split by ANY of chars in delimiter
    //LE("END p");


    //LE("START f");
    for (int i = 0; i < textCollection.size(); i++)
    {
        std::wstring word = lowercase(textCollection.at(i));

        //LE("word before trim = [%ls]", word.c_str());
        word = trimPunct(word);
        //LE("word after trim = [%ls]", word.c_str());
        auto it = textmap.find((word));
        if(it == textmap.end())
        {
            textmap.insert(std::make_pair(word,std::vector<int>(1,i)));
        }
        else
        {
            it->second.push_back(i);
        }

        if(word.length() >= 3)
        {
            std::wstring short_word = word.substr(0,3);
            auto it_2 = shortmap.find((short_word));
            if(it_2 == shortmap.end())
            {
                shortmap.insert(std::make_pair(short_word,std::vector<std::wstring>(1,word)));
            }
            else
            {
                it_2->second.push_back(word);
            }
        }

        //LE("word added [%s] [%d]",LCSTR(word), i);
    }
    //LE("END f");

    //LE("START s");
    //LE("map   size = %d",textmap.size());
    //LE("query size = %d",query_dictionary.size());

    std::vector<std::wstring> result;

    for (int i = 0; i < query_dictionary.size(); i++)
    {
        std::wstring query;
        std::wstring query_orig;
        query = query_dictionary.at(i).second;
        query_orig = query_dictionary.at(i).first;

        if(query.find(' ')!= std::string::npos)
        {
            std::vector<int> combo_vec = isComboPresent(query,textmap,shortmap);

            if(!combo_vec.empty())
            {
                result.push_back(query_orig);
                //std::string res = " ";
                //for (int j = 0; j < combo_vec.size(); j++)
                //{
                //    res += std::to_string(combo_vec.at(j)) + " ";
                //}
                //LE("search [%ls] -> [%s]", query.c_str(), res.c_str());
            }
            //else
            //{
            //    LE("search [%ls] -> [NOT FOUND]", query.c_str());
            //}
        }
        else
        {
            auto it = textmap.find(query);
            if (it != textmap.end())
            {
                result.push_back(query_orig);
                //std::string res = " ";
                //for (int j = 0; j < it->second.size(); j++)
                //{
                //    res += std::to_string(it->second.at(j)) + " ";
                //}
                //LE("search [%ls] -> [%s]", query.c_str(), res.c_str());
            }
            else if (query.length() >= 3) /// USE FLAG TO TURN THIS OFF (if needed)
            {
                std::wstring shortword = query.substr(0,3);
                auto it_2 = shortmap.find(shortword);
                if(it_2 != shortmap.end())
                {
                    std::vector<std::wstring> words = it_2->second;
                    for (const auto& word : words)
                    {
                        if(word.rfind(query,0) == 0) //starts with
                        {
                            //LE("found word [%ls] that starts with [%ls]",word.c_str(),query.c_str());
                            result.push_back(query_orig);
                            break;
                        }
                    }
                }
            }
            //else
            //{
            //    LE("search [%ls] -> [NOT FOUND]", query.c_str());
            //}
        }
    }
    //LE("END s");
    //todo check map mem freeing
    return result;
}

std::vector<std::wstring> checkTextForDictSimple(const std::wstring& base, std::vector<std::wstring> query_dictionary)
{
    if (base.empty())
    {
        return std::vector<std::wstring>();
    }
    std::vector<std::wstring> result;

    //LE("checkTextForDictSimple text [%ls]",base.c_str());

    for (auto & query : query_dictionary)
    {
        auto query_orig = query;
        query = trim(query);

        //LE("checkTextForDictSimple q = [%ls]",query.c_str());
        if(base.find(lowercase(query)) != std::wstring::npos)
        {
            //LE("checkTextForDictSimple q = [%ls] FOUND",query.c_str());
            result.push_back(query_orig);
        }
        else
        {
            //LE("checkTextForDictSimple q = [%ls] NOT FOUND",query.c_str());
        }
    }
    return result;
}

std::wstring replaceDelimWithSpace(const std::wstring& word, const std::wstring& delim)
{
    std::wstring result = word;
    for(auto wchar : delim)
    {
        std::replace(result.begin(), result.end(), wchar, L' ');
    }
    return result;
}

std::wstring trim(const std::wstring& input)
{
    std::wstring result = input;
    while (!result.empty() &&
           (UnicodeHelper::isPunct(result.front()) || UnicodeHelper::isSpace(result.front())))
    {
        result = result.substr(1);
    }
    while (!result.empty() &&
           (UnicodeHelper::isPunct(result.back()) || UnicodeHelper::isSpace(result.back())))
    {
        result = result.substr(0, result.length() - 1);
    }
    return result;
}

std::wstring trimPunct(const std::wstring& input)
{
    std::wstring result = input;
    while (!result.empty() && UnicodeHelper::isPunct(result.front()))
    {
        result = result.substr(1);
    }
    while (!result.empty() && UnicodeHelper::isPunct(result.back()))
    {
        result = result.substr(0, result.length() - 1);
    }
    return result;
}

std::wstring trimSpaces(const std::wstring& input)
{
    std::wstring result = input;
    while (!result.empty() && UnicodeHelper::isSpace(result.front()))
    {
        result = result.substr(1);
    }
    while (!result.empty() && UnicodeHelper::isSpace(result.back()))
    {
        result = result.substr(0, result.length() - 1);
    }
    return result;
}

bool endsWith(const std::wstring &str, const std::wstring &suffix)
{
    return str.size() >= suffix.size() && 0 == str.compare(str.size()-suffix.size(), suffix.size(), suffix);
}

bool startsWith(const std::wstring &str, const std::wstring &prefix)
{
    return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix);
}

bool startsWith(const std::wstring &text, const std::vector<std::wstring> &parts)
{
    return std::any_of(parts.begin(), parts.end(), [&text](const std::wstring& part) { return startsWith( text, part ); } );
}

bool endsWith(const std::wstring &text, const std::vector<std::wstring> &parts)
{
    return std::any_of(parts.begin(), parts.end(), [&text](const std::wstring& part) { return endsWith( text, part ); } );
}

bool contains(const std::wstring &text, const std::vector<std::wstring> &parts)
{
    return std::any_of(parts.begin(), parts.end(), [&text](const std::wstring& part) { return text.find(part) != std::wstring::npos; } );
}