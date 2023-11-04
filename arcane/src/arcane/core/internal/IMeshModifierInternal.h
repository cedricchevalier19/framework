﻿// -*- tab-width: 2; indent-tabs-mode: nil; coding: utf-8-with-signature -*-
//-----------------------------------------------------------------------------
// Copyright 2000-2023 CEA (www.cea.fr) IFPEN (www.ifpenergiesnouvelles.com)
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: Apache-2.0
//-----------------------------------------------------------------------------
/*---------------------------------------------------------------------------*/
/* IMeshModifierInternal.h                                     (C) 2000-2023 */
/*                                                                           */
/* Partie interne à Arcane de IMeshModifier.                                 */
/*---------------------------------------------------------------------------*/
#ifndef ARCANE_CORE_INTERNAL_IMESHMODIFIERINTERNAL_H
#define ARCANE_CORE_INTERNAL_IMESHMODIFIERINTERNAL_H
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#include "arcane/ArcaneTypes.h"

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

namespace Arcane
{

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \internal
 * \brief Partie interne de IMeshModifier.
 */
class ARCANE_CORE_EXPORT IMeshModifierInternal
{
 public:

  virtual ~IMeshModifierInternal() = default;
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

} // namespace Arcane

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#endif