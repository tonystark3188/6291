#ifndef _PPC_DEAL_PATH_H_
#define _PPC_DEAL_PATH_H_

#ifdef __cplusplus
extern "C"{
#endif

#include "base.h"

int create_new_full_path(const char *src_path, char **dest_path, _int64_t token);

#ifdef __cplusplus
}
#endif

#endif
