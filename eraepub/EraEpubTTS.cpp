//
// Created by Tarasus on 03.04.2023.
//


/*

1 - весь текст со страницы порезанный по предложениям
req: page
res: array[] {start-xpath; text; end-xpath}

2 - в каком предлодении xpath
req: xpath
res: start-xpath; text; end-xpath

3 - хитбоксы для страницы сгруппированные по предложениям
req: page
res: array[]{l,t,r,b,xpath} // как у цитат

 */

#include "EraEpubBridge.h"
#include "StSearchUtils.h"


int getFirstValidHitboxIndex(std::vector<Hitbox> sentence)
{
    std::string xp_start;
    for ( int i = 0; i < sentence.size() ; i++)
    {
        xp_start = sentence.at(i).getXpointer();
        if (!xp_start.empty())
        {
            return i;
        }
    }
    return -1;
}

int getLastValidHitboxIndex(std::vector<Hitbox> sentence)
{
    std::string xp_end;
    for( int i = sentence.size() - 1; i >= 0 ; i--)
    {
        xp_end = sentence.at(i).getXpointer();
        if (!xp_end.empty())
        {
            return i;
        }
    }
    return -1;
}

std::vector<Hitbox> slice(const std::vector<Hitbox>& v, unsigned int start, unsigned int n)
{
    //LE("sliceHitbox_st_vector [%d] %d + %d = %d",v.size(),start,n, start + n);
    if(start + n > v.size())
    {
        n = v.size() - start;
        //LE("sliceHitbox_st_vector FIXED [%d] %d + %d = %d",v.size(),start,n, start + n);
    }
    if(n <= 0)
    {
        return std::vector<Hitbox>();
    }

    auto first = v.cbegin() + start;
    auto last  = v.cbegin() + start + n;

    std::vector<Hitbox> vec(first, last);
    return vec;
}

std::vector<Hitbox> slice(const std::vector<Hitbox>& v, HitboxSentence s)
{
    return slice(v, s.start, s.length());
}

std::wstring hitboxVectorToString(const std::vector<Hitbox>& input)
{
    std::wstring res;
    for(const auto& hb : input)
    {
        res += hb.text_;
    }
    return res;
}

std::wstring applyFootnoteSignature(const HitboxSentence& sentence_in, const std::vector<Hitbox>& page_hitboxes)
{
    auto text = sentence_in.text;
    //LE("applyFootnoteSignature in = [%ls]",text.c_str());
    while( UnicodeHelper::isTypographicalBullet(text.front()) ||
           UnicodeHelper::isSpace(text.front()) ||
           UnicodeHelper::isLeftBracket(text.front()) ||
           UnicodeHelper::isLeftQuotation(text.front()) ||
           UnicodeHelper::isEndSentence(text.front()))
    {
        text = text.substr(1);
    }

    std::wstring digits;
    while(UnicodeHelper::isDigit(text.front()))
    {
        digits += text.at(0);
        text = text.substr(1);
    }

    while( UnicodeHelper::isTypographicalBullet(text.front()) ||
           UnicodeHelper::isSpace(text.front()) ||
           UnicodeHelper::isEndSentence(text.front()))
    {
        text = text.substr(1);
    }

    std::wstring linkText = hitboxVectorToString(slice(page_hitboxes,sentence_in));
    while( UnicodeHelper::isTypographicalBullet(linkText.front()) ||
           UnicodeHelper::isSpace(linkText.front()) ||
           UnicodeHelper::isLeftBracket(linkText.front()) ||
           UnicodeHelper::isLeftQuotation(linkText.front()) ||
           UnicodeHelper::isEndSentence(linkText.front()))
    {
        linkText = linkText.substr(1);
    }

    while( UnicodeHelper::isTypographicalBullet(linkText.back()) ||
           UnicodeHelper::isSpace(linkText.back()) ||
           UnicodeHelper::isRightBracket(linkText.back()) ||
           UnicodeHelper::isRightQuotation(linkText.back()) ||
           UnicodeHelper::isEndSentence(linkText.back()))
    {
        linkText.pop_back();
    }
    //LE("digits   = %ls",digits.c_str());
    //LE("linkText = %ls",linkText.c_str());

    std::wstring result;

    if(digits == linkText || digits.empty())
    {
        result.append(linkText);
        result.append(L". ");
        result.append(text);
    }
    //LE("applyFootnoteSignature result = [%ls]",result.c_str());

    return result;
}

