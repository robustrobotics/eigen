// This file is part of Eigen, a lightweight C++ template library
// for linear algebra. Eigen itself is part of the KDE project.
//
// Copyright (C) 2007 Michael Olbrich <michael.olbrich@gmx.net>
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

#ifndef EI_LOOP_H
#define EI_LOOP_H

template<int UnrollCount, int Rows> class EiLoop
{
    static const int col = (UnrollCount-1) / Rows;
    static const int row = (UnrollCount-1) % Rows;

  public:
    template <typename Derived1, typename Derived2>
    static void copy(Derived1 &dst, const Derived2 &src)
    {
      EiLoop<UnrollCount-1, Rows>::copy(dst, src);
      dst.write(row, col) = src.read(row, col);
    }
};

template<int Rows> class EiLoop<0, Rows>
{
  public:
    template <typename Derived1, typename Derived2>
    static void copy(Derived1 &dst, const Derived2 &src)
    {
      EI_UNUSED(dst);
      EI_UNUSED(src);
    }
};

#endif //EI_LOOP_H
