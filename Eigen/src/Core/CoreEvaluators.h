// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2011 Benoit Jacob <jacob.benoit.1@gmail.com>
// Copyright (C) 2011 Gael Guennebaud <gael.guennebaud@inria.fr>
// Copyright (C) 2011 Jitse Niesen <jitse@maths.leeds.ac.uk>
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


#ifndef EIGEN_COREEVALUATORS_H
#define EIGEN_COREEVALUATORS_H

namespace internal {
  
template<typename T>
struct evaluator_impl {};

template<typename T>
struct evaluator
{
  typedef evaluator_impl<T> type;
};

template<typename T>
struct evaluator<const T>
{
  typedef evaluator_impl<T> type;
};

// -------------------- Transpose --------------------

template<typename ExpressionType>
struct evaluator_impl<Transpose<ExpressionType> >
{
  typedef Transpose<ExpressionType> TransposeType;
  evaluator_impl(const TransposeType& t) : m_argImpl(t.nestedExpression()) {}

  typedef typename TransposeType::Index Index;

  typename TransposeType::CoeffReturnType coeff(Index i, Index j) const
  {
    return m_argImpl.coeff(j, i);
  }

  typename TransposeType::CoeffReturnType coeff(Index index) const
  {
    return m_argImpl.coeff(index);
  }

  typename TransposeType::Scalar& coeffRef(Index i, Index j)
  {
    return m_argImpl.coeffRef(j, i);
  }

  typename TransposeType::Scalar& coeffRef(Index index)
  {
    return m_argImpl.coeffRef(index);
  }

  // TODO: Difference between PacketScalar and PacketReturnType?
  template<int LoadMode>
  const typename ExpressionType::PacketScalar packet(Index row, Index col) const
  {
    return m_argImpl.template packet<LoadMode>(col, row);
  }

  template<int LoadMode>
  const typename ExpressionType::PacketScalar packet(Index index) const
  {
    return m_argImpl.template packet<LoadMode>(index);
  }

  template<int StoreMode> 
  void writePacket(Index row, Index col, const typename ExpressionType::PacketScalar& x)
  {
    m_argImpl.template writePacket<StoreMode>(col, row, x);
  }

  template<int StoreMode> 
  void writePacket(Index index, const typename ExpressionType::PacketScalar& x)
  {
    m_argImpl.template writePacket<StoreMode>(index, x);
  }

protected:
  typename evaluator<ExpressionType>::type m_argImpl;
};

// -------------------- Matrix --------------------

template<typename Scalar, int Rows, int Cols, int Options, int MaxRows, int MaxCols>
struct evaluator_impl<Matrix<Scalar, Rows, Cols, Options, MaxRows, MaxCols> >
{
  typedef Matrix<Scalar, Rows, Cols, Options, MaxRows, MaxCols> MatrixType;

  evaluator_impl(const MatrixType& m) : m_matrix(m) {}

  typedef typename MatrixType::Index Index;

  typename MatrixType::CoeffReturnType coeff(Index i, Index j) const
  {
    return m_matrix.coeff(i, j);
  }

  typename MatrixType::CoeffReturnType coeff(Index index) const
  {
    return m_matrix.coeff(index);
  }

  typename MatrixType::Scalar& coeffRef(Index i, Index j)
  {
    return m_matrix.const_cast_derived().coeffRef(i, j);
  }

  typename MatrixType::Scalar& coeffRef(Index index)
  {
    return m_matrix.const_cast_derived().coeffRef(index);
  }

  template<int LoadMode> 
  typename MatrixType::PacketReturnType packet(Index row, Index col) const
  {
    return m_matrix.template packet<LoadMode>(row, col);
  }

  template<int LoadMode> 
  typename MatrixType::PacketReturnType packet(Index index) const
  {
    // eigen_internal_assert(index >= 0 && index < size());
    return m_matrix.template packet<LoadMode>(index);
  }

  template<int StoreMode> 
  void writePacket(Index row, Index col, const typename MatrixType::PacketScalar& x)
  {
    m_matrix.const_cast_derived().template writePacket<StoreMode>(row, col, x);
  }

  template<int StoreMode> 
  void writePacket(Index index, const typename MatrixType::PacketScalar& x)
  {
    // eigen_internal_assert(index >= 0 && index < size());
    m_matrix.const_cast_derived().template writePacket<StoreMode>(index, x);
  }

protected:
  const MatrixType &m_matrix;
};

// -------------------- Array --------------------

// TODO: should be sharing code with Matrix case

template<typename Scalar, int Rows, int Cols, int Options, int MaxRows, int MaxCols>
struct evaluator_impl<Array<Scalar, Rows, Cols, Options, MaxRows, MaxCols> >
{
  typedef Array<Scalar, Rows, Cols, Options, MaxRows, MaxCols> ArrayType;

