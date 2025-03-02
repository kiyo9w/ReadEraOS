/*
 * Copyright (C) 2013 The MuPDF CLI viewer interface Project
 * Copyright (C) 2013-2020 READERA LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __MUPDF_BRIDGE_H__
#define __MUPDF_BRIDGE_H__

#include <string>
#include <set>
#include <vector>
#include <map>

extern "C" {
#include <mupdf/pdf.h>
#include <mupdf/xps.h>
#include <../erapdf/include/mupdf/fitz/output.h>
};

#define FORMAT_PDF 1
#define FORMAT_XPS 2

#include "StBridge.h"
#include "StSearchUtils.h"
#include "openreadera.h"

#define CURRENT_MAX_VERSION 2

class EraConfig{
public:
    int erapdf_twilight_mode = 0;
    float f_font_color[3] = {1,1,1};
    float f_bg_color[3]   = {0,0,0};

/*
    #if OREDEBUG
    EraConfig()
    {
        erapdf_twilight_mode = 1;
        const float FLOAT_COEFF = 0.00390625f; // == 1/256

        unsigned int c = 0x2b2b2b;
        f_bg_color[0] = ((c >> 16) & 0xFF) * FLOAT_COEFF ; //r
        f_bg_color[1] = ((c >> 8 ) & 0xFF) * FLOAT_COEFF ; //g
        f_bg_color[2] = ( c        & 0xFF) * FLOAT_COEFF ; //b

        //c = 0xC7BAAA; //жёлтый
        c = 0xA9B7C6; //синий
        //c = 0xC5C6C7; //серый
        f_font_color[0] = ((c >> 16) & 0xFF) * FLOAT_COEFF ; //r
        f_font_color[1] = ((c >> 8 ) & 0xFF) * FLOAT_COEFF ; //g
        f_font_color[2] = ( c        & 0xFF) * FLOAT_COEFF ; //b
    }
    #endif
*/

    void applyToCtx(fz_context *ctx)
    {
        ctx->erapdf_twilight_mode = erapdf_twilight_mode;

        ctx->f_font_color[0] = f_font_color[0];
        ctx->f_font_color[1] = f_font_color[1];
        ctx->f_font_color[2] = f_font_color[2];

        ctx->f_bg_color[0] = f_bg_color[0];
        ctx->f_bg_color[1] = f_bg_color[1];
        ctx->f_bg_color[2] = f_bg_color[2];
    };
};

class ReflowManager;

class SearchResult{
public:
    SearchResult(){};
    SearchResult(std::wstring pre, std::vector<std::string> xps)
    {
        preview_ = pre;
        xpointers_ = xps;
    }
    std::wstring preview_;
    std::vector<std::string> xpointers_;
};

class SearchWindow
{
public:
    int start_ = -1;
    int pos_   = -1;
    int end_   = -1;
    int startpos_   = -1;
    std::vector<Hitbox_st> text_;
    std::wstring query_;

    std::vector<std::string> xp_array_;
    SearchWindow() {};

    SearchWindow(std::vector<Hitbox_st> & text, std::wstring& query, int startpos)
    {
        startpos_ = startpos;
        text_  = text;
        query_ = query;
    }

    void addpos(int pos, std::string xp)
    {
        pos_ = pos;
        xp_array_.push_back(xp);
        if( start_==-1 && end_ == -1) // if not initialised
        {
            init();
        }
        else
        {
            run();
        }
    }

    void init()
    {
        int counter = 0;
        int backoffset = pos_;
        int frontoffset = pos_ + query_.length();

        int addcount = 0;
        while(counter < TEXT_SEARCH_PREVIEW_WORD_NUM)
        {
            backoffset--;
            if(backoffset <= 0)// || text.at(backoffset)==L'\n')
            {
                backoffset = 0;
                addcount = TEXT_SEARCH_PREVIEW_WORD_NUM - counter;
                break;
            }
            if(backoffset <= startpos_)
            {
                addcount = TEXT_SEARCH_PREVIEW_WORD_NUM - counter;
                break;
            }
            if(text_.at(backoffset).text_.at(0)==L' ')
            {
                counter++;
            }
        }
        counter = 0;
        while (counter < TEXT_SEARCH_PREVIEW_WORD_NUM + addcount)
        {
            frontoffset++;
            if(frontoffset>=text_.size())// || text.at(frontoffset)==L'\n')
            {
                frontoffset = text_.size();
                break;
            }
            if(text_.at(frontoffset).text_.at(0)==L' ')
            {
                counter++;
            }
        }
        start_ = backoffset;
        end_ = frontoffset;
    }

    void run(){
        if(pos_ + query_.length() > end_)
        {
            end_ = pos_ + query_.length();
        }
        if(end_< text_.size())
        {
            for (int i = end_ ; i <  text_.size(); i++)
            {
                if(text_.at(i).text_.at(0) == L' ')
                {
                    end_ = i;
                    break;
                }
            }
        }
    };

