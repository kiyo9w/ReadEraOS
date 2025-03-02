//
// Created by Tarasus on 04.04.2023.
//

#include <openreadera.h>
#include "include/StUnicodeHelper.h"


template <typename iter, typename T>
bool contains(iter begin, iter end, T value) {
    for (; begin != end; ++begin) {
        if (static_cast<T>(*begin) == value) {
            return true;
        }
    }
    return false;
}


const std::vector<wchar_t> UnicodeHelper::sLeftQuotationSet         = {L'«', L'‹', L'‟', L'„', L'‚', L'“', L'‘', L'„', L'‚', L'»', L'›', L'»', L'›', L'”', L'’', L'“', L'‘', L'‛', L'「', L'『', L'"'};
const std::vector<wchar_t> UnicodeHelper::sRightQuotationSet        = {L'»', L'›', L'”', L'“', L'‘', L'”', L'’', L'”', L'’', L'«', L'‹', L'»', L'›', L'”', L'’', L'„', L'‚', L'’', L'」', L'』', L'"'};
const std::vector<wchar_t> UnicodeHelper::sTypographicalBulletsSet  = {L'•', L'‣', L'⁃', L'⁌', L'⁍', L'∙', L'○', L'●', L'◘', L'◦', L'☙', L'❥', L'❧', L'⦾', L'⦿', L'►', L'❖'};
const std::vector<wchar_t> UnicodeHelper::sLeftBracketsSet          = {0x0028,0x207D,0x208D,0x239B,0x239C,0x239D,0x2768,0x276A,0xFD3E,0xFE35,0xFE59,0xFF08,0x005B,0x007B,0x2045,0x2329,0x23A1,0x23A2,
                                                                       0x23A3,0x23A7,0x23A8,0x23A9,0x23AA,0x2770,0x2772,0x2774,0x27E6,0x27E8,0x27EA,0x27EC,0x2983,0x2987,0x2989,0x298B,0x298D,0x298F,
                                                                       0x2991,0x2993,0x2995,0x2997,0x29FC,0x2E02,0x2E04,0x2E09,0x2E0C,0x2E1C,0x2E22,0x2E24,0x2E26,0x3008,0x300A,0x300C,0x300E,0x3010,
                                                                       0x3014,0x3016,0x3018,0x301A};
const std::vector<wchar_t> UnicodeHelper::sRightBracketsSet         = {0x0029,0x207E,0x208E,0x239E,0x239F,0x23A0,0x2769,0x276B,0xFD3F,0xFE36,0xFE5A,0xFF09,0x005D,0x007D,0x2046,0x232A,0x23A4,0x23A5,
                                                                       0x23A6,0x23AB,0x23AC,0x23AD,0x276D,0x2771,0x2773,0x2775,0x27E7,0x27E9,0x27EB,0x27ED,0x2984,0x2988,0x298A,0x298C,0x298E,0x2990,
                                                                       0x2992,0x2994,0x2996,0x2998,0x29FD,0x2E03,0x2E05,0x2E0A,0x2E0D,0x2E1D,0x2E23,0x2E25,0x2E27,0x3009,0x300B,0x300D,0x300F,0x3011,
                                                                       0x3015,0x3017,0x3019,0x301B};

bool UnicodeHelper::isLeftBracket(wchar_t c)
{
    return contains(sLeftBracketsSet.begin(), sLeftBracketsSet.end(), c );
}

bool UnicodeHelper::isRightBracket(wchar_t c)
{
    return contains(sRightBracketsSet.begin(), sRightBracketsSet.end(), c );
}

bool UnicodeHelper::isLeftQuotation(wchar_t c)
{
    return contains(sLeftQuotationSet.begin(), sLeftQuotationSet.end(), c );
}

bool UnicodeHelper::isRightQuotation(wchar_t c)
{
    return contains(sRightQuotationSet.begin(), sRightQuotationSet.end(), c );
}

