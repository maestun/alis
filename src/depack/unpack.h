//
//  unpack.h
//  silmarils-unpacker
//

#pragma once

#include "config.h"

typedef enum {
    EUnpackErrorInput = -1,
    EUnpackErrorOutput = -2,
    EUnpackErrorFormat = -3
} EUnpackError;

/// @brief Unpacks a script file to buffer
/// @param packed_file_path full path to packed file
/// @param is_le zero if file to unpack is big-endian encoded
/// @param unpacked_buffer output buffer
/// @return unpacked file size if success, a negative value if error, or
/// zero if the input file is not packed
int unpack_script(const char* packed_file_path, u8 is_le, u8** unpacked_data);
