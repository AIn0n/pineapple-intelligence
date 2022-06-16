#include "bgd.h"
#include "stdlib.h"

void 
bgd_optimize(void *params, mx_t *weights, mx_t *vdelta, const nn_size idx)
{	
	(void)(idx); /* to stop compiler screaming about unused parameters ;) */
	const bgd_data_t *data = (bgd_data_t *) params;
	mx_mp_num(vdelta, data->alpha);
	mx_sub(*weights, *vdelta, weights);
}

void 
bgd_destroy(const nn_size unused, void* data)
{
	(void) (unused);
	bgd_data_t *cast_data = (bgd_data_t *) data;
	free(cast_data);
	cast_data = NULL;
}

uint8_t
add_batch_gradient_descent(nn_t *nn, const mx_type alpha)
{
	bgd_data_t *data = calloc(1, sizeof(*data));
	if (data == NULL || nn == NULL || nn->optimizer.params != NULL)
		return 1;
	data->alpha = alpha;
	nn->optimizer = (optimizer_t) {
		.update = bgd_optimize,
		.params_destroy = bgd_destroy,
		.params = (void *) data
	};
	return 0;
}
