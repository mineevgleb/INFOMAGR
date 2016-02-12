#include "precomp.h"

int CounterTypesDesired[MAXCOUNTERS] = 
{
	1,      // core clock cycles (Intel Core 2 and later)
	2,		// ref clock
	9,      // instructions (not P4)
	10,		// instructions
	207,	// branches missed
	311,    // L1 data cache miss
	320		// L2 data cache miss
};

SCounterDefinition CounterDefinitions[] = 
{
    //  id   scheme cpu    countregs eventreg event  mask   name
    {100,  S_P4, PRALL,  4,   7,     0,      9,      7,  "Uops"     }, // uops from any source
    {101,  S_P4, PRALL,  4,   7,     0,      9,      2,  "UopsTC"   }, // uops from trace cache
    {102,  S_P4, PRALL,  4,   7,     0,      9,      1,  "UopsDec"  }, // uops directly from decoder
    {103,  S_P4, PRALL,  4,   7,     0,      9,      4,  "UopsMCode"}, // uops from microcode ROM
    {110,  S_P4, PRALL, 12,  17,     4,      1,      1,  "UopsNB"   }, // uops non-bogus
    {111,  S_P4, PRALL, 12,  17,     4,      2,   0x0c,  "UopsBogus"}, // uops bogus
    {150,  S_P4, PRALL,  8,  11,     1,      4, 0x8000,  "UopsFP"   }, // uops floating point, except move etc.
    {151,  S_P4, PRALL,  8,  11,     1,   0x2e,      8,  "UopsFPMov"}, // uops floating point and SIMD move
    {152,  S_P4, PRALL,  8,  11,     1,   0x2e,   0x10,  "UopsFPLd" }, // uops floating point and SIMD load
    {160,  S_P4, PRALL,  8,  11,     1,      2, 0x8000,  "UopsMMX"  }, // uops 64-bit MMX
    {170,  S_P4, PRALL,  8,  11,     1,   0x1a, 0x8000,  "UopsXMM"  }, // uops 128-bit integer XMM
    {200,  S_P4, PRALL, 12,  17,     5,      6,   0x0f,  "Branch"   }, // branches
    {201,  S_P4, PRALL, 12,  17,     5,      6,   0x0c,  "BrTaken"  }, // branches taken
    {202,  S_P4, PRALL, 12,  17,     5,      6,   0x03,  "BrNTaken" }, // branches not taken
    {203,  S_P4, PRALL, 12,  17,     5,      6,   0x05,  "BrPredict"}, // branches predicted
    {204,  S_P4, PRALL, 12,  17,     4,      3,   0x01,  "BrMispred"}, // branches mispredicted
    {210,  S_P4, PRALL,  4,   7,     2,      5,   0x02,  "CondJMisp"}, // conditional jumps mispredicted
    {211,  S_P4, PRALL,  4,   7,     2,      5,   0x04,  "CallMisp" }, // indirect call mispredicted
    {212,  S_P4, PRALL,  4,   7,     2,      5,   0x08,  "RetMisp"  }, // return mispredicted
    {220,  S_P4, PRALL,  4,   7,     2,      5,   0x10,  "IndirMisp"}, // indirect calls, jumps and returns mispredicted
    {310,  S_P4, PRALL,  0,   3,     0,      3,   0x01,  "TCMiss"   }, // trace cache miss
    {320,  S_P4, PRALL,  0,   3,     7,   0x0c,  0x100,  "Cach2Miss"}, // level 2 cache miss
    {321,  S_P4, PRALL,  0,   3,     7,   0x0c,  0x200,  "Cach3Miss"}, // level 3 cache miss
    {330,  S_P4, PRALL,  0,   3,     3,   0x18,   0x02,  "ITLBMiss" }, // instructions TLB Miss
    {340,  S_P4, PRALL,  0,   3,     2,      3,   0x3a,  "LdReplay" }, // memory load replay
    //  id   scheme cpu    countregs eventreg event  mask   name
    {  9,  S_P1, PRALL,  0,   1,     0,   0x16,        2,  "Instruct" }, // instructions executed
    { 11,  S_P1, PRALL,  0,   1,     0,   0x17,        2,  "InstVpipe"}, // instructions executed in V-pipe
    {202,  S_P1, PRALL,  0,   1,     0,   0x15,        2,  "Flush"    }, // pipeline flush due to branch misprediction or serializing event   
    {310,  S_P1, PRALL,  0,   1,     0,   0x0e,        2,  "CodeMiss" }, // code cache miss
    {311,  S_P1, PRALL,  0,   1,     0,   0x29,        2,  "DataMiss" }, // data cache miss
    //  id   scheme  cpu     countregs eventreg event  mask   name
    {  9, S_P2MC, PRALL,    0,   1,     0,   0xc0,     0,  "Instruct" }, // instructions executed
    { 10, S_P2MC, PRALL,    0,   1,     0,   0xd0,     0,  "IDecode"  }, // instructions decoded
    { 20, S_P2MC, PRALL,    0,   1,     0,   0x80,     0,  "IFetch"   }, // instruction fetches
    { 21, S_P2MC, PRALL,    0,   1,     0,   0x86,     0,  "IFetchStl"}, // instruction fetch stall
    { 22, S_P2MC, PRALL,    0,   1,     0,   0x87,     0,  "ILenStal" }, // instruction length decoder stalls
    {100, S_P2MC, INTEL_PM, 0,   1,     0,   0xc2,     0,  "Uops(F)"  }, // microoperations in fused domain
    {100, S_P2MC, PRALL,    0,   1,     0,   0xc2,     0,  "Uops"     }, // microoperations
    {110, S_P2MC, INTEL_PM, 0,   1,     0,   0xa0,     0,  "Uops(UF)" }, // unfused microoperations submitted to execution units (Undocumented counter!)
    {104, S_P2MC, INTEL_PM, 0,   1,     0,   0xda,     0,  "UopsFused"}, // fused uops
    {115, S_P2MC, INTEL_PM, 0,   1,     0,   0xd3,     0,  "SynchUops"}, // stack synchronization uops
    {121, S_P2MC, PRALL,    0,   1,     0,   0xd2,     0,  "PartRStl" }, // partial register access stall
    {130, S_P2MC, PRALL,    0,   1,     0,   0xa2,     0,  "Rs Stall" }, // all resource stalls
    {201, S_P2MC, PRALL,    0,   1,     0,   0xc9,     0,  "BrTaken"  }, // branches taken
    {204, S_P2MC, PRALL,    0,   1,     0,   0xc5,     0,  "BrMispred"}, // mispredicted branches
    {205, S_P2MC, PRALL,    0,   1,     0,   0xe6,     0,  "BTBMiss"  }, // static branch prediction made
    {310, S_P2MC, PRALL,    0,   1,     0,   0x28,  0x0f,  "CodeMiss" }, // level 2 cache code fetch
    {311, S_P2MC, INTEL_P23,0,   1,     0,   0x29,  0x0f,  "L1D Miss" }, // level 2 cache data fetch
    // Core 2:
    // The first three counters are fixed-function counters having their own register,
    // The rest of the counters are competing for the same two counter registers.
    //  id   scheme cpu      countregs eventreg event  mask   name
    {1,   S_ID2, PRALL,   0x40000001,  0,0,   0,      0,   "Core cyc"  }, // core clock cycles
    {2,   S_ID2, PRALL,   0x40000002,  0,0,   0,      0,   "Ref cyc"   }, // Reference clock cycles
    {9,   S_ID2, PRALL,   0x40000000,  0,0,   0,      0,   "Instruct"  }, // Instructions (reference counter)
    {10,  S_ID2, PRALL,   0,   1,     0,   0xc0,     0x0f, "Instruct"  }, // Instructions
    {11,  S_ID2, PRALL,   0,   1,     0,   0xc0,     0x01, "Read inst" }, // Instructions involving read, fused count as one
    {12,  S_ID2, PRALL,   0,   1,     0,   0xc0,     0x02, "Write ins" }, // Instructions involving write, fused count as one
    {13,  S_ID2, PRALL,   0,   1,     0,   0xc0,     0x04, "NonMem in" }, // Instructions without memory
    {20,  S_ID2, PRALL,   0,   1,     0,   0x80,      0,   "Insfetch"  }, // instruction fetches. < instructions ?
    {21,  S_ID2, PRALL,   0,   1,     0,   0x86,      0,   "IFetchStl" }, // instruction fetch stall
    {22,  S_ID2, PRALL,   0,   1,     0,   0x87,      0,   "ILenStal"  }, // instruction length decoder stalls (length changing prefix)
    {23,  S_ID2, PRALL,   0,   1,     0,   0x83,      0,   "IQue ful"  }, // instruction queue full
    {100, S_ID2, PRALL,   0,   1,     0,   0xc2,     0x0f, "Uops"      }, // uops retired, fused domain
    {101, S_ID2, PRALL,   0,   1,     0,   0xc2,     0x01, "Fused Rd"  }, // fused read uops
    {102, S_ID2, PRALL,   0,   1,     0,   0xc2,     0x02, "Fused Wrt" }, // fused write uops
    {103, S_ID2, PRALL,   0,   1,     0,   0xc2,     0x04, "Macrofus"  }, // macrofused uops
    {104, S_ID2, PRALL,   0,   1,     0,   0xc2,     0x07, "FusedUop"  }, // fused uops, all kinds
    {105, S_ID2, PRALL,   0,   1,     0,   0xc2,     0x08, "NotFusUop" }, // uops, not fused
    {110, S_ID2, PRALL,   0,   1,     0,   0xa0,        0, "Uops UFD"  }, // uops dispatched, unfused domain. Imprecise
    {111, S_ID2, PRALL,   0,   1,     0,   0xa2,        0, "res.stl."  }, // any resource stall
    {115, S_ID2, PRALL,   0,   1,     0,   0xab,     0x01, "SP synch"  }, // Stack synchronization uops
    {116, S_ID2, PRALL,   0,   1,     0,   0xab,     0x02, "SP engine" }, // Stack engine additions
    {121, S_ID2, PRALL,   0,   1,     0,   0xd2,     0x02, "Part.reg"  }, // Partial register synchronization, clock cycles
    {122, S_ID2, PRALL,   0,   1,     0,   0xd2,     0x04, "part.flag" }, // partial flags stall, clock cycles
    {123, S_ID2, PRALL,   0,   1,     0,   0xd2,     0x08, "FP SW stl" }, // floating point status word stall
    {130, S_ID2, PRALL,   0,   1,     0,   0xd2,     0x01, "R Rd stal" }, // ROB register read stall
    {140, S_ID2, PRALL,   0,   1,     0,   0x19,     0x00, "I2FP pass" }, // bypass delay to FP unit from int unit
    {141, S_ID2, PRALL,   0,   1,     0,   0x19,     0x01, "FP2I pass" }, // bypass delay to SIMD/int unit from fp unit (These counters cannot be used simultaneously)
    {150, S_ID2, PRALL,   0,   0,     0,   0xa1,     0x01, "uop p0"    }, // uops port 0. Can only use first counter
    {151, S_ID2, PRALL,   0,   0,     0,   0xa1,     0x02, "uop p1"    }, // uops port 1. Can only use first counter
    {152, S_ID2, PRALL,   0,   0,     0,   0xa1,     0x04, "uop p2"    }, // uops port 2. Can only use first counter
    {153, S_ID2, PRALL,   0,   0,     0,   0xa1,     0x08, "uop p3"    }, // uops port 3. Can only use first counter
    {154, S_ID2, PRALL,   0,   0,     0,   0xa1,     0x10, "uop p4"    }, // uops port 4. Can only use first counter
    {155, S_ID2, PRALL,   0,   0,     0,   0xa1,     0x20, "uop p5"    }, // uops port 5. Can only use first counter
    {201, S_ID2, PRALL,   0,   1,     0,   0xc4,     0x0c, "BrTaken"   }, // branches taken. (Mask: 1=pred.not taken, 2=mispred not taken, 4=pred.taken, 8=mispred taken)
    {204, S_ID2, PRALL,   0,   1,     0,   0xc4,     0x0a, "BrMispred" }, // mispredicted branches
    {205, S_ID2, PRALL,   0,   1,     0,   0xe6,      0,   "BTBMiss"   }, // static branch prediction made
    {210, S_ID2, PRALL,   0,   1,     0,   0x97,      0,   "BranchBu1" }, // branch taken bubble 1
    {211, S_ID2, PRALL,   0,   1,     0,   0x98,      0,   "BranchBu2" }, // branch taken bubble 2 (these two values must be added)
    {310, S_ID2, PRALL,   0,   1,     0,   0x28,     0x0f, "CodeMiss"  }, // level 2 cache code fetch
    {311, S_ID2, PRALL,   0,   1,     0,   0x29,     0x0f, "L1D Miss"  }, // level 2 cache data fetch
    {320, S_ID2, PRALL,   0,   1,     0,   0x24,     0x00, "L2 Miss"   }, // level 2 cache miss
    // Nehalem, Sandy Bridge, Ivy Bridge
    // The first three counters are fixed-function counters having their own register,
    // The rest of the counters are competing for the same counter registers.
    // id   scheme  cpu       countregs eventreg event  mask   name
    {1,   S_ID3,  INTEL_7I,  0x40000001,  0,0,   0,      0,   "Core cyc"   }, // core clock cycles
    {2,   S_ID3,  INTEL_7I,  0x40000002,  0,0,   0,      0,   "Ref cyc"    }, // Reference clock cycles
    {9,   S_ID3,  INTEL_7I,  0x40000000,  0,0,   0,      0,   "Instruct"   }, // Instructions (reference counter)
    {10,  S_ID3,  INTEL_7I,  0,   3,     0,   0xc0,     0x01, "Instruct"   }, // Instructions
    {22,  S_ID3,  INTEL_7I,  0,   3,     0,   0x87,      0,   "ILenStal"   }, // instruction length decoder stalls (length changing prefix)
    {24,  S_ID3,  INTEL_7I,  0,   3,     0,   0xA8,     0x01, "Loop uops"  }, // uops from loop stream detector
    {25,  S_ID3,  INTEL_7I,  0,   3,     0,   0x79,     0x04, "Dec uops"   }, // uops from decoders. (MITE = Micro-instruction Translation Engine)
    {26,  S_ID3,  INTEL_7I,  0,   3,     0,   0x79,     0x08, "Cach uops"  }, // uops from uop cache. (DSB = Decoded Stream Buffer)
    {100, S_ID3,  INTEL_7I,  0,   3,     0,   0xc2,     0x01, "Uops"       }, // uops retired, unfused domain
    {103, S_ID3,  INTEL_7I,  0,   3,     0,   0xc2,     0x04, "Macrofus"   }, // macrofused uops, Sandy Bridge
    {104, S_ID3,  INTEL_7I,  0,   3,     0,   0x0E,     0x01, "Uops F.D."  }, // uops, fused domain, Sandy Bridge
    {105, S_ID3,  INTEL_7,   0,   3,     0,   0x0E,     0x02, "fused uop"  }, // microfused uops 
    {110, S_ID3,  INTEL_7,   0,   3,     0,   0xa0,        0, "Uops UFD?"  }, // uops dispatched, unfused domain. Imprecise, Sandy Bridge
    {111, S_ID3,  INTEL_7I,  0,   3,     0,   0xa2,        1, "res.stl."   }, // any resource stall
    {121, S_ID3,  INTEL_7,   0,   3,     0,   0xd2,     0x02, "Part.reg"   }, // Partial register synchronization, clock cycles, Sandy Bridge
    {122, S_ID3,  INTEL_7,   0,   3,     0,   0xd2,     0x01, "part.flag"  }, // partial flags stall, clock cycles, Sandy Bridge
    {123, S_ID3,  INTEL_7,   0,   3,     0,   0xd2,     0x04, "R Rd stal"  }, // ROB register read stall, Sandy Bridge
    {124, S_ID3,  INTEL_7,   0,   3,     0,   0xd2,     0x0F, "RAT stal"   }, // RAT stall, any, Sandy Bridge
    {150, S_ID3,  INTEL_7,   0,   3,     0,   0xb1,     0x01, "uop p0"     }, // uops port 0.
    {151, S_ID3,  INTEL_7,   0,   3,     0,   0xb1,     0x02, "uop p1"     }, // uops port 1.
    {152, S_ID3,  INTEL_7,   0,   3,     0,   0xb1,     0x04, "uop p2"     }, // uops port 2.
    {153, S_ID3,  INTEL_7,   0,   3,     0,   0xb1,     0x08, "uop p3"     }, // uops port 3.
    {154, S_ID3,  INTEL_7,   0,   3,     0,   0xb1,     0x10, "uop p4"     }, // uops port 4.
    {155, S_ID3,  INTEL_7,   0,   3,     0,   0xb1,     0x20, "uop p5"     }, // uops port 5.
    {156, S_ID3,  INTEL_7,   0,   3,     0,   0xb1,     0x40, "uop p015"   }, // uops port 0,1,5.
    {157, S_ID3,  INTEL_7,   0,   3,     0,   0xb1,     0x80, "uop p234"   }, // uops port 2,3,4.
    {201, S_ID2,  INTEL_IVY, 0,   1,     0,   0xc4,     0x20, "BrTaken"    }, // branches taken. (Mask: 1=pred.not taken, 2=mispred not taken, 4=pred.taken, 8=mispred taken)
    {204, S_ID3,  INTEL_7,   0,   3,     0,   0xc5,     0x0a, "BrMispred"  }, // mispredicted branches
    {207, S_ID3,  INTEL_7I,  0,   3,     0,   0xc5,     0x0,  "BrMispred"  }, // mispredicted branches
    {205, S_ID3,  INTEL_7,   0,   3,     0,   0xe6,      2,   "BTBMiss"    }, // static branch prediction made, Sandy Bridge
    {220, S_ID3,  INTEL_IVY, 0,   3,     0,   0x58,     0x03, "Mov elim"   }, // register moves eliminated
    {221, S_ID3,  INTEL_IVY, 0,   3,     0,   0x58,     0x0C, "Mov elim-"  }, // register moves elimination unsuccessful
    {311, S_ID3,  INTEL_7I,  0,   3,     0,   0x28,     0x0f, "L1D Miss"   }, // level 1 data cache miss
    {312, S_ID3,  INTEL_7,   0,   3,     0,   0x24,     0x0f, "L1 Miss"    }, // level 2 cache requests
    {150, S_ID3,  INTEL_IVY, 0,   3,     0,   0xa1,     0x01, "uop p0"     }, // uops port 0
    {151, S_ID3,  INTEL_IVY, 0,   3,     0,   0xa1,     0x02, "uop p1"     }, // uops port 1
    {152, S_ID3,  INTEL_IVY, 0,   3,     0,   0xa1,     0x0c, "uop p2"     }, // uops port 2
    {153, S_ID3,  INTEL_IVY, 0,   3,     0,   0xa1,     0x30, "uop p3"     }, // uops port 3
    {154, S_ID3,  INTEL_IVY, 0,   3,     0,   0xa1,     0x40, "uop p4"     }, // uops port 4
    {155, S_ID3,  INTEL_IVY, 0,   3,     0,   0xa1,     0x80, "uop p5"     }, // uops port 5
    {160, S_ID3,  INTEL_IVY, 0,   3,     0,   0xa1,     0xFF, "uop p05"    }, // uops port 0-5
    // Haswell
    // The first three counters are fixed-function counters having their own register,
    // The rest of the counters are competing for the same four counter registers.
    // id   scheme  cpu       countregs eventreg event  mask   name
    {1,   S_ID3,  INTEL_HASW, 0x40000001,  0,0,   0,     0,   "Core cyc"   }, // core clock cycles
    {2,   S_ID3,  INTEL_HASW, 0x40000002,  0,0,   0,     0,   "Ref cyc"    }, // Reference clock cycles
    {9,   S_ID3,  INTEL_HASW, 0x40000000,  0,0,   0,     0,   "Instruct"   }, // Instructions (reference counter)
    {10,  S_ID3,  INTEL_HASW, 0,  3,     0,   0xc0,     0x01, "Instruct"   }, // Instructions
    {22,  S_ID3,  INTEL_HASW, 0,  3,     0,   0x87,     0x01, "ILenStal"   }, // instruction length decoder stall due to length changing prefix
    {24,  S_ID3,  INTEL_HASW, 0,  3,     0,   0xA8,     0x01, "Loop uops"  }, // uops from loop stream detector
    {25,  S_ID3,  INTEL_HASW, 0,  3,     0,   0x79,     0x04, "Dec uops"   }, // uops from decoders. (MITE = Micro-instruction Translation Engine)
    {26,  S_ID3,  INTEL_HASW, 0,  3,     0,   0x79,     0x08, "Cach uops"  }, // uops from uop cache. (DSB = Decoded Stream Buffer)
    {100, S_ID3,  INTEL_HASW, 0,  3,     0,   0xc2,     0x01, "Uops"       }, // uops retired, unfused domain
    {104, S_ID3,  INTEL_HASW, 0,  3,     0,   0x0e,     0x01, "uops RAT"   }, // uops from RAT to RS
    {111, S_ID3,  INTEL_HASW, 0,  3,     0,   0xa2,     0x01, "res.stl."   }, // any resource stall
    {131, S_ID3,  INTEL_HASW, 0,  3,     0,   0xC1,     0x18, "AVX trans"  }, // VEX - non-VEX transition penalties
    {201, S_ID3,  INTEL_HASW, 0,  3,     0,   0xC4,     0x20, "BrTaken"    }, // branches taken
    {207, S_ID3,  INTEL_HASW, 0,  3,     0,   0xc5,     0x00, "BrMispred"  }, // mispredicted branches
    {220, S_ID3,  INTEL_HASW, 0,  3,     0,   0x58,     0x03, "Mov elim"   }, // register moves eliminated
    {221, S_ID3,  INTEL_HASW, 0,  3,     0,   0x58,     0x0C, "Mov elim-"  }, // register moves elimination unsuccessful
    {310, S_ID2,  INTEL_HASW, 0,  3,     0,   0x80,     0x02, "CodeMiss"   }, // code cache misses
    {311, S_ID3,  INTEL_HASW, 0,  3,     0,   0x24,     0xe1, "L1D Miss"   }, // level 1 data cache miss
    {320, S_ID3,  INTEL_HASW, 0,  3,     0,   0x24,     0x27, "L2 Miss"    }, // level 2 cache misses
    {150, S_ID3,  INTEL_HASW, 0,  3,     0,   0xa1,     0x01, "uop p0"     }, // uops port 0.
    {151, S_ID3,  INTEL_HASW, 0,  3,     0,   0xa1,     0x02, "uop p1"     }, // uops port 1.
    {152, S_ID3,  INTEL_HASW, 0,  3,     0,   0xa1,     0x04, "uop p2"     }, // uops port 2.
    {153, S_ID3,  INTEL_HASW, 0,  3,     0,   0xa1,     0x08, "uop p3"     }, // uops port 3.
    {154, S_ID3,  INTEL_HASW, 0,  3,     0,   0xa1,     0x10, "uop p4"     }, // uops port 4.
    {155, S_ID3,  INTEL_HASW, 0,  3,     0,   0xa1,     0x20, "uop p5"     }, // uops port 5.
    {156, S_ID3,  INTEL_HASW, 0,  3,     0,   0xa1,     0x40, "uop p6"     }, // uops port 6.
    {157, S_ID3,  INTEL_HASW, 0,  3,     0,   0xa1,     0x80, "uop p7"     }, // uops port 7.
    {160, S_ID3,  INTEL_HASW, 0,  3,     0,   0xa1,     0xFF, "uop p07"    }, // uops port 0-7
    // Intel Atom:
    // The first counter is fixed-function counter having its own register,
    // The rest of the counters are competing for the same two counter registers.
    //  id   scheme  cpu         countregs eventreg event  mask   name
    {9,   S_ID3, INTEL_ATOM,  0x40000000, 0,0,    0,      0,   "Instruct"   }, // Instructions (reference counter)
    {10,  S_ID3, INTEL_ATOM,  0,   1,     0,   0xc0,     0x00, "Instr"      }, // Instructions retired
    {20,  S_ID3, INTEL_ATOM,  0,   1,     0,   0x80,     0x03, "Insfetch"   }, // instruction fetches
    {21,  S_ID3, INTEL_ATOM,  0,   1,     0,   0x80,     0x02, "I miss"     }, // instruction cache miss
    {30,  S_ID3, INTEL_ATOM,  0,   1,     0,   0x40,     0x21, "L1 read"    }, // L1 data cache read
    {31,  S_ID3, INTEL_ATOM,  0,   1,     0,   0x40,     0x22, "L1 write"   }, // L1 data cache write
    {100, S_ID3, INTEL_ATOM,  0,   1,     0,   0xc2,     0x10, "Uops"       }, // uops retired
    {200, S_ID3, INTEL_ATOM,  0,   1,     0,   0xc4,     0x00, "Branch"     }, // branches
    {201, S_ID3, INTEL_ATOM,  0,   1,     0,   0xc4,     0x0c, "BrTaken"    }, // branches taken. (Mask: 1=pred.not taken, 2=mispred not taken, 4=pred.taken, 8=mispred taken)
    {204, S_ID3, INTEL_ATOM,  0,   1,     0,   0xc4,     0x0a, "BrMispred"  }, // mispredicted branches
    {205, S_ID3, INTEL_ATOM,  0,   1,     0,   0xe6,     0x01, "BTBMiss"    }, // Baclear
    {310, S_ID3, INTEL_ATOM,  0,   1,     0,   0x28,     0x4f, "CodeMiss"   }, // level 2 cache code fetch
    {311, S_ID3, INTEL_ATOM,  0,   1,     0,   0x29,     0x4f, "L1D Miss"   }, // level 2 cache data fetch
    {320, S_ID3, INTEL_ATOM,  0,   1,     0,   0x24,     0x00, "L2 Miss"    }, // level 2 cache miss
    {501, S_ID3, INTEL_ATOM,  0,   1,     0,   0xC0,     0x00, "inst re"    }, // instructions retired
    {505, S_ID3, INTEL_ATOM,  0,   1,     0,   0xAA,     0x02, "CISC"       }, // CISC macro instructions decoded
    {506, S_ID3, INTEL_ATOM,  0,   1,     0,   0xAA,     0x03, "decoded"    }, // all instructions decoded
    {601, S_ID3, INTEL_ATOM,  0,   1,     0,   0x02,     0x81, "st.forw"    }, // Successful store forwards
    {640, S_ID3, INTEL_ATOM,  0,   1,     0,   0x12,     0x81, "mul"        }, // Int and FP multiply operations
    {641, S_ID3, INTEL_ATOM,  0,   1,     0,   0x13,     0x81, "div"        }, // Int and FP divide and sqrt operations
    {651, S_ID3, INTEL_ATOM,  0,   1,     0,   0x10,     0x81, "fp uop"     }, // Floating point uops
    // Silvermont
    // The first three counters are fixed-function counters having their own register,
    // The rest of the counters are competing for the same two counter registers.
    //id  scheme  cpu       countregs eventreg event  mask   name
    {1,   S_ID3,  INTEL_SILV, 0x40000001,  0,0,   0,     0,   "Core cyc"   }, // core clock cycles
    {2,   S_ID3,  INTEL_SILV, 0x40000002,  0,0,   0,     0,   "Ref cyc"    }, // Reference clock cycles
    {9,   S_ID3,  INTEL_SILV, 0x40000000,  0,0,   0,     0,   "Instruct"   }, // Instructions (reference counter)
    {100, S_ID3,  INTEL_SILV, 0,  1,     0,   0xc2,     0x10, "Uops"       }, // uops retired
    {103, S_ID3,  INTEL_SILV, 0,  1,     0,   0xc2,     0x01, "Uop micr"   }, // uops from microcode rom
    {111, S_ID3,  INTEL_SILV, 0,  1,     0,   0xc3,     0x08, "stall"      }, // any stall
    {150, S_ID3,  INTEL_SILV, 0,  1,     0,   0xCB,     0x02, "p0i full"   }, // port 0 integer pipe full
    {151, S_ID3,  INTEL_SILV, 0,  1,     0,   0xCB,     0x04, "p1i full"   }, // port 1 integer pipe full
    {152, S_ID3,  INTEL_SILV, 0,  1,     0,   0xCB,     0x08, "p0f full"   }, // port 0 f.p. pipe full
    {153, S_ID3,  INTEL_SILV, 0,  1,     0,   0xCB,     0x10, "p1f full"   }, // port 1 f.p. pipe full
    {158, S_ID3,  INTEL_SILV, 0,  1,     0,   0xCB,     0x01, "mec full"   }, // memory execution cluster pipe full
    {201, S_ID3,  INTEL_SILV, 0,  1,     0,   0xC4,     0xFE, "BrPrTak"    }, // branches predicted taken
    {202, S_ID3,  INTEL_SILV, 0,  1,     0,   0xC5,     0xFE, "BrMprNT"    }, // branches not taken mispredicted 
    {207, S_ID3,  INTEL_SILV, 0,  1,     0,   0xc5,     0x00, "BrMispred"  }, // mispredicted branches
    {209, S_ID3,  INTEL_SILV, 0,  1,     0,   0xe6,     0x01, "BACLEAR"    }, // early prediction corrected by later prediction
    {310, S_ID2,  INTEL_SILV, 0,  1,     0,   0x80,     0x02, "CodeMiss"   }, // code cache misses
    {311, S_ID3,  INTEL_SILV, 0,  1,     0,   0x04,     0x01, "L1D LMis"   }, // level 1 data cache load miss
    {320, S_ID3,  INTEL_SILV, 0,  1,     0,   0x2E,     0x41, "L2 Miss"    }, // level 2 cache misses
    //  id   scheme  cpu         countregs eventreg event  mask   name
    {  9, S_AMD, AMD_ALL,      0,   3,     0,   0xc0,      0,  "Instruct" }, // x86 instructions executed
    {100, S_AMD, AMD_ALL,      0,   3,     0,   0xc1,      0,  "Uops"     }, // microoperations
    {204, S_AMD, AMD_ALL,      0,   3,     0,   0xc3,      0,  "BrMispred"}, // mispredicted branches
    {201, S_AMD, AMD_BULLD,    0,   3,     0,   0xc4,   0x00,  "BrTaken"  }, // branches taken
    {209, S_AMD, AMD_BULLD,    0,   3,     0,   0xc2,   0x00,  "RSBovfl"  }, // return stack buffer overflow
    {310, S_AMD, AMD_ALL,      0,   3,     0,   0x81,      0,  "CodeMiss" }, // instruction cache misses
    {311, S_AMD, AMD_ALL,      0,   3,     0,   0x41,      0,  "L1D Miss" }, // L1 data cache misses
    {320, S_AMD, AMD_ALL,      0,   3,     0,   0x43,   0x1f,  "L2 Miss"  }, // L2 cache misses
    {150, S_AMD, AMD_ATHLON64, 0,   3,     0,   0x00,   0x3f,  "UopsFP"   }, // microoperations in FP pipe
    {151, S_AMD, AMD_ATHLON64, 0,   3,     0,   0x00,   0x09,  "FPADD"    }, // microoperations in FP ADD unit
    {152, S_AMD, AMD_ATHLON64, 0,   3,     0,   0x00,   0x12,  "FPMUL"    }, // microoperations in FP MUL unit
    {153, S_AMD, AMD_ATHLON64, 0,   3,     0,   0x00,   0x24,  "FPMISC"   }, // microoperations in FP Store unit
    {150, S_AMD, AMD_BULLD,    3,   3,     0,   0x00,   0x01,  "UopsFP0"  }, // microoperations in FP pipe 0
    {151, S_AMD, AMD_BULLD,    3,   3,     0,   0x00,   0x02,  "UopsFP1"  }, // microoperations in FP pipe 1
    {152, S_AMD, AMD_BULLD,    3,   3,     0,   0x00,   0x04,  "UopsFP2"  }, // microoperations in FP pipe 2
    {153, S_AMD, AMD_BULLD,    3,   3,     0,   0x00,   0x08,  "UopsFP3"  }, // microoperations in FP pipe 3
    {110, S_AMD, AMD_BULLD,    0,   3,     0,   0x04,   0x0a,  "UopsElim" }, // move eliminations and scalar op optimizations
    {120, S_AMD, AMD_BULLD,    0,   3,     0,   0x2A,   0x01,  "Forwfail" }, // load-to-store forwarding failed
    {160, S_AMD, AMD_BULLD,    0,   3,     0,   0xCB,   0x01,  "x87"      }, // FP x87 instructions
    {161, S_AMD, AMD_BULLD,    0,   3,     0,   0xCB,   0x02,  "MMX"      }, // MMX instructions
    {162, S_AMD, AMD_BULLD,    0,   3,     0,   0xCB,   0x04,  "XMM"      }, // XMM and YMM instructions
    // VIA Nano counters are undocumented
    // These are the ones I have found that counts. Most have unknown purpose
    //  id      scheme cpu    countregs eventreg event  mask   name
    {0x1000, S_VIA, PRALL,   0,   1,     0,   0x000,    0,  "Instr" }, // Instructions
    {0x0001, S_VIA, PRALL,   0,   1,     0,   0x001,    0,  "uops"  }, // micro-ops?
    {0x0002, S_VIA, PRALL,   0,   1,     0,   0x002,    0,  "2"     }, // 
    {0x0003, S_VIA, PRALL,   0,   1,     0,   0x003,    0,  "3"     }, // 
    {0x0004, S_VIA, PRALL,   0,   1,     0,   0x004,    0,  "bubble"}, // Branch bubble clock cycles?
    {0x0005, S_VIA, PRALL,   0,   1,     0,   0x005,    0,  "5"     }, // 
    {0x0006, S_VIA, PRALL,   0,   1,     0,   0x006,    0,  "6"     }, // 
    {0x0007, S_VIA, PRALL,   0,   1,     0,   0x007,    0,  "7"     }, // 
    {0x0008, S_VIA, PRALL,   0,   1,     0,   0x008,    0,  "8"     }, // 
    {0x0009, S_VIA, PRALL,   0,   1,     0,   0x000,    0,  "Instr" }, // Instructions
    {0x0010, S_VIA, PRALL,   0,   1,     0,   0x010,    0,  "10"    }, // 
    {0x0014, S_VIA, PRALL,   0,   1,     0,   0x014,    0,  "14"    }, // 
    {0x0020, S_VIA, PRALL,   0,   1,     0,   0x020,    0,  "Br NT" }, // Branch not taken
    {0x0021, S_VIA, PRALL,   0,   1,     0,   0x021,    0,  "Br NT Pr"}, // Branch not taken, predicted
    {0x0022, S_VIA, PRALL,   0,   1,     0,   0x022,    0,  "Br Tk"   }, // Branch taken
    {0x0023, S_VIA, PRALL,   0,   1,     0,   0x023,    0,  "Br Tk Pr"}, // Branch taken, predicted
    {0x0024, S_VIA, PRALL,   0,   1,     0,   0x024,    0,  "Jmp"    }, // Jump or call
    {0x0025, S_VIA, PRALL,   0,   1,     0,   0x025,    0,  "Jmp"    }, // Jump or call, predicted
    {0x0026, S_VIA, PRALL,   0,   1,     0,   0x026,    0,  "Ind.Jmp"}, // Indirect jump or return
    {0x0027, S_VIA, PRALL,   0,   1,     0,   0x027,    0,  "Ind.J. Pr"}, // Indirect jump or return, predicted
    {0x0034, S_VIA, PRALL,   0,   1,     0,   0x034,    0,  "34"    }, // 
    {0x0040, S_VIA, PRALL,   0,   1,     0,   0x040,    0,  "40"    }, // 
    {0x0041, S_VIA, PRALL,   0,   1,     0,   0x040,    0,  "41"    }, // 
    {0x0042, S_VIA, PRALL,   0,   1,     0,   0x040,    0,  "42"    }, // 
    {0x0043, S_VIA, PRALL,   0,   1,     0,   0x040,    0,  "43"    }, // 
    {0x0044, S_VIA, PRALL,   0,   1,     0,   0x040,    0,  "44"    }, // 
    {0x0046, S_VIA, PRALL,   0,   1,     0,   0x040,    0,  "46"    }, // 
    {0x0048, S_VIA, PRALL,   0,   1,     0,   0x040,    0,  "48"    }, // 
    {0x0082, S_VIA, PRALL,   0,   1,     0,   0x082,    0,  "82"    }, // 
    {0x0083, S_VIA, PRALL,   0,   1,     0,   0x083,    0,  "83"    }, // 
    {0x0084, S_VIA, PRALL,   0,   1,     0,   0x084,    0,  "84"    }, // 
    {0x00B4, S_VIA, PRALL,   0,   1,     0,   0x0B4,    0,  "B4"    }, // 
    {0x00C0, S_VIA, PRALL,   0,   1,     0,   0x0C0,    0,  "C0"    }, // 
    {0x00C4, S_VIA, PRALL,   0,   1,     0,   0x0C4,    0,  "C4"    }, // 
    {0x0104, S_VIA, PRALL,   0,   1,     0,   0x104,    0, "104"    }, // 
    {0x0105, S_VIA, PRALL,   0,   1,     0,   0x105,    0, "105"    }, // 
    {0x0106, S_VIA, PRALL,   0,   1,     0,   0x106,    0, "106"    }, // 
    {0x0107, S_VIA, PRALL,   0,   1,     0,   0x107,    0, "107"    }, // 
    {0x0109, S_VIA, PRALL,   0,   1,     0,   0x109,    0, "109"    }, // 
    {0x010A, S_VIA, PRALL,   0,   1,     0,   0x10A,    0, "10A"    }, // 
    {0x010B, S_VIA, PRALL,   0,   1,     0,   0x10B,    0, "10B"    }, // 
    {0x010C, S_VIA, PRALL,   0,   1,     0,   0x10C,    0, "10C"    }, // 
    {0x0110, S_VIA, PRALL,   0,   1,     0,   0x110,    0, "110"    }, // 
    {0x0111, S_VIA, PRALL,   0,   1,     0,   0x111,    0, "111"    }, // 
    {0x0116, S_VIA, PRALL,   0,   1,     0,   0x116,    0, "116"    }, // 
    {0x0120, S_VIA, PRALL,   0,   1,     0,   0x120,    0, "120"    }, // 
    {0x0121, S_VIA, PRALL,   0,   1,     0,   0x121,    0, "121"    }, // 
    {0x013C, S_VIA, PRALL,   0,   1,     0,   0x13C,    0, "13C"    }, // 
    {0x0200, S_VIA, PRALL,   0,   1,     0,   0x200,    0, "200"    }, // 
    {0x0201, S_VIA, PRALL,   0,   1,     0,   0x201,    0, "201"    }, // 
    {0x0206, S_VIA, PRALL,   0,   1,     0,   0x206,    0, "206"    }, // 
    {0x0207, S_VIA, PRALL,   0,   1,     0,   0x207,    0, "207"    }, // 
    {0x0301, S_VIA, PRALL,   0,   1,     0,   0x301,    0, "301"    }, // 
    {0x0302, S_VIA, PRALL,   0,   1,     0,   0x302,    0, "302"    }, // 
    {0x0303, S_VIA, PRALL,   0,   1,     0,   0x303,    0, "303"    }, // 
    {0x0304, S_VIA, PRALL,   0,   1,     0,   0x304,    0, "304"    }, // 
    {0x0305, S_VIA, PRALL,   0,   1,     0,   0x305,    0, "305"    }, // 
    {0x0306, S_VIA, PRALL,   0,   1,     0,   0x306,    0, "306"    }, // 
    {0x0502, S_VIA, PRALL,   0,   1,     0,   0x502,    0, "502"    }, // 
    {0x0507, S_VIA, PRALL,   0,   1,     0,   0x507,    0, "507"    }, // 
    {0x0508, S_VIA, PRALL,   0,   1,     0,   0x508,    0, "508"    }, // 
    {0x050D, S_VIA, PRALL,   0,   1,     0,   0x50D,    0, "50D"    }, // 
    {0x0600, S_VIA, PRALL,   0,   1,     0,   0x600,    0, "600"    }, // 
    {0x0605, S_VIA, PRALL,   0,   1,     0,   0x605,    0, "605"    }, // 
    {0x0607, S_VIA, PRALL,   0,   1,     0,   0x607,    0, "607"    }, // 
    //  end of list   
    {0, S_UNKNOWN, PRUNKNOWN, 0,  0,     0,      0,     0,    0     }  // list must end with a record of all 0
};

