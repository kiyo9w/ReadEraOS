//
// Created by Tarasus on 13.04.2023.
//

#include <utility>
#include <sstream>
#include "ore_log.h"
#include "StSearchUtils.h"

const std::vector<std::wstring> url_parts_start = { L"https://", L"http://", L"www.", L"WWW."};
std::vector<std::wstring> url_parts_end = { L".info", L".com", L".org", L".net", L".edu", L".top",
                                            L".xyz", L".ru", L".en", L".co", L".us", L".de", L".uk",
                                            L".io", L".tk", L".ga", L".cn", L".cf", L".nl", L".htm", L".html"};

std::vector<std::wstring> url_parts_contains = { L".info/", L".com/", L".org/", L".net/", L".edu/", L".top/",
                                                 L".xyz/", L".ru/", L".en/", L".co/", L".us/", L".de/", L".uk/",
                                                 L".io/", L".tk/", L".ga/", L".cn/", L".cf/", L".nl/" };

const std::vector<std::wstring> parts_end   = { L" ", L"\n", L".", L"," };


std::vector<HitboxSentence> postProcessSentences(const std::vector<HitboxSentence> &sentences_in)
{
    std::vector<HitboxSentence> result_sentences;

    bool isFirstSentence = true;
    for (int i = 0; i < sentences_in.size(); i++)
    {
        auto sentence = sentences_in[i];
        bool isLastSentence = (i == sentences_in.size() - 1);

        //LE("postProcessSentences IN first [%d] last [%d] [%ls]",isFirstSentence,isLastSentence,sentence.text.c_str());
        if(sentence.empty())
        {
            continue;
        }
        else if(sentence.length() == 1)
        {
            wchar_t ch = sentence.text.front();
            if( UnicodeHelper::isTypographicalBullet(ch) ||
                UnicodeHelper::isSpace(ch) ||
                UnicodeHelper::isEndSentence(ch) ||
                UnicodeHelper::isNewline(ch) ||
                ch == L',' ||
                ch == L';' ||
                ch == L'—' ||
                ch == L':')
            {
                continue;
            }
            else
            {
                //LE("postProcessSentences OUT 1 [%ls]",sentence.text.c_str());
                result_sentences.emplace_back(sentence);
            }
        }
        else
        {
            auto new_sentence = sentence;
            int bracketCounter = 0;
            int quoteCounter = 0;

            int backBracketCounter = 0;
            int backQuoteCounter = 0;
            auto cnt_text = sentence.text;

            //count back brackets and quotes first
            while(!cnt_text.empty())
            {
                wchar_t ch = cnt_text.back();

                if (UnicodeHelper::isTypographicalBullet(ch) ||
                    UnicodeHelper::isSpace(ch))
                {
                    cnt_text.pop_back();
                }
                else if(UnicodeHelper::isRightBracket(ch))
                {
                    backBracketCounter++;
                    cnt_text.pop_back();
                }
                else if(UnicodeHelper::isRightQuotation(ch))
                {
                    backQuoteCounter++;
                    cnt_text.pop_back();
                }
                else
                {
                    break;
                }
            }

            while(!new_sentence.text.empty())
            {
                wchar_t ch = new_sentence.text.front();

                if (UnicodeHelper::isTypographicalBullet(ch) ||
                    UnicodeHelper::isSpace(ch) )
                {
                    //LE("front removed [%lc]",ch);
                    new_sentence.text = new_sentence.text.substr(1, new_sentence.text.length() - 1);
                    new_sentence.start ++;
                }
                else if(backBracketCounter > 0 && UnicodeHelper::isLeftBracket(ch))
                {
                    //remove front bracket
                    backBracketCounter--;
                    bracketCounter++;
                    new_sentence.text = new_sentence.text.substr(1);
                    new_sentence.start ++;
                    //LE("front removed bracket %lc : %d",ch,bracketCounter);
                }
                else if(backQuoteCounter > 0 && UnicodeHelper::isLeftQuotation(ch))
                {
                    //remove front bracket
                    backQuoteCounter--;
                    quoteCounter++;
                    new_sentence.text = new_sentence.text.substr(1);
                    new_sentence.start ++;
                    //LE("front removed quote %lc : %d",ch,quoteCounter);
                }
                else
                {
                    break;
                }
            }

            while(!new_sentence.text.empty())
            {
                wchar_t ch = new_sentence.text.back();

                if (UnicodeHelper::isTypographicalBullet(ch))
                {
                    new_sentence.text.pop_back();
                    new_sentence.end --;
                    //LE("back removed [%lc]",ch);
                }
                else if (UnicodeHelper::isSpace(ch) && !isFirstSentence && !isLastSentence)
                {
                    new_sentence.text.pop_back();
                    new_sentence.end --;
                    //LE("back removed space [%lc]",ch);
                }
                else if(UnicodeHelper::isRightBracket(ch) && bracketCounter > 0)
                {
                    //remove back bracket
                    bracketCounter--;
                    new_sentence.text.pop_back();
                    new_sentence.end --;
                    //LE("back removed bracket %lc : %d",ch,bracketCounter);
                }
                else if(UnicodeHelper::isRightQuotation(ch) && quoteCounter > 0)
                {
                    //remove back bracket
                    quoteCounter--;
                    new_sentence.text.pop_back();
                    new_sentence.end --;
                    //LE("back removed quote %lc : %d",ch,quoteCounter);
                }
                else
                {
                    break;
                }
            }

            if(bracketCounter > 0)
            {
                LE("postProcessSentences deleted uneven amount of left/right brackets bracketCounter = [%d]",bracketCounter);
            }

            if(quoteCounter > 0)
            {
                LE("postProcessSentences deleted uneven amount of left/right quotes quoteCounter = [%d]",quoteCounter);
            }

            if(new_sentence.length() > 0)
            {
                //LE("postProcessSentences OUT 2 [%ls]",new_sentence.text.c_str());
                isFirstSentence = false;
                result_sentences.emplace_back(new_sentence);
            }
        }
    }
    return result_sentences;
}

int findUrlPart(const std::wstring& text, const std::vector<std::wstring>& parts, int begin, bool isEnd = false )
{
    int foundPos = -1;
    for (const auto& url_part : parts)
    {
        auto pos = text.find(url_part, begin);
        if( pos != std::wstring::npos)
        {
            foundPos = pos;
            if(isEnd)
            {
                foundPos+= url_part.size();
            }
            break;
        }
    }
    return foundPos;
}

