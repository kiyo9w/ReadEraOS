//
// Created by Tarasus on 04.04.2023.
//

#ifndef CODE_READERA_TARASUS_STUNICODEHELPER_H
#define CODE_READERA_TARASUS_STUNICODEHELPER_H

#include <vector>
#include <string>
#include <cctype>

class UnicodeHelper
{
    static const int UNICODE_CJK_IDEOGRAPHS_BEGIN = 0x4e00;
    static const int UNICODE_CJK_IDEOGRAPHS_END  = 0x9FFF;

    static const int UNICODE_CJK_PUNCTUATION_BEGIN = 0x3000;
    static const int UNICODE_CJK_PUNCTUATION_END = 0x303F;
    static const int UNICODE_CJK_PUNCTUATION_HALF_AND_FULL_WIDTH_BEGIN = 0xFF01;
    static const int UNICODE_CJK_PUNCTUATION_HALF_AND_FULL_WIDTH_END = 0xFFEE;
    static const int UNICODE_GENERAL_PUNCTUATION_BEGIN = 0x2000;
    static const int UNICODE_GENERAL_PUNCTUATION_END = 0x206F;

    //Japanese
    static const int UNICODE_JP_HIRAGANA_BEGIN = 0x3040;
    static const int UNICODE_JP_HIRAGANA_END = 0x309F;
    static const int UNICODE_JP_KATAKANA_BEGIN = 0x30A0;
    static const int UNICODE_JP_KATAKANA_END = 0x30FF;

    //korean
    static const int UNICODE_HANGUL_JAMO_BEGIN = 0x1100;
    static const int UNICODE_HANGUL_JAMO_END = 0x11FF;
    static const int UNICODE_HANGUL_COMPAT_BEGIN = 0x3130;
    static const int UNICODE_HANGUL_COMPAT_END = 0x318F;
    static const int UNICODE_HANGUL_EXT_A_BEGIN = 0xA960;
    static const int UNICODE_HANGUL_EXT_A_END = 0xA97F;
    static const int UNICODE_HANGUL_SYLLABLES_BEGIN = 0xAC00;
    static const int UNICODE_HANGUL_SYLLABLES_END = 0xD7AF;
    static const int UNICODE_HANGUL_EXT_B_BEGIN = 0xD7B0;
    static const int UNICODE_HANGUL_EXT_B_END = 0xD7FF;

    static const std::vector<wchar_t> sLeftQuotationSet;
    static const std::vector<wchar_t> sRightQuotationSet;
    static const std::vector<wchar_t> sTypographicalBulletsSet;
    static const std::vector<wchar_t> sLeftBracketsSet;
    static const std::vector<wchar_t> sRightBracketsSet;

public:
    static bool isLeftBracket(wchar_t c);

    static bool isRightBracket(wchar_t c);

    static bool isLeftQuotation(wchar_t c);

    static bool isRightQuotation(wchar_t c);

    static bool isTypographicalBullet(wchar_t c);

    static bool isCJKPunctuation(wchar_t c);

    static bool isChineseJapaneseIdeograph(wchar_t c);

    static bool isChineseJapanese(wchar_t c);

    static bool isLatin(wchar_t c);

    static bool isCyrillic(wchar_t c);

    static bool isPunct(wchar_t c);

    static bool isSpace(wchar_t ch);

    static bool isRTL(wchar_t c);

    static bool checkCJ(std::wstring wstring);

    static bool checkK(std::wstring wstring);

    static bool checkRTL(std::wstring wstring);

    static bool isCJKLeftPunctuation(wchar_t c);

    static bool isCJKIdeograph(wchar_t c);

    static bool isKoreanIdeograph(wchar_t c);

    static bool isLetterOrDigit(wchar_t c);

    static bool isLetter(wchar_t c);

    static bool hasDigits(std::wstring wstring);
    static bool isDigit(wchar_t c);
    static bool isDigitsOnly(std::wstring in);

    static bool isUpperCase(std::wstring wstring);
    static bool isUpperCase(wchar_t c);

    static bool isLatinUppercase(wchar_t c);

    static bool isCyrillicUppercase(wchar_t c);

    static bool isNewline(wchar_t c);

    static bool isVertLine(wchar_t c);

    static bool isNotLetter(wchar_t c, bool greedy);

    static bool isCJK_RTL_Indic_EndSentence(wchar_t c);

    static bool isEndSentence(wchar_t c);

    static bool isIndicLetter(wchar_t ch);

    static bool isArmenianLetter(wchar_t ch);

    static bool isWordJoiner(wchar_t c);

    static bool isWordBreaker(wchar_t c);

    // RETURNS:
    // -1 IF UNKNOWN
    //  0 IF MIXED
    //  1 if LAT
    //  2 if CYR
    //  3 if CJK
    static int getUnicodeRange(std::wstring text_in);

    static bool RFCValidURLChar(wchar_t c);
};


#endif //CODE_READERA_TARASUS_STUNICODEHELPER_H
