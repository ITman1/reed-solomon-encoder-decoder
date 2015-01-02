Simple Reed Solomon encoder & decoder
==============

Repository contains Makefile for building:

1.  Simple Reed Solomon encoder that encodes given `file` passed as run argument into file named `file.out`. Filename of executable is `bms1A`.
2.  Decoder that accepts also file and decodes it into file named `file.ok`. Filename of executable is `bms1B`.

Parity bits and message length can be set via constants `NPAR` and `MSG_MAXSIZE` defined in `ecc.h`. Encoding is interleaved with given stack size. After one stack is full another interleaving stack interleavead is started. Stack size can be defined in `bms1A.cpp` and `bms1B.cpp` via constant `INTERLEAVING_STACKSIZE`.

# Usage
```
make               compile project - release version
make pack          packs all required files to compile this project    
make clean         clean temp compilers files    
make debug         builds in debug mode    
make release       builds in release mode 
```

# Features
- custom definition of block length
- number of parity bits
- interleaving with custom stack size

## Contact and credits
                             
**Author:**    Radim Loskot  
**gmail.com:** radim.loskot (e-mail)