bool CheckStringUrl(const std::wstring& text_in)
{
    std::wstring text = text_in;
    while (endsWith(text, parts_end))
    {
        text.pop_back();
    }
    while (startsWith(text, L" "))
    {
        text = text.substr(1); // pop_front
    }
    if(text.find(L'.') == std::wstring::npos)
    {
        return false;
    }
    if(std::any_of(text.begin(),text.end(),
                   [](wchar_t ch){
                                    return !(UnicodeHelper::isLatin(ch) ||
                                             UnicodeHelper::RFCValidURLChar(ch) ||
                                             UnicodeHelper::isDigit(ch));
                                 }))
    {
        return false;
    }
    return startsWith(text, url_parts_start) || endsWith(text, url_parts_end) || contains(text, url_parts_contains);
}

std::vector<HitboxSentence> splitSentenceByURL(const HitboxSentence& sentence_in)
{
    std::vector<HitboxSentence> result_sentences;
    std::vector<std::wstring> parts;
    //LE("splitSentenceByURL [%ls] ", sentence_in.text.c_str());

    std::wstring delims = L" \n\t";
    auto start = 0;
    auto end = sentence_in.text.find_first_of(delims);
    while (end != std::string::npos)
    {
        parts.emplace_back(sentence_in.text.substr(start, (end - start) + 1));
        start = end + 1;
        end = sentence_in.text.find_first_of(delims, start);
    }
    parts.emplace_back(sentence_in.text.substr(start));

    if(parts.empty())
    {
        //error?
        result_sentences.emplace_back(sentence_in);
        return result_sentences;
    }

    HitboxSentence s;
    s.start = sentence_in.start;
    s.end = s.start;

    bool lastIsURL = CheckStringUrl(parts.front());

    for( const auto& text : parts)
    {
        bool isUrl = CheckStringUrl(text);
        //text += L" ";
        //LE("part isUrl [%d] [%ls] ", isUrl, text.c_str());

        if(!isUrl)
        {
            //накапливаем
            s.end   = s.end + text.length();
            s.text  = s.text + text;
            //LE("collect text [%ls]",s.text.c_str());
            lastIsURL = isUrl;
        }
        else if (!lastIsURL) // && isUrl //always true
        {
            //сохраняем накопленное
            //LE("save text [%d] -> [%d] [%ls]", s.start, s.end,s.text.c_str());
            result_sentences.emplace_back(s);

            lastIsURL = isUrl;
            //сохраняем ссылку
            s.start = s.end;
            s.end = s.start + text.length();
            s.text = text;
            s.type = 1;
            //LE("save new link [%d] - [%d] [%ls]",s.start, s.end, s.text.c_str());
            result_sentences.emplace_back(s);
            s.text.clear();
            s.start = s.end;
            s.type = 0;
        }
        else // if (isUrl && lastIsURL) //always true
        {
            lastIsURL = isUrl;
            //сохраняем ссылку
            s.start = s.end;
            s.end = s.start + text.length();
            s.text = text;
            s.type = 1;
            //LE("save new link 2 [%d] - [%d] [%ls]",s.start, s.end, s.text.c_str());
            result_sentences.emplace_back(s);
            s.text.clear();
            s.start = s.end;
            s.type = 0;
        }
    }
    if(!s.text.empty())
    {
        bool isUrl = CheckStringUrl(s.text);
        s.type = isUrl ? 1 : 0;
        //LE("save last [%d] -> [%d] [%ls]", s.start, s.end,s.text.c_str());
        result_sentences.emplace_back(s);
    }

    for (int i = 0; i < result_sentences.size(); i++)
    {
        if(result_sentences.at(i).type == 0)
        {
            continue;
        }
        while(endsWith(result_sentences.at(i).text, parts_end))
        {
            result_sentences.at(i).text.pop_back();
            result_sentences.at(i).end--;
        }
    }

    return result_sentences;
}

/*
std::vector<HitboxSentence> splitSentenceByURLOld(const HitboxSentence& sentence_in)
{
    std::vector<HitboxSentence> result_sentences;

    std::wstring text = sentence_in.text;
    //LE("splitSentenceByURL in [%ls]",text.c_str());
    int start_part_pos = findUrlPart(text, url_parts_start, 0);
    if(start_part_pos == -1 )
    {
        //no url found
        result_sentences.emplace_back(sentence_in);
        return result_sentences;
    }

    int end_part_pos  = findUrlPart(text, url_parts_end, start_part_pos, true);
    if(end_part_pos == -1)
    {
        //no url found 2
        result_sentences.emplace_back(sentence_in);
        return result_sentences;
    }

    std::wstring firstText = sentence_in.text.substr(0, start_part_pos);
    std::wstring urlText   = sentence_in.text.substr(start_part_pos   , text.size() - start_part_pos - (text.size() - end_part_pos));
    std::wstring lastText  = sentence_in.text.substr(end_part_pos     , text.size() - end_part_pos );

    HitboxSentence startSentence = HitboxSentence(sentence_in.start                 , sentence_in.start + start_part_pos, firstText );
    HitboxSentence URLSentence   = HitboxSentence(sentence_in.start + start_part_pos, sentence_in.start + end_part_pos  , urlText );
    HitboxSentence endSentence   = HitboxSentence(sentence_in.start + end_part_pos  , sentence_in.end                   , lastText );

    //LE("splitSentenceByURL startSentence [%d] -> [%d] [%ls]",startSentence.start, startSentence.end, startSentence.text.c_str());
    //LE("splitSentenceByURL URLSentence   [%d] -> [%d] [%ls]",URLSentence.start, URLSentence.end, URLSentence.text.c_str());
    //LE("splitSentenceByURL endSentence   [%d] -> [%d] [%ls]",endSentence.start, endSentence.end, endSentence.text.c_str());

    if(!startSentence.empty())
    {
        result_sentences.emplace_back(startSentence);
    }

    URLSentence.type = 1;
    result_sentences.emplace_back(URLSentence);

    if(findUrlPart(text, url_parts_start, end_part_pos) > 0)
    {
        LE("multiple links in one sentence, going recursive");
        std::vector<HitboxSentence> endParts = splitSentenceByURL(endSentence);
        result_sentences.insert(result_sentences.end(), endParts.begin(), endParts.end());
    }
    else if (!endSentence.empty())
    {
        result_sentences.emplace_back(endSentence);
    }

    return result_sentences;
}
*/

