// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2009 Gael Guennebaud <gael.guennebaud@inria.fr>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef EIGEN_SELFADJOINTMATRIX_H
#define EIGEN_SELFADJOINTMATRIX_H

namespace Eigen { 

/** \class SelfAdjointView
  * \ingroup Core_Module
  *
  *
  * \brief Expression of a selfadjoint matrix from a triangular part of a dense matrix
  *
  * \param MatrixType the type of the dense matrix storing the coefficients
  * \param TriangularPart can be either \c #Lower or \c #Upper
  *
  * This class is an expression of a sefladjoint matrix from a triangular part of a matrix
  * with given dense storage of the coefficients. It is the return type of MatrixBase::selfadjointView()
  * and most of the time this is the only way that it is used.
  *
  * \sa class TriangularBase, MatrixBase::selfadjointView()
  */

namespace internal {
template<typename MatrixType, unsigned int UpLo>
struct traits<SelfAdjointView<MatrixType, UpLo> > : traits<MatrixType>
{
  typedef typename nested<MatrixType>::type MatrixTypeNested;
  typedef typename remove_all<MatrixTypeNested>::type MatrixTypeNestedCleaned;
  typedef MatrixType ExpressionType;
  typedef typename MatrixType::PlainObject DenseMatrixType;
  enum {
    Mode = UpLo | SelfAdjoint,
    Flags =  MatrixTypeNestedCleaned::Flags & (HereditaryBits)
           & (~(PacketAccessBit | DirectAccessBit | LinearAccessBit)) // FIXME these flags should be preserved
#ifndef EIGEN_TEST_EVALUATORS
    ,
    CoeffReadCost = MatrixTypeNestedCleaned::CoeffReadCost
#endif
  };
};
}

#ifndef EIGEN_TEST_EVALUATORS
template <typename Lhs, int LhsMode, bool LhsIsVector,
          typename Rhs, int RhsMode, bool RhsIsVector>
struct SelfadjointProductMatrix;
#endif

// FIXME could also be called SelfAdjointWrapper to be consistent with DiagonalWrapper ??
template<typename _MatrixType, unsigned int UpLo> class SelfAdjointView
  : public TriangularBase<SelfAdjointView<_MatrixType, UpLo> >
{
  public:

    typedef _MatrixType MatrixType;
    typedef TriangularBase<SelfAdjointView> Base;
    typedef typename internal::traits<SelfAdjointView>::MatrixTypeNested MatrixTypeNested;
    typedef typename internal::traits<SelfAdjointView>::MatrixTypeNestedCleaned MatrixTypeNestedCleaned;

    /** \brief The type of coefficients in this matrix */
    typedef typename internal::traits<SelfAdjointView>::Scalar Scalar; 

    typedef typename MatrixType::Index Index;

    enum {
      Mode = internal::traits<SelfAdjointView>::Mode,
      Flags = internal::traits<SelfAdjointView>::Flags
    };
    typedef typename MatrixType::PlainObject PlainObject;

    EIGEN_DEVICE_FUNC
    inline SelfAdjointView(MatrixType& matrix) : m_matrix(matrix)
    {}

    EIGEN_DEVICE_FUNC
    inline Index rows() const { return m_matrix.rows(); }
    EIGEN_DEVICE_FUNC
    inline Index cols() const { return m_matrix.cols(); }
    EIGEN_DEVICE_FUNC
    inline Index outerStride() const { return m_matrix.outerStride(); }
    EIGEN_DEVICE_FUNC
    inline Index innerStride() const { return m_matrix.innerStride(); }

    /** \sa MatrixBase::coeff()
      * \warning the coordinates must fit into the referenced triangular part
      */
    EIGEN_DEVICE_FUNC
    inline Scalar coeff(Index row, Index col) const
    {
      Base::check_coordinates_internal(row, col);
      return m_matrix.coeff(row, col);
    }

    /** \sa MatrixBase::coeffRef()
      * \warning the coordinates must fit into the referenced triangular part
      */
    EIGEN_DEVICE_FUNC
    inline Scalar& coeffRef(Index row, Index col)
    {
      Base::check_coordinates_internal(row, col);
      return m_matrix.const_cast_derived().coeffRef(row, col);
    }

    /** \internal */
    EIGEN_DEVICE_FUNC
    const MatrixTypeNestedCleaned& _expression() const { return m_matrix; }

    EIGEN_DEVICE_FUNC
    const MatrixTypeNestedCleaned& nestedExpression() const { return m_matrix; }
    EIGEN_DEVICE_FUNC
    MatrixTypeNestedCleaned& nestedExpression() { return *const_cast<MatrixTypeNestedCleaned*>(&m_matrix); }

#ifdef EIGEN_TEST_EVALUATORS

    /** Efficient triangular matrix times vector/matrix product */
    template<typename OtherDerived>
    EIGEN_DEVICE_FUNC
    const Product<SelfAdjointView,OtherDerived>
    operator*(const MatrixBase<OtherDerived>& rhs) const
    {
      return Product<SelfAdjointView,OtherDerived>(*this, rhs.derived());
    }

    /** Efficient vector/matrix times triangular matrix product */
    template<typename OtherDerived> friend
    EIGEN_DEVICE_FUNC
    const Product<OtherDerived,SelfAdjointView>
    operator*(const MatrixBase<OtherDerived>& lhs, const SelfAdjointView& rhs)
    {
      return Product<OtherDerived,SelfAdjointView>(lhs.derived(),rhs);
    }
    
    friend EIGEN_DEVICE_FUNC
    const SelfAdjointView<const CwiseUnaryOp<internal::scalar_multiple_op<Scalar>,MatrixType>,UpLo>
    operator*(const Scalar& s, const SelfAdjointView& mat)
    {
      return (s*mat.nestedExpression()).template selfadjointView<UpLo>();
    }

#else // EIGEN_TEST_EVALUATORS

    /** Efficient self-adjoint matrix times vector/matrix product */
    template<typename OtherDerived>
    EIGEN_DEVICE_FUNC
    SelfadjointProductMatrix<MatrixType,Mode,false,OtherDerived,0,OtherDerived::IsVectorAtCompileTime>
    operator*(const MatrixBase<OtherDerived>& rhs) const
    {
      return SelfadjointProductMatrix
              <MatrixType,Mode,false,OtherDerived,0,OtherDerived::IsVectorAtCompileTime>
              (m_matrix, rhs.derived());
    }

    /** Efficient vector/matrix times self-adjoint matrix product */
    template<typename OtherDerived> friend
    EIGEN_DEVICE_FUNC
    SelfadjointProductMatrix<OtherDerived,0,OtherDerived::IsVectorAtCompileTime,MatrixType,Mode,false>
    operator*(const MatrixBase<OtherDerived>& lhs, const SelfAdjointView& rhs)
    {
      return SelfadjointProductMatrix
              <OtherDerived,0,OtherDerived::IsVectorAtCompileTime,MatrixType,Mode,false>
              (lhs.derived(),rhs.m_matrix);
    }
#endif

    /** Perform a symmetric rank 2 update of the selfadjoint matrix \c *this:
      * \f$ this = this + \alpha u v^* + conj(\alpha) v u^* \f$
      * \returns a reference to \c *this
      *
      * The vectors \a u and \c v \b must be column vectors, however they can be
      * a adjoint expression without any overhead. Only the meaningful triangular
      * part of the matrix is updated, the rest is left unchanged.
      *
      * \sa rankUpdate(const MatrixBase<DerivedU>&, Scalar)
      */
    template<typename DerivedU, typename DerivedV>
    EIGEN_DEVICE_FUNC
    SelfAdjointView& rankUpdate(const MatrixBase<DerivedU>& u, const MatrixBase<DerivedV>& v, const Scalar& alpha = Scalar(1));

    /** Perform a symmetric rank K update of the selfadjoint matrix \c *this:
      * \f$ this = this + \alpha ( u u^* ) \f$ where \a u is a vector or matrix.
      *
      * \returns a reference to \c *this
      *
      * Note that to perform \f$ this = this + \alpha ( u^* u ) \f$ you can simply
      * call this function with u.adjoint().
      *
      * \sa rankUpdate(const MatrixBase<DerivedU>&, const MatrixBase<DerivedV>&, Scalar)
      */
    template<typename DerivedU>
    EIGEN_DEVICE_FUNC
    SelfAdjointView& rankUpdate(const MatrixBase<DerivedU>& u, const Scalar& alpha = Scalar(1));

/////////// Cholesky module ///////////

    const LLT<PlainObject, UpLo> llt() const;
    const LDLT<PlainObject, UpLo> ldlt() const;

/////////// Eigenvalue module ///////////

    /** Real part of #Scalar */
    typedef typename NumTraits<Scalar>::Real RealScalar;
    /** Return type of eigenvalues() */
    typedef Matrix<RealScalar, internal::traits<MatrixType>::ColsAtCompileTime, 1> EigenvaluesReturnType;

    EIGEN_DEVICE_FUNC
    EigenvaluesReturnType eigenvalues() const;
    EIGEN_DEVICE_FUNC
    RealScalar operatorNorm() const;
    
    #ifdef EIGEN2_SUPPORT
    template<typename OtherDerived>
    EIGEN_DEVICE_FUNC
    SelfAdjointView& operator=(const MatrixBase<OtherDerived>& other)
    {
      enum {
        OtherPart = UpLo == Upper ? StrictlyLower : StrictlyUpper
      };
      m_matrix.const_cast_derived().template triangularView<UpLo>() = other;
      m_matrix.const_cast_derived().template triangularView<OtherPart>() = other.adjoint();
      return *this;
    }
    template<typename OtherMatrixType, unsigned int OtherMode>
    EIGEN_DEVICE_FUNC
    SelfAdjointView& operator=(const TriangularView<OtherMatrixType, OtherMode>& other)
    {
      enum {
        OtherPart = UpLo == Upper ? StrictlyLower : StrictlyUpper
      };
      m_matrix.const_cast_derived().template triangularView<UpLo>() = other.toDenseMatrix();
      m_matrix.const_cast_derived().template triangularView<OtherPart>() = other.toDenseMatrix().adjoint();
      return *this;
    }
    #endif

  protected:
    MatrixTypeNested m_matrix;
};


// template<typename OtherDerived, typename MatrixType, unsigned int UpLo>
// internal::selfadjoint_matrix_product_returntype<OtherDerived,SelfAdjointView<MatrixType,UpLo> >
// operator*(const MatrixBase<OtherDerived>& lhs, const SelfAdjointView<MatrixType,UpLo>& rhs)
// {
//   return internal::matrix_selfadjoint_product_returntype<OtherDerived,SelfAdjointView<MatrixType,UpLo> >(lhs.derived(),rhs);
// }

// selfadjoint to dense matrix

namespace internal {

#ifndef EIGEN_TEST_EVALUATORS
 
template<typename Derived1, typename Derived2, int UnrollCount, bool ClearOpposite>
struct triangular_assignment_selector<Derived1, Derived2, (SelfAdjoint|Upper), UnrollCount, ClearOpposite>
{
  enum {
    col = (UnrollCount-1) / Derived1::RowsAtCompileTime,
    row = (UnrollCount-1) % Derived1::RowsAtCompileTime
  };

