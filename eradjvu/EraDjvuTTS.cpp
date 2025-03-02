//
// Created by Tarasus on 20.04.2023.
//

#include "EraDjvuBridge.h"
#include "StSearchUtils.h"

std::vector<std::vector<Hitbox_st>> DjvuBridge::getPageSentences(int page, std::vector<HitboxSentence> *curr_sentences)
{
    //LE("getPageSentences [%d]",page);

    if(curr_sentences == NULL)
    {
        LE("getPageSentences ERROR : curr_sentences == NULL");
        return std::vector<std::vector<Hitbox_st>>();
    }

    bool pre_last   = false;
    bool curr_first = false;
    bool curr_last  = false;
    bool post_first = false;

    std::vector<Hitbox_st> prev_hbs_vec;
    std::vector<Hitbox_st> curr_hbs_vec;
    std::vector<Hitbox_st> next_hbs_vec;

    HitboxSentence preSentence;
    HitboxSentence postSentence;

    std::vector<Hitbox_st> preSentence_hbs;
    std::vector<Hitbox_st> postSentence_hbs;

    curr_hbs_vec = processTextToArray(page);
    curr_hbs_vec = filterTextViaColontitles(page, curr_hbs_vec, colontitles_map);

    std::wstring curr = hitbox_stVectorToString(curr_hbs_vec);
    *curr_sentences = generatePageSentences(curr);

    *curr_sentences = performAllSplittings(curr_hbs_vec, *curr_sentences, true);
    std::vector<std::vector<Hitbox_st>> result = convertSentencesToVectors(curr_hbs_vec, *curr_sentences);

    if(result.empty() || curr_sentences->empty())
    {
        return std::vector<std::vector<Hitbox_st>>();
    }

    if(!curr_sentences->empty())
    {
        curr_first = checkFirst(curr_sentences->front().text);
        curr_last  = checkLast(curr_sentences->back().text);
    }

    if(page > 0)
    {
        prev_hbs_vec = processTextToArray(page-1);
        prev_hbs_vec = filterTextViaColontitles(page-1, prev_hbs_vec, colontitles_map);

        std::wstring prev = hitbox_stVectorToString(prev_hbs_vec);

        std::vector<HitboxSentence> prev_sentences = generatePageSentences(prev);
        prev_sentences = performAllSplittings(prev_hbs_vec, prev_sentences, true);

        if(!prev_sentences.empty())
        {
            preSentence = prev_sentences.back();
            preSentence_hbs = sliceHitbox_st_vector(prev_hbs_vec, preSentence);
            pre_last = checkLast(preSentence.text);
            //LE("prev page [%d] last sentence [%ls] ",page-1, preSentence.text.c_str());
            //LE("pre_last1 = %d",pre_last);
            if(pre_last && !preSentence_hbs.empty())
            {
                Hitbox_st pre_end_hb   = preSentence_hbs.back();
                Hitbox_st curr_hb      = result.front().front();

                float pre_height  = fabs(pre_end_hb.top_ - pre_end_hb.bottom_);
                float curr_height = fabs(curr_hb.top_    - curr_hb.bottom_);
                float height_threshold = std::min(curr_height, pre_height) * 0.3f;

                //LE("pre_height = %f , curr_height = %f, diff = %f < %f",pre_height, curr_height, fabs(pre_height - curr_height), height_threshold);

                bool size_difference_too_big = fabs(pre_height - curr_height) > height_threshold;
                if(size_difference_too_big)
                {
                    pre_last = false;
                    //LE("pre_last size_difference_too_big = %f > %f ",fabs(pre_height - curr_height), height_threshold);
                }
                else
                {
                    //end is not in zone
                    pre_last = (pre_end_hb.left_ <= (curr_hb.left_ - TTS_ALLOWED_ZONE_SIZE)) ||
                               (pre_end_hb.left_ >= (curr_hb.left_ + TTS_ALLOWED_ZONE_SIZE));
                    //LE("pre_last = %d : end is not in zone ",pre_last);
                }

                if(pre_last)
                {
                    pre_last = (pre_end_hb.top_ > 0.75f && pre_end_hb.bottom_ > 0.75f);
                    //LE("pre_last end geo = %d : top: %f ? 0.75f , bottom ? 0.75f",pre_last, pre_end_hb.top_, pre_end_hb.bottom_ );
                }

                //LE("pre_last2 = %d",pre_last);
            }
        }
    }


    if(curr_last && page + 1 < this->pageCount)
    {
        next_hbs_vec = processTextToArray(page+1);
        next_hbs_vec = filterTextViaColontitles(page+1, next_hbs_vec, colontitles_map);

        std::wstring next = hitbox_stVectorToString(next_hbs_vec);

        if(!next.empty())
        {
            postSentence = getFirstSentence(next, next_hbs_vec, true);
            postSentence_hbs = sliceHitbox_st_vector(next_hbs_vec, postSentence);
            post_first = checkFirst(postSentence.text);
            //LE("next page [%d] first sentence [%ls]",page+1, postSentence.text.c_str());
            //LE("post_first1 = %d",post_first);

            if(post_first && !postSentence_hbs.empty())
            {
                Hitbox_st curr_hb = result.back().back();
                Hitbox_st post_start_hb = postSentence_hbs.front();

                float curr_height = fabs(curr_hb.top_ - curr_hb.bottom_);
                float post_height = fabs(post_start_hb.top_ - post_start_hb.bottom_);
                float height_threshold = std::min(curr_height, post_height) * 0.3f;
                //LE("pre_height = %f , curr_height = %f, diff = %f < %f",post_height, curr_height, fabs(post_height - curr_height), height_threshold);

                bool size_difference_too_big = fabs(post_height - curr_height) > height_threshold;
                if(size_difference_too_big)
                {
                    post_first = false;
                    //LE("post_first size_difference_too_big = %f > %f ",fabs(post_height - curr_height), height_threshold);
                }

                if(post_first)
                {
                    post_first = (post_start_hb.top_ < 0.25f && post_start_hb.bottom_ < 0.25f);
                    //LE("post_first start geo = %d : top: %f ? 0.25f , bottom %f ? 0.25f",pre_last, post_start_hb.top_, post_start_hb.bottom_ );
                }
                //LE("post_first2 = %d",post_first);
            }
        }
    }


    bool need_merge_first_split = pre_last  && curr_first;
    bool need_merge_last_split  = curr_last && post_first;

    //LE("need_merge_first_split [%d]",need_merge_first_split);

    if(need_merge_first_split && !preSentence.empty())
    {
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

    //LE("need_merge_last_split  [%d]",need_merge_last_split);

    if(need_merge_last_split && !postSentence.empty())
    {
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

        //LE("merging end sentence [%s]",xp.c_str());
        curr_sentences->back().end += postSentence.length();
        result.back().insert(result.back().end(),postSentence_hbs.begin(),postSentence_hbs.end());
        //result.back().back().xpointer_ = xp;
    }

    return result;
}

void DjvuBridge::processGetTTSSentence(CmdRequest &request, CmdResponse &response)
{
    response.cmd = CMD_RES_TTS_SENTENCE;
    uint32_t page;
    CmdDataIterator iter(request.first);
    if (!iter.getInt(&page).isValid())
    {
        LE("Bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }

    LD("processGetTTSSentence page %d", page);

    std::vector<HitboxSentence> sentences_list;
    std::vector<std::vector<Hitbox_st>> sentences = getPageSentences(page, &sentences_list);

    //        xpointer_ = std::string(LCSTR(word.getStartXPointer().toString()));
    for( int i = 0; i < sentences.size(); i++)
    {
        auto sentence_vec = sentences.at(i);
        auto sentence = sentences_list.at(i);

        std::wstring wtext1    = filterStringForTTS(hitbox_stVectorToString(sentence_vec));
        //std::wstring wtext2    = filterStringForTTS(sentence.text);
        //if(wtext1 != wtext2)
        //{
        //    LE("processGetTTSSentence SENTENCE [%d] TEXTS ARE NOT EQUAL \n[%ls] \n!= \n[%ls]", i, wtext1.c_str(), wtext2.c_str());
        //}
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
            xp_start = sentence_vec.at(startIndex).getXpointer() + getCoords(sentence_vec.at(startIndex));;
            x = sentence_vec.at(startIndex).left_;
            y = sentence_vec.at(startIndex).top_;
        }
        else
        {
            xp_start += getCoords(sentence_vec.front());
        }

        if(xp_end.empty())
        {
            int endIndex = getLastValidHitboxIndex(sentence_vec);
            if (endIndex == -1)
            {
                LE("processGetTTSSentence DEBUG XP_END EMPTY FOR SENTENCE [%s]", text.c_str());
                continue;
            }
            xp_end = sentence_vec.at(endIndex).getXpointer() + getCoords(sentence_vec.at(endIndex));
        }
        else
        {
            xp_end += getCoords(sentence_vec.back());
        }

        //LE("EraDjvu processGetTTSSentence [%d] type [%d] text [%s]", page, sentence.type , text.c_str());
        //LE("EraDjvu processGetTTSSentence [%d]       start xp [%s]", page, xp_start.c_str());
        //LE("EraDjvu processGetTTSSentence [%d]       end   xp [%s]", page, xp_end.c_str());

        response.addIpcString(text.c_str(), true);
        response.addIpcString(xp_start.c_str(), true);
        response.addIpcString(xp_end.c_str()  , true);
        response.addInt(sentence.type);
        response.addFloat(x);
        response.addFloat(y);
    }
}

void DjvuBridge::processGetTTSXpathSentence(CmdRequest &request, CmdResponse &response)
{
    response.cmd = CMD_RES_TTS_XPATH_SENTENCE;
    CmdDataIterator iter(request.first);
    uint32_t page = 0;
    uint8_t* xpath_string;

    iter.getInt(&page);
    iter.getByteArray(&xpath_string);
    if (!iter.isValid())
    {
        LE("processGetTTSXpathSentence bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    std::string xpath(reinterpret_cast<const char*>(xpath_string));

    std::vector<Hitbox_st> hbs = processTextToArray(page);
    hbs = filterTextViaColontitles(page, hbs, colontitles_map);

    int selected_index = -1;

    for(int i = 0; i < hbs.size(); i++)
    {
        auto hb = hbs.at(i);
        auto xp = hb.getXpointer();

        if(xp == xpath)
        {
            selected_index = i;
            break;
        }
    }

    if (selected_index == -1)
    {
        LE("processGetTTSXpathSentence could not find xpath [%s]",xpath.c_str());
        return;
    }

    std::vector<HitboxSentence> sentences_list;
    std::vector<std::vector<Hitbox_st>> sentences = getPageSentences(page, &sentences_list);
    std::vector<Hitbox_st> sentence_vec = std::vector<Hitbox_st>();
    HitboxSentence sentence;

    //LE("selected_index = %d",selected_index);

    for (int i = 0; i < sentences_list.size(); i++)
    {
        auto s = sentences_list.at(i);
        if(s.contains(selected_index))
        {
            sentence_vec = sentences.at(i);
            sentence = sentences_list.at(i);
            //LE("found at = %d [%ls]",i,hitbox_stVectorToString(result_sentence).c_str());
            break;
        }
    }

    if(sentence_vec.empty())
    {
        LE("processGetTTSXpathSentence could not find sentence");
        return;
    }

    std::wstring wtext1    = filterStringForTTS(hitbox_stVectorToString(sentence_vec));
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
        xp_start = sentence_vec.at(startIndex).getXpointer() + getCoords(sentence_vec.at(startIndex));;
        x = sentence_vec.at(startIndex).left_;
        y = sentence_vec.at(startIndex).top_;
    }
    else
    {
        xp_start += getCoords(sentence_vec.front());
    }

    if(xp_end.empty())
    {
        int endIndex = getLastValidHitboxIndex(sentence_vec);
        if (endIndex == -1)
        {
            LE("processGetTTSXpathSentence DEBUG XP_END EMPTY FOR SENTENCE [%s]", text.c_str());
            return;
        }
        xp_end = sentence_vec.at(endIndex).getXpointer() + getCoords(sentence_vec.at(endIndex));
    }
    else
    {
        xp_end += getCoords(sentence_vec.back());
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

void DjvuBridge::processGetTTSHitboxes(CmdRequest &request, CmdResponse &response)
{
    response.cmd = CMD_RES_TTS_HITBOXES;
    CmdDataIterator iter(request.first);
    uint32_t page = 0;
    iter.getInt(&page);
    if (!iter.isValid())
    {
        LE("processGetTTSHitboxes bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    std::vector<HitboxSentence> sentences_list;
    std::vector<std::vector<Hitbox_st>> sentences = getPageSentences(page, &sentences_list);

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
        //LE("processGetTTSHitboxes sentence [%ls]",hitbox_stVectorToString(sentence).c_str());
        if(sentence.empty())
        {
            response.addFloat(0);
            response.addFloat(0);
            response.addFloat(0);
            response.addFloat(0);
            responseAddString(response, L":");
            continue;
        }
        std::vector<Hitbox_st> filteredHitboxes;
        for(auto hb: sentence)
        {
            if(hb.isZero())
            {
                continue;
            }
            filteredHitboxes.emplace_back(hb);
        }

        filteredHitboxes = unionRects(filteredHitboxes);

        std::string xp_start = sentence.front().getXpointer();
        std::string xp_end   = sentence.back().getXpointer();

        if (xp_start.empty())
        {
            int startIndex = getFirstValidHitboxIndex(sentence);
            if (startIndex == -1)
            {
                LE("processGetTTSHitboxes DEBUG XP_START EMPTY FOR SENTENCE [%ls]", hitbox_stVectorToString(sentence).c_str());
                continue;
            }
            xp_start = sentence.at(startIndex).getXpointer();
        }

        if(xp_end.empty())
        {
            int endIndex = getLastValidHitboxIndex(sentence);
            if (endIndex == -1)
            {
                LE("processGetTTSHitboxes DEBUG XP_END EMPTY FOR SENTENCE  [%ls]", hitbox_stVectorToString(sentence).c_str());
                continue;
            }
            xp_end = sentence.at(endIndex).getXpointer();
        }

        std::string xpathKey;
        xpathKey.append(xp_start);
        xpathKey.append(":");
        xpathKey.append(xp_end);
        //LE("resp_key [%s]",xpathKey.c_str());

        for (auto hb : filteredHitboxes)
        {
            response.addFloat(hb.left_ );
            response.addFloat(hb.top_);
            response.addFloat(hb.right_ );
            response.addFloat(hb.bottom_);

            response.addIpcString(xpathKey.c_str(), true);
        }
    }
}

void DjvuBridge::processSmartColontitlesText(CmdRequest& request, CmdResponse& response)
{
    response.cmd = CMD_RES_CALCULATE_COLONTITLES_TEXT;
    if (request.dataCount == 0) {
        LE("No request data found");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    if (doc == NULL || pages == NULL) {
        LE("Document not yet opened");
        response.result = RES_ILLEGAL_STATE;
        return;
    }

    uint32_t page_index;
    CmdDataIterator iter(request.first);
    iter.getInt(&page_index);

    if (!iter.isValid()) {
        LE("Bad request data");
        response.result = RES_BAD_REQ_DATA;
        return;
    }
    //LE("processSmartColontitlesText page [%d]",page_index);

    std::vector<Hitbox_st> page_hitboxes = processTextToArray(page_index);
    float colontitles[4] = {0, 0, 1, 1};

    std::sort(page_hitboxes.begin(), page_hitboxes.end(), hitboxComparator );

    colontitles[1] = calculateUpperColontitle(page_hitboxes);
    colontitles[3] = calculateLowerColontitle(page_hitboxes);

    if(colontitles[1] > 0.2)
    {
        colontitles[1] = 0;
    }

    if(colontitles[3] < 0.8)
    {
        colontitles[3] = 1;
    }

    response.addFloatArray(4, colontitles, true);
    //LE("processSmartColontitlesText t = [%f] b = [%f]",colontitles[1],colontitles[3]);
}
