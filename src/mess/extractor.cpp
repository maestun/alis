//
//  extractor.cpp
//  silm-extract
//
//  Created on 08.08.2022.
//

#include "extractor.h"

#include <filesystem>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <set>
#include <ranges>

extern "C"
{
    #include "alis.h"
    #include "alis_private.h"
    #include "utils.h"
}

using std::cout; using std::cin;
using std::endl; using std::string;

void extract_resources(uint8_t *buffer, int length, alis_rsrc ***entries, int *entry_count)
{
    *entry_count = 0;
    
    extractor ex;
    ex.extract_resources(buffer, length);
    
    if (entries)
    {
        *entries = (alis_rsrc **)malloc(sizeof(alis_rsrc *) * ex._entry_map.size());
        
        int i = 0;
        for (auto & entryPair : ex._entry_map)
        {
            (*entries)[i++] = entryPair.second;
        }
    }
    
    *entry_count = (int)ex._entry_map.size();
}


extractor::extractor()
{
}

extractor::~extractor()
{
//    // cleanup
//
//    for (auto & entryPair : _entry_map)
//    {
//        Entry *e = entryPair.second;
//        if (e->buffer.data)
//            delete [] e->buffer.data;
//
//        delete e;
//    }
//
//    _entry_map.clear();
}

const char *string_for_type(data_type type)
{
    switch (type)
    {
        case none:
            return "none";
        case image2:
            return "bitmap 2 bit";
        case image4ST:
            return "bitmap 4 bit";
        case image4:
            return "bitmap 4 bit (using 8 bit palette)";
        case image8:
            return "bitmap 8 bit";
        case video:
            return "video";
        case palette4:
            return "4 bit palette";
        case palette8:
            return "8 bit palette";
        case composite:
            return "composite";
        case rectangle:
            return "rectangle";
        default:
            break;
    };

    return "unknown";
}

void log_data(const uint8_t *p, int f, int s0, int s1, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    char t0[4] = "]";
    if (s0 > 24)
    {
        s0 = 24;
        strcpy(t0, "...");
    }

    char t1[4] = "]";
    if (s1 > 24)
    {
        s1 = 24;
        strcpy(t1, "...");
    }

    s1 += s0 + f;
    s0 += f;

    if (f < s0)
    {
        printf("[");
        
        for (int i = f; i < s0; i++)
            printf(" %.2x", p[i]);
        
        printf(" %s", t0);
    }
    
    if (s0 < s1)
    {
        printf("[");
        
        for (int i = s0; i < s1; i++)
            printf(" %.2x", p[i]);
        
        printf(" %s", t1);
    }

    printf("\n");
}

int extractor::asset_size(const uint8_t *buffer)
{
    int h0 = buffer[0];
    int h1 = buffer[1];

    switch (h0)
    {
        case 0x01:
        {
            return 2 * 2;
        }
        case 0x00:
        case 0x02:
        {
            int width = read16(buffer + 2, alis.platform) + 1;
            int height = read16(buffer + 4, alis.platform) + 1;
//            return width / (_platform_ext == "mo" ? 4 :  2) * height;
            return (width / 2) * height;
        }
        case 0x10:
        case 0x12:
        {
            int width = read16(buffer + 2, alis.platform) + 1;
            int height = read16(buffer + 4, alis.platform) + 1;
            return (width / 2) * height;
        }
        case 0x14:
        case 0x16:
        {
            int width = read16(buffer + 2, alis.platform) + 1;
            int height = read16(buffer + 4, alis.platform) + 1;
            return width * height;
        }
        case 0x40:
        {
            return read32(buffer + 2, alis.platform) - 1;
        }
        case 0xfe:
        {
            return h1 == 0x00 ? 32 : (h1 + 1) * 3;
        }
        case 0xff:
        {
            return h1 * 8;
        }
        case 0x100:
        case 0x101:
        case 0x102:
        case 0x104:
        {
            return read32(buffer + 2, alis.platform) - 1;
        }

        default:
        {
            break;
        }
    }

    return 0;
}