std::vector<HitboxSentence> splitSentenceByUnicodeRanges(HitboxSentence sentence_in)
{
    std::vector<HitboxSentence> result_sentences;
    std::wstring in_text = sentence_in.text;
    //LD("splitSentenceByUnicodeRanges s = [%d] -> [%d] [%ls]", sentence_in.start, sentence_in.end ,in_text.c_str());

    if(in_text.find(L',') == std::wstring::npos)
    {
        result_sentences.emplace_back(sentence_in);
        return result_sentences;
    }

    std::wstringstream input_str(in_text);
    std::wstring part;
    std::vector<std::pair<std::wstring, int>> pairs;

    while(std::getline(input_str, part, L','))
    {
        int range = UnicodeHelper::getUnicodeRange(part);
        pairs.emplace_back(part + L",", range);
    }

    if(pairs.empty())
    {
        //error???
    }
    pairs.back().first.pop_back();     //remove last appended comma


    HitboxSentence s;
    s.start = sentence_in.start;
    s.end = s.start;

    int lastRange = pairs.back().second;

    for( const auto& pair : pairs)
    {
        auto text = pair.first;
        auto range = pair.second;
        //LE("part [%d] [%ls]",range ,text.c_str());

        if(range == -1)
        {
            //форсим накопление для частей с неизвестным диапазоном
            s.end   = s.end + text.length();
            s.text  = s.text  + text;
            //LE("forced collect [%ls]",s.text.c_str());
            continue;
        }

        if(range != lastRange)
        {
            //LE("save [%d] -> [%d] [%ls]", s.start, s.end,s.text.c_str());
            //сохраняем накопленное
            result_sentences.emplace_back(s);

            //запоминаем новый рейндж
            lastRange = range;
            //накапливаем новое (затирая уже сохранённое)
            s.start = s.end;
            s.end = s.start + text.length();
            s.text = text;
            //LE("collect new range [%d] - [%d] [%ls]",s.start, s.end, s.text.c_str());
        }
        else
        {
            //накапливаем
            s.end   = s.end + text.length();
            s.text  = s.text  + text;
            //LE("collect [%ls]",s.text.c_str());
        }
    }
    if(!s.text.empty())
    {
        //LE("save last [%d] -> [%d] [%ls]", s.start, s.end,s.text.c_str());
        result_sentences.emplace_back(s);
    }

    return result_sentences;
}

std::vector<HitboxSentence> splitSentenceGeometrically(HitboxSentence sentence_in , const std::vector<Hitbox_st>& page_hitboxes, bool isDJVU)
{
    //LE("splitSentenceGeometrically [%d -> %d] [%ls] ",sentence_in.start, sentence_in.end, sentence_in.text.c_str());

    std::vector<Hitbox_st> sentence_hbs = sliceHitbox_st_vector(page_hitboxes, sentence_in);

    std::vector<HitboxSentence> result_sentences;

    std::wstring sentence_part_text;
    HitboxSentence sentence_part;
    sentence_part.start = sentence_in.start;

    float prev_width = 0;
    float prev_height = 0;
    float prev_top = 0;
    float prev_bottom = 0;

    for (int i = 0; i < sentence_hbs.size(); i++)
    {
        auto hb = sentence_hbs.at(i);
        float height = fabs(hb.top_   - hb.bottom_);
        float width  = fabs(hb.right_ - hb.left_);
        if(i == 0)
        {
            sentence_part_text.append(hb.text_);
            continue;
        }

        bool startIsCaps = false;
        if(sentence_part.start > 0 && sentence_part.start < page_hitboxes.size())
        {
            int start = sentence_part.start;
            wchar_t ch = L'_';
            while ( start < sentence_in.start + i)
            {
                ch = page_hitboxes.at(start).text_.front();
                start++;
                if( UnicodeHelper::isLatin(ch) || UnicodeHelper::isCyrillic(ch))
                {
                    break;
                }
            }
            startIsCaps = UnicodeHelper::isUpperCase(ch);
            //LE("SENTENCE PART START isCaps [%d] = [%lc]", startIsCaps , ch);
        }
        bool isCaps = UnicodeHelper::isUpperCase(hb.text_);

        auto prev_hb = sentence_hbs.at(i - 1);
        //LE("letter pair [%ls][%ls]",prev_hb.text_.c_str(), hb.text_.c_str());

        float prev_h = fabs(prev_hb.bottom_ - prev_hb.top_ );
        float prev_w = fabs(prev_hb.right_  - prev_hb.left_);
        if(prev_hb.text_ != L" ")
        {
            prev_width = prev_w;
            prev_height = prev_h;
            prev_top = prev_hb.top_;
            prev_bottom = prev_hb.top_;
        }

        float prev_vert_center = prev_top + (prev_height / 2);
        bool onSameLine = (prev_vert_center > hb.top_) && (prev_vert_center < hb.bottom_);

        float height_diff = fabs(prev_height - height);
        float heightThreshold = std::min(height, prev_height) * 0.3f;
        if(isDJVU)
        {
            heightThreshold = std::min(height, prev_height) * 1.0f;
        }

        float horizontal_gap = fabs(hb.left_ - prev_hb.right_);
        float widthThreshold = std::min(width, prev_width) * 4;

        //LE("heghts [%f][%f] height_diff [%f] > [%f]", prev_h, height ,height_diff, heightThreshold);
        //LE("prev_vert_center [%f] [%f][%f] -> [%d] ", prev_vert_center, hb.top_, hb.bottom_, onSameLine);

        if(onSameLine)
        {
            if (prev_hb.right_ < hb.left_ && horizontal_gap > widthThreshold)
            {
                //split
                //LE("Gsplit SAME LINE B %f > %f",horizontal_gap, widthThreshold);
                sentence_part.end =  i + sentence_in.start;
                sentence_part.text = sentence_part_text;
                result_sentences.emplace_back(sentence_part);
                sentence_part.start =  i + sentence_in.start;

                sentence_part_text.clear();
            }
            else if (hb.text_ == L" " && width > widthThreshold)
            {
                // skip WIDE space between sentences
                //LE("Gsplit SAME LINE C %f > %f", width, widthThreshold);
                sentence_part.end = i + sentence_in.start;
                sentence_part.text = sentence_part_text;
                result_sentences.emplace_back(sentence_part);
                sentence_part.start = i + 1 + sentence_in.start;

                sentence_part_text.clear();
                continue;
            }
        }
        else // !onSameLine
        {
            if (height_diff > heightThreshold)
            {
                //split
                //LE("Gsplit SAME LINE A %f > %f", height_diff, heightThreshold);
                sentence_part.end = i + sentence_in.start;
                sentence_part.text = sentence_part_text;
                result_sentences.emplace_back(sentence_part);
                sentence_part.start =  i + sentence_in.start;

                sentence_part_text.clear();
            }
            else if (hb.top_ > prev_top && hb.bottom_ > prev_bottom && hb.top_ > prev_hb.bottom_) // next line or lower
            {
                //LE("Gsplit DIFFERENT LINE D %f - %f", hb.top_, prev_bottom);
                float vertical_gap = hb.top_ - prev_hb.bottom_;
                float heightThreshold2 = ((height + prev_height) / 2 ) * 0.2f;
                float heightThreshold3 = ((height + prev_height) / 2 ) * 2.0f;
                if(isDJVU)
                {
                    heightThreshold2 = ((height + prev_height) / 2 ) * 3.0f;
                    heightThreshold3 = ((height + prev_height) / 2 ) * 6.0f;
                }
                if(startIsCaps)
                {
                    if (isCaps && vertical_gap > heightThreshold2)
                    {
                        //LE("Gsplit DIFFERENT_LINE E1 %f > %f", vertical_gap, heightThreshold2);
                        sentence_part.end = i + sentence_in.start;
                        sentence_part.text = sentence_part_text;
                        result_sentences.emplace_back(sentence_part);
                        sentence_part.start = i + sentence_in.start;

                        sentence_part_text.clear();
                    }
                    else if (vertical_gap > heightThreshold3)
                    {
                        //LE("Gsplit DIFFERENT_LINE F1 %f > %f", vertical_gap, heightThreshold3);
                        sentence_part.end = i + sentence_in.start;
                        sentence_part.text = sentence_part_text;
                        result_sentences.emplace_back(sentence_part);
                        sentence_part.start = i + sentence_in.start;

                        sentence_part_text.clear();
                    }
                }
                else
                {
                    if (vertical_gap > heightThreshold2)
                    {
                        //LE("Gsplit DIFFERENT_LINE E2 %f > %f", vertical_gap, heightThreshold2);
                        sentence_part.end = i + sentence_in.start;
                        sentence_part.text = sentence_part_text;
                        result_sentences.emplace_back(sentence_part);
                        sentence_part.start = i + sentence_in.start;

                        sentence_part_text.clear();
                    }
                    else if (vertical_gap > heightThreshold3)
                    {
                        //LE("Gsplit DIFFERENT_LINE F2 %f > %f", vertical_gap, heightThreshold3);
                        sentence_part.end = i + sentence_in.start;
                        sentence_part.text = sentence_part_text;
                        result_sentences.emplace_back(sentence_part);
                        sentence_part.start = i + sentence_in.start;

                        sentence_part_text.clear();
                    }
                }
            }
            else if ( prev_top > hb.top_  && prev_bottom > hb.bottom_ && prev_top > hb.top_) //prev char is LOWER than current
            {
                //LE("Gsplit DIFFERENT_LINE G %f > %f && %f > %f", prev_top , hb.top_ , prev_bottom , hb.bottom_);
                sentence_part.end =  i + sentence_in.start;
                sentence_part.text = sentence_part_text;
                result_sentences.emplace_back(sentence_part);
                sentence_part.start =  i + sentence_in.start;

                sentence_part_text.clear();
            }
        }

        sentence_part_text.append(hb.text_);
    }
    if(!sentence_part_text.empty())
    {
        sentence_part.end = sentence_hbs.size() + sentence_in.start;
        sentence_part.text = sentence_part_text;
        result_sentences.emplace_back(sentence_part);
    }

    return result_sentences;
}