std::vector<HitboxSentence> splitSentenceByFootnotes(const HitboxSentence& sentence_in, const std::vector<Hitbox>& page_hitboxes)
{
    std::vector<HitboxSentence> result_sentences;
    std::vector<Hitbox> sentence_hbs = slice(page_hitboxes, sentence_in);

    //LE("splitSentenceByFootnotes [%ls] ", sentence_in.text.c_str());

    if( sentence_hbs.empty())
    {
        LE("splitSentenceByFootnotes EMPTY SENTENCE!");
        result_sentences.emplace_back(sentence_in);
        return result_sentences;
    }

    if(!UnicodeHelper::hasDigits(sentence_in.text))
    {
        result_sentences.emplace_back(sentence_in);
        return result_sentences;
    }

    ldomNode* firstnode = NULL;
    int firstnodeIndex = 0;
    for (int index = 0; index < sentence_hbs.size() && firstnode == NULL; index++)
    {
        firstnode = sentence_hbs.at(index).getNode();
        firstnodeIndex = index;
    }

    if(firstnode == NULL)
    {
        LE("splitSentenceByFootnotes CAN'T ACQUIRE NODE ");
        result_sentences.emplace_back(sentence_in);
        return result_sentences;
    }

    lString16 nref;
    for (int index = firstnodeIndex; index < sentence_hbs.size(); index++)
    {
        ldomNode* node = sentence_hbs.at(index).getNode();
        if(node == NULL)
        {
            continue;
        }
        ldomNode* parent_check = node->getParentNode("a");
        if(parent_check != NULL)
        {
            nref = parent_check->getAttributeValue("nref");
            if(!nref.empty())
            {
                break;
            }
        }
    }

    if (nref.empty())
    {
        LE("splitSentenceByFootnotes CAN'T SEE ANY FOOTNOTE SIGNS ");
        result_sentences.emplace_back(sentence_in);
        return result_sentences;
    }

    CrDom * doc = firstnode->getCrDom();
    if(doc == NULL)
    {
        LE("splitSentenceByFootnotes CAN'T ACQUIRE CRDOM ");
        result_sentences.emplace_back(sentence_in);
        return result_sentences;
    }

    //LE("splitSentenceByFootnotes valid sentence [%ls] [%d]", sentence_in.text.c_str(), sentence_hbs.size());

    HitboxSentence s;
    s.start = sentence_in.start;
    s.end = s.start;

    ldomNode* fnote_parent = NULL;
    for (int i = 0; i < sentence_hbs.size(); i++)
    {
        auto hb = sentence_hbs.at(i);
        ldomNode* node = hb.getNode();
        if(node == NULL)
        {
            continue;
        }
        ldomNode* parent = node->getParentNode("a");
        lString16 nref = lString16::empty_str;
        if(parent != NULL)
        {
            nref = parent->getAttributeValue("nref");
        }
        //LE("hb [%ls] node [%s]",hb.text_.c_str(), LCSTR(node->getXPath()));

        if (fnote_parent != NULL && nref.empty()) // left the footnote
        {
            //LE("left the footnote");

            fnote_parent = parent;
            s.end += 1;
            s.text = applyFootnoteSignature(s, page_hitboxes);
            //LE("save [%d] -> [%d] [%ls] [%ls]", s.start, s.end,s.text.c_str(), hitboxVectorToString(slice(page_hitboxes,s)).c_str());
            result_sentences.emplace_back(s);
            s.type = 0;
            s.text = hb.text_;
            s.start = sentence_in.start + i;
            s.end = s.start;
            continue;
        }
        else if ( nref.empty() ) // not in footnote
        {
            //LE("not in footnote");
            fnote_parent = parent;

            //форсим накопление для хитбоксов не являющихся футноутом
            s.end   = s.end  + hb.text_.length();
            s.text  = s.text + hb.text_;
            //LE("forced collect [%ls]",s.text.c_str());
            continue;
        }
        else if (fnote_parent == NULL && parent != NULL && !nref.empty())  // just entered footnote
        {
            fnote_parent = parent;
            //LE("just entered footnote");

            if(!s.empty())
            {
                //LE("save [%d] -> [%d] [%ls] [%ls]", s.start, s.end,s.text.c_str(), hitboxVectorToString(slice(page_hitboxes,s)).c_str());
                result_sentences.emplace_back(s);
            }

            lString16 href = parent->getAttributeValue("href");
            if (href.empty())
            {
                continue;
            }
            //создаём новый футноут
            if (href.startsWith("#"))
            {
                href = href.substr(1);
            }
            ldomNode *linkednode = doc->getElementById(href.c_str());
            std::wstring fnote_text;
            if (linkednode != NULL)
            {
                fnote_text = std::wstring(linkednode->getText().c_str());
                //LE("Footnote text = [%ls]", fnote_text.c_str());
            }
            s.text = fnote_text;
            s.start = sentence_in.start + i;
            s.end = s.start;
            s.type = 2;
            continue;
        }
        else if (parent == fnote_parent) // still in footnote
        {
            //LE("still in footnote");

            //fnote_parent = parent;

            //накапливаем в текущий футноут
            //s.text = s.text + text // не нужно т.к в текст положили содержимое сноски
            s.end += 1;
            continue;
        }

    }
    if(!s.text.empty())
    {
        s.end += 1;
        if(s.type == 2)
        {
            s.text = applyFootnoteSignature(s, page_hitboxes);
        }
        //LE("save last [%d] -> [%d] [%ls] [%ls]", s.start, s.end,s.text.c_str(), hitboxVectorToString(slice(page_hitboxes,s)).c_str());
        result_sentences.emplace_back(s);
    }

    for(auto s: result_sentences)
    {
        //LE("splitSentenceByFootnotes result        [%d] -> [%d] [%ls] [%ls]",s.start, s.end ,s.text.c_str(), hitboxVectorToString(slice(page_hitboxes,s)).c_str());
    }
    return result_sentences;
}

