from testsGenerator import *
import numpy as np
import random

DELTA = .0001

#	matrix create tests

gen = TestsGenerator()
for pair in [[0, 0], [1, 0], [0, 1]]:
	gen.genTest('mx_create',
		'\tmx_t *a = mx_create({}, {});\n'.format(*pair) +
		genAssert('a != NULL'))

x, y = random.randint(1, 16), random.randint(1, 16)
gen.genTest('mx_create',
	f'\tmx_t *a = mx_create({x}, {y});\n' +
	genAssert('a == NULL') + 
	genAssert(f'a->x != {x} || a->y != {y} || a->size != {x * y}') +
	'\tmx_destroy(a);')

#	matrix multiplication tests

for t in ('0', 'A', 'B', 'BOTH'):
	for n in range(5):
		x1, y1 = random.randint(1, 16), random.randint(1, 16)
		x2, y2 = {
			'0'	: (random.randint(1, 16), x1), 
			'A'	: (random.randint(1, 16), y1),
			'B' 	: (x1, random.randint(1, 16)), 
			'BOTH'	: (y1, random.randint(1, 16))}[t]

		x3, y3 = {'0':(x2,y1),'A':(x2,x1),'B':(y2,y1),'BOTH':(y2,x1)}[t]
		a = np.random.randint(-254, high = 254, size = (y1, x1))
		b = np.random.randint(-254, high = 254, size = (y2, x2))
		expected = np.matmul(
			np.transpose(a) if t in ('A', 'BOTH') else a, 
			np.transpose(b) if t in ('B', 'BOTH') else b).flatten()

		gen.genTest('mx_mp' if t == '0' else 'mx_mp (transposed)',
			genStaticMxDec(a, 'a') +
			genStaticMxDec(b, 'b') +
			genStaticEmptyMxDec((y3, x3), 'res') +
			genStaticListDec(expected, 'exp') +
			f'\tmx_mp(a, b, &res, {t});\n\n' +
			genMxComp('res', 'exp', DELTA))

#	matrix hadamard product tests

for n in range(5):
	x, y = random.randint(1, 16), random.randint(1, 16)
	a = np.random.randint(-254, high = 256, size = (y, x))
	b = np.random.randint(-254, high = 256, size = (y, x))
	expected = np.multiply(a, b).flatten()
	gen.genTest('mx_hadamard',
		genStaticMxDec(a, 'a') +
		genStaticMxDec(b, 'b') +
		genStaticEmptyMxDec((y, x), 'res') +
		genStaticListDec(expected, 'exp') +
		f'\tmx_hadamard(a, b, &res);\n' +
		genMxComp('res', 'exp', DELTA))

#	matrix substraction

for n in range(5):
	x, y = random.randint(1, 16), random.randint(1, 16)
	a = np.random.randint(-254, high = 254, size = (y, x))
	b = np.random.randint(-254, high = 254, size = (y, x))
	expected = (a - b).flatten()
	gen.genTest('mx_substraction',
		genStaticMxDec(a, 'a') +
		genStaticMxDec(b, 'b') +
		genStaticEmptyMxDec((y, x), 'res') +
		genStaticListDec(expected, 'exp') +
		f'\tmx_sub(a, b, &res);\n' +
		genMxComp('res', 'exp', DELTA))

#	matrix multiply by number

for n in range(5):
	x, y = random.randint(1, 16), random.randint(1, 16)
	a = np.random.randint(-254, high = 254, size = (y, x))
	num = random.randint(-16, 16)
	expected = (a * num).flatten()
	gen.genTest('mx_mp_num',
		genStaticMxDec(a, 'a') +
		genStaticListDec(expected, 'exp') +
		f'\tmx_mp_num(&a, {num});\n' +
		genMxComp('a', 'exp', DELTA))

#	matrix hadamard product with lambda

gen.appendCode('\nMX_TYPE foo(MX_TYPE a) {return (a > 4) ? 0 : a;}\n')

x, y = random.randint(1, 16), random.randint(1, 16)
a = np.random.randint(-254, high = 254, size = (y, x))
b = np.random.randint(-254, high = 254, size = (y, x))
new_b = np.array([[0 if x > 4 else x for x in y] for y in b])
expected = (a * new_b).flatten()

gen.genTest('mx_hadam_lambda',
	genStaticMxDec(a, 'a') +
	genStaticMxDec(b, 'b') +
	f'\tmx_hadam_lambda(&a, b, (&foo));\n' +
	genStaticListDec(expected, 'exp') +
	genMxComp('a', 'exp', DELTA))

#	neural network create