    bool contains(int pos)
    {
        if( start_==-1 && end_ == -1) // if not initialised
        {
            return  true;
        }
        return (pos >= start_ && pos <= end_);
    }
};

class PageHitboxesCache
{
private:
    int page_ = -1;
    std::vector<Hitbox_st> PageHitboxes_;
public:
    PageHitboxesCache(){};
    PageHitboxesCache(int page, std::vector<Hitbox_st> Hitboxes){
        page_ = page;
        PageHitboxes_ = Hitboxes;
    }

    int getPage()
    {
        return page_;
    }

    std::vector<Hitbox_st> getArray()
    {
        return PageHitboxes_;
    };

    Hitbox_st get(int num)
    {
        return PageHitboxes_.at(num);
    }

    int length() {
        return PageHitboxes_.size();
    }

    bool isEmpty()
    {
        return ((page_ == -1) || (PageHitboxes_.empty()));
    }

    void reset()
    {
        page_ = -1;
        PageHitboxes_.clear();
    }
};

#define PAGE_CACHE_MODE_DEFAULT   0
#define PAGE_CACHE_MODE_FAST      1
#define PAGE_CACHE_MODE_SUPERFAST 2 //render-time text

class PageTextCache
{
    std::wstring wstring_ = std::wstring();
    int page_ = -1;
    int mode_ = -1; // 0 = default, 1 = fast, 2 = superfast(render-time text)
public:
    PageTextCache(){};
    PageTextCache(int page, std::wstring in, int mode)
    {
        page_ = page;
        wstring_ = in;
        mode_ = mode;
    }

    std::wstring getText() { return wstring_;};
    int getPage()          { return page_; }
    int getMode()          { return mode_;}
    int length()           { return wstring_.length();}
    bool isEmpty()         { return ((page_ == -1) || (wstring_.empty()) || (mode_ == -1));}

    void reset()
    {
        mode_ = -1;
        page_ = -1;
        wstring_.clear();
    }
};

class TextCache
{
private:
    std::vector<PageTextCache> array;
    const int maxsize = 5;

public:
    TextCache()
    {
        array.reserve(maxsize);
    }

    void clear()
    {
        for (int i = 0; i < array.size(); i++)
        {
            array.at(i).reset();
        }
        array.clear();
    }

    void addItem(int page, std::wstring text, int mode)
    {
        if(text.empty())
        {
            return;
        }


        if(!pageExists(page))
        {
            LE("added text to cache [%d] [%d]",page,mode);
            array.emplace_back(page, text, mode);
        }

        //cleanup of the oldest item in cache
        int diff = array.size() - maxsize;
        if (diff > 0)
        {
            for (int i = 0; i < diff; i++)
            {
                array.erase(array.begin());
            }
        }
    }

    std::wstring findPageText(int pagenum)
    {
        if (array.empty())
        {
            //LE("cachedText empty array");
            return std::wstring();
        }
        for (int i = array.size() - 1; i >= 0; i--)
        {
            //LE("cachedText i = [%d] FOR PAGE [%d] = [%ls]", i, array.at(i).getPage(), array.at(i).getText().c_str());
            if (array.at(i).getPage() == pagenum)
            {
                //LE("cachedText FOUND");
                return array.at(i).getText();
            }
        }
        //LE("cachedText NOT FOUND");
        return std::wstring();
    };

private:
    bool pageExists(int page)
    {
        for(auto& item: array)
        {
            if (item.getPage() == page)
            {
                return true;
            }
        }
        return false;
    }
};

class ReflowManager;
class MuPdfBridge : public StBridge
{
private:
    int config_format = 0;
    int config_invert_images = 0;
	int fd;
    char* password;

    fz_context *ctx;
    fz_document *document;
    fz_outline *outline;

    uint32_t pageCount;
    fz_page **pages;
    fz_display_list **pageLists;

    int storememory;
    int format;
    int layersmask;

    int searchPackCounter = 0;
    std::set<std::string> fonts;
    std::vector<PageHitboxesCache> hitboxesCache;
    ReflowManager* reflowManager;
    EraConfig eraConfig;

    std::vector<std::pair<std::wstring,std::wstring>> searchDictionary;
    std::vector<std::wstring> searchDictionaryCJK;
    std::vector<std::pair<std::wstring,std::wstring>> searchDictionaryRTL;

    std::string precise_cachedPageText = std::string();
    int precise_cachedPageNum = -1;

    TextCache textCache;

    std::wstring cachedHitboxesPageText = std::wstring();
    int cachedHitboxesPageNum = -1;
public:
    MuPdfBridge();
    ~MuPdfBridge();

