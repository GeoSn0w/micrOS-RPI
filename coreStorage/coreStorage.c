#include "../coreOS/GPIO/gpio.h"
#include "../coreOS/GPIO/delays.h"
#include "coreStorage.h"
#include "coreStorage_types.h"

int coreStorage_initialize(unsigned int llcommand){
    unsigned int r, command=llcommand | INT_ERROR_MASK;
    
    int cnt = 1000000;
    
    while(!(*EMMC_INTERRUPT & command) && cnt--){
        wait_msec(1);
        r=*EMMC_INTERRUPT;
    }
    
    if (cnt<=0 || (r & INT_CMD_TIMEOUT) || (r & INT_DATA_TIMEOUT) ) { *EMMC_INTERRUPT=r;
        return coreStorage_TIMOUT;
            
    } else if (r & INT_ERROR_MASK) {
        *EMMC_INTERRUPT=r;
        return coreStorage_FAILURE;
    }
    
    *EMMC_INTERRUPT = llcommand;
    return coreStorage_SUCCESS;
}
