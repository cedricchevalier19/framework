#ifndef ALIEN_HPDDMIMPL_HPDDMBACKEND_H
#define ALIEN_HPDDMIMPL_HPDDMBACKEND_H
/* Author : mesriy at Tue Jul 24 15:56:45 2012
 * Generated by createNew
 */

#include <alien/utils/Precomp.h>
#include <alien/core/backend/BackEnd.h>
#include <arccore/message_passing/IMessagePassingMng.h>

/*---------------------------------------------------------------------------*/

class IOptionsHPDDMSolver;

namespace Alien {

/*---------------------------------------------------------------------------*/

class MultiVectorImpl;
class MatrixData;
class MatrixExp;
class VectorData;
class VectorData;
class VectorExp;
class ILinearSolver;
class ILinearAlgebra;

template <typename T> class SimpleCSRMatrix;
template <typename T> class SimpleCSRVector;
// template<typename T> class HPDDMMatrix ;

class Space;

template <class Matrix, class Vector> class IInternalLinearAlgebra;
template <class Matrix, class Vector> class IInternalLinearSolver;

extern IInternalLinearAlgebra<SimpleCSRMatrix<Real>, SimpleCSRVector<Real>>*
HPDDMSolverInternalLinearAlgebraFactory();

// extern IInternalLinearSolver<SimpleCSRMatrix<Real>, SimpleCSRVector<Real>>*
extern ILinearSolver* HPDDMInternalLinearSolverFactory(
    IMessagePassingMng* p_mng, IOptionsHPDDMSolver* options);

// extern IInternalLinearAlgebra<HPDDMMatrix<Real>, SimpleCSRVector<Real>>*
// HPDDMInternalLinearAlgebraFactory();

/*---------------------------------------------------------------------------*/

namespace BackEnd {
  namespace tag {
    struct hpddm
    {
    };
  }
}

template <> struct AlgebraTraits<BackEnd::tag::hpddm>
{
  typedef SimpleCSRMatrix<Real> matrix_type;
  typedef SimpleCSRVector<Real> vector_type;
  typedef IInternalLinearAlgebra<matrix_type, vector_type> algebra_type;
  // typedef IInternalLinearSolver<matrix_type, vector_type>  solver_type;
  typedef ILinearSolver solver_type;
  typedef IOptionsHPDDMSolver options_type;

  static algebra_type* algebra_factory(IMessagePassingMng* p_mng = nullptr)
  {
    return HPDDMSolverInternalLinearAlgebraFactory();
  }

  static solver_type* solver_factory(IMessagePassingMng* p_mng, options_type* options)
  {
    return HPDDMInternalLinearSolverFactory(p_mng, options);
  }

  static BackEndId name() { return "hpddm"; }
};

/*---------------------------------------------------------------------------*/

} // namespace Alien

/*---------------------------------------------------------------------------*/

#endif /* ALIEN_HPDDMIMPL_HPDDMBACKEND_H */
