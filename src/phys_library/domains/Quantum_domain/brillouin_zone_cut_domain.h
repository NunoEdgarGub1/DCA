//-*-C++-*-

#ifndef BRILLOUIN_ZONE_CUT_DOMAIN_H
#define BRILLOUIN_ZONE_CUT_DOMAIN_H

/*!
 *  \author Peter Staar
 */
template<int N=101>
class brillouin_zone_cut_domain 
{
public:

  static const int INTERPOLATION_POINTS = N;

public:

  typedef brillouin_zone_cut_domain this_type;
  typedef std::vector<double>       element_type;

public:

  static int&                       get_size();
  static std::vector<element_type>& get_elements();

//   template<typename parameters_type>
//   static void initialize(parameters_type& parameters);

  template<class stream_type>
  static void to_JSON(stream_type& ss);
};

template<int N>
int& brillouin_zone_cut_domain<N>::get_size()  
{
  static int size = 0;
  return size;
}

template<int N>
std::vector<std::vector<double> >& brillouin_zone_cut_domain<N>::get_elements()
{
  static std::vector<std::vector<double> > v(0);
  return v;
}

template<int N>
template<class stream_type>
void brillouin_zone_cut_domain<N>::to_JSON(stream_type& ss)
{
  ss << "\"brillouin_zone_cut_domain\" : [\n";
    
  for(int i=0; i<get_size(); i++){

    ss << "[ ";
    for(size_t z=0; z<get_elements()[i].size(); z++){
      if(z == get_elements()[i].size()-1)
	ss << get_elements()[i][z] << "]";
      else
	ss << get_elements()[i][z] << ", ";
    }

    if(i == int(get_elements().size())-1)
      ss << "\n";
    else
      ss << ",\n";
  }
  ss << "]\n";
}

#endif