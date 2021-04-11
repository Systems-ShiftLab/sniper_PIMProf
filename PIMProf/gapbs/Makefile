# See LICENSE.txt for license details.
include ../buildconf.makefile

CXX = /usr/bin/clang++-7
PIMPROF_ROOT = /home/warsier/Documents/PIMProf_solver/
VTUNE_ROOT = /home/warsier/intel/vtune_profiler/

CXX_FLAGS = -std=c++11 -O3 -Wall -fopenmp -g -I$(PIMPROF_ROOT)/LLVMAnalysis
#CXX_FLAGS = -std=c++11 -O3 -Wall -fopenmp -g -march=skylake-avx512 -I$(PIMPROF_ROOT)/LLVMAnalysis
#CXX_FLAGS_2 = -std=c++11 -O3 -Wall -g -I$(PIMPROF_ROOT)/LLVMAnalysis
CXX_INJ_FLAGS = $(CXX_FLAGS) -Xclang -load -Xclang $(PIMPROF_ROOT)/build/LLVMAnalysis/libAnnotationInjection.so
CXX_OFF_FLAGS += $(CXX_FLAGS) -Xclang -load -Xclang $(PIMPROF_ROOT)/build/LLVMAnalysis/libOffloaderInjection.so
LD_FLAGS = -pthread -Wl,-Bstatic -L$(SNIPER_ROOT)/lib -Wl,-Bdynamic -lm

KERNELS = bc bfs cc pr sssp # cc_sv tc
SUITE = $(KERNELS) converter
#SUITE_ZSIM = $(SUITE:=.zsim)
#SUITE_FINAL = $(SUITE:=.final)

inj: $(KERNELS:=.inj)
inj2: $(KERNELS:=.inj2)
pim: $(KERNELS:=.pim)

%.inj: src/%.cc src/*.h
	export PIMPROFINJECTMODE=SNIPER && $(CXX) $(CXX_INJ_FLAGS) $(SNIPER_CFLAGS) -o $@ $< $(LD_FLAGS) > $(@:.inj=.inj.dump) 2>&1

%.inj2: src/%.cc src/*.h
	export PIMPROFINJECTMODE=SNIPER2 && $(CXX) $(CXX_INJ_FLAGS) $(SNIPER_CFLAGS) -o $@ $< $(LD_FLAGS) > $(@:.inj=.inj.dump) 2>&1

# # PIMProf-auto
# %.cpu : src/%.cc src/*.h
# 	export PIMPROFDECISION=$(shell pwd)/gapbs-32/$(@:.cpu=_defaultconfig_32.out) && export PIMPROFROI=NOTPIM && export PIMPROFINJECTMODE=SNIPER && $(CXX) $(CXX_FLAGS) $(CXX_OFF_FLAGS) $(SNIPER_CFLAGS) -o $@ $< $(LD_FLAGS) > $(@:.cpu=.cpu.dump) 2>&1

# %.cpuonly : src/%.cc src/*.h
# 	export PIMPROFDECISION=$(shell pwd)/gapbs-32/$(@:.cpuonly=_defaultconfig_32.out) && export PIMPROFROI=CPUONLY && export PIMPROFINJECTMODE=SNIPER && $(CXX) $(CXX_FLAGS) $(CXX_OFF_FLAGS) $(SNIPER_CFLAGS) -o $@ $< $(LD_FLAGS) > $(@:.cpuonly=.cpuonly.dump) 2>&1

%.pim : src/%.cc src/*.h
	export PIMPROFDECISION=$(shell pwd)/gapbs-32/$(@:.pim=.out) && export PIMPROFROI=PIMONLY && export PIMPROFINJECTMODE=SNIPER && $(CXX) $(CXX_FLAGS) $(CXX_OFF_FLAGS) $(SNIPER_CFLAGS) -o $@ $< $(LD_FLAGS) > $(@:.pim=.pim.dump) 2>&1

# %.all : src/%.cc src/*.h
# 	export PIMPROFDECISION=$(shell pwd)/gapbs-32/$(@:.all=_defaultconfig_32.out) && export PIMPROFROI=ALL && export PIMPROFINJECTMODE=SNIPER && $(CXX) $(CXX_FLAGS) $(CXX_OFF_FLAGS) $(SNIPER_CFLAGS) -o $@ $< $(LD_FLAGS) > $(@:.all=.all.dump) 2>&1

# # # MPKI-hand
# # %.cpu2 : src/%.cc src/*.h
# # 	$(CXX) $(CXX_FLAGS) $(SNIPER_CFLAGS) -DSNIPERTEST=0 -o $@ $< $(LD_FLAGS)

# # %.pim2 : src/%.cc src/*.h
# # 	$(CXX) $(CXX_FLAGS) $(SNIPER_CFLAGS) -DSNIPERTEST=1 -o $@ $< $(LD_FLAGS)

# # PIMProf-hand
# %.cpu3 : src/%.cc src/*.h
# 	$(CXX) $(CXX_FLAGS) $(SNIPER_CFLAGS) -DSNIPER=5 -o $@ $< $(LD_FLAGS)

# %.pim3 : src/%.cc src/*.h
# 	$(CXX) $(CXX_FLAGS) $(SNIPER_CFLAGS) -DSNIPER=6 -o $@ $< $(LD_FLAGS)

# # MPKI-hand-2
# %.cpu4 : src/%.cc src/*.h
# 	$(CXX) $(CXX_FLAGS) $(SNIPER_CFLAGS) -DSNIPERTEST=2 -o $@ $< $(LD_FLAGS)

# %.pim4 : src/%.cc src/*.h
# 	$(CXX) $(CXX_FLAGS) $(SNIPER_CFLAGS) -DSNIPERTEST=3 -o $@ $< $(LD_FLAGS)

# # VTune PIMProf-hand
# %.vtune : src/%.cc src/*.h
# 	$(CXX) $(CXX_FLAGS) $(SNIPER_CFLAGS) -I$(VTUNE_ROOT)/include -DVTUNE -o $@ $< $(LD_FLAGS) -L$(VTUNE_ROOT)/lib64/ -littnotify -ldl

# # VTune PIMProf-auto
# %.vtune2 : src/%.cc src/*.h
# 	export PIMPROFDECISION=$(shell pwd)/gapbs-32/$(@:.vtune2=_defaultconfig_32.out) && export PIMPROFROI=ALL && export PIMPROFINJECTMODE=VTUNE && $(CXX) $(CXX_FLAGS) $(CXX_OFF_FLAGS) $(SNIPER_CFLAGS) -I$(VTUNE_ROOT)/include -o $@ $< $(LD_FLAGS) -Wl,-rpath=$(PIMPROF_ROOT)/build/LLVMAnalysis -L$(PIMPROF_ROOT)/build/LLVMAnalysis -lPIMProfOffloader -L$(VTUNE_ROOT)/lib64/ -littnotify -ldl > $(@:.vtune2=.vtune2.dump) 2>&1

.PHONY: clean
clean:
	rm -f $(SUITE) test/out/* *.inj *.inj2 *.pim *.pim.dump

clean_kernel:
	rm -f $(KERNELS)
