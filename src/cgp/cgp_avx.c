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


#include <stdlib.h>
#include <assert.h>
#include <immintrin.h>

#include "cgp_avx.h"
#include "../random.h"


#define CURRENT(x) \
  (x == 0? current0 \
    : x == 1? current1 \
    : x == 2? current2 \
    : current3)

#define ASSIGN_CURRENT(x, val) do { \
  if (x == 0) current0 = (val); \
  else if (x == 1) current1 = (val); \
  else if (x == 2) current2 = (val); \
  else if (x == 3) current3 = (val); \
  else assert(false); \
} while(0);

#define LOAD_INPUT(reg, idx) do { \
    if ((idx) < CGP_INPUTS) reg = inputs[(idx)]; \
    else if ((idx) == CGP_INPUTS + offset) reg = prev0; \
    else if ((idx) == CGP_INPUTS + offset + 1) reg = prev1; \
    else if ((idx) == CGP_INPUTS + offset + 2) reg = prev2; \
    else if ((idx) == CGP_INPUTS + offset + 3) reg = prev3; \
    else { \
        fprintf(stderr, "Invalid index %d, (range %d-%d)\n", \
            idx, CGP_INPUTS + offset, CGP_INPUTS + offset + 3); \
        assert(false); \
    } \
} while(0);

#define UCFMT1 "%u"
#define UCFMT4 UCFMT1 ", " UCFMT1 ", " UCFMT1 ", " UCFMT1
#define UCFMT16 UCFMT4 ", " UCFMT4 ", " UCFMT4 ", " UCFMT4
#define UCFMT32 UCFMT16 ", " UCFMT16

#define UCVAL1(n) _tmp[n]
#define UCVAL4(n) UCVAL1(n), UCVAL1(n+1), UCVAL1(n+2), UCVAL1(n+3)
#define UCVAL16(n) UCVAL4(n), UCVAL4(n+4), UCVAL4(n+8), UCVAL4(n+12)
#define UCVAL32(n) UCVAL16(n), UCVAL16(n+16)

#define PRINT_REG(reg) do { \
    __m256i _tmpval = reg; \
    unsigned char *_tmp = (unsigned char*) &_tmpval; \
    printf(UCFMT32 "\n", UCVAL32(0)); \
} while(0);


/**
 * Calculate output of given chromosome and inputs using AVX2 instructions
 * @param chr
 * @param inputs
 * @param outputs
 */