std::vector<HitboxSentence> splitSentencesURLwise(const std::vector<HitboxSentence>& sentences)
{
    std::vector<HitboxSentence> new_sentences;

    for (const auto& sentence : sentences)
    {
        if(sentence.type != 0)
        {
            new_sentences.emplace_back(sentence);
            continue;
        }
        //LE("splitSentencesURLwise sentence [%ls]",sentence_in.text.c_str());
        std::vector<HitboxSentence> urlSplit = splitSentenceByURL(sentence);
        new_sentences.insert(new_sentences.end(), urlSplit.begin(), urlSplit.end());
    }
    return new_sentences;
}

std::vector<HitboxSentence> splitSentencesUnicodeWise(const std::vector<HitboxSentence>& sentences)
{
    std::vector<HitboxSentence> new_sentences;

    for (const auto& sentence : sentences)
    {
        if(sentence.type != 0)
        {
            new_sentences.emplace_back(sentence);
            continue;
        }
        //LE("splitSentencesUnicodeWise sentence [%ls]",sentence_in.text.c_str());
        std::vector<HitboxSentence> unicodeSplit = splitSentenceByUnicodeRanges(sentence);
        new_sentences.insert(new_sentences.end(), unicodeSplit.begin(), unicodeSplit.end());
    }
    return new_sentences;
}

std::vector<HitboxSentence> splitSentencesGeometryWise(const std::vector<Hitbox_st>& page_hitboxes, const std::vector<HitboxSentence>& sentences, bool isDJVU)
{
    std::vector<HitboxSentence> new_sentences;

    for(const auto& sentence : sentences)
    {
        if(sentence.type != 0)
        {
            new_sentences.emplace_back(sentence);
            continue;
        }
        //LE("splitSentencesGeometryWise sentence [%ls]",sentence_in.text.c_str());
        std::vector<HitboxSentence> split_geo = splitSentenceGeometrically(sentence, page_hitboxes, isDJVU);
        new_sentences.insert(new_sentences.end(), split_geo.begin(), split_geo.end());
    }
    return new_sentences;
}

std::vector<HitboxSentence> performAllSplittings(const std::vector<Hitbox_st>& page_hitboxes , const std::vector<HitboxSentence>& sentences_in, bool isDJVU)
{
    std::vector<HitboxSentence> new_sentences;
    new_sentences = splitSentencesURLwise(sentences_in);
    new_sentences = splitSentencesGeometryWise(page_hitboxes, new_sentences, isDJVU);
    new_sentences = splitSentencesUnicodeWise(new_sentences);
    new_sentences = postProcessSentences(new_sentences);
    return new_sentences;
}