#define CACHELINESIZE  64
ALIGNEDSTRUCTURE( SThreadData, CACHELINESIZE ) 
{
    int CountTemp[MAXCOUNTERS + 1];				// temporary storage of clock counts and PMC counts
    int CountOverhead[MAXCOUNTERS + 1];			// temporary storage of count overhead
    int ClockResults;							// clock count results
    int PMCResults[MAXCOUNTERS];				// PMC count results
};

static SThreadData ThreadData;					// Results for all threads
static int NumCounters = 0;						// Number of valid PMC counters in Counters[]
static int* PThreadData = (int*)&ThreadData;	// Pointer to measured data for all threads
// offset of PMC results of first thread into ThreadData (bytes)
static int PMCResultsOS = int(ThreadData.PMCResults - ThreadData.CountTemp) * sizeof(int);
// counter register numbers used
static int Counters[MAXCOUNTERS] = { 0 };
static int EventRegistersUsed[MAXCOUNTERS] = { 0 };
static CCounters MSRCounters;					// CCounters instance

void InitPerformanceCounters()
{
	int procthreads, i, j;
	SyS::ProcMaskType ProcessAffMask = SyS::GetProcessMask();
	for( procthreads = i = 0; i < MAXTHREADS; i++ )
	if (SyS::TestProcessMask(i, &ProcessAffMask)) procthreads++;
	MSRCounters.LockProcessor();
	MSRCounters.QueueCounters();
	MSRCounters.StartDriver();
	SyS::SetProcessPriorityHigh();
	SyS::SetProcessMask( 0 );
	MSRCounters.StartCounters( 0 );
	SyS::Sleep0();
	// measure overhead
	for( i = 0; i < MAXCOUNTERS + 1; i++ ) ThreadData.CountOverhead[i] = 0x7FFFFFFF;
	for( j = 0; j < 512; j++ ) 
	{
		Serialize();
		for( i = 0; i < MAXCOUNTERS; i++ ) ThreadData.CountTemp[i+1] = (int)Readpmc(Counters[i]);
		Serialize();
		ThreadData.CountTemp[0] = (int)Readtsc();
		Serialize();
		Serialize();
		ThreadData.CountTemp[0] -= (int)Readtsc();
		Serialize();
		for( i = 0; i < MAXCOUNTERS; i++ ) ThreadData.CountTemp[i+1] -= (int)Readpmc(Counters[i]);
		Serialize();
		for( i = 0; i < MAXCOUNTERS+1; i++ ) if (-ThreadData.CountTemp[i] < ThreadData.CountOverhead[i]) 
			ThreadData.CountOverhead[i] = -ThreadData.CountTemp[i];
	}
	MSRCounters.PrintCPUInfo();
}

