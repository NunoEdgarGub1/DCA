//-*-C++-*-

#ifndef ELECTRON_BAND_DOMAIN_H
#define ELECTRON_BAND_DOMAIN_H

/*!
 *  \author Peter Staar
 */
struct band_element
{
  band_element();
  ~band_element();

  int                 number;
  int                 flavor;
  std::vector<double> a_vec;

//   template<class stream_type>
//   void to_JSON(stream_type& ss);
};

band_element::band_element():
  number(-1),
  flavor(-1),
  a_vec(0,0)
{}

band_element::~band_element()
{}

/*
template<class stream_type>
void band_element::to_JSON(stream_type& ss)
{
  ss << "\n { \n";
  //ss << "\"number\" : {\n";
  ss << "\"number\" : " << number << ",\n";
  ss << "\"flavor\" : " << flavor << ",\n";

  ss << "\"a_vec\"  : [ ";
  for(size_t i=0; i<a_vec.size(); ++i){
    if(i == a_vec.size()-1)
      ss << a_vec[i] << "]";
    else
      ss << a_vec[i] << ", ";
  }

  ss << "\n}";
}
*/

/*!
 *  \author Peter Staar
 */
class electron_band_domain 
{
public:

  //typedef int element_type;
  typedef band_element element_type;

public:

  static int&                       get_size();
  static std::string                get_name();

  static std::vector<element_type>& get_elements();

  template<IO::FORMAT DATA_FORMAT>
  static void read(IO::reader<DATA_FORMAT>& reader);

  template<IO::FORMAT DATA_FORMAT>
  static void write(IO::writer<DATA_FORMAT>& writer);

  template<typename parameters_type>
  static void initialize(parameters_type& parameters, int Nb_bands, std::vector<int> flavors, std::vector<std::vector<double> > a_vecs);
  
  template<class stream_type>
  static void to_JSON(stream_type& ss);

private:

  static std::vector<element_type>& initialize_elements();
};

int& electron_band_domain::get_size()  
{
  static int size = 0;
  return size;
}

std::string electron_band_domain::get_name()  
{
  static std::string name = "electron-band-domain";
  return name;
}

std::vector<electron_band_domain::element_type>& electron_band_domain::get_elements()
{
  static std::vector<element_type> elements(get_size());
  return elements;
}

template<IO::FORMAT DATA_FORMAT>
void electron_band_domain::write(IO::writer<DATA_FORMAT>& writer)
{
  writer.open_group(get_name());
  writer.execute(get_elements());
  writer.close_group();
}

template<typename parameters_type>
void electron_band_domain::initialize(parameters_type& parameters, int NB_BANDS, std::vector<int> flavors, std::vector<std::vector<double> > a_vecs)
{
  get_size() = NB_BANDS;

  assert(NB_BANDS==int(flavors.size()));
  assert(NB_BANDS==int(a_vecs .size()));

  for(size_t i=0; i<get_elements().size(); ++i){
    
    get_elements()[i].number = i;
    get_elements()[i].flavor = i;//flavors[i];
    get_elements()[i].a_vec  = a_vecs[i];
  }

//   {
//     parameters.is_interacting_band().resize(NB_BANDS, false);

//     for(size_t l=0; l<parameters.get_interacting_bands().size(); l++)
//       parameters.is_interacting_band()[parameters.get_interacting_bands()[l]] = true;
//   }
}

/*
template<class stream_type>
void electron_band_domain::to_JSON(stream_type& ss)
{
  ss << "\"electron_band_domain\" : {";
    
  for(int l=0; l<get_size(); l++){
    
    ss << "\n\"orbital " << l << "\" :";

    get_elements()[l].to_JSON(ss);

    if(l == get_size()-1)
      ss << "\n}\n";
    else
      ss << ", ";
  }
}
*/

#endif