//
// Copyright 2023 Olivier Huguenot, Vadim Kindl
//
// Permission is hereby granted, free of charge, to any person obtaining a copy 
// of this software and associated documentation files (the “Software”), 
// to deal in the Software without restriction, including without limitation 
// the rights to use, copy, modify, merge, publish, distribute, sublicense, 
// and/or sell copies of the Software, and to permit persons to whom the 
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in 
// all copies or substantial portions of the Software.
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, 
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE 
// OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

#include "alis.h"
#include "mem.h"

// =============================================================================
// MARK: - MEMORY ACCESS
// =============================================================================

fRead16 read16;
fRead24 read24;
fRead32 read32;

static u16 _read16(const u8 *ptr) {
    return *((u16*)ptr);
}

static u16 _read16LE(const u8 *ptr) {
    return (u16)ptr[0] | ((u16)ptr[1] << 8);
}

static u16 _read16BE(const u8 *ptr) {
    return (u16)ptr[1] | ((u16)ptr[0] << 8);
}

static u32 _read24LE(const u8 *ptr) {
    u32 result = (u32)ptr[0] | ((u32)ptr[1] << 8) | ((u32)ptr[2] << 16);
    return (result > 0x7FFFFF) ? (result << 8) & 0xff : result;
}

static u32 _read24BE(const u8 *ptr) {
    u32 result = (u32)ptr[2] | ((u32)ptr[1] << 8) | ((u32)ptr[0] << 16);
    return (result > 0x7FFFFF) ? (result << 8) & 0xff : result;
}

static u32 _read32(const u8 *ptr) {
    return *((u32*)ptr);
}

static u32 _read32LE(const u8 *ptr) {
    return (u32)ptr[0] | ((u32)ptr[1] << 8) | ((u32)ptr[2] << 16) | ((u32)ptr[3] << 24);
}

static u32 _read32BE(const u8 *ptr) {
    return (u32)ptr[3] | ((u32)ptr[2] << 8) | ((u32)ptr[1] << 16) | ((u32)ptr[0] << 24);
}

typedef void (*fWrite16)(u32, s16);
typedef void (*fWrite32)(u32, s32);

static fWrite16 _write16_fptr;
static fWrite32 _write24_fptr;
static fWrite32 _write32_fptr;

typedef u16 (*fConvert16)(u16);
typedef u32 (*fConvert32)(u32);

static fConvert16 _convert16;
static fConvert32 _convert32;

static fConvert16 _pcconvert16;
static fConvert32 _pcconvert32;

static u16 _swap16(u16 value) {
    return (value <<  8) | (value >>  8);
}

static u16 _linear16(u16 value) {
    return value;
}

static u32 _swap32(u32 value) {
    return ((value >> 24) & 0xff) |
           ((value <<  8) & 0xff0000) |
           ((value >>  8) & 0xff00) |
           ((value << 24) & 0xff000000);
}

static u32 _linear32(u32 value) {
    return value;
}

void vram_init(void) {
    _convert16 = (alis.platform.is_little_endian == is_host_le()) ? _linear16 : _swap16;
    _convert32 = (alis.platform.is_little_endian == is_host_le()) ? _linear32 : _swap32;

    read16 = (alis.platform.is_little_endian == is_host_le()) ? _read16 : alis.platform.is_little_endian ? _read16LE : _read16BE;
    read24 = alis.platform.is_little_endian ? _read24LE : _read24BE;
    read32 = (alis.platform.is_little_endian == is_host_le()) ? _read32 : alis.platform.is_little_endian ? _read32LE : _read32BE;

    _pcconvert16 = (alis.platform.kind == EPlatformPC) ? ((alis.platform.is_little_endian == is_host_le()) ? _swap16 : _linear16) : _convert16;
    _pcconvert32 = (alis.platform.kind == EPlatformPC) ? ((alis.platform.is_little_endian == is_host_le()) ? _swap32 : _linear32) : _convert32;
}

void write32(const u8 *ptr, u32 value) {
    *((u32*)ptr) = _convert32(value);
}

u16 swap16(const u8 *ptr) {
    return _convert16(*((u16*)ptr));
}

u32 swap32(const u8 *ptr) {
    return _convert32(*((u32*)ptr));
}

u8 xread8(u32 offset) {
    ALIS_DEBUG(EDebugVerbose, " [%.2x <= %.6x]", *(u8 *)(alis.mem + offset), offset);
    return *(alis.mem + offset);
}

extern u32 g_val;

s16 xread16(u32 offset) {
    g_val = _convert16(*(u16*)(alis.mem + offset));
    ALIS_DEBUG(EDebugVerbose, " [%.4x <= %.6x]", g_val, offset);
    return (s16)g_val;
}

s32 xread32(u32 offset) {
    g_val = _convert32(*(u32*)(alis.mem + offset));
    ALIS_DEBUG(EDebugVerbose, " [%.4x <= %.6x]", g_val, offset);
    return (s32)g_val;
}

s16 xpcswap16(u16 value) {
    return _pcconvert16(value);
}

s32 xpcswap32(u32 value) {
    return _pcconvert32(value);
}

