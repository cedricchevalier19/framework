// -*- C++ -*-
#ifndef ALIEN_PETSCIMPL_PETSCVECTOR_H
#define ALIEN_PETSCIMPL_PETSCVECTOR_H
/* Author : havep at Wed Jul 18 14:08:21 2012
 * Generated by createNew
 */

#include <alien/kernels/petsc/PETScPrecomp.h>
#include <alien/core/impl/IVectorImpl.h>
#include <alien/data/ISpace.h>
#include <alien/distribution/VectorDistribution.h>

/*---------------------------------------------------------------------------*/

namespace Alien::PETScInternal {

struct VectorInternal;
}

/*---------------------------------------------------------------------------*/

namespace Alien {

/*---------------------------------------------------------------------------*/

class PETScVector : public IVectorImpl
{
 public:
  typedef PETScInternal::VectorInternal VectorInternal;

 public:
  PETScVector(const MultiVectorImpl* multi_impl);

  virtual ~PETScVector();

 public:
  void init(const VectorDistribution& dist, const bool need_allocate,
      Arccore::Integer block_size = 1);
  void allocate();

  void free() {}
  void clear() {}

 private:
  bool setValues(const int nrow, const double* values);

  bool setValues(const int nrow, const int* rows, const double* values);

 public:
  bool setValues(Arccore::ConstArrayView<Arccore::Real> values);

  bool getValues(const int nrow, const int* rows, double* values) const;

  bool getValues(const int nrow, double* values) const;

 public:
  // Méthodes restreintes à usage interne de l'implémentation PETSc
  VectorInternal* internal() { return m_internal.get(); }
  const VectorInternal* internal() const { return m_internal.get(); }

  /*
  void update(const IFPVector & v);
  void update(const MTLVector & v);
  void update(const HypreVector & v);
  void update(const MCGVector & v);
  */
  // void update(const __NewSolverTemplate__Vector & v) { /* YOUR CODE IN .cc */ }

 private:
  bool assemble();

 private:
  std::unique_ptr<VectorInternal> m_internal;
  Arccore::Integer m_block_size = 1;
};

/*---------------------------------------------------------------------------*/

} // namespace Alien

/*---------------------------------------------------------------------------*/

#endif /* ALIEN_PETSCIMPL_PETSCVECTOR_H */