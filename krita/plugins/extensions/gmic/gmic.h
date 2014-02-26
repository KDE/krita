/*
 #
 #  File        : gmic.h
 #                ( C++ header file )
 #
 #  Description : GREYC's Magic for Image Computing
 #                ( http://gmic.sourceforge.net )
 #                This file is also a part of the CImg Library project.
 #                ( http://cimg.sourceforge.net )
 #
 #  Note        : Include this file in your C++ source code, if you
 #                want to use the G'MIC interpreter in your own program.
 #
 #  Copyright   : David Tschumperle
 #                ( http://tschumperle.users.greyc.fr/ )
 #
 #  License     : CeCILL v2.0
 #                ( http://www.cecill.info/licences/Licence_CeCILL_V2-en.html )
 #
 #  This software is governed by the CeCILL  license under French law and
 #  abiding by the rules of distribution of free software.  You can  use,
 #  modify and/ or redistribute the software under the terms of the CeCILL
 #  license as circulated by CEA, CNRS and INRIA at the following URL
 #  "http://www.cecill.info".
 #
 #  As a counterpart to the access to the source code and  rights to copy,
 #  modify and redistribute granted by the license, users are provided only
 #  with a limited warranty  and the software's author,  the holder of the
 #  economic rights,  and the successive licensors  have only  limited
 #  liability.
 #
 #  In this respect, the user's attention is drawn to the risks associated
 #  with loading,  using,  modifying and/or developing or reproducing the
 #  software by the user in light of its specific status of free software,
 #  that may mean  that it is complicated to manipulate,  and  that  also
 #  therefore means  that it is reserved for developers  and  experienced
 #  professionals having in-depth computer knowledge. Users are therefore
 #  encouraged to load and test the software's suitability as regards their
 #  requirements in conditions enabling the security of their systems and/or
 #  data to be ensured and,  more generally, to use and operate it in the
 #  same conditions as regards security.
 #
 #  The fact that you are presently reading this means that you have had
 #  knowledge of the CeCILL license and that you accept its terms.
 #
*/
#include <locale>
#ifndef gmic_version
#define gmic_version 1584

// Define environment variables.
#ifndef gmic_split_compilation
#define gmic_float
#endif // #ifndef gmic_split_compilation
#ifndef gmic_is_beta
#define gmic_is_beta 0
#endif // #ifndef gmic_is_beta
#ifndef cimg_verbosity
#define cimg_verbosity 1
#endif // #ifndef cimg_verbosity
#if defined(cimg_build)
#define cimg_plugin "examples/gmic.cpp"
#define cimg_include_file "../CImg.h"
#elif defined(gmic_build) // #if defined(cimg_build)
#define cimg_plugin "gmic.cpp"
#define cimg_include_file "./CImg.h"
#endif // #if defined(cimg_build)

#ifdef cimg_include_file
#include cimg_include_file
#if cimg_OS==2
#include <process.h>
#pragma comment(linker,"/STACK:8388608")
#elif cimg_OS==1
#include <cerrno>
#endif // #if cimg_OS==2

// Define some special character codes used for replacement in double quoted strings.
const char _dollar = 23, _lbrace = 24, _rbrace = 25, _comma = 26, _dquote = 28, _arobace = 29,
  _newline = 30;

// Replace special characters in a string.
inline char *gmic_strreplace(char *const str) {
  for (char *s = str ; *s; ++s) {
    const char c = *s;
    if (c<' ')
      *s = c==_dollar?'$':c==_lbrace?'{':c==_rbrace?'}':c==_comma?',':
        c==_dquote?'\"':c==_arobace?'@':c;
  }
  return str;
}

#else // #ifdef cimg_include_file
#include <cstdio>
#include <cstring>

#ifndef cimg_version
namespace cimg_library {