  EIGEN_DEVICE_FUNC
  static inline void run(Derived1 &dst, const Derived2 &src)
  {
    triangular_assignment_selector<Derived1, Derived2, (SelfAdjoint|Upper), UnrollCount-1, ClearOpposite>::run(dst, src);

    if(row == col)
      dst.coeffRef(row, col) = numext::real(src.coeff(row, col));
    else if(row < col)
      dst.coeffRef(col, row) = numext::conj(dst.coeffRef(row, col) = src.coeff(row, col));
  }
};

template<typename Derived1, typename Derived2, bool ClearOpposite>
struct triangular_assignment_selector<Derived1, Derived2, SelfAdjoint|Upper, 0, ClearOpposite>
{
  EIGEN_DEVICE_FUNC
  static inline void run(Derived1 &, const Derived2 &) {}
};

template<typename Derived1, typename Derived2, int UnrollCount, bool ClearOpposite>
struct triangular_assignment_selector<Derived1, Derived2, (SelfAdjoint|Lower), UnrollCount, ClearOpposite>
{
  enum {
    col = (UnrollCount-1) / Derived1::RowsAtCompileTime,
    row = (UnrollCount-1) % Derived1::RowsAtCompileTime
  };

  EIGEN_DEVICE_FUNC
  static inline void run(Derived1 &dst, const Derived2 &src)
  {
    triangular_assignment_selector<Derived1, Derived2, (SelfAdjoint|Lower), UnrollCount-1, ClearOpposite>::run(dst, src);

    if(row == col)
      dst.coeffRef(row, col) = numext::real(src.coeff(row, col));
    else if(row > col)
      dst.coeffRef(col, row) = numext::conj(dst.coeffRef(row, col) = src.coeff(row, col));
  }
};

template<typename Derived1, typename Derived2, bool ClearOpposite>
struct triangular_assignment_selector<Derived1, Derived2, SelfAdjoint|Lower, 0, ClearOpposite>
{
  EIGEN_DEVICE_FUNC
  static inline void run(Derived1 &, const Derived2 &) {}
};

template<typename Derived1, typename Derived2, bool ClearOpposite>
struct triangular_assignment_selector<Derived1, Derived2, SelfAdjoint|Upper, Dynamic, ClearOpposite>
{
  typedef typename Derived1::Index Index;
  EIGEN_DEVICE_FUNC
  static inline void run(Derived1 &dst, const Derived2 &src)
  {
    for(Index j = 0; j < dst.cols(); ++j)
    {
      for(Index i = 0; i < j; ++i)
      {
        dst.copyCoeff(i, j, src);
        dst.coeffRef(j,i) = numext::conj(dst.coeff(i,j));
      }
      dst.copyCoeff(j, j, src);
    }
  }
};

template<typename Derived1, typename Derived2, bool ClearOpposite>
struct triangular_assignment_selector<Derived1, Derived2, SelfAdjoint|Lower, Dynamic, ClearOpposite>
{
  EIGEN_DEVICE_FUNC
  static inline void run(Derived1 &dst, const Derived2 &src)
  {
  typedef typename Derived1::Index Index;
    for(Index i = 0; i < dst.rows(); ++i)
    {
      for(Index j = 0; j < i; ++j)
      {
        dst.copyCoeff(i, j, src);
        dst.coeffRef(j,i) = numext::conj(dst.coeff(i,j));
      }
      dst.copyCoeff(i, i, src);
    }
  }
};

#endif // EIGEN_TEST_EVALUATORS

#ifdef EIGEN_ENABLE_EVALUATORS
// TODO currently a selfadjoint expression has the form SelfAdjointView<.,.>
//      in the future selfadjoint-ness should be defined by the expression traits
//      such that Transpose<SelfAdjointView<.,.> > is valid. (currently TriangularBase::transpose() is overloaded to make it work)
template<typename MatrixType, unsigned int Mode>
struct evaluator_traits<SelfAdjointView<MatrixType,Mode> >
{
  typedef typename storage_kind_to_evaluator_kind<typename MatrixType::StorageKind>::Kind Kind;
  typedef SelfAdjointShape Shape;
  
