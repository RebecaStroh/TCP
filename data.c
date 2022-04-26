#include "data.h"
#include <stdlib.h>

data* newData() {
    data *aData = (data*) malloc(sizeof(data));

    aData->gender = "a titlekaka";
    return aData;
}