# gs_register_bench
Benchmarks different methods of userspace thread context switching inspired by the x86 GS segement register 

Benchmarks under WSL2 (Linux 6.1.21.1, Debian unstable) on an i7 12700k:
```
2023-05-18T15:00:54+02:00
Running ./build-gcc-release/gs_register_bench
Run on (20 X 3609.6 MHz CPU s)
CPU Caches:
  L1 Data 48 KiB (x10)
  L1 Instruction 32 KiB (x10)
  L2 Unified 1280 KiB (x10)
  L3 Unified 25600 KiB (x1)
Load Average: 0.14, 0.04, 0.09
--------------------------------------------------------------------------
Benchmark                Time             CPU   Iterations UserCounters...
--------------------------------------------------------------------------
BM_ReadGSDirect        773 ns          791 ns       856599 bytes_per_second=4.71111G/s items_per_second=1.26463G/s
BM_ReadGSBase         2203 ns         2203 ns       314212 bytes_per_second=1.69079G/s items_per_second=453.867M/s
BM_ReadTLS             351 ns          351 ns      1982611 bytes_per_second=10.6062G/s items_per_second=2.84709G/s
BM_Read                349 ns          349 ns      2011044 bytes_per_second=10.6652G/s items_per_second=2.86292G/s
```

An implementation that stores the current base address in thread local storage appears to be the most efficient option.
It achieves nearly identical performance to reading directly from a buffer, with the buffer base address residing in a register.
