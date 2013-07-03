#include <stdlib.h>
#include <stdint.h>
#include "neostore.h"
#include "reltypestore.h"

unsigned char *
get_reltype_rec(unsigned char **ptr, uint64_t id) {
    return ptr[id / LIMB_SIZE] + ((id % LIMB_SIZE) * RELTYPE_SIZE);
}