s16 xpcread16(u32 offset) {
    u16* ptr = (u16*)(alis.mem + offset);
    g_val = _pcconvert16(*ptr);
    ALIS_DEBUG(EDebugVerbose, " [%.4x <= %.6x]", g_val, offset);
    return (s16)g_val;
}

s32 xpcread32(u32 offset) {
    u32* ptr = (u32*)(alis.mem + offset);
    g_val = _pcconvert32(*ptr);
    ALIS_DEBUG(EDebugVerbose, " [%.4x <= %.6x]", g_val, offset);
    return (s32)g_val;
}

void xpcwrite16(u32 offset, s16 value) {
    ALIS_DEBUG(EDebugVerbose, " [%.4x => %.6x]", value, offset);
    *(s16 *)(alis.mem + offset) = _pcconvert16(value);
}

void xpcwrite32(u32 offset, s32 value) {
    ALIS_DEBUG(EDebugVerbose, " [%.8x => %.6x]", value, offset);
    *(s32 *)(alis.mem + offset) = _pcconvert32(value);
}

u8 * get_vram(s16 offset) {
    ALIS_DEBUG(EDebugVerbose, " [%s <= %.6x]", (char *)(alis.mem + alis.script->vram_org + offset), alis.script->vram_org + offset);
    return (u8 *)(alis.mem + alis.script->vram_org + offset);
}

u16 xswap16(u16 value) {
    return _convert16(value);
}

u32 xswap32(u32 value) {
    return _convert32(value);
}

u8 * xreadptr(u32 offset) {
    ALIS_DEBUG(EDebugVerbose, " [\"%s\" <= %.6x]", (char *)(alis.mem + offset), offset);
    return (u8 *)(alis.mem + offset);
}

void xwrite8(u32 offset, u8 value) {
    ALIS_DEBUG(EDebugVerbose, " [%.2x => %.6x]", value, offset);
    *(u8 *)(alis.mem + offset) = value;
}

void xwrite16(u32 offset, s16 value) {
    ALIS_DEBUG(EDebugVerbose, " [%.4x => %.6x]", value, offset);
    *(s16 *)(alis.mem + offset) = _convert16(value);
}

void xwrite32(u32 offset, s32 value) {
    ALIS_DEBUG(EDebugVerbose, " [%.8x => %.6x]", value, offset);
    *(s32 *)(alis.mem + offset) = _convert32(value);
}

void xadd8(s32 offset, s8 value) {
    ALIS_DEBUG(EDebugVerbose, " [%d + %d => %.6x]", *(u8 *)(alis.mem + offset), value, offset);
    *(s8 *)(alis.mem + offset) += value;
}

void xadd16(s32 offset, s16 addition) {
    g_val = xread16(offset);
    ALIS_DEBUG(EDebugVerbose, " [%d + %d => %.6x]", g_val, addition, offset);
    xwrite16(offset, g_val + addition);
}

void xadd32(s32 offset, s32 addition) {
    g_val = xread32(offset);
    ALIS_DEBUG(EDebugVerbose, " [%d + %d => %.6x]", g_val, addition, offset);
    xwrite32(offset, g_val + addition);
}

void xsub8(s32 offset, s8 value) {
    ALIS_DEBUG(EDebugVerbose, " [%d - %d => %.6x]", *(u8 *)(alis.mem + offset), value, offset);
    *(s8 *)(alis.mem + offset) -= value;
}

void xsub16(s32 offset, s16 sub) {
    g_val = xread16(offset);
    ALIS_DEBUG(EDebugVerbose, " [%d - %d => %.6x]", g_val, sub, offset);
    xwrite16(offset, g_val - sub);
}

void xsub32(s32 offset, s32 sub) {
    g_val = xread32(offset);
    ALIS_DEBUG(EDebugVerbose, " [%d - %d => %.6x]", g_val, sub, offset);
    xwrite32(offset, g_val - sub);
}

void xpush32(s32 value) {
    alis.script->vacc_off -= sizeof(u32);
    xwrite32(alis.script->vram_org + alis.script->vacc_off, value);
    ALIS_DEBUG(EDebugVerbose, " [%.8x => va %.4x + %.6x (%.6x)]", value, (s16)alis.script->vacc_off, alis.script->vram_org, alis.script->vacc_off + alis.script->vram_org);
}

s32 xpeek32(void) {
    return xread32(alis.script->vram_org + alis.script->vacc_off);
}

s32 xpop32(void) {
    g_val = xpeek32();
    ALIS_DEBUG(EDebugVerbose, " [%.8x <= va %.4x + %.6x (%.6x)]", g_val, (s16)alis.script->vacc_off, alis.script->vram_org, alis.script->vacc_off + alis.script->vram_org);
    alis.script->vacc_off += sizeof(s32);
    return g_val;
}

u16 fread16(FILE* fp) {
    fread(&g_val, sizeof(u16), 1, fp);
    return swap16((u8 *)&g_val);
}


u32 fread32(FILE* fp) {
    fread(&g_val, sizeof(u32), 1, fp);
    return swap32((u8 *)&g_val);
}