void StartMeasurement()
{
	SyS::SetProcessMask( 0 );
	SyS::Sleep0();
    Serialize();
    for( int i = 0; i < MAXCOUNTERS; i++ ) ThreadData.CountTemp[i + 1] = (int)Readpmc(Counters[i]);
    Serialize();
    ThreadData.CountTemp[0] = (int)Readtsc();
    Serialize();
}

void StopMeasurement()
{
    Serialize();
    ThreadData.CountTemp[0] -= (int)Readtsc();
    Serialize();
    for( int i = 0; i < MAXCOUNTERS; i++ ) ThreadData.CountTemp[i+1] -= (int)Readpmc(Counters[i]);
    Serialize();
    ThreadData.ClockResults = -ThreadData.CountTemp[0] - ThreadData.CountOverhead[0];
    for( int i = 0; i < MAXCOUNTERS; i++ )
        ThreadData.PMCResults[i] = -ThreadData.CountTemp[i+1] - ThreadData.CountOverhead[i+1];
	SyS::Sleep0();
}

int GetNrCounters() { return NumCounters; }
char* GetCounterName( int idx ) { return MSRCounters.CounterNames[idx]; }
int GetCounterValue( int idx ) { return PThreadData[idx + PMCResultsOS / sizeof( int )]; }

