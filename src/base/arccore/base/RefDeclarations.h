﻿// -*- tab-width: 2; indent-tabs-mode: nil; coding: utf-8-with-signature -*-
/*---------------------------------------------------------------------------*/
/* RefDeclarations.h                                           (C) 2000-2020 */
/*                                                                           */
/* Déclarations liées à la gestion des références sur une instance.          */
/*---------------------------------------------------------------------------*/
#ifndef ARCCORE_BASE_REFDECLARATIONS_H
#define ARCCORE_BASE_REFDECLARATIONS_H
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#include "arccore/base/ArccoreGlobal.h"

#include <type_traits>
#include <memory>

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

namespace Arccore
{

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Structure servant à tagger les interfaces/classes qui utilisent
 * un compteur de référence interne.
 *
 * Ce tag s'utilise via un typedef comme suit:
 *
 * \code
 * class MyClass
 * {
 *   public:
 *    typedef ReferenceCounterTag ReferenceCounterTagType;
 *   public:
 *    void addReference();
 *    void removeReference();
 * };
 * \endcode
 */
struct ReferenceCounterTag {};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

constexpr int REF_TAG_SHARED_PTR = 0;
constexpr int REF_TAG_REFERENCE_COUNTER = 1;

/*!
 * \brief Caractéristiques pour gérer les compteurs de référence.
 *
 * Par défaut, on utilise comme implémentation la classe std::shared_ptr.
 */
template<typename InstanceType,class T = void>
struct RefTraits
{
  static constexpr int TagId = REF_TAG_SHARED_PTR;
};

/*!
 * \brief Spécialisation de la classe gérant un compteur de référence
 * si la classe utilise le tag 'ReferenceCounterTag'.
 *
 * Dans ce cas, on utilise 'ReferenceCounter' comme implémentation.
 */
template<typename InstanceType>
struct RefTraits<InstanceType,std::enable_if_t<std::is_same_v<typename InstanceType::ReferenceCounterTagType,ReferenceCounterTag>>>
{
  static constexpr int TagId = REF_TAG_REFERENCE_COUNTER;
};

template<typename InstanceType,
         int ImplTagId = RefTraits<InstanceType>::TagId>
class Ref;

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

template<typename InstanceType,int TagType>
struct RefTraitsTagId;

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

} // End namespace Arccore

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#endif  

