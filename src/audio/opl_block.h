// opl_block.h — the host→DSP wire contract for FM music.
//
// The host (68k) decodes the OPL register stream and, once per music frame,
// emits one of these per channel: the resolved 2-operator FM parameters. tll/rks
// are computed on the host so the DSP needs no tables. dsp_opl_feed() packs
// these into host-port words.
//
// op[0] = modulator, op[1] = carrier. FB lives on the modulator only.

#ifndef OPL_BLOCK_H
#define OPL_BLOCK_H

#include <stdint.h>

// Key event for the frame. The sequencer keys OFF then ON within one frame for
// every new note, so a re-trigger of an already-playing channel is 1->0->1 (net
// state unchanged). The host therefore reports the LAST key edge of the frame,
// not the net state — that edge fully determines the DSP's eg_state/phase.
enum { OPL_KEY_NONE = 0, OPL_KEY_ON = 1, OPL_KEY_OFF = 2 };

typedef struct {
    uint8_t  key;          // OPL_KEY_NONE / OPL_KEY_ON / OPL_KEY_OFF (last edge)
    uint8_t  connection;   // ch_alg: 0=FM (mod→car), 1=additive (mod+car)
    uint16_t fnum;         // 10-bit F-number (phase increment)
    uint8_t  blk;          // 3-bit block/octave
    struct {
        uint8_t  ML, AM, PM, EG;   // multiplier, tremolo, vibrato, sustain-type
        uint8_t  AR, DR, SL, RR;   // ADSR rates + sustain level
        uint8_t  FB;               // feedback (modulator only)
        uint32_t tll;              // resolved total level + key-scale level (host)
        int32_t  rks;              // resolved key-scale rate (host)
    } op[2];
} opl_block_t;

#endif
