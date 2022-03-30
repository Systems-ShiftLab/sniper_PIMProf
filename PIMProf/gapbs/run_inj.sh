export OMP_NUM_THREADS=32 && ../../run-sniper --roi -n 32 -c pimprof_pim -d bfs_pimprof_pim_32 -- ./bfs.inj -f ./benchmark/kron_15.sg -n1 &
export OMP_NUM_THREADS=1 && ../../run-sniper --roi -n 1 -c pimprof_cpu -d bfs_pimprof_cpu_1 -- ./bfs.inj -f ./benchmark/kron_15.sg -n1