bool UnicodeHelper::isTypographicalBullet(wchar_t c)
{
    return contains(sTypographicalBulletsSet.begin(), sTypographicalBulletsSet.end(), c );
}

bool UnicodeHelper::isCJKPunctuation(wchar_t c)
{
    return (c >= UNICODE_CJK_PUNCTUATION_BEGIN && c <= UNICODE_CJK_PUNCTUATION_END)
           || (c >= UNICODE_CJK_PUNCTUATION_HALF_AND_FULL_WIDTH_BEGIN &&
               c <= UNICODE_CJK_PUNCTUATION_HALF_AND_FULL_WIDTH_END)
           || (c >= UNICODE_GENERAL_PUNCTUATION_BEGIN && c <= UNICODE_GENERAL_PUNCTUATION_END);
}

bool UnicodeHelper::isChineseJapaneseIdeograph(wchar_t c)
{
    if (c < UNICODE_HANGUL_JAMO_BEGIN)
    {
        return false;
    }
    return ((c >= UNICODE_CJK_IDEOGRAPHS_BEGIN && c <= UNICODE_CJK_IDEOGRAPHS_END)
            && (c <= UNICODE_CJK_PUNCTUATION_HALF_AND_FULL_WIDTH_BEGIN ||
                c >= UNICODE_CJK_PUNCTUATION_HALF_AND_FULL_WIDTH_END))
           || (c >= UNICODE_JP_HIRAGANA_BEGIN && c <= UNICODE_JP_HIRAGANA_END)
           || (c >= UNICODE_JP_KATAKANA_BEGIN && c <= UNICODE_JP_KATAKANA_END);
}

bool UnicodeHelper::isChineseJapanese(wchar_t c)
{
    return isChineseJapaneseIdeograph(c) || isCJKPunctuation(c);
}

bool UnicodeHelper::isLatin(wchar_t c)
{
    return ((c >= 0x0041 && c <= 0x005A) ||
            (c >= 0x0061 && c <= 0x007A) ||
            (c >= 0x00C0 && c <= 0x02AF) ||
            (c >= 0x1E00 && c <= 0x1EFF) ||
            (c >= 0x2C60 && c <= 0x2C7F) ||
            (c >= 0xA720 && c <= 0xA7FF) ||
            (c >= 0xAB30 && c <= 0xAB6F));
    //(ch >= 0x10780 && ch <= 0x107BF ) // widechar
    //(ch >= 0x1DF00 && ch <= 0x1DFFF ) // widechar
}

