// Copyright (C) 2009-2016 ETH Zurich
// Copyright (C) 2007?-2016 Center for Nanophase Materials Sciences, ORNL
// All rights reserved.
//
// See LICENSE.txt for terms of usage.
// See CITATION.txt for citation guidelines if you use this code for scientific publications.
//
// Author: Giovanni Balduzzi (gbalduzz@itp.phys.ethz.ch)
//
// This file provides a function that computes the local work size given local id and total amount
// of work.

#ifndef DCA_PARALLEL_UTIL_GET_WORKLOAD_HPP
#define DCA_PARALLEL_UTIL_GET_WORKLOAD_HPP

#include <cassert>

namespace dca {
namespace parallel {
namespace util {
// dca::parallel::util::

inline int getWorkload(const uint total_work, const uint n_workers, const uint id) {
  int work = total_work / n_workers;
  if (id < (total_work % n_workers))
    ++work;
  return work;
}

template <class Concurrency>
int getWorkload(const uint total_work, const Concurrency& concurrency) {
  return getWorkload(total_work, concurrency.number_of_processors(), concurrency.id());
}

template <class Concurrency>
int getWorkload(const uint total_work, const uint n_local_workers, const uint local_id,
                const Concurrency& concurrency) {
  assert(local_id < n_local_workers);
  const int local_work = getWorkload(total_work, concurrency);

  return getWorkload(local_work, n_local_workers, local_id);
}

}  // util
}  // parallel
}  // dca

#endif  // DCA_PARALLEL_UTIL_GET_WORKLOAD_HPP
