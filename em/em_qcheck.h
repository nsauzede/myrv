#ifndef EM_QCHECK_H_
#define EM_QCHECK_H_

#include "rv.h"

int
qinit(rv_ctx* ctx, char* esw);
int
qcheck(rv_ctx* ctx);
int
qcleanup(rv_ctx* ctx);

#endif /*EM_QCHECK_H_*/
