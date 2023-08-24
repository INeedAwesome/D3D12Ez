/*
	Root Signature layout CPU
	 - 0: float3 "Color"


	Root Signature layout GPU
	 - b0: float3 "Color"

*/

#define ROOTSIG \
"RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT)," \
"RootConstants(num32BitConstants=3, b0)"