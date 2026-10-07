// opl_host.h — host-side OPL register decoder + per-frame FM block emitter.
// Models the 68k side of the DSP OPL music player: consume the same register
// writes the ALIS sequencer emits, then hand the DSP resolved opl_block_t's.

#ifndef OPL_HOST_H
#define OPL_HOST_H

#include <stdint.h>
#include "opl_block.h"

typedef struct opl_host opl_host_t;

opl_host_t *opl_host_new(void);
void        opl_host_reset(opl_host_t *h);
void        opl_host_write_reg(opl_host_t *h, uint32_t reg, uint8_t data);
// Resolve channel ch (0..8) into *out (tll/rks computed here).
void        opl_host_get_block(opl_host_t *h, int ch, opl_block_t *out);
void        opl_host_delete(opl_host_t *h);

#endif
