
#pragma once

#include <vector>
#include <type_traits>

namespace tn {
namespace stitching {

template<typename T, bool USE_LITTLE_ENDIAN = true>
class NumberPacker {
#ifndef TN_PLATFORM_QNX_700_QSD_8155
    static_assert(std::is_floating_point<T>::value || std::is_integral<T>::value);
#endif
public:
    NumberPacker() {
        if (std::is_floating_point<T>::value) {
            T x = -1;
            is_little_endian = (*(uint8_t*)&x == 0);
        } else if (std::is_integral<T>::value) {
            T x = 1;
            is_little_endian = (*(uint8_t*)&x == 1);
        }
    }

    std::vector<uint8_t> ToBuffer(const T& value) const
    {
        std::vector<uint8_t> buf;
        for (size_t i = 0; i < sizeof(value); ++i) {
            int index = i;
            if (USE_LITTLE_ENDIAN != is_little_endian) {
                index = (int)sizeof(value) - i - 1;
            }
            buf.push_back(*((uint8_t*)&value + index));
        }
        return buf;
    }

    T FromBuffer(const uint8_t* buf) const
    {
        T value;
        for (size_t i = 0; i < sizeof(value); ++i) {
            int index = i;
            if (USE_LITTLE_ENDIAN != is_little_endian) {
                index = (int)sizeof(value) - i - 1;
            }
            *((char*)&value + i) = *(buf + index);
        }
        return value;
    }

private:
    bool is_little_endian;
};

// static const NumberPacker<float> g_float32_packer;
// static const NumberPacker<int32_t> g_int32_packer;

}}
