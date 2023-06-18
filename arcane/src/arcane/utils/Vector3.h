﻿// -*- tab-width: 2; indent-tabs-mode: nil; coding: utf-8-with-signature -*-
//-----------------------------------------------------------------------------
// Copyright 2000-2023 CEA (www.cea.fr) IFPEN (www.ifpenergiesnouvelles.com)
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: Apache-2.0
//-----------------------------------------------------------------------------
/*---------------------------------------------------------------------------*/
/* Vector3.h                                                   (C) 2000-2023 */
/*                                                                           */
/* Vecteur à 3 dimensions.                                                   */
/*---------------------------------------------------------------------------*/
#ifndef ARCANE_UTILS_VECTOR3_H
#define ARCANE_UTILS_VECTOR3_H
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#include "arcane/utils/UtilsTypes.h"

#include <iostream>

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

namespace Arcane
{

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Classe gérant un vecteur de dimension 3 de type \a T
 *
 * Les 3 éléments du vecteur peuvent être accéder par \a x, \a y ou \a z.
 */
template <typename T>
class ARCANE_UTILS_EXPORT Vector3
{
 public:

  T x = {};
  T y = {};
  T z = {};

 public:

  //! Construit le vecteur nul.
  constexpr ARCCORE_HOST_DEVICE
  Vector3() = default;

  //! Construit le triplet (ax,ay,az)
  constexpr ARCCORE_HOST_DEVICE Vector3(const T& ax, const T& ay, const T& az)
  : x(ax)
  , y(ay)
  , z(az)
  {
  }

  //! Construit l'instance avec le triplet (v,v,v).
  constexpr ARCCORE_HOST_DEVICE explicit Vector3(const T& v)
  : x(v)
  , y(v)
  , z(v)
  {
  }

  friend constexpr ARCCORE_HOST_DEVICE bool
  operator<(const Vector3<T>& v1, const Vector3<T>& v2)
  {
    if (v1.x == v2.x) {
      if (v1.y == v2.y)
        return v1.z < v2.z;
      else
        return v1.y < v2.y;
    }
    return (v1.x < v2.x);
  }

  //! Ecrit le triplet \a t sur le flot \a o
  friend std::ostream& operator<<(std::ostream& o, const Vector3<T>& t)
  {
    t._print(o);
    return o;
  }

  friend constexpr ARCCORE_HOST_DEVICE bool
  operator==(const Vector3<T>& v1, const Vector3<T>& v2)
  {
    return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z;
  }

  friend constexpr ARCCORE_HOST_DEVICE bool
  operator!=(const Vector3<T>& v1, const Vector3<T>& v2)
  {
    return !(v1 == v2);
  }

 private:

  void _print(std::ostream& o) const
  {
    o << '{' << x << ',' << y << ',' << z << '}';
  }
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

} // namespace Arcane

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#endif