std::vector<HitboxSentence> splitSentencesFootnotesWise(const std::vector<Hitbox>& page_hitboxes, const std::vector<HitboxSentence> &sentences)
{
    std::vector<HitboxSentence> new_sentences;

    for (const auto& sentence : sentences)
    {
        if(sentence.type != 0)
        {
            new_sentences.emplace_back(sentence);
            continue;
        }
        //LE("splitSentencesFootnotesWise sentence [%ls]",sentence.text.c_str());
        std::vector<HitboxSentence> urlSplit = splitSentenceByFootnotes(sentence, page_hitboxes);

        new_sentences.insert(new_sentences.end(), urlSplit.begin(), urlSplit.end());
    }
    return new_sentences;
}


std::vector<HitboxSentence> performAllSplittingsNoGeometry(const std::vector<Hitbox> &page_hitboxes, const std::vector<HitboxSentence> &sentences_in)
{
    std::vector<HitboxSentence> new_sentences;
    new_sentences = splitSentencesURLwise(sentences_in);
    //new_sentences = splitSentencesGeometryWise(page_hitboxes, new_sentences);
    new_sentences = splitSentencesUnicodeWise(new_sentences);
    new_sentences = splitSentencesFootnotesWise(page_hitboxes, new_sentences);
    new_sentences = postProcessSentences(new_sentences);
    return new_sentences;
}

HitboxSentence getFirstSentence(const std::wstring &input, const std::vector<Hitbox>& page_hitboxes)
{
    //if(input.empty()) { return std::wstring(); }
    int right_bound = sentenceRightBound(input, 0);
    auto text = input.substr(0,right_bound);

    auto sentence_in = HitboxSentence(0,right_bound,text);

    //LE("sentence [%ls]",sentence.text.c_str());
    std::vector<HitboxSentence> temp;
    temp.emplace_back(sentence_in);
    std::vector<HitboxSentence> split = performAllSplittingsNoGeometry(page_hitboxes, temp);

    if(split.empty())
    {
        return HitboxSentence();
    }
    return split.front();
}

