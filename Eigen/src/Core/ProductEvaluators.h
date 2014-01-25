// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2006-2008 Benoit Jacob <jacob.benoit.1@gmail.com>
// Copyright (C) 2008-2010 Gael Guennebaud <gael.guennebaud@inria.fr>
// Copyright (C) 2011 Jitse Niesen <jitse@maths.leeds.ac.uk>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.


#ifndef EIGEN_PRODUCTEVALUATORS_H
#define EIGEN_PRODUCTEVALUATORS_H

namespace Eigen {
  
namespace internal {

// Like more general binary expressions, products need their own evaluator:
template< typename T,
          int ProductTag = internal::product_type<typename T::Lhs,typename T::Rhs>::ret,
          typename LhsShape = typename evaluator_traits<typename T::Lhs>::Shape,
          typename RhsShape = typename evaluator_traits<typename T::Rhs>::Shape,
          typename LhsScalar = typename T::Lhs::Scalar,
          typename RhsScalar = typename T::Rhs::Scalar
        > struct product_evaluator;

template<typename Lhs, typename Rhs, int Options>
struct evaluator<Product<Lhs, Rhs, Options> > 
 : public product_evaluator<Product<Lhs, Rhs, Options> >
{
  typedef Product<Lhs, Rhs, Options> XprType;
  typedef product_evaluator<XprType> Base;
  
  typedef evaluator type;
  typedef evaluator nestedType;
  
  evaluator(const XprType& xpr) : Base(xpr) {}
};
 
// Catch scalar * ( A * B ) and transform it to (A*scalar) * B
// TODO we should apply that rule if that's really helpful
template<typename Lhs, typename Rhs, typename Scalar>
struct evaluator<CwiseUnaryOp<internal::scalar_multiple_op<Scalar>,  const Product<Lhs, Rhs, DefaultProduct>  > > 
 : public evaluator<Product<CwiseUnaryOp<internal::scalar_multiple_op<Scalar>,const Lhs>, Rhs, DefaultProduct> >
{
  typedef CwiseUnaryOp<internal::scalar_multiple_op<Scalar>, const Product<Lhs, Rhs, DefaultProduct> > XprType;
  typedef evaluator<Product<CwiseUnaryOp<internal::scalar_multiple_op<Scalar>,const Lhs>, Rhs, DefaultProduct> > Base;
  
  typedef evaluator type;
  typedef evaluator nestedType;
  
  evaluator(const XprType& xpr)
    : Base(xpr.functor().m_other * xpr.nestedExpression().lhs() * xpr.nestedExpression().rhs())
  {}
};


template<typename Lhs, typename Rhs, int DiagIndex>
struct evaluator<Diagonal<const Product<Lhs, Rhs, DefaultProduct>, DiagIndex> > 
 : public evaluator<Diagonal<const Product<Lhs, Rhs, LazyProduct>, DiagIndex> >
{
  typedef Diagonal<const Product<Lhs, Rhs, DefaultProduct>, DiagIndex> XprType;
  typedef evaluator<Diagonal<const Product<Lhs, Rhs, LazyProduct>, DiagIndex> > Base;
  
  typedef evaluator type;
  typedef evaluator nestedType;
//   
  evaluator(const XprType& xpr)
    : Base(Diagonal<const Product<Lhs, Rhs, LazyProduct>, DiagIndex>(
        Product<Lhs, Rhs, LazyProduct>(xpr.nestedExpression().lhs(), xpr.nestedExpression().rhs()),
        xpr.index() ))
  {}
};


// Helper class to perform a matrix product with the destination at hand.
// Depending on the sizes of the factors, there are different evaluation strategies
// as controlled by internal::product_type.
template< typename Lhs, typename Rhs,
          typename LhsShape = typename evaluator_traits<Lhs>::Shape,
          typename RhsShape = typename evaluator_traits<Rhs>::Shape,
          int ProductType = internal::product_type<Lhs,Rhs>::value>
struct generic_product_impl;

template<typename Lhs, typename Rhs>
struct evaluator_traits<Product<Lhs, Rhs, DefaultProduct> > 
 : evaluator_traits_base<Product<Lhs, Rhs, DefaultProduct> >
{
  enum { AssumeAliasing = 1 };
};

// The evaluator for default dense products creates a temporary and call generic_product_impl
template<typename Lhs, typename Rhs, int ProductTag>
struct product_evaluator<Product<Lhs, Rhs, DefaultProduct>, ProductTag, DenseShape, DenseShape, typename Lhs::Scalar, typename Rhs::Scalar> 
  : public evaluator<typename Product<Lhs, Rhs, DefaultProduct>::PlainObject>::type
{
  typedef Product<Lhs, Rhs, DefaultProduct> XprType;
  typedef typename XprType::PlainObject PlainObject;
  typedef typename evaluator<PlainObject>::type Base;

  product_evaluator(const XprType& xpr)
    : m_result(xpr.rows(), xpr.cols())
  {
    ::new (static_cast<Base*>(this)) Base(m_result);
    generic_product_impl<Lhs, Rhs>::evalTo(m_result, xpr.lhs(), xpr.rhs());
  }
  
protected:  
  PlainObject m_result;
};

// Dense = Product
template< typename DstXprType, typename Lhs, typename Rhs, typename Scalar>
struct Assignment<DstXprType, Product<Lhs,Rhs,DefaultProduct>, internal::assign_op<Scalar>, Dense2Dense, Scalar>
{
  typedef Product<Lhs,Rhs,DefaultProduct> SrcXprType;
  static void run(DstXprType &dst, const SrcXprType &src, const internal::assign_op<Scalar> &)
  {
    generic_product_impl<Lhs, Rhs>::evalTo(dst, src.lhs(), src.rhs());
  }
};

// Dense += Product
template< typename DstXprType, typename Lhs, typename Rhs, typename Scalar>
struct Assignment<DstXprType, Product<Lhs,Rhs,DefaultProduct>, internal::add_assign_op<Scalar>, Dense2Dense, Scalar>
{
  typedef Product<Lhs,Rhs,DefaultProduct> SrcXprType;
  static void run(DstXprType &dst, const SrcXprType &src, const internal::add_assign_op<Scalar> &)
  {
    generic_product_impl<Lhs, Rhs>::addTo(dst, src.lhs(), src.rhs());
  }
};

// Dense -= Product
template< typename DstXprType, typename Lhs, typename Rhs, typename Scalar>
struct Assignment<DstXprType, Product<Lhs,Rhs,DefaultProduct>, internal::sub_assign_op<Scalar>, Dense2Dense, Scalar>
{
  typedef Product<Lhs,Rhs,DefaultProduct> SrcXprType;
  static void run(DstXprType &dst, const SrcXprType &src, const internal::sub_assign_op<Scalar> &)
  {
    generic_product_impl<Lhs, Rhs>::subTo(dst, src.lhs(), src.rhs());
  }
};


// Dense ?= scalar * Product
// TODO we should apply that rule if that's really helpful
// for instance, this is not good for inner products
template< typename DstXprType, typename Lhs, typename Rhs, typename AssignFunc, typename Scalar, typename ScalarBis>
struct Assignment<DstXprType, CwiseUnaryOp<internal::scalar_multiple_op<ScalarBis>,
                                           const Product<Lhs,Rhs,DefaultProduct> >, AssignFunc, Dense2Dense, Scalar>
{
  typedef CwiseUnaryOp<internal::scalar_multiple_op<ScalarBis>,
                       const Product<Lhs,Rhs,DefaultProduct> > SrcXprType;
  static void run(DstXprType &dst, const SrcXprType &src, const AssignFunc& func)
  {
    // TODO use operator* instead of prod() once we have made enough progress
    call_assignment(dst.noalias(), prod(src.functor().m_other * src.nestedExpression().lhs(), src.nestedExpression().rhs()), func);
  }
};


template<typename Lhs, typename Rhs>
struct generic_product_impl<Lhs,Rhs,DenseShape,DenseShape,InnerProduct>
{
  template<typename Dst>
  static inline void evalTo(Dst& dst, const Lhs& lhs, const Rhs& rhs)
  {
    dst.coeffRef(0,0) = (lhs.transpose().cwiseProduct(rhs)).sum();
  }
  
