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

DDS_CC_FLAGS	:= -O3 -flto
DDS_LD_FLAGS	:= 		\
	-Wl,--dynamicbase 	\
	-Wl,--nxcompat 		\
	-Wl,--no-seh 		\
	-Wl,--enable-stdcall-fixup
DDS_WARN_FLAGS	:= 		\
	-Wshadow 		\
	-Wsign-conversion 	\
	-pedantic -Wall -Wextra  \
	-Wcast-align -Wcast-qual \
	-Wctor-dtor-privacy 	\
	-Wdisabled-optimization \
	-Winit-self 		\
	-Wlogical-op 		\
	-Wmissing-declarations 	\
	-Wmissing-include-dirs 	\
	-Wnoexcept 		\
	-Wold-style-cast 	\
	-Woverloaded-virtual 	\
	-Wredundant-decls 	\
	-Wsign-promo 		\
	-Wstrict-null-sentinel 	\
	-Wstrict-overflow=1 	\
	-Wswitch-default -Wundef \
	-Werror 		\
	-Wno-unused 		\
	-Wno-unknown-pragmas 	\
	-Wno-long-long		\
	-Wno-format

LOCAL_MODULE    := dds
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../include
LOCAL_CFLAGS := $(DDS_CC_FLAGS) $(DDS_CC_THR)
LOCAL_LD_FLAGS := $(DDS_LD_FLAGS) -O3 -flto
LOCAL_SRC_FILES := \
    dds.cpp         \
    ABsearch.cpp        \
    ABstats.cpp     \
    CalcTables.cpp      \
    DealerPar.cpp       \
    Init.cpp        \
    LaterTricks.cpp     \
    Moves.cpp       \
    Par.cpp         \
    PlayAnalyser.cpp    \
    PBN.cpp         \
    QuickTricks.cpp     \
    Scheduler.cpp       \
    SolveBoard.cpp      \
    SolverIF.cpp        \
    Stats.cpp       \
    Timer.cpp       \
    TransTable.cpp

include $(BUILD_STATIC_LIBRARY)
