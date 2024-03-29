#ifndef _MX_H_
#define _MX_H_

#include <stdlib.h>
#include <stdint.h>

/** @def    mx_type
*   @brief  Type of variables which contain neural values.
*
*   By default it is "double".
*/
typedef double mx_type;

/** @def    NN_ZERO
 *  @brief  zero cast on mx_type.
 */
#define NN_ZERO ((mx_type)0)

/** @def mx_size
 *  @brief Type of variables which contain matrix size, x, y. 
 * 
 *  By default it is uint32_t. I made this macro 'cause many times I used many 
 *  diffrent types for same tasks like iteration over matrix etc.
 */
typedef uint32_t mx_size;

/** @struct mx_t 
*   @brief  Structure with matrix data and size.
*
*   I designed this struct as interpreted 1 dimensional array to make it as fast
*   and small as possible. To change the data type in matrices, change mx_type.
*   @see mx_type
*/
typedef struct {
	mx_type *arr;	/**<    Array with matrix data */
	mx_size x;	/**<    matrix width */
	mx_size y;	/**<    matrix height*/
	mx_size size;	/**<    matrix height multiplied by width. 
				Nothing particular special but very often used*/
} mx_t;

/** @brief Matrix mutliply parameters.
 *  
 *  Set this to choose which matrix is transposed in mulitplication.
 */
typedef enum {
	DEF = 0,/**< default option */
	A = 1,	/**< first matrix is transposed */
	B = 2,	/**< second matrix is transposed */
	BOTH = 3/**< both matrix are transposed */
} mx_mp_params;

/** @brief Function creates a new matrix and returns a pointer to it. 
* 
*   Function get size of the output matrix <x, y>, checks the input, allocates memory 
*   (with calloc) for it and returns the pointer. Size of output matrix in memory 
*   depend on mx_type. If you use thread-safe calloc this function is thread-safe.
* 
*   On success, pointer to matrix is returned. If errors occurs, function return NULL;
* 
*   @param [in] x output matrix width.
*   @param [in] y output matrix height
*
*   @see mx_type
 */
mx_t *mx_create(const mx_size x, const mx_size y);

/**
*   @brief Function free memory allocated for matrix.
*
*   Function get pointer to mx_t and frees both structure and mx_type array.
*   This is not thread-safe.
*
*   @param [in] mx pointer to matrix allocated by matrix_create function.
*
*   @see mx_create
*/
void mx_destroy(mx_t *mx);

/**
 *  @brief Matrix multiply with transposing.
 * 
 *  Function get two matrices, multiply them and store output in out.
 *  You can choose which matrix is interpreted as transposed by configuring
 *  mx_mp_params enum. Not thread-safe, the input is not validated.
 * 
 *  @param [in] a first input matrix
 *  @param [in] b second output matrix
 *  @param [in] params transposing directing structure
 *  @param [out] out result of matrix multiplication (a * b = out) 
 *
 *  @see mx_mp_params
 */
void mx_mp(const mx_t a, const mx_t b, mx_t *out, const mx_mp_params params);

/** @brief Matrix hadamard product function.
 * 
 *  Function takes two matrixes, executes hadamard protuct and stores
 *  output under <out> pointer. Not thread-safe and input is not validated.
 * 
 *  @param [in] a first input matrix
 *  @param [in] b second output matrix
 *  @param [out] out result of hadamard product (a o b = out)
 */
void mx_hadamard(const mx_t a, const mx_t b, mx_t *out);

/** @brief Matrix substraction function.
 * 
 *  Function takes two input matrices <a, b> and substract them. Result is
 *  store in <out> matrix pointer. Not thread-safe and the input is not validated.
 * 
 *  @param [in] a first input matrix
 *  @param [in] b second output matrix
 *  @param [out] out result of matrix substraction (a - b = out)
 */
void mx_sub(const mx_t a, const mx_t b, mx_t *out);

/** @brief Matrix multiplication by single number.
 * 
 *  Function assumes a pointer to <a> matrix and multiply every cell by <num>.
 *  Not thread-safe and input is not validated.
 * 
 *  @param [in] a input matrix
 *  @param [in] num every cell multiplier
 */
void mx_mp_num(mx_t *a, const mx_type num);

/** @brief matrix A = matrix A o lambda on cell( matrix B )
 * 
 * function uses lambda on every cell of second matrix and 
 * return to first matrix hadamard product.
 */
void mx_hadam_lambda(mx_t *a, const mx_t b, mx_type (*lambda)(mx_type));

/** @brief matrix elementwise power by two
 * 
 * function calculates power of two in every matrix cell, modify matrix in place.
 * Not thread-safe and input is not validated.
 * 
 * @param mx input matrix
 */
void mx_elem_power_by_two(mx_t *mx);

extern void mx_set_size(mx_t *mx, const mx_size x, const mx_size y);

/**
 * @brief sum matrix a and b, store result in a
 * 
 */
void mx_add_to_first(mx_t *a, const mx_t *b);

uint8_t mx_recreate(mx_t *mx, const mx_size x, const mx_size y);

void mx_fill_rng(mx_t *values, const mx_type min, const mx_type max);

/**
 * @brief reacreate matrix if dims are smaller than x and y attributes
 * 
 * Function check mx matrix size, if it is smaller than x y product, then
 * matrix will be recreated with mx_recreate function. Return 0 in case of success,
 * if realloctaion go wrong - 1.
 * 
 * @param mx matrix to recreation
 * @param [in] x  expected x
 * @param [in] y expected y
 * @return uint8_t 
 */
uint8_t mx_recreate_if_too_small(mx_t *mx, const mx_size x, const mx_size y);

/**
 * @brief find indexes of horizontal (row-wise) max values and compare them.
 * 
 * Function takes two matrixes, in each row find max values of both matrices and compare
 * their indexes. Returns number of indexes where max values indexes are same.
 * All operations assume that both matrices have the same dimensions, in other case
 * behavior is undefined!
 * 
 * @param a 
 * @param b 
 * @return int 
 */
int mx_hor_max_idx_cmp(const mx_t a, const mx_t b);

void mx_print(const mx_t *a, char *name);

#endif

/**
*   @file  mx.h
*   @brief Header file for matrix math.
*/
