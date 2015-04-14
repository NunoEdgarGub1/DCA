//-*-C++-*-

#ifndef COMPUTE_SECOND_ORDER_SIGMA_H
#define COMPUTE_SECOND_ORDER_SIGMA_H

namespace DCA
{

  namespace SERIES_EXPANSION
  {
    /*!
     * \authors Peter Staar, Urs R. Haehner
     *
     * \brief  This class implements the computation of the self-energy in second order.
     */
    template<class parameters_type, class k_dmn_t>
    class sigma_perturbation<2, parameters_type, k_dmn_t>
    {
#include "type_definitions.h"

      typedef typename parameters_type::profiler_type    profiler_type;
      typedef typename parameters_type::concurrency_type concurrency_type;

    public:

      typedef typename compute_interaction::function_type U_function_type;

      typedef compute_bubble<ph, parameters_type, k_dmn_t, w> ph_bubble_t;
      typedef compute_bubble<ph, parameters_type, k_dmn_t, w> pp_bubble_t;

      typedef typename ph_bubble_t::function_type chi_function_type;
      typedef typename pp_bubble_t::function_type phi_function_type;

      typedef function<std::complex<double>, dmn_4<nu,nu, k_dmn_t, w> > sp_function_type;

    public:

      sigma_perturbation(parameters_type&      parameters_ref,
                         compute_interaction& interaction_obj,
                         compute_bubble<ph, parameters_type, k_dmn_t, w>& chi_obj,
                         compute_bubble<pp, parameters_type, k_dmn_t, w>& phi_obj);

      ~sigma_perturbation();

      sp_function_type& get_function() { return Sigma; }

      void execute_on_cluster         (function<std::complex<double>, dmn_4<nu,nu, k_dmn_t, w> >& G);
      void threaded_execute_on_cluster(function<std::complex<double>, dmn_4<nu,nu, k_dmn_t, w> >& G);

      //void execute_on_cluster_2(function<std::complex<double>, dmn_4<nu,nu, k_dmn_t, w> >& G);

      template<IO::FORMAT DATA_FORMAT>
      void write(IO::writer<DATA_FORMAT>& writer);

    private:

      void execute_2A(function<std::complex<double>, dmn_4<nu,nu, k_dmn_t, w> >& G);
      void execute_2B(function<std::complex<double>, dmn_4<nu,nu, k_dmn_t, w> >& G);

      static void* threaded_execute_2B(void* data);

      int subtract_freq_bf(int, int); // fermion-boson

    protected:

      parameters_type&  parameters;
      concurrency_type& concurrency;

      U_function_type& U;

      chi_function_type& chi;
      phi_function_type& phi;

      function<std::complex<double>, dmn_4<b,b, k_dmn_t, w_VERTEX_BOSONIC> > U_chi_U;

      sp_function_type Sigma;

      sp_function_type Sigma_2A;
      sp_function_type Sigma_2B;

    private:

      struct sigma_perturbation_data
      {
	U_function_type* U_ptr;

        sp_function_type*  G_ptr;
        sp_function_type*  S_ptr;

        chi_function_type* chi_ptr;
        phi_function_type* phi_ptr;

	concurrency_type* concurrency_ptr;
      };

    };

    template<class parameters_type, class k_dmn_t>
    sigma_perturbation<2, parameters_type, k_dmn_t>::sigma_perturbation(parameters_type&      parameters_ref,
                                                                       compute_interaction& interaction_obj,
                                                                       compute_bubble<ph, parameters_type, k_dmn_t, w>& chi_obj,
                                                                       compute_bubble<pp, parameters_type, k_dmn_t, w>& phi_obj):
      parameters(parameters_ref),
      concurrency(parameters.get_concurrency()),

      U(interaction_obj.get_function()),

      chi(chi_obj.get_function()),
      phi(phi_obj.get_function()),

      U_chi_U("U-chi-U"),

      Sigma("Sigma-2nd-order"),
      Sigma_2A("Sigma-2nd-order-A"),
      Sigma_2B("Sigma-2nd-order-B")
    {}

    template<class parameters_type, class k_dmn_t>
    sigma_perturbation<2, parameters_type, k_dmn_t>::~sigma_perturbation()
    {}