  template<typename Dst>
  static inline void addTo(Dst& dst, const Lhs& lhs, const Rhs& rhs)
  {
    dst.coeffRef(0,0) += (lhs.transpose().cwiseProduct(rhs)).sum();
  }
  
  template<typename Dst>
  static void subTo(Dst& dst, const Lhs& lhs, const Rhs& rhs)
  { dst.coeffRef(0,0) -= (lhs.transpose().cwiseProduct(rhs)).sum(); }
};



template<typename Lhs, typename Rhs>
struct generic_product_impl<Lhs,Rhs,DenseShape,DenseShape,OuterProduct>
{
  typedef typename Product<Lhs,Rhs>::Scalar Scalar;
  
  template<typename Dst>
  static inline void evalTo(Dst& dst, const Lhs& lhs, const Rhs& rhs)
  {
    // TODO bypass GeneralProduct class
    GeneralProduct<Lhs, Rhs, OuterProduct>(lhs,rhs).evalTo(dst);
  }
  
  template<typename Dst>
  static inline void addTo(Dst& dst, const Lhs& lhs, const Rhs& rhs)
  {
    // TODO bypass GeneralProduct class
    GeneralProduct<Lhs, Rhs, OuterProduct>(lhs,rhs).addTo(dst);
  }
  
  template<typename Dst>
  static inline void subTo(Dst& dst, const Lhs& lhs, const Rhs& rhs)
  {
    // TODO bypass GeneralProduct class
    GeneralProduct<Lhs, Rhs, OuterProduct>(lhs,rhs).subTo(dst);
  }
  