std::vector<std::vector<Hitbox_st>> convertSentencesToVectors(const std::vector<Hitbox_st> &page_hitboxes, std::vector<HitboxSentence> sentences)
{
    std::vector<std::vector<Hitbox_st>> hitbox_sentences;

    for (auto sentence : sentences)
    {
        //LE("new_sentence [%d -> %d] [%ls] ",sentence.start, sentence.end, sentence.text.c_str());
        std::vector<Hitbox_st> hb_sentence = sliceHitbox_st_vector(page_hitboxes, sentence);
        hitbox_sentences.emplace_back(hb_sentence);
    }
    return hitbox_sentences;
}

std::vector<Hitbox_st> sliceHitbox_st_vector(const std::vector<Hitbox_st> &v, unsigned int start, unsigned int n)
{
    //LE("sliceHitbox_st_vector [%d] %d + %d = %d",v.size(),start,n, start + n);
    if(start >= v.size() )
    {
        return std::vector<Hitbox_st>();
    }
    if(start + n > v.size())
    {
        n = v.size() - start;
        //LE("sliceHitbox_st_vector FIXED [%d] %d + %d = %d",v.size(),start,n, start + n);
    }
    if(n <= 0)
    {
        return std::vector<Hitbox_st>();
    }

    auto first = v.cbegin() + start;
    auto last  = v.cbegin() + start + n;

    std::vector<Hitbox_st> vec(first, last);
    return vec;
}

std::vector<Hitbox_st> sliceHitbox_st_vector(const std::vector<Hitbox_st>& v, HitboxSentence s)
{
    return sliceHitbox_st_vector(v, s.start, s.length());
}

std::wstring hitbox_stVectorToString(const std::vector<Hitbox_st> &input)
{
    std::wstring res;
    for(const auto& hb : input)
    {
        res.append(hb.text_);
    }
    if(res.size() != input.size())
    {
        LE("hitbox_stVectorToString SIZE MISMATCH ERROR: [%d] != [%d]",input.size(), res.size());
    }
    return res;
}

int sentenceRightBound(const std::wstring &pageTexts, int start)
{
    //LW("sentenceRightBound [%d]",start);
    //LE("sentenceRightBound [%ls]",pageTexts.c_str());

    if (pageTexts.empty() || start < 0 || start >= pageTexts.size() )
    {
        return 0;
    }

    //auto startText = pageTexts.at(start);
    //LE("sentenceRightBound startText [%d] = [%lc]",start, startText);
    //if (UnicodeHelper::isNewline(startText) || UnicodeHelper::isEndSentence(startText))
    //{
    //    //LE("sentenceRightBound return start");
    //    return start;
    //}

    int bound = -1;
    bool isEndSentenceFound = false;
    wchar_t stopper;
    bool inBrackets = false;
    bool isIndic = false;
    bool isArmenian = false;

    for (int i = start + 1; i < pageTexts.size(); i++)
    {
        auto curr = pageTexts.at(i);
        //LE("sentenceRightBound curr [%d] = [%lc], isEndSentenceFound = [%d], bound = [%d]", i, curr, isEndSentenceFound, bound);
        if(UnicodeHelper::isNewline(curr))
        {
            //LE("sentenceRightBound \\n return %d",i);
            return i;
        }
        else if ( UnicodeHelper::isEndSentence(curr) || UnicodeHelper::isTypographicalBullet(curr))
        {
            isEndSentenceFound = true;
            bound = i + 1;
            stopper = curr;
            if(stopper == L'।' || stopper == L'॥' )
            {
                isIndic = true;
            }
            else if (i - 1 > 0)
            {
                isIndic = UnicodeHelper::isIndicLetter(pageTexts.at(i - 1));
            }

            //LE("sentenceRightBound isEndSentence bound = [%d] [%lc]", bound, stopper);
            continue;
        }
        else if( curr == L':')
        {
            if (i - 1 > 0 && UnicodeHelper::isArmenianLetter(pageTexts.at(i - 1)))
            {
                isEndSentenceFound = true;
                bound = i + 1;
                stopper = curr;
                isArmenian = true;
                //LE("sentenceRightBound armenian");
                continue;
            }
        }
        else if (  UnicodeHelper::isSpace(pageTexts.at(i-1))  &&  UnicodeHelper::isSpace(curr) )
        {
            isEndSentenceFound = true;
            bound = i + 1;
            stopper = curr;
            //LE("sentenceRightBound double space skip");
            continue;
        }
        if (!isEndSentenceFound)
        {
            // Съедаем все символы пока не найдем конец предложения
            //LE("sentenceRightBound continue");
            continue;
        }
        if(isIndic || isArmenian)
        {
            if(UnicodeHelper::isSpace(curr))
            {
                return i+1;
            }
            else
            {
                bound = i;
                continue;
            }
        }
        if ( curr == L',' && ( bound == i || bound == i-1) )
        {
            // скип если запятая сразу после точки, например "и.т.д., "
            isEndSentenceFound = false;
            stopper = 0;
            bound = -1;
            //LE("sentenceRightBound \"e.g.,\" skip");
            continue;
        }
        if (UnicodeHelper::isRightQuotation(curr) && (i - 1 == bound))
        {
            // Сдвигаем границу на кавычку
            bound = i;
            //LE("sentenceRightBound isRightQuotation bound = [%d]",bound);
        }
        if (UnicodeHelper::isDigit(curr))
        {
            //LE("sentenceRightBound curr.isDigit() stopper [%lc]", stopper);
            if(inBrackets)
            {
                //LE("sentenceRightBound isRightQuotation isDigit in brackets - continue");
                //skip any number in brackets;
                continue;
            }
            //LE("sentenceRightBound curr.isDigit()");
            if (stopper == L'.')
            {
                if (i + 1 < pageTexts.size())
                {
                    wchar_t next = pageTexts.at(i + 1);
                    //LE("sentenceRightBound curr.isDigit() nextcheck [%lc]", next);
                    if (UnicodeHelper::isSpace(next))
                    {
                        //LE("sentenceRightBound curr.isDigit() space detected");
                        //сразу после точки идёт цифра, после - пробел
                        return i + 1;
                    }
                    else
                    {
                        //LE("sentenceRightBound curr.isDigit() no space");
                        //сразу после точки идёт цифра, пропускаем (например пункт договора "3.14.1")
                        isEndSentenceFound = false;
                        stopper = 0;
                        bound = -1;
                        continue;
                    }
                }
                else
                {
                    //LE("sentenceRightBound curr.isDigit() NO nextcheck");
                    //сразу после точки идёт цифра, пропускаем (например пункт договора "3.14.1")
                    isEndSentenceFound = false;
                    stopper = 0;
                    bound = -1;
                    continue;
                }
            }
        }
        if (!UnicodeHelper::isLetterOrDigit(curr))
        {
            //LE("sentenceRightBound !curr.isLetterOrDigit()");

            // Пропускаем все символы пунктуации
            if(UnicodeHelper::isLeftBracket(curr))
            {
                inBrackets = true;
            }
            if(inBrackets && UnicodeHelper::isRightBracket(curr))
            {
                inBrackets = false;
                // isEndSentenceFound is always true here
                return i+1;
            }
            bound = i;
            continue;
        }
        if (!UnicodeHelper::isLatin(curr) && !UnicodeHelper::isCyrillic(curr))
        {
            //LE("sentenceRightBound !curr.isLatin() && !curr.isCyrillic() = [%d]",bound);
            return bound;
        }
        if(UnicodeHelper::isUpperCase(curr) && stopper == L'.')
        {
            if(bound - 2 >= 0  && UnicodeHelper::isUpperCase(pageTexts.at(bound - 2)))
            {
                //LE("sentenceRightBound Name short skip");
                //Перед и после точки - верхний регистр (и любое количество знаков препинания (напр. пробелов) после точки) - пропускаем (например "М. Горькому")
                isEndSentenceFound = false;
                stopper = 0;
                bound = -1;
                continue;
            }
            if(bound - 3 >= 0  && !UnicodeHelper::isUpperCase(pageTexts.at(bound - 2)) && UnicodeHelper::isUpperCase(pageTexts.at(bound - 3)))
            {
                //LE("sentenceRightBound Name short skip 2");
                //Перед и после точки - верхний регистр (и любое количество знаков препинания (напр. пробелов) после точки) - пропускаем (например "Дж. Кэнфилд")
                isEndSentenceFound = false;
                stopper = 0;
                bound = -1;
                continue;
            }
        }

        if (UnicodeHelper::isUpperCase(curr))
        {
            if (i + 1 < pageTexts.size())
            {
                auto next = pageTexts.at(i + 1);
                if (next == L'.')
                { // Однобуквенное сокращение
                    continue;
                }
            }
            //LE("sentenceRightBound new sentence = [%d]",bound);
            // Начало нового предложения
            return bound;
        }
        /*
        else if( UnicodeHelper::isLetter(curr))
        {
            if(bound - 2 >= 0 )
            {
                if(UnicodeHelper::isLetter(pageTexts.at(bound - 2)))
                {
                    //валидный символ перед точкой в сокращении
                    //LE("sentenceRightBound short skip");
                    // Поймали сокращение идем дальше
                    isEndSentenceFound = false;
                    bound = -1;
                }
                else
                {
                    //не валидный символ перед точкой в сокращении
                    //LE("sentenceRightBound invalid shortening");
                    //режем
                    return bound;
                }
            }
        }
         */
        else if( UnicodeHelper::isLetter(curr))
        {
            // Поймали сокращение идем дальше
            isEndSentenceFound = false;
            bound = -1;
        }

        if(UnicodeHelper::isSpace(curr))
        {
            //LE("sentenceRightBound space after endsentence");
            return i;
        }
    }
    if (bound == -1)
    {
        //LE("sentenceRightBound last");
        // Последнее предложение на странице
        return pageTexts.size();
    }
    return bound;
}

