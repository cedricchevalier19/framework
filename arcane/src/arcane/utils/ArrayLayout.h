﻿// -*- tab-width: 2; indent-tabs-mode: nil; coding: utf-8-with-signature -*-
//-----------------------------------------------------------------------------
// Copyright 2000-2022 CEA (www.cea.fr) IFPEN (www.ifpenergiesnouvelles.com)
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: Apache-2.0
//-----------------------------------------------------------------------------
/*---------------------------------------------------------------------------*/
/* ArrayLayout.h                                               (C) 2000-2022 */
/*                                                                           */
/* Gestion de la disposition mémoire pour les tableaux N-dimensions.         */
/*---------------------------------------------------------------------------*/
#ifndef ARCANE_UTILS_ARRAYLAYOUT_H
#define ARCANE_UTILS_ARRAYLAYOUT_H
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#include "arcane/utils/ArrayBoundsIndex.h"
#include "arcane/utils/ArrayExtentsValue.h"
#include "arcane/utils/MDDim.h"

#include <array>

/*
 * ATTENTION:
 *
 * Toutes les classes de ce fichier sont expérimentales et l'API n'est pas
 * figée. A NE PAS UTILISER EN DEHORS DE ARCANE.
 */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

namespace Arcane
{

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

template<Int32 I,Int32 J>
class ArrayLayout2
{
 public:

  static constexpr Int64 LastExtent = J;

  static ARCCORE_HOST_DEVICE constexpr Int64
  offset(ArrayBoundsIndex<2> idx,Int64 extent1)
  {
    return (extent1 * idx[I]) + Int64(idx[J]);
  }

  static constexpr std::array<Int32,2> layoutInfo() { return { I, J }; }
  static constexpr ARCCORE_HOST_DEVICE Int32 layout0() { return I; }
  static constexpr ARCCORE_HOST_DEVICE Int32 layout1() { return J; }
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

template<Int32 I,Int32 J,Int32 K>
class ArrayLayout3
{
 public:

  static constexpr Int64 LastExtent = K;

  static ARCCORE_HOST_DEVICE constexpr Int64
  offset(ArrayBoundsIndex<3> idx,Int64 extent1,Int64 extent2)
  {
    return (extent2 * idx[I]) + (extent1*idx[J]) + idx.asInt64(K);
  }

  template<typename ExtentType> static ARCCORE_HOST_DEVICE constexpr Int64
  computeOffsetIndexes(const ExtentType& extents)
  {
    return extents.template constLargeExtent<J>() * extents.template constLargeExtent<K>();
  }

  static constexpr std::array<Int32,3> layoutInfo() { return { I, J, K }; }

  static constexpr ARCCORE_HOST_DEVICE Int32 layout0() { return I; }
  static constexpr ARCCORE_HOST_DEVICE Int32 layout1() { return J; }
  static constexpr ARCCORE_HOST_DEVICE Int32 layout2() { return K; }
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
// Layout par défaut pour chaque dimension

template<int N> class RightLayout;
template<int N> class LeftLayout;

template<> class RightLayout<2> : public ArrayLayout2<0,1> {};
template<> class RightLayout<3> : public ArrayLayout3<0,1,2> {};
using RightLayout2 = RightLayout<2>;
using RightLayout3 = RightLayout<3>;

template<> class LeftLayout<2> : public ArrayLayout2<1,0> {};
template<> class LeftLayout<3> : public ArrayLayout3<2,1,0> {};
using LeftLayout2 = LeftLayout<2>;
using LeftLayout3 = LeftLayout<3>;

//! Le layout par défaut est toujours RightLayout
template<> class DefaultLayout<2> : public RightLayout<2> {};
//! Le layout par défaut est toujours RightLayout
template<> class DefaultLayout<3> : public RightLayout<3> {};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

} // End namespace Arcane

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#endif
