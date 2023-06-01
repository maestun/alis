//
//  alis_private.h
//  lc3vm
//

#ifndef alis_private_h
#define alis_private_h

#include "alis.h"
#include "debug.h"
#include "sys/sys.h"

alisRet readexec_opcode(void);
alisRet readexec_opername(void);
alisRet readexec_storename(void);
alisRet readexec_addname(void);
alisRet readexec_opername_saveD7(void);
alisRet readexec_opername_saveD6(void);
alisRet readexec_addname_swap(void);
alisRet readexec_opername_swap(void);


extern sAlisOpcode  opcodes[];
extern sAlisOpcode  opernames[];
extern sAlisOpcode  storenames[];
extern sAlisOpcode  addnames[];
extern sAlisError   errors[];

// common opcode helpers
extern u16  loctc_common(u16 offset);
extern u16  locti_common(u16 offset);
extern u16  loctp_common(u16 offset);
extern void oeval(void);
extern void ofin(void);

#endif /* alis_private_h */
