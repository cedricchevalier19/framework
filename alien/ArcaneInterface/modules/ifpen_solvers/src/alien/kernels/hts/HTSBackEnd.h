#ifndef ALIEN_HTSIMPL_HTSBACKEND_H
#define ALIEN_HTSIMPL_HTSBACKEND_H
/* Author : mesriy at Tue Jul 24 15:56:45 2012
 * Generated by createNew
 */

#include <alien/utils/Precomp.h>
#include <alien/core/backend/BackEnd.h>
#include <arccore/message_passing/IMessagePassingMng.h>

/*---------------------------------------------------------------------------*/

class IOptionsHTSSolver;

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
template <typename T> class HTSMatrix;

class Space;

template <class Matrix, class Vector> class IInternalLinearAlgebra;
template <class Matrix, class Vector> class IInternalLinearSolver;

extern IInternalLinearAlgebra<SimpleCSRMatrix<Real>, SimpleCSRVector<Real>>*
HTSSolverInternalLinearAlgebraFactory();

// extern IInternalLinearSolver<SimpleCSRMatrix<Real>, SimpleCSRVector<Real>>*
extern ILinearSolver* HTSInternalLinearSolverFactory(
    Arccore::MessagePassing::IMessagePassingMng* p_mng, IOptionsHTSSolver* options);

extern IInternalLinearAlgebra<HTSMatrix<Real>, SimpleCSRVector<Real>>*
HTSInternalLinearAlgebraFactory();

/*---------------------------------------------------------------------------*/

namespace BackEnd {
  namespace tag {
    struct htssolver
    {
    };
    struct hts
    {
    };
  }
}

template <> struct AlgebraTraits<BackEnd::tag::htssolver>
{
  typedef SimpleCSRMatrix<Real> matrix_type;
  typedef SimpleCSRVector<Real> vector_type;
  typedef IInternalLinearAlgebra<matrix_type, vector_type> algebra_type;
  // typedef IInternalLinearSolver<matrix_type, vector_type>  solver_type;
  typedef ILinearSolver solver_type;
  typedef IOptionsHTSSolver options_type;

  static algebra_type* algebra_factory(
      Arccore::MessagePassing::IMessagePassingMng* p_mng = nullptr)
  {
    return HTSSolverInternalLinearAlgebraFactory();
  }

  static solver_type* solver_factory(
      Arccore::MessagePassing::IMessagePassingMng* p_mng, options_type* options)
  {
    return HTSInternalLinearSolverFactory(p_mng, options);
  }

  static BackEndId name() { return "htssolver"; }
};

template <> struct AlgebraTraits<BackEnd::tag::hts>
{
  typedef HTSMatrix<Real> matrix_type;
  typedef SimpleCSRVector<Real> vector_type;
  typedef IInternalLinearAlgebra<matrix_type, vector_type> algebra_type;

  static algebra_type* algebra_factory(
      Arccore::MessagePassing::IMessagePassingMng* p_mng = nullptr)
  {
    return HTSInternalLinearAlgebraFactory();
  }

  static BackEndId name() { return "hts"; }
};
/*---------------------------------------------------------------------------*/

} // namespace Alien

/*---------------------------------------------------------------------------*/

#endif /* ALIEN_HTSIMPL_HTSBACKEND_H */