//////////////////////////////////////////////////////////////////////////////
//        CMSRInOutQue class member functions
//////////////////////////////////////////////////////////////////////////////

// Constructor
CMSRInOutQue::CMSRInOutQue() 
{
	n = 0;
	for (int i = 0; i < MAX_QUE_ENTRIES+1; i++) queue[i].msr_command = MSR_STOP;
}

// Put data record in queue
int CMSRInOutQue::put (EMSR_COMMAND msr_command, unsigned int register_number, unsigned int value_lo, unsigned int value_hi) 
{
	if (n >= MAX_QUE_ENTRIES) return -10;
	queue[n].msr_command = msr_command;
	queue[n].register_number = register_number;
	queue[n].val[0] = value_lo;
	queue[n++].val[1] = value_hi;
	return 0;
}

//////////////////////////////////////////////////////////////////////////////
//        CCounters class member functions
//////////////////////////////////////////////////////////////////////////////

CCounters::CCounters() : MVendor( VENDOR_UNKNOWN ), MFamily( PRUNKNOWN ), MScheme( S_UNKNOWN ), NumPMCs( 0 ), NumFixedPMCs( 0 )
{
	ProcessorNumber = 0;
	for (int i = 0; i < MAXCOUNTERS; i++) CounterNames[i] = 0;
}

