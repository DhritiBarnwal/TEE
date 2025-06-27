# LOCAL_PATH := $(call my-dir)

# include $(CLEAR_VARS)

# LOCAL_MODULE    := tee_client_demo
# LOCAL_SRC_FILES := src/main.c
# LOCAL_C_INCLUDES := $(LOCAL_PATH)/include

# include $(BUILD_EXECUTABLE)
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := tee_client_demo
LOCAL_SRC_FILES := src/main.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_LDLIBS := -llog

# Point to prebuilt .so
LOCAL_SHARED_LIBRARIES := openteec

include $(BUILD_EXECUTABLE)

# Declare the prebuilt shared lib
include $(CLEAR_VARS)
LOCAL_MODULE := openteec
LOCAL_SRC_FILES := prebuilt/libopenteec.so
include $(PREBUILT_SHARED_LIBRARY)
