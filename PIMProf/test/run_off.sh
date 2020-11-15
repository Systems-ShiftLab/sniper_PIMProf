export OMP_NUM_THREADS=1 && /home/warsier/Documents/sniper_PIMProf/run-sniper --roi -n 1 -c pimprof_cpu -d off_cpu -- /home/warsier/Documents/sniper_PIMProf/PIMProf/test/test.pim
export OMP_NUM_THREADS=4 && /home/warsier/Documents/sniper_PIMProf/run-sniper --roi -n 4 -c pimprof_pim -d off_pim -- /home/warsier/Documents/sniper_PIMProf/PIMProf/test/test.pim