std::vector<HitboxSentence> generatePageSentences(std::wstring input)
{
    //LE("generatePageSentences [%ls]",input.size(),input.c_str());
    std::vector<int> rightBounds;
    int pageEnd = input.size()-1;

    int right_bound = 0;
    do
    {
        right_bound = sentenceRightBound(input, right_bound);
        rightBounds.emplace_back(right_bound);
        right_bound += 1;
    }  while (right_bound < pageEnd);

    std::vector<std::pair<int,int>> bounds;

    if(rightBounds.empty())
    {
        bounds.emplace_back(0,pageEnd);
        //LE("rightBounds empty 0 -> %d",pageEnd);
    }
    else
    {
        bounds.emplace_back(0,rightBounds.at(0));

        for( int i = 1; i < rightBounds.size(); i++)
        {
            int first = rightBounds.at( i - 1 );
            int last  = rightBounds.at( i );
            while( UnicodeHelper::isEndSentence( input.at(first) ) ||
                   UnicodeHelper::isSpace      ( input.at(first) ) ||
                   UnicodeHelper::isNewline    ( input.at(first) ) )
            {
                if( first + 1 >= input.size())
                {
                    break;
                }
                first++;
            }
            bounds.emplace_back(first,last);
        }
    }


    std::vector<HitboxSentence> result;

    for ( auto  pair : bounds)
    {
        //LE("sentence [%d -> %d]",pair.first, pair.second);
        if(pair.second - pair.first <= 0)
        {
            //LE("sentence [%d -> %d] HAS WRONG BOUNDS",pair.first, pair.second);
            continue;
        }
        auto text = input.substr(pair.first, pair.second - pair.first);

        HitboxSentence sentence = HitboxSentence(pair.first, pair.second, text);

        result.emplace_back(sentence);
    }

    return result;
}

HitboxSentence getFirstSentence(const std::wstring &input, const std::vector<Hitbox_st>& page_hitboxes, bool isDJVU)
{
    //if(input.empty()) { return std::wstring(); }
    int right_bound = sentenceRightBound(input, 0);
    auto text = input.substr(0,right_bound);

    auto sentence_in = HitboxSentence(0,right_bound,text);

    //LE("sentence [%ls]",sentence.text.c_str())
    std::vector<HitboxSentence> in = {sentence_in};
    std::vector<HitboxSentence> split = performAllSplittings(page_hitboxes, in, isDJVU);

    if(split.empty())
    {
        return HitboxSentence();
    }
    return split.front();
}


