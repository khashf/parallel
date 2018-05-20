## How to...
### Compile
1. Source the necessary environment variables and settings. See [Intel's getting started guide](file:///opt/intel/documentation_2018/en/compiler_c/ps2018/get_started_lc.htm). In my computer, it's
```
source /opt/intel/parallel_studio_xe_2018/bin/psxevars.sh 
```
2. Make sure you now can run Intel C++ compiler
```
icpc -help
```
3. Run make
Compile non-vectorized program
```
make compile-novec
```
Compile vectorized program
```
make compile-vec
```
Compile both version
```
make
```
### Run
From the program root directory
For non-vectorized version:
```
./bin/simd-novec <number of threads> <array size>
```
For vectorized version:
```
./bin/simd-novec <number of threads> <array size>
```

## Benchmark
From the program root directory
```
./bin/benchmark.sh `data-3`
```
This will create 2 benchmarking data `./data/data-3-novec.csv` and ./data/data-3-vec.csv`