  // Define the G'MIC image structure.
  //----------------------------------
  template<typename T> struct CImg {
    unsigned int _width;       // Number of image columns (dimension along the X-axis).
    unsigned int _height;      // Number of image lines (dimension along the Y-axis)
    unsigned int _depth;       // Number of image slices (dimension along the Z-axis).
    unsigned int _spectrum;    // Number of image channels (dimension along the C-axis).
    bool _is_shared;           // Tells if the data buffer is shared by another structure.
    T *_data;                  // Pointer to the first pixel value.

    // Destructor.
    ~CImg();
    // Empty constructor.
    CImg():_width(0),_height(0),_depth(0),_spectrum(0),_is_shared(false),_data(0) {}
    // Use to allocate a new image with specified dimension.
    CImg<T>& assign(const unsigned int w, const unsigned int h=1,
                    const unsigned int d=1, const unsigned int s=1);
  };

  // Define the G'MIC image list structure.
  //---------------------------------------
  template<typename T> struct CImgList {
    unsigned int _width;           // Number of images in the list.
    unsigned int _allocated_width; // Allocated items in the list (must be 2^N and >size).
    CImg<T> *_data;                // Pointer to the first image of the list.

    // Destructor.
    ~CImgList();
    // Empty constructor.
    CImgList():_width(0),_allocated_width(0),_data(0) {}
    // Use to allocate a new image list with specified dimension.
    CImgList<T>& assign(const unsigned int n);
  };
}
#endif // #ifndef cimg_version
#endif // #ifdef cimg_include_file
#define gmic_image cimg_library::CImg
#define gmic_list cimg_library::CImgList
#define gmic_display cimg_library::CImgDisplay

// Define the G'MIC exception class.
//----------------------------------
struct gmic_exception {
  gmic_image<char> _command_help, _message;

  gmic_exception() {}

  gmic_exception(const char *const command, const char *const message) {
    if (command) {
      _command_help.assign(std::strlen(command)+1,1,1,1);
      std::strcpy(_command_help._data,command);
    }
    if (message) {
      _message.assign(std::strlen(message)+1,1,1,1);
      std::strcpy(_message._data,message);
    }
  }

  const char *what() const { // Give the error message returned by the G'MIC interpreter.
    return _message._data?_message._data:"";
  }
  const char *command_help() const {
    return _command_help._data?_command_help._data:"";
  }
};

// Define the G'MIC interpreter class.
//------------------------------------
struct gmic {

  // Constructors - Destructors.
  // Use the methods below to create and run the G'MIC interpreter from your C++ source.

#define gmic_new_attr commands(new CImgList<char>[256]), commands_names(new CImgList<char>[256]), commands_has_arguments(new CImgList<char>[256]), \
    _variables(new CImgList<char>[256]), _variables_names(new CImgList<char>[256]), variables(new CImgList<char>*[256]), variables_names(new CImgList<char>*[256])

  // Destructor.
  ~gmic();

  // Constructors.
  gmic();

  gmic(const char *const commands_line, const char *const custom_commands=0,
       const bool include_default_commands=true, float *const p_progress=0, int *const p_cancel=0);

  template<typename T>
  gmic(const int argc, const char *const *const argv, gmic_list<T>& images,
       gmic_list<char>& images_names,
       const char *const custom_commands=0, const bool include_default_commands=true,
       float *const p_progress=0, int *const p_cancel=0);

  template<typename T>
  gmic(const char *const commands_line, gmic_list<T>& images, gmic_list<char>& images_names,
       const char *const custom_commands=0, const bool include_default_commands=true,
       float *const p_progress=0, int *const p_cancel=0);

  // Method to call parser on an already constructed gmic object.
  template<typename T>
  gmic& parse(const char *const commands_line,
              gmic_list<T> &images, gmic_list<char> &images_names) {
    return parse(commands_line_to_CImgList(commands_line),images,images_names);
  }

  //--------------------------------------------------------------------------------------
  // All functions below should be considered as 'private' and thus, should not be used
  // in your own C++ source code. Use them at your own risk.
  //--------------------------------------------------------------------------------------
  template<typename T>
  void _gmic(const char *const commands_line, gmic_list<T>& images, gmic_list<char>& images_names,
             const char *const custom_commands, const bool include_default_commands,
             float *const p_progress, int *const p_cancel);