    template<class parameters_type, class k_dmn_t>
    template<IO::FORMAT DATA_FORMAT>
    void sigma_perturbation<2, parameters_type, k_dmn_t>::write(IO::writer<DATA_FORMAT>& writer)
    {}

    template<class parameters_type, class k_dmn_t>
    void sigma_perturbation<2, parameters_type, k_dmn_t>::execute_on_cluster(function<std::complex<double>, dmn_4<nu,nu, k_dmn_t, w> >& G)
    {
      if(concurrency.id()==0)
	cout << __FUNCTION__ << endl;

      //cout << "\t U : " << U(0,0,0,1) << endl;

      sigma_perturbation<2, parameters_type, k_dmn_t>::execute_2B(G);

      //Sigma_2A *= 1.;
      //Sigma_2B *= 2.;

      Sigma = 0.;
      //Sigma += Sigma_2A;
      Sigma += Sigma_2B;

      if(true)
        {
          std::complex<double> I(0,1);
          for(int b_ind=0; b_ind<2*b::dmn_size(); ++b_ind){

            for(int k_ind=0; k_ind<k_dmn_t::dmn_size(); ++k_ind){

              int wc_ind = w::dmn_size()/8;

              double wc = w::get_elements()[wc_ind];

              std::complex<double> Sigma_wc = Sigma(b_ind, b_ind, k_ind, wc_ind);

              double alpha = real(Sigma_wc);
              double beta  = imag(Sigma_wc*wc);

              for(int w_ind=0; w_ind<wc_ind; ++w_ind){
                Sigma(b_ind, b_ind, k_ind,                 w_ind) = alpha + beta*I/w::get_elements()[w_ind];
                Sigma(b_ind, b_ind, k_ind, w::dmn_size()-1-w_ind) = alpha - beta*I/w::get_elements()[w_ind];
              }
            }
          }
        }
    }

    template<class parameters_type, class k_dmn_t>
    void sigma_perturbation<2, parameters_type, k_dmn_t>::execute_2A(function<std::complex<double>, dmn_4<nu,nu, k_dmn_t, w> >& G)
    {
      if(concurrency.id()==0)
	cout << __FUNCTION__ << endl;

      double U_value = U(0,0,0,1);

      Sigma_2A = 0.;

      for(int nu1_ind=0; nu1_ind<w_VERTEX_BOSONIC::dmn_size(); ++nu1_ind){
        for(int q1_ind=0; q1_ind<k_dmn_t::dmn_size(); ++q1_ind){

          for(int nu2_ind=0; nu2_ind<w_VERTEX_BOSONIC::dmn_size(); ++nu2_ind){
            for(int q2_ind=0; q2_ind<k_dmn_t::dmn_size(); ++q2_ind){

              for(int w_ind=0; w_ind<w::dmn_size(); ++w_ind){

                int w_minus_nu1 = subtract_freq_bf(nu1_ind, w_ind);
                int w_minus_nu2 = subtract_freq_bf(nu2_ind, w_ind);
                int w_minus_nu1_minus_nu2 = subtract_freq_bf(nu2_ind, w_minus_nu1);

                if(w_minus_nu1 < 0 || w_minus_nu1 >= w::dmn_size() || w_minus_nu2 < 0 || w_minus_nu2 >= w::dmn_size()
                   || w_minus_nu1_minus_nu2 < 0 || w_minus_nu1_minus_nu2 >= w::dmn_size())
                  continue;

                for(int k_ind=0; k_ind<k_dmn_t::dmn_size(); ++k_ind){
                  int k_minus_q1 = k_dmn_t::parameters_type::subtract(q1_ind, k_ind);
                  int k_minus_q2 = k_dmn_t::parameters_type::subtract(q2_ind, k_ind);
                  int k_minus_q1_minus_q2 = k_dmn_t::parameters_type::subtract(q2_ind, k_minus_q1);

                  Sigma_2A(0,0, 0,0, k_ind, w_ind) += G(0,0, 0,0, k_minus_q1, w_minus_nu1)
                    * G(0,0, 0,0, k_minus_q2, w_minus_nu2) * G(0,0, 0,0, k_minus_q1_minus_q2, w_minus_nu1_minus_nu2);

                }
              }
            }
          }
        }
      }

      for(int w_ind=0; w_ind<w::dmn_size(); ++w_ind)
        for(int k_ind=0; k_ind<k_dmn_t::dmn_size(); ++k_ind)
          Sigma_2A(0,1, 0,1, k_ind, w_ind) = Sigma_2A(0,0, 0,0, k_ind, w_ind);

      double factor = 1./(parameters.get_beta()*parameters.get_beta()*k_dmn_t::dmn_size()*k_dmn_t::dmn_size())*U_value*U_value;
      Sigma_2A *= factor;
    }