std::vector<std::vector<Hitbox>> convertSentencesToVectors_cre(const std::vector<Hitbox> &page_hitboxes, std::vector<HitboxSentence> sentences)
{
    std::vector<std::vector<Hitbox>> hitbox_sentences;

    for (auto sentence : sentences)
    {
        //LE("new_sentence [%d -> %d] [%ls] ",sentence.start, sentence.end, sentence.text.c_str());
        std::vector<Hitbox> hb_sentence = slice(page_hitboxes, sentence);
        hitbox_sentences.emplace_back(hb_sentence);
    }
    return hitbox_sentences;
}

std::vector<std::vector<Hitbox>> getPageSentences(LVDocView* docView, int page, std::vector<HitboxSentence> *curr_sentences)
{
    //LE("getPageSentences [%d]",page);
    if(curr_sentences == NULL)
    {
        LE("getPageSentences ERROR : curr_sentences == NULL");
        return std::vector<std::vector<Hitbox>>();
    }

    bool pre_last   = false;
    bool curr_first = false;
    bool curr_last  = false;
    bool post_first = false;

    LVArray<Hitbox> prev_hbs;
    LVArray<Hitbox> curr_hbs;
    LVArray<Hitbox> post_hbs;

    std::vector<Hitbox> prev_hbs_vec;
    std::vector<Hitbox> curr_hbs_vec;
    std::vector<Hitbox> next_hbs_vec;

    HitboxSentence preSentence;
    HitboxSentence postSentence;

    //// CURRENT PAGE

    curr_hbs = docView->hitboxesCacheManager.findPageHitboxes(page);
    if (curr_hbs.empty())
    {
        docView->GoToPage(page);
        curr_hbs = docView->GetPageHitboxes();
        docView->hitboxesCacheManager.addItem(PageHitboxesCache(page, curr_hbs));
    }
    std::wstring curr;

    for(int i = 0; i < curr_hbs.size(); i++)
    {
        auto hb = curr_hbs.at(i);
        if( hb.text_.empty()) { continue; }
        auto text = hb.text_;
        auto node = hb.getNode();
        if( text.empty() || text.length() > 1) { continue; }
        if( node != NULL && node->getParentNode("rt") != NULL) { continue; }
        curr.append(hb.text_);
        curr_hbs_vec.emplace_back(hb);
    }
    *curr_sentences = generatePageSentences(curr);
    *curr_sentences = performAllSplittingsNoGeometry(curr_hbs_vec, *curr_sentences);

    std::vector<std::vector<Hitbox>> result = convertSentencesToVectors_cre(curr_hbs_vec,*curr_sentences);

    if(result.empty() || curr_sentences->empty())
    {
        return std::vector<std::vector<Hitbox>>();
    }

    if(!curr_sentences->empty())
    {
        curr_first = checkFirst(curr_sentences->front().text);
        curr_last  = checkLast(curr_sentences->back().text);
    }

    //// PREVIOUS PAGE

    int prevpage = page - 1;
    if(docView->GetColumns() >= 2)
    {
        prevpage = page - 2;
    }

    if(prevpage >= 0)
    {
        prev_hbs = docView->hitboxesCacheManager.findPageHitboxes(prevpage);
        if (prev_hbs.empty())
        {
            docView->GoToPage(prevpage);
            prev_hbs = docView->GetPageHitboxes();
            docView->hitboxesCacheManager.addItem(PageHitboxesCache(prevpage, prev_hbs));
        }

        std::wstring prev;
        for(int i = 0; i < prev_hbs.size(); i++)
        {
            auto hb = prev_hbs.at(i);
            if( hb.text_.empty()) { continue; }
            auto text = hb.text_;
            auto node = hb.getNode();
            if( text.empty() || text.length() > 1) { continue; }
            if( node != NULL && node->getParentNode("rt") != NULL) { continue; }
            prev.append(hb.text_);
            prev_hbs_vec.emplace_back(hb);
        }
        std::vector<HitboxSentence> prev_sentences = generatePageSentences(prev);
        prev_sentences = performAllSplittingsNoGeometry(prev_hbs_vec, prev_sentences);

        if(!prev_sentences.empty())
        {
            preSentence = prev_sentences.back();
            pre_last    = checkLast(preSentence.text);
            //LE("prev page [%d] last sentence [%d]->[%d] [%ls]",prevpage, preSentence.start, preSentence.end, preSentence.text.c_str());
        }
    }

    //// NEXT PAGE

    int nextpage = page + 1;
    if(docView->GetColumns() >= 2)
    {
        nextpage = page + 2;
    }

    if(curr_last && nextpage < docView->GetPagesCount())
    {
        post_hbs = docView->hitboxesCacheManager.findPageHitboxes(nextpage);
        if (post_hbs.empty())
        {
            docView->GoToPage(nextpage);
            post_hbs = docView->GetPageHitboxes();
            docView->hitboxesCacheManager.addItem(PageHitboxesCache(nextpage, post_hbs));
        }
        std::wstring next;
        for(int i = 0; i < post_hbs.size(); i++)
        {
            auto hb = post_hbs.at(i);
            auto text = hb.text_;
            auto node = hb.getNode();
            if( text.empty() || text.length() > 1) { continue; }
            if( node != NULL && node->getParentNode("rt") != NULL) { continue; }
            next.append(hb.text_);
            next_hbs_vec.emplace_back(hb);
        }

        if(!next.empty())
        {
            postSentence = getFirstSentence(next,next_hbs_vec);
            post_first = checkFirst(postSentence.text);
            //LE("next page [%d] first sentence [%d]->[%d] [%ls]",nextpage, postSentence.start, postSentence.end, postSentence.text.c_str());
        }
    }


    //// MERGING
    bool need_merge_first_split = pre_last && curr_first;
    bool need_merge_last_split  = curr_last && post_first;

    //LE("need_merge_first_split [%d]",need_merge_first_split);

    //// MERGE FIRST
    if(need_merge_first_split && !preSentence.empty())
    {
        //LE("PRE [%d] ?? [%d] [%d]", prev_hbs_vec.size(), preSentence.start, preSentence.end);
        std::vector<Hitbox> preSentence_hbs = slice(prev_hbs_vec, preSentence);

        unsigned int i = 0;
        std::string xp = prev_hbs_vec.at(i).getXpointer();
        while(xp.empty() && i < prev_hbs_vec.size() && i < preSentence.length() )
        {
            i++;
            xp = prev_hbs_vec.at(i).getXpointer();
        }

        for(auto & hb : preSentence_hbs)
        {
            hb.left_   = 0;
            hb.top_    = 0;
            hb.right_  = 0;
            hb.bottom_ = 0;
        }

        //LE("merging start sentence [%s]",xp.c_str());
        curr_sentences->front().start -= preSentence.length();
        result.front().insert(result.front().begin(),preSentence_hbs.begin(),preSentence_hbs.end());
        //result.front().front().xpointer_ = xp;
    }

    //// MERGE LAST

    //LE("need_merge_last_split  [%d]",need_merge_last_split);

    if(need_merge_last_split && !postSentence.empty())
    {
        //LE("POST [%d] ?? [%d] [%d]", next_hbs_vec.size(), postSentence.start, postSentence.end);
        std::vector<Hitbox> postSentence_hbs = slice(next_hbs_vec, postSentence);

        unsigned int i = postSentence_hbs.size() - 1;
        std::string xp = postSentence_hbs.at(i).getXpointer();
        while(xp.empty() && i >= 0 )
        {
            i--;
            xp = postSentence_hbs.at(i).getXpointer();
        }

        for(auto & hb : postSentence_hbs)
        {
            hb.left_   = 0;
            hb.top_    = 0;
            hb.right_  = 0;
            hb.bottom_ = 0;
        }

        //LE("merging end sentence [%ls] + [%ls]",hitboxVectorToString(result.back()).c_str(), hitboxVectorToString(postSentence_hbs).c_str());
        curr_sentences->back().end += postSentence.length();
        result.back().insert(result.back().end(),postSentence_hbs.begin(),postSentence_hbs.end());
        //result.back().back().xpointer_ = xp;
    }

    //for(auto s : result)
    //{
    //    LE("getPageSentences          [%ls]",filterStringForTTS(hitboxVectorToString(s)).c_str());
    //}
    return result;
}


