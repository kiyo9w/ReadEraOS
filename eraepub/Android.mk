LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := eraepub
LOCAL_STATIC_LIBRARIES := orebridge orelibjpeg orelibpng webp libfreetype2
LOCAL_LDLIBS += -llog -lz
LOCAL_CPP_FEATURES += exceptions

LOCAL_CPPFLAGS := $(APP_CPPFLAGS)
LOCAL_CFLAGS := $(APP_CFLAGS)
LOCAL_CFLAGS += -DHAVE_CONFIG_H -DLINUX=1 -D_LINUX=1 -DFT2_BUILD_LIBRARY=1
LOCAL_CFLAGS += -DCR3_ANTIWORD_PATCH=1 -DENABLE_ANTIWORD=1
LOCAL_CFLAGS += -DJPEG_LIB_VERSION=80

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/../orebridge/include \
    $(LOCAL_PATH)/../ \
    $(LOCAL_PATH)/libpng \
    $(LOCAL_PATH)/freetype/include

LOCAL_SRC_FILES := \
	EraEpubMain.cpp \
	EraEpubBridge.cpp \
	EraEpubConf.cpp \
	EraEpubMeta.cpp \
	EraEpubOutline.cpp \
	EraEpubLinks.cpp \
	EraEpubSearch.cpp \
	EraEpubTexts.cpp \
	EraEpubImages.cpp \
	EraEpubTTS.cpp

LOCAL_SRC_FILES += \
	src/erae_log.cpp \
	src/bookmark.cpp \
	src/lvtoc.cpp \
	src/chmfmt.cpp \
    src/cp_stats.cpp \
    src/crtxtenc.cpp \
    src/epubfmt.cpp \
    src/hyphman.cpp \
    src/lstridmap.cpp \
    src/lvbmpbuf.cpp \
    src/lvdocview.cpp \
    src/crcss.cpp \
    src/lvdrawbuf.cpp \
    src/lvfnt.cpp \
    src/lvfntman.cpp \
    src/lvimg.cpp \
    src/lvpagesplitter.cpp \
    src/lvrend.cpp \
    src/lvstream.cpp \
    src/lvstring.cpp \
    src/lStringCollection.cpp \
    src/lvstsheet.cpp \
    src/lvstyles.cpp \
    src/lvtextfm.cpp \
    src/lvtinydom.cpp \
    src/lvxml.cpp \
    src/pdbfmt.cpp \
    src/props.cpp \
    src/rtfimp.cpp \
    src/txtselector.cpp \
    src/wordfmt.cpp \
    src/docxhandler.cpp \
    src/odthandler.cpp \
    src/RectHelper.cpp \
    src/fb2fmt.cpp \
    src/fb3fmt.cpp \
    src/FootnotesPrinter.cpp \
    src/rtlhandler.cpp \
    src/EpubItems.cpp \
    src/crconfig.cpp \
    src/charProps.cpp \
    src/dvngLig.cpp \
    src/serialBuf.cpp \
    src/indicUtils.cpp

LOCAL_SRC_FILES += \
    src/indic/devanagariManager.cpp \
    src/indic/banglaManager.cpp \
    src/indic/malayalamManager.cpp \
    src/indic/kannadaManager.cpp \
    src/indic/tamilManager.cpp \
    src/indic/teluguManager.cpp \
    src/indic/gujaratiManager.cpp \
    src/indic/oriyaManager.cpp

LOCAL_SRC_FILES += \
    chmlib/src/chm_lib.c \
    chmlib/src/lzx.c 

LOCAL_SRC_FILES += \
    antiword/asc85enc.c \
    antiword/blocklist.c \
    antiword/chartrans.c \
    antiword/datalist.c \
    antiword/depot.c \
    antiword/doclist.c \
    antiword/fail.c \
    antiword/finddata.c \
    antiword/findtext.c \
    antiword/fontlist.c \
    antiword/fonts.c \
    antiword/fonts_u.c \
    antiword/hdrftrlist.c \
    antiword/imgexam.c \
    antiword/listlist.c \
    antiword/misc.c \
    antiword/notes.c \
    antiword/options.c \
    antiword/out2window.c \
    antiword/pdf.c \
    antiword/pictlist.c \
    antiword/prop0.c \
    antiword/prop2.c \
    antiword/prop6.c \
    antiword/prop8.c \
    antiword/properties.c \
    antiword/propmod.c \
    antiword/rowlist.c \
    antiword/sectlist.c \
    antiword/stylelist.c \
    antiword/stylesheet.c \
    antiword/summary.c \
    antiword/tabstop.c \
    antiword/unix.c \
    antiword/utf8.c \
    antiword/word2text.c \
    antiword/worddos.c \
    antiword/wordlib.c \
    antiword/wordmac.c \
    antiword/wordole.c \
    antiword/wordwin.c \
    antiword/xmalloc.c

include $(BUILD_EXECUTABLE)