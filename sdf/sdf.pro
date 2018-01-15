######################################################################
# Automatically generated by qmake (3.0) Thu Jan 26 16:35:29 2017
######################################################################

include(../common/common.pri)
TARGET = sdf

# Input
HEADERS += \
           ../common/include/camera.h \
           ../common/include/fixedcamera.h \
           ../common/include/scene.h \
           ../common/include/trackballcamera.h \
           src/sdfscene.h
SOURCES += src/main.cpp \
           ../common/src/camera.cpp \
           ../common/src/fixedcamera.cpp \
           ../common/src/scene.cpp \
           ../common/src/trackballcamera.cpp \
           src/sdfscene.cpp

OTHER_FILES += shaders/sdf_frag.glsl \
	       shaders/sdf_vert.glsl 

DISTFILES += $OTHER_FILES


