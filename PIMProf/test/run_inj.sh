export OMP_NUM_THREADS=1 && /home/warsier/Documents/sniper_PIMProf/run-sniper -n 1 -c pimprof_cpu -d inj_cpu -- /home/warsier/Documents/sniper_PIMProf/PIMProf/test/test.inj
export OMP_NUM_THREADS=4 && /home/warsier/Documents/sniper_PIMProf/run-sniper -n 4 -c pimprof_pim -d inj_pim -- /home/warsier/Documents/sniper_PIMProf/PIMProf/test/test.inj
/home/warsier/Documents/PIMProf_solver/build/PIMProfSolver/Solver.exe mpki -c inj_cpu/pimprofstats.out -p inj_pim/pimprofstats.out -o pimprofdecision.out
