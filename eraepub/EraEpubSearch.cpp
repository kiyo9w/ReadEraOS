/*
 * Copyright (C) 2013-2020 READERA LLC
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Developers: ReadEra Team (2013-2020), Playful Curiosity (2013-2020),
 * Tarasus (2018-2020).
 */

#include "EraEpubBridge.h"

void CreBridge::processTextSearchPreviews(CmdRequest &request, CmdResponse &response)
{
    response.cmd = CMD_RES_SEARCH_PREVIEWS;
    CmdDataIterator iter(request.first);
    uint8_t *temp_val;
    iter.getByteArray(&temp_val);
    if (!iter.isValid())
    {
        LE("processTextSearchPreviews bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    const char *val = reinterpret_cast<const char *>(temp_val);
    lString16 inputstr = lString16(val);

    if(inputstr.empty())
    {
        LE("processTextSearchPreviews bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }

    lString16 query  = inputstr;
    int pagestart = 0;
    int pageend = doc_view_->GetPagesCount()-1;

    int sep = inputstr.pos(":");
    if (sep > 0)
    {
        lString16 pages_s = inputstr.substr(0, sep);
        lString16Collection pages_c;
        pages_c.parse(pages_s, L"-", false);
        pagestart = atoi(LCSTR(pages_c.at(0)));
        pageend   = atoi(LCSTR(pages_c.at(1)));
        query = inputstr.substr(sep + 1, inputstr.length() - sep + 1);
    }

    if (query.empty())
    {
        LE("processTextSearchPreviews bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    int slash_n = query.pos("\n");
    while (slash_n!=-1)
    {
        query.replace(slash_n,1,lString16(" "));
        slash_n = query.pos("\n");
    }

    query = query.processIndicText();
    if(gDocumentRTL && query.CheckRTL())
    {
        query.PrepareRTL();
    }

    for (int p = pagestart; p <= pageend; p ++)
    {
        auto page = (uint32_t) ImportPage(p, doc_view_->GetColumns());
        LVArray<SearchResult> searchPreviews = doc_view_->SearchForTextPreviews(page, query);

        for (int i = 0; i < searchPreviews.length(); i++)
        {
            SearchResult curr = searchPreviews.get(i);

            lString16 text = curr.preview_;
            while (text.pos(L"\n")!=-1)
            {
                text.replace(L"\n", L" ");
            }
            //text.trimDoubleSpaces(false,false,false);

            lString16 xpointers_str;
            for (int j = 0; j < curr.xpointers_.length(); j++)
            {
                xpointers_str += curr.xpointers_.get(j);
                xpointers_str += ";";
            }
            xpointers_str = xpointers_str.substr(0, xpointers_str.length() - 1);

            //LE("to response:");
            //LE("Page      : %d", page);
            //LE("xplen     : %d", curr.xpointers_.length());
            //LE("Xpointers : %s", LCSTR(xpointers_str));
            //LE("Preview   : %s", LCSTR(text));
            response.addInt(p);
            responseAddString(response, xpointers_str); //xpaths
            responseAddString(response, text.restoreIndicText());
        }
    }
}

void CreBridge::processTextSearchHitboxes(CmdRequest &request, CmdResponse &response)
{
    response.cmd = CMD_RES_SEARCH_HITBOXES;

    CmdDataIterator iter(request.first);
    uint8_t *temp_val;
    iter.getByteArray(&temp_val);
    if (!iter.isValid())
    {
        LE("processTextSearchHitboxes bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    const char *val = reinterpret_cast<const char *>(temp_val);
    lString16 inputstr = lString16(val);
    //lString16 inputstr = lString16("3:text"); //page:query

    if(inputstr.empty())
    {
        LE("processTextSearchHitboxes bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }

    lString16Collection collection;
    collection.parse(inputstr, L":", false);

    if(collection.length() < 3)
    {
        LE("processTextSearchHitboxes bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }

    uint32_t external_page = atoi(LCSTR(collection.at(0)));
    bool wordBeginFlag = (atoi(LCSTR(collection.at(1))) == 1);
    lString16 query = collection.at(2);

    //collect all ":" from query back into it
    for(int i = 3; i < collection.length(); i++)
    {
        query += ":" + collection.at(i);
    }

    if (query.empty())
    {
        LE("processTextSearchHitboxes bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    int slash_n = query.pos("\n");
    while (slash_n!=-1)
    {
        query.replace(slash_n,1,lString16::empty_str);
        slash_n = query.pos("\n");
    }

    auto page = (uint32_t) ImportPage(external_page, doc_view_->GetColumns());

    query = query.processIndicText();
    if(gDocumentRTL && query.CheckRTL())
    {
        query.PrepareRTL();
    }
    query = query.trim();

    if(query.empty())
    {
        LE("processTextSearchHitboxes bad request data: query is empty after filtering");
        response.result = RES_BAD_REQ_DATA;
        return;
    }

    LE("processTextSearchHitboxes [%ls] on page [%d]",query.c_str(),page);
    LVArray<Hitbox> hitboxes = doc_view_->SearchForTextHitboxes(page, query, wordBeginFlag);

    LE("processTextSearchHitboxes result length = [%d]",hitboxes.length());

    for (int i = 0; i < hitboxes.length(); i++)
    {
        Hitbox curr = hitboxes.get(i);
        ldomNode *node = curr.getNode();
        //LE("processTextSearchHitboxes RAW    hitbox = [%s] [%f : %f : %f : %f]",LCSTR(curr.text_.restoreIndicText()),curr.left_,curr.top_,curr.right_,curr.bottom_);
        if (RTL_DISPLAY_ENABLE && gDocumentRTL == 1 && node->isRTL() && node->getText().CheckRTL())
        {
            float offset = 0;

            LVFont *font = curr.word_.getNode()->getParentNode()->getFont().get();
            if (gFlgFloatingPunctuationEnabled)
            {
                offset = font->getVisualAligmentWidth() / 2;
                offset = offset / doc_view_->GetWidth();
            }

            curr.left_ = curr.left_ - 0.5f;
            curr.left_ = curr.left_ * -1;
            curr.left_ = curr.left_ + 0.5f + offset;

            curr.right_ = curr.right_ - 0.5f;
            curr.right_ = curr.right_ * -1;
            curr.right_ = curr.right_ + 0.5f + offset;

            if(curr.left_ > curr.right_)
            {
                //switch left and right places for correctness
                auto temp = curr.right_;
                curr.right_ = curr.left_;
                curr.left_ = temp;
            }

            hitboxes.set(i, curr);
        }
    }
    for (int i = 0; i < hitboxes.length(); i++)
    {
        Hitbox currHitbox = hitboxes.get(i);
        if (gJapaneseVerticalMode)
        {
            float t = currHitbox.top_;
            float b = currHitbox.bottom_;

            t -= 0.5f;
            b -= 0.5f;

            t = (t < 0) ? fabs(t) : -t;
            b = (b < 0) ? fabs(b) : -b;

            t += 0.5f;
            b += 0.5f;

            response.addFloat(b);
            response.addFloat(currHitbox.left_);
            response.addFloat(t);
            response.addFloat(currHitbox.right_);
        }
        else
        {
            response.addFloat(currHitbox.left_);
            response.addFloat(currHitbox.top_);
            response.addFloat(currHitbox.right_);
            response.addFloat(currHitbox.bottom_);
        }
        //LE("processTextSearchHitboxes result hitbox = [%s] [%f : %f : %f : %f]",LCSTR(currHitbox.text_.restoreIndicText()),currHitbox.left_,currHitbox.top_,currHitbox.right_,currHitbox.bottom_);
        lString16 restoredText = lString16(currHitbox.text_.c_str()).restoreIndicText();
        responseAddString(response,restoredText);
    }
}

void CreBridge::processTextSearchGetSuggestionsIndex(CmdRequest &request, CmdResponse &response)
{
    response.cmd = CMD_RES_INDEXER;
    CmdDataIterator iter(request.first);
    uint8_t *temp_val;
    iter.getByteArray(&temp_val);
    if (!iter.isValid())
    {
        LE("processTextSearchGetSuggestionsIndex bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    const char *val = reinterpret_cast<const char *>(temp_val);
    lString16 inputstr = lString16(val);

    if (inputstr.empty())
    {
        LE("processTextSearchGetSuggestionsIndex bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }

    lString16Collection pages_c;
    pages_c.parse(inputstr, L"-", false);
    int pagestart = atoi(LCSTR(pages_c.at(0)));
    int pageend = atoi(LCSTR(pages_c.at(1)));

    //lString16Map map = doc_view_->GetWordsIndexesMap();

    lString16Map globalMap;

    for (int page_index = pagestart; page_index < pageend; page_index++)
    {
        lString16Map pageMap = doc_view_->GetPhrasesIndexesMapForPage(page_index);
        for(auto& it : pageMap)
        {
            if(globalMap.find(it.first) != globalMap.end())
            {
                globalMap[it.first] += it.second;
            }
            else
            {
                globalMap[it.first] = it.second;
            }
        }
    }

    LE("processTextSearchGetSuggestionsIndex [%s]",LCSTR(inputstr));

    for (auto & it : globalMap)
    {
        LE("processTextSearchGetSuggestionsIndex [%s] = %d", it.first.c_str(), it.second);
        responseAddString(response, lString16(it.first.c_str()));
        response.addInt(it.second);
    }
}

void CreBridge::processRegisterDictionary(CmdRequest &request, CmdResponse &response)
{
    response.cmd = CMD_RES_PAGE_DICT_REGISTER;
    CmdDataIterator iter(request.first);
    uint8_t *temp_val;
    iter.getByteArray(&temp_val);
    if (!iter.isValid())
    {
        LE("processRegisterDictionary bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    const char *val = reinterpret_cast<const char *>(temp_val);

    lString16 temp(val);
    std::wstring query_string(temp.c_str(), temp.length());
    query_string = lowercase(query_string);

    doc_view_->searchDictionary.clear();
    doc_view_->searchDictionaryCJK.clear();
    doc_view_->searchDictionaryRTL.clear();
    doc_view_->searchDictionaryIndic.clear();
    std::vector<std::wstring> origs = split(query_string, '|');

    for (auto word : origs)
    {
        if(UnicodeHelper::checkCJ(word) )
        {
            //LE("processRegisterDictionary query [%ls] IS CJK",word.c_str());
            doc_view_->searchDictionaryCJK.emplace_back(word);
        }
        else if (UnicodeHelper::checkRTL(word))
        {
            //LE("processRegisterDictionary query [%ls] IS RTL",word.c_str());
            lString16 raw(word.c_str(), word.size());
            raw.PrepareRTL();
            std::wstring prepared = trim(std::wstring(raw.c_str(), raw.length()));
            doc_view_->searchDictionaryRTL.emplace_back(word,replaceDelimWithSpace(prepared,DELIM));
        }
        else if (checkIndic(word))
        {
            //LE("processRegisterDictionary query [%ls] IS INDIC",word.c_str());
            lString16 raw(word.c_str(), word.size());
            raw = raw.processIndicText();
            std::wstring prepared = trim(std::wstring(raw.c_str(), raw.length()));
            //LE("processRegisterDictionary  [%ls] -> [%ls]",word.c_str(), prepared.c_str());
            doc_view_->searchDictionaryIndic.emplace_back(word,replaceDelimWithSpace(prepared,DELIM));
        }
        else
        {
            //LE("processRegisterDictionary query [%ls] IS REGULAR",word.c_str());
            doc_view_->searchDictionary.emplace_back(word,replaceDelimWithSpace(word,DELIM));
        }
    }

    //for (const auto& pair : searchDictionary)
    //{
    //    LE("search dict [%ls] + [%ls]",pair.first.c_str(),pair.second.c_str());
    //}
    //for (const auto& str : searchDictionaryCJK)
    //{
    //    LE("search dict CJK [%ls]",str.c_str());
    //}

    response.addInt(0);
}

//checks if this page contains any word in the list
void CreBridge::processPageDictionaryCheck(CmdRequest &request, CmdResponse &response)
{
    response.cmd = CMD_RES_PAGE_DICT_CHECK;
    CmdDataIterator iter(request.first);
    uint32_t pagenum;
    iter.getInt(&pagenum);
    if (!iter.isValid())
    {
        LE("processPageDictionaryCheck bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }

    std::vector<std::wstring> result = doc_view_->checkIfPageHasWords(pagenum);

    lString16 result_str;
    for (auto & word : result)
    {
        //LE("word = [%ls]",word.c_str());
        result_str = result_str + lString16(word.c_str()) + L"|";
    }
    if(result_str.endsWith("|"))
    {
        result_str.erase(result_str.length()-1);
    }
    LE("processPageDictionaryCheck result_str = [%s]",LCSTR(result_str));
    responseAddString(response, result_str.restoreIndicText());
}