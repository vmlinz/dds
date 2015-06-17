# Copyright (C) 2009 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

DDS_THREADS := none
ifeq ($(DDS_THREADS),none)
DDS_CC_THR		:= -DDDS_THREADS_SINGLE
else
DDS_CC_THR		:=
endif

LOCAL_MODULE    := dds
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)
LOCAL_CFLAGS += $(DDS_CC_THR)
LOCAL_SRC_FILES := \
    dds.cpp \
    Par.cpp \
    DealerPar.cpp

include $(BUILD_STATIC_LIBRARY)
