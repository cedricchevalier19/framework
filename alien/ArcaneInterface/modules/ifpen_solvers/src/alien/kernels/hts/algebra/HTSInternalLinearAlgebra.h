// -*- C++ -*-
#ifndef ALIEN_KERNELS_HTS_ALGEBRA_HTSINTERNALLINEARALGEBRA_H
#define ALIEN_KERNELS_HTS_ALGEBRA_HTSINTERNALLINEARALGEBRA_H
/* Author : mesriy at Tue Jul 24 15:56:45 2012
 * Generated by createNew
 */

#include <alien/utils/Precomp.h>

#include <alien/kernels/hts/HTSBackEnd.h>
#include <alien/core/backend/IInternalLinearAlgebraT.h>

#include <alien/expression/solver/ILinearAlgebra.h>
#include <alien/kernels/simple_csr/algebra/SimpleCSRInternalLinearAlgebra.h>
#include <alien/AlienIFPENSolversPrecomp.h>

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

namespace Alien {

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

typedef SimpleCSRInternalLinearAlgebra HTSSolverInternalLinearAlgebra;

typedef AlgebraTraits<BackEnd::tag::hts>::matrix_type HTSCSRMatrix;
typedef AlgebraTraits<BackEnd::tag::simplecsr>::vector_type CSRVector;

class ALIEN_IFPEN_SOLVERS_EXPORT HTSInternalLinearAlgebra
    : public IInternalLinearAlgebra<HTSCSRMatrix, CSRVector>
{
 public:
  HTSInternalLinearAlgebra();
  virtual ~HTSInternalLinearAlgebra();

  // IInternalLinearAlgebra interface.
  Real norm0(const Vector& x) const;
  Real norm1(const Vector& x) const;
  Real norm2(const Vector& x) const;
  void mult(const Matrix& a, const Vector& x, Vector& r) const;
  void axpy(Real alpha, const Vector& x, Vector& r) const;
  void aypx(Real alpha, Vector& y, const Vector& x) const;
  void copy(const Vector& x, Vector& r) const;
  Real dot(const Vector& x, const Vector& y) const;
  void scal(Real alpha, Vector& x) const;
  void diagonal(const Matrix& a, Vector& x) const;
  void reciprocal(Vector& x) const;
  void pointwiseMult(const Vector& x, const Vector& y, Vector& w) const;

  void mult(const Matrix& a, const UniqueArray<Real>& x, UniqueArray<Real>& r) const;
  void axpy(Real alpha, const UniqueArray<Real>& x, UniqueArray<Real>& r) const;
  void aypx(Real alpha, UniqueArray<Real>& y, const UniqueArray<Real>& x) const;
  void copy(const UniqueArray<Real>& x, UniqueArray<Real>& r) const;
  Real dot(
      Integer local_size, const UniqueArray<Real>& x, const UniqueArray<Real>& y) const;
  void scal(Real alpha, UniqueArray<Real>& x) const;
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

} // namespace Alien

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#endif /* ALIEN_HTSIMPL_HTSLINEARALGEBRA_H */