    void process(CmdRequest& request, CmdResponse& response);
    static void responseAddString(CmdResponse &response, std::wstring str16);
    std::map<int, std::pair<float,float>> colontitles_map; // page : (top,bottom);

protected:
    void processOpen(CmdRequest& request, CmdResponse& response);
    void processQuit(CmdRequest& request, CmdResponse& response);
    void processPageInfo(CmdRequest& request, CmdResponse& response);
    void processPage(CmdRequest& request, CmdResponse& response);
	void processPageLinks(CmdRequest& request, CmdResponse& response);
    void processPageRender(CmdRequest& request, CmdResponse& response);
    void processPageFree(CmdRequest& request, CmdResponse& response);
    void processOutline(CmdRequest& request, CmdResponse& response);
    void processPageText(CmdRequest& request, CmdResponse& response);
    void processFontsConfig(CmdRequest& request, CmdResponse& response);
    void processConfig(CmdRequest& request, CmdResponse& response);
    void processStorage(CmdRequest& request, CmdResponse& response);
    void processSystemFont(CmdRequest& request, CmdResponse& response);
    void processGetMissedFonts(CmdRequest& request, CmdResponse& response);
    void processGetLayersList(CmdRequest& request, CmdResponse& response);
    void processSetLayersMask(CmdRequest& request, CmdResponse& response);
	void processSmartCrop(CmdRequest& request, CmdResponse& response);
	void processXPathByRectId(CmdRequest &request, CmdResponse &response);

    void processAnalyzeDocForReflow(CmdRequest& request, CmdResponse& response);
    void processDocToFB2(CmdRequest& request, CmdResponse& response);
    void processXpathByCoords(CmdRequest& request, CmdResponse& response);

    friend class ReflowManager;


    fz_page* getPage(uint32_t index, bool decode);
    bool renderPage(uint32_t index, int w, int h, unsigned char* pixels, const fz_matrix_s* ctm);
    bool restart();
    void release();
    void resetFonts();
    void processLinks(int pageNo, CmdResponse& response);
    void processOutline(fz_outline *outline, int level, int index, CmdResponse& response);
    void processTextSearchPreviews(CmdRequest &request, CmdResponse &response);
    void processTextSearchHitboxes(CmdRequest &request, CmdResponse &response);
    void processSearchCounter(CmdRequest &request, CmdResponse &response);
    void processPageRangeText(CmdRequest &request, CmdResponse &response);
    void processRtlText(CmdRequest &request, CmdResponse &response);

    void processPageDictionaryCheck(CmdRequest &request, CmdResponse &response);
    void processRegisterDictionary(CmdRequest &request, CmdResponse &response);

    void processGetTTSSentence(CmdRequest &request, CmdResponse &response);
    void processGetTTSXpathSentence(CmdRequest &request, CmdResponse &response);
    void processGetTTSHitboxes(CmdRequest &request, CmdResponse &response);

    void processSmartColontitles(CmdRequest &request, CmdResponse &response);
    void processRegisterColontitles(CmdRequest &request, CmdResponse &response);
    void processSmartColontitlesText(CmdRequest &request, CmdResponse &response);

    void applyLayersMask();

    std::string GetXpathFromPageById(std::vector<Hitbox_st> hitboxes, int id, bool addcoords, bool reverse);
    std::string GetXpathFromPageById(int page, int id, bool addcoords, bool reverse);
    std::vector<std::string> GetXpathFromPageById(std::vector<Hitbox_st> hitboxes, std::vector<int> pos_arr, bool addcoords, int qlen);
    std::vector<Hitbox_st> processTextToArray(int pageNo, int version = CURRENT_MAX_VERSION);
    std::vector<Hitbox_st> processTextToArray_imp(int pageNo, int version = CURRENT_MAX_VERSION);
    std::string getPageText(int pageNo, bool fast = false);
    // Search functions
    std::vector<SearchResult> SearchForTextPreviews(int page, std::wstring query);
    std::vector<SearchResult> FindAndTrim(std::wstring query, int page, int &end);
    std::vector<Hitbox_st> GetSearchHitboxes(std::vector<Hitbox_st> hitboxes, int page, std::wstring query, bool wordBeginFlag = false);
    std::vector<Hitbox_st> SearchForTextHitboxes(int page, std::wstring query, bool wordBeginFlag = false);
    std::vector<Hitbox_st> GetHitboxesBetweenXpaths(int page, std::string xpStart, std::string xpEnd, int startPage, int version);
    /*
    SearchResult FindAndTrimNextPage(std::wstring query, int page, int start_last);
    int GetSearchHitboxesNextPageLite(int page, std::wstring query);
    std::vector<Hitbox_st> GetSearchHitboxesPrevPage(std::vector<Hitbox_st> base, std::wstring query, int last_len);
    std::vector<Hitbox_st> GetSearchHitboxesNextPage(std::vector<Hitbox_st> base, int page, std::wstring query);
    */
    void analyzePageForDarkMode(fz_context *ctx, int index, int w, int h);

    std::string GetXpathFromPageByCoords(int page, float x, float y, bool addcoords, bool reverse);


    std::vector<std::wstring> checkIfPageHasWords(uint32_t pagenum);

    std::vector<std::vector<Hitbox_st>> getPageSentences(int page, std::vector<HitboxSentence> *curr_sentences);
};

bool isQuote(int ch);

#endif