    template<class parameters_type, class k_dmn_t>
    void sigma_perturbation<2, parameters_type, k_dmn_t>::execute_2B(function<std::complex<double>, dmn_4<nu,nu,k_dmn_t,w> >& G)
    {
      if(concurrency.id()==0)
	cout << __FUNCTION__ << endl;

      double U_value = U(0,0,0,1);

      Sigma_2B = 0.;

      for(int q_ind=0; q_ind<k_dmn_t::dmn_size(); ++q_ind){
        for(int k_ind=0; k_ind<k_dmn_t::dmn_size(); ++k_ind){

          int k_minus_q  = k_dmn_t::parameters_type::subtract(q_ind, k_ind);

          for(int nu_ind=0; nu_ind<w_VERTEX_BOSONIC::dmn_size(); ++nu_ind){

            int nu_c = (nu_ind-w_VERTEX_BOSONIC::dmn_size()/2);

            for(int w_ind=fabs(nu_c); w_ind<w::dmn_size()-fabs(nu_c); ++w_ind){

              int w_minus_nu = w_ind-nu_c;

              Sigma_2B(0,0, 0,0, k_ind, w_ind) += G(0,0, 0,0, k_minus_q, w_minus_nu)*chi(0,0, 0,0, q_ind, nu_ind);
            }
          }
        }
      }

      for(int w_ind=0; w_ind<w::dmn_size(); ++w_ind)
        for(int k_ind=0; k_ind<k_dmn_t::dmn_size(); ++k_ind)
          Sigma_2B(0,1, 0,1, k_ind, w_ind) = Sigma_2B(0,0, 0,0, k_ind, w_ind);

      double factor = 1./(parameters.get_beta()*k_dmn_t::dmn_size())*U_value*U_value;
      Sigma_2B *= factor;
    }

    template<class parameters_type, class k_dmn_t>
    void sigma_perturbation<2, parameters_type, k_dmn_t>::threaded_execute_on_cluster(function<std::complex<double>, dmn_4<nu,nu, k_dmn_t, w> >& G)
    {
      if(concurrency.id()==0)
	cout << "\n\n\t\t second-order Self-energy \n\n" << endl;

//       cout << "\t U : " << U(0,0,0,1) << endl;

      int nr_threads = parameters.get_nr_HTS_threads();

      sigma_perturbation_data args;
      {
        args.U_ptr = &U;

        args.G_ptr = &G;
        args.S_ptr = &Sigma_2B;

        args.chi_ptr = &chi;
        args.phi_ptr = &phi;

	args.concurrency_ptr = &concurrency;
      }

      {
	COMP_LIB::parallelization<COMP_LIB::POSIX_LIBRARY> pthreads;

	pthreads.execute(nr_threads, threaded_execute_2B, (void*) &args);
      }

      {
	double U_value = U(0,0,0,1);      
	double factor  = 1./(parameters.get_beta()*k_dmn_t::dmn_size())*U_value*U_value;
	Sigma_2B *= factor;

	concurrency.sum(Sigma_2B);
      }

      Sigma = 0.;
      Sigma += Sigma_2B;

      if(true)
        {
          std::complex<double> I(0,1);
          for(int b_ind=0; b_ind<2*b::dmn_size(); ++b_ind){

            for(int k_ind=0; k_ind<k_dmn_t::dmn_size(); ++k_ind){

              int wc_ind = w::dmn_size()/8;

              double wc = w::get_elements()[wc_ind];

              std::complex<double> Sigma_wc = Sigma(b_ind, b_ind, k_ind, wc_ind);

              double alpha = real(Sigma_wc);
              double beta  = imag(Sigma_wc*wc);

              for(int w_ind=0; w_ind<wc_ind; ++w_ind){
                Sigma(b_ind, b_ind, k_ind,                 w_ind) = alpha + beta*I/w::get_elements()[w_ind];
                Sigma(b_ind, b_ind, k_ind, w::dmn_size()-1-w_ind) = alpha - beta*I/w::get_elements()[w_ind];
              }
            }
          }
        }
    }