void CCounters::QueueCounters() 
{
	int n = 0; 
	while (CounterDefinitions[n].ProcessorFamily || CounterDefinitions[n].CounterType) n++;
	NumCounterDefinitions = n;
	GetProcessorVendor();               // get microprocessor vendor
	GetProcessorFamily();               // get microprocessor family
	GetPMCScheme();                     // get PMC scheme
	for( int i = 0; i < MAXCOUNTERS; i++ ) 
	{
		const char* err = DefineCounter( CounterTypesDesired[i] );
		if (err) printf( "\nCannot make counter %i. %s\n", i+1, err );
	}  
}

void CCounters::LockProcessor() 
{
	queue1[0].put( PROC_SET, 0, 0 );
	queue2[0].put( PROC_SET, 0, 0 );
	queue1[0].put( PMC_ENABLE, 0, 0 );
	queue2[0].put( PMC_DISABLE, 0, 0 );
}

int CCounters::StartDriver() 
{
	return msr.LoadDriver();
}

void CCounters::CleanUp() 
{
	// msr.UnloadDriver();
	// msr.UnInstallDriver();
}

// put record into multiple start queues
void CCounters::Put1( int num_threads, EMSR_COMMAND msr_command, unsigned int register_number, unsigned int value_lo, unsigned int value_hi ) 
{
	for( int t = 0; t < num_threads; t++) queue1[t].put(msr_command, register_number, value_lo, value_hi );
}

