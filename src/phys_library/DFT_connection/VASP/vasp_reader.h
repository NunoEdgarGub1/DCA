//-*-C++-*-

#ifndef DFT_VASP_READER_H
#define DFT_VASP_READER_H

namespace DFT
{
  namespace VASP
  {

    /*!
     *   \ingroup  PARAMETERS
     *
     *   \author   Peter Staar, Long Zhang
     *   \brief    ...
     */
    template<class parameters_type>
    class reader
    {
#include "type_definitions.h"

    public:

      reader(parameters_type&       parameters,
             data<parameters_type>& data);

      ~reader();

      void execute();

    private:

      parameters_type&       parameters;

      data<parameters_type>& vasp_data;

      function<double, k_vasp>& kx;
      function<double, k_vasp>& ky;
      function<double, k_vasp>& kz;

      function<double, dmn_2<k_vasp, b_vasp> >& band_e;
      function<double, dmn_2<k_vasp, b_vasp> >& band_o;

      function<double, dmn_3<k_vasp, b_vasp, o_vasp> >& proj_magni; // proj_magni[nKpoints][nBands][nOrbi]
      function<double, dmn_3<k_vasp, b_vasp, o_vasp> >& proj_phaRe; // proj_phaRe[nKpoints][nBands][nOrbi]
      function<double, dmn_3<k_vasp, b_vasp, o_vasp> >& proj_phaIm; // proj_phaIm[nKpoints][nBands][nOrbi]

      function<std::complex<double>, dmn_3<k_vasp, b_dmft, o_dmft> >& projector;

      //       function<double, dmn_3<k_vasp, b_dmft, o_dmft> >& projection_re; // projection_Re[nKpoints][nCorrBands][nCorrOrbis]
      //       function<double, dmn_3<k_vasp, b_dmft, o_dmft> >& projection_im; // projection_Im[nKpoints][nCorrBands][nCorrOrbis]

      //       function<double, dmn_3<k_vasp, b_dmft, b_dmft> >& BlochHami; // BlochHami[nKpoints][nCorrBands][nCorrBands]
      //       function<double, dmn_3<k_vasp, b_dmft, b_dmft> >& BlochHami_re;// dfBlochHamiRe  [nKpoints][nCorrOrbis][nCorrOrbis]
      //       function<double, dmn_3<k_vasp, b_dmft, b_dmft> >& BlochHami_im;// dfBlochHamiRe  [nKpoints][nCorrOrbis][nCorrOrbis]

      /*!
       *   extra data for the parsing ...
       */

      string target1;
      string target2;
      string target3;
      string target4;
      string target5;

      string tagk1;
      string tagk2;
      string tagb1;
      string tagb2;

      double s_R, py_R, pz_R, px_R, dxy_R, dyz_R, dz2_R, dxz_R, dx2_R;
      double s_I, py_I, pz_I, px_I, dxy_I, dyz_I, dz2_I, dxz_I, dx2_I;

      double xx, yy;

      int counter1, counter2, counter3, counter4;
      string oneline, dummy, dummy2;

      int pos0, pos1, pos2, pos3;
      int ndp1, ndp2, ndp3;
      int nstar1, nstar2, nstar3;
      std::vector<int> head;
      std::vector<int> tail;
      std::vector<int> pdp;

      //const double PI = 3.14159265358979323846;

      std::vector<std::string> useful_lines;
    };

    template<class parameters_type>
    reader<parameters_type>::reader(parameters_type&       parameters_ref,
                                    data<parameters_type>& vasp_data_ref):
      parameters(parameters_ref),
      vasp_data (vasp_data_ref),

      kx(vasp_data.kx),
      ky(vasp_data.ky),
      kz(vasp_data.kz),

      band_e(vasp_data.band_e),
      band_o(vasp_data.band_o),

      proj_magni(vasp_data.proj_magni), // proj_magni[nKpoints][nBands][nOrbi]
      proj_phaRe(vasp_data.proj_phaRe), // proj_phaRe[nKpoints][nBands][nOrbi]
      proj_phaIm(vasp_data.proj_phaIm), // proj_phaIm[nKpoints][nBands][nOrbi]

