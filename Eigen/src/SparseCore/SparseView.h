// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2011 Gael Guennebaud <gael.guennebaud@inria.fr>
// Copyright (C) 2010 Daniel Lowengrub <lowdanie@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef EIGEN_SPARSEVIEW_H
#define EIGEN_SPARSEVIEW_H

namespace Eigen { 

namespace internal {

template<typename MatrixType>
struct traits<SparseView<MatrixType> > : traits<MatrixType>
{
  typedef typename MatrixType::Index Index;
  typedef Sparse StorageKind;
  enum {
    Flags = int(traits<MatrixType>::Flags) & (RowMajorBit)
  };
};

} // end namespace internal

template<typename MatrixType>
class SparseView : public SparseMatrixBase<SparseView<MatrixType> >
{
  typedef typename MatrixType::Nested MatrixTypeNested;
  typedef typename internal::remove_all<MatrixTypeNested>::type _MatrixTypeNested;
public:
  EIGEN_SPARSE_PUBLIC_INTERFACE(SparseView)
  typedef typename internal::remove_all<MatrixType>::type NestedExpression;

  SparseView(const MatrixType& mat, const Scalar& m_reference = Scalar(0),
             RealScalar m_epsilon = NumTraits<Scalar>::dummy_precision()) : 
    m_matrix(mat), m_reference(m_reference), m_epsilon(m_epsilon) {}

#ifndef EIGEN_TEST_EVALUATORS
  class InnerIterator;
#endif // EIGEN_TEST_EVALUATORS

  inline Index rows() const { return m_matrix.rows(); }
  inline Index cols() const { return m_matrix.cols(); }

  inline Index innerSize() const { return m_matrix.innerSize(); }
  inline Index outerSize() const { return m_matrix.outerSize(); }
  
  /** \returns the nested expression */
  const typename internal::remove_all<MatrixTypeNested>::type&
  nestedExpression() const { return m_matrix; }
  
  Scalar reference() const { return m_reference; }
  RealScalar epsilon() const { return m_epsilon; }
  
protected:
  MatrixTypeNested m_matrix;
  Scalar m_reference;
  RealScalar m_epsilon;
};

#ifndef EIGEN_TEST_EVALUATORS
template<typename MatrixType>
class SparseView<MatrixType>::InnerIterator : public _MatrixTypeNested::InnerIterator
{
  typedef typename SparseView::Index Index;
public:
  typedef typename _MatrixTypeNested::InnerIterator IterBase;
  InnerIterator(const SparseView& view, Index outer) :
  IterBase(view.m_matrix, outer), m_view(view)
  {
    incrementToNonZero();
  }

  EIGEN_STRONG_INLINE InnerIterator& operator++()
  {
    IterBase::operator++();
    incrementToNonZero();
    return *this;
  }

  using IterBase::value;

protected:
  const SparseView& m_view;

private:
  void incrementToNonZero()
  {
    while((bool(*this)) && internal::isMuchSmallerThan(value(), m_view.reference(), m_view.epsilon()))
    {
      IterBase::operator++();
    }
  }
};

#else // EIGEN_TEST_EVALUATORS

namespace internal {

// TODO find a way to unify the two following variants
// This is tricky because implementing an inner iterator on top of an IndexBased evaluator is
// not easy because the evaluators do not expose the sizes of the underlying expression.
  
template<typename ArgType>
struct unary_evaluator<SparseView<ArgType>, IteratorBased>
  : public evaluator_base<SparseView<ArgType> >
{
    typedef typename evaluator<ArgType>::InnerIterator EvalIterator;
  public:
    typedef SparseView<ArgType> XprType;
    
    class InnerIterator : public EvalIterator
    {
        typedef typename XprType::Scalar Scalar;
      public:

        EIGEN_STRONG_INLINE InnerIterator(const unary_evaluator& sve, typename XprType::Index outer)
          : EvalIterator(sve.m_argImpl,outer), m_view(sve.m_view)
        {
          incrementToNonZero();
        }

        EIGEN_STRONG_INLINE InnerIterator& operator++()
        {
          EvalIterator::operator++();
          incrementToNonZero();
          return *this;
        }

        using EvalIterator::value;

      protected:
        const XprType &m_view;

      private:
        void incrementToNonZero()
        {
          while((bool(*this)) && internal::isMuchSmallerThan(value(), m_view.reference(), m_view.epsilon()))
          {
            EvalIterator::operator++();
          }
        }
    };
    
    enum {
      CoeffReadCost = evaluator<ArgType>::CoeffReadCost,
      Flags = XprType::Flags
    };
    
    unary_evaluator(const XprType& xpr) : m_argImpl(xpr.nestedExpression()), m_view(xpr) {}

  protected:
    typename evaluator<ArgType>::nestedType m_argImpl;
    const XprType &m_view;
};

template<typename ArgType>
struct unary_evaluator<SparseView<ArgType>, IndexBased>
  : public evaluator_base<SparseView<ArgType> >
{
  public:
    typedef SparseView<ArgType> XprType;
  protected:
    enum { IsRowMajor = (XprType::Flags&RowMajorBit)==RowMajorBit };
    typedef typename XprType::Index Index;
    typedef typename XprType::Scalar Scalar;
  public:
    
    class InnerIterator
    {
      public:

        EIGEN_STRONG_INLINE InnerIterator(const unary_evaluator& sve, typename XprType::Index outer)
          : m_sve(sve), m_inner(0), m_outer(outer), m_end(sve.m_view.innerSize())
        {
          incrementToNonZero();
        }

        EIGEN_STRONG_INLINE InnerIterator& operator++()
        {
          m_inner++;
          incrementToNonZero();
          return *this;
        }

        EIGEN_STRONG_INLINE Scalar value() const
        {
          return (IsRowMajor) ? m_sve.m_argImpl.coeff(m_outer, m_inner)
                              : m_sve.m_argImpl.coeff(m_inner, m_outer);
        }

        EIGEN_STRONG_INLINE Index index() const { return m_inner; }
        inline Index row() const { return IsRowMajor ? m_outer : index(); }
        inline Index col() const { return IsRowMajor ? index() : m_outer; }

        EIGEN_STRONG_INLINE operator bool() const { return m_inner < m_end && m_inner>=0; }

      protected:
        const unary_evaluator &m_sve;
        Index m_inner;
        const Index m_outer;
        const Index m_end;

      private:
        void incrementToNonZero()
        {
          while((bool(*this)) && internal::isMuchSmallerThan(value(), m_sve.m_view.reference(), m_sve.m_view.epsilon()))
          {
            m_inner++;
          }
        }
    };
    
    enum {
      CoeffReadCost = evaluator<ArgType>::CoeffReadCost,
      Flags = XprType::Flags
    };
    
    unary_evaluator(const XprType& xpr) : m_argImpl(xpr.nestedExpression()), m_view(xpr) {}

  protected:
    typename evaluator<ArgType>::nestedType m_argImpl;
    const XprType &m_view;
};

} // end namespace internal

#endif // EIGEN_TEST_EVALUATORS

template<typename Derived>
const SparseView<Derived> MatrixBase<Derived>::sparseView(const Scalar& m_reference,
                                                          const typename NumTraits<Scalar>::Real& m_epsilon) const
{
  return SparseView<Derived>(derived(), m_reference, m_epsilon);
}

} // end namespace Eigen

#endif
