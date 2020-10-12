include ../buildconf.makefile

PIMPROFOFFLOADER_SO := $(LLVM_BUILD_DIR)/libPIMProfOffloader.so

#ifndef PIMPROF_DIR
#$(error PIMPROF_DIR is not set in Makefile.config)
#endif

DECISION            := decision.out
TEST_OBJ            := test.o # print4.o pthreads.o
TEST_SO             := libtest.so
TEST_EXE            := test.exe
TEST_PY             := test.py

TEST_OBJ_2          := test.o.2 print4.o.2 pthreads.o.2
TEST_EXE_2          := test.exe.2

PIMPROF_ROOT = /home/warsier/Documents/PIMProf_solver
CXXFLAGS := -fPIC -std=c++11 -g -O3 -fopenmp -I$(PIMPROF_ROOT)/LLVMAnalysis
CXX_INJ_FLAGS = $(CXXFLAGS) $(SNIPER_CFLAGS) -Xclang -load -Xclang $(PIMPROF_ROOT)/build/LLVMAnalysis/libAnnotationInjection.so
CXX_OFF_FLAGS += $(CXXFLAGS) $(SNIPER_CFLAGS) -Xclang -load -Xclang $(PIMPROF_ROOT)/build/LLVMAnalysis/libOffloaderInjection.so

test.inj: test.cpp # print4.cpp pthreads.cpp
	export PIMPROFINJECTMODE=SNIPER &&clang++-7 $(CXX_INJ_FLAGS) -o $@ $^ -pthread

test.pim: test.cpp # print4.cpp pthreads.cpp
	export PIMPROFDECISION=pimprofdecision.out && export PIMPROFROI=PIMONLY && export PIMPROFINJECTMODE=SNIPER && clang++-7 $(CXX_OFF_FLAGS) -o $@ $^ -pthread > test.dump 2>&1

clean:
	rm -rf *.inj *.pim inj_cpu inj_pim off_cpu off_pim

.PHONY: all offload clean
