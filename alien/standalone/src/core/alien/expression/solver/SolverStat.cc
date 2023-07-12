/*
 * Copyright 2020 IFPEN-CEA
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/* Author : havep at Wed Sep  5 15:48:15 2012
 * Generated by createNew
 */

#include "SolverStat.h"

#include <iomanip>

#include <arccore/trace/TraceMessage.h>

/*---------------------------------------------------------------------------*/

namespace Alien
{

using namespace Arccore;

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

class SolverStat::InternalTraceSizer : public Arccore::ITraceMessageListener
{
 public:
  InternalTraceSizer(Integer& m)
  : m_size(m)
  {}

 public:
  //! Implement ITraceMessageListener
  bool visitMessage(const Arccore::TraceMessageListenerArgs& args) override;

 private:
  Integer& m_size;
};

/*---------------------------------------------------------------------------*/

bool SolverStat::InternalTraceSizer::visitMessage(
const Arccore::TraceMessageListenerArgs& args)
{
  // const TraceMessage * message = args.message();
  ConstArrayView<char> input = args.buffer();
  m_size = input.size();
  return true; // Le ITraceMessageMng won't write this message when leaving
  // ITraceMessageListener
}
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

SolverStat::SolverStat()
: m_solve_count(0)
, m_iteration_count(0)
, m_last_iteration_count(0)
, m_initialization_time(0)
, m_initialization_cpu_time(0)
, m_prepare_time(0)
, m_prepare_cpu_time(0)
, m_last_prepare_time(0)
, m_last_prepare_cpu_time(0)
, m_solve_time(0)
, m_solve_cpu_time(0)
, m_last_solve_time(0)
, m_last_solve_cpu_time(0)
{}

/*---------------------------------------------------------------------------*/
void SolverStat::reset()
{
  m_solve_count = 0;
  m_iteration_count = 0;
  m_last_iteration_count = 0;
  m_initialization_time = m_initialization_cpu_time = 0;
  m_prepare_time = m_prepare_cpu_time = 0;
  m_last_prepare_time = m_last_prepare_cpu_time = 0;
  m_solve_time = m_solve_cpu_time = 0;
  m_last_solve_time = m_last_solve_cpu_time = 0;
}

Integer
SolverStat::solveCount() const
{
  return m_solve_count;
}

/*---------------------------------------------------------------------------*/

Integer
SolverStat::iterationCount() const
{
  return m_iteration_count;
}

/*---------------------------------------------------------------------------*/

Real SolverStat::initializationTime() const
{
  return m_initialization_time;
}

/*---------------------------------------------------------------------------*/

Real SolverStat::initializationCpuTime() const
{
  return m_initialization_cpu_time;
}

/*---------------------------------------------------------------------------*/

Real SolverStat::prepareTime() const
{
  return m_prepare_time;
}

/*---------------------------------------------------------------------------*/

Real SolverStat::prepareCpuTime() const
{
  return m_prepare_cpu_time;
}

/*---------------------------------------------------------------------------*/

Real SolverStat::solveTime() const
{
  return m_solve_time;
}

/*---------------------------------------------------------------------------*/

Real SolverStat::solveCpuTime() const
{
  return m_solve_cpu_time;
}

/*---------------------------------------------------------------------------*/

Integer
SolverStat::lastIterationCount() const
{
  return m_last_iteration_count;
}

/*---------------------------------------------------------------------------*/

Real SolverStat::lastPrepareTime() const
{
  return m_last_prepare_time;
}

/*---------------------------------------------------------------------------*/

Real SolverStat::lastPrepareCpuTime() const
{
  return m_last_prepare_cpu_time;
}

/*---------------------------------------------------------------------------*/

Real SolverStat::lastSolveTime() const
{
  return m_last_solve_time;
}

/*---------------------------------------------------------------------------*/

Real SolverStat::lastSolveCpuTime() const
{
  return m_last_solve_cpu_time;
}

/*---------------------------------------------------------------------------*/

void SolverStat::print(
ITraceMng* traceMng, const Alien::SolverStatus& status, String title) const
{
  Integer prefix_size = 0;
  InternalTraceSizer sizer(prefix_size);
  if (!traceMng)
    return;
  traceMng->addListener(&sizer);
  traceMng->info() << " ";
  traceMng->removeListener(&sizer);
  if (prefix_size > 0)
    --prefix_size; // remove whitespace

  std::ostringstream oss;
  _internalPrint(oss, prefix_size, status, title);
  traceMng->info() << oss.str();
}

/*---------------------------------------------------------------------------*/

void SolverStat::_internalPrint(std::ostream& o, const Integer prefix_size,
                                const Alien::SolverStatus& status, String title = String()) const
{
  char* prefix = NULL;
  if (prefix_size == 0) {
    prefix = new char[2];
    prefix[0] = '\n';
    prefix[1] = 0;
  }
  else {
    prefix = new char[prefix_size + 1];
    prefix[0] = '\n';
    for (Integer i = 1; i < prefix_size; ++i)
      prefix[i] = ' ';
    prefix[prefix_size] = 0;
  }

  auto ss = o.precision();
  o << "|--------------------------------------------------------|";
  if (!title.null()) {
    o << prefix << "| " << title << prefix
      << "|--------------------------------------------------------|";
  }
  o << prefix << "| Last convergence      : " << std::boolalpha << status.succeeded
    << prefix << "| Last iteration number : " << status.iteration_count << prefix
    << "| Last relative residual: " << status.residual << std::setprecision(5) << prefix
    << "| Last prepare time     : wct:" << std::setw(10) << lastPrepareTime()
    << " ; cpu:" << std::setw(10) << lastPrepareCpuTime() << prefix
    << "| Last solve time       : wct:" << std::setw(10) << lastSolveTime()
    << " ; cpu:" << std::setw(10) << lastSolveCpuTime() << prefix
    << "| Initialization time   : wct:" << std::setw(10) << initializationTime()
    << " ; cpu:" << std::setw(10) << initializationCpuTime() << prefix
    << "| Total prepare time    : wct:" << std::setw(10) << prepareTime()
    << " ; cpu:" << std::setw(10) << prepareCpuTime() << prefix
    << "| Total solve time      : wct:" << std::setw(10) << solveTime()
    << " ; cpu:" << std::setw(10) << solveCpuTime() << prefix
    << "| Total iterations      : " << iterationCount() << prefix
    << "| Solve count           : " << solveCount() << prefix
    << "|--------------------------------------------------------|";
  o.precision(ss);
  delete[] prefix;
}

/*---------------------------------------------------------------------------*/

} // namespace Alien

/*---------------------------------------------------------------------------*/