    template<class parameters_type, class k_dmn_t>
    void* sigma_perturbation<2, parameters_type, k_dmn_t>::threaded_execute_2B(void* void_ptr)
    {
      COMP_LIB::posix_data*    data_ptr       = static_cast<COMP_LIB::posix_data   *>(void_ptr);
      sigma_perturbation_data* sigma_pert_ptr = static_cast<sigma_perturbation_data*>(data_ptr->args);

      //U_function_type&   U   = *(sigma_pert_ptr->U_ptr);

      sp_function_type&  G   = *(sigma_pert_ptr->G_ptr);
      sp_function_type&  S   = *(sigma_pert_ptr->S_ptr);
      chi_function_type& chi = *(sigma_pert_ptr->chi_ptr);

      concurrency_type& concurrency = *(sigma_pert_ptr->concurrency_ptr);

      k_dmn_t q_dmn;
      std::pair<int, int> q_bounds = concurrency.get_bounds(q_dmn);

      int id         = data_ptr->id;
      int nr_threads = data_ptr->nr_threads;

      k_dmn_t k_dmn;
      std::pair<int, int> k_bounds = COMP_LIB::parallelization<COMP_LIB::POSIX_LIBRARY>::get_bounds(id, nr_threads, k_dmn);
      
      for(int k_ind=k_bounds.first; k_ind<k_bounds.second; k_ind++)
        {
	  double percentage = double(k_ind-k_bounds.first)/double(k_bounds.second-k_bounds.first);

          if(concurrency.id()==0 and id==0 and ( int(100*percentage) % 10==0 ) )
            cout << "\t" << int(100*percentage) << " % finished\t" << print_time() << "\n";
	  
	  for(int q_ind=q_bounds.first; q_ind<q_bounds.second; q_ind++){
	    
	    int k_minus_q  = k_dmn_t::parameter_type::subtract(q_ind, k_ind);
	    
	    for(int nu_ind=0; nu_ind<w_VERTEX_BOSONIC::dmn_size(); ++nu_ind){
	      
	      int nu_c = (nu_ind-w_VERTEX_BOSONIC::dmn_size()/2);
	      
	      for(int w_ind=fabs(nu_c); w_ind<w::dmn_size()-fabs(nu_c); ++w_ind){
		
		int w_minus_nu = w_ind-nu_c;
		
		S(0,0, 0,0, k_ind, w_ind) += G(0,0, 0,0, k_minus_q, w_minus_nu)*chi(0,0, 0,0, q_ind, nu_ind);//*factor;		
	      }
	    }
	  }
	  
	  for(int w_ind=0; w_ind<w::dmn_size(); ++w_ind)
	    S(0,1, 0,1, k_ind, w_ind) = S(0,0, 0,0, k_ind, w_ind);
	}
      
//       for(int w_ind=0; w_ind<w::dmn_size(); ++w_ind)
// 	for(int k_ind=0; k_ind<k_dmn_t::dmn_size(); ++k_ind)
// 	  S(0,1, 0,1, k_ind, w_ind) = S(0,0, 0,0, k_ind, w_ind);
      
      
//       Sigma_2B *= factor;

      return 0;
    }

    template<class parameters_type, class k_dmn_t>
    int sigma_perturbation<2, parameters_type, k_dmn_t>::subtract_freq_bf(int w1, int w2)
    {
      int w_f = 2*(w2 - w::dmn_size()/2) + 1;             // transform fermionic
      int w_b = 2*(w1 - w_VERTEX_BOSONIC::dmn_size()/2);  // transform bosonic
      int res = ((w_f-w_b) - 1 + w::dmn_size()) / 2;      // result is fermionic
      return res;
    }
    
  }
  
}

#endif