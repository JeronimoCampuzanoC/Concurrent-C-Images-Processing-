#ifndef BORDER_H
#define BORDER_H

// Include necessary headers
#include <stdio.h>
#include <stdlib.h>
#include "imagen_info.h"

int detectarBordesSobel(ImagenInfo *info, int nHilos);

#endif // BORDER_H