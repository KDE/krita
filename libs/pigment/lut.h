/*
 *  Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _LUT_H_
#define _LUT_H_

template<typename _InputT_>
class LutKey;

template<typename _InputT_>
class FullLutKey;

/**
 * Provide an implementation for a look-up table for a function. Do not use directly, instead
 * use @ref Lut when you are not interested in having the full look-up table or for floating
 * point, or use @ref FullLut for 8bits and 16bits when you want to have a full Lut.
 *
 * @code
 * struct MyFunction {
 *  inline static int compute(int i)
 *  {
 *    return 1-i;
 *  }
 * }
 * Lut<MyFunction, int, int> myLut;
 * @endcode
 */
template<typename _FunctionT_, typename _OutputT_, typename _InputT_, typename _LutKeyT_ >
class BaseLut {
  private:
    /**
     * Initialization of the table.
     */
    inline void init()
    {
      int size = m_key.size();
      m_table = new _OutputT_[size];
      for(int i = 0; i < size; ++i)
      {
        m_table[i] = m_function(m_key.keyToInput(i));
      }
    }
  public:
    /**
     * Create the lut with the specific key.
     */
    inline BaseLut(_LutKeyT_ key, _FunctionT_ function = _FunctionT_()) : m_key(key), m_function(function)
    {
      init();
    }
    inline ~BaseLut() {
      delete[] m_table;
    }
  public:
    /**
     * @return the function value for parameter @p i
     */
    inline _OutputT_ operator()(_InputT_ i) const
    {
      if(m_key.inrange(i))
      {
        return m_table[m_key.inputToKey(i)];
      }
      return m_function(i);
    }
  private:
    _OutputT_* m_table;
    _LutKeyT_ m_key;
    _FunctionT_ m_function;
};

/**
 * This Lut is limited to a range of values.
 */
template<typename _FunctionT_, typename _OutputT_, typename _InputT_>
class Lut : public BaseLut<_FunctionT_, _OutputT_, _InputT_, LutKey<_InputT_> > {
  public:
    /**
     * Create the lut between @p _min and @p _max .
     */
    inline Lut(_InputT_ _min, _InputT_ _max, _FunctionT_ function = _FunctionT_()) :
        BaseLut<_FunctionT_, _OutputT_, _InputT_, LutKey<_InputT_> >( LutKey<_InputT_>(_min, _max), function)
    {
    }
    inline Lut(LutKey<_InputT_> key, _FunctionT_ function = _FunctionT_()) :
        BaseLut<_FunctionT_, _OutputT_, _InputT_, LutKey<_InputT_> >( key, function)
    {
    }
};

/**
 * This Lut has precomputed values for all elements.
 */
template<typename _FunctionT_, typename _OutputT_, typename _InputT_>
class FullLut : public BaseLut<_FunctionT_, _OutputT_, _InputT_, FullLutKey<_InputT_> > {
  public:
    inline FullLut( _FunctionT_ function = _FunctionT_()) :
        BaseLut<_FunctionT_, _OutputT_, _InputT_, FullLutKey<_InputT_> >( FullLutKey<_InputT_>(), function)
    {
    }
};

#ifdef _USE_QT_TYPES_
typedef quint8 lut_uint8;
typedef quint16 lut_uint16;
typedef quint32 lut_uint32;
#else
#include <stdint.h>
typedef uint8_t lut_uint8;
typedef uint16_t lut_uint16;
typedef uint32_t lut_uint32;
#endif

// integer specialization

#define PARTIAL_LUT_INT_SPECIALIZATION(_INT_TYPE_)                                \
  template<>                                                                      \
  class LutKey<_INT_TYPE_> {                                               \
    public:                                                                       \
      LutKey<_INT_TYPE_>(_INT_TYPE_ min, _INT_TYPE_ max) : m_min(min), m_max(max) \
      {                                                                           \
      }                                                                           \
    public:                                                                       \
      inline int inputToKey(_INT_TYPE_ i) const                                   \
      {                                                                           \
        return i - m_min;                                                         \
      }                                                                           \
      inline _INT_TYPE_ keyToInput(int k) const                                   \
      {                                                                           \
        return k + m_min;                                                         \
      }                                                                           \
      inline bool inrange(_INT_TYPE_ i) const                                     \
      {                                                                           \
        return i >= m_min && i <= m_max;                                          \
      }                                                                           \
      inline _INT_TYPE_ minimum() const                                           \
      {                                                                           \
        return m_min;                                                             \
      }                                                                           \
      inline _INT_TYPE_ maximum() const                                           \
      {                                                                           \
        return m_max;                                                             \
      }                                                                           \
      inline int size() const                                                     \
      {                                                                           \
        return m_max - m_min + 1;                                                 \
      }                                                                           \
    private:                                                                      \
      _INT_TYPE_ m_min, m_max;                                                    \
  };

PARTIAL_LUT_INT_SPECIALIZATION(lut_uint8)
PARTIAL_LUT_INT_SPECIALIZATION(lut_uint16)
PARTIAL_LUT_INT_SPECIALIZATION(lut_uint32)

