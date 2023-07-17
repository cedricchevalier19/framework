#include "PETScVector.h"
/* Author : havep at Wed Jul 18 14:08:21 2012
 * Generated by createNew
 */

#include <alien/kernels/petsc/PETScBackEnd.h>
#include <alien/kernels/petsc/data_structure/PETScInternal.h>

#include <arccore/message_passing_mpi/MpiMessagePassingMng.h>

/*---------------------------------------------------------------------------*/

namespace Alien {

/*---------------------------------------------------------------------------*/

PETScVector::PETScVector(const MultiVectorImpl* multi_impl)
: IVectorImpl(multi_impl, AlgebraTraits<BackEnd::tag::petsc>::name())
{
  ;
}

/*---------------------------------------------------------------------------*/

PETScVector::~PETScVector()
{}

/*---------------------------------------------------------------------------*/

void
PETScVector::init(const VectorDistribution& dist,
    const bool need_allocate, Arccore::Integer block_size ALIEN_UNUSED_PARAM)
{
  if (need_allocate)
    allocate();
}

/*---------------------------------------------------------------------------*/

void
PETScVector::allocate()
{
  const VectorDistribution& dist = this->distribution();

  Arccore::MessagePassing::Mpi::MpiMessagePassingMng*
  mpi_pm = dynamic_cast<Arccore::MessagePassing::Mpi::MpiMessagePassingMng*>(dist.parallelMng()) ;
  MPI_Comm comm ;

  if(mpi_pm && mpi_pm->getMPIComm())
    comm = *mpi_pm->getMPIComm() ;
  else
    comm = MPI_COMM_NULL ;

  m_internal.reset(new VectorInternal(this->scalarizedLocalSize(), this->scalarizedOffset(),
      this->scalarizedGlobalSize(), dist.isParallel(),comm));
}

/*---------------------------------------------------------------------------*/

bool
PETScVector::setValues(const int nrow, const int* rows, const double* values)
{
  if (m_internal->m_internal == nullptr)
    return false;
  int ierr = VecSetValues(m_internal->m_internal,
      nrow, // nb de valeurs
      rows, values, INSERT_VALUES);
  return (ierr == 0);
}

/*---------------------------------------------------------------------------*/

bool
PETScVector::setValues(const int nrow, const double* values)
{
  if (!m_internal.get())
    return false;

  int* rows = new int[nrow];
  for (int i = 0; i < nrow; ++i)
    rows[i] = m_internal->m_offset + i;
  int ierr = VecSetValues(m_internal->m_internal,
      nrow, // nb de valeurs
      rows, values, INSERT_VALUES);
  delete[] rows;
  return (ierr == 0);
}

/*---------------------------------------------------------------------------*/

bool
PETScVector::setValues(Arccore::ConstArrayView<Arccore::Real> values)
{
  ALIEN_ASSERT((m_internal.get()), ("Not initialized PETScVector before updating"));
  if (not setValues(this->scalarizedLocalSize(), values.unguardedBasePointer()))
    throw Arccore::FatalErrorException(A_FUNCINFO, "Error while setting vetor data");
  if (not assemble())
    throw Arccore::FatalErrorException(A_FUNCINFO, "Error while assembling vector data");
  return true;
}

/*---------------------------------------------------------------------------*/

bool
PETScVector::getValues(const int nrow, const int* rows, double* values) const
{
  if (!m_internal.get())
    return false;

  int ierr = VecGetValues(m_internal->m_internal,
      nrow, // nb de valeurs
      rows, values);
  return (ierr == 0);
}

/*---------------------------------------------------------------------------*/

bool
PETScVector::getValues(const int nrow, double* values) const
{
  if (!m_internal.get())
    return false;

  int* rows = new int[nrow];
  for (int i = 0; i < nrow; ++i)
    rows[i] = m_internal->m_offset + i;
  int ierr = VecGetValues(m_internal->m_internal,
      nrow, // nb de valeurs
      rows, values);
  delete[] rows;
  return (ierr == 0);
}

/*---------------------------------------------------------------------------*/

bool
PETScVector::assemble()
{
  int ierr = 0;
  ierr += VecAssemblyBegin(m_internal->m_internal);
  ierr += VecAssemblyEnd(m_internal->m_internal);
  return (ierr == 0);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*
void
PETScVector::
update(const SimpleCSRVector<double> & v)
{
  ALIEN_ASSERT((m_internal!=NULL),("Not initialized PETScVector before updating"));
  ConstArrayView<Real> values = v.values();
  if (not setValues(m_space->localSize()*m_block_size,values.unguardedBasePointer()))
    throw FatalErrorException(A_FUNCINFO, "Error while setting vetor data");
  if (not assemble())
    throw FatalErrorException(A_FUNCINFO, "Error while assembling vector data");
}
*/
/*---------------------------------------------------------------------------*/
/*
void
PETScVector::
update(const PETScVector & v)
{
  ALIEN_ASSERT((this == &v),("Unexpected error"));
}
*/
/*---------------------------------------------------------------------------*/
/*
void
PETScVector::
update(const IFPVector & v)
{
  UniqueArray<Real> values(m_space->localSize()*m_block_size);
  v.getValues(values.size(),values.unguardedBasePointer()) ;
  if (not setValues(values.size(),values.unguardedBasePointer()))
    throw FatalErrorException(A_FUNCINFO, "Error while setting vetor data");
  if (not assemble())
    throw FatalErrorException(A_FUNCINFO, "Error while assembling vector data");
}
*/
/*---------------------------------------------------------------------------*/
/*
void
PETScVector::
update(const MTLVector & v)
{
  UniqueArray<Real> values(m_space->localSize()*m_block_size);
  v.getValues(values.size(),values.unguardedBasePointer()) ;
  if (not setValues(values.size(),values.unguardedBasePointer()))
    throw FatalErrorException(A_FUNCINFO, "Error while setting vetor data");
  if (not assemble())
    throw FatalErrorException(A_FUNCINFO, "Error while assembling vector data");
}

void
PETScVector::
update(const HypreVector & v)
{
  UniqueArray<Real> values(m_space->localSize()*m_block_size);
  v.getValues(values.size(),values.unguardedBasePointer()) ;
  if (not setValues(values.size(),values.unguardedBasePointer()))
    throw FatalErrorException(A_FUNCINFO, "Error while setting vetor data");
  if (not assemble())
    throw FatalErrorException(A_FUNCINFO, "Error while assembling vector data");
}


void
PETScVector::
update(const MCGVector & v)
{
  UniqueArray<Real> values(m_space->localSize()*m_block_size);
  v.getValues(values.size(),values.unguardedBasePointer()) ;
  if (not setValues(values.size(),values.unguardedBasePointer()))
    throw FatalErrorException(A_FUNCINFO, "Error while setting vetor data");
  if (not assemble())
    throw FatalErrorException(A_FUNCINFO, "Error while assembling vector data");
}
*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

} // namespace Alien

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/