std::wstring CreBridge::TTSRestoreIndicRTLText(const HitboxSentence& sentence, int external_page)
{
    int id_start = sentence.start;
    int id_end   = sentence.end;
    auto page = (uint32_t) ImportPage(external_page, doc_view_->GetColumns());
    doc_view_->GoToPage(page);
    LVArray<Hitbox> hitboxes = doc_view_->GetPageHitboxes();
    LVArray<SortStruct> array;
    long long int lastindex;

    if(id_start < 0)
    {
        id_start = 0;
    }
    if(id_end >= hitboxes.size())
    {
        id_end = hitboxes.size()-1;
    }

    for (int i = id_start; i <= id_end; i++)
    {
        Hitbox hb = hitboxes.get(i);
        long long int index;
        if (hb.getNode()->isNull())
        {
            index = lastindex;
        }
        else
        {
            index = hb.getNode()->getDataIndex() * 1000000;
            index += hb.word_.getStartXPointer().getOffset();
            lastindex = index;
        }
        SortStruct str;
        str.text = hb.text_.c_str();
        str.weight = index;
        array.add(str);
    }

    qsort(array.get(), array.length(), sizeof(struct SortStruct), compareByWeight);

    lString16 result;

    for (int i = 0; i < array.length(); i++)
    {
        result+= array.get(i).text;
    }
    //LE("TTSRestoreIndicRTLText in = [%s]",LCSTR(result));
    result = result.ReversePrettyLetters();
    result = result.restoreIndicText();
    //LE("TTSRestoreIndicRTLText out = [%s]",LCSTR(result));

    return std::wstring(result.c_str());
}

