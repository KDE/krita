/**
 * @file pfstmo_trilateral.cpp
 * @brief Tone map XYZ channels using trilateral filter model

 */

#include <config.h>

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <math.h>

#include <pfs.h>

#include <tmo_trilateral.h>

using namespace std;

#define PROG_NAME "pfstmo_trilateral"

/// verbose mode
bool verbose = false;

class QuietException 
{
};

void printHelp()
{
  fprintf( stderr, PROG_NAME ": \n"
    "\t[--saturation <val>] [--sigma <val>] \n"
    "\t[--contrast <val>] [--shift <val>] \n"
    "\t[--verbose] [--help] \n"
    "See man page for more information.\n" );
}

void pfstmo_trilateral( int argc, char* argv[] )
{
  pfs::DOMIO pfsio;

  //--- default tone mapping parameters;
static double    sigma_c          = 21.;         /* The only user paramter                */
static double    saturation       = 1.0;
static double    base_range       = 5.0;
static double    base_value       = 0.0;


  static struct option cmdLineOptions[] = {
	
	
    { "help", no_argument, NULL, 'h' },
	{ "verbose", no_argument, NULL, 'v' },
/////////////////////////////////////////////////////////
	{ "saturation", required_argument, NULL, 's' },
	{ "sigma", required_argument, NULL, 'g' },
	{ "contrast", required_argument, NULL, 'c' },
	{ "shift", required_argument, NULL, 't' },
/////////////////////////////////////////////////////////
	{ NULL, 0, NULL, 0 }
  };

  int optionIndex = 0;
  while( 1 ) {
	  int c = getopt_long (argc, argv, "hv:c:g:s:t", cmdLineOptions, &optionIndex);
    if( c == -1 ) break;
    switch( c ) {
    case 'h':
      printHelp();
      throw QuietException();
    case 'v':
      verbose = true;
      break;
    case 'g':
      sigma_c = (float)strtod( optarg, NULL );
            break;
    case 'c':
      base_range = (float)strtod( optarg, NULL );
	       break;
    case 's':
      saturation = (float)strtod( optarg, NULL );
            break;
	case 't':
      base_value = (float)strtod( optarg, NULL );
            break;
    case '?':
      throw QuietException();
    case ':':
      throw QuietException();
    }
  }

  VERBOSE_STR << "contrast value: " << base_range << endl;
  VERBOSE_STR << "sigma value: " << sigma_c << endl;
  VERBOSE_STR << "shift value: " << base_value << endl;
  VERBOSE_STR << "saturation value: " << saturation << endl;


  while( true ) 
  {
    pfs::Frame *frame = pfsio.readFrame( stdin );
    if( frame == NULL ) break; // No more frames

    pfs::Channel *X, *Y, *Z;
    frame->getXYZChannels( X, Y, Z );
    //---

    if( Y==NULL || X==NULL || Z==NULL)
      throw pfs::Exception( "Missing X, Y, Z channels in the PFS stream" );
        
    // tone mapping
    int w = Y->getCols();
    int h = Y->getRows();
    pfs::Array2D* L = new pfs::Array2DImpl(w,h);
	
    tmo_trilateral( Y, L, base_range, sigma_c, base_value, saturation);

    for( int x=0 ; x<w ; x++ )
      for( int y=0 ; y<h ; y++ )
      {
        float scale = (*L)(x,y) / (*Y)(x,y);
        (*Y)(x,y) *= scale;
        (*X)(x,y) *= scale;
        (*Z)(x,y) *= scale;
      }

    delete L;

    //---
    pfsio.writeFrame( frame, stdout );
    pfsio.freeFrame( frame );        
  }
}

int main( int argc, char* argv[] )
{
  try {
    pfstmo_trilateral( argc, argv );
  }
  catch( pfs::Exception ex ) {
    fprintf( stderr, PROG_NAME " error: %s\n", ex.getMessage() );
    return EXIT_FAILURE;
  }        
  catch( QuietException  ex ) {
    return EXIT_FAILURE;
  }       
  return EXIT_SUCCESS;
}
