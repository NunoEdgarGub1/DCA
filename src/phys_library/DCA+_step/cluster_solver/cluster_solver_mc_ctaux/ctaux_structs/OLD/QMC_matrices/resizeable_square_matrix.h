//-*-C++-*-

/** \ingroup DCA */

/*@{*/

/*! \file   
 *
 *  Contains a class to represent resizeable square matrices
 */

#ifndef QMC_RESIZEABLE_SQUARE_MATRICES_H
#define QMC_RESIZEABLE_SQUARE_MATRICES_H

namespace QMC {
  
  template<typename scalartype>
  class resizeable_square_matrix
  {
    //#include "type_definitions.h" 

  public:

    typedef scalartype                       matrix_scalartype;
    typedef resizeable_square_matrix<scalartype> this_type;

    resizeable_square_matrix(int                 current_size,
			 int                 global_size);

    ~resizeable_square_matrix();

    scalartype& operator()(int i, int j);

    int&        get_current_size();
    int         get_global_size();


    void resize(int new_global_size);
    void resize_no_copy(int new_current_size);
    void resize_no_copy(int new_current_size, int new_global_size);

    void copy_from(this_type& other_matrix);

    void remove_row_and_column(int i);
    void swap_row_and_column(int i, int j);
    void copy_row_and_column_into(int i, int j);

    bool difference(this_type& mat);
    void print();

  private:

    int                 current_size;
    int                 global_size;

    scalartype*         data;
  };


  template<typename scalartype>
  resizeable_square_matrix<scalartype>::resizeable_square_matrix(int                 crrnt_size,
								 int                 glbl_size):
    current_size(crrnt_size),
    global_size(glbl_size)
  {
    if( current_size > 0.8*double(global_size) )
      global_size = int(1.2*double(current_size));

    data = new scalartype[global_size*global_size];

    memset(data, 0, sizeof(scalartype)*global_size*global_size);
  }



  template<  typename scalartype>
  resizeable_square_matrix<scalartype>::~resizeable_square_matrix()
  {
    delete [] data;
  }

  template<  typename scalartype>
  int& resizeable_square_matrix<scalartype>::get_current_size()
  {
    return current_size;
  }

  template<  typename scalartype>
  int resizeable_square_matrix<scalartype>::get_global_size()
  {
    return global_size;
  }

  template<  typename scalartype>
  scalartype& resizeable_square_matrix<scalartype>::operator()(int i, int j)
  {
    assert(i >= 0 && j >= 0 && i < current_size && j < current_size);
    return data[i + j * global_size];
  }

  template<  typename scalartype>
  void resizeable_square_matrix<scalartype>::resize(int new_current_size)
  {
    if(new_current_size > global_size)
      {
	int new_global_size = int(1.2*double(new_current_size));

	scalartype* new_data = new scalartype[new_global_size*new_global_size];
	memset(new_data, 0, sizeof(scalartype)*new_global_size*new_global_size);
	
	// copy column by column
	for(int i=0; i<current_size; i++)
	  memcpy(&new_data[i*new_global_size], &data[i*global_size], sizeof(scalartype)*current_size);
	
	delete [] data;

	// is this efficient ??? can I not just swap pointers and kill pointer new_data ???
	data = new scalartype[new_global_size*new_global_size];
	memcpy(data, new_data, sizeof(scalartype)*new_global_size*new_global_size);
	
	delete [] new_data;
	
	global_size  = new_global_size;
	current_size = new_current_size;
      }
    else      
      {
	current_size = new_current_size;
      }
    
  }

  template<  typename scalartype>
  void resizeable_square_matrix<scalartype>::resize_no_copy(int new_current_size)
  {
    if(new_current_size > global_size)
      {
	current_size = new_current_size;
	global_size = int(1.2*double(current_size));

	delete [] data;
	data = new scalartype[global_size*global_size];
      }
    else
      {
	current_size = new_current_size;
      }

    memset(data, 0, sizeof(scalartype)*global_size*global_size);
  }

  template<  typename scalartype>
  void resizeable_square_matrix<scalartype>::resize_no_copy(int new_current_size,
									     int new_global_size)
  {
    current_size = new_current_size;
    global_size  = new_global_size;

    delete [] data;
    data = new scalartype[global_size*global_size];
    
    memset(data, 0, sizeof(scalartype)*global_size*global_size);
  }

 
  template<  typename scalartype>
  void resizeable_square_matrix<scalartype>::copy_from(this_type& other_matrix)
  {
    assert(global_size == other_matrix.get_global_size());

    current_size = other_matrix.get_current_size();

    if(current_size > 0)
	memcpy(data, &(other_matrix(0,0)), sizeof(scalartype)*global_size*global_size);
    //else
    //cout << __FUNCTION__ << "\t" << current_size << endl;
  }

 