#define FULL_LUT_INT_SPECIALIZATION(_INT_TYPE_, _MIN_, _MAX_)                     \
  template<>                                                                      \
  class FullLutKey<_INT_TYPE_> {                                                  \
    public:                                                                       \
      FullLutKey<_INT_TYPE_>()                                                    \
      {                                                                           \
      }                                                                           \
    public:                                                                       \
      inline int inputToKey(_INT_TYPE_ i) const                                   \
      {                                                                           \
        return i - _MIN_;                                                         \
      }                                                                           \
      inline _INT_TYPE_ keyToInput(int k) const                                   \
      {                                                                           \
        return k + _MIN_;                                                         \
      }                                                                           \
      inline bool inrange(_INT_TYPE_ ) const                                      \
      {                                                                           \
        return true;                                                              \
      }                                                                           \
      inline _INT_TYPE_ minimum() const                                           \
      {                                                                           \
        return _MIN_;                                                             \
      }                                                                           \
      inline _INT_TYPE_ maximum() const                                           \
      {                                                                           \
        return _MAX_;                                                             \
      }                                                                           \
      inline int size() const                                                     \
      {                                                                           \
        return _MAX_ - _MIN_ + 1;                                                 \
      }                                                                           \
    private:                                                                      \
  };

FULL_LUT_INT_SPECIALIZATION(lut_uint8, 0, 255)
FULL_LUT_INT_SPECIALIZATION(lut_uint16, 0, 65535)

// float specialization

/**
 * This provide an implementation for a LutKey for floating point input values.
 *
 * Based on "High-speed Conversion of Floating Point Images to 8-bit" by Bill Spitzaks
 * (http://mysite.verizon.net/spitzak/conversion/)
 */
template<>
class LutKey<float> {
  public:
    union IFNumber {
      lut_uint32 i;
      float f;
    };
  public:
    LutKey<float>(float min, float max, float precision) : m_min(min), m_max(max), m_precision(precision)
    {
      // Those values where computed using the test_linear and setting the shift and then using
      // the standard deviation.
      if (precision <= 0.000011809f) {
        m_min =  1;
        m_max = -1;
      }
      else if (precision <= 0.0000237291f) m_shift =  8;
      else if (precision <= 0.0000475024f) m_shift =  9;
      else if (precision <= 0.0000948575f) m_shift = 10;
      else if (precision <= 0.00019013f) m_shift = 11;
      else if (precision <= 0.000379523f) m_shift = 12;
      else if (precision <= 0.000758431f) m_shift = 13;
      else if (precision <= 0.00151891f) m_shift = 14;
      else if (precision <= 0.00303725f) m_shift = 15;
      else m_shift = 16;

      if ( 0.0 <= m_min && m_min <= precision)
        m_min = precision;
      if ( -precision <= m_max && m_max <= 0.0)
        m_max = -precision;
      
      IFNumber uf;
      
      if(m_min > 0 && m_max > 0)
      {
        uf.f = m_min;
        m_tMin_p = uf.i >> m_shift;
        uf.f = m_max;
        m_tMax_p = uf.i >> m_shift;
        m_tMin_n = m_tMax_p;
        m_tMax_n = m_tMax_p;
     } else if( m_max < 0)
      {
        uf.f = m_min;
        m_tMax_n = uf.i >> m_shift;
        uf.f = m_max;
        m_tMin_n = uf.i >> m_shift;
        m_tMin_p = m_tMax_n;
        m_tMax_p = m_tMax_n;
      } else { // m_min <0 && m_max > 0
        uf.f = precision;
        m_tMin_p = uf.i >> m_shift;
        uf.f = m_max;
        m_tMax_p = uf.i >> m_shift;
        uf.f = -precision;
        m_tMin_n = uf.i >> m_shift;
        uf.f = m_min;
        m_tMax_n = uf.i >> m_shift;
      }
      m_diff_p = m_tMax_p - m_tMin_p;
    }
  public:
    inline int inputToKey(float i) const
    {
      IFNumber uf;
      uf.f = i;
      int k = (uf.i >> m_shift);
      if(k <= m_tMax_p)
      {
        return k - m_tMin_p;
      } else {
        return k - m_tMin_n + m_diff_p;
      }
    }
    inline float keyToInput(int k) const
    {
      IFNumber uf;
      if( k <= m_diff_p ) {
        uf.i = ((k + m_tMin_p) << m_shift);
      } else {
        uf.i = ((k + m_tMin_n - m_diff_p ) << m_shift);
      }
      return uf.f;
    }
    inline bool inrange(float i) const
    {
       return i >= m_min && i <= m_max && (i < -m_precision || i > m_precision);
    }
    inline float minimum() const
    {
      return m_min;
    }
    inline float maximum() const
    {
      return m_max;
    }
    inline int size() const
    {
      return m_diff_p + m_tMax_n - m_tMin_p + 1;
    }
  private:
    float m_min, m_max, m_precision;
    int m_tMin_p, m_tMax_p, m_tMin_n, m_tMax_n, m_diff_p;
    int m_shift;
};

#endif