bool extractor::does_it_overlap(const uint8_t *buffer, int address, int entries, int skip_entry, int location, int length)
{
    for (int e = 0; e < entries; e ++)
    {
        if (skip_entry != e)
        {
            uint32_t position = address + e * 4;
            uint32_t eloc = position + 2 + read32(buffer + position, alis.platform);
            uint32_t esize = asset_size(buffer + eloc - 2);
            
            if (eloc + esize > location && eloc < location + length)
            {
                return true;
            }
        }
    }
    
    return false;
}

bool extractor::does_it_fit(const uint8_t *buffer, int length, int a, int e)
{
    uint32_t value;

    for (int i = 0; i < e; i ++)
    {
        uint32_t position = a + i * 4;
        value = read32(buffer + position, alis.platform);
        if (value <= 0 || (position + 2 + value) >= length)
        {
            return false;
        }
    }
    
    return true;
}

int compare_arrays(const uint8_t *a, const uint8_t *b, int n)
{
    for (int i = 0; i < n; i++)
    {
        if (a[i] != b[i])
            return 0;
    }

    return 1;
}

bool extractor::find_assets(const uint8_t *buffer, int length, uint32_t& address, uint32_t& entries, uint32_t& mod)
{
    uint32_t location;
    int add = buffer[0] ? 6 : 0;

    // look for graphics
    location = read32(buffer + 0xe + add, alis.platform) + add;
    address = read32(buffer + location, alis.platform) + location;
    entries = read16(buffer + location + 4, alis.platform);
    if (entries)
    {
        cout << "Found gfx address block [0x" << std::hex << std::setw(6) << std::setfill('0') << location << "]" << " [0x" << std::hex << std::setw(6) << std::setfill('0') << address << "]" << endl;
        mod = 0;
        return true;
    }
    
    // look for sound
    address = read32(buffer + 0xc + location, alis.platform) + location;
    entries = read32(buffer + 0x10 + location, alis.platform);
    if (entries)
    {
        cout << "Found snd address block [0x" << std::hex << std::setw(6) << std::setfill('0') << location << "]" << " [0x" << std::hex << std::setw(6) << std::setfill('0') << address << "]" << endl;
        mod = 0x100;
        return true;
    }

    cout << "Can't found address block" << endl;
    return false;
}

