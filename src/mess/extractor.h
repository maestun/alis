//
//  extractor.hpp
//  silm-extract
//
//  Created on 08.08.2022.
//

#ifndef extractor_hpp
#define extractor_hpp

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef enum data_type {
    
    none        = 0,
    image2      = 1,
    image4ST    = 2,
    image4      = 3,
    image8      = 4,
    video       = 5,
    palette4    = 6,
    palette8    = 7,
    composite   = 8,
    rectangle   = 9,
    sample      = 10,
    pattern     = 11,
    unknown     = 12
} data_type;

typedef struct alis_buffer {
  
    uint8_t *data;
    uint32_t size;
} alis_buffer;

typedef struct alis_rsrc {
  
    uint32_t *params;
    
    data_type type;
    alis_buffer buffer;
} alis_rsrc;

void extract_resources(uint8_t *buffer, int length, alis_rsrc ***entries, int *entry_count);

#ifdef __cplusplus
}

#include <filesystem>
#include <map>
#include <stdio.h>
#include <string>

class Buffer : public alis_buffer {
public:

    Buffer() { data = NULL; size = 0; }
    Buffer(uint8_t *d, uint32_t s) { data = d; size = s; }

    uint8_t& operator[] (int x) { return data[x]; }
};

class Entry : public alis_rsrc {
public:

    Entry() { type = none; buffer = Buffer(); params = NULL; };
    Entry(data_type t, uint32_t *p, const Buffer& b) { type = t; params = p; buffer = b; };
};

class extractor {
    
public:
    
    extractor();
    ~extractor();
    
    void extract_resources(uint8_t *buffer, int length);
    std::map<int, Entry *> _entry_map;

private:
    
    int asset_size(const uint8_t *buffer);
    bool does_it_overlap(const uint8_t *buffer, int address, int entries, int skip_entry, int location, int length);
    bool does_it_fit(const uint8_t *buffer, int length, int a, int e);
    bool find_assets(const uint8_t *buffer, int length, uint32_t& address, uint32_t& entries, uint32_t& mod);
    Entry *get_entry_data(Buffer& script, uint32_t address, uint32_t entries, uint32_t index, uint32_t mod);
};

#endif
#endif /* extractor_hpp */