  evaluator_impl(const ArrayType& a) : m_array(a) {}

  typedef typename ArrayType::Index Index;
  
  typename ArrayType::CoeffReturnType coeff(Index i, Index j) const
  {
    return m_array.coeff(i, j);
  }

  typename ArrayType::CoeffReturnType coeff(Index index) const
  {
    return m_array.coeff(index);
  }

  typename ArrayType::Scalar& coeffRef(Index i, Index j)
  {
    return m_array.const_cast_derived().coeffRef(i, j);
  }

  typename ArrayType::Scalar& coeffRef(Index index)
  {
    return m_array.const_cast_derived().coeffRef(index);
  }

  template<int LoadMode> 
  typename ArrayType::PacketReturnType packet(Index row, Index col) const
  {
    return m_array.template packet<LoadMode>(row, col);
  }

  template<int LoadMode> 
  typename ArrayType::PacketReturnType packet(Index index) const
  {
    // eigen_internal_assert(index >= 0 && index < size());
    return m_array.template packet<LoadMode>(index);
  }

  template<int StoreMode> 
  void writePacket(Index row, Index col, const typename ArrayType::PacketScalar& x)
  {
    m_array.const_cast_derived().template writePacket<StoreMode>(row, col, x);
  }

  template<int StoreMode> 
  void writePacket(Index index, const typename ArrayType::PacketScalar& x)
  {
    // eigen_internal_assert(index >= 0 && index < size());
    m_array.const_cast_derived().template writePacket<StoreMode>(index, x);
  }

protected:
  const ArrayType &m_array;
};

// -------------------- CwiseNullaryOp --------------------

template<typename NullaryOp, typename PlainObjectType>
struct evaluator_impl<CwiseNullaryOp<NullaryOp,PlainObjectType> >
{
  typedef CwiseNullaryOp<NullaryOp,PlainObjectType> NullaryOpType;

  evaluator_impl(const NullaryOpType& n) : m_nullaryOp(n) {}

  typedef typename NullaryOpType::Index Index;

  typename NullaryOpType::CoeffReturnType coeff(Index i, Index j) const
  {
    return m_nullaryOp.coeff(i, j);
  }

  typename NullaryOpType::CoeffReturnType coeff(Index index) const
  {
    return m_nullaryOp.coeff(index);
  }

  template<int LoadMode>
  typename NullaryOpType::PacketScalar packet(Index index) const
  {
    return m_nullaryOp.template packet<LoadMode>(index);
  }

protected:
  const NullaryOpType& m_nullaryOp;
};

// -------------------- CwiseUnaryOp --------------------

template<typename UnaryOp, typename ArgType>
struct evaluator_impl<CwiseUnaryOp<UnaryOp, ArgType> >
{
  typedef CwiseUnaryOp<UnaryOp, ArgType> UnaryOpType;

  evaluator_impl(const UnaryOpType& op) : m_unaryOp(op), m_argImpl(op.nestedExpression()) {}

  typedef typename UnaryOpType::Index Index;

  typename UnaryOpType::CoeffReturnType coeff(Index i, Index j) const
  {
    return m_unaryOp.functor()(m_argImpl.coeff(i, j));
  }

  typename UnaryOpType::CoeffReturnType coeff(Index index) const
  {
    return m_unaryOp.functor()(m_argImpl.coeff(index));
  }

  template<int LoadMode>
  typename UnaryOpType::PacketScalar packet(Index index) const
  {
    return m_unaryOp.functor().packetOp(m_argImpl.template packet<LoadMode>(index));
  }

  template<int LoadMode>
  typename UnaryOpType::PacketScalar packet(Index row, Index col) const
  {
    return m_unaryOp.functor().packetOp(m_argImpl.template packet<LoadMode>(row, col));
  }

protected:
  const UnaryOpType& m_unaryOp;
  typename evaluator<ArgType>::type m_argImpl;
};

// -------------------- CwiseBinaryOp --------------------

template<typename BinaryOp, typename Lhs, typename Rhs>
struct evaluator_impl<CwiseBinaryOp<BinaryOp, Lhs, Rhs> >
{
  typedef CwiseBinaryOp<BinaryOp, Lhs, Rhs> BinaryOpType;

  evaluator_impl(const BinaryOpType& xpr) : m_binaryOp(xpr), m_lhsImpl(xpr.lhs()), m_rhsImpl(xpr.rhs())  {}