bool isOtherLatinUppercase(wchar_t c)
{
    if (c < 0x0100 || c > 0x024E)
    {
        return false;
    }

    return (c == 0x0100 || c == 0x0102 || c == 0x0104 || c == 0x0106 || c == 0x0108 ||
            c == 0x010A || c == 0x010C || c == 0x010E || c == 0x0110 || c == 0x0112 ||
            c == 0x0114 || c == 0x0116 || c == 0x0118 || c == 0x011A || c == 0x011C ||
            c == 0x011E || c == 0x0120 || c == 0x0122 || c == 0x0124 || c == 0x0126 ||
            c == 0x0128 || c == 0x012A || c == 0x012C || c == 0x012E || c == 0x0130 ||
            c == 0x0132 || c == 0x0134 || c == 0x0136 || c == 0x0139 || c == 0x013B ||
            c == 0x013D || c == 0x013F || c == 0x0141 || c == 0x0143 || c == 0x0145 ||
            c == 0x0147 || c == 0x014A || c == 0x014C || c == 0x014E || c == 0x0150 ||
            c == 0x0152 || c == 0x0154 || c == 0x0156 || c == 0x0158 || c == 0x015A ||
            c == 0x015C || c == 0x015E || c == 0x0160 || c == 0x0162 || c == 0x0164 ||
            c == 0x0166 || c == 0x0168 || c == 0x016A || c == 0x016C || c == 0x016E ||
            c == 0x0170 || c == 0x0172 || c == 0x0174 || c == 0x0176 || c == 0x0178 ||
            c == 0x0179 || c == 0x017B || c == 0x017D || c == 0x0181 || c == 0x0182 ||
            c == 0x0184 || c == 0x0186 || c == 0x0187 || c == 0x0189 || c == 0x018A ||
            c == 0x018B || c == 0x018E || c == 0x018F || c == 0x0190 || c == 0x0191 ||
            c == 0x0193 || c == 0x0194 || c == 0x0196 || c == 0x0197 || c == 0x0198 ||
            c == 0x019C || c == 0x019D || c == 0x019F || c == 0x01A0 || c == 0x01A2 ||
            c == 0x01A4 || c == 0x01A6 || c == 0x01A7 || c == 0x01A9 || c == 0x01AC ||
            c == 0x01AE || c == 0x01AF || c == 0x01B1 || c == 0x01B2 || c == 0x01B3 ||
            c == 0x01B5 || c == 0x01B7 || c == 0x01B8 || c == 0x01BC || c == 0x01C4 ||
            c == 0x01C7 || c == 0x01CA || c == 0x01CD || c == 0x01CF || c == 0x01D1 ||
            c == 0x01D3 || c == 0x01D5 || c == 0x01D7 || c == 0x01D9 || c == 0x01DB ||
            c == 0x01DE || c == 0x01E0 || c == 0x01E2 || c == 0x01E4 || c == 0x01E6 ||
            c == 0x01E8 || c == 0x01EA || c == 0x01EC || c == 0x01EE || c == 0x01F1 ||
            c == 0x01F4 || c == 0x01F6 || c == 0x01F7 || c == 0x01F8 || c == 0x01FA ||
            c == 0x01FC || c == 0x01FE || c == 0x0200 || c == 0x0202 || c == 0x0204 ||
            c == 0x0206 || c == 0x0208 || c == 0x020A || c == 0x020C || c == 0x020E ||
            c == 0x0210 || c == 0x0212 || c == 0x0214 || c == 0x0216 || c == 0x0218 ||
            c == 0x021A || c == 0x021C || c == 0x021E || c == 0x0220 || c == 0x0222 ||
            c == 0x0224 || c == 0x0226 || c == 0x0228 || c == 0x022A || c == 0x022C ||
            c == 0x022E || c == 0x0230 || c == 0x0232 || c == 0x023A || c == 0x023B ||
            c == 0x023D || c == 0x023E || c == 0x0241 || c == 0x0243 || c == 0x0244 ||
            c == 0x0245 || c == 0x0246 || c == 0x0248 || c == 0x024A || c == 0x024C ||
            c == 0x024E);
}

bool UnicodeHelper::isLatinUppercase(wchar_t c)
{
    if(!isLatin(c))
    {
        return false;
    }

    return ((c >= 0x0041 && c <= 0x005A) ||
            (c >= 0x00C0 && c <= 0x00D6) ||
            (c >= 0x00D8 && c <= 0x00DE) ||
            isOtherLatinUppercase(c) );
}

