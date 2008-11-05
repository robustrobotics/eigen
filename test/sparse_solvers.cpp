// This file is part of Eigen, a lightweight C++ template library
// for linear algebra. Eigen itself is part of the KDE project.
//
// Copyright (C) 2008 Daniel Gomez Ferro <dgomezferro@gmail.com>
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

#include "sparse.h"

template<typename Scalar> void sparse_solvers(int rows, int cols)
{
  double density = std::max(8./(rows*cols), 0.01);
  typedef Matrix<Scalar,Dynamic,Dynamic> DenseMatrix;
  typedef Matrix<Scalar,Dynamic,1> DenseVector;
  Scalar eps = 1e-6;

  DenseVector vec1 = DenseVector::Random(rows);

  std::vector<Vector2i> zeroCoords;
  std::vector<Vector2i> nonzeroCoords;

  // test triangular solver
  {
    DenseVector vec2 = vec1, vec3 = vec1;
    SparseMatrix<Scalar> m2(rows, cols);
    DenseMatrix refMat2 = DenseMatrix::Zero(rows, cols);

    // lower
    initSparse<Scalar>(density, refMat2, m2, ForceNonZeroDiag|MakeLowerTriangular, &zeroCoords, &nonzeroCoords);
    VERIFY_IS_APPROX(refMat2.template marked<Lower>().solveTriangular(vec2),
                     m2.template marked<Lower>().solveTriangular(vec3));

    // upper
    initSparse<Scalar>(density, refMat2, m2, ForceNonZeroDiag|MakeUpperTriangular, &zeroCoords, &nonzeroCoords);
    VERIFY_IS_APPROX(refMat2.template marked<Upper>().solveTriangular(vec2),
                     m2.template marked<Upper>().solveTriangular(vec3));

    // TODO test row major
  }

  // test LLT
  if (!NumTraits<Scalar>::IsComplex)
  {
    // TODO fix the issue with complex (see SparseLLT::solveInPlace)
    SparseMatrix<Scalar> m2(rows, cols);
    DenseMatrix refMat2(rows, cols);

    DenseVector b = DenseVector::Random(cols);
    DenseVector refX(cols), x(cols);

    initSparse<Scalar>(density, refMat2, m2, ForceNonZeroDiag|MakeLowerTriangular, &zeroCoords, &nonzeroCoords);
    refMat2 += refMat2.adjoint();
    refMat2.diagonal() *= 0.5;

    refMat2.llt().solve(b, &refX);
    typedef SparseMatrix<Scalar,Lower|SelfAdjoint> SparseSelfAdjointMatrix;
    x = b;
    SparseLLT<SparseSelfAdjointMatrix> (m2).solveInPlace(x);
    //VERIFY(refX.isApprox(x,test_precision<Scalar>()) && "LLT: default");
    #ifdef EIGEN_CHOLMOD_SUPPORT
    x = b;
    SparseLLT<SparseSelfAdjointMatrix,Cholmod>(m2).solveInPlace(x);
    VERIFY(refX.isApprox(x,test_precision<Scalar>()) && "LLT: cholmod");
    #endif
    #ifdef EIGEN_TAUCS_SUPPORT
    x = b;
    SparseLLT<SparseSelfAdjointMatrix,Taucs>(m2,IncompleteFactorization).solveInPlace(x);
    VERIFY(refX.isApprox(x,test_precision<Scalar>()) && "LLT: taucs (IncompleteFactorization)");
    x = b;
    SparseLLT<SparseSelfAdjointMatrix,Taucs>(m2,SupernodalMultifrontal).solveInPlace(x);
    VERIFY(refX.isApprox(x,test_precision<Scalar>()) && "LLT: taucs (SupernodalMultifrontal)");
    x = b;
    SparseLLT<SparseSelfAdjointMatrix,Taucs>(m2,SupernodalLeftLooking).solveInPlace(x);
    VERIFY(refX.isApprox(x,test_precision<Scalar>()) && "LLT: taucs (SupernodalLeftLooking)");
    #endif
  }

  // test LDLT
  if (!NumTraits<Scalar>::IsComplex)
  {
    // TODO fix the issue with complex (see SparseLDT::solveInPlace)
    SparseMatrix<Scalar> m2(rows, cols);
    DenseMatrix refMat2(rows, cols);

    DenseVector b = DenseVector::Random(cols);
    DenseVector refX(cols), x(cols);

    initSparse<Scalar>(density, refMat2, m2, ForceNonZeroDiag|MakeUpperTriangular, &zeroCoords, &nonzeroCoords);
    refMat2 += refMat2.adjoint();
    refMat2.diagonal() *= 0.5;

    refMat2.ldlt().solve(b, &refX);
    typedef SparseMatrix<Scalar,Lower|SelfAdjoint> SparseSelfAdjointMatrix;
    x = b;
    SparseLDLT<SparseSelfAdjointMatrix> ldlt(m2);
    if (ldlt.succeeded())
      ldlt.solveInPlace(x);
    VERIFY(refX.isApprox(x,test_precision<Scalar>()) && "LDLT: default");
  }

  // test LU
  {
    static int count = 0;
    SparseMatrix<Scalar> m2(rows, cols);
    DenseMatrix refMat2(rows, cols);

    DenseVector b = DenseVector::Random(cols);
    DenseVector refX(cols), x(cols);

    initSparse<Scalar>(density, refMat2, m2, ForceNonZeroDiag, &zeroCoords, &nonzeroCoords);

    LU<DenseMatrix> refLu(refMat2);
    refLu.solve(b, &refX);
    Scalar refDet = refLu.determinant();
    x.setZero();
    // // SparseLU<SparseMatrix<Scalar> > (m2).solve(b,&x);
    // // VERIFY(refX.isApprox(x,test_precision<Scalar>()) && "LU: default");
    #ifdef EIGEN_SUPERLU_SUPPORT
    {
      x.setZero();
      SparseLU<SparseMatrix<Scalar>,SuperLU> slu(m2);
      if (slu.succeeded())
      {
        if (slu.solve(b,&x)) {
          VERIFY(refX.isApprox(x,test_precision<Scalar>()) && "LU: SuperLU");
        }
        // std::cerr << refDet << " == " << slu.determinant() << "\n";
        if (count==0) {
          VERIFY_IS_APPROX(refDet,slu.determinant()); // FIXME det is not very stable for complex
        }
      }
    }
    #endif
    #ifdef EIGEN_UMFPACK_SUPPORT
    {
      // check solve
      x.setZero();
      SparseLU<SparseMatrix<Scalar>,UmfPack> slu(m2);
      if (slu.succeeded()) {
        if (slu.solve(b,&x)) {
          if (count==0) {
            VERIFY(refX.isApprox(x,test_precision<Scalar>()) && "LU: umfpack");  // FIXME solve is not very stable for complex
          }
        }
        VERIFY_IS_APPROX(refDet,slu.determinant());
        // TODO check the extracted data
        //std::cerr << slu.matrixL() << "\n";
      }
    }
    #endif
    count++;
  }

}

void test_sparse_solvers()
{
  for(int i = 0; i < g_repeat; i++) {
    CALL_SUBTEST( sparse_solvers<double>(8, 8) );
    CALL_SUBTEST( sparse_solvers<std::complex<double> >(16, 16) );
    CALL_SUBTEST( sparse_solvers<double>(33, 33) );
  }
}
