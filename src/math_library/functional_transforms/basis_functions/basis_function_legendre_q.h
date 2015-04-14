//-*-C++-*-

#ifndef BASIS_FUNCTION_LEGENDRE_Q_H
#define BASIS_FUNCTION_LEGENDRE_Q_H

namespace TRAFOR
{
  template<typename lh_dmn_type, typename rh_dmn_type>
  class basis_function<lh_dmn_type, rh_dmn_type, LEGENDRE_Q>
  {
  public:

    typedef typename lh_dmn_type::dmn_specifications_type lh_spec_dmn_type;
    typedef typename rh_dmn_type::dmn_specifications_type rh_spec_dmn_type;

    typedef typename lh_spec_dmn_type::scalar_type lh_scalar_type;
    typedef typename rh_spec_dmn_type::scalar_type rh_scalar_type;

    typedef typename lh_spec_dmn_type::element_type lh_element_type;
    typedef typename rh_spec_dmn_type::element_type rh_element_type;
    
    typedef lh_scalar_type f_scalar_type;

  public:

    static f_scalar_type execute(int i, int j)
    {
      return execute(lh_dmn_type::get_elements()[i], rh_dmn_type::get_elements()[j]);
    }

    static f_scalar_type execute(lh_element_type& lh_elem, rh_element_type& rh_elem)
    {
      lh_scalar_type delta = get_delta();
      lh_scalar_type min   = lh_dmn_type::get_elements()[0];

      return compute(2*(lh_elem-min)/delta-1, rh_elem);
    }

  private:

    inline static lh_scalar_type get_delta()
    {
      lh_scalar_type result = 0;

      switch(lh_spec_dmn_type::boundary_condition)
 	{
 	case INTERVAL:
 	  result = (lh_dmn_type::get_elements()[1]-lh_dmn_type::get_elements()[0])*(lh_dmn_type::get_size()-1);
 	  break;

	case PERIODIC:
	case ANTIPERIODIC:
	  result = (lh_dmn_type::get_elements()[1]-lh_dmn_type::get_elements()[0])*(lh_dmn_type::get_size()+0);
	  break;

	default:
	  throw std::logic_error(__FUNCTION__);
	}

      return result;
    }

    inline static f_scalar_type compute(lh_scalar_type t, int l)
    {
      assert(t>-1.-1.e-6 and t<1.+1.e-6);

      f_scalar_type  f_t = 0;
      lh_scalar_type l_v = l;

      switch(l)
	{
	case 0:
	  f_t = 0.5*std::log((1+x)/(1-x));
	  break;
	  
	case 1:
	  f_t = x/2.*std::log((1+x)/(1-x))-1;
	  break;
	  
	default:
	  f_t += ((2*l_v-1)*t*compute(t, l-1) - (l_v-1)*compute(t, l-2))/l_v;
	}

      return f_t;
    }    
  };

}

#endif