      projector(vasp_data.projector),

      //       projection_re(vasp_data.projection_re), // projection_Re[nKpoints][nCorrBands][nCorrOrbis]
      //       projection_im(vasp_data.projection_im), // projection_Im[nKpoints][nCorrBands][nCorrOrbis]

      //       BlochHami(vasp_data.BlochHami), // BlochHami[nKpoints][nCorrBands][nCorrBands]

      //       BlochHami_re(vasp_data.BlochHami_re),// dfBlochHamiRe  [nKpoints][nCorrOrbis][nCorrOrbis]
      //       BlochHami_im(vasp_data.BlochHami_im),// dfBlochHamiRe  [nKpoints][nCorrOrbis][nCorrOrbis]

      target1 ("ion"),                      // tag the lines that are not useful
      target2 ("tot"),                      // only my choices
      target3 ("PROCAR lm decomposed"),
      target4 ("# of bands:"),
      target5 ("# of k-points:"),

      tagk1 ("k-point"),                    // tag k-point line
      tagk2 ("weight"),                     // tag k-point line
      tagb1 ("band"),                       // tag band energy line
      tagb2 ("occ.")                       // tag band energy line
    {
      cout << "\n PART I starts. \n" ;
    }

    template<class parameters_type>
    reader<parameters_type>::~reader()
    {
      cout << "\n PART I done. \n" ;
    }