// put record into multiple stop queues
void CCounters::Put2( int num_threads, EMSR_COMMAND msr_command, unsigned int register_number, unsigned int value_lo, unsigned int value_hi ) 
{
	for( int t = 0; t < num_threads; t++) queue2[t].put(msr_command, register_number, value_lo, value_hi );
}

// Start counting
void CCounters::StartCounters( int ThreadNum ) 
{
	msr.AccessRegisters( queue1[ThreadNum] );
}

// Stop and reset counters
void CCounters::StopCounters(int ThreadNum) 
{
	msr.AccessRegisters( queue2[ThreadNum] );
}

void CCounters::PrintCPUInfo()
{
	int CpuIdOutput[4];
	int Family, Model;
	MFamily = PRUNKNOWN;
	Cpuid(CpuIdOutput, 0);
	if (CpuIdOutput[0] == 0) return;
	Cpuid(CpuIdOutput, 1);
	Family = ((CpuIdOutput[0] >> 8) & 0x0F) + ((CpuIdOutput[0] >> 20) & 0xFF);   // family code
	Model  = ((CpuIdOutput[0] >> 4) & 0x0F) | ((CpuIdOutput[0] >> 12) & 0xF0);   // model code
	printf( "processor: " );
	if (MVendor == INTEL) printf( "INTEL" );
	else if (MVendor == AMD) printf( "AMD" );
	else if (MVendor == VIA) printf( "VIA" );
	else printf( "UNKNOWN" );
	printf( " (" );
	if (MVendor == INTEL)  
	{
		if (Family == 5) printf( "Pentium 1" );
		else if (Family == 0x0F) printf( "Pentium 4" );
		else if (Family == 6) 
		{
			switch(Model) 
			{
			case 0x09:  case 0x0D: printf( "Pentium M" ); break;
			case 0x0E: printf( "Core 1" ); break;
			case 0x0F: case 0x16: printf( "Core 2, 65 nm" ); break;
			case 0x17: case 0x1D: printf( "Core 2, 45 nm" ); break;
			case 0x1A: case 0x1E: case 0x1F: case 0x2E: printf( "Nehalem" ); break;
			case 0x25: case 0x2C: case 0x2F: printf( "Westmere" ); break;
			case 0x2A: case 0x2D: printf( "Sandy Bridge" ); break;
			case 0x3A: case 0x3E: printf( "Ivy Bridge" ); break;
			case 0x3C: case 0x3F: case 0x45: case 0x46: printf( "Haswell" ); break;
			case 0x3D: case 0x4F: case 0x56: printf( "Broadwell" ); break;
			case 0x1C: case 0x26: case 0x27: case 0x35: case 0x36: printf( "Atom" ); break;
			case 0x37: case 0x4A: case 0x4D: printf( "Silvermont" ); break;
			default:
			if (Model >= 0x3C) printf( "Haswell" ); else printf( "Pentium 2 or 3" );
			}
		}
	}
	else if (MVendor == AMD)  
	{
		if (Family == 6) printf( "Athlon" );
		else if (Family >= 0x0F && Family <= 0x14) printf( "Athlon 64 or Opteron" );
		else if (Family >= 0x15) printf( "Family 15h" );
	}
	else if (MVendor == VIA) printf( "Nano" );
	printf( ")\n" );
}