bool isOtherCyrillicUppercase(wchar_t c)
{
    if (c < 0x0460 || c > 0x052E)
    {
        return false;
    }

    return (c == 0x0460 || c == 0x0462 || c == 0x0464 || c == 0x0466 || c == 0x0468 ||
            c == 0x046A || c == 0x046C || c == 0x046E || c == 0x0470 || c == 0x0472 ||
            c == 0x0474 || c == 0x0476 || c == 0x0478 || c == 0x047A || c == 0x047C ||
            c == 0x047E || c == 0x0480 || c == 0x048A || c == 0x048C || c == 0x048E ||
            c == 0x0490 || c == 0x0492 || c == 0x0494 || c == 0x0496 || c == 0x0498 ||
            c == 0x049A || c == 0x049C || c == 0x049E || c == 0x04A0 || c == 0x04A2 ||
            c == 0x04A4 || c == 0x04A6 || c == 0x04A8 || c == 0x04AA || c == 0x04AC ||
            c == 0x04AE || c == 0x04B0 || c == 0x04B2 || c == 0x04B4 || c == 0x04B6 ||
            c == 0x04B8 || c == 0x04BA || c == 0x04BC || c == 0x04BE || c == 0x04C0 ||
            c == 0x04C1 || c == 0x04C3 || c == 0x04C5 || c == 0x04C7 || c == 0x04C9 ||
            c == 0x04CB || c == 0x04CD || c == 0x04D0 || c == 0x04D2 || c == 0x04D4 ||
            c == 0x04D6 || c == 0x04D8 || c == 0x04DA || c == 0x04DC || c == 0x04DE ||
            c == 0x04E0 || c == 0x04E2 || c == 0x04E4 || c == 0x04E6 || c == 0x04E8 ||
            c == 0x04EA || c == 0x04EC || c == 0x04EE || c == 0x04F0 || c == 0x04F2 ||
            c == 0x04F4 || c == 0x04F6 || c == 0x04F8 || c == 0x04FA || c == 0x04FC ||
            c == 0x04FE || c == 0x0500 || c == 0x0502 || c == 0x0504 || c == 0x0506 ||
            c == 0x0508 || c == 0x050A || c == 0x050C || c == 0x050E || c == 0x0510 ||
            c == 0x0512 || c == 0x0514 || c == 0x0516 || c == 0x0518 || c == 0x051A ||
            c == 0x051C || c == 0x051E || c == 0x0520 || c == 0x0522 || c == 0x0524 ||
            c == 0x0526 || c == 0x0528 || c == 0x052A || c == 0x052C || c == 0x052E);
}


bool UnicodeHelper::isCyrillicUppercase(wchar_t c)
{
    if(!isCyrillic(c))
    {
        return false;
    }
    return ((c >= 0x0400 && c <= 0x042F) || isOtherCyrillicUppercase(c) );
}

bool UnicodeHelper::isCyrillic(wchar_t c)
{
    return ((c >= 0x0400 && c <= 0x052F) ||
            (c >= 0x1C80 && c <= 0x1C8F) ||
            (c >= 0x2DE0 && c <= 0x2DFF) ||
            (c >= 0xA640 && c <= 0xA69F));
    //(c >= 0x1E030 && c <= 0x1E08F )); //widechar
}

bool UnicodeHelper::isPunct(wchar_t c)
{
    return ((c>=0x0000) && (c<=0x001F)) ||
           ((c>=0x0021) && (c<=0x002F)) ||
           ((c>=0x003A) && (c<=0x0040)) ||
           ((c>=0x005B) && (c<=0x0060)) ||
           ((c>=0x007B) && (c<=0x007E)) ||
           ((c>=0x0080) && (c<=0x00BF)) ||
           ((c>=0x02B9) && (c<=0x0362)) ||
           ((c>=0x0590) && (c<=0x05CF)) ||
           ((c>=0x2000) && (c<=0x206F)) ||
           (c == 0x00A6)||
           (c == 0x060C)||
           (c == 0x060D)||
           (c == 0x060E)||
           (c == 0x060F)||
           (c == 0x061F)||
           (c == 0x066D)||
           (c == 0x06DD)||
           (c == 0x06DE)||
           (c == 0x06E9)||
           (c == 0xFD3E)||
           (c == 0x0621)|| // hamza
           (c == 0xFD3F);
}

bool UnicodeHelper::isSpace(wchar_t ch)
{
    return( ch == 0x0009 ||
            ch == 0x0020 ||
            ch == 0x00A0 ||
            ch == 0x180E ||
            ch == 0x2000 ||
            ch == 0x2001 ||
            ch == 0x2002 ||
            ch == 0x2003 ||
            ch == 0x2004 ||
            ch == 0x2005 ||
            ch == 0x2006 ||
            ch == 0x2007 ||
            ch == 0x2008 ||
            ch == 0x2009 ||
            ch == 0x200A ||
            ch == 0x200B ||
            ch == 0x202F ||
            ch == 0x205F ||
            ch == 0x3000 ||
            ch == 0xFEFF );
}

