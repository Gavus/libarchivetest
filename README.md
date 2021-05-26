# libarchivetest
This is a test to compare the performance of libarchive and openssl speed when using aes-256 encryption.

## Requirements
Cmake, glib, libarchive, openssl

## Build and run
```
mkdir build && cd build
cmake ..
make -j12
./libarchivetest /dev/zero
```

## Generate Flamegraphs
```
perf record --no-buildid-cache -F99 -e cpu-clock --call-graph dwarf --pid $(pidof libarchivetest) -o ./perf.data -- sleep 10
perf script | ~/tools/FlameGraph/stackcollapse-perf.pl | ~/tools/FlameGraph/flamegraph.pl > out.svg
```

## Results on certain device
| type                           | 16 bytes   | 64 bytes   | 256 bytes  | 1024 bytes | 8192 bytes | 16384 bytes |
| ------------------------------ | ---------- | ---------- | ---------- | ---------- | ---------- | ----------- |
| openssl speed -evp aes-256-cbc | 249079.28k | 582640.11k | 798624.60k | 872552.11k | 904836.44k | 902206.81k  |
| openssl speed aes-256-cbc      | 34770.16k  | 56918.59k  | 61765.03k  | 62623.94k  | 62562.30k  | 62746.88k   |
| libarchivetest aes-256         | 5995.79k   | 10005.85k  | 11403.01k  | 15923.54k  | 25859.41k  | 26208.94k   |
| libarchivetest no encryption   | 148724.14k | 176436.07k | 348910.08k | 452351.32k | 491760.30k | 494616.58k  |

## Links
+ https://github.com/libarchive/libarchive
+ https://github.com/openssl/openssl
+ https://github.com/brendangregg/FlameGraph