void CCounters::GetProcessorVendor() 
{
	int CpuIdOutput[4];
	Cpuid( CpuIdOutput, 0 );
	MVendor = VENDOR_UNKNOWN;
	if (CpuIdOutput[2] == 0x6C65746E) MVendor = INTEL;  // Intel "GenuineIntel"
	if (CpuIdOutput[2] == 0x444D4163) MVendor = AMD;    // AMD   "AuthenticAMD"
	if (CpuIdOutput[1] == 0x746E6543) MVendor = VIA;    // VIA   "CentaurHauls"
}

void CCounters::GetProcessorFamily() 
{
    int CpuIdOutput[4];
    int Family, Model;
    MFamily = PRUNKNOWN;                 // default = unknown
    Cpuid(CpuIdOutput, 0);
    if (CpuIdOutput[0] == 0) return;     // cpuid function 1 not supported
    Cpuid(CpuIdOutput, 1);
    Family = ((CpuIdOutput[0] >> 8) & 0x0F) + ((CpuIdOutput[0] >> 20) & 0xFF);   // family code
    Model  = ((CpuIdOutput[0] >> 4) & 0x0F) | ((CpuIdOutput[0] >> 12) & 0xF0);   // model code
    if (MVendor == INTEL)  
	{
        if (Family <  5)    MFamily = PRUNKNOWN;           // less than Pentium
        if (Family == 5)    MFamily = INTEL_P1MMX;         // pentium 1 or mmx
        if (Family == 0x0F) MFamily = INTEL_P4;            // pentium 4 or other netburst
        if (Family == 6) 
		{
            switch(Model) 
			{  // list of known Intel families with different performance monitoring event tables
            case 0x09:  case 0x0D:
                MFamily = INTEL_PM;  break;                // Pentium M
            case 0x0E: 
                MFamily = INTEL_CORE;  break;              // Core 1
            case 0x0F: case 0x16: 
                MFamily = INTEL_CORE2;  break;             // Core 2, 65 nm
            case 0x17: case 0x1D:
                MFamily = INTEL_CORE2;  break;             // Core 2, 45 nm
            case 0x1A: case 0x1E: case 0x1F: case 0x2E:
                MFamily = INTEL_7;  break;                 // Nehalem
            case 0x25: case 0x2C: case 0x2F:
                MFamily = INTEL_7;  break;                 // Westmere
            case 0x2A: case 0x2D:
                MFamily = INTEL_IVY;  break;               // Sandy Bridge
            case 0x3A: case 0x3E:
                MFamily = INTEL_IVY;  break;               // Ivy Bridge
            case 0x3C: case 0x3F: case 0x45: case 0x46:
                MFamily = INTEL_HASW;  break;              // Haswell
            case 0x3D: case 0x4F: case 0x56:
                MFamily = INTEL_HASW;  break;              // Broadwell
            // low power processors:
            case 0x1C: case 0x26: case 0x27: case 0x35: case 0x36:
                MFamily = INTEL_ATOM;  break;              // Atom
            case 0x37: case 0x4A: case 0x4D:
                MFamily = INTEL_SILV;  break;              // Silvermont
            default:
                MFamily = INTEL_P23;                       // Pentium 2 or 3
                if (Model >= 0x3C) MFamily = INTEL_HASW;   // Haswell
            }
        }
    }
    if (MVendor == AMD)  
	{
        MFamily = PRUNKNOWN;                               // old or unknown AMD
        if (Family == 6)    MFamily = AMD_ATHLON;          // AMD Athlon
        if (Family >= 0x0F && Family <= 0x14) MFamily = AMD_ATHLON64; // Athlon 64, Opteron, etc
        if (Family >= 0x15) MFamily = AMD_BULLD;           // Family 15h
    }
    if (MVendor == VIA)  
        if (Family == 6 && Model >= 0x0F) MFamily = VIA_NANO; // VIA Nano
}

void CCounters::GetPMCScheme() 
{
	MScheme = S_UNKNOWN;
	NumPMCs = 2;
	NumFixedPMCs = 0;
	if (MVendor == AMD) MScheme = S_AMD, NumPMCs = 4;
	if (MVendor == VIA) MScheme = S_VIA;
	if (MVendor == INTEL)  
	{
		int CpuIdOutput[4];
		Cpuid(CpuIdOutput, 0);
		if (CpuIdOutput[0] >= 0x0A) 
		{
			Cpuid(CpuIdOutput, 0x0A);
			if (CpuIdOutput[0] & 0xFF) 
			{
				MScheme = EPMCScheme(S_ID1 << ((CpuIdOutput[0] & 0xFF) - 1));
				NumPMCs = (CpuIdOutput[0] >> 8) & 0xFF;
				NumFixedPMCs = CpuIdOutput[3] & 0x1F;
			}
		}
		if (MScheme == S_UNKNOWN) 
		{
			switch (MFamily) 
			{
			case INTEL_P1MMX: MScheme = S_P1; break;
			case INTEL_P23: case INTEL_PM: MScheme = S_P2; break;
			case INTEL_P4: MScheme = S_P4; break;
			case INTEL_CORE: MScheme = S_ID1; break;
			case INTEL_CORE2: MScheme = S_ID2; break;
			case INTEL_7: case INTEL_ATOM: case INTEL_SILV: MScheme = S_ID3; break;
			default: break;
			}
		}
	}
}