  gmic& add_commands(const char *const data_commands,
                     gmic_list<char> commands_names[256],
                     gmic_list<char> commands[256],
                     gmic_list<char> commands_has_arguments[256],
                     const char *const commands_file=0);
  gmic& add_commands(std::FILE *const file,
                     const char *const filename,
                     gmic_list<char> commands_names[256],
                     gmic_list<char> commands[256],
                     gmic_list<char> commands_has_arguments[256],
                     const bool add_debug_infos=false);
  gmic_image<char> scope2string(const bool is_last_slash=true) const;
  gmic_image<char> scope2string(const gmic_image<unsigned int>& scope_selection,
                                const bool is_last_slash=true) const;

  gmic_image<unsigned int> selection2cimg(const char *const string, const unsigned int indice_max,
                                          const gmic_list<char>& names,
                                          const char *const command, const bool is_selection,
                                          const bool allow_new_name, gmic_image<char>& new_name);

  gmic_image<char> selection2string(const gmic_image<unsigned int>& selection,
                                    const gmic_list<char>& images_names,
                                    const bool display_selection) const;

  gmic_list<char> commands_line_to_CImgList(const char *const commands_line);

  template<typename T>
  gmic_image<char> substitute_item(const char *const source,
                                   gmic_list<T>& images, gmic_list<char>& images_names,
				   unsigned int variables_sizes[256]);

  gmic& print(const char *format, ...);
  template<typename T>
  gmic& print(const gmic_list<T>& list, const char *format, ...);
  template<typename T>
  gmic& print(const gmic_list<T>& list, const gmic_image<unsigned int>& scope_selection,
	      const char *format, ...);

  gmic& warn(const char *format, ...);
  template<typename T>
  gmic& warn(const gmic_list<T>& list, const char *format, ...);
  template<typename T>
  gmic& warn(const gmic_list<T>& list, const gmic_image<unsigned int>& scope_selection,
             const char *format, ...);

  gmic& error(const char *format, ...);
  template<typename T>
  gmic& error(const gmic_list<T>& list, const char *format, ...);
  template<typename T>
  gmic& error(const char *const command, const gmic_list<T>& list, const char *format, ...);
  template<typename T>
  gmic& error(const gmic_list<T>& list, const gmic_image<unsigned int>& scope_selection,
	      const char *format, ...);
  template<typename T>
  gmic& _arg_error(const gmic_list<T>& list, const char *const command,
		   const char *const argument);

  gmic& debug(const char *format, ...);
  template<typename T>
  gmic& debug(const gmic_list<T>& list, const char *format, ...);

  template<typename T>
  gmic& print_images(const gmic_list<T>& images,
                     const gmic_list<char>& images_names,
                     const gmic_image<unsigned int>& selection,
                     const bool is_header=true);
  template<typename T>
  gmic& display_images(const gmic_list<T>& images,
                       const gmic_list<char>& images_names,
                       const gmic_image<unsigned int>& selection,
                       unsigned int *const XYZ);
  template<typename T>
  gmic& display_plots(const gmic_list<T>& images,
                      const gmic_list<char>& images_names,
                      const gmic_image<unsigned int>& selection,
                      const unsigned int plot_type, const unsigned int vertex_type,
                      const double xmin, const double xmax,
                      const double ymin, const double ymax);
  template<typename T>
  gmic& display_objects3d(const gmic_list<T>& images,
                          const gmic_list<char>& images_names,
                          const gmic_image<unsigned int>& selection);
  template<typename T>
  gmic_image<T>& check_image(const gmic_list<T>& list, gmic_image<T>& img);
  template<typename T>
  const gmic_image<T>& check_image(const gmic_list<T>& list, const gmic_image<T>& img);

  template<typename T>
  gmic& remove_images(gmic_list<T>& images, gmic_list<char>& images_names,
                      const gmic_image<unsigned int>& selection,
                      const unsigned int start, const unsigned int end);

