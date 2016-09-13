// Copyright (C) 2009-2016 ETH Zurich
// Copyright (C) 2007?-2016 Center for Nanophase Materials Sciences, ORNL
// All rights reserved.
//
// See LICENSE.txt for terms of usage.
// See CITATION.txt for citation guidelines if you use this code for scientific publications.
//
// Author: Peter Staar (taa@zurich.ibm.com)
//
// Template specialization for CPU.

#ifndef PHYS_LIBRARY_DCA_STEP_CLUSTER_SOLVER_CLUSTER_SOLVER_MC_CTAUX_CTAUX_WALKER_CTAUX_WALKER_TOOLS_CTAUX_G_MATRIX_ROUTINES_CTAUX_G_MATRIX_ROUTINES_CPU_H
#define PHYS_LIBRARY_DCA_STEP_CLUSTER_SOLVER_CLUSTER_SOLVER_MC_CTAUX_CTAUX_WALKER_CTAUX_WALKER_TOOLS_CTAUX_G_MATRIX_ROUTINES_CTAUX_G_MATRIX_ROUTINES_CPU_H

#include "phys_library/DCA+_step/cluster_solver/cluster_solver_mc_ctaux/ctaux_walker/ctaux_walker_tools/ctaux_G_matrix_routines/ctaux_G_matrix_routines_TEM.h"

#include <cassert>

#include "dca/linalg/matrix.hpp"
#include "dca/linalg/vector.hpp"

namespace DCA {
namespace QMCI {
// DCA::QMCI::

template <typename parameters_type>
class G_MATRIX_TOOLS<dca::linalg::CPU, parameters_type> {
  typedef typename parameters_type::concurrency_type concurrency_type;
  typedef typename parameters_type::profiler_type profiler_t;

public:
  G_MATRIX_TOOLS(int id, parameters_type& parameters_ref)
      : thread_id(id),

        parameters(parameters_ref),
        concurrency(parameters.get_concurrency()) {}

  void read_G_matrix_elements(dca::linalg::Vector<int, dca::linalg::CPU>& i_index,
                              dca::linalg::Vector<int, dca::linalg::CPU>& j_index,
                              dca::linalg::Vector<bool, dca::linalg::CPU>& is_Bennett,
                              dca::linalg::Vector<double, dca::linalg::CPU>& exp_Vj,
                              dca::linalg::Matrix<double, dca::linalg::CPU>& N,
                              dca::linalg::Matrix<double, dca::linalg::CPU>& G_precomputed,
                              double* result_ptr, int incr) {
    assert(i_index.size() == j_index.size());
    assert(i_index.size() == is_Bennett.size());
    assert(i_index.size() == exp_Vj.size());

    double result, delta;

    int vertex_index = N.nrCols() - G_precomputed.nrCols();

    for (int l = 0; l < i_index.size(); ++l) {
      result = 0;

      if (j_index[l] < vertex_index) {
        delta = i_index[l] == j_index[l] ? 1 : 0;
        result = (N(i_index[l], j_index[l]) * exp_Vj[l] - delta) / (exp_Vj[l] - 1.);
      }
      else
        result = G_precomputed(i_index[l], j_index[l] - vertex_index);

      result_ptr[l * incr] = is_Bennett[l] ? 0. : result;
    }
  }

  void compute_row_on_Gamma_matrix(int row_index, dca::linalg::Vector<int, dca::linalg::CPU>& indices,
                                   dca::linalg::Vector<double, dca::linalg::CPU>& exp_V,
                                   dca::linalg::Matrix<double, dca::linalg::CPU>& N,
                                   dca::linalg::Matrix<double, dca::linalg::CPU>& G_precomputed,
                                   double* row_ptr, int incr) {
    int i_index, j_index;
    double result, delta;

    int vertex_index = N.nrCols() - G_precomputed.nrCols();

    i_index = indices[row_index];

    for (int l = 0; l < indices.size(); ++l) {
      j_index = indices[l];

      result = 0;

      if (j_index < vertex_index) {
        delta = i_index == j_index ? 1 : 0;
        result = (N(i_index, j_index) * exp_V[l] - delta) / (exp_V[l] - 1.);
      }
      else
        result = G_precomputed(i_index, j_index - vertex_index);

      row_ptr[l * incr] = result;
    }
  }

  void compute_col_on_Gamma_matrix(int col_index, dca::linalg::Vector<int, dca::linalg::CPU>& indices,
                                   dca::linalg::Vector<double, dca::linalg::CPU>& exp_V,
                                   dca::linalg::Matrix<double, dca::linalg::CPU>& N,
                                   dca::linalg::Matrix<double, dca::linalg::CPU>& G_precomputed,
                                   double* col_ptr, int incr) {
    int i_index, j_index;
    double result, delta, exp_Vj;

    int vertex_index = N.nrCols() - G_precomputed.nrCols();

    j_index = indices[col_index];
    exp_Vj = exp_V[col_index];

    for (int l = 0; l < indices.size(); ++l) {
      i_index = indices[l];

      result = 0;

      if (j_index < vertex_index) {
        delta = i_index == j_index ? 1 : 0;
        result = (N(i_index, j_index) * exp_Vj - delta) / (exp_Vj - 1.);
      }
      else
        result = G_precomputed(i_index, j_index - vertex_index);

      col_ptr[l * incr] = result;
    }
  }

private:
  int thread_id;

  parameters_type& parameters;
  concurrency_type& concurrency;
};

}  // QMCI
}  // DCA

#endif  // PHYS_LIBRARY_DCA_STEP_CLUSTER_SOLVER_CLUSTER_SOLVER_MC_CTAUX_CTAUX_WALKER_CTAUX_WALKER_TOOLS_CTAUX_G_MATRIX_ROUTINES_CTAUX_G_MATRIX_ROUTINES_CPU_H
