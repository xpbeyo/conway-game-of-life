/*****************************************************************************
 * life.c
 * Parallelized and optimized implementation of the game of life resides here
 ****************************************************************************/
#include "life.h"
#include "util.h"

/*****************************************************************************
 * Helper function definitions
 ****************************************************************************/

/*****************************************************************************
 * Game of life implementation
 ****************************************************************************/
char*
game_of_life (char* outboard, 
	      char* inboard,
	      const int nrows,
	      const int ncols,
	      const int gens_max)
{
  #define algo 1

  #if algo == 0
  return sequential_game_of_life(outboard, inboard, nrows, ncols, gens_max);

  #elif algo == 1
  return my_sequential_game_of_life(outboard, inboard, nrows, ncols, gens_max);
  #endif
}