Entry *extractor::get_entry_data(Buffer& script, uint32_t address, uint32_t entries, uint32_t index, uint32_t mod)
{
    if (_entry_map[index])
    {
        return _entry_map[index];
    }

    uint32_t position = address + index * 4;
    uint32_t value = read32(script.data + position, alis.platform);
    uint32_t location = position + 2 + value;

    int h0 = mod + script.data[location - 2];
    int h1 = script.data[location - 1];
    
    switch (h0)
    {
        case 0x01:
        {
            uint32_t *params = new uint32_t[4];
            params[0] = h0;
            params[1] = h1;
            params[2] = read16(script.data + location, alis.platform) + 1;
            params[3] = read16(script.data + location + 2, alis.platform) + 1;
            return (_entry_map[index] = new Entry(data_type::rectangle, params, Buffer()));
        }
        case 0x00:
        case 0x02:
        {
            int width = read16(script.data + location, alis.platform) + 1;
            int height = read16(script.data + location + 2, alis.platform) + 1;
            int at = location + 4;
            int to = 0;
            
            uint32_t *params = new uint32_t[4];
            params[0] = h0;
            params[1] = h1;
            params[2] = width;
            params[3] = height;

            if (does_it_overlap(script.data, address, entries, index, location, (width / 2) * height) == false && at + (width / 2) * height < script.size)
            {
                uint8_t *data = new uint8_t[width * height];
                
                for (int x = 0; x < (width / 2) * height; x++, at++)
                {
                    uint8_t r = script.data[at];
                    uint8_t a = ((r & 0b11110000) >> 4);
                    uint8_t b = (r & 0b00001111);
                    
                    data[to++] = a;
                    data[to++] = b;
                }
                
                return (_entry_map[index] = new Entry(data_type::image4ST, params, Buffer(data, width * height)));
            }
            break;
        }
        case 0x10:
        case 0x12:
        {
            int width = read16(script.data + location, alis.platform) + 1;
            int height = read16(script.data + location + 2, alis.platform) + 1;
            int at = location + 4 + 2;
            int to = 0;

            int palIdx = script.data[location + 4];
            int clrIdx = script.data[location + 5];

            uint32_t *params = new uint32_t[6];
            params[0] = h0;
            params[1] = h1;
            params[2] = width;
            params[3] = height;
            params[4] = palIdx;
            params[5] = clrIdx;

            uint8_t *data = new uint8_t[width * height];

            for (int x = 0; x < (width / 2) * height; x++, at++)
            {
                uint8_t r = script.data[at];
                uint8_t a = palIdx + ((r & 0b11110000) >> 4);
                uint8_t b = palIdx + (r & 0b00001111);
                
                data[to++] = a;
                data[to++] = b;
            }
            
            return (_entry_map[index] = new Entry(data_type::image4, params, Buffer(data, width * height)));
        }
        case 0x14:
        case 0x16:
        {
            int width = read16(script.data + location, alis.platform) + 1;
            int height = read16(script.data + location + 2, alis.platform) + 1;

            int palIdx = script.data[location + 4];
            int clrIdx = script.data[location + 5];

            uint32_t *params = new uint32_t[6];
            params[0] = h0;
            params[1] = h1;
            params[2] = width;
            params[3] = height;
            params[4] = palIdx;
            params[5] = clrIdx;

            uint8_t *data = new uint8_t[width * height];
            memcpy(data, script.data + location + 4 + 2, width * height);
                
            return (_entry_map[index] = new Entry(data_type::image8, params, Buffer(data, width * height)));
        }
        case 0x40:
        {
            uint32_t size = read32(script.data + location, alis.platform);
            char *fliname = (char *)&script.data[location + 4];
            cout << "FLI video (" << fliname << ") " << std::dec << size << " bytes [";

            uint32_t size2 = (*(uint32_t *)(&script.data[location + 30]));
            uint16_t frames = (*(uint16_t *)(&script.data[location + 36]));

            cout << "size: " << std::dec << size << " frames: "  << std::dec << frames << "]" << endl;
            
            uint8_t *data = new uint8_t[size];
            memcpy(data, &script.data[location + 30], size);
            
            uint32_t *params = new uint32_t[4];
            params[0] = h0;
            params[1] = h1;
            params[2] = size;
            params[3] = frames;

            return (_entry_map[index] = new Entry(data_type::video, params, Buffer(data, size)));
        }
        case 0xfe:
        {
            uint8_t *palette_data = new uint8_t[256 * 3];
            memset(palette_data, 0, 256 * 3);

            int to = 0;

            if (h1 == 0x00)
            {
                uint32_t *params = new uint32_t[3];
                params[0] = h0;
                params[1] = h1;
                params[2] = 16;

                for (int f = 0; f < 16; f++)
                {
                    uint8_t r = script.data[location + f * 2 + 0];
                    r = (r & 0b00000111) << 5;
                    uint8_t g = script.data[location + f * 2 + 1];
                    g = (g >> 4) << 5;
                    uint8_t b = script.data[location + f * 2 + 1];
                    b = (b & 0b00000111) << 5;
                    
                    palette_data[to++] = r;
                    palette_data[to++] = g;
                    palette_data[to++] = b;
                }
                
                return (_entry_map[index] = new Entry(data_type::palette4, params, Buffer(palette_data, 256 * 3)));
            }
            else
            {
                uint32_t *params = new uint32_t[3];
                params[0] = h0;
                params[1] = h1;
                params[2] = h1;
 
                for (int f = 0; f < h1 + 1; f++)
                {
                    palette_data[to++] = script.data[2 + location + (f * 3) + 0];
                    palette_data[to++] = script.data[2 + location + (f * 3) + 1];
                    palette_data[to++] = script.data[2 + location + (f * 3) + 2];
                }
                
                return (_entry_map[index] = new Entry(data_type::palette8, params, Buffer(palette_data, 256 * 3)));
            }
        }
        case 0xff:
        {
            uint32_t *params = new uint32_t[2];
            params[0] = h0;
            params[1] = h1;

            uint8_t *data = NULL;

            if (h1 > 0)
            {
                data = new uint8_t[h1 * 8];

                std::map<int, std::vector<int>> layers;

                // HACK: we don't know where on screen script wants to draw
                // so, to actually display anything, check if positions fit screen, if not center it.
                
                for (int b = 0; b < h1; b++)
                {
                    uint8_t index = script[b * 8 + location + (alis.platform.is_little_endian ? 0 : 1)];
                    int16_t x = read16(script.data + b * 8 + location + 2, alis.platform);
                    int16_t d = read16(script.data + b * 8 + location + 4, alis.platform);
                    int16_t y = read16(script.data + b * 8 + location + 6, alis.platform);
                    layers[d].push_back(b);
                }

                for (auto it = layers.rbegin(); it != layers.rend(); it++)
                {
                    for (auto &b: it->second)
                    {
                        memcpy(data + b * 8, script.data + b * 8 + location, 8);
                    }
                }
            
//                memcpy(data, script.data + location, h1 * 8);
            }
            
            return (_entry_map[index] = new Entry(data_type::composite, params, Buffer(data, h1 * 8)));
        }
        case 0x100:
        case 0x104:
        {
            uint32_t len = read32(script.data + location, alis.platform) - 1;

            uint32_t *params = new uint32_t[3];
            params[0] = h0;
            params[1] = h1;
            params[2] = len;
            params[3] = read32(script.data + location, alis.platform);

            uint8_t *data = new uint8_t[len];
            memcpy(data, script.data + location + 4, len);

            return (_entry_map[index] = new Entry(data_type::pattern, params, Buffer(data, len)));
        }
        case 0x101:
        case 0x102:
        {
            uint32_t len = read32(script.data + location, alis.platform) - 1;

            uint32_t *params = new uint32_t[3];
            params[0] = h0;
            params[1] = h1;
            params[2] = len;

            uint8_t *data = new uint8_t[len];
            memcpy(data, script.data + location + 4, len);

            return (_entry_map[index] = new Entry(data_type::sample, params, Buffer(data, len)));
        }

        default:
        {
            if (h0 > 0x100)
            {
                cout << "Unknown sound type!" << endl;
            }
            
            break;
        }
    }
    
    return (_entry_map[index] = new Entry());
}

