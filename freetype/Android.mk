LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE:= libfreetype2
#LOCAL_SHARED_LIBRARIES += orelibpng minizip
LOCAL_CFLAGS = $(APP_CFLAGS) -W -Wall -fPIC -DPIC "-DDARWIN_NO_CARBON" "-DFT2_BUILD_LIBRARY"

# compile in ARM mode, since the glyph loader/renderer is a hotspot
ifeq ($(TARGET_ARCH),arm)
    LOCAL_ARM_MODE := arm
endif
ifeq ($(TARGET_ARCH),x86)
endif

## LOOK AT INSTALL.ANY if you need more info on compiled files

LOCAL_SRC_FILES:= \
src/base/ftsystem.c     \
src/base/ftinit.c       \
src/base/ftdebug.c      \
src/base/ftbase.c       \
src/base/ftbbox.c       \
src/base/ftglyph.c      \
src/base/ftbdf.c        \
src/base/ftbitmap.c     \
src/base/ftcid.c        \
src/base/ftfstype.c     \
src/base/ftgasp.c       \
src/base/ftgxval.c      \
src/base/ftmm.c         \
src/base/ftotval.c      \
src/base/ftpatent.c     \
src/base/ftpfr.c        \
src/base/ftstroke.c     \
src/base/ftsynth.c      \
src/base/fttype1.c      \
src/base/ftwinfnt.c     \
src/bdf/bdf.c           \
src/cff/cff.c           \
src/cid/type1cid.c      \
src/pcf/pcf.c           \
src/pfr/pfr.c           \
src/sfnt/sfnt.c         \
src/truetype/truetype.c \
src/type1/type1.c       \
src/type42/type42.c     \
src/winfonts/winfnt.c   \
src/smooth/smooth.c     \
src/raster/raster.c     \
src/sdf/sdf.c           \
src/autofit/autofit.c   \
src/cache/ftcache.c     \
src/gzip/ftgzip.c       \
src/lzw/ftlzw.c         \
src/bzip2/ftbzip2.c     \
src/gxvalid/gxvalid.c   \
src/otvalid/otvalid.c   \
src/psaux/psaux.c       \
src/pshinter/pshinter.c \
src/psnames/psnames.c   \
src/svg/ftsvg.c         \
src/svg/svg.c

##src/base/ftmac.c MAC ONLY

LOCAL_C_INCLUDES += \
    $(LOCAL_PATH)/../ \
    $(LOCAL_PATH)/include \
    $(LOCAL_PATH)/include/freetype \
    $(LOCAL_PATH)/include/freetype/internal \
    $(LOCAL_PATH)/../orelibpng \
    $(LOCAL_PATH)/../minizip/zlib

include $(BUILD_STATIC_LIBRARY)