// This file is part of Eigen, a lightweight C++ template library
// for linear algebra. Eigen itself is part of the KDE project.
//
// Copyright (C) 2006-2008 Benoit Jacob <jacob@math.jussieu.fr>
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

template<typename VectorType> void tmap(const VectorType& m)
{
  typedef typename VectorType::Scalar Scalar;

  int size = m.size();

  // test Map.h
  Scalar* array1 = ei_aligned_malloc<Scalar>(size);
  Scalar* array2 = ei_aligned_malloc<Scalar>(size);
  Map<VectorType, Aligned>(array1, size) = VectorType::random(size);
  Map<VectorType>(array2, size) = Map<VectorType>(array1, size);
  VectorType ma1 = Map<VectorType>(array1, size);
  VectorType ma2 = Map<VectorType, Aligned>(array2, size);
  VERIFY_IS_APPROX(ma1, ma2);
  ei_aligned_free(array1);
  ei_aligned_free(array2);
}

void test_map()
{
  for(int i = 0; i < g_repeat; i++) {
    CALL_SUBTEST( tmap(Matrix<float, 1, 1>()) );
    CALL_SUBTEST( tmap(Vector4d()) );
    CALL_SUBTEST( tmap(RowVector4f()) );
    CALL_SUBTEST( tmap(VectorXcf(8)) );
    CALL_SUBTEST( tmap(VectorXi(12)) );
  }
}
