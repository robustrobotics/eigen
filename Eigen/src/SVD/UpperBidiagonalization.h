// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2010 Benoit Jacob <jacob.benoit.1@gmail.com>
//
// Eigen is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or (at your option) any later version.
//
// Alternatively, you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// Eigen is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License or the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License and a copy of the GNU General Public License along with
// Eigen. If not, see <http://www.gnu.org/licenses/>.

#ifndef EIGEN_BIDIAGONALIZATION_H
#define EIGEN_BIDIAGONALIZATION_H

template<typename _MatrixType> class UpperBidiagonalization
{
  public:

    typedef _MatrixType MatrixType;
    enum {
      RowsAtCompileTime = MatrixType::RowsAtCompileTime,
      ColsAtCompileTime = MatrixType::ColsAtCompileTime,
      ColsAtCompileTimeMinusOne = ei_decrement_size<ColsAtCompileTime>::ret
    };
    typedef typename MatrixType::Scalar Scalar;
    typedef typename MatrixType::RealScalar RealScalar;
    typedef Matrix<Scalar, 1, ColsAtCompileTime> RowVectorType;
    typedef Matrix<Scalar, RowsAtCompileTime, 1> ColVectorType;
    typedef BandMatrix<RealScalar, ColsAtCompileTime, ColsAtCompileTime, 1, 0> BidiagonalType;
    typedef Matrix<Scalar, ColsAtCompileTime, 1> DiagVectorType;
    typedef Matrix<Scalar, ColsAtCompileTimeMinusOne, 1> SuperDiagVectorType;
    typedef HouseholderSequence<
              MatrixType,
              CwiseUnaryOp<ei_scalar_conjugate_op<Scalar>, Diagonal<MatrixType,0> >
            > HouseholderUSequenceType;
    typedef HouseholderSequence<
              MatrixType,
              Diagonal<MatrixType,1>,
              OnTheRight
            > HouseholderVSequenceType;
    
    /**
    * \brief Default Constructor.
    *
    * The default constructor is useful in cases in which the user intends to
    * perform decompositions via Bidiagonalization::compute(const MatrixType&).
    */
    UpperBidiagonalization() : m_householder(), m_bidiagonal(), m_isInitialized(false) {}

    UpperBidiagonalization(const MatrixType& matrix)
      : m_householder(matrix.rows(), matrix.cols()),
        m_bidiagonal(matrix.cols(), matrix.cols()),
        m_isInitialized(false)
    {
      compute(matrix);
    }
    
    UpperBidiagonalization& compute(const MatrixType& matrix);
    
    const MatrixType& householder() const { return m_householder; }
    const BidiagonalType& bidiagonal() const { return m_bidiagonal; }
    
    HouseholderUSequenceType householderU() const
    {
      ei_assert(m_isInitialized && "UpperBidiagonalization is not initialized.");
      return HouseholderUSequenceType(m_householder, m_householder.diagonal().conjugate());
    }

    HouseholderVSequenceType householderV() // const here gives nasty errors and i'm lazy
    {
      ei_assert(m_isInitialized && "UpperBidiagonalization is not initialized.");
      return HouseholderVSequenceType(m_householder, m_householder.template diagonal<1>(),
                                      false, m_householder.cols()-1, 1);
    }
    
  protected:
    MatrixType m_householder;
    BidiagonalType m_bidiagonal;
    bool m_isInitialized;
};

template<typename _MatrixType>
UpperBidiagonalization<_MatrixType>& UpperBidiagonalization<_MatrixType>::compute(const _MatrixType& matrix)
{
  int rows = matrix.rows();
  int cols = matrix.cols();
  
  ei_assert(rows >= cols && "UpperBidiagonalization is only for matrices satisfying rows>=cols.");
  
  m_householder = matrix;

  ColVectorType temp(rows);

  for (int k = 0; /* breaks at k==cols-1 below */ ; ++k)
  {
    int remainingRows = rows - k;
    int remainingCols = cols - k - 1;

    // construct left householder transform in-place in m_householder
    m_householder.col(k).tail(remainingRows)
                 .makeHouseholderInPlace(m_householder.coeffRef(k,k),
                                         m_bidiagonal.template diagonal<0>().coeffRef(k));
    // apply householder transform to remaining part of m_householder on the left
    m_householder.bottomRightCorner(remainingRows, remainingCols)
                 .applyHouseholderOnTheLeft(m_householder.col(k).tail(remainingRows-1),
                                            m_householder.coeff(k,k),
                                            temp.data());

    if(k == cols-1) break;
    
    // construct right householder transform in-place in m_householder
    m_householder.row(k).tail(remainingCols)
                 .makeHouseholderInPlace(m_householder.coeffRef(k,k+1),
                                         m_bidiagonal.template diagonal<1>().coeffRef(k));
    // apply householder transform to remaining part of m_householder on the left
    m_householder.bottomRightCorner(remainingRows-1, remainingCols)
                 .applyHouseholderOnTheRight(m_householder.row(k).tail(remainingCols-1).transpose(),
                                             m_householder.coeff(k,k+1),
                                             temp.data());
  }
  m_isInitialized = true;
  return *this;
}

#if 0
/** \return the Householder QR decomposition of \c *this.
  *
  * \sa class Bidiagonalization
  */
template<typename Derived>
const UpperBidiagonalization<typename MatrixBase<Derived>::PlainObject>
MatrixBase<Derived>::bidiagonalization() const
{
  return UpperBidiagonalization<PlainObject>(eval());
}
#endif


#endif // EIGEN_BIDIAGONALIZATION_H