bool checkFirst(const std::wstring &firstSentenceOfPage)
{
    //returns whether we need to glue next page first sentence to last sentence on current page
    //false == no
    //true == yes
    if(firstSentenceOfPage.empty())// || firstSentenceOfPage.length() == 1)
    {
        return false;
    }
    for (unsigned int i = 0; i < firstSentenceOfPage.length() ; i++)
    {
        auto ch = firstSentenceOfPage.at(i);
        if(UnicodeHelper::isLeftBracket(ch) ||
           UnicodeHelper::isRightBracket(ch) ||
           UnicodeHelper::isLeftQuotation(ch) ||
           UnicodeHelper::isRightQuotation(ch) ||
           UnicodeHelper::isDigit(ch))
        {
            //skip
            continue;
        }
        if(UnicodeHelper::isEndSentence(ch) ||
           UnicodeHelper::isNewline(ch) )
        {
            //no glue
            return false;
        }
        else if( ch == L':' && i+1 < firstSentenceOfPage.length() )
        {
            wchar_t next = firstSentenceOfPage.at(i+1);
            if(UnicodeHelper::isArmenianLetter(next))
            {
                return false;
            }
        }
        else if ( UnicodeHelper::isLatin(ch) || UnicodeHelper::isCyrillic(ch))
        {
            return !UnicodeHelper::isUpperCase(ch);
        }
        else
        {
            //CJK, RTL, INDIC, OTHER
            //glue
            return true;
        }
    }

    return false;
}

bool checkLast(const std::wstring &lastSentenceOfPage)
{
    //returns whether we need to glue last page last sentence to first sentence on current page
    //false == no
    //true == yes
    if(lastSentenceOfPage.empty())// || lastSentenceOfPage.length() == 1)
    {
        return false;
    }

    for (unsigned int i = lastSentenceOfPage.length()-1; i > 0 ; i--)
    {
        auto ch = lastSentenceOfPage.at(i);
        if(UnicodeHelper::isLeftBracket(ch) ||
           UnicodeHelper::isRightBracket(ch) ||
           UnicodeHelper::isLeftQuotation(ch) ||
           UnicodeHelper::isRightQuotation(ch) ||
           UnicodeHelper::isDigit(ch))
        {
            //skip
            continue;
        }
        if(UnicodeHelper::isEndSentence(ch) ||
           UnicodeHelper::isNewline(ch) )
        {
            //no glue
            return false;
        }
        else if( ch == L':' && i-1 > 0 )
        {
            wchar_t prev = lastSentenceOfPage.at(i-1);
            if(UnicodeHelper::isArmenianLetter(prev))
            {
                return false;
            }
        }
        else if ( UnicodeHelper::isLatin(ch) || UnicodeHelper::isCyrillic(ch))
        {
            return !UnicodeHelper::isUpperCase(ch);
        }
        else
        {
            //CJK, RTL, INDIC, OTHER
            //glue
            return true;
        }
    }
    return false;
}

std::wstring trimDoubleChar(std::wstring input, wchar_t query)
{
    //LE("trimDoubleChar        [%lc] [%ls]",query,input.c_str());

    std::wstring result;

    bool isChar = false;
    for(wchar_t c : input)
    {
        if(c == query)
        {
            if(isChar)
            {
                continue;
            }
            else
            {
                isChar = true;
                result += c;
                continue;
            }
        }

        isChar = false;
        result += c;
    }
    //LE("trimDoubleChar result [%lc] [%ls]",query,result.c_str());

    return result;
}

void strReplace(std::wstring* input, const std::wstring& query, const std::wstring& replacement )
{
    int index = 0;
    int rsize = replacement.size();
    while (true)
    {
        /* Locate the substring to replace. */
        index = input->find(query, index);
        if (index == std::string::npos)
        {
            break;
        }
        input->replace(index, rsize, replacement);
        index += rsize;
    }
}

std::wstring filterStringForTTS(std::wstring input)
{
    std::wstring result;
    result = trimDoubleChar(std::move(input), L'.');
    result = trimDoubleChar(result, 0x2026); //c = '…'
    result = trimDoubleChar(result, L' ');
    result = trimDoubleChar(result, L'_');
    result = trimDoubleChar(result, L'\n');
    result = trimDoubleChar(result, L'*');
    strReplace( &result, L"Ĳ", L"Ij");
    strReplace( &result, L"ĳ", L"ij");
    strReplace( &result, L"ﬀ", L"ff");
    strReplace( &result, L"ﬁ", L"fi");
    strReplace( &result, L"ﬂ", L"fl");
    strReplace( &result, L"ﬃ", L"ffi");
    strReplace( &result, L"ﬄ", L"ffl");
    strReplace( &result, L"ﬅ", L"ft");
    strReplace( &result, L"ﬆ", L"st");
    //std::replace( result.begin(), result.end(), '\n', '@'); //debug
    return result;
}

std::string getCoords(const Hitbox_st &hb)
{
    float x = hb.left_;
    float y = hb.top_;
    std::string xstr = std::to_string(x).substr(0, 6);
    std::string ystr = std::to_string(y).substr(0, 6);
    std::string coords = "@" + xstr + ":" + ystr;
    return coords;
}

int getFirstValidHitboxIndex(std::vector<Hitbox_st> sentence)
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

int getLastValidHitboxIndex(std::vector<Hitbox_st> sentence)
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

std::vector<Hitbox_st> filterTextViaColontitles(int page, const std::vector<Hitbox_st> &hitboxes, const std::map<int, std::pair<float, float>>& map)
{
    std::vector<Hitbox_st> result;

    auto it = map.find(page);
    if( it == map.end())
    {
        return hitboxes;
    }
    float top    = it->second.first;
    float bottom = it->second.second;

    if(top == 0.0f && bottom == 1.0f)
    {
        return hitboxes;
    }

    for( auto hb: hitboxes)
    {
        if(hb.isInVerticalRange(top,bottom))
        {
            result.emplace_back(hb);
        }
    }
    LE("filterTextViaColontitles removed [%d] hitboxes",hitboxes.size() - result.size());
    return result;
}


