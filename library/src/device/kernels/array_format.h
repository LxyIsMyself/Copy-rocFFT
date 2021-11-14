// Copyright (c) 2020 - present Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#ifndef ARRAY_FORMAT_H
#define ARRAY_FORMAT_H

#include "callback.h"
#include "common.h"
#include "rocfft/rocfft.h"

//-----------------------------------------------------------------------------
// To support planar format with template, we have the below simple conventions.

template <typename PRECISION>
struct planar
{
    planar(void* r_in, void* i_in)
        : R(static_cast<real_type_t<PRECISION>*>(r_in))
        , I(static_cast<real_type_t<PRECISION>*>(i_in))
    {
    }
    planar(const planar<PRECISION>& p) = default;
    real_type_t<PRECISION>*         R; // points to real part array
    real_type_t<PRECISION>*         I; // points to imag part array
    typedef real_type_t<PRECISION>* ptr_type;
    typedef PRECISION               complex_type;
};

template <typename PRECISION>
struct interleaved
{
    interleaved(void* in)
        : C(static_cast<PRECISION*>(in))
    {
    }
    interleaved(const interleaved<PRECISION>& p) = default;
    PRECISION*         C; // points to complex interleaved array
    typedef PRECISION* ptr_type;
    typedef PRECISION  complex_type;
};

template <typename T, CallbackType cbtype>
struct Handler
{
};

template <CallbackType cbtype>
struct Handler<interleaved<float2>, cbtype>
{
    static __host__ __device__ inline float2
        read(const interleaved<float2> in, size_t idx, void* load_cb_fn, void* load_cb_data)
    {
        auto load_cb = get_load_cb<float2, cbtype>(load_cb_fn);
        // callback might modify input, but it's otherwise const
        return load_cb(const_cast<float2*>(in.C), idx, load_cb_data, nullptr);
    }

    static __host__ __device__ inline void
        write(interleaved<float2> out, size_t idx, float2 v, void* store_cb_fn, void* store_cb_data)
    {
        auto store_cb = get_store_cb<float2, cbtype>(store_cb_fn);
        store_cb(out.C, idx, v, store_cb_data, nullptr);
    }
};

template <CallbackType cbtype>
struct Handler<interleaved<double2>, cbtype>
{
    static __host__ __device__ inline double2
        read(const interleaved<double2> in, size_t idx, void* load_cb_fn, void* load_cb_data)
    {
        auto load_cb = get_load_cb<double2, cbtype>(load_cb_fn);
        // callback might modify input, but it's otherwise const
        return load_cb(const_cast<double2*>(in.C), idx, load_cb_data, nullptr);
    }

    static __host__ __device__ inline void write(
        interleaved<double2> out, size_t idx, double2 v, void* store_cb_fn, void* store_cb_data)
    {
        auto store_cb = get_store_cb<double2, cbtype>(store_cb_fn);
        store_cb(out.C, idx, v, store_cb_data, nullptr);
    }
};

template <CallbackType cbtype>
struct Handler<planar<float2>, cbtype>
{
    static __host__ __device__ inline float2
        read(const planar<float2> in, size_t idx, void* load_cb_fn, void* load_cb_data)
    {
        float2 t;
        t.x = in.R[idx];
        t.y = in.I[idx];
        return t;
    }

    static __host__ __device__ inline void
        write(planar<float2> out, size_t idx, float2 v, void* store_cb_fn, void* store_cb_data)
    {
        out.R[idx] = v.x;
        out.I[idx] = v.y;
    }
};

template <CallbackType cbtype>
struct Handler<planar<double2>, cbtype>
{
    static __host__ __device__ inline double2
        read(const planar<double2> in, size_t idx, void* load_cb_fn, void* load_cb_data)
    {
        double2 t;
        t.x = in.R[idx];
        t.y = in.I[idx];
        return t;
    }

    static __host__ __device__ inline void
        write(planar<double2> out, size_t idx, double2 v, void* store_cb_fn, void* store_cb_data)
    {
        out.R[idx] = v.x;
        out.I[idx] = v.y;
    }
};

static bool is_complex_planar(rocfft_array_type type)
{
    return type == rocfft_array_type_complex_planar || type == rocfft_array_type_hermitian_planar;
}
static bool is_complex_interleaved(rocfft_array_type type)
{
    return type == rocfft_array_type_complex_interleaved
           || type == rocfft_array_type_hermitian_interleaved;
}

#endif
