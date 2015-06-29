#include <iostream>
#include <fstream>
#include <algorithm>

#include <minc_1_simple.h>
#include <assert.h>
#include <string.h>
// for get_opt_long
#include <getopt.h>
#include <unistd.h>
#include <minc_1_simple_rw.h>


using namespace  std;
using namespace  minc;

static void show_usage(const char *name)
{
  std::cerr 
    << "Usage: "<<name<<" <output_grid_mnc>" << std::endl
    << "Optional parameters:" << std::endl
    << "\t--verbose be verbose" << std::endl
    << "\t--clobber clobber the output files" << std::endl
    << "\t--version print version" << std::endl ;
}

  int main (int argc, char **argv)
  {
  int clobber=0;
  int verbose=0;

  // read the arguments
  static struct option long_options[] =
    {
  	  {"verbose", no_argument, &verbose, 1},
  	  {"quiet",   no_argument, &verbose, 0},
  	  {"clobber", no_argument, &clobber, 1},
      {"version", no_argument, 0, 'v'},
  	  {0, 0, 0, 0}
    };

  int c;
  for (;;)
  {
  	/* getopt_long stores the option index here. */
  	int option_index = 0;

  	c = getopt_long (argc, argv, "vt:m:", long_options, &option_index);

  	/* Detect the end of the options. */
  	if (c == -1)
  		break;

  	switch (c)
  	{
  	case 0:
  		break;
  	case 'v':
  		cout << "Version: 1.0" << endl;
  		return 0;
  	case '?':
  		/* getopt_long already printed an error message. */
  	default:
  		show_usage(argv[0]);
  		return 1;
  	}
  }

  if ((argc - optind) < 1)
  {
  	show_usage(argv[0]);
  	return 1;
  }
  
  std::string output  =argv[optind];
  
  if(!clobber && !access (output.c_str(), F_OK))
  {
    std::cerr << output.c_str () << " Exists!" << std::endl;
    return 1;
  }

  try
  {
    minc_info output_info_grid;
    
    const size_t nx=20,ny=20,nz=20;
    const double step=10;
    const double start_x=-100,start_y=-100,start_z=-100;
    const double amp=2.0; // amplitude of the harmonic deformation 

    // describe minc volume
    output_info_grid.push_back(
      dim_info(3,0,1,dim_info::DIM_VEC,false));
    output_info_grid.push_back(
      dim_info(nx,start_x,step,dim_info::DIM_X,false));
    output_info_grid.push_back(
      dim_info(ny,start_y,step,dim_info::DIM_Y,false));
    output_info_grid.push_back(
      dim_info(nz,start_z,step,dim_info::DIM_Z,false));
    
    //allocate volume
    minc::minc_grid_volume     grid(nx,ny,nz);
    //assign coordinates
    grid.start()=IDX<double>(start_x,start_y,start_z);
    grid.step()=IDX<double>(step,step,step);
    
    for(size_t x=0;x<nx;x++)
      for(size_t y=0;y<ny;y++)
        for(size_t z=0;z<nz;z++)
    {
      fixed_vec<3,double> pos=grid.voxel_to_world(IDX<size_t>(x,y,z));
      fixed_vec<3,float> def;
      
      def[0]=amp*sin(M_PI*pos[0]/100.0);
      def[1]=amp*sin(2*M_PI*pos[1]/100.0);
      def[2]=amp*cos(3*M_PI*pos[2]/100.0);
      
      grid.set(x,y,z,def);
    }
    //
    minc_1_writer wrt_vec;
    wrt_vec.open(output.c_str(),output_info_grid,3,NC_FLOAT);
    save_simple_volume(wrt_vec,grid);
    
  } catch (const minc::generic_error & err) {
    std::cerr << "Got an error at:" << err.file () << ":" << err.line () << std::endl;
    std::cerr << err.msg() << std::endl;
    return 1;
  }
  return 0;
}