bool UnicodeHelper::isRTL(wchar_t c)
{
    if(c < 0x0590)
    {
        return false;
    }
    return ( //(c==0x00A6) ||  // UNICODE "BROKEN BAR"
                   (c>=0x0590)&&(c<=0x05FF)) ||
           (c == 0x05BE) || (c == 0x05C0) || (c == 0x05C3) || (c == 0x05C6) ||
           ((c>=0x05D0)&&(c<=0x05F4)) ||
           (c==0x0608) || (c==0x060B) ||
           (c==0x060D) ||
           ((c>=0x0600)&&(c<=0x06FF)) ||
           ((c>=0x06FF)&&(c<=0x0710)) ||
           ((c>=0x0712)&&(c<=0x072F)) ||
           ((c>=0x074D)&&(c<=0x07A5)) ||
           ((c>=0x07B1)&&(c<=0x07EA)) ||
           ((c>=0x07F4)&&(c<=0x07F5)) ||
           ((c>=0x07FA)&&(c<=0x0815)) ||
           (c==0x081A) || (c==0x0824) ||
           (c==0x0828) ||
           ((c>=0x0830)&&(c<=0x0858)) ||
           ((c>=0x085E)&&(c<=0x08AC)) ||
           (c==0x200F) || (c==0xFB1D) ||
           ((c>=0xFB1F)&&(c<=0xFB28)) ||
           ((c>=0xFB2A)&&(c<=0xFD3D)) ||
           ((c>=0xFD50)&&(c<=0xFDFC)) ||
           ((c>=0xFE70)&&(c<=0xFEFC)) ||
           ((c>=0x10800)&&(c<=0x1091B)) ||
           ((c>=0x10920)&&(c<=0x10A00)) ||
           ((c>=0x10A10)&&(c<=0x10A33)) ||
           ((c>=0x10A40)&&(c<=0x10B35)) ||
           ((c>=0x10B40)&&(c<=0x10C48)) ||
           ((c>=0x1EE00)&&(c<=0x1EEBB));
}

bool UnicodeHelper::isCJKIdeograph(wchar_t c)
{
    if(c < UNICODE_HANGUL_JAMO_BEGIN)
    {
        return false;
    }

    return (
                   (c >= UNICODE_CJK_IDEOGRAPHS_BEGIN && c <= UNICODE_CJK_IDEOGRAPHS_END)  &&
                   (c <= UNICODE_CJK_PUNCTUATION_HALF_AND_FULL_WIDTH_BEGIN || c >= UNICODE_CJK_PUNCTUATION_HALF_AND_FULL_WIDTH_END)
           ) ||
           //japanese
           (c >= UNICODE_JP_HIRAGANA_BEGIN      && c <= UNICODE_JP_HIRAGANA_END      ) ||
           (c >= UNICODE_JP_KATAKANA_BEGIN      && c <= UNICODE_JP_KATAKANA_END      ) ||
           //korean
           (c >= UNICODE_HANGUL_JAMO_BEGIN      && c <= UNICODE_HANGUL_JAMO_END      ) ||
           (c >= UNICODE_HANGUL_COMPAT_BEGIN    && c <= UNICODE_HANGUL_COMPAT_END    ) ||
           (c >= UNICODE_HANGUL_EXT_A_BEGIN     && c <= UNICODE_HANGUL_EXT_A_END     ) ||
           (c >= UNICODE_HANGUL_SYLLABLES_BEGIN && c <= UNICODE_HANGUL_SYLLABLES_END ) ||
           (c >= UNICODE_HANGUL_EXT_B_BEGIN     && c <= UNICODE_HANGUL_EXT_B_END     );
}

