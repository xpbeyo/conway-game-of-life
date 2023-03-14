#ifndef _util_h
#define _util_h

/**
 * C's mod ('%') operator is mathematically correct, but it may return
 * a negative remainder even when both inputs are nonnegative.  This
 * function always returns a nonnegative remainder (x mod m), as long
 * as x and m are both positive.  This is helpful for computing
 * toroidal boundary conditions.
 */
static inline int 
mod (int x, int m)
{
  return (x < 0) ? ((x % m) + m) : (x % m);
}

static inline int
get_board_index(int x, int m)
{
  if (x >= m)
  {
    return x - m;
  }

  if (x < 0)
  {
    return x + m;
  }

  return x;
}

/**
 * Given neighbor count and current state, return zero if cell will be
 * dead, or nonzero if cell will be alive, in the next round.
 */
static inline char 
alivep (char count, char state)
{
  return count == 3 || (state == 1 && count == 2);
}
#define GET_ELE(__board, __ncols, __i, __j) ((__board)[(__i) * (__ncols) + (__j)])
#define BOARD(__board, __i, __j) ((__board)[(__i) + LDA * (__j)])
#define MY_BOARD(__board, __ncols, __i, __j) ((__board)[(__i) + (__ncols) * (__j)])
/**
 * Swapping the two boards only involves swapping pointers, not
 * copying values.
 */
#define SWAP_BOARDS(b1, b2)                                                    \
  do {                                                                         \
    char *temp = b1;                                                           \
    b1 = b2;                                                                   \
    b2 = temp;                                                                 \
  } while (0)

#endif /* _util_h */