void CreBridge::processGetTTSSentence(CmdRequest &request, CmdResponse &response)
{
    response.cmd = CMD_RES_TTS_SENTENCE;
    CmdDataIterator iter(request.first);
    uint32_t external_page = 0;
    iter.getInt(&external_page);
    if (!iter.isValid())
    {
        LE("processGetTTSSentence bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    auto page = (uint32_t) ImportPage(external_page, doc_view_->GetColumns());
    //page = 6;
    std::vector<HitboxSentence> sentences_list;
    std::vector<std::vector<Hitbox>> sentences = getPageSentences(doc_view_,page, &sentences_list);

    //        xpointer_ = std::string(LCSTR(word.getStartXPointer().toString()));
    for( int i = 0; i < sentences.size(); i++)
    {

        auto sentence_vec = sentences.at(i);
        auto sentence = sentences_list.at(i);

        std::wstring wtext1    = filterStringForTTS(hitboxVectorToString(sentence_vec));

        //LE("processGetTTSSentence [%d] type [%d] [%d] -> [%d] text [%ls]", page, sentence.start, sentence.end, sentence.type, wtext1.c_str());

        if(sentence.type == 2) // footnote
        {
            wtext1 = filterStringForTTS(sentence.text);
            //LE("                                               sub text [%ls]", wtext1.c_str());
        }


        //std::wstring wtext2    = filterStringForTTS(sentence.text);
        //if(wtext1 != wtext2)
        //{
        //    LE("processGetTTSSentence SENTENCE [%d] TEXTS ARE NOT EQUAL \n[%ls] \n!= \n[%ls]", i, wtext1.c_str(), wtext2.c_str());
        //}

        if( checkIndic(wtext1) || UnicodeHelper::checkRTL(wtext1))
        {
            wtext1 = TTSRestoreIndicRTLText(sentence, page);
        }

        std::string  text     = wstringToString(wtext1);
        std::string  xp_start = sentence_vec.front().getXpointer();
        std::string  xp_end   = sentence_vec.back().getXpointer();
        float x = sentence_vec.front().left_;
        float y = sentence_vec.front().top_;

        if (xp_start.empty())
        {
            int startIndex = getFirstValidHitboxIndex(sentence_vec);
            if (startIndex == -1)
            {
                LE("processGetTTSSentence DEBUG XP_START EMPTY FOR SENTENCE [%s]", text.c_str());
                continue;
            }
            xp_start = sentence_vec.at(startIndex).getXpointer();
            x = sentence_vec.at(startIndex).left_;
            y = sentence_vec.at(startIndex).top_;
        }

        if(xp_end.empty())
        {
            int endIndex = getLastValidHitboxIndex(sentence_vec);
            if (endIndex == -1)
            {
                LE("processGetTTSSentence DEBUG XP_END EMPTY FOR SENTENCE [%s]", text.c_str());
                continue;
            }
            xp_end = sentence_vec.at(endIndex).getXpointer();
        }

        //LE("processGetTTSSentence [%d] type [%d] text [%s]", page, sentence.type, text.c_str());
        //LE("processGetTTSSentence [%d]       start xp [%s]", page, xp_start.c_str());
        //LE("processGetTTSSentence [%d]       end   xp [%s]", page, xp_end.c_str());

        response.addIpcString(text.c_str(), true);
        response.addIpcString(xp_start.c_str(), true);
        response.addIpcString(xp_end.c_str()  , true);
        response.addInt(sentence.type);
        response.addFloat(x);
        response.addFloat(y);
    }
}

void CreBridge::processGetTTSXpathSentence(CmdRequest &request, CmdResponse &response)
{
    response.cmd = CMD_RES_TTS_XPATH_SENTENCE;
    CmdDataIterator iter(request.first);
    uint32_t external_page = 0;
    uint8_t* xpath_string;

    iter.getInt(&external_page);
    iter.getByteArray(&xpath_string);
    if (!iter.isValid())
    {
        LE("processGetTTSXpathSentence bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    auto page = (uint32_t) ImportPage(external_page, doc_view_->GetColumns());
    std::string xpath(reinterpret_cast<const char*>(xpath_string));

    //LE("processGetTTSXpathSentence page [%d] xpath [%s]",page,xpath.c_str());

    LVArray<Hitbox> hbs = doc_view_->hitboxesCacheManager.findPageHitboxes(page);
    if (hbs.empty())
    {
        doc_view_->GoToPage(page);
        hbs = doc_view_->GetPageHitboxes();
        doc_view_->hitboxesCacheManager.addItem(PageHitboxesCache(page, hbs));
    }

    int selected_index = -1;

    for(int i = 0; i < hbs.size(); i++)
    {
        auto hb = hbs.at(i);
        if(hb.checkXpointer(xpath))
        {
            selected_index = i;
            break;
        }
    }

    if (selected_index == -1)
    {
        //fallback search
        selected_index = doc_view_->doRoughXpathSearch(lString16(xpath.c_str(), xpath.length()), hbs);

        if (selected_index == -1)
        {
            LE("processGetTTSXpathSentence could not find xpath [%s]",xpath.c_str());
            return;
        }
    }

    std::vector<HitboxSentence> sentences_list;
    std::vector<std::vector<Hitbox>> sentences = getPageSentences(doc_view_,page, &sentences_list);
    std::vector<Hitbox> sentence_vec = std::vector<Hitbox>();
    HitboxSentence sentence;
    //LW("selected_index = %d",selected_index);

    for (int i = 0; i < sentences_list.size(); i++)
    {
        auto s = sentences_list.at(i);
        if(s.contains(selected_index))
        {
            sentence_vec = sentences.at(i);
            sentence = sentences_list.at(i);
            //LE("found at = %d [%ls]",i,sentence.text.c_str());
            break;
        }
    }

    if(sentence_vec.empty())
    {
        LE("processGetTTSXpathSentence could not find sentence");
        return;
    }

    std::wstring wtext1    = filterStringForTTS(hitboxVectorToString(sentence_vec));
    if( checkIndic(wtext1) || UnicodeHelper::checkRTL(wtext1))
    {
        wtext1 = TTSRestoreIndicRTLText(sentence, page);
    }

    //std::wstring wtext2    = filterStringForTTS(sentence.text);
    //if(wtext1 != wtext2)
    //{
    //    LE("processGetTTSXpathSentence SENTENCE TEXTS ARE NOT EQUAL \n[%ls] \n!= \n[%ls]", wtext1.c_str(), wtext2.c_str());
    //}

    std::string text     = wstringToString(wtext1);
    std::string xp_start = sentence_vec.front().getXpointer();
    std::string xp_end   = sentence_vec.back().getXpointer();
    float x = sentence_vec.front().left_;
    float y = sentence_vec.front().top_;

    if (xp_start.empty())
    {
        int startIndex = getFirstValidHitboxIndex(sentence_vec);
        if (startIndex == -1)
        {
            LE("processGetTTSXpathSentence DEBUG XP_START EMPTY FOR SENTENCE [%s]", text.c_str());
            return;
        }
        xp_start = sentence_vec.at(startIndex).getXpointer();
        x = sentence_vec.at(startIndex).left_;
        y = sentence_vec.at(startIndex).top_;
    }


    if(xp_end.empty())
    {
        int endIndex = getLastValidHitboxIndex(sentence_vec);
        if (endIndex == -1)
        {
            LE("processGetTTSXpathSentence DEBUG XP_END EMPTY FOR SENTENCE [%s]", text.c_str());
            return;
        }
        xp_end = sentence_vec.at(endIndex).getXpointer();
    }

    //LE("processGetTTSXpathSentence text     [%s]",text.c_str());
    //LE("processGetTTSXpathSentence start xp [%s]",xp_start.c_str());
    //LE("processGetTTSXpathSentence end   xp [%s]",xp_end.c_str());

    response.addIpcString(text.c_str(), true);
    response.addIpcString(xp_start.c_str(), true);
    response.addIpcString(xp_end.c_str()  , true);
    response.addInt(sentence.type);
    response.addFloat(x);
    response.addFloat(y);
}

void CreBridge::processGetTTSHitboxes(CmdRequest &request, CmdResponse &response)
{
    response.cmd = CMD_RES_TTS_HITBOXES;
    CmdDataIterator iter(request.first);
    uint32_t external_page = 0;
    iter.getInt(&external_page);
    if (!iter.isValid())
    {
        LE("processGetTTSHitboxes bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    auto page = (uint32_t) ImportPage(external_page, doc_view_->GetColumns());

    std::vector<HitboxSentence> sentences_list;
    std::vector<std::vector<Hitbox>> sentences = getPageSentences(doc_view_, page, &sentences_list);

    if (sentences.empty())
    {
        response.addFloat(0);
        response.addFloat(0);
        response.addFloat(0);
        response.addFloat(0);
        responseAddString(response, L":");
        return;
    }

    for (auto sentence : sentences)
    {
        //LE("processGetTTSHitboxes sentence [%ls]",hitboxVectorToString(sentence).c_str());
        if(sentence.empty())
        {
            response.addFloat(0);
            response.addFloat(0);
            response.addFloat(0);
            response.addFloat(0);
            responseAddString(response, L":");
            continue;
        }

        std::vector<Hitbox> filteredHitboxes;
        for(auto hb: sentence)
        {
            if(hb.isZero())
            {
                continue;
            }
            filteredHitboxes.emplace_back(hb);
        }

        filteredHitboxes = doc_view_->unionRects(filteredHitboxes);

        //        xpointer_ = std::string(LCSTR(word.getStartXPointer().toString()));

        std::string xp_start = sentence.front().getXpointer();
        std::string xp_end   = sentence.back().getXpointer();

        if (xp_start.empty())
        {
            int startIndex = getFirstValidHitboxIndex(sentence);
            if (startIndex == -1)
            {
                LE("processGetTTSHitboxes DEBUG XP_START EMPTY FOR SENTENCE [%ls]", hitboxVectorToString(sentence).c_str());
                continue;
            }
            xp_start = sentence.at(startIndex).getXpointer();
        }

        if(xp_end.empty())
        {
            int endIndex = getLastValidHitboxIndex(sentence);
            if (endIndex == -1)
            {
                LE("processGetTTSHitboxes DEBUG XP_END EMPTY FOR SENTENCE  [%ls]", hitboxVectorToString(sentence).c_str());
                continue;
            }
            xp_end = sentence.at(endIndex).getXpointer();
        }

        std::string xpathKey;
        xpathKey.append(xp_start);
        xpathKey.append(":");
        xpathKey.append(xp_end);
        //LE("resp_key [%s]",xpathKey.c_str());

        for (int i = 0; i < filteredHitboxes.size(); i++)
        {
            Hitbox_st currHitbox = filteredHitboxes.at(i);
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
            response.addIpcString(xpathKey.c_str(), true);
        }
    }
}

