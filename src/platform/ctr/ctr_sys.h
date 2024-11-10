#ifndef FALLOUT_PLATFORM_CTR_SYS_H_
#define FALLOUT_PLATFORM_CTR_SYS_H_

#include <3ds.h>

namespace fallout {

extern size_t linearHeapAvailable;
extern size_t heapAvailable;

extern size_t linearHeapAvailableAtStart;
extern size_t heapAvailableAtStart;


size_t ctr_sys_check_linear_heap();
size_t ctr_sys_check_heap();

float ctr_sys_get_fps();

#ifdef _DEBUG
void ctr_sys_3dslink_stdio();
#endif

} // namespace fallout

#endif /* FALLOUT_PLATFORM_CTR_SYS_H_ */