bool UnicodeHelper::isKoreanIdeograph(wchar_t c)
{
    if(c < UNICODE_HANGUL_JAMO_BEGIN)
    {
        return false;
    }

    return (c >= UNICODE_HANGUL_JAMO_BEGIN      && c <= UNICODE_HANGUL_JAMO_END      ) ||
           (c >= UNICODE_HANGUL_COMPAT_BEGIN    && c <= UNICODE_HANGUL_COMPAT_END    ) ||
           (c >= UNICODE_HANGUL_EXT_A_BEGIN     && c <= UNICODE_HANGUL_EXT_A_END     ) ||
           (c >= UNICODE_HANGUL_SYLLABLES_BEGIN && c <= UNICODE_HANGUL_SYLLABLES_END ) ||
           (c >= UNICODE_HANGUL_EXT_B_BEGIN     && c <= UNICODE_HANGUL_EXT_B_END     );

}

bool UnicodeHelper::checkCJ(std::wstring wstring)
{
    return std::any_of(wstring.begin(), wstring.end(), [](wchar_t ch){ return isChineseJapaneseIdeograph(ch); } );
}

bool UnicodeHelper::checkK(std::wstring wstring)
{
    return std::any_of(wstring.begin(), wstring.end(), [](wchar_t ch){ return isKoreanIdeograph(ch); } );
}

bool UnicodeHelper::checkRTL(std::wstring wstring)
{
    return std::any_of(wstring.begin(), wstring.end(), [](wchar_t ch){ return isRTL(ch); } );
}

bool UnicodeHelper::isCJKLeftPunctuation(wchar_t c)
{
    return c==L'“' || c==L'‘' || c==L'「' || c==L'『' || c==L'《' || c==L'〈' || c==L'（' || c==L'【';
}

bool UnicodeHelper::isLetterOrDigit(wchar_t c)
{
    return isDigit(c) || isLetter(c);
}

bool UnicodeHelper::hasDigits(std::wstring wstring)
{
    return std::any_of(wstring.begin(), wstring.end(), [](wchar_t ch){ return isDigit(ch); } );
}

bool UnicodeHelper::isDigit(wchar_t c)
{
    return iswdigit(c);
}

bool UnicodeHelper::isLetter(wchar_t c)
{
    return isLatin(c) || isCyrillic(c) || isIndicLetter(c) || isRTL(c) || isCJKIdeograph(c);
}

bool UnicodeHelper::isUpperCase(std::wstring wstring)
{
    return std::all_of(wstring.begin(), wstring.end(), [](wchar_t ch){ return isUpperCase(ch); } );
}

bool UnicodeHelper::isUpperCase(wchar_t c)
{
    if( !isLatin(c) && !isCyrillic(c))
    {
        return false;
    }
    return ( isLatinUppercase(c) || isCyrillicUppercase(c));
}

bool UnicodeHelper::isNewline(wchar_t c)
{
    return L'\n' == c;
}

bool UnicodeHelper::isVertLine(wchar_t c)
{
    return L'|' == c;
}

bool UnicodeHelper::isNotLetter(wchar_t c, bool greedy)
{
    if (greedy)
    {
        return isWordBreaker(c);
    }
    if (isWordJoiner(c))
    {
        return false;
    }
    if (isIndicLetter(c))
    {
        return false;
    }
    return !UnicodeHelper::isLetterOrDigit(c);
}

bool UnicodeHelper::isCJK_RTL_Indic_EndSentence(wchar_t c)
{
    return c == L'。' || c == L'・' || c == L'．' ||
           c == L'･' || c == L'？'  || c == L'！' ||  // CJK
           c == L'؞' || c == L'۔' || c == L'؟'|| // Arabic
           c == L'।' || c == L'॥'; //indic
}

bool UnicodeHelper::isEndSentence(wchar_t c)
{
    return c == L'.' || c == L'!' || c == L'?' || c == L'…' || isCJK_RTL_Indic_EndSentence(c);
}