  typedef typename BinaryOpType::Index Index;

  typename BinaryOpType::CoeffReturnType coeff(Index i, Index j) const
  {
    return m_binaryOp.functor()(m_lhsImpl.coeff(i, j), m_rhsImpl.coeff(i, j));
  }

  typename BinaryOpType::CoeffReturnType coeff(Index index) const
  {
    return m_binaryOp.functor()(m_lhsImpl.coeff(index), m_rhsImpl.coeff(index));
  }

  template<int LoadMode>
  typename BinaryOpType::PacketScalar packet(Index index) const
  {
    return m_binaryOp.functor().packetOp(m_lhsImpl.template packet<LoadMode>(index),
					 m_rhsImpl.template packet<LoadMode>(index));
  }

  template<int LoadMode>
  typename BinaryOpType::PacketScalar packet(Index row, Index col) const
  {
    return m_binaryOp.functor().packetOp(m_lhsImpl.template packet<LoadMode>(row, col),
					 m_rhsImpl.template packet<LoadMode>(row, col));
  }

protected:
  const BinaryOpType& m_binaryOp;
  typename evaluator<Lhs>::type m_lhsImpl;
  typename evaluator<Rhs>::type m_rhsImpl;
};

// -------------------- Product --------------------

template<typename Lhs, typename Rhs>
struct evaluator_impl<Product<Lhs,Rhs> > : public evaluator<typename Product<Lhs,Rhs>::PlainObject>::type
{
  typedef Product<Lhs,Rhs> XprType;
  typedef typename XprType::PlainObject PlainObject;
  typedef typename evaluator<PlainObject>::type evaluator_base;
  
//   enum {
//     EvaluateLhs = ;
//     EvaluateRhs = ;
//   };
  
  evaluator_impl(const XprType& product) : evaluator_base(m_result)
  {
    // here we process the left and right hand sides with a specialized evaluator
    // perhaps this step should be done by the TreeOptimizer to get a canonical tree and reduce evaluator instanciations
    // typename product_operand_evaluator<Lhs>::type m_lhsImpl(product.lhs());
    // typename product_operand_evaluator<Rhs>::type m_rhsImpl(product.rhs());
  
    // TODO do not rely on previous product mechanism !!
    m_result.resize(product.rows(), product.cols());
    m_result.noalias() = product.lhs() * product.rhs();
  }
  
protected:  
  PlainObject m_result;
};

// -------------------- Block --------------------
//
// This evaluator is implemented as a dumb wrapper around Block expression class.
// TODO: Make this a real evaluator

template<typename XprType, int BlockRows, int BlockCols, bool InnerPanel, bool HasDirectAccess> 
struct evaluator_impl<Block<XprType, BlockRows, BlockCols, InnerPanel, HasDirectAccess> >
{
  typedef Block<XprType, BlockRows, BlockCols, InnerPanel, HasDirectAccess> BlockType;
  evaluator_impl(const BlockType& block) : m_block(block) { }
 
  typedef typename BlockType::Index Index;
  typedef typename BlockType::Scalar Scalar;
  typedef typename BlockType::CoeffReturnType CoeffReturnType;
  typedef typename BlockType::PacketScalar PacketScalar;
  typedef typename BlockType::PacketReturnType PacketReturnType;
 
 
  CoeffReturnType coeff(Index i, Index j) const 
  { 
    return m_block.coeff(i,j); 
  }
  
  CoeffReturnType coeff(Index index) const 
  { 
    return m_block.coeff(index); 
  }

  Scalar& coeffRef(Index i, Index j) 
  { 
    return m_block.const_cast_derived().coeffRef(i,j); 
  }
  
  Scalar& coeffRef(Index index) 
  { 
    return m_block.const_cast_derived().coeffRef(index); 
  }
 
  template<int LoadMode> 
  PacketReturnType packet(Index row, Index col) const 
  { 
    return m_block.template packet<LoadMode>(row, col); 
  }

  template<int LoadMode> 
  PacketReturnType packet(Index index) const 
  { 
    return m_block.template packet<LoadMode>(index); 
  }
  
  template<int StoreMode> 
  void writePacket(Index row, Index col, const PacketScalar& x) 
  { 
    m_block.const_cast_derived().template writePacket<StoreMode>(row, col, x); 
  }
  
  template<int StoreMode> 
  void writePacket(Index index, const PacketScalar& x) 
  { 
    m_block.const_cast_derived().template writePacket<StoreMode>(index, x); 
  }
 
protected:
  const BlockType& m_block;
};


} // namespace internal

#endif // EIGEN_COREEVALUATORS_H