  static const int AssumeAliasing = 0;
};

template<int UpLo, int SetOpposite, typename DstEvaluatorTypeT, typename SrcEvaluatorTypeT, typename Functor, int Version>
class triangular_dense_assignment_kernel<UpLo,SelfAdjoint,SetOpposite,DstEvaluatorTypeT,SrcEvaluatorTypeT,Functor,Version>
  : public generic_dense_assignment_kernel<DstEvaluatorTypeT, SrcEvaluatorTypeT, Functor, Version>
{
protected:
  typedef generic_dense_assignment_kernel<DstEvaluatorTypeT, SrcEvaluatorTypeT, Functor, Version> Base;
  typedef typename Base::DstXprType DstXprType;
  typedef typename Base::SrcXprType SrcXprType;
  using Base::m_dst;
  using Base::m_src;
  using Base::m_functor;
public:
  
  typedef typename Base::DstEvaluatorType DstEvaluatorType;
  typedef typename Base::SrcEvaluatorType SrcEvaluatorType;
  typedef typename Base::Scalar Scalar;
  typedef typename Base::Index Index;
  typedef typename Base::AssignmentTraits AssignmentTraits;
  
  
  triangular_dense_assignment_kernel(DstEvaluatorType &dst, const SrcEvaluatorType &src, const Functor &func, DstXprType& dstExpr)
    : Base(dst, src, func, dstExpr)
  {}
  
  void assignCoeff(Index row, Index col)
  {
    eigen_internal_assert(row!=col);
    Scalar tmp = m_src.coeff(row,col);
    m_functor.assignCoeff(m_dst.coeffRef(row,col), tmp);
    m_functor.assignCoeff(m_dst.coeffRef(col,row), numext::conj(tmp));
  }
  
  void assignDiagonalCoeff(Index id)
  {
    Base::assignCoeff(id,id);
  }
  
  void assignOppositeCoeff(Index, Index)
  { eigen_internal_assert(false && "should never be called"); }
};

#endif // EIGEN_ENABLE_EVALUATORS

} // end namespace internal

/***************************************************************************
* Implementation of MatrixBase methods
***************************************************************************/

template<typename Derived>
template<unsigned int UpLo>
typename MatrixBase<Derived>::template ConstSelfAdjointViewReturnType<UpLo>::Type
MatrixBase<Derived>::selfadjointView() const
{
  return derived();
}

template<typename Derived>
template<unsigned int UpLo>
typename MatrixBase<Derived>::template SelfAdjointViewReturnType<UpLo>::Type
MatrixBase<Derived>::selfadjointView()
{
  return derived();
}

} // end namespace Eigen

#endif // EIGEN_SELFADJOINTMATRIX_H
