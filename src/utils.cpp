
#include "utils.h"

#include <stdlib.h>

float rand_between(float min, float max) {

	float range = max - min;
	float r = (float)rand() / (float)RAND_MAX;

	return min + r * range;
}