bool UnicodeHelper::isIndicLetter(wchar_t ch)
{
    if (ch >= 0xE000 && ch <= 0xF8FF)
    {
        return true; // Лигатуры
    }
    if (ch >= 0x0980 && ch <= 0x09FF)
    {
        return true; // Bengali
    }
    if ((ch >= 0x0900 && ch <= 0x097F) ||
        (ch >= 0xA8E0 && ch <= 0xA8FF) ||
        (ch >= 0x1CD0 && ch <= 0x1CFA))
    {
        return true; // Denavagari
    }
    if (ch >= 0x0C80 && ch <= 0x0CF2)
    {
        return true; // Каннада
    }
    if (ch >= 0x0D00 && ch <= 0x0D7F)
    {
        return true; // Малайялам
    }
    if (ch >= 0x0B80 && ch <= 0x0BFF)
    {
        return true; // Тамильский
    }
    if (ch >= 0x0C00 && ch <= 0x0C7F)
    {
        return true;  // Телугу
    }
    if (ch >= 0x0A80 && ch <= 0x0AFF)
    {
        return true;  // Гуджарати
    }
    if (ch >= 0x0B00 && ch <= 0x0B7F)
    {
        return true; // Oriya
    }
    return false;
}

bool UnicodeHelper::isArmenianLetter(wchar_t ch)
{
    if (ch >= 0x0530 && ch <= 0x058F)
    {
        return true;
    }
    return false;
}

bool UnicodeHelper::isWordJoiner(wchar_t c)
{
    return L'-' == c      || L'&' == c      || L'\'' == c ||
           L'\u0301' == c || L'\u2019' == c || L'`' == c ||
           c == L'\u0083'; // Unicode Character 'NO BREAK HERE'
}

bool UnicodeHelper::isWordBreaker(wchar_t c)
{
    return isSpace(c) || isNewline(c);
}

int UnicodeHelper::getUnicodeRange(std::wstring text_in)
{
    // RETURNS:
    // -1 IF UNKNOWN
    //  0 IF MIXED
    //  1 if LAT
    //  2 if CYR
    //  3 if CJK

    if(text_in.empty()) { return 0; }
    if(text_in.length() == 1)
    {
        if(UnicodeHelper::isLatin(text_in.front()))
        {
            return 1;
        }
        else if(UnicodeHelper::isCyrillic(text_in.front()))
        {
            return 2;
        }
        else if(UnicodeHelper::isCJKIdeograph(text_in.front()))
        {
            return 3;
        }
    }

    int suspect = -1; // Unknown yet
    for(auto ch : text_in)
    {
        if(UnicodeHelper::isLatin(ch))
        {
            if(suspect == -1)
            {
                suspect = 1;
            }
            else if(suspect != 1)
            {
                return 0; //return mixed
            }

        }
        else if(UnicodeHelper::isCyrillic(ch))
        {
            if(suspect == -1)
            {
                suspect = 2;
            }
            else if(suspect != 2)
            {
                return 0; //return mixed
            }
        }
        else if(UnicodeHelper::isCJKIdeograph(ch))
        {
            if(suspect == -1)
            {
                suspect = 3;
            }
            else if(suspect != 3)
            {
                return 0; //return mixed
            }
        }
    }
    return suspect;
}

bool UnicodeHelper::RFCValidURLChar(wchar_t c)
{
    return ( c == L'-' || c == L'.' || c == L'_' || c == L'~' ||
             c == L':' || c == L'/' || c == L'?' || c == L'#' ||
             c == L'[' || c == L']' || c == L'@' || c == L'!' ||
             c == L'$' || c == L'&' || c == '\'' || c == L'(' ||
             c == L')' || c == L'*' || c == L'+' || c == L',' ||
             c == L',' || c == L';' || c == L'%' || c == L'=');
}

bool UnicodeHelper::isDigitsOnly(std::wstring in)
{
    if(!hasDigits(in))
    {
        return false;
    }
    std::wstring wstring = in;

    while (isSpace(wstring.front()) || isNewline(wstring.front()))
    {
        wstring = wstring.substr(1);
    }
    while (isSpace(wstring.back()) || isNewline(wstring.back()))
    {
        wstring.pop_back();
    }

    return std::all_of(wstring.begin(), wstring.end(), [](wchar_t ch){ return isDigit(ch); } );
}