  template<typename Dst>
  static inline void scaleAndAddTo(Dst& dst, const Lhs& lhs, const Rhs& rhs, const Scalar& alpha)
  {
    // TODO bypass GeneralProduct class
    GeneralProduct<Lhs, Rhs, OuterProduct>(lhs,rhs).scaleAndAddTo(dst, alpha);
  }
  
};


// This base class provides default implementations for evalTo, addTo, subTo, in terms of scaleAndAddTo
template<typename Lhs, typename Rhs, typename Derived>
struct generic_product_impl_base
{
  typedef typename Product<Lhs,Rhs>::Scalar Scalar;
  
  template<typename Dst>
  static void evalTo(Dst& dst, const Lhs& lhs, const Rhs& rhs)
  { dst.setZero(); scaleAndAddTo(dst, lhs, rhs, Scalar(1)); }

  template<typename Dst>
  static void addTo(Dst& dst, const Lhs& lhs, const Rhs& rhs)
  { scaleAndAddTo(dst,lhs, rhs, Scalar(1)); }

  template<typename Dst>
  static void subTo(Dst& dst, const Lhs& lhs, const Rhs& rhs)
  { scaleAndAddTo(dst, lhs, rhs, Scalar(-1)); }
  
  template<typename Dst>
  static void scaleAndAddTo(Dst& dst, const Lhs& lhs, const Rhs& rhs, const Scalar& alpha)
  { Derived::scaleAndAddTo(dst,lhs,rhs,alpha); }

};

template<typename Lhs, typename Rhs>
struct generic_product_impl<Lhs,Rhs,DenseShape,DenseShape,GemvProduct>
  : generic_product_impl_base<Lhs,Rhs,generic_product_impl<Lhs,Rhs,DenseShape,DenseShape,GemvProduct> >
{
  typedef typename Product<Lhs,Rhs>::Scalar Scalar;
  enum { Side = Lhs::IsVectorAtCompileTime ? OnTheLeft : OnTheRight };
  typedef typename internal::conditional<int(Side)==OnTheRight,Lhs,Rhs>::type MatrixType;

  template<typename Dest>
  static void scaleAndAddTo(Dest& dst, const Lhs& lhs, const Rhs& rhs, const Scalar& alpha)
  {
    internal::gemv_selector<Side,
                            (int(MatrixType::Flags)&RowMajorBit) ? RowMajor : ColMajor,
                            bool(internal::blas_traits<MatrixType>::HasUsableDirectAccess)
                           >::run(GeneralProduct<Lhs,Rhs,GemvProduct>(lhs,rhs), dst, alpha);
  }
};

template<typename Lhs, typename Rhs>
struct generic_product_impl<Lhs,Rhs,DenseShape,DenseShape,GemmProduct>
  : generic_product_impl_base<Lhs,Rhs,generic_product_impl<Lhs,Rhs,DenseShape,DenseShape,GemmProduct> >
{
  typedef typename Product<Lhs,Rhs>::Scalar Scalar;
  
  template<typename Dest>
  static void scaleAndAddTo(Dest& dst, const Lhs& lhs, const Rhs& rhs, const Scalar& alpha)
  {
    // TODO bypass GeneralProduct class
    GeneralProduct<Lhs, Rhs, GemmProduct>(lhs,rhs).scaleAndAddTo(dst, alpha);
  }
};

template<typename Lhs, typename Rhs>
struct generic_product_impl<Lhs,Rhs,DenseShape,DenseShape,CoeffBasedProductMode> 
{
  typedef typename Product<Lhs,Rhs>::Scalar Scalar;
  
  template<typename Dst>
  static inline void evalTo(Dst& dst, const Lhs& lhs, const Rhs& rhs)
  {
    // TODO: use the following instead of calling call_assignment, same for the other methods
    // dst = lazyprod(lhs,rhs);
    call_assignment(dst, lazyprod(lhs,rhs), internal::assign_op<Scalar>());
  }
  
  template<typename Dst>
  static inline void addTo(Dst& dst, const Lhs& lhs, const Rhs& rhs)
  {
    // dst += lazyprod(lhs,rhs);
    call_assignment(dst, lazyprod(lhs,rhs), internal::add_assign_op<Scalar>());
  }
  