  template<  typename scalartype>
  void resizeable_square_matrix<scalartype>::remove_row_and_column(int index)
  {
    assert(index>=0);
    assert(index<current_size);

    // copy the remaining columns on the right
    memmove(&data[index*global_size], &data[(index+1)*global_size], sizeof(scalartype)*global_size*(current_size-1-index));
    //memcpy(&data[index*global_size], &data[(index+1)*global_size], sizeof(scalartype)*global_size*(current_size-1-index));

    // copy the remaining rows on the bottom
    for(int i=0; i<current_size-1; i++){
      memmove(&data[index + i*global_size], &data[1 + index + i*global_size], sizeof(scalartype)*(current_size-1-index));
      //memcpy(&data[index + i*global_size], &data[1 + index + i*global_size], sizeof(scalartype)*(current_size-1-index));
    }
    current_size -= 1;
  }

  template<  typename scalartype>
  void resizeable_square_matrix<scalartype>::swap_row_and_column(int i, int j)
  {
    assert(i>=0 && i<current_size && j>=0 && j<current_size);
    
    scalartype tmp_row, tmp_col;
    for(int l=0; l<current_size; l++)
      {
	tmp_row = data[i + l*global_size];
	data[i + l*global_size] = data[j + l*global_size];
	data[j + l*global_size] = tmp_row;
      }

    for(int l=0; l<current_size; l++)
      {
	tmp_col = data[l + i*global_size];
	data[l + i*global_size] = data[l + j*global_size];
	data[l + j*global_size] = tmp_col;
      }
  }

  template<  typename scalartype>
  void resizeable_square_matrix<scalartype>::copy_row_and_column_into(int i, int j)
  {
    assert(i>=0 && i<current_size && j>=0 && j<current_size);
    // j --> i

    // column
    memcpy(&data[i*global_size], &data[j*global_size], sizeof(scalartype)*(current_size));

    // row (?? Givens rotation ??)
    for(int l=0; l<current_size; l++)
      data[i + l*global_size] = data[j + l*global_size];
  }


  template<  typename scalartype>
  bool resizeable_square_matrix<scalartype>::difference(this_type& mat)
  {
    if(current_size != mat.get_current_size())
      throw std::logic_error(__FUNCTION__);

    cout << scientific;
    cout.precision(8);

    bool OK = true ;
    double max = 0;

    int I,J;

    for(int i=0; i<current_size; i++){
      for(int j=0; j<current_size; j++){

	if(max < fabs(mat(i,j)-operator()(i,j)) ){
	  I = i;
	  J = j;
	  max = fabs(mat(i,j)-operator()(i,j));
	}

      }
    }

    //if(current_size > 0)
    //max = max/fabs(mat(I,J));

    //cout << "\t\t Max Diff : " << max << endl;

    if(max > 1.e-12){
      cout << "\t\t Max Diff : " << max << endl;
    }
    else
      cout << "\t\t\t\tOK\n";

    /*if(max > 1.e-6){
      OK = false;
      //throw std::logic_error(__FUNCTION__);
    }
    
    if(!OK)
      {
	for(int i=0; i<current_size; i++){
	  for(int j=0; j<current_size; j++){
	    
	    if(fabs(mat(i,j)-operator()(i,j)) > 1.e-6)
	      cout << "\t" << i<<";" << j;
	    else
	      cout << "\t --- ";
	  }
	  cout << endl;
	}
	cout << endl;
      }
    */

    return OK;
  }

  template<  typename scalartype>
  void resizeable_square_matrix<scalartype>::print()
  {
    
    std::stringstream ss;
    ss << scientific;
    ss.precision(6);
    ss.width(6);
 
    ss << "\n\n";

    /*for(int i=0; i<configuration.size(); i++)
      ss << "\t" << "==============";

    ss << "\n";

    for(int i=0; i<configuration.size(); i++)
      ss << "\t" << configuration[i].get_tau();

    ss << "\n";

    for(int i=0; i<configuration.size(); i++)
      ss << "\t\t" << configuration[i].get_site();

    ss << "\n";

    for(int i=0; i<configuration.size(); i++)
      ss << "\t\t" << configuration[i].get_spin();

    ss << "\n";



    for(int i=0; i<configuration.size(); i++)
    ss << "\t" << "--------------";*/

    ss << "\n";

    for(int i=0; i<current_size; i++){
      for(int j=0; j<current_size; j++){
	ss << "\t" << this->operator()(i,j);
      }
      ss << endl;
    }
    ss << endl;

    /*for(int i=0; i<configuration.size(); i++)
      ss << "\t" << "==============";*/

    ss << endl<<endl;

    cout << ss.str(); 
  }


}

#endif