const char* CCounters::DefineCounter( int CounterType ) 
{
	if (CounterType == 0) return NULL;
	int i;
	SCounterDefinition * p;
	// Search for matching counter definition
	for( i = 0, p = CounterDefinitions; i < NumCounterDefinitions; i++, p++ ) 
		if (p->CounterType == CounterType && (p->PMCScheme & MScheme) && (p->ProcessorFamily & MFamily))
			break;
	if (i >= NumCounterDefinitions) return "No matching counter definition found"; // not found in list
	return DefineCounter( *p );
}

const char* CCounters::DefineCounter( SCounterDefinition& CDef ) 
{
    int i, counternr, a, b, reg, eventreg, tag;
    static int CountersEnabled = 0, FixedCountersEnabled = 0;
    if (!(CDef.ProcessorFamily & MFamily)) return "Counter not defined for present microprocessor family";
    if (NumCounters >= 8) return "Too many counters";
    if (CDef.CounterFirst & 0x40000000) counternr = CDef.CounterFirst; else 
	{
        // check CounterLast
        if (CDef.CounterLast < CDef.CounterFirst) CDef.CounterLast = CDef.CounterFirst;
        if (CDef.CounterLast >= NumPMCs && (MScheme & S_INTL)) {}   
		// Find vacant counter
        for (counternr = CDef.CounterFirst; counternr <= CDef.CounterLast; counternr++) 
		{
            // Check if this counter register is already in use
            for( i = 0; i < NumCounters; i++ ) if (counternr == Counters[i]) goto USED;
            if (MFamily == INTEL_P4) 
			{
                // Check if the corresponding event register ESCR is already in use
                eventreg = GetP4EventSelectRegAddress( counternr, CDef.EventSelectReg ); 
                for( i = 0; i < NumCounters; i++ ) if (EventRegistersUsed[i] == eventreg) goto USED;
            }
            // Vacant counter found. stop searching
            break;
        USED:; // This counter is occupied. keep searching
        }
        if (counternr > CDef.CounterLast) return "Counter registers are already in use";
    }
    // Vacant counter found. Save name   
    CounterNames[NumCounters] = CDef.Description;
    // Put MSR commands for this counter in queues
    switch (MScheme) 
	{
    case S_P1:
        // Pentium 1 and Pentium MMX
        a = CDef.Event | (CDef.EventMask << 6);
        if (counternr == 1) a = EventRegistersUsed[0] | (a << 16);
        Put1( 1, MSR_WRITE, 0x11, a );
        Put2( 1, MSR_WRITE, 0x11, 0 );
        Put1( 1, MSR_WRITE, 0x12 + counternr, 0 );
        Put2( 1, MSR_WRITE, 0x12 + counternr, 0 );
        EventRegistersUsed[0] = a;
        break;
    case S_ID2: 
	case S_ID3:
        // Intel Core 2 and later
        if (counternr & 0x40000000) 
		{
            // This is a fixed function counter
            if (!(FixedCountersEnabled++)) 
			{
                // Enable fixed function counters
                for (a = i = 0; i < NumFixedPMCs; i++) 
				{
                    b = 2;  // 1=privileged level, 2=user level, 4=any thread
                    a |= b << (4*i);
                }
                // Set MSR_PERF_FIXED_CTR_CTRL
                Put1( 1, MSR_WRITE, 0x38D, a ); 
                Put2( 1, MSR_WRITE, 0x38D, 0 );
            }
            break;
        }
        if (!(CountersEnabled++)) 
		{
            // Enable counters
            a = (1 << NumPMCs) - 1;      // one bit for each pmc counter
            b = (1 << NumFixedPMCs) - 1; // one bit for each fixed counter
            // set MSR_PERF_GLOBAL_CTRL
            Put1( 1, MSR_WRITE, 0x38F, a, b );
            Put2( 1, MSR_WRITE, 0x38F, 0 );
        }
        // All other counters continue in next case:
    case S_P2: 
	case S_ID1:
        // Pentium Pro, Pentium II, Pentium III, Pentium M, Core 1, (Core 2 continued):
        a = CDef.Event | (CDef.EventMask << 8) | (1 << 16) | (1 << 22);
        if (MScheme == S_ID1) a |= (1 << 14);  // Means this core only
        eventreg = 0x186 + counternr;             // IA32_PERFEVTSEL0,1,..
        reg = 0xc1 + counternr;                   // IA32_PMC0,1,..
        Put1( 1, MSR_WRITE, eventreg, a );
        Put2( 1, MSR_WRITE, eventreg, 0 );
        Put1( 1, MSR_WRITE, reg, 0 );
        Put2( 1, MSR_WRITE, reg, 0 );
        break;
    case S_P4:
        // Pentium 4 and Pentium 4 with EM64T
        // ESCR register
        eventreg = GetP4EventSelectRegAddress(counternr, CDef.EventSelectReg); 
        tag = 1;
        a = 0x1C | (tag << 5) | (CDef.EventMask << 9) | (CDef.Event << 25);
        Put1( 1, MSR_WRITE, eventreg, a );
        Put2( 1, MSR_WRITE, eventreg, 0 );
        // Remember this event register is used
        EventRegistersUsed[NumCounters] = eventreg;
        // CCCR register
        reg = counternr + 0x360;
        a = (1 << 12) | (3 << 16) | (CDef.EventSelectReg << 13);
        Put1( 1, MSR_WRITE, reg, a );
        Put2( 1, MSR_WRITE, reg, 0 );
        // Reset counter register
        reg = counternr + 0x300;
        Put1( 1, MSR_WRITE, reg, 0 );
        Put2( 1, MSR_WRITE, reg, 0 );
        // Set high bit for fast readpmc
        counternr |= 0x80000000;
        break;
    case S_AMD:
        // AMD Athlon, Athlon 64, Opteron
        a = CDef.Event | (CDef.EventMask << 8) | (1 << 16) | (1 << 22);
        eventreg = 0xc0010000 + counternr;
        reg = 0xc0010004 + counternr;
        Put1( 1, MSR_WRITE, eventreg, a );
        Put2( 1, MSR_WRITE, eventreg, 0 );
        Put1( 1, MSR_WRITE, reg, 0 );
        Put2( 1, MSR_WRITE, reg, 0 );
        break;
    case S_VIA:
        // VIA Nano. Undocumented!
        a = CDef.Event | (1 << 16) | (1 << 22);
        eventreg = 0x186 + counternr;
        reg = 0xc1 + counternr;
        Put1( 1, MSR_WRITE, eventreg, a );
        Put2( 1, MSR_WRITE, eventreg, 0 );
        Put1( 1, MSR_WRITE, reg, 0 );
        Put2( 1, MSR_WRITE, reg, 0 );
        break;
    default:
        return "No counters defined for present microprocessor family";
    }
    Counters[NumCounters++] = counternr;
    return NULL; // NULL = success
}

int CCounters::GetP4EventSelectRegAddress( int CounterNr, int EventSelectNo )
{
	static int TranslationTables[4][8] = 
	{
		{0x3B2, 0x3B4, 0x3AA, 0x3B6, 0x3AC, 0x3C8, 0x3A2, 0x3A0}, // counter 0-3
		{0x3C0, 0x3C4, 0x3C2,    -1,    -1,    -1,    -1,    -1}, // counter 4-7
		{0x3A6, 0x3A4, 0x3AE, 0x3B0,    -1, 0x3A8,    -1,    -1}, // counter 8-11
		{0x3BA, 0x3CA, 0x3BC, 0x3BE, 0x3B8, 0x3CC, 0x3E0,    -1}  // counter 12-17
	};
	unsigned int n = CounterNr;
	if (n > 17) return -1;
	if (n > 15) n -= 3;
	if ((unsigned int)EventSelectNo > 7) return -1;
	int a = TranslationTables[n/4][EventSelectNo];
	if (a < 0) return a;
	if (n & 2) a++;
	return a;
}