  template<typename Dst>
  static inline void subTo(Dst& dst, const Lhs& lhs, const Rhs& rhs)
  {
    // dst -= lazyprod(lhs,rhs);
    call_assignment(dst, lazyprod(lhs,rhs), internal::sub_assign_op<Scalar>());
  }
  
//   template<typename Dst>
//   static inline void scaleAndAddTo(Dst& dst, const Lhs& lhs, const Rhs& rhs, const Scalar& alpha)
//   { dst += alpha * lazyprod(lhs,rhs); }
};

// This specialization enforces the use of a coefficient-based evaluation strategy
// template<typename Lhs, typename Rhs>
// struct generic_product_impl<Lhs,Rhs,DenseShape,DenseShape,LazyCoeffBasedProductMode>
//   : generic_product_impl<Lhs,Rhs,DenseShape,DenseShape,CoeffBasedProductMode> {};

// Case 2: Evaluate coeff by coeff
//
// This is mostly taken from CoeffBasedProduct.h
// The main difference is that we add an extra argument to the etor_product_*_impl::run() function
// for the inner dimension of the product, because evaluator object do not know their size.

template<int Traversal, int UnrollingIndex, typename Lhs, typename Rhs, typename RetScalar>
struct etor_product_coeff_impl;

template<int StorageOrder, int UnrollingIndex, typename Lhs, typename Rhs, typename Packet, int LoadMode>
struct etor_product_packet_impl;

template<typename Lhs, typename Rhs, int ProductTag>
struct product_evaluator<Product<Lhs, Rhs, LazyProduct>, ProductTag, DenseShape, DenseShape, typename Lhs::Scalar, typename Rhs::Scalar > 
    : evaluator_base<Product<Lhs, Rhs, LazyProduct> >
{
  typedef Product<Lhs, Rhs, LazyProduct> XprType;
  typedef CoeffBasedProduct<Lhs, Rhs, 0> CoeffBasedProductType;

  product_evaluator(const XprType& xpr) 
    : m_lhsImpl(xpr.lhs()), 
      m_rhsImpl(xpr.rhs()),  
      m_innerDim(xpr.lhs().cols())
  { }

  typedef typename XprType::Index Index;
  typedef typename XprType::Scalar Scalar;
  typedef typename XprType::CoeffReturnType CoeffReturnType;
  typedef typename XprType::PacketScalar PacketScalar;
  typedef typename XprType::PacketReturnType PacketReturnType;

  // Everything below here is taken from CoeffBasedProduct.h

  enum {
    RowsAtCompileTime = traits<CoeffBasedProductType>::RowsAtCompileTime,
    PacketSize = packet_traits<Scalar>::size,
    InnerSize  = traits<CoeffBasedProductType>::InnerSize,
    CoeffReadCost = traits<CoeffBasedProductType>::CoeffReadCost,
    Unroll = CoeffReadCost != Dynamic && CoeffReadCost <= EIGEN_UNROLLING_LIMIT,
    CanVectorizeInner = traits<CoeffBasedProductType>::CanVectorizeInner,
    Flags = traits<CoeffBasedProductType>::Flags
  };

  typedef typename evaluator<Lhs>::type LhsEtorType;
  typedef typename evaluator<Rhs>::type RhsEtorType;
  
  typedef etor_product_coeff_impl<CanVectorizeInner ? InnerVectorizedTraversal : DefaultTraversal,
                                  Unroll ? InnerSize-1 : Dynamic,
                                  LhsEtorType, RhsEtorType, Scalar> CoeffImpl;

  const CoeffReturnType coeff(Index row, Index col) const
  {
    Scalar res;
    CoeffImpl::run(row, col, m_lhsImpl, m_rhsImpl, m_innerDim, res);
    return res;
  }

  /* Allow index-based non-packet access. It is impossible though to allow index-based packed access,
   * which is why we don't set the LinearAccessBit.
   */
  const CoeffReturnType coeff(Index index) const
  {
    Scalar res;
    const Index row = RowsAtCompileTime == 1 ? 0 : index;
    const Index col = RowsAtCompileTime == 1 ? index : 0;
    CoeffImpl::run(row, col, m_lhsImpl, m_rhsImpl, m_innerDim, res);
    return res;
  }

  template<int LoadMode>
  const PacketReturnType packet(Index row, Index col) const
  {
    PacketScalar res;
    typedef etor_product_packet_impl<Flags&RowMajorBit ? RowMajor : ColMajor,
                                     Unroll ? InnerSize-1 : Dynamic,
                                     LhsEtorType, RhsEtorType, PacketScalar, LoadMode> PacketImpl;
    PacketImpl::run(row, col, m_lhsImpl, m_rhsImpl, m_innerDim, res);
    return res;
  }

protected:
  typename evaluator<Lhs>::type m_lhsImpl;
  typename evaluator<Rhs>::type m_rhsImpl;

  // TODO: Get rid of m_innerDim if known at compile time
  Index m_innerDim;
};

template<typename Lhs, typename Rhs>
struct product_evaluator<Product<Lhs, Rhs, DefaultProduct>, LazyCoeffBasedProductMode, DenseShape, DenseShape, typename Lhs::Scalar, typename Rhs::Scalar > 
  : product_evaluator<Product<Lhs, Rhs, LazyProduct>, CoeffBasedProductMode, DenseShape, DenseShape, typename Lhs::Scalar, typename Rhs::Scalar >
{
  typedef Product<Lhs, Rhs, DefaultProduct> XprType;
  typedef Product<Lhs, Rhs, LazyProduct> BaseProduct;
  typedef product_evaluator<BaseProduct, CoeffBasedProductMode, DenseShape, DenseShape, typename Lhs::Scalar, typename Rhs::Scalar > Base;
  product_evaluator(const XprType& xpr) 
    : Base(BaseProduct(xpr.lhs(),xpr.rhs()))
  {}
};

/***************************************************************************
* Normal product .coeff() implementation (with meta-unrolling)
***************************************************************************/

/**************************************
*** Scalar path  - no vectorization ***
**************************************/

template<int UnrollingIndex, typename Lhs, typename Rhs, typename RetScalar>
struct etor_product_coeff_impl<DefaultTraversal, UnrollingIndex, Lhs, Rhs, RetScalar>
{
  typedef typename Lhs::Index Index;
  static EIGEN_STRONG_INLINE void run(Index row, Index col, const Lhs& lhs, const Rhs& rhs, Index innerDim, RetScalar &res)
  {
    etor_product_coeff_impl<DefaultTraversal, UnrollingIndex-1, Lhs, Rhs, RetScalar>::run(row, col, lhs, rhs, innerDim, res);
    res += lhs.coeff(row, UnrollingIndex) * rhs.coeff(UnrollingIndex, col);
  }
};

template<typename Lhs, typename Rhs, typename RetScalar>
struct etor_product_coeff_impl<DefaultTraversal, 0, Lhs, Rhs, RetScalar>
{
  typedef typename Lhs::Index Index;
  static EIGEN_STRONG_INLINE void run(Index row, Index col, const Lhs& lhs, const Rhs& rhs, Index /*innerDim*/, RetScalar &res)
  {
    res = lhs.coeff(row, 0) * rhs.coeff(0, col);
  }
};

template<typename Lhs, typename Rhs, typename RetScalar>
struct etor_product_coeff_impl<DefaultTraversal, Dynamic, Lhs, Rhs, RetScalar>
{
  typedef typename Lhs::Index Index;
  static EIGEN_STRONG_INLINE void run(Index row, Index col, const Lhs& lhs, const Rhs& rhs, Index innerDim, RetScalar& res)
  {
    eigen_assert(innerDim>0 && "you are using a non initialized matrix");
    res = lhs.coeff(row, 0) * rhs.coeff(0, col);
    for(Index i = 1; i < innerDim; ++i)
      res += lhs.coeff(row, i) * rhs.coeff(i, col);
  }
};

/*******************************************
*** Scalar path with inner vectorization ***
*******************************************/

template<int UnrollingIndex, typename Lhs, typename Rhs, typename Packet>
struct etor_product_coeff_vectorized_unroller
{
  typedef typename Lhs::Index Index;
  enum { PacketSize = packet_traits<typename Lhs::Scalar>::size };
  static EIGEN_STRONG_INLINE void run(Index row, Index col, const Lhs& lhs, const Rhs& rhs, Index innerDim, typename Lhs::PacketScalar &pres)
  {
    etor_product_coeff_vectorized_unroller<UnrollingIndex-PacketSize, Lhs, Rhs, Packet>::run(row, col, lhs, rhs, innerDim, pres);
    pres = padd(pres, pmul( lhs.template packet<Aligned>(row, UnrollingIndex) , rhs.template packet<Aligned>(UnrollingIndex, col) ));
  }
};

template<typename Lhs, typename Rhs, typename Packet>
struct etor_product_coeff_vectorized_unroller<0, Lhs, Rhs, Packet>
{
  typedef typename Lhs::Index Index;
  static EIGEN_STRONG_INLINE void run(Index row, Index col, const Lhs& lhs, const Rhs& rhs, Index /*innerDim*/, typename Lhs::PacketScalar &pres)
  {
    pres = pmul(lhs.template packet<Aligned>(row, 0) , rhs.template packet<Aligned>(0, col));
  }
};

template<int UnrollingIndex, typename Lhs, typename Rhs, typename RetScalar>
struct etor_product_coeff_impl<InnerVectorizedTraversal, UnrollingIndex, Lhs, Rhs, RetScalar>
{
  typedef typename Lhs::PacketScalar Packet;
  typedef typename Lhs::Index Index;
  enum { PacketSize = packet_traits<typename Lhs::Scalar>::size };
  static EIGEN_STRONG_INLINE void run(Index row, Index col, const Lhs& lhs, const Rhs& rhs, Index innerDim, RetScalar &res)
  {
    Packet pres;
    etor_product_coeff_vectorized_unroller<UnrollingIndex+1-PacketSize, Lhs, Rhs, Packet>::run(row, col, lhs, rhs, innerDim, pres);
    res = predux(pres);
  }
};

template<typename Lhs, typename Rhs, int LhsRows = Lhs::RowsAtCompileTime, int RhsCols = Rhs::ColsAtCompileTime>
struct etor_product_coeff_vectorized_dyn_selector
{
  typedef typename Lhs::Index Index;
  static EIGEN_STRONG_INLINE void run(Index row, Index col, const Lhs& lhs, const Rhs& rhs, Index /*innerDim*/, typename Lhs::Scalar &res)
  {
    res = lhs.row(row).transpose().cwiseProduct(rhs.col(col)).sum();
  }
};

// NOTE the 3 following specializations are because taking .col(0) on a vector is a bit slower
// NOTE maybe they are now useless since we have a specialization for Block<Matrix>
template<typename Lhs, typename Rhs, int RhsCols>
struct etor_product_coeff_vectorized_dyn_selector<Lhs,Rhs,1,RhsCols>
{
  typedef typename Lhs::Index Index;
  static EIGEN_STRONG_INLINE void run(Index /*row*/, Index col, const Lhs& lhs, const Rhs& rhs, Index /*innerDim*/, typename Lhs::Scalar &res)
  {
    res = lhs.transpose().cwiseProduct(rhs.col(col)).sum();
  }
};

template<typename Lhs, typename Rhs, int LhsRows>
struct etor_product_coeff_vectorized_dyn_selector<Lhs,Rhs,LhsRows,1>
{
  typedef typename Lhs::Index Index;
  static EIGEN_STRONG_INLINE void run(Index row, Index /*col*/, const Lhs& lhs, const Rhs& rhs, Index /*innerDim*/, typename Lhs::Scalar &res)
  {
    res = lhs.row(row).transpose().cwiseProduct(rhs).sum();
  }
};

template<typename Lhs, typename Rhs>
struct etor_product_coeff_vectorized_dyn_selector<Lhs,Rhs,1,1>
{
  typedef typename Lhs::Index Index;
  EIGEN_STRONG_INLINE void run(Index /*row*/, Index /*col*/, const Lhs& lhs, const Rhs& rhs, Index /*innerDim*/, typename Lhs::Scalar &res)
  {
    res = lhs.transpose().cwiseProduct(rhs).sum();
  }
};

template<typename Lhs, typename Rhs, typename RetScalar>
struct etor_product_coeff_impl<InnerVectorizedTraversal, Dynamic, Lhs, Rhs, RetScalar>
{
  typedef typename Lhs::Index Index;
  static EIGEN_STRONG_INLINE void run(Index row, Index col, const Lhs& lhs, const Rhs& rhs, Index innerDim, typename Lhs::Scalar &res)
  {
    etor_product_coeff_vectorized_dyn_selector<Lhs,Rhs>::run(row, col, lhs, rhs, innerDim, res);
  }
};

/*******************
*** Packet path  ***
*******************/

template<int UnrollingIndex, typename Lhs, typename Rhs, typename Packet, int LoadMode>
struct etor_product_packet_impl<RowMajor, UnrollingIndex, Lhs, Rhs, Packet, LoadMode>
{
  typedef typename Lhs::Index Index;
  static EIGEN_STRONG_INLINE void run(Index row, Index col, const Lhs& lhs, const Rhs& rhs, Index innerDim, Packet &res)
  {
    etor_product_packet_impl<RowMajor, UnrollingIndex-1, Lhs, Rhs, Packet, LoadMode>::run(row, col, lhs, rhs, innerDim, res);
    res =  pmadd(pset1<Packet>(lhs.coeff(row, UnrollingIndex)), rhs.template packet<LoadMode>(UnrollingIndex, col), res);
  }
};

template<int UnrollingIndex, typename Lhs, typename Rhs, typename Packet, int LoadMode>
struct etor_product_packet_impl<ColMajor, UnrollingIndex, Lhs, Rhs, Packet, LoadMode>
{
  typedef typename Lhs::Index Index;
  static EIGEN_STRONG_INLINE void run(Index row, Index col, const Lhs& lhs, const Rhs& rhs, Index innerDim, Packet &res)
  {
    etor_product_packet_impl<ColMajor, UnrollingIndex-1, Lhs, Rhs, Packet, LoadMode>::run(row, col, lhs, rhs, innerDim, res);
    res =  pmadd(lhs.template packet<LoadMode>(row, UnrollingIndex), pset1<Packet>(rhs.coeff(UnrollingIndex, col)), res);
  }
};

template<typename Lhs, typename Rhs, typename Packet, int LoadMode>
struct etor_product_packet_impl<RowMajor, 0, Lhs, Rhs, Packet, LoadMode>
{
  typedef typename Lhs::Index Index;
  static EIGEN_STRONG_INLINE void run(Index row, Index col, const Lhs& lhs, const Rhs& rhs, Index /*innerDim*/, Packet &res)
  {
    res = pmul(pset1<Packet>(lhs.coeff(row, 0)),rhs.template packet<LoadMode>(0, col));
  }
};

template<typename Lhs, typename Rhs, typename Packet, int LoadMode>
struct etor_product_packet_impl<ColMajor, 0, Lhs, Rhs, Packet, LoadMode>
{
  typedef typename Lhs::Index Index;
  static EIGEN_STRONG_INLINE void run(Index row, Index col, const Lhs& lhs, const Rhs& rhs, Index /*innerDim*/, Packet &res)
  {
    res = pmul(lhs.template packet<LoadMode>(row, 0), pset1<Packet>(rhs.coeff(0, col)));
  }
};

template<typename Lhs, typename Rhs, typename Packet, int LoadMode>
struct etor_product_packet_impl<RowMajor, Dynamic, Lhs, Rhs, Packet, LoadMode>
{
  typedef typename Lhs::Index Index;
  static EIGEN_STRONG_INLINE void run(Index row, Index col, const Lhs& lhs, const Rhs& rhs, Index innerDim, Packet& res)
  {
    eigen_assert(innerDim>0 && "you are using a non initialized matrix");
    res = pmul(pset1<Packet>(lhs.coeff(row, 0)),rhs.template packet<LoadMode>(0, col));
    for(Index i = 1; i < innerDim; ++i)
      res =  pmadd(pset1<Packet>(lhs.coeff(row, i)), rhs.template packet<LoadMode>(i, col), res);
  }
};

template<typename Lhs, typename Rhs, typename Packet, int LoadMode>
struct etor_product_packet_impl<ColMajor, Dynamic, Lhs, Rhs, Packet, LoadMode>
{
  typedef typename Lhs::Index Index;
  static EIGEN_STRONG_INLINE void run(Index row, Index col, const Lhs& lhs, const Rhs& rhs, Index innerDim, Packet& res)
  {
    eigen_assert(innerDim>0 && "you are using a non initialized matrix");
    res = pmul(lhs.template packet<LoadMode>(row, 0), pset1<Packet>(rhs.coeff(0, col)));
    for(Index i = 1; i < innerDim; ++i)
      res =  pmadd(lhs.template packet<LoadMode>(row, i), pset1<Packet>(rhs.coeff(i, col)), res);
  }
};


/***************************************************************************
* Triangular products
***************************************************************************/

template<typename Lhs, typename Rhs, int ProductTag>
struct generic_product_impl<Lhs,Rhs,TriangularShape,DenseShape,ProductTag>
  : generic_product_impl_base<Lhs,Rhs,generic_product_impl<Lhs,Rhs,TriangularShape,DenseShape,ProductTag> >
{
  typedef typename Product<Lhs,Rhs>::Scalar Scalar;
  
  template<typename Dest>
  static void scaleAndAddTo(Dest& dst, const Lhs& lhs, const Rhs& rhs, const Scalar& alpha)
  {
    // TODO bypass TriangularProduct class
    TriangularProduct<Lhs::Mode,true,typename Lhs::MatrixType,false,Rhs, Rhs::IsVectorAtCompileTime>(lhs.nestedExpression(),rhs).scaleAndAddTo(dst, alpha);
  }
};

template<typename Lhs, typename Rhs, int ProductTag>
struct product_evaluator<Product<Lhs, Rhs, DefaultProduct>, ProductTag, TriangularShape, DenseShape, typename Lhs::Scalar, typename Rhs::Scalar> 
  : public evaluator<typename Product<Lhs, Rhs, DefaultProduct>::PlainObject>::type
{
  typedef Product<Lhs, Rhs, DefaultProduct> XprType;
  typedef typename XprType::PlainObject PlainObject;
  typedef typename evaluator<PlainObject>::type Base;

  product_evaluator(const XprType& xpr)
    : m_result(xpr.rows(), xpr.cols())
  {
    ::new (static_cast<Base*>(this)) Base(m_result);
    generic_product_impl<Lhs, Rhs, TriangularShape, DenseShape, ProductTag>::evalTo(m_result, xpr.lhs(), xpr.rhs());
  }
  
protected:  
  PlainObject m_result;
};


template<typename Lhs, typename Rhs, int ProductTag>
struct generic_product_impl<Lhs,Rhs,DenseShape,TriangularShape,ProductTag>
: generic_product_impl_base<Lhs,Rhs,generic_product_impl<Lhs,Rhs,DenseShape,TriangularShape,ProductTag> >
{
  typedef typename Product<Lhs,Rhs>::Scalar Scalar;
  
  template<typename Dest>
  static void scaleAndAddTo(Dest& dst, const Lhs& lhs, const Rhs& rhs, const Scalar& alpha)
  {
    // TODO bypass TriangularProduct class
    TriangularProduct<Rhs::Mode,false,Lhs,Lhs::IsVectorAtCompileTime, typename Rhs::MatrixType, false>(lhs,rhs.nestedExpression()).scaleAndAddTo(dst, alpha);
  }
};

template<typename Lhs, typename Rhs, int ProductTag>
struct product_evaluator<Product<Lhs, Rhs, DefaultProduct>, ProductTag, DenseShape, TriangularShape, typename Lhs::Scalar, typename Rhs::Scalar> 
  : public evaluator<typename Product<Lhs, Rhs, DefaultProduct>::PlainObject>::type
{
  typedef Product<Lhs, Rhs, DefaultProduct> XprType;
  typedef typename XprType::PlainObject PlainObject;
  typedef typename evaluator<PlainObject>::type Base;

  product_evaluator(const XprType& xpr)
    : m_result(xpr.rows(), xpr.cols())
  {
    ::new (static_cast<Base*>(this)) Base(m_result);
    generic_product_impl<Lhs, Rhs, DenseShape, TriangularShape, ProductTag>::evalTo(m_result, xpr.lhs(), xpr.rhs());
  }
  
protected:  
  PlainObject m_result;
};



/***************************************************************************
* SelfAdjoint products
***************************************************************************/

template<typename Lhs, typename Rhs, int ProductTag>
struct generic_product_impl<Lhs,Rhs,SelfAdjointShape,DenseShape,ProductTag>
  : generic_product_impl_base<Lhs,Rhs,generic_product_impl<Lhs,Rhs,SelfAdjointShape,DenseShape,ProductTag> >
{
  typedef typename Product<Lhs,Rhs>::Scalar Scalar;
  
  template<typename Dest>
  static void scaleAndAddTo(Dest& dst, const Lhs& lhs, const Rhs& rhs, const Scalar& alpha)
  {// SelfadjointProductMatrix<MatrixType,Mode,false,OtherDerived,0,OtherDerived::IsVectorAtCompileTime>
    // TODO bypass SelfadjointProductMatrix class
    SelfadjointProductMatrix<typename Lhs::MatrixType,Lhs::Mode,false,Rhs,0,Rhs::IsVectorAtCompileTime>(lhs.nestedExpression(),rhs).scaleAndAddTo(dst, alpha);
  }
};

template<typename Lhs, typename Rhs, int ProductTag>
struct product_evaluator<Product<Lhs, Rhs, DefaultProduct>, ProductTag, SelfAdjointShape, DenseShape, typename Lhs::Scalar, typename Rhs::Scalar> 
  : public evaluator<typename Product<Lhs, Rhs, DefaultProduct>::PlainObject>::type
{
  typedef Product<Lhs, Rhs, DefaultProduct> XprType;
  typedef typename XprType::PlainObject PlainObject;
  typedef typename evaluator<PlainObject>::type Base;

  product_evaluator(const XprType& xpr)
    : m_result(xpr.rows(), xpr.cols())
  {
    ::new (static_cast<Base*>(this)) Base(m_result);
    generic_product_impl<Lhs, Rhs, SelfAdjointShape, DenseShape, ProductTag>::evalTo(m_result, xpr.lhs(), xpr.rhs());
  }
  
protected:  
  PlainObject m_result;
};


template<typename Lhs, typename Rhs, int ProductTag>
struct generic_product_impl<Lhs,Rhs,DenseShape,SelfAdjointShape,ProductTag>
: generic_product_impl_base<Lhs,Rhs,generic_product_impl<Lhs,Rhs,DenseShape,SelfAdjointShape,ProductTag> >
{
  typedef typename Product<Lhs,Rhs>::Scalar Scalar;
  
  template<typename Dest>
  static void scaleAndAddTo(Dest& dst, const Lhs& lhs, const Rhs& rhs, const Scalar& alpha)
  {//SelfadjointProductMatrix<OtherDerived,0,OtherDerived::IsVectorAtCompileTime,MatrixType,Mode,false>
    // TODO bypass SelfadjointProductMatrix class
    SelfadjointProductMatrix<Lhs,0,Lhs::IsVectorAtCompileTime,typename Rhs::MatrixType,Rhs::Mode,false>(lhs,rhs.nestedExpression()).scaleAndAddTo(dst, alpha);
  }
};

template<typename Lhs, typename Rhs, int ProductTag>
struct product_evaluator<Product<Lhs, Rhs, DefaultProduct>, ProductTag, DenseShape, SelfAdjointShape, typename Lhs::Scalar, typename Rhs::Scalar> 
  : public evaluator<typename Product<Lhs, Rhs, DefaultProduct>::PlainObject>::type
{
  typedef Product<Lhs, Rhs, DefaultProduct> XprType;
  typedef typename XprType::PlainObject PlainObject;
  typedef typename evaluator<PlainObject>::type Base;

  product_evaluator(const XprType& xpr)
    : m_result(xpr.rows(), xpr.cols())
  {
    ::new (static_cast<Base*>(this)) Base(m_result);
    generic_product_impl<Lhs, Rhs, DenseShape, SelfAdjointShape, ProductTag>::evalTo(m_result, xpr.lhs(), xpr.rhs());
  }
  
protected:  
  PlainObject m_result;
};


} // end namespace internal

} // end namespace Eigen

#endif // EIGEN_PRODUCT_EVALUATORS_H
