/*
 * Colearning in Coevolutionary Algorithms
 * Bc. Michal Wiglasz <xwigla00@stud.fit.vutbr.cz>
 *
 * Master Thesis
 * 2014/2015
 *
 * Supervisor: Ing. Michaela Šikulová <isikulova@fit.vutbr.cz>
 *
 * Faculty of Information Technologies
 * Brno University of Technology
 * http://www.fit.vutbr.cz/
 *
 * Started on 28/07/2014.
 *      _       _
 *   __(.)=   =(.)__
 *   \___)     (___/
 */


/*
    CPU new instruction set check.
    Source: https://software.intel.com/en-us/articles/how-to-detect-new-instruction-support-in-the-4th-generation-intel-core-processor-family
 */


#include "cpu.h"


#if defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 1300)

#include <immintrin.h>

int check_4th_gen_intel_core_features()
{
    const int the_4th_gen_features =
        (_FEATURE_AVX2 | _FEATURE_FMA | _FEATURE_BMI | _FEATURE_LZCNT | _FEATURE_MOVBE);
    return _may_i_use_cpu_feature( the_4th_gen_features );
}

int check_sse4_1()
{
    return _may_i_use_cpu_feature(_FEATURE_SSE4_1);
}

int check_sse2()
{
    return _may_i_use_cpu_feature(_FEATURE_SSE2);
}

#else /* non-Intel compiler */

#include <stdint.h>
#if defined(_MSC_VER)
# include <intrin.h>
#endif

void run_cpuid(uint32_t eax, uint32_t ecx, uint32_t* abcd)
{
    #if defined(_MSC_VER)
        __cpuidex(abcd, eax, ecx);
    #else
        uint32_t ebx = 0, edx;
    # if defined( __i386__ ) && defined ( __PIC__ )
         /* in case of PIC under 32-bit EBX cannot be clobbered */
        __asm__ ( "movl %%ebx, %%edi \n\t cpuid \n\t xchgl %%ebx, %%edi" : "=D" (ebx),
    # else
        #pragma GCC diagnostic ignored "-Wuninitialized"
        __asm__ ( "cpuid" : "+b" (ebx),
    # endif
                  "+a" (eax), "+c" (ecx), "=d" (edx) );
        abcd[0] = eax; abcd[1] = ebx; abcd[2] = ecx; abcd[3] = edx;
    #endif
}

int check_xcr0_ymm()
{
        uint32_t xcr0;
    #if defined(_MSC_VER)
        xcr0 = (uint32_t)_xgetbv(0);  /* min VS2010 SP1 compiler is required */
    #else
        __asm__ ("xgetbv" : "=a" (xcr0) : "c" (0) : "%edx" );
    #endif
        return ((xcr0 & 6) == 6); /* checking if xmm and ymm state are enabled in XCR0 */
}


int check_4th_gen_intel_core_features()
{
    uint32_t abcd[4];
    uint32_t fma_movbe_osxsave_mask = ((1 << 12) | (1 << 22) | (1 << 27));
    uint32_t avx2_bmi12_mask = (1 << 5) | (1 << 3) | (1 << 8);

    /* CPUID.(EAX=01H, ECX=0H):ECX.FMA[bit 12]==1   &&
       CPUID.(EAX=01H, ECX=0H):ECX.MOVBE[bit 22]==1 &&
       CPUID.(EAX=01H, ECX=0H):ECX.OSXSAVE[bit 27]==1 */
    run_cpuid( 1, 0, abcd );
    if ( (abcd[2] & fma_movbe_osxsave_mask) != fma_movbe_osxsave_mask )
        return 0;

    if ( ! check_xcr0_ymm() )
        return 0;

    /*  CPUID.(EAX=07H, ECX=0H):EBX.AVX2[bit 5]==1  &&
        CPUID.(EAX=07H, ECX=0H):EBX.BMI1[bit 3]==1  &&
        CPUID.(EAX=07H, ECX=0H):EBX.BMI2[bit 8]==1  */
    run_cpuid( 7, 0, abcd );
    if ( (abcd[1] & avx2_bmi12_mask) != avx2_bmi12_mask )
        return 0;

    /* CPUID.(EAX=80000001H):ECX.LZCNT[bit 5]==1 */
    run_cpuid( 0x80000001, 0, abcd );
    if ( (abcd[2] & (1 << 5)) == 0)
        return 0;

    return 1;
}

/**
 * Checks whether current CPU supports SSE4.1 instruction set
 *
 * Source:
 *     http://stackoverflow.com/questions/6121792/how-to-check-if-a-cpu-supports-the-sse3-instruction-set
 *     http://stackoverflow.com/a/7495023
 */
int check_sse4_1()
{
    uint32_t abcd[4];
    uint32_t sse4_1_mask = (1 << 19);

    run_cpuid(1, 0, abcd);
    /* CPUID.(EAX=01H, ECX=0H):ECX.SSE41[bit 19]==1 */
    return (abcd[2] & sse4_1_mask) != 0;
}


/**
 * Checks whether current CPU supports SSE2 instruction set
 *
 * Source:
 *     http://stackoverflow.com/questions/6121792/how-to-check-if-a-cpu-supports-the-sse3-instruction-set
 *     http://stackoverflow.com/a/7495023
 */
int check_sse2()
{
    uint32_t abcd[4];
    uint32_t sse2_mask = (1 << 26);

    run_cpuid(1, 0, abcd);
    /* CPUID.(EAX=01H, ECX=0H):EDX.SSE2[bit 26]==1 */
    return (abcd[3] & sse2_mask) != 0;
}

#endif /* non-Intel compiler */


/**
 * Checks whether current CPU supports AVX2 and other New Haswell features
 */
bool can_use_intel_core_4th_gen_features()
{
    static int the_4th_gen_features_available = -1;
    /* test is performed once */
    if (the_4th_gen_features_available < 0 )
        the_4th_gen_features_available = check_4th_gen_intel_core_features();

    return the_4th_gen_features_available;
}


/**
 * Checks whether current CPU supports SSE4.1 instruction set
 */
bool can_use_sse4_1()
{
    return check_sse4_1();
}


/**
 * Checks whether current CPU supports SSE2 instruction set
 */
bool can_use_sse2()
{
    return check_sse2();
}
