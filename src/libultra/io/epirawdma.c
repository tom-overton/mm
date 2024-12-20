#include "ultra64.h"

s32 __osEPiRawStartDma(OSPiHandle* handle, s32 direction, uintptr_t cartAddr, void* dramAddr, size_t size) {
    s32 status;
    OSPiHandle* curHandle;

    while (status = IO_READ(PI_STATUS_REG), status & (PI_STATUS_DMA_BUSY | PI_STATUS_IO_BUSY)) {
        ;
    }

    if (__osCurrentHandle[handle->domain]->type != handle->type) {
        curHandle = __osCurrentHandle[handle->domain];

        if (handle->domain == 0) {
            if (curHandle->latency != handle->latency) {
                IO_WRITE(PI_BSD_DOM1_LAT_REG, handle->latency);
            }

            if (curHandle->pageSize != handle->pageSize) {
                IO_WRITE(PI_BSD_DOM1_PGS_REG, handle->pageSize);
            }

            if (curHandle->relDuration != handle->relDuration) {
                IO_WRITE(PI_BSD_DOM1_RLS_REG, handle->relDuration);
            }

            if (curHandle->pulse != handle->pulse) {
                IO_WRITE(PI_BSD_DOM1_PWD_REG, handle->pulse);
            }
        } else {
            if (curHandle->latency != handle->latency) {
                IO_WRITE(PI_BSD_DOM2_LAT_REG, handle->latency);
            }

            if (curHandle->pageSize != handle->pageSize) {
                IO_WRITE(PI_BSD_DOM2_PGS_REG, handle->pageSize);
            }

            if (curHandle->relDuration != handle->relDuration) {
                IO_WRITE(PI_BSD_DOM2_RLS_REG, handle->relDuration);
            }

            if (curHandle->pulse != handle->pulse) {
                IO_WRITE(PI_BSD_DOM2_PWD_REG, handle->pulse);
            }
        }

        curHandle->type = handle->type;
        curHandle->latency = handle->latency;
        curHandle->pageSize = handle->pageSize;
        curHandle->relDuration = handle->relDuration;
        curHandle->pulse = handle->pulse;
    }

    IO_WRITE(PI_DRAM_ADDR_REG, osVirtualToPhysical(dramAddr));
    IO_WRITE(PI_CART_ADDR_REG, K1_TO_PHYS(handle->baseAddress | cartAddr));

    switch (direction) {
        case OS_READ:
            IO_WRITE(PI_WR_LEN_REG, size - 1);
            break;

        case OS_WRITE:
            IO_WRITE(PI_RD_LEN_REG, size - 1);
            break;

        default:
            return -1;
    }

    return 0;
}