float calculateUpperColontitle(const std::vector<Hitbox_st> &hitboxes)
{
    std::wstring top_line_text;
    for (int i = 1; i < hitboxes.size(); i++)
    {
        auto hb = hitboxes.at(i);
        float height = fabs(hb.top_   - hb.bottom_);

        auto prev_hb = hitboxes.at(i - 1);
        //LE("calculateUpperColontitle letter pair [%ls][%ls]",prev_hb.text_.c_str(), hb.text_.c_str());
        float prev_h = fabs(prev_hb.bottom_ - prev_hb.top_ );
        float prev_vert_center = prev_hb.top_ + (prev_h / 2);
        bool onSameLine = (prev_vert_center > hb.top_) && (prev_vert_center < hb.bottom_);
        float vertical_gap = hb.top_ - prev_hb.bottom_;
        float vertical_threshold = std::min(height,prev_h) * 0.6;

        if(onSameLine)
        {
            //LE("calculateUpperColontitle OnSameLine");
            top_line_text.append(prev_hb.text_);
            continue;
        }
        else if(vertical_gap >= vertical_threshold) // !onSameLine
        {
            //LE("calculateUpperColontitle 1 vertical_gap [%f], vertical_threshold [%f]",vertical_gap,vertical_threshold);
            while (UnicodeHelper::isTypographicalBullet(top_line_text.front()) ||
                   UnicodeHelper::isSpace(top_line_text.front()) ||
                   UnicodeHelper::isLeftBracket(top_line_text.front()) ||
                   UnicodeHelper::isLeftQuotation(top_line_text.front()) ||
                   UnicodeHelper::isEndSentence(top_line_text.front()))
            {
                top_line_text = top_line_text.substr(1);
            }

            while ( UnicodeHelper::isTypographicalBullet(top_line_text.back()) ||
                    UnicodeHelper::isSpace(top_line_text.back()) ||
                    UnicodeHelper::isRightBracket(top_line_text.back()) ||
                    UnicodeHelper::isRightQuotation(top_line_text.back()) ||
                    UnicodeHelper::isEndSentence(top_line_text.back()))
            {
                top_line_text.pop_back();
            }

            //LE("calculateUpperColontitle top_line_text = [%ls]",top_line_text.c_str());

            if(UnicodeHelper::isDigit(top_line_text.front()) || UnicodeHelper::isDigit(top_line_text.back()))
            {
                //LE("calculateUpperColontitle 1 return = [%f]",prev_hb.bottom_);
                return prev_hb.bottom_;
            }
            else
            {
                //LE("calculateUpperColontitle !digit return 0");
                return 0;
            }
        }
        else if (UnicodeHelper::isDigitsOnly(top_line_text) && (vertical_gap >= std::min(height,prev_h) * 0.1))
        {
            //LE("calculateUpperColontitle 2 return = [%f]",prev_hb.bottom_);
            return prev_hb.bottom_;
        }
        else
        {
            //LE("calculateUpperColontitle 3 vertical_gap [%f], vertical_threshold [%f]",vertical_gap,vertical_threshold);
            return 0;
        }
    }

    return 0;
}

float calculateLowerColontitle(const std::vector<Hitbox_st> &hitboxes_in)
{
    std::vector<Hitbox_st> hitboxes = hitboxes_in;
    std::reverse(hitboxes.begin(), hitboxes.end());
    std::wstring bottom_line_text;

    for (int i = 1; i < hitboxes.size(); i++)
    {
        auto hb = hitboxes.at(i);
        float height = fabs(hb.top_   - hb.bottom_);
        auto prev_hb = hitboxes.at(i - 1);
        //LE("calculateLowerColontitle letter pair [%ls][%ls]",prev_hb.text_.c_str(), hb.text_.c_str());
        float prev_h = fabs(prev_hb.bottom_ - prev_hb.top_ );
        float prev_vert_center = prev_hb.top_ + (prev_h / 2);
        bool onSameLine = (prev_vert_center > hb.top_) && (prev_vert_center < hb.bottom_);
        float vertical_gap = prev_hb.top_ - hb.bottom_;
        float vertical_threshold = std::min(height,prev_h) * 0.8;

        if(onSameLine)
        {
            //LE("calculateLowerColontitle OnSameLine");

            bottom_line_text.append(prev_hb.text_);
            continue;
        }
        else if(vertical_gap >= vertical_threshold) // !onSameLine
        {
            //LE("calculateLowerColontitle 1 vertical_gap [%f], vertical_threshold [%f]",vertical_gap,vertical_threshold);
            while (UnicodeHelper::isTypographicalBullet(bottom_line_text.front()) ||
                   UnicodeHelper::isSpace(bottom_line_text.front()) ||
                   UnicodeHelper::isLeftBracket(bottom_line_text.front()) ||
                   UnicodeHelper::isLeftQuotation(bottom_line_text.front()) ||
                   UnicodeHelper::isEndSentence(bottom_line_text.front()))
            {
                bottom_line_text = bottom_line_text.substr(1);
            }

            while ( UnicodeHelper::isTypographicalBullet(bottom_line_text.back()) ||
                    UnicodeHelper::isSpace(bottom_line_text.back()) ||
                    UnicodeHelper::isRightBracket(bottom_line_text.back()) ||
                    UnicodeHelper::isRightQuotation(bottom_line_text.back()) ||
                    UnicodeHelper::isEndSentence(bottom_line_text.back()))
            {
                bottom_line_text.pop_back();
            }
            //LE("calculateLowerColontitle bottom_line_text = [%ls]",bottom_line_text.c_str());

            if(UnicodeHelper::isDigit(bottom_line_text.front()) || UnicodeHelper::isDigit(bottom_line_text.back()))
            {
                //LE("calculateLowerColontitle 1 return = [%f]",prev_hb.top_);
                return prev_hb.top_;
            }
            else
            {
                //LE("calculateLowerColontitle !digit return 1");
                return 1;
            }
        }
        else if (UnicodeHelper::isDigitsOnly(bottom_line_text) && (vertical_gap >= std::min(height,prev_h) * 0.1))
        {
            //LE("calculateLowerColontitle 2 return = [%f]",prev_hb.top_);
            return prev_hb.top_;
        }
        else
        {
            //LE("calculateLowerColontitle 3 vertical_gap [%f], vertical_threshold [%f]",vertical_gap,vertical_threshold);
            return 1;
        }
    }

    return 1;
}

bool hitboxComparator(Hitbox_st &a, Hitbox_st &b)
{
    float a_x =  a.left_ + ( fabs(a.right_  - a.left_) / 2 );
    float b_x =  b.left_ + ( fabs(b.right_  - b.left_) / 2 );

    float a_y =  a.top_  + ( fabs(a.bottom_ - a.top_ ) / 2 );
    float b_y =  b.top_  + ( fabs(b.bottom_ - b.top_ ) / 2 );

    if((a_y > b.top_) && (a_y < b.bottom_)) //onSameLine
    {
        return ( a_x < b_x );
    }

    return a_y < b_y;
}
