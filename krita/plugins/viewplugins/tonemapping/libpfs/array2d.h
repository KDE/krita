/**
 * @file
 * @brief PFS library - general 2d array interface
 *
 * All pfs::Array2D classes are part of pfs library. However, to
 * lessen coupling of the code with pfs library, Array2D classes are
 * declared in this separate file. Therefore it is possible to write
 * the code that implements or uses Array2D interface while it has no
 * knowledge of other pfs library classes.
 * 
 * This file is a part of PFSTOOLS package.
 * ---------------------------------------------------------------------- 
 * Copyright (C) 2003,2004 Rafal Mantiuk and Grzegorz Krawczyk
 * 
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * ---------------------------------------------------------------------- 
 * 
 * @author Rafal Mantiuk, <mantiuk@mpi-sb.mpg.de>
 *
 * $Id: array2d.h,v 1.1 2005/06/15 13:36:55 rafm Exp $
 */

#ifndef Array2D_H
#define Array2D_H

#include <assert.h>

namespace pfs
{

/**
 * @brief Interface for 2 dimensional array of floats.
 *
 * This is a thin interface of classes that hold 2 dimensional arrays
 * of floats. The interface lets to access all types of arrays in the
 * same way, regardless how data is really stored (row major, column
 * major, 2D array, array of pointers, etc.). It also simplifies
 * indexing.
 *
 * See also implementing classes.
 */
  class Array2D
    {

    public:

      /**
       * Get number of columns or, in case of an image, width.
       */
      virtual int getCols() const = 0;

      /**
       * Get number of rows or, in case of an image, height.
       */
      virtual int getRows() const = 0;


      /**
       * Access an element of the array for reading and
       * writing. Whether the given row and column are checked against
       * array bounds depends on an implementing class.
       *
       * Note, that if an Array2D object is passed as a pointer (what
       * is usually the case), to access its elements, you have to use
       * somewhat strange syntax: (*array)(row, column).
       *
       * @param col number of a column (x) within the range 0..(getCols()-1)
       * @param row number of a row (y) within the range 0..(getRows()-1)
       */
      virtual float& operator()( int col, int row ) = 0;

      /**
       * Access an element of the array for reading. Whether the given
       * row and column are checked against array bounds depends on an
       * implementing class.
       *
       * Note, that if an Array2D object is passed as a pointer (what
       * is usually the case), to access its elements, you have to use
       * somewhat strange syntax: (*array)(row, column).
       *
       * @param col number of a column (x) within the range 0..(getCols()-1)
       * @param row number of a row (y) within the range 0..(getRows()-1)
       */
      virtual const float& operator()( int col, int row ) const = 0;

      /**
       * Access an element of the array for reading and writing. This
       * is probably faster way of accessing elements than
       * operator(col, row). However there is no guarantee on the
       * order of elements as it usually depends on an implementing
       * class. The only assumption that can be make is that there are
       * exactly columns*rows elements and they are all unique.
       *
       * Whether the given index is checked against array bounds
       * depends on an implementing class.
       *
       * Note, that if an Array2D object is passed as a pointer (what
       * is usually the case), to access its elements, you have to use
       * somewhat strange syntax: (*array)(index).
       *
       * @param index index of an element within the range 0..(getCols()*getRows()-1)
       */      
      virtual float& operator()( int index ) = 0;

      /**
       * Access an element of the array for reading. This
       * is probably faster way of accessing elements than
       * operator(col, row). However there is no guarantee on the
       * order of elements as it usually depends on an implementing
       * class. The only assumption that can be make is that there are
       * exactly columns*rows elements and they are all unique.
       *
       * Whether the given index is checked against array bounds
       * depends on an implementing class.
       *
       * Note, that if an Array2D object is passed as a pointer (what
       * is usually the case), to access its elements, you have to use
       * somewhat strange syntax: (*array)(index).
       *
       * @param index index of an element within the range 0..(getCols()*getRows()-1)
       */      
      virtual const float& operator()( int index ) const = 0;

      /**
       * Each implementing class should provide its own destructor.
       */
      virtual ~Array2D()
        {
        }

    };


/**
 * @brief Two dimensional array of floats
 *
 * Holds 2D data in column-major oder. Allows easy indexing
 * and retrieving array dimensions.
 */
  class Array2DImpl: public Array2D
    {
      float *data;
      int cols, rows;
    
    public:

      Array2DImpl( int cols, int rows ) : cols( cols ), rows( rows )
        {
          data = new float[cols*rows];
        }
    
      ~Array2DImpl()
        {
          delete[] data;
        }

      inline int getCols() const { return cols; }
      inline int getRows() const { return rows; }

      inline float& operator()( int col, int row ) {
        assert( col >= 0 && col < cols );
        assert( row >= 0 && row < rows );
        return data[ col+row*cols ];
      }
      inline const float& operator()( int col, int row ) const {
        assert( col >= 0 && col < cols );
        assert( row >= 0 && row < rows );
        return data[ col+row*cols ];
      }

      inline float& operator()( int index ) {
        assert( index >= 0 && index < rows*cols );
        return data[index];
      }
      inline const float& operator()( int index ) const {
        assert( index >= 0 && index <= rows*cols );        
        return data[index];
      }

      float* getRawData() {
        return data;
      }
    
    
    };        

/**
 * Copy data from one Array2D to another. Dimensions of the arrays must be the same.
 *
 * @param from array to copy from
 * @param to array to copy to
 */
  inline void copyArray(const Array2D *from, Array2D *to)
    {
      assert( from->getRows() == to->getRows() );
      assert( from->getCols() == to->getCols() );
  
      const int elements = from->getRows()*from->getCols();
      for( int i = 0; i < elements; i++ )
        (*to)(i) = (*from)(i);
    }

/**
 * Set all elements of the array to a give value.
 *
 * @param array array to modify
 * @param value all elements of the array will be set to this value
 */
  inline void setArray(Array2D *array, const float value )
    {
      const int elements = array->getRows()*array->getCols();
      for( int i = 0; i < elements; i++ )
        (*array)(i) = value;
    }

/**
 * Perform element-by-element multiplication: z = x * y. z can be the same as x or y.
 *
 * @param z array where the result is stored
 * @param x first element of the multiplication
 * @param y second element of the multiplication
 */
  inline void multiplyArray(Array2D *z, const Array2D *x, const Array2D *y)
    {
      assert( x->getRows() == y->getRows() );
      assert( x->getCols() == y->getCols() );
      assert( x->getRows() == z->getRows() );
      assert( x->getCols() == z->getCols() );
  
      const int elements = x->getRows()*x->getCols();
      for( int i = 0; i < elements; i++ )
        (*z)(i) = (*x)(i) * (*y)(i);
    }

}

#endif
