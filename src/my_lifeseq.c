#include "life.h"
#include "util.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <omp.h>
typedef unsigned long long ull;

#define n_bits (8*sizeof(ull))

int* north_table;
int* south_table;
int* west_table;
int* east_table;

typedef struct
{
  ull r13_0;
  ull r13_1;
  ull r123_0;
  ull r123_1;
} row_sums_t;

row_sums_t* row_sums;

static inline ull bitcell_state(ull* bit_board, int i, int j, int ncols)
{
  return (bit_board[i * (ncols / n_bits) + j / n_bits] >> (n_bits - j % n_bits - 1)) & 1U;
}

static inline void bitcell_set_cell(ull* bit_board, int i, int j, int ncols)
{
  bit_board[i * (ncols / n_bits) + j / n_bits] |= (1UL << (n_bits - j % n_bits - 1));
}

ull* init_bit_board(char* board, int nrows, int ncols)
{
  assert(nrows % (n_bits) == 0);
  assert(ncols % (n_bits) == 0);

  int n_bitcells_a_row = ncols / (n_bits);
  ull* bit_board = (ull*)malloc(2 * sizeof(ull) * nrows * n_bitcells_a_row);
  memset(bit_board, 0, sizeof(ull) * nrows * n_bitcells_a_row);

  for (int i = 0; i < nrows; i ++)
  {
    for (int j = 0; j < ncols; j ++)
    {
      if (GET_ELE(board, ncols, i, j))
      {
        bitcell_set_cell(bit_board, i, j, ncols);
      }
    }
  }

  return bit_board;
}

void bitboard_write_back(ull* bit_board, char* board, int nrows, int ncols)
{
  for (int i = 0; i < nrows; i++)
  {
    for (int j = 0; j < ncols; j++)
    {
      GET_ELE(board, ncols, i, j) = (char)bitcell_state(bit_board, i, j, ncols);
    }
  }
}

static inline row_sums_t get_row_sums(
  ull* bitboard,
  int loc
)
{
  row_sums_t r123;
  const int north = north_table[loc];
  const int south = south_table[loc];

  ull north_row = bitboard[north];
  ull this_row = bitboard[loc];
  ull south_row = bitboard[south];

  r123.r13_0 = north_row ^ south_row;
  r123.r13_1 = north_row & south_row;

  r123.r123_0 = r123.r13_0 ^ this_row;
  r123.r123_1 = ((north_row | south_row) & this_row) | r123.r13_1;

  return r123;
}
char* bitcell_seq_life(
  char *outboard,
  char *inboard,
  const int nrows,
  const int ncols,
  const int gens_max
)
{
  omp_set_dynamic(0);
  omp_set_num_threads(10);
  int n_bitcells_a_row = ncols / (n_bits);

  north_table = (int*)malloc(sizeof(int) * nrows * n_bitcells_a_row);
  south_table = (int*)malloc(sizeof(int) * nrows * n_bitcells_a_row);
  east_table = (int*)malloc(sizeof(int) * nrows * n_bitcells_a_row);
  west_table = (int*)malloc(sizeof(int) * nrows * n_bitcells_a_row);
  row_sums = (row_sums_t*)malloc(sizeof(row_sums_t) * nrows * n_bitcells_a_row);

  for (int i = 0; i < nrows; i++)
  {
    for (int j = 0; j < n_bitcells_a_row; j++)
    {
      north_table[i * n_bitcells_a_row + j] = get_board_index(i - 1, nrows) * n_bitcells_a_row + j;
      south_table[i * n_bitcells_a_row + j] = get_board_index(i + 1, nrows) * n_bitcells_a_row + j;
      west_table[i * n_bitcells_a_row + j] = i * n_bitcells_a_row + get_board_index(j - 1, n_bitcells_a_row);
      east_table[i * n_bitcells_a_row + j] = i * n_bitcells_a_row + get_board_index(j + 1, n_bitcells_a_row);
    }
  }

  ull* bit_board = init_bit_board(inboard, nrows, ncols);
  for (int curgen = 0; curgen < gens_max; curgen++) {
    ull* in_bitboard;
    ull* out_bitboard;
    if (curgen % 2 == 0)
    {
      in_bitboard = bit_board;
      out_bitboard = bit_board + nrows * n_bitcells_a_row;
    }
    else
    {
      out_bitboard = bit_board;
      in_bitboard = bit_board + nrows * n_bitcells_a_row;
    }

    #pragma omp parallel for
    for (int loc = 0; loc < nrows * n_bitcells_a_row; loc++)
    {
      row_sums[loc] = get_row_sums(in_bitboard, loc);
    }

    #pragma omp parallel for
    for (int loc = 0; loc < nrows * n_bitcells_a_row; loc++)
    {
      const int west = west_table[loc];
      const int east = east_table[loc];
      ull this_row = in_bitboard[loc];

      // row_sums_t this_rowsums = get_row_sums(in_bitboard, loc);
      row_sums_t this_rowsums = row_sums[loc];
      // row_sums_t east_rowsums = get_row_sums(in_bitboard, east);
      row_sums_t east_rowsums = row_sums[east];
      // row_sums_t west_rowsums = get_row_sums(in_bitboard, west);
      row_sums_t west_rowsums = row_sums[west];

      ull r123_0shift_right = (this_rowsums.r123_0 >> 1) | ((west_rowsums.r123_0 & 1UL) << (n_bits - 1));
      ull r123_1shift_right = (this_rowsums.r123_1 >> 1) | ((west_rowsums.r123_1 & 1UL) << (n_bits - 1));

      ull r123_0shift_left = (this_rowsums.r123_0 << 1) | ((east_rowsums.r123_0 & (0x8000000000000000)) >> (n_bits - 1));
      ull r123_1shift_left = (this_rowsums.r123_1 << 1) | ((east_rowsums.r123_1 & (0x8000000000000000)) >> (n_bits - 1));

      ull S0 = r123_0shift_left ^ r123_0shift_right ^ this_rowsums.r13_0;
      ull S0_carry = ((r123_0shift_left | r123_0shift_right) & this_rowsums.r13_0) | (r123_0shift_right & r123_0shift_left);
      
      ull S1 = r123_1shift_left ^ S0_carry ^ r123_1shift_right ^ this_rowsums.r13_1;

      ull C1 = r123_1shift_left & this_rowsums.r13_1;
      ull G1 = r123_1shift_left ^ this_rowsums.r13_1;

      ull C2 = r123_1shift_right & S0_carry;
      ull G2 = r123_1shift_right ^ S0_carry;

      ull S2 = C1 | C2 | (G1 & G2);
      ull new_state = (S0 | this_row) & S1 & ~S2;

      out_bitboard[loc] = new_state;
    }
  }

  ull* bitboard_to_write_from;
  if (gens_max % 2 == 0)
  {
    bitboard_to_write_from = bit_board;
  }
  else
  {
    bitboard_to_write_from = bit_board + nrows * n_bitcells_a_row;
  }
  bitboard_write_back(bitboard_to_write_from, outboard, nrows, ncols);

  return outboard;
}

char *my_sequential_game_of_life(
  char *outboard,
  char *inboard,
  const int nrows,
  const int ncols,
  const int gens_max
  )
{
  return bitcell_seq_life(outboard, inboard, nrows, ncols, gens_max);
}