for n in range(5):
	inputSize = random.randint(1, 16)
	batchSize = random.randint(1, 16)
	denseSize1 = random.randint(1, 16)
	denseSize2 = random.randint(1, 16)
	maxTmp = max([inputSize * denseSize1, denseSize1 * denseSize2])
	gen.genTest('nn_create',

	#initializer of neural network
	f'''	nn_params_t initializer[] = {{
		{{.type = DENSE, .activ_func = RELU, .max = 0.2, .min=0.1, .size = {denseSize1}}},
		{{.type = DENSE, .activ_func = NO_FUNC, .max = 0.2, .min=01, .size = {denseSize2}}}
	}};
	nn_array_t *nn = nn_create({inputSize}, {batchSize}, 2, 0.01, initializer);\n''' +

	# check size of first layer
		genAssert('nn->layers[0].delta == NULL || nn->layers[0].out == NULL') +
		genAssert(f'nn->layers[0].delta->x != {denseSize1} || nn->layers[0].delta->y != {batchSize}') +
		genAssert(f'nn->layers[0].out->x != {denseSize1} || nn->layers[0].out->y != {batchSize}') +
		'''\n\tdense_data_t *ptr = nn->layers[0].data;\n''' +
		genAssert(f'ptr->val->x != {inputSize} || ptr->val->y != {denseSize1}') +

	# check size of second layer
		genAssert('nn->layers[1].delta == NULL || nn->layers[1].out == NULL') +
		genAssert(f'nn->layers[1].delta->x != {denseSize2} || nn->layers[1].delta->y != {batchSize}') +
		genAssert(f'nn->layers[1].out->x != {denseSize2} || nn->layers[1].out->y != {batchSize}') +
		'''\n\tptr = (dense_data_t *) nn->layers[1].data;\n''' +
		genAssert(f'ptr->val->x != {denseSize1} || ptr->val->y != {denseSize2}') +

	# check size of temporary matrix stored in neural network struct
		genAssert(f'nn->temp->size != {maxTmp}') +
		'\tnn_destroy(nn);\n')

#	neural network predict

gen.genTest('nn_predict',
'''
	nn_params_t initializer[] = 
	{
        	{.type = DENSE, .activ_func = NO_FUNC, .size = 3},
		{.type = DENSE, .activ_func = NO_FUNC, .size = 3}
	};\n''' +
	genStaticMxDec(np.array([[8.5, 0.65, 1.2]]), 'input') + '''
	nn_array_t *n = nn_create(3, 1, 2, 0.01, initializer);
	dense_data_t *ptr = (n->layers[0].data);\n''' +	
	genStaticListDec([.1, .2, -.1, -.1, .1, .9, .1, .4, .1], 'val0') +
	genStaticListDec([.3, 1.1, -.3, .1, .2, .0, .0, 1.3, .1], 'val1') +
	genListCpy('val0', 'ptr->val->arr','ptr->val->size') +
	'\tptr = n->layers[1].data;\n' +
	genListCpy('val1', 'ptr->val->arr', 'ptr->val->size') +
	'\tnn_predict(n, &input);\n' +
	genStaticListDec([.2135, .145, .5065], 'expected') +
	genMxComp('(*n->layers[1].out)', 'expected', DELTA) +
	'\tnn_destroy(n);\n')

gen.genTest('nn_predict',
'''	nn_params_t initializer[] = {
		{.type = DENSE, .activ_func = RELU, .size = 3},
		{.type = DENSE, .activ_func = NO_FUNC, .size = 1}
	};
	nn_array_t *nn = nn_create(1, 1, 2, 0.01, initializer);\n''' +
	genStaticListDec([.1, -.1, .1], 'val0') +
	genStaticListDec([.3, 1.1, -.3], 'val1') +
	'\tdense_data_t *ptr = (dense_data_t *) nn->layers->data;\n' +
	genListCpy('val0', 'ptr->val->arr', 'ptr->val->size') +
	'\n\tptr = (dense_data_t *) nn->layers[1].data;\n' +
	genListCpy('val1', 'ptr->val->arr', 'ptr->val->size') +
	genStaticMxDec(np.array([[8.5]]), 'input') +
	'\tnn_predict(nn, &input);\n' +
	genAssert(f'nn->layers[1].out->arr[0] > {DELTA} || nn->layers[1].out->arr[0] < -{DELTA}') +
	'\tnn_destroy(nn);\n')

#	neural network predict

gen.genTest('nn_fit',
'''   	nn_params_t initializer[] = {
		{.type = DENSE, .activ_func = RELU, .size = 3},
		{.type = DENSE, .activ_func = NO_FUNC, .size = 3}
	};
	nn_array_t* nn = nn_create(3, 4, 2, 0.01, initializer);\n''' +
	genStaticMxDec(np.array([
		[8.5, .65, 1.2], 
		[9.5, .8, 1.3], 
		[9.9, .8, .5],
		[9.0, .9, 1.0]]), 'input') +
	genStaticListDec([.1, .2, -.1, -.1, .1, .9, .1, .4, .1], 'val0') +
	genStaticListDec([.3, 1.1, -.3, .1, .2, .0, .0, 1.3, .1], 'val1') +
	'\tdense_data_t* ptr = nn->layers->data;\n' +
	genListCpy('val0', 'ptr->val->arr', 'ptr->val->size') + 
	'\n\tptr = nn->layers[1].data;\n' +
	genListCpy('val1', 'ptr->val->arr', 'ptr->val->size') +
	genStaticMxDec(np.array([
		[0.1, 1.0, 0.1], 
		[0.0, 1.0, 0.0], 
		[0.0, 0.0, 0.1], 
		[0.1, 1.0, 0.2]]),'out') +
	'\tnn_fit(nn, &input, &out);\n' +
	genStaticListDec([.118847, .201724, -.0977926, -.190674, .0930147, .886871, .093963, .399449, .0994944], 'exp_val0') +
	genStaticListDec([.29901, 1.09916, -.301627, .123058, .205844, .0328309, -.0096053, 1.29716, .0863697], 'exp_val1') +
	genMxComp('(* ptr->val)', 'exp_val1', DELTA) +
	'\tptr = (dense_data_t *) nn->layers[0].data;\n' +
	genMxComp('(* ptr->val)', 'exp_val0', DELTA) +
	'\tnn_destroy(nn);\n')

gen.save('sources/main.c')