void extractor::extract_resources(uint8_t *buffer, int length)
{
    uint32_t value = 0;
    uint32_t location = 0;
    uint32_t address = 0;
    uint32_t entries = 0;
    uint32_t mod = 0;

    // find adresses for all assets in file
    
    if (find_assets(buffer, length, address, entries, mod))
    {
        cout << " containing " << std::dec << entries << " assets" << endl;
    }
    else
    {
        return;
    }
    
    Buffer script(buffer, length);

    std::vector<Entry *> entryList;

    for (int i = 0; i < entries; i ++)
    {
        entryList.push_back(NULL);
        
        uint32_t position = address + i * 4;
        value = read32(buffer + position, alis.platform);
        
        location = position + 2 + value;
        if (value > 0 && location < length)
        {
            entryList[i] = get_entry_data(script, address, entries, i, mod);
        }
        else
        {
            cout << "OUT OF BOUNDS!" << endl;
        }
    }
    
    for (int i = 0; i < entries; i ++)
    {
        uint32_t position = address + i * 4;
        value = read32(buffer + position, alis.platform);

        location = position + 2 + value;

        printf("Entry %d [0x%.6x => 0x%.6x]: ", i, position, location);

        Entry *entry = entryList[i];
        switch (entry->type)
        {
            case data_type::palette4:
            {
                cout << "palette 16" << endl;
//                    entry->buffer
                break;
            }
            case data_type::palette8:
            {
                cout << "palette 256" << endl;
//                    entry->buffer
                break;
            }
            case data_type::image2:
            case data_type::image4ST:
            case data_type::image4:
            case data_type::image8:
            {
                log_data(buffer, location - 2, 2, 0, "%s, %d x %d ", string_for_type(entry->type), entry->params[2], entry->params[3]);
//                    entry->buffer
                break;
            }
            case data_type::video:
            {
                log_data(buffer, location - 2, 2, 4, "FLI video [size: %d frames: %d]", entry->params[2], entry->params[3]);
//                    entry->buffer
                break;
            }
            case data_type::composite:
            {
                log_data(buffer, location - 2, 2, 0, "%d draw instructions ", entry->params[1]);
                
                for (int b = 0; b < entry->params[1]; b++)
                {
                    uint8_t cmd = entry->buffer.data[b * 8 + (alis.platform.is_little_endian ? 1 : 0)];
                    uint8_t index = entry->buffer.data[b * 8 + (alis.platform.is_little_endian ? 0 : 1)];
                    int16_t x = read16(entry->buffer.data + b * 8 + 2, alis.platform);
                    int16_t o = read16(entry->buffer.data + b * 8 + 4, alis.platform);
                    int16_t y = read16(entry->buffer.data + b * 8 + 6, alis.platform);

                    Entry *e = entryList[index];
                    if (e->type != none && e->type != unknown)
                    {
                        uint32_t width = e->params[2];
                        uint32_t height = e->params[3];
                        cout << "  cmd: " << std::dec << std::setw(3) << (int)cmd << " index: " << std::dec << std::setw(3) << (int)index << " type: " << string_for_type(e->type) << " x " << std::dec << x << " y " << std::dec << y  << " w " << std::dec << width << " h " << std::dec << height << " order: " << std::dec << o << endl;
                    }
                    else
                    {
                        cout << "  cmd: " << std::dec << std::setw(3) << (int)cmd << " index: " << std::dec << std::setw(3) << (int)index << " type: " << string_for_type(e->type) << " x " << std::dec << x << " y " << std::dec << y  << " w ? h ? order: " << std::dec << o << endl;
                    }
                }
                //                    entry->buffer
                break;
            }
            case data_type::rectangle:
            {
                log_data(buffer, location - 2, 2, 4, "rectangle Id %d, %d x %d ", entry->params[1], entry->params[2], entry->params[3]);
                break;
            }
            case data_type::pattern:
            {
                log_data(buffer, location - 2, 2, 4, "Possible mod pattern? (%d bytes)", entry->buffer.size);
                break;
            }
            case data_type::sample:
            {
                int freq = entry->params[1];
                if (freq < 3 || freq > 23)
                    freq = 8;
                
                int len = entry->buffer.size;

                log_data(buffer, location - 2, 2, 4, "PCM sample %d bytes %d Hz", len, freq);
                break;
            }
            default:
            {
                log_data(buffer, location - 2, 2, 8, "unknown ");
                break;
            }
        }
    }

//    // cleanup
//
//    for (auto & entryPair : _entry_map)
//    {
//        Entry *e = entryPair.second;
//        if (e->buffer.data)
//            delete [] e->buffer.data;
//
//        delete e;
//    }
//
//    _entry_map.clear();
}
