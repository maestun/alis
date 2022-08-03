//
//  alis_private.h
//  lc3vm
//

#ifndef alis_private_h
#define alis_private_h

#include "alis.h"
#include "alis_vars.h"
#include "debug.h"
#include "sys/sys.h"

#define     ALIS_BIT_0      (0)
#define     ALIS_BIT_1      (1)
#define     ALIS_BIT_2      (2)
#define     ALIS_BIT_3      (3)
#define     ALIS_BIT_4      (4)
#define     ALIS_BIT_5      (5)
#define     ALIS_BIT_6      (6)
#define     ALIS_BIT_7      (7)

alisRet readexec_opcode(void);
alisRet readexec_opername(void);
alisRet readexec_storename(void);
alisRet readexec_addname(void);
alisRet readexec_opername_saveD7(void);
alisRet readexec_addname_swap(void);
alisRet readexec_opername_swap(void);


extern sAlisOpcode  opcodes[];
extern sAlisOpcode  opernames[];
extern sAlisOpcode  storenames[];
extern sAlisOpcode  addnames[];
extern sAlisError   errors[];

// common opcode helpers
extern u16  char_array_common(u16 offset);
extern u16  int_array_common(u16 offset);
extern u16  string_array_common(u16 offset);
extern void oeval(void);
extern void ofin(void);

#endif /* alis_private_h */