  template<typename T>
  gmic& parse(const gmic_list<char>& commands_line,
	      gmic_list<T> &images, gmic_list<char> &images_names) {
    unsigned int variables_sizes[256] = { 0 };
    unsigned int position = 0;
    setlocale(LC_NUMERIC,"C");
    scope.assign(1U);
    scope._data[0].assign(2,1,1,1);
    scope._data[0]._data[0] = '.';
    scope._data[0]._data[1] = 0;
    dowhiles.assign(0U);
    repeatdones.assign(0U);
    status.assign(0U);
    is_start = true;
    is_quit = false;
    is_return = false;
    is_default_type = true;
    *progress = -1;
    return _parse(commands_line,position,images,images_names,variables_sizes);
  }

  template<typename T>
  gmic& _parse(const gmic_list<char>& commands_line, unsigned int& position,
               gmic_list<T> &images, gmic_list<char> &images_names,
               unsigned int variables_sizes[256]);
  gmic& _parse_bool(const gmic_list<char>& commands_line, unsigned int& position,
                    gmic_list<bool>& images, gmic_list<char> &images_names,
                    unsigned int variables_sizes[256]);
  gmic& _parse_uchar(const gmic_list<char>& commands_line, unsigned int& position,
                     gmic_list<unsigned char>& images, gmic_list<char> &images_names,
                     unsigned int variables_sizes[256]);
  gmic& _parse_char(const gmic_list<char>& commands_line, unsigned int& position,
                    gmic_list<char>& images, gmic_list<char> &images_names,
                    unsigned int variables_sizes[256]);
  gmic& _parse_ushort(const gmic_list<char>& commands_line, unsigned int& position,
                      gmic_list<unsigned short>& images, gmic_list<char> &images_names,
                      unsigned int variables_sizes[256]);
  gmic& _parse_short(const gmic_list<char>& commands_line, unsigned int& position,
                     gmic_list<short>& images, gmic_list<char> &images_names,
                     unsigned int variables_sizes[256]);
  gmic& _parse_uint(const gmic_list<char>& commands_line, unsigned int& position,
                    gmic_list<unsigned int>& images, gmic_list<char> &images_names,
                    unsigned int variables_sizes[256]);
  gmic& _parse_int(const gmic_list<char>& commands_line, unsigned int& position,
                   gmic_list<int>& images, gmic_list<char> &images_names,
                   unsigned int variables_sizes[256]);
  gmic& _parse_float(const gmic_list<char>& commands_line, unsigned int& position,
                     gmic_list<float>& images, gmic_list<char> &images_names,
                     unsigned int variables_sizes[256]);
  gmic& _parse_double(const gmic_list<char>& commands_line, unsigned int& position,
                      gmic_list<double>& images, gmic_list<char> &images_names,
                      unsigned int variables_sizes[256]);

  // Internal environment variables.
#if cimg_display!=0
  gmic_display instant_window[10];
#endif // #if cimg_display!=0
  gmic_list<char> *const commands, *const commands_names, *const commands_has_arguments,
    *const _variables, *const _variables_names, **const variables, **const variables_names,
    commands_files, scope;
  gmic_list<unsigned int> dowhiles, repeatdones;
  gmic_image<unsigned char> background3d, light3d;
  gmic_image<float> pose3d;
  gmic_image<char> status;
  float focale3d, light3d_x, light3d_y, light3d_z, specular_lightness3d, specular_shininess3d,
    _progress, *progress;
  bool is_released, is_debug, is_start, is_quit, is_return, is_double3d, is_default_type,
    is_debug_infos, check_elif;
  int verbosity, render3d, renderd3d;
  volatile int _cancel, *cancel;
  unsigned int nb_carriages, debug_filename, debug_line;
  unsigned long reference_time;

}; // End of the 'gmic' class.

#endif // #ifndef gmic_version

// Local Variables:
// mode: c++
// End:
