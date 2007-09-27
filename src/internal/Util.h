// This file is part of Eigen, a lightweight C++ template library
// for linear algebra. Eigen itself is part of the KDE project.
//
// Copyright (C) 2006-2007 Benoit Jacob <jacob@math.jussieu.fr>
//
// Eigen is free software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation; either version 2 or (at your option) any later version.
//
// Eigen is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License along
// with Eigen; if not, write to the Free Software Foundation, Inc., 51
// Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. This exception does not invalidate any other reasons why a work
// based on this file might be covered by the GNU General Public License.

#ifndef EIGEN_UTIL_H
#define EIGEN_UTIL_H

#include <iostream>
#include <complex>
#include <cassert>

#undef minor

#define EIGEN_UNUSED(x) (void)x
#define EIGEN_CHECK_RANGES(matrix, row, col) \
  assert(row >= 0 && row < (matrix).rows() && col >= 0 && col < (matrix).cols())
#define EIGEN_CHECK_ROW_RANGE(matrix, row) \
  assert(row >= 0 && row < (matrix).rows())
#define EIGEN_CHECK_COL_RANGE(matrix, col) \
  assert(col >= 0 && col < (matrix).cols())

//forward declarations
template<typename _Scalar, int _Rows, int _Cols> class EiMatrix;
template<typename MatrixType> class EiMatrixRef;
template<typename MatrixType> class EiRow;
template<typename MatrixType> class EiColumn;
template<typename MatrixType> class EiMinor;
template<typename MatrixType> class EiBlock;
template<typename Lhs, typename Rhs> class EiSum;
template<typename Lhs, typename Rhs> class EiDifference;
template<typename Lhs, typename Rhs> class EiMatrixProduct;
template<typename MatrixType> class EiScalarProduct;

template<typename T> struct EiForwardDecl
{
  typedef T Ref;
};

template<typename _Scalar, int _Rows, int _Cols>
struct EiForwardDecl<EiMatrix<_Scalar, _Rows, _Cols> >
{
  typedef EiMatrixRef<EiMatrix<_Scalar, _Rows, _Cols> > Ref;
};

const int EiDynamic = -1;

#define EIGEN_UNUSED(x) (void)x

#define EIGEN_INHERIT_ASSIGNMENT_OPERATOR(Derived, Op) \
template<typename OtherScalar, typename OtherDerived> \
Derived& operator Op(const EiObject<OtherScalar, OtherDerived>& other) \
{ \
  return EiObject<Scalar, Derived>::operator Op(other); \
} \
Derived& operator Op(const Derived& other) \
{ \
  return EiObject<Scalar, Derived>::operator Op(other); \
}

#define EIGEN_INHERIT_SCALAR_ASSIGNMENT_OPERATOR(Derived, Op) \
template<typename Other> \
Derived& operator Op(const Other& scalar) \
{ \
  return EiObject<Scalar, Derived>::operator Op(scalar); \
}

#define EIGEN_INHERIT_ASSIGNMENT_OPERATORS(Derived) \
EIGEN_INHERIT_ASSIGNMENT_OPERATOR(Derived, =) \
EIGEN_INHERIT_ASSIGNMENT_OPERATOR(Derived, +=) \
EIGEN_INHERIT_ASSIGNMENT_OPERATOR(Derived, -=) \
EIGEN_INHERIT_SCALAR_ASSIGNMENT_OPERATOR(Derived, *=) \
EIGEN_INHERIT_SCALAR_ASSIGNMENT_OPERATOR(Derived, /=)

#endif // EIGEN_UTIL_H
