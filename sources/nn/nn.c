#include "nn.h"
#include "dense.h"

void
nn_destroy(nn_array_t* nn)
{
	if (nn == NULL)
		return;
	for (NN_SIZE i = 0; i < nn->size; ++i) {
		mx_destroy(nn->layers[i].delta);
		mx_destroy(nn->layers[i].out);
		nn->layers[i].free_data(nn->layers[i].data);
	}
	free(nn->layers);
	mx_destroy(nn->temp);
	free(nn);
}

nn_array_t*
nn_create(
	const MX_SIZE in_len,
	const MX_SIZE batch_len,
	const MX_TYPE alpha)
{
	nn_array_t* result = (nn_array_t *) calloc(1, sizeof(nn_array_t));
	if (in_len < 1 || batch_len < 1 || result == NULL)
		return NULL;
	result->alpha		= alpha;
	result->in_len		= in_len;
	result->batch_len	= batch_len;
	result->size		= 0;

	result->layers = (struct nn_layer_t *) calloc(0, sizeof(struct nn_layer_t));
	if (result->layers == NULL) {
		free(result);
		return NULL;
	}

	result->temp = mx_create(1, 1);
	if (result->temp == NULL) {
		free(result);
		free(result->layers);
	}
	return result;
}

/*
nn_array_t* 
nn_create(
	MX_SIZE         input_size, 
	MX_SIZE         b_size,
	NN_SIZE         nn_size, 
	MX_TYPE         alpha, 
	nn_params_t*    params)
{
	if (!input_size || !b_size || !nn_size || params == NULL) 
		return NULL;

	nn_array_t* ret = (nn_array_t*) calloc(1, sizeof(nn_array_t));
	if (ret == NULL) 
		return NULL;

	ret->layers = (struct nn_layer_t *) calloc
		(nn_size, sizeof(struct nn_layer_t));

	if (ret->layers == NULL) {
		free(ret); 
		return NULL;
	}
	MX_SIZE temp_size = 0;
	for (NN_SIZE i = 0; i < nn_size; ++i) {
		MX_SIZE err = (*setup_list[params[i].type])(
			(ret->layers + i), 
			input_size, 
			b_size, 
			(params + i), 
			CREATE);
		if (!err) {
			nn_destroy(ret);
			return NULL;
		}
		temp_size = MAX(temp_size, err);
		input_size = params[i].size; // layer input size = previous layer output size
	}
	ret->alpha  = alpha;
	ret->size   = nn_size;
	ret->temp   = mx_create(temp_size, 1);
	if (ret->temp == NULL) 
		nn_destroy(ret);
	return ret;
}
*/

void
nn_predict(nn_array_t* nn, const mx_t* input)
{
	const mx_t* prev_out = input;
	for (NN_SIZE i = 0; i < nn->size; ++i) {
		nn->layers[i].forwarding((nn->layers + i), prev_out);
		prev_out = nn->layers[i].out;
	}
}

void
nn_fit(nn_array_t* nn, const mx_t *input, const mx_t* output)
{
	nn_predict(nn, input);
	const NN_SIZE end = nn->size - 1;
	//delta = output - expected output (last layer case)
	mx_sub(*nn->layers[end].out, *output, nn->layers[end].delta);

	for (NN_SIZE i = end; i > 0; --i) {
		nn->layers[i].backwarding(
			(nn->layers + i), 
			nn, 
			nn->layers[i - 1].out, 
			nn->layers[i - 1].delta);
	}
	//vdelta = delta^T * input
	nn->layers->backwarding(nn->layers, nn, input, NULL);
}

//ACTIVATION FUNCS
//TODO: split activation funcs to other files or even folder

void relu_mx(mx_t *a)
{
	for (MX_SIZE i = 0; i < a->size; ++i)
		a->arr[i] = MAX(a->arr[i], NN_ZERO);
}

MX_TYPE 
relu_deriv_cell(MX_TYPE a) 
{
	return (MX_TYPE)((a > NN_ZERO) ? 1 : 0);
}