    template<class parameters_type>
    void reader<parameters_type>::execute()
    {
      int nLinesTot = 0;

      ifstream input_file(parameters.get_PROCAR_file().c_str());   // input file name

      if(!input_file.is_open())
        cout << "\n ERROR: PROCAR_* is not found \n" << endl;

      while( std::getline(input_file, oneline) )
        ++nLinesTot;

      input_file.clear();

      input_file.seekg(0, input_file.beg);

      int ndp, nstar;

      for(int jj=0; jj<nLinesTot; jj++)
        {
          oneline.clear();
          std::getline(input_file, oneline);

          if ( !(oneline.empty())                     &&
               !(oneline==" ")                        &&
               !(oneline.find(target1)!=string::npos) &&
               !(oneline.find(target2)!=string::npos) &&
               !(oneline.find(target3)!=string::npos) &&
               !(oneline.find(target4)!=string::npos) &&
               !(oneline.find(target5)!=string::npos)    )
            {
              ndp=0;
              nstar=0;
              for (int j=0; j<oneline.length(); j++) { if ( oneline.substr(j,1) == "."  ) ndp=ndp+1; }
              for (int j=0; j<oneline.length(); j++) { if ( oneline.substr(j,6) == "******"  ) nstar=nstar+1; }


              if ( oneline.find(tagk1)!=string::npos && oneline.find(tagk2)!=string::npos )
                {

                  oneline.insert(0, "  ");
                  oneline.append("  ");
                  useful_lines.push_back(oneline);

                }
              else
                if ( oneline.find(tagb1)!=string::npos && oneline.find(tagb2)!=string::npos )
                  {

                    oneline.insert(0, "  ");
                    oneline.append("  ");
                    useful_lines.push_back(oneline);

                  }
                else
                  if ( (ndp+nstar)==9 || (ndp+nstar)==10 )
                    {
                      int ion=0;

                      for (int i=0; i<oneline.size(); i++) if (std::isdigit(oneline[i])) { ion=i; break; }

                      if ( !( oneline.substr(ion,1)!="1" ||
                              oneline.substr(ion,1)!="2" ||
                              oneline.substr(ion,1)!="3" ||
                              oneline.substr(ion,1)!="4" ||
                              oneline.substr(ion,1)!="5"   ) )
                        cout << " Error: atom id problem. " << endl;

                      if ( atof((oneline.substr(ion,1)).c_str()) == parameters.get_iid() )
                        {
                          oneline.insert(0, "  ");
                          oneline.append("  ");
                          useful_lines.push_back(oneline);
                        }

                    }
                  else
                    {
                      cout << "\n You missed some possibility. \n" << endl;
                    }
            }

        }


      cout << "\n Total # of useful lines extracted from PROCAR (expected "
           << parameters.get_nKpoints()*(1+parameters.get_nBands()*4) << ") is: "
           << " " << useful_lines.size() << endl;
      input_file.close();


      int aa;

      // loop k-points
      for(int ik=0; ik<parameters.get_nKpoints(); ik++)
        {
          // cut out sub-string containing kx,ky,kz
          pos1 = -1;
          pos2 = -1;
          dummy.clear();
          pos1   = useful_lines[ik*(1+parameters.get_nBands()*4)].find(":");
          pos2   = useful_lines[ik*(1+parameters.get_nBands()*4)].find("weight");
          dummy  = useful_lines[ik*(1+parameters.get_nBands()*4)].substr( pos1+1, pos2-pos1-1 );

          // store kx,ky,kz values
          pdp.clear();
          for (int i=0; i<dummy.length(); i++)
            if ( dummy.substr(i,1) == "." )
              pdp.push_back(i);

          if (pdp.size()!=3)
            cout << "\n Error: k-point line does not have 3 d.p. \n" << dummy << endl;

          kx(ik) = atof( ( dummy.substr( pdp[0]-2, (pdp[1]-2)-(pdp[0]-2) ) ).c_str() );
          ky(ik) = atof( ( dummy.substr( pdp[1]-2, (pdp[2]-2)-(pdp[1]-2) ) ).c_str() );
          kz(ik) = atof( ( dummy.substr( pdp[2]-2                        ) ).c_str() );

          //cout << "\n";
          if(ik<10)
            cout << ik << "\t" << kx(ik) << "\t" << ky(ik) << "\t" << kz(ik) << "\n";
          //cout << "\n";

          // loop bands, under that k-point
          for (int ib=0; ib<parameters.get_nBands(); ib++)
            {

              int bb = ik*(1+parameters.get_nBands()*4) + (1+ib*4);

              ndp1=0; ndp2=0; ndp3=0; nstar1=0; nstar2=0; nstar3=0;
              for (int j=0; j<useful_lines[bb+1].length(); j++) { if ( useful_lines[bb+1].substr(j,1) == "." )        ndp1=ndp1+1; }
              for (int j=0; j<useful_lines[bb+2].length(); j++) { if ( useful_lines[bb+2].substr(j,1) == "." )        ndp2=ndp2+1; }
              for (int j=0; j<useful_lines[bb+3].length(); j++) { if ( useful_lines[bb+3].substr(j,1) == "." )        ndp3=ndp3+1; }
              for (int j=0; j<useful_lines[bb+1].length(); j++) { if ( useful_lines[bb+1].substr(j,6) == "******" ) nstar1=nstar1+1; }
              for (int j=0; j<useful_lines[bb+2].length(); j++) { if ( useful_lines[bb+2].substr(j,6) == "******" ) nstar2=nstar2+1; }
              for (int j=0; j<useful_lines[bb+3].length(); j++) { if ( useful_lines[bb+3].substr(j,6) == "******" ) nstar3=nstar3+1; }

              if ( !(useful_lines[bb].find("energy")!=string::npos) ||
                   !(useful_lines[bb].find("occ.")!=string::npos)   ||
                   (ndp1+nstar1)!=10                               ||
                   (ndp2+nstar2)!=9                                ||
                   (ndp3+nstar3)!=9                                   )
                cout << "\n Error: problem in the chunk of (kpoint index, band index)= "
                     << "(" << ik << ", " << ib << ")" << endl;

              // deal with band energy line, bb
              pos1 = -1;
              pos2 = -1;
              pos1 = useful_lines[bb].find("energy");
              pos2 = useful_lines[bb].find("occ.");

              band_e(ik,ib) = atof( (useful_lines[bb].substr( pos1+6, (pos2-3)-(pos1+6)+1 )).c_str() ) - parameters.get_Fermi_level();  // E_Fermi!
              band_o(ik,ib) = atof( (useful_lines[bb].substr( pos2+4 )).c_str() );


              dummy.insert(0, "  ");
              dummy.append("  ");

              {
                // deal with projection magnitude line, bb+1
                aa = bb + 1;
                dummy.clear(); head.clear(); tail.clear();
                for (int i=0; i<(useful_lines[aa].length()-1); i++)
                  if (  std::isspace(useful_lines[aa].at(i))   && !std::isspace(useful_lines[aa].at(i+1)) ) head.push_back(i);
                for (int i=1; i<(useful_lines[aa].length()-0); i++)
                  if ( !std::isspace(useful_lines[aa].at(i-1)) &&  std::isspace(useful_lines[aa].at(i))   ) tail.push_back(i);

                if ( head.size()!=tail.size() || (head.size()!=parameters.get_vasp_orbitals()+2 || tail.size()!=parameters.get_vasp_orbitals()+2) )                // check
                  cout << "\n Error: projection magnitude line has problem. " << endl;
                if ( 1.0*parameters.get_iid() != 1.0 * atof( (useful_lines[aa].substr(head[0]+1, tail[0]-head[0]-1)).c_str() ) )  // check
                  cout << "\n Error: you got the wrong impurity atom. " << endl;

                for (int j=0; j<parameters.get_vasp_orbitals(); j++)
                  proj_magni(ik,ib,j) = 1.0*atof( (useful_lines[aa].substr(head[j+1]+1, tail[j+1]-head[j+1]-1)).c_str() );
              }

              {
                // deal with projection phase-Re line, bb+2
                aa = bb + 2;
                dummy.clear(); head.clear(); tail.clear();
                for (int i=0; i<(useful_lines[aa].length()-1); i++)
                  if (  std::isspace(useful_lines[aa].at(i))   && !std::isspace(useful_lines[aa].at(i+1)) ) head.push_back(i);
                for (int i=1; i<(useful_lines[aa].length()-0); i++)
                  if ( !std::isspace(useful_lines[aa].at(i-1)) &&  std::isspace(useful_lines[aa].at(i))   ) tail.push_back(i);

                if ( head.size()!=tail.size() || (head.size()!=parameters.get_vasp_orbitals()+1 || tail.size()!=parameters.get_vasp_orbitals()+1) )                // check
                  cout << "\n Error: projection phase-Re line has problem. " << endl;
                if ( 1.0*parameters.get_iid() != 1.0 * atof( (useful_lines[aa].substr(head[0]+1, tail[0]-head[0]-1)).c_str() ) )  // check
                  cout << "\n Error: you got the wrong impurity atom. " << endl;

                for (int j=0; j<parameters.get_vasp_orbitals(); j++)
                  proj_phaRe(ik,ib,j) = 1.0*atof( (useful_lines[aa].substr(head[j+1]+1, tail[j+1]-head[j+1]-1)).c_str() );
              }

              {
                // deal with projection phase-Im line, bb+3
                aa = bb + 3;
                dummy.clear(); head.clear(); tail.clear();
                for (int i=0; i<(useful_lines[aa].length()-1); i++)
                  if (  std::isspace(useful_lines[aa].at(i))   && !std::isspace(useful_lines[aa].at(i+1)) ) head.push_back(i);
                for (int i=1; i<(useful_lines[aa].length()-0); i++)
                  if ( !std::isspace(useful_lines[aa].at(i-1)) &&  std::isspace(useful_lines[aa].at(i))   ) tail.push_back(i);

                if ( head.size()!=tail.size() || (head.size()!=parameters.get_vasp_orbitals()+1 || tail.size()!=parameters.get_vasp_orbitals()+1) )                // check
                  cout << "\n Error: projection phase-Im line has problem. " << endl;
                if ( 1.0*parameters.get_iid() != 1.0 * atof( (useful_lines[aa].substr(head[0]+1, tail[0]-head[0]-1)).c_str() ) )  // check
                  cout << "\n Error: you got the wrong impurity atom. " << endl;

                for (int j=0; j<parameters.get_vasp_orbitals(); j++)
                  proj_phaIm(ik,ib,j) = 1.0*atof( (useful_lines[aa].substr(head[j+1]+1, tail[j+1]-head[j+1]-1)).c_str() );
              }
            } // loop bands

        } // loop k-points

    }

  }

}

#endif
