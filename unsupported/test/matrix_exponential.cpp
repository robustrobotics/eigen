// This file is part of Eigen, a lightweight C++ template library
// for linear algebra.
//
// Copyright (C) 2009 Jitse Niesen <jitse@maths.leeds.ac.uk>
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

#include "main.h"
#include <unsupported/Eigen/MatrixFunctions>

double binom(int n, int k)
{
  double res = 1;
  for (int i=0; i<k; i++)
    res = res * (n-k+i+1) / (i+1);
  return res;
}

template <typename Derived, typename OtherDerived>
double relerr(const MatrixBase<Derived>& A, const MatrixBase<OtherDerived>& B)
{
  return std::sqrt((A - B).cwiseAbs2().sum() / std::min(A.cwiseAbs2().sum(), B.cwiseAbs2().sum()));
}

template <typename T>
T expfn(T x, int)
{
  return std::exp(x);
}

template <typename T>
void test2dRotation(double tol)
{
  Matrix<T,2,2> A, B, C;
  T angle;

  A << 0, 1, -1, 0;
  for (int i=0; i<=20; i++)
  {
    angle = static_cast<T>(pow(10, i / 5. - 2));
    B << cos(angle), sin(angle), -sin(angle), cos(angle);

    C = ei_matrix_function(angle*A, expfn);
    std::cout << "test2dRotation: i = " << i << "   error funm = " << relerr(C, B);
    VERIFY(C.isApprox(B, static_cast<T>(tol)));

    C = ei_matrix_exponential(angle*A);
    std::cout << "   error expm = " << relerr(C, B) << "\n";
    VERIFY(C.isApprox(B, static_cast<T>(tol)));
  }
}

template <typename T>
void test2dHyperbolicRotation(double tol)
{
  Matrix<std::complex<T>,2,2> A, B, C;
  std::complex<T> imagUnit(0,1);
  T angle, ch, sh;

  for (int i=0; i<=20; i++)
  {
    angle = static_cast<T>((i-10) / 2.0);
    ch = std::cosh(angle);
    sh = std::sinh(angle);
    A << 0, angle*imagUnit, -angle*imagUnit, 0;
    B << ch, sh*imagUnit, -sh*imagUnit, ch;

    C = ei_matrix_function(A, expfn);
    std::cout << "test2dHyperbolicRotation: i = " << i << "   error funm = " << relerr(C, B);
    VERIFY(C.isApprox(B, static_cast<T>(tol)));

    C = ei_matrix_exponential(A);
    std::cout << "   error expm = " << relerr(C, B) << "\n";
    VERIFY(C.isApprox(B, static_cast<T>(tol)));
  }
}

template <typename T>
void testPascal(double tol)
{
  for (int size=1; size<20; size++)
  {
    Matrix<T,Dynamic,Dynamic> A(size,size), B(size,size), C(size,size);
    A.setZero();
    for (int i=0; i<size-1; i++)
      A(i+1,i) = static_cast<T>(i+1);
    B.setZero();
    for (int i=0; i<size; i++)
      for (int j=0; j<=i; j++)
    B(i,j) = static_cast<T>(binom(i,j));

    C = ei_matrix_function(A, expfn);
    std::cout << "testPascal: size = " << size << "   error funm = " << relerr(C, B);
    VERIFY(C.isApprox(B, static_cast<T>(tol)));

    C = ei_matrix_exponential(A);
    std::cout << "   error expm = " << relerr(C, B) << "\n";
    VERIFY(C.isApprox(B, static_cast<T>(tol)));
  }
}

template<typename MatrixType>
void randomTest(const MatrixType& m, double tol)
{
  /* this test covers the following files:
     Inverse.h
  */
  int rows = m.rows();
  int cols = m.cols();
  MatrixType m1(rows, cols), m2(rows, cols), m3(rows, cols),
             identity = MatrixType::Identity(rows, rows);

  typedef typename NumTraits<typename ei_traits<MatrixType>::Scalar>::Real RealScalar;

  for(int i = 0; i < g_repeat; i++) {
    m1 = MatrixType::Random(rows, cols);

    m2 = ei_matrix_function(m1, expfn) * ei_matrix_function(-m1, expfn);
    std::cout << "randomTest: error funm = " << relerr(identity, m2);
    VERIFY(identity.isApprox(m2, static_cast<RealScalar>(tol)));

    m2 = ei_matrix_exponential(m1) * ei_matrix_exponential(-m1);
    std::cout << "   error expm = " << relerr(identity, m2) << "\n";
    VERIFY(identity.isApprox(m2, static_cast<RealScalar>(tol)));
  }
}

void test_matrix_exponential()
{
  CALL_SUBTEST_2(test2dRotation<double>(1e-13));
  CALL_SUBTEST_1(test2dRotation<float>(1e-5));
  CALL_SUBTEST_2(test2dHyperbolicRotation<double>(1e-14));
  CALL_SUBTEST_1(test2dHyperbolicRotation<float>(1e-5));
  CALL_SUBTEST_6(testPascal<float>(1e-6));
  CALL_SUBTEST_5(testPascal<double>(1e-15));
  CALL_SUBTEST_2(randomTest(Matrix2d(), 1e-13));
  CALL_SUBTEST_7(randomTest(Matrix<double,3,3,RowMajor>(), 1e-13));
  CALL_SUBTEST_3(randomTest(Matrix4cd(), 1e-13));
  CALL_SUBTEST_4(randomTest(MatrixXd(8,8), 1e-13));
  CALL_SUBTEST_1(randomTest(Matrix2f(), 1e-4));
  CALL_SUBTEST_5(randomTest(Matrix3cf(), 1e-4));
  CALL_SUBTEST_1(randomTest(Matrix4f(), 1e-4));
  CALL_SUBTEST_6(randomTest(MatrixXf(8,8), 1e-4));
}