void cgp_get_output_avx(ga_chr_t chromosome,
    __m256i_aligned inputs[CGP_INPUTS], __m256i_aligned outputs[CGP_OUTPUTS])
{
#ifndef AVX2
    assert(false);
#else
    assert(CGP_OUTPUTS == 1);
    assert(CGP_ROWS == 4);
    assert(CGP_LBACK == 1);

    // previous and currently computed column
    register __m256i prev0, prev1, prev2, prev3;
    register __m256i current0, current1, current2, current3;

    // 0xFF constant
    static __m256i_aligned FF;
    FF = _mm256_set1_epi8(0xFF);

    cgp_genome_t genome = (cgp_genome_t) chromosome->genome;

#ifdef TEST_EVAL_AVX

    for (int i = 0; i < CGP_INPUTS; i++) {
        unsigned char *_tmp = (unsigned char*) &inputs[i];
        printf("I: %2d = " UCFMT32 "\n", i, UCVAL32(0));
    }
#endif

    int offset = -CGP_ROWS;

    for (int x = 0; x < CGP_COLS; x++) {
        for (int y = 0; y < CGP_ROWS; y++) {
            int idx = cgp_node_index(x, y);
            cgp_node_t *n = &(genome->nodes[idx]);

            // skip inactive blocks
            if (!n->is_active) continue;

            register __m256i A;
            register __m256i B;
            register __m256i Y;
            register __m256i TMP;
            register __m256i mask;

            LOAD_INPUT(A, n->inputs[0]);
            LOAD_INPUT(B, n->inputs[1]);

            switch (n->function) {
                case c255:
                    Y = FF;
                    break;

                case identity:
                    Y = A;
                    break;

                case inversion:
                    Y = _mm256_sub_epi8(FF, A);
                    break;

                case b_or:
                    Y = _mm256_or_si256(A, B);
                    break;

                case b_not1or2:
                    // we don't have NOT instruction, we need to XOR with FF
                    Y = _mm256_xor_si256(FF, A);
                    Y = _mm256_or_si256(Y, B);
                    break;

                case b_and:
                    Y = _mm256_and_si256(A, B);
                    break;

                case b_nand:
                    Y = _mm256_and_si256(A, B);
                    Y = _mm256_xor_si256(FF, Y);
                    break;

                case b_xor:
                    Y = _mm256_xor_si256(A, B);
                    break;

                case rshift1:
                    // no SR instruction for 8bit data, we need to shift
                    // 16 bits and apply mask
                    // IN : [ 1 2 3 4 5 6 7 8 | A B C D E F G H]
                    // SHR: [ 0 1 2 3 4 5 6 7 | 8 A B C D E F G]
                    // MSK: [ 0 1 2 3 4 5 6 7 | 0 A B C D E F G]
                    mask = _mm256_set1_epi8(0x7F);
                    Y = _mm256_srli_epi16(A, 1);
                    Y = _mm256_and_si256(Y, mask);
                    break;

                case rshift2:
                    // similar to rshift1
                    // IN : [ 1 2 3 4 5 6 7 8 | A B C D E F G H]
                    // SHR: [ 0 0 1 2 3 4 5 6 | 7 8 A B C D E F]
                    // MSK: [ 0 0 1 2 3 4 5 6 | 0 0 A B C D E F]
                    mask = _mm256_set1_epi8(0x3F);
                    Y = _mm256_srli_epi16(A, 2);
                    Y = _mm256_and_si256(Y, mask);
                    break;

                case swap:
                    // SWAP(A, B) (((A & 0x0F) << 4) | ((B & 0x0F)))
                    // Shift A left by 4 bits
                    // IN : [ 1 2 3 4 5 6 7 8 | A B C D E F G H]
                    // SHL: [ 5 6 7 8 A B C D | E F G H 0 0 0 0]
                    // MSK: [ 5 6 7 8 0 0 0 0 | E F G H 0 0 0 0]
                    mask = _mm256_set1_epi8(0xF0);
                    TMP = _mm256_slli_epi16(A, 4);
                    TMP = _mm256_and_si256(TMP, mask);

                    // Mask B
                    // IN : [ 1 2 3 4 5 6 7 8 | A B C D E F G H]
                    // MSK: [ 0 0 0 0 5 6 7 8 | 0 0 0 0 E F G H]
                    mask = _mm256_set1_epi8(0x0F);
                    Y = _mm256_and_si256(B, mask);

                    // Combine
                    Y = _mm256_or_si256(Y, TMP);
                    break;

                case add:
                    Y = _mm256_add_epi8(A, B);
                    break;

                case add_sat:
                    Y = _mm256_adds_epu8(A, B);
                    break;

                case avg:
                    // shift right first, then add, to avoid overflow
                    mask = _mm256_set1_epi8(0x7F);
                    TMP = _mm256_srli_epi16(A, 1);
                    TMP = _mm256_and_si256(TMP, mask);

                    Y = _mm256_srli_epi16(B, 1);
                    Y = _mm256_and_si256(Y, mask);

                    Y = _mm256_add_epi8(Y, TMP);
                    break;

                case max:
                    Y = _mm256_max_epu8(A, B);
                    break;

                case min:
                    Y = _mm256_min_epu8(A, B);
                    break;
            }


#ifdef TEST_EVAL_AVX
            __m256i _tmpval = Y;
            unsigned char *_tmp = (unsigned char*) &_tmpval;
            printf("N: %2d = " UCFMT32 "\n", idx + CGP_INPUTS, UCVAL32(0));

            bool mismatch = false;
            for (int i = 1; i < 32; i++) {
                if (_tmp[i] != _tmp[0]) {
                    fprintf(stderr,
                        "Value mismatch on index %2d (%u instead of %u)\n",
                        i, _tmp[i], _tmp[0]);
                    mismatch = true;
                }
            }
            if (mismatch) {
                abort();
            }
#endif

            if (idx + CGP_INPUTS == genome->outputs[0]) {
                _mm256_store_si256(&outputs[0], Y);
#ifndef TEST_EVAL_AVX
                return;
#endif
            }

            ASSIGN_CURRENT(y, Y);

        } // end of column

        offset += CGP_ROWS;
        prev0 = current0;
        prev1 = current1;
        prev2 = current2;
        prev3 = current3;
    } // end of row

#ifdef TEST_EVAL_AVX
    for (int i = 0; i < CGP_OUTPUTS; i++) {
        unsigned char *_tmp = (unsigned char*) &outputs[i];
        printf("O: %2d = " UCFMT32 "\n", i, UCVAL32(0));
    }
#endif


#endif
}
