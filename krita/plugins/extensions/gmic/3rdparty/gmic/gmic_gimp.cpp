/*
 #
 #  File        : gmic_gimp.cpp
 #                ( C++ source file )
 #
 #  Description : G'MIC for GIMP - A plug-in to allow the use
 #                of G'MIC commands in GIMP.
 #                This file is a part of the CImg Library project.
 #                ( http://cimg.sourceforge.net )
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
 #  data to be ensured and, more generally, to use and operate it in the
 #  same conditions as regards security.
 #
 #  The fact that you are presently reading this means that you have had
 #  knowledge of the CeCILL license and that you accept its terms.
 #
*/

// Include necessary header files.
//--------------------------------
#define cimg_display_type 0
#include "gmic.h"
#undef _gmic_path
#if cimg_OS==2
#define _gmic_path "_gmic\\"
#define _gmic_file_prefix ""
#else
#define _gmic_path ""
#define _gmic_file_prefix "."
#endif
#if !defined(__MACOSX__) && !defined(__APPLE__)
#include <pthread.h>
#endif
#include <locale>
#include <gtk/gtk.h>
#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>
#undef min
#undef max
extern char data_gmic_def[];
extern unsigned int size_data_gmic_def;
extern unsigned char data_gmic_logo[];
extern unsigned int size_data_gmic_logo;
using namespace cimg_library;

// Define plug-in global variables.
//---------------------------------
CImgList<char> gmic_entries;                   // The list of recognized G'MIC menu entries.
CImgList<char> gmic_1stlevel_entries;          // The treepath positions of 1st-level G'MIC menu entries.
CImgList<char> gmic_commands;                  // The list of corresponding G'MIC commands to process the image.
CImgList<char> gmic_preview_commands;          // The list of corresponding G'MIC commands to preview the image.
CImgList<char> gmic_arguments;                 // The list of corresponding needed filter arguments.
CImgList<char> gmic_faves;                     // The list of favorites filters and their default parameters.
CImgList<double> gmic_preview_factors;         // The list of default preview factors for each filter.
CImgList<unsigned int> gmic_button_parameters; // The list of button parameters for the current filter.
CImg<gmic_pixel_type> computed_preview;        // The last computed preview image.
CImg<char> gmic_additional_commands;           // The buffer of additional G'MIC command implementations.
bool _create_dialog_gui;                       // Return value for 'create_gui_dialog()' (set by events handlers).
bool is_block_preview = false;                 // Flag to block preview, when double-clicking on the filter tree.
void **event_infos;                            // Infos that are passed to the GUI callback functions.
int image_id = 0;                              // The image concerned by the plug-in execution.
unsigned int indice_faves = 0;                 // The starting index of favorite filters.
unsigned int nb_available_filters = 0;         // The number of available filters (non-testing).
std::FILE *logfile = 0;                        // The log file if any.
GimpRunMode run_mode;                          // Run-mode used to call the plug-in.
GtkTreeStore *tree_view_store = 0;             // The list of the filters as a GtkTreeView model.
GimpDrawable *drawable_preview = 0;            // The drawable used by the preview window.
GtkWidget *dialog_window = 0;                  // The plug-in dialog window.
GtkWidget *left_pane = 0;                      // The left pane, containing the preview window.
GtkWidget *gui_preview = 0;                    // The preview window.
GtkWidget *relabel_hbox = 0;                   // The entire widget to relabel filter.
GtkWidget *relabel_entry = 0;                  // The text entry to relabel filter.
GtkWidget *tree_view = 0;                      // The filter treeview.
GtkWidget *tree_mode_stock = 0;                // A temporary stock button for the expand/collapse button.
GtkWidget *tree_mode_button = 0;               // Expand/Collapse button for the treeview.
GtkWidget *refresh_stock = 0;                  // A temporary stock button for the refresh button.
GtkWidget *fave_stock = 0;                     // A temporary stock button for the fave button.
GtkWidget *delete_stock = 0;                   // A temporary stock button for the fave button 2.
GtkWidget *fave_add_button = 0;                // Fave button.
GtkWidget *fave_delete_button = 0;             // Fave delete button.
GtkWidget *right_frame = 0;                    // The right frame containing the filter parameters.
GtkWidget *right_pane = 0;                     // The right scrolled window, containing the right frame.
GimpPDBStatusType status = GIMP_PDB_SUCCESS;   // The plug-in return status.
const char *s_blendmode[] = { "alpha","dissolve","behind","multiply","screen","overlay","difference",
                              "add","subtract","darken","lighten","hue","saturation","color","value",
                              "divide","dodge","burn","hardlight","softlight","grainextract",
                              "grainmerge","colorerase" };

// Set/get plug-in persistent variables, using GIMP {get,set}_data() features.
//-----------------------------------------------------------------------------

// Get the folder path of configuration files.
const char* get_conf_path() {
  const char *path_conf = getenv("GMIC_GIMP_PATH");
  if (!path_conf) {
#if cimg_OS!=2
    path_conf = getenv("HOME");
#else
    path_conf = getenv("APPDATA");
#endif
  }
  return path_conf;
}

// Set/get the indice of the currently selected filter.
void set_current_filter(const unsigned int current_filter) {
  const unsigned int ncurrent_filter = current_filter>=gmic_entries.size()?0:current_filter;
  gimp_set_data("gmic_current_filter",&ncurrent_filter,sizeof(unsigned int));
}

unsigned int get_current_filter() {
  unsigned int current_filter = 0;
  gimp_get_data("gmic_current_filter",&current_filter);
  if (current_filter>=gmic_entries.size()) current_filter = 0;
  return current_filter;
}

// Set/get the number of parameters of the specified filter.
void set_filter_nbparams(const unsigned int filter, const unsigned int nbparams) {
  char s_tmp[48];
  cimg_snprintf(s_tmp,sizeof(s_tmp),"gmic_filter%u_nbparams",filter);
  gimp_set_data(s_tmp,&nbparams,sizeof(unsigned int));
}

unsigned int get_filter_nbparams(const unsigned int filter) {
  char s_tmp[48];
  cimg_snprintf(s_tmp,sizeof(s_tmp),"gmic_filter%u_nbparams",filter);
  unsigned int nbparams = 0;
  gimp_get_data(s_tmp,&nbparams);
  return nbparams;
}

// Set/get one particular parameter of a filter.
void set_filter_parameter(const unsigned int filter, const unsigned int n, const char *const param) {
  char s_tmp[48];
  cimg_snprintf(s_tmp,sizeof(s_tmp),"gmic_filter%u_parameter%u",filter,n);
  gimp_set_data(s_tmp,param,std::strlen(param)+1);
}

char *get_filter_parameter(const unsigned int filter, const unsigned int n) {
  static CImg<char> s_param;
  char s_tmp[48];
  cimg_snprintf(s_tmp,sizeof(s_tmp),"gmic_filter%u_parameter%u",filter,n);
  const unsigned int siz = 1U + gimp_get_data_size(s_tmp);
  if (s_param._width<siz) s_param.assign(siz);
  *s_param = 0;
  gimp_get_data(s_tmp,s_param);
  return s_param;
}

// Set/get one particular default parameter of a fave filter.
void set_fave_parameter(const unsigned int filter, const unsigned int n, const char *const param) {
  char s_tmp[48];
  cimg_snprintf(s_tmp,sizeof(s_tmp),"gmic_fave%u_parameter%u",filter,n);
  gimp_set_data(s_tmp,param,std::strlen(param)+1);
}

char *get_fave_parameter(const unsigned int filter, const unsigned int n) {
  static CImg<char> s_param;
  char s_tmp[48];
  cimg_snprintf(s_tmp,sizeof(s_tmp),"gmic_fave%u_parameter%u",filter,n);
  const unsigned int siz = 1U + gimp_get_data_size(s_tmp);
  if (s_param._width<siz) s_param.assign(siz);
  *s_param = 0;
  gimp_get_data(s_tmp,s_param);
  return s_param;
}

// Reset all parameters of all filters.
void reset_filters_parameters() {
  const char *const empty = "";
  for (unsigned int i = 1; i<gmic_entries.size(); ++i)
    for (unsigned int j = 0; ; ++j) {
      const char *const val = get_filter_parameter(i,j);
      if (*val) set_filter_parameter(i,j,empty); else break;
    }
}

// Set/get the filter input, output and preview and verbosity modes.
void set_input_mode(const unsigned int input_mode) {
  gimp_set_data("gmic_input_mode",&input_mode,sizeof(unsigned int));
}

unsigned int get_input_mode(const bool normalized=true) {
  unsigned int input_mode = 0;
  gimp_get_data("gmic_input_mode",&input_mode);
  return normalized?(input_mode<2?1:(input_mode-2)):input_mode;
}

void set_output_mode(const unsigned int output_mode) {
  gimp_set_data("gmic_output_mode",&output_mode,sizeof(unsigned int));
}

unsigned int get_output_mode(const bool normalized=true) {
  unsigned int output_mode = 0;
  gimp_get_data("gmic_output_mode",&output_mode);
  return normalized?(output_mode<2?0:(output_mode-2)):output_mode;
}

void set_preview_mode(const unsigned int preview_mode) {
  gimp_set_data("gmic_preview_mode",&preview_mode,sizeof(unsigned int));
}

unsigned int get_preview_mode(const bool normalized=true) {
  unsigned int preview_mode = 0;
  gimp_get_data("gmic_preview_mode",&preview_mode);
  return normalized?(preview_mode<2?0:(preview_mode-2)):preview_mode;
}

void set_preview_size(const unsigned int preview_size) {
  gimp_set_data("gmic_preview_size",&preview_size,sizeof(unsigned int));
}

unsigned int get_preview_size(const bool normalized=true) {
  unsigned int preview_size = 0;
  gimp_get_data("gmic_preview_size",&preview_size);
  return normalized?(preview_size<2?0:(preview_size-2)):preview_size;
}

void set_verbosity_mode(const unsigned int verbosity) {
  gimp_set_data("gmic_verbosity_mode",&verbosity,sizeof(unsigned int));
}

unsigned int get_verbosity_mode(const bool normalized=true) {
  unsigned int verbosity_mode = 0;
  gimp_get_data("gmic_verbosity_mode",&verbosity_mode);
  return normalized?(verbosity_mode<2?0:(verbosity_mode-2)):verbosity_mode;
}

void set_logfile() {
  const unsigned int verbosity = get_verbosity_mode();
  if (verbosity==3 || verbosity==5 || verbosity==7) {
    if (!logfile) {
      char filename[1024];
      cimg_snprintf(filename,sizeof(filename),"%s%cgmic_log",cimg::temporary_path(),cimg_file_separator);
      logfile = std::fopen(filename,"a");
    }
    cimg::output(logfile?logfile:stdout);
  } else {
    if (logfile) std::fclose(logfile);
    logfile = 0;
    cimg::output(stdout);
  }
}

// Set/get the tree collapse/expand mode.
void set_tree_mode(const bool expand) {
  gimp_set_data("gmic_tree_mode",&expand,sizeof(bool));
}

bool get_tree_mode() {
  bool tree_mode = false;
  gimp_get_data("gmic_tree_mode",&tree_mode);
  return tree_mode;
}

// Set/get the net update activation state.
void set_net_update(const bool net_update) {
  gimp_set_data("gmic_net_update",&net_update,sizeof(bool));
}

bool get_net_update() {
  bool net_update = true;
  gimp_get_data("gmic_net_update",&net_update);
  return net_update;
}

// Set/get the current locale.
void set_locale() {
  char locale[16] = { 0 };
  const char *s_locale = setlocale(LC_CTYPE,0);
  if (!s_locale || std::strlen(s_locale)<2 || !cimg::strncasecmp("lc",s_locale,2)) s_locale = getenv("LANG");
  if (!s_locale || std::strlen(s_locale)<2) s_locale = getenv("LANGUAGE");
  if (!s_locale || std::strlen(s_locale)<2) s_locale = getenv("LC_ALL");
  if (!s_locale || std::strlen(s_locale)<2) s_locale = getenv("LC_CTYPE");
  if (!s_locale || std::strlen(s_locale)<2) s_locale = getenv("LC_TIME");
  if (!s_locale || std::strlen(s_locale)<2) s_locale = getenv("LC_NAME");
  if (!s_locale || std::strlen(s_locale)<2) s_locale = "en";
  std::sscanf(s_locale,"%c%c",&(locale[0]),&(locale[1]));
  cimg::uncase(locale);
  gimp_set_data("gmic_locale",locale,std::strlen(locale)+1);
}

const char *get_locale() {
  static char locale[16];
  *locale = 0;
  gimp_get_data("gmic_locale",locale);
  return locale;
}

// Get layer blending mode from string.
void get_output_layer_props(const char *const s, GimpLayerModeEffects &blendmode, double &opacity,
                            int &posx, int &posy, CImg<char>& name) {
#define _get_output_layer_blendmode(in,out) \
  if (S && !is_blendmode) { \
    const unsigned int l = (unsigned int)std::strlen(in); \
    if (!std::strncmp(S+5,in,l) && S[5+l]==')') { blendmode = out; is_blendmode = true; } \
  }

  if (!s || !*s) return;

  // Read output blending mode.
  const char *S = std::strstr(s,"mode(");
  bool is_blendmode = false;
  _get_output_layer_blendmode("alpha",GIMP_NORMAL_MODE);
  _get_output_layer_blendmode("normal",GIMP_NORMAL_MODE);
  _get_output_layer_blendmode("dissolve",GIMP_DISSOLVE_MODE);
  _get_output_layer_blendmode("lighten",GIMP_LIGHTEN_ONLY_MODE);
  _get_output_layer_blendmode("screen",GIMP_SCREEN_MODE);
  _get_output_layer_blendmode("dodge",GIMP_DODGE_MODE);
  _get_output_layer_blendmode("add",GIMP_ADDITION_MODE);
  _get_output_layer_blendmode("darken",GIMP_DARKEN_ONLY_MODE);
  _get_output_layer_blendmode("multiply",GIMP_MULTIPLY_MODE);
  _get_output_layer_blendmode("burn",GIMP_BURN_MODE);
  _get_output_layer_blendmode("overlay",GIMP_OVERLAY_MODE);
  _get_output_layer_blendmode("softlight",GIMP_SOFTLIGHT_MODE);
  _get_output_layer_blendmode("hardlight",GIMP_HARDLIGHT_MODE);
  _get_output_layer_blendmode("difference",GIMP_DIFFERENCE_MODE);
  _get_output_layer_blendmode("subtract",GIMP_SUBTRACT_MODE);
  _get_output_layer_blendmode("grainextract",GIMP_GRAIN_EXTRACT_MODE);
  _get_output_layer_blendmode("grainmerge",GIMP_GRAIN_MERGE_MODE);
  _get_output_layer_blendmode("divide",GIMP_DIVIDE_MODE);
  _get_output_layer_blendmode("hue",GIMP_HUE_MODE);
  _get_output_layer_blendmode("saturation",GIMP_SATURATION_MODE);
  _get_output_layer_blendmode("color",GIMP_COLOR_MODE);
  _get_output_layer_blendmode("value",GIMP_VALUE_MODE);

  // Read output opacity.
  double _opacity = 0;
  char sep = 0;
  S = std::strstr(s,"opacity(");
  if (S && std::sscanf(S+8,"%lf%c",&_opacity,&sep)==2 && sep==')') opacity = _opacity;

  // Read output positions.
  int _posx = 0, _posy = 0;
  sep = 0;
  S = std::strstr(s,"pos(");
  if (S && std::sscanf(S+4,"%d%*c%d%c",&_posx,&_posy,&sep)==3 && sep==')') { posx = _posx; posy = _posy; }

  // Read output name.
  S = std::strstr(s,"name(");
  if (S) {
    const char *ps = S + 5;
    unsigned int level = 1;
    while (*ps && level) {
      if (*ps=='(') ++level;
      else if (*ps==')') --level;
      ++ps;
    }
    if (!level || *(ps-1)==')') name.assign(S+5,(unsigned int)(ps-S-5)).back() = 0;
  }
}

// Test if a drawable is valid.
bool gmic_drawable_is_valid(const GimpDrawable *const drawable) {
#if GIMP_MINOR_VERSION<=6
  return gimp_drawable_is_valid(drawable->drawable_id);
#else
  return gimp_item_is_valid(drawable->drawable_id);
#endif
}

// Translate string into the current locale.
//------------------------------------------
#define _t(source,dest) if (!std::strcmp(source,s)) { static const char *const ns = dest; return ns; }
const char *t(const char *const s) {

  // Catalan translation
  if (!std::strcmp(get_locale(),"ca")) {
    if (!s) {
      static const char *const ns = "No ha estat possible establir una connexi&#243; a Internet !\n\n"
        "No es possible arribar a aquestes fonts de filtres :\n";
      return ns;
    }
    _t("G'MIC for GIMP","G'MIC per al GIMP");
    _t("<i>Select a filter...</i>","<i>Selecciona un filtre...</i>");
    _t("<i>No parameters to set...</i>","<i>Sense par\303\240metres...</i>");
    _t("<b> Input / Output : </b>","<b> Entrades / Sortides : </b>");
    _t("Input layers...","Capes d'entrada...");
    _t("None","Cap");
    _t("Active (default)","Actiu (predet.)");
    _t("All","Tots");
    _t("Active & below","L'activa i les de sota");
    _t("Active & above","L'activa i les de sobre");
    _t("All visibles","Totes les visibles");
    _t("All invisibles","Totes les invisibles");
    _t("All visibles (decr.)","Totes les visibles (decr.)");
    _t("All invisibles (decr.)","Totes les invisibles (decr.)");
    _t("All (decr.)","Totes (decr.)");
    _t("Output mode...","Mode de sortida...");
    _t("In place (default)","A la capa actual (predet.)");
    _t("New layer(s)","Nova/es capa/es");
    _t("New active layer(s)","Nova/es capa/es actius");
    _t("New image","Nova imatge");
    _t("Preview mode...","Previsualitzaci\303\263 de sortida...");
    _t("1st output (default)","1era imatge (predet.)");
    _t("2nd output","2ona imatge");
    _t("3rd output","3era imatge");
    _t("4th output","4rta imatge");
    _t("1st -> 2nd","1era -> 2ona");
    _t("1st -> 3rd","1era -> 3era");
    _t("1st -> 4th","1era -> 4rta");
    _t("All outputs","Totes les imatges");
    _t("Output messages...","Missatges de sortida...");
    _t("Quiet (default)","Sense missatges (predet.)");
    _t("Verbose (layer name)","Verb\303\263s (nom de la capa)");
    _t("Verbose (console)","Verb\303\263s (consola)");
    _t("Verbose (logfile)","Verb\303\263s (arxiu)");
    _t("Very verbose (console)","Molt verb\303\263s (consola)");
    _t("Very verbose (logfile)","Molt verb\303\263s (arxiu)");
    _t("Debug mode (console)","Depuraci\303\263 (consola)");
    _t("Debug mode (logfile)","Depuraci\303\263 (arxiu)");
    _t("Preview size...","Mida de previsualitzaci\303\263...");
    _t("Tiny","Molt petita");
    _t("Small","Petita");
    _t("Normal","Normal");
    _t("Large","Gran");
    _t(" Available filters (%u) :"," Filtres disponibles (%u) :");
    _t("Update","Actualitzaci\303\263");
    _t("Rename","Canviar");
  }

  // Dutch translation
  if (!std::strcmp(get_locale(),"nl")) {
    if (!s) {
      static const char *const ns = "Geen internet-update mogelijk !\n\n"
        "Kan deze filters bronnen te bereiken :\n";
      return ns;
    }
    _t("G'MIC for GIMP","G'MIC voor GIMP");
    _t("<i>Select a filter...</i>","<i>Kies een filter...</i>");
    _t("<i>No parameters to set...</i>","<i>Geen parameters nodig...</i>");
    _t("<b> Input / Output : </b>","<b> Input / Output : </b>");
    _t("Input layers...","Input lagen...");
    _t("None","Geen");
    _t("Active (default)","Actieve laag (standaard)");
    _t("All","Alle");
    _t("Active & below","Actieve & onderliggende");
    _t("Active & above","Actieve & bovenliggende");
    _t("All visibles","Alle zichtbare");
    _t("All invisibles","Alle niet zichtbare");
    _t("All visibles (decr.)","Alle zichtbare (afnemend)");
    _t("All invisibles (decr.)","Alle niet zichtbare (afnemend)");
    _t("All (decr.)","Alle (afnemend)");
    _t("Output mode...","Output mode...");
    _t("In place (default)","Vervang bestaande (standaard)");
    _t("New layer(s)","Nieuwe laag/lagen");
    _t("New active layer(s)","Nieuwe actieve laag/lagen");
    _t("New image","Nieuwe afbeelding");
    _t("Preview mode...","Output voorbeeld...");
    _t("1st output (default)","1e Resultaat (standaard)");
    _t("2nd output","2e Resultaat");
    _t("3rd output","3e Resultaat");
    _t("4th output","4e Resultaat");
    _t("1st -> 2nd","1e -> 2e");
    _t("1st -> 3rd","1e -> 3e");
    _t("1st -> 4th","1e -> 4e");
    _t("All outputs","Alle resultaten");
    _t("Output messages...","Output berichten...");
    _t("Quiet (default)","Geen melding (standaard)");
    _t("Verbose (layer name)","Uitgebreid (laag naam)");
    _t("Verbose (console)","Uitgebreid (console)");
    _t("Verbose (logfile)","Uitgebreid (logfile)");
    _t("Very verbose (console)","Heel uitgebreid (console)");
    _t("Very verbose (logfile)","Heel uitgebreid (logfile)");
    _t("Debug mode (console)","Debug mode (console)");
    _t("Debug mode (logfile)","Debug mode (logfile)");
    _t("Preview size...","Voorbeeldformaat...");
    _t("Tiny","Zeer klein");
    _t("Small","Klein");
    _t("Normal","Normale");
    _t("Large","Groot");
    _t(" Available filters (%u) :"," Beschikbare filters (%u) :");
    _t("Rename","Hernoemen");
  }

  // French translation
  if (!std::strcmp(get_locale(),"fr")) {
    if (!s) {
      static const char *const ns = "Mise &#224; jour depuis Internet incompl&#232;te !\n\n"
        "Acc&#232;s impossible aux sources de filtres :\n";
      return ns;
    }
    _t("G'MIC for GIMP","G'MIC pour GIMP");
    _t("<i>Select a filter...</i>","<i>Choisissez un filtre...</i>");
    _t("<i>No parameters to set...</i>","<i>Pas de param&#232;tres...</i>");
    _t("<b> Input / Output : </b>","<b> Entr&#233;es / Sorties : </b>");
    _t("Input layers...","Calques d'entr\303\251e...");
    _t("None","Aucun");
    _t("Active (default)","Actif (d\303\251faut)");
    _t("All","Tous");
    _t("Active & below","Actif & en dessous");
    _t("Active & above","Actif & au dessus");
    _t("All visibles","Tous les visibles");
    _t("All invisibles","Tous les invisibles");
    _t("All visibles (decr.)","Tous les visibles (d\303\251cr.)");
    _t("All invisibles (decr.)","Tous les invisibles (d\303\251cr.)");
    _t("All (decr.)","Tous (d\303\251cr.)");
    _t("Output mode...","Mode de sortie...");
    _t("In place (default)","Sur place (d\303\251faut)");
    _t("New layer(s)","Nouveau(x) calque(s)");
    _t("New active layer(s)","Nouveau(x) calque(s) actifs");
    _t("New image","Nouvelle image");
    _t("Preview mode...","Mode d'aper\303\247u...");
    _t("1st output (default)","1\303\250re image (d\303\251faut)");
    _t("2nd output","2\303\250me image");
    _t("3rd output","3\303\250me image");
    _t("4th output","4\303\250me image");
    _t("1st -> 2nd","1\303\250re -> 2\303\250me");
    _t("1st -> 3rd","1\303\250re -> 3\303\250me");
    _t("1st -> 4th","1\303\250re -> 4\303\250me");
    _t("All outputs","Toutes les images");
    _t("Output messages...","Messages de sortie...");
    _t("Quiet (default)","Aucun message (d\303\251faut)");
    _t("Verbose (layer name)","Mode verbeux (nom de calque)");
    _t("Verbose (console)","Mode verbeux (console)");
    _t("Verbose (logfile)","Mode verbeux (fichier log)");
    _t("Very verbose (console)","Mode tr\303\250s verbeux (console)");
    _t("Very verbose (logfile)","Mode tr\303\250s verbeux (fichier log)");
    _t("Debug mode (console)","Mode d\303\251bogage (console)");
    _t("Debug mode (logfile)","Mode d\303\251bogage (fichier log)");
    _t("Preview size...","Taille d'aper\303\247u...");
    _t("Tiny","Minuscule");
    _t("Small","Petite");
    _t("Normal","Normal");
    _t("Large","Grande");
    _t(" Available filters (%u) :"," Filtres disponibles (%u) :");
    _t("Update","Actualiser");
    _t("Rename","Renommer");
  }

  // German translation
  if (!std::strcmp(get_locale(),"de")) {
    if (!s) {
      static const char *const ns = "Kein Internet-Update m\303\266glich !\n\n"
        "Kann diese Filter Quellen erreichen :\n";
      return ns;
    }
    _t("G'MIC for GIMP","G'MIC f\303\274r GIMP");
    _t("<i>Select a filter...</i>","<i>W\303\244hlen Sie einen Filter...</i>");
    _t("<i>No parameters to set...</i>","<i>Keine w\303\244hlbaren Parameter...</i>");
    _t("<b> Input / Output : </b>","<b> Eingabe / Ausgabe : </b>");
    _t("Input layers...","Eingabeebenen...");
    _t("None","Keine");
    _t("Active (default)","Aktive (Standard)");
    _t("All","Alle");
    _t("Active & below","Aktive & darunterliegende");
    _t("Active & above","Aktive & dar\303\274berliegende");
    _t("All visibles","Alle sichtbaren");
    _t("All invisibles","Alle nicht sichtbaren");
    _t("All visibles (decr.)","Alle sichtbaren (absteigend)");
    _t("All invisibles (decr.)","Alle nicht sichtbaren (absteigend)");
    _t("All (decr.)","Alle (absteigend)");
    _t("Output mode...","Ausgabemodus...");
    _t("In place (default)","Bestehende ersetzen (standard)");
    _t("New layer(s)","Neue Ebene(n)");
    _t("New active layer(s)","Neue aktive Ebene(n)");
    _t("New image","Neues Bild");
    _t("Preview mode...","Ausgabevorschau...");
    _t("1st output (default)","1. Ausgabe (Standard)");
    _t("2nd output","2. Ausgabe");
    _t("3rd output","3. Ausgabe");
    _t("4th output","4. Ausgabe");
    _t("1st -> 2nd","1. -> 2.");
    _t("1st -> 3rd","1. -> 3.");
    _t("1st -> 4th","1. -> 4.");
    _t("All outputs","Alle Ausgaben");
    _t("Output messages...","Ausgabemeldungen...");
    _t("Quiet (default)","Keine Meldung (Standard)");
    _t("Verbose (layer name)","Ausf\303\274hrlich (Ebenennamen)");
    _t("Verbose (console)","Ausf\303\274hrlich (Konsole)");
    _t("Verbose (logfile)","Ausf\303\274hrlich (Logfile)");
    _t("Very verbose (console)","Sehr ausf\303\274hrlich (Konsole)");
    _t("Very verbose (logfile)","Sehr ausf\303\274hrlich (Logfile)");
    _t("Debug mode (console)","Debug-Modus (Konsole)");
    _t("Debug mode (logfile)","Debug-Modus (Logfile)");
    _t("Preview size...","Vorschaugrosse...");
    _t("Tiny","Winzig");
    _t("Small","Klein");
    _t("Normal","Normal");
    _t("Large","Gross");
    _t(" Available filters (%u) :"," Verf\303\274gbare Filter (%u) :");
    _t("Rename","Umbenennen");
  }

  // Italian translation
  if (!std::strcmp(get_locale(),"it")) {
    if (!s) {
      static const char *const ns = "Impossibile aggiornare da Internet !\n\n"
        "Impossibile raggiungere queste fonti filtri :\n";
      return ns;
    }
    _t("G'MIC for GIMP","G'MIC per GIMP");
    _t("<i>Select a filter...</i>","<i>Sciegliete un Filtro...</i>");
    _t("<i>No parameters to set...</i>","<i>Filtro senza Parametri...</i>");
    _t("<b> Input / Output : </b>","<b> Input / Output : </b>");
    _t("Input layers...","Input da Layers...");
    _t("None","Nessuno");
    _t("Active (default)","Layer Attivo (default)");
    _t("All","Tutti");
    _t("Active & below","Attivo & superiori");
    _t("Active & above","Attivo & inferiori");
    _t("All visibles","Tutti i Visibili");
    _t("All invisibles","Tutti gli invisibili");
    _t("All visibles (decr.)","Tutti i visibili (dal fondo)");
    _t("All invisibles (decr.)","Tutti gli invisibili (dal fondo)");
    _t("All (decr.)","Tutti");
    _t("Output mode...","Tipo di output...");
    _t("In place (default)","Applica al Layer attivo (default) ");
    _t("New layer(s)","Nuovo(i) Layer(s)");
    _t("New active layer(s)","Attiva Nuovo(i) Layer(s)");
    _t("New image","Nuova Immagine");
    _t("Preview mode...","Anteprima...");
    _t("1st output (default)","Primo Output (default)");
    _t("2nd output","Secondo Output");
    _t("3rd output","Terzo Output");
    _t("4th output","Quarto Output");
    _t("1st -> 2nd","1 -> 2");
    _t("1st -> 3rd","1 -> 3");
    _t("1st -> 4th","1 -> 4");
    _t("All outputs","Tutti i layers");
    _t("Output messages...","Messaggi di Output...");
    _t("Quiet (default)","Nessun Messaggio (default)");
    _t("Verbose (layer name)","Messagi (nome del Layer)");
    _t("Verbose (console)","Messagi (console)");
    _t("Verbose (logfile)","Messagi (logfile)");
    _t("Very verbose (console)","Messaggi Dettagliati (console)");
    _t("Very verbose (logfile)","Messaggi Dettagliati (logfile)");
    _t("Debug mode (console)","Debug Mode (console)");
    _t("Debug mode (logfile)","Debug Mode (logfile)");
    _t("Preview size...","Anteprima dimensioni...");
    _t("Tiny","Minuscolo");
    _t("Small","Piccolo");
    _t("Normal","Normale");
    _t("Large","Grande");
    _t(" Available filters (%u) :"," Filtri disponibili (%u) :");
    _t("Update","Aggiornare");
  }

  // Polish translation
  if (!std::strcmp(get_locale(),"pl")) {
    if (!s) {
      static const char *const ns = "Aktualizacja filtr\303\263w przez internet (cz\304\231\305\233ciowo) nie "
        "powiod\305\202a si\304\231 !\n\n"
        "Brak dost\304\231pu do tych \305\272r\303\263de\305\202 filtr\303\263w :\n";
      return ns;
    }
    _t("G'MIC for GIMP","G'MIC dla GIMP");
    _t("<i>Select a filter...</i>","<i>Wybierz filtr...</i>");
    _t("<i>No parameters to set...</i>","<i>Brak parametr\304\205w do ustawienia...</i>");
    _t("<b> Input / Output : </b>","<b> Wej\305\233cie / Wyj\305\233cie : </b>");
    _t("Input layers...","Warstwy wej\305\233cia...");
    _t("None","Brak");
    _t("Active (default)","Aktywna (domy\305\233lnie)");
    _t("All","Wszystkie");
    _t("Active & below","Aktywna & poni\305\274ej");
    _t("Active & above","Aktywna & powy\305\274ej");
    _t("All visibles","Wszystkie widoczne");
    _t("All invisibles","Wszystkie niewidoczne");
    _t("All visibles (decr.)","Wszystkie widoczne (od do\305\202u)");
    _t("All invisibles (decr.)","Wszystkie niewidoczne (od do\305\202u)");
    _t("All (decr.)","Wszystkie (od do\305\202u)");
    _t("Output mode...","Tryb wyj\305\233cia...");
    _t("In place (default)","Na miejscu (domy\305\233lnie)");
    _t("New layer(s)","Nowa/e warstwa/y");
    _t("New active layer(s)","Nowa/e aktywna/e warstwa/y");
    _t("New image","Nowy obraz");
    _t("Preview mode...","Podgl\304\205d wyj\305\233cia dla warstw...");
    _t("1st output (default)","Pierwszej (domy\305\233lnie)");
    _t("2nd output","Drugiej");
    _t("3rd output","Trzeciej");
    _t("4th output","Czwartej");
    _t("1st -> 2nd","Od 1 do 2");
    _t("1st -> 3rd","Od 1 do 3");
    _t("1st -> 4th","Od 1 do 4");
    _t("All outputs","Wszystkich");
    _t("Output messages...","Komunikat wyj\305\233cia...");
    _t("Quiet (default)","Brak (domy\305\233lnie)");
    _t("Verbose (layer name)","Og\303\263lny (nazwa warstwy)");
    _t("Verbose (console)","Og\303\263lny (konsola)");
    _t("Verbose (logfile)","Og\303\263lny (plik log)");
    _t("Very verbose (console)","Dok\305\202adny (konsola)");
    _t("Very verbose (logfile)","Dok\305\202adny (plik log)");
    _t("Debug mode (console)","Debugowanie (konsola)");
    _t("Debug mode (logfile)","Debugowanie (plik log)");
    _t("Preview size...","Rozmiar podgl\304\205d...");
    _t("Tiny","Male");
    _t("Small","Mala");
    _t("Normal","Normalne");
    _t("Large","Duza");
    _t(" Available filters (%u) :"," Dost\304\231pne filtry (%u) :");
    _t("Update","Uaktualnij");
    _t("Rename","Zmiana nazwy");
  }

  // Portuguese translation
  if (!std::strcmp(get_locale(),"pt")) {
    if (!s) {
      static const char *const ns = "A atualiza\303\247\303\243o pela internet falhou !\n\n"
        "Incapaz de chegar a essas fontes de filtros :\n";
      return ns;
    }
    _t("G'MIC for GIMP","G'MIC para o GIMP");
    _t("<i>Select a filter...</i>","<i>Escolha um filtro</i>");
    _t("<i>No parameters to set...</i>","<i>Sem par\303\242metros para configurar...</i>");
    _t("<b> Input / Output : </b>","<b> Entrada / Saida : </b>");
    _t("Input layers...","Camadas de Entrada...");
    _t("None","Nenhuma");
    _t("Active (default)","Ativo (Padr\303\243o)");
    _t("All","Todos");
    _t("Active & below","Ativo & abaixo");
    _t("Active & above","Ativo & acima");
    _t("All visibles","Todos vis\303\255veis");
    _t("All invisibles","Todos invis\303\255veis");
    _t("All visibles (decr.)","Todos vis\303\255veis (decr.)");
    _t("All invisibles (decr.)","Todos invis\303\255veis (decr.)");
    _t("All (decr.)","Todos (decr.)");
    _t("Output mode...","Modo de saida...");
    _t("In place (default)","No lugar (Padr\303\243o)");
    _t("New layer(s)","Nova(s) camada(s)");
    _t("New active layer(s)","Nova(s) camadas(s) ativa");
    _t("New image","Nova imagem");
    _t("Preview mode...","Pr\303\251 Visualiza\303\247\303\243o");
    _t("1st output (default)","Primeira pr\303\251via (Padr\303\243o)");
    _t("2nd output","2 pr\303\251via imagem");
    _t("3rd output","3 pr\303\251via imagem");
    _t("4th output","4 pr\303\251via imagem");
    _t("1st -> 2nd","1st -> 2nd");
    _t("1st -> 3rd","1st -> 3rd");
    _t("1st -> 4th","1st -> 4th");
    _t("All outputs","Todas as imagens");
    _t("Output messages...","Mensagens de saida...");
    _t("Quiet (default)","Quieto (Padr\303\243o)");
    _t("Verbose (layer name)","Mode verbose (nome da camada)");
    _t("Verbose (console)","Mode verbose (console)");
    _t("Verbose (logfile)","Mode verbose (arquivo)");
    _t("Very verbose (console)","Modo verbose ampliada (console)");
    _t("Very verbose (logfile)","Modo verbose ampliada (arquivo)");
    _t("Debug mode (console)","Modo Debug (console)");
    _t("Debug mode (logfile)","Modo Debug (arquivo)");
    _t("Preview size...","O tamanho da visualiza\303\247\303\243o...");
    _t("Tiny","Minusculo");
    _t("Small","Pequeno");
    _t("Normal","Normal");
    _t("Large","Grande");
    _t(" Available filters (%u) :"," Filtros dispon\303\255veis (%u) :");
    _t("Update","Atualizar");
    _t("Rename","Renomear");
  }

  // Serbian translation
  if (!std::strcmp(get_locale(),"sr")) {
    if (!s) {
      static const char *const ns = "A\305\276uriranje filtera sa interneta (delimi\304\215no) neuspe\305\241no !\n\n"
        "Nije mogu\304\207e dospeti do izvorne lokacije ovih filtera :\n";
      return ns;
    }
    _t("G'MIC for GIMP","G'MIC za GIMP");
    _t("<i>Select a filter...</i>","<i>Izaberite filter...</i>");
    _t("<i>No parameters to set...</i>","<i>Nema parametara za pode\305\241avanje...</i>");
    _t("<b> Input / Output : </b>","<b> Ulazni podaci / Rezultati : </b>");
    _t("Input layers...","Ulazni slojevi...");
    _t("None","Nijedan");
    _t("Active (default)","Aktivan (podrazumevana opcija)");
    _t("All","Svi");
    _t("Active & below","Aktivni & ispod");
    _t("Active & above","Aktivni & iznad");
    _t("All visibles","Svi vidljivi");
    _t("All invisibles","Svi nevidljivi");
    _t("All visibles (decr.)","Svi vidljivi (po opadaju\304\207em nizu)");
    _t("All invisibles (decr.)","Svi nevidljivi (po opadaju\304\207em nizu)");
    _t("All (decr.)","Svi (po opadaju\304\207em nizu)");
    _t("Output mode...","Izlazni mod...");
    _t("In place (default)","Umesto (podrazumevana opcija)");
    _t("New layer(s)","Novi sloj(evi)");
    _t("New active layer(s)","Novi aktivni sloj(evi)");
    _t("New image","Nova slika");
    _t("Preview mode...","Pregled rezultata...");
    _t("1st output (default)","prvi rezultat (podrazumevana opcija)");
    _t("2nd output","drugi rezultat");
    _t("3rd output","tre\304\207i rezultat");
    _t("4th output","\304\215etvrti rezultat");
    _t("1st -> 2nd","prvi -> drugi");
    _t("1st -> 3rd","prvi -> tre\304\207i");
    _t("1st -> 4th","prvi -> \304\215etvrti");
    _t("All outputs","Svi rezultati");
    _t("Output messages...","Izlazne poruke...");
    _t("Quiet (default)","Tiho (podrazumevana opcija)");
    _t("Verbose (layer name)","Op\305\241irnije (slojevi)");
    _t("Verbose (console)","Op\305\241irnije (konzola)");
    _t("Verbose (logfile)","Op\305\241irnije (log fajl)");
    _t("Very verbose (console)","Vrlo op\305\241irno (konzola)");
    _t("Very verbose (logfile)","Vrlo op\305\241irno (log fajl)");
    _t("Debug mode (console)","Mod za otklanjanje programskih gre\305\241aka (konzola)");
    _t("Debug mode (logfile)","Mod za otklanjanje programskih gre\305\241aka (log fajl)");
    _t("Preview size...","Pregled velicine...");
    _t("Tiny","Veoma mali");
    _t("Small","Mali");
    _t("Normal","Normalno");
    _t("Large","Veliki");
    _t(" Available filters (%u) :"," Raspolo\305\276ivi filteri (%u) :");
    _t("Rename","Preimenovati");
  }

  // Spanish translation (Castillan)
  if (!std::strcmp(get_locale(),"es")) {
    if (!s) {
      static const char *const ns = "No es posible establecer conexión a Internet !\n\n"
        "No es posible acceder a estas fuentes de filtros :\n";
      return ns;
    }
    _t("G'MIC for GIMP","G'MIC para GIMP");
    _t("<i>Select a filter...</i>","<i>Selecciona un filtro...</i>");
    _t("<i>No parameters to set...</i>","<i>Sin par\303\241metros...</i>");
    _t("<b> Input / Output : </b>","<b> Entrada / Salida : </b>");
    _t("Input layers...","Capas de entrada...");
    _t("None","Ninguna");
    _t("Active (default)","Activa (predet.)");
    _t("All","Todas");
    _t("Active & below","Activa e inferior");
    _t("Active & above","Activa y superior");
    _t("All visibles","Todas las visibles");
    _t("All invisibles","Todas las invisibles");
    _t("All visibles (decr.)","Todas las visibles (decr.)");
    _t("All invisibles (decr.)","Todas las invisibles (decr.)");
    _t("All (decr.)","Todas (decr.)");
    _t("Output mode...","Modo de salida...");
    _t("In place (default)","En la capa actual (predet.)");
    _t("New layer(s)","Capa/as nueva/as");
    _t("New active layer(s)","Capa/as nueva/as activa");
    _t("New image","Imagen nueva");
    _t("Preview mode...","Previsualizaci\303\263n de la salida...");
    _t("1st output (default)","1ra imagen (predet.)");
    _t("2nd output","2da imagen");
    _t("3rd output","3ra imagen");
    _t("4th output","4ta imagen");
    _t("1st -> 2nd","1ra -> 2da");
    _t("1st -> 3rd","1ra -> 3ra");
    _t("1st -> 4th","1ra -> 4ta");
    _t("All outputs","Todas las imagenes (salida)");
    _t("Output messages...","Mensajes de salida...");
    _t("Quiet (default)","Sin mensajes (predet.)");
    _t("Verbose (layer name)","Detallado (nombre de la capa)");
    _t("Verbose (console)","Detallado (consola)");
    _t("Verbose (logfile)","Detallado (archivo_registro)");
    _t("Very verbose (console)","Muy detallado (consola)");
    _t("Very verbose (logfile)","Muy detallado (archivo_registro)");
    _t("Debug mode (console)","Depuraci\303\263n (consola)");
    _t("Debug mode (logfile)","Depuraci\303\263n (archivo_registro)");
    _t("Preview size...","Tamano de previsualizaci\303\263n...");
    _t("Tiny","Muy pequena");
    _t("Small","Pequena");
    _t("Normal","Normal");
    _t("Large","Grande");
    _t(" Available filters (%u) :"," Filtros disponibles (%u) :");
    _t("Update","Actualitzaci\303\263n");
    _t("Rename","Renombrar");
  }

  // English translation (default)
  if (!s) {
    static const char *const ns = "Filters update from Internet (partially) failed !\n\n"
      "Unable to reach these filters sources :\n";
    return ns;
  }
  return s;
}

// Flush filter tree view
//------------------------
void flush_tree_view(GtkWidget *const tree_view) {
  const unsigned int filter = get_current_filter();
  bool tree_mode = get_tree_mode();
  unsigned int current_dir = 0;
  char current_path[64] = { 0 };
  gimp_get_data("gmic_current_treepath",current_path);
  if (tree_mode) { // Expand
    cimglist_for(gmic_1stlevel_entries,l) {
      GtkTreePath *path = gtk_tree_path_new_from_string(gmic_1stlevel_entries[l].data());
      gtk_tree_view_expand_row(GTK_TREE_VIEW(tree_view),path,false);
      gtk_tree_path_free(path);
    }
  } else { // Collapse
    if (filter && *current_path && std::sscanf(current_path,"%u",&current_dir)==1) {
      cimglist_for(gmic_1stlevel_entries,l) {
        const char *const s_path = gmic_1stlevel_entries[l].data();
        unsigned int dir = 0;
        if (std::sscanf(s_path,"%u",&dir)!=1 || dir!=current_dir) {
          GtkTreePath *path = gtk_tree_path_new_from_string(gmic_1stlevel_entries[l].data());
          gtk_tree_view_collapse_row(GTK_TREE_VIEW(tree_view),path);
          gtk_tree_path_free(path);
        }
      }
    } else gtk_tree_view_collapse_all(GTK_TREE_VIEW(tree_view));
  }
  if (filter && *current_path) {
    GtkTreePath *path = gtk_tree_path_new_from_string(current_path);
    gtk_tree_view_expand_to_path(GTK_TREE_VIEW(tree_view),path);
    gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(tree_view),path,NULL,FALSE,0,0);
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view));
    gtk_tree_selection_select_path(selection,path);
    gtk_tree_path_free(path);
  }
  if (indice_faves<gmic_entries.size()) { // Always shows 'Faves' folder when available.
    GtkTreePath *path = gtk_tree_path_new_from_string(gmic_1stlevel_entries[0].data());
    gtk_tree_view_expand_row(GTK_TREE_VIEW(tree_view),path,false);
    gtk_tree_path_free(path);
  }
  if (tree_mode_stock) gtk_widget_destroy(tree_mode_stock);
  tree_mode_stock = gtk_button_new_from_stock(tree_mode?GTK_STOCK_ZOOM_OUT:GTK_STOCK_ZOOM_IN);
  GtkWidget *tree_image = gtk_button_get_image(GTK_BUTTON(tree_mode_stock));
  gtk_button_set_image(GTK_BUTTON(tree_mode_button),tree_image);
  gtk_widget_show(tree_mode_button);

  gtk_tree_view_remove_column(GTK_TREE_VIEW(tree_view),gtk_tree_view_get_column(GTK_TREE_VIEW(tree_view),0));
  GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
  char tree_view_title[64] = { 0 };
  cimg_snprintf(tree_view_title,sizeof(tree_view_title),t(" Available filters (%u) :"),nb_available_filters);
  GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(tree_view_title,renderer,"markup",1,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view),column);
}

// Retrieve files and update filter tree structure.
//-------------------------------------------------
CImgList<char> update_filters(const bool try_net_update) {

  // Build list of filter sources.
  CImgList<gmic_pixel_type> _sources;
  CImgList<char> _names;
  char command[1024] = { 0 };
  cimg_snprintf(command,sizeof(command),"%s-gimp_filter_sources",
                get_verbosity_mode()>5?"-debug ":get_verbosity_mode()>3?"":"-v -99 ");
  try { gmic(command,_sources,_names,gmic_additional_commands,true); } catch (...) {}
  CImgList<char> sources;
  _sources.move_to(sources);
  cimglist_for(sources,l) {
    char &c = sources[l].unroll('x').back();
    if (c) {
      if (c==1) { c = 0; sources[l].columns(0,sources[l].width()); sources[l].back() = 1; }
      else sources[l].columns(0,sources[l].width());
    }
  }

  // Free existing definitions.
  if (tree_view_store) g_object_unref(tree_view_store);
  gmic_additional_commands.assign();
  gmic_1stlevel_entries.assign();
  gmic_faves.assign();
  gmic_entries.assign(1);
  gmic_commands.assign(1);
  gmic_preview_commands.assign(1);
  gmic_preview_factors.assign(1);
  gmic_arguments.assign(1);
  if (try_net_update) gimp_progress_init(" G'MIC : Update filters...");

  // Get filter definition files from external web servers.
  const char *const path_conf = get_conf_path();
  char filename[1024] = { 0 };
  if (try_net_update) gimp_progress_pulse();
  char filename_tmp[1024] = { 0 }, sep = 0;
  CImgList<char> invalid_servers;
  cimglist_for(sources,l) if (try_net_update && (!cimg::strncasecmp(sources[l],"http://",7) ||
                                                 !cimg::strncasecmp(sources[l],"https://",8))) {
    const char *const s_basename = gmic_basename(sources[l]);
    gimp_progress_set_text_printf(" G'MIC : Update filters '%s'...",s_basename);
    cimg_snprintf(filename,sizeof(filename),"%s%c%s%s",
                  path_conf,cimg_file_separator,_gmic_file_prefix,s_basename);

    // Download filter file.
    if (get_verbosity_mode()>1) { // Verbose mode.
      std::fprintf(cimg::output(),"\n[gmic_gimp]./update/ Download new filter data from '%s'.\n",
                   sources[l].data());
      std::fflush(cimg::output());
    }

    const unsigned int omode = cimg::exception_mode();
    cimg::exception_mode() = 0;
    try {
      cimg::load_network(sources[l],filename_tmp);
      std::FILE *file = std::fopen(filename_tmp,"rb");

      // Eventually, uncompress .cimgz file.
      if (file && (std::fscanf(file," #@gmi%c",&sep)!=1 || sep!='c')) {
        std::rewind(file);
        try {
          CImg<unsigned char> buffer; buffer.load_cimg(file); std::fclose(file);
          if (get_verbosity_mode()>1)
            std::fprintf(cimg::output(),
                         "\n[gmic_gimp]./update/ Uncompress file '%s' (was '%s').\n",
                         filename_tmp,sources[l].data());
          buffer.save_raw(filename_tmp); file = std::fopen(filename_tmp,"rb"); }
        catch (...) { std::rewind(file); }
      }

      // Copy file to its final location.
      if (file && std::fscanf(file," #@gmi%c",&sep)==1 && sep=='c') {
        std::fclose(file);
        if (get_verbosity_mode()>1)
          std::fprintf(cimg::output(),
                       "\n[gmic_gimp]./update/ Copy temporary file '%s' at destination '%s'.\n",
                       filename_tmp,filename);
        CImg<unsigned char>::get_load_raw(filename_tmp).save_raw(filename);
      } else invalid_servers.insert(sources[l]); // Failed in recognizing file header.

    } catch (...) { // Failed in downloading file.
      invalid_servers.insert(sources[l]);
    }
    cimg::exception_mode() = omode;
    gimp_progress_pulse();
    std::remove(filename_tmp);
  }
  gimp_progress_set_text(" G'MIC : Update filters...");

  // Read local source files.
  CImgList<char> _gmic_additional_commands;
  bool is_default_update = false;
  cimglist_for(sources,l) {
    const char *s_basename = gmic_basename(sources[l]);
    cimg_snprintf(filename,sizeof(filename),"%s%c%s%s",
                  path_conf,cimg_file_separator,_gmic_file_prefix,s_basename);
    const unsigned int omode = cimg::exception_mode();
    try {
      cimg::exception_mode(0);
      CImg<char>::get_load_raw(filename).move_to(_gmic_additional_commands);
      CImg<char>::string("\n#@gimp ________\n",false).unroll('y').move_to(_gmic_additional_commands);
      if (sources[l].back()==1) is_default_update = true;
    } catch(...) {
      if (get_verbosity_mode()>1)
        std::fprintf(cimg::output(),
                     "\n[gmic_gimp]./update/ Filter file '%s' not found.\n",
                     filename);
      std::fflush(cimg::output());
    }
    cimg::exception_mode(omode);
    if (try_net_update) gimp_progress_pulse();
  }

  if (!is_default_update) { // Add hardcoded default filters if no updates of the default commands.
    CImg<char>(data_gmic_def,1,size_data_gmic_def-1,1,1,true).move_to(_gmic_additional_commands);
    CImg<char>::string("\n#@gimp ________\n",false).unroll('y').move_to(_gmic_additional_commands);
  }
  CImg<char>::vector(0).move_to(_gmic_additional_commands);
  (_gmic_additional_commands>'y').move_to(gmic_additional_commands);

  // Add fave folder if necessary (make it before actually adding faves to make tree paths valids).
  CImgList<char> gmic_1stlevel_names;
  GtkTreeIter iter, fave_iter, parent[8];
  char filename_gmic_faves[1024] = { 0 };
  tree_view_store = gtk_tree_store_new(2,G_TYPE_UINT,G_TYPE_STRING);
  cimg_snprintf(filename_gmic_faves,sizeof(filename_gmic_faves),"%s%c%sgmic_faves",
                path_conf,cimg_file_separator,_gmic_file_prefix);
  std::FILE *file_gmic_faves = std::fopen(filename_gmic_faves,"rb");
  if (file_gmic_faves) {
    gtk_tree_store_append(tree_view_store,&fave_iter,0);
    gtk_tree_store_set(tree_view_store,&fave_iter,0,0,1,"<b>Faves</b>",-1);
    const char *treepath = gtk_tree_model_get_string_from_iter(GTK_TREE_MODEL(tree_view_store),&fave_iter);
    CImg<char>::vector(0).move_to(gmic_1stlevel_names);
    CImg<char>::string(treepath).move_to(gmic_1stlevel_entries);
  }

  // Parse filters descriptions for GIMP, and create corresponding sorted treeview_store.
  char line[256*1024] = { 0 }, preview_command[256] = { 0 }, arguments[65536] = { 0 },
       entry[256] = { 0 }, locale[16] = { 0 };
  std::strcpy(locale,get_locale());
  int level = 0, err = 0;
  bool is_testing = false;
  nb_available_filters = 0;
  cimg_snprintf(line,sizeof(line),"#@gimp_%s ",locale);

  // Use English for default language if no translated filters found.
  if (!std::strstr(gmic_additional_commands,line)) { locale[0] = 'e'; locale[1] = 'n'; locale[2] = 0; }
  for (const char *data = gmic_additional_commands; *data; ) {
    char *_line = line;
    // Read new line.
    while (*data!='\n' && *data && _line<line+sizeof(line)) *(_line++) = *(data++); *_line = 0;
    while (*data=='\n') ++data; // Skip next '\n'.
    for (_line = line; *_line; ++_line) if (*_line<' ') *_line = ' '; // Replace non-usual characters by spaces.
    if (line[0]!='#' || line[1]!='@' || line[2]!='g' || // Check for a '#@gimp' line.
        line[3]!='i' || line[4]!='m' || line[5]!='p') continue;
    if (line[6]=='_') { // Check for a localized filter.
      // Weither the entry match current locale or not.
      if (line[7]==locale[0] && line[8]==locale[1] && line[9]==' ') _line = line + 10;
      else continue;
    } else if (line[6]==' ') _line = line + 7; else continue; // Check for a non-localized filter.

    if (*_line!=':') { // Check for a description of a possible filter or menu folder.
      *entry = *command = *preview_command = *arguments = 0;
      err = std::sscanf(_line," %4095[^:]: %4095[^,]%*c %4095[^,]%*c %65533[^\n]",
                        entry,command,preview_command,arguments);
      if (err==1) { // If entry defines a menu folder.
        cimg::strpare(entry,' ',false,true);
        char *nentry = entry; while (*nentry=='_') { ++nentry; --level; }
        if (level<0) level = 0; else if (level>7) level = 7;
        cimg::strpare(nentry,' ',false,true); cimg::strpare(nentry,'\"',true);
        if (*nentry) {
          if (level) {
            gtk_tree_store_append(tree_view_store,&parent[level],level?&parent[level-1]:0);
            gtk_tree_store_set(tree_view_store,&parent[level],0,0,1,nentry,-1);
          } else { // 1st-level folder.
            bool is_duplicate = false;
            cimglist_for(gmic_1stlevel_names,l)
              if (!std::strcmp(nentry,gmic_1stlevel_names[l].data())) { // Folder name is a duplicate.
                if (gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(tree_view_store),&parent[level],
                                                        gmic_1stlevel_entries[l].data())) {
                  is_duplicate = true;
                  break;
                }
              }

            // Detect if filter is in 'Testing/' (won't be count in number of filters).
            GtkWidget *const markup2ascii = gtk_label_new(0);
            gtk_label_set_markup(GTK_LABEL(markup2ascii),nentry);
            const char *_nentry = gtk_label_get_text(GTK_LABEL(markup2ascii));
            is_testing = !std::strcmp(_nentry,"Testing");

            if (!is_duplicate) {
              gtk_tree_store_append(tree_view_store,&parent[level],level?&parent[level-1]:0);
              gtk_tree_store_set(tree_view_store,&parent[level],0,0,1,nentry,-1);
              const char *treepath = gtk_tree_model_get_string_from_iter(GTK_TREE_MODEL(tree_view_store),
                                                                         &parent[level]);
              CImg<char>::string(nentry).move_to(gmic_1stlevel_names);
              CImg<char>::string(treepath).move_to(gmic_1stlevel_entries);
              unsigned int order = 0;
              for (unsigned int i = 0; i<4; ++i) {
                order<<=8;
                if (*_nentry) order|=(unsigned char)cimg::uncase(*(_nentry++));
              }
            }
            gtk_widget_destroy(markup2ascii);
          }
          ++level;
        }
      } else if (err>=2) { // If entry defines a regular filter.
        cimg::strpare(entry,' ',false,true);
        char *nentry = entry; while (*nentry=='_') { ++nentry; --level; }
        if (level<0) level = 0; else if (level>7) level = 7;
        cimg::strpare(nentry,' ',false,true); cimg::strpare(nentry,'\"',true);
        cimg::strpare(command,' ',false,true);
        cimg::strpare(arguments,' ',false,true);
        if (*nentry) {
          CImg<char>::string(nentry).move_to(gmic_entries);
          CImg<char>::string(command).move_to(gmic_commands);
          CImg<char>::string(arguments).move_to(gmic_arguments);
          if (err>=3) { // Filter has a specified preview command.
            cimg::strpare(preview_command,' ',false,true);
            char *const preview_mode = std::strchr(preview_command,'(');
            double factor = 1;
            char sep = 0;
            if (preview_mode && std::sscanf(preview_mode+1,"%lf%c",&factor,&sep)==2 && factor>=0 && sep==')')
              *preview_mode = 0;
            else factor = -1;
            CImg<char>::string(preview_command).move_to(gmic_preview_commands);
            CImg<double>::vector(factor).move_to(gmic_preview_factors);
          } else {
            CImg<char>::string("_none_").move_to(gmic_preview_commands);
            CImg<double>::vector(-1).move_to(gmic_preview_factors);
          }
          gtk_tree_store_append(tree_view_store,&iter,level?&parent[level-1]:0);
          gtk_tree_store_set(tree_view_store,&iter,0,gmic_entries.size()-1,1,nentry,-1);
          if (!level) {
            GtkWidget *const markup2ascii = gtk_label_new(0);
            gtk_label_set_markup(GTK_LABEL(markup2ascii),nentry);
            const char *_nentry = gtk_label_get_text(GTK_LABEL(markup2ascii));
            unsigned int order = 0;
            for (unsigned int i = 0; i<3; ++i) { order<<=8; if (*_nentry) order|=cimg::uncase(*(_nentry++)); }
            gtk_widget_destroy(markup2ascii);
          }
          if (!is_testing) ++nb_available_filters;  // Count only non-testing filters.
        }
      }
    } else { // Line is the continuation of an entry.
      if (gmic_arguments) {
        if (gmic_arguments.back()) gmic_arguments.back().back() = ' ';
        cimg::strpare(++_line,' ',false,true);
        gmic_arguments.back().append(CImg<char>(_line,std::strlen(_line)+1,1,1,1,true),'x');
      }
    }
  }

  if (try_net_update) gimp_progress_pulse();

  // Load faves.
  char label[256] = { 0 };
  indice_faves = gmic_entries.size();
  if (file_gmic_faves) {
    for (unsigned int line_nb = 1; std::fscanf(file_gmic_faves," %[^\n]",line)==1; ++line_nb) {
      char sep = 0;
      if (std::sscanf(line,"{%255[^}]}{%255[^}]}{%255[^}]}{%255[^}]%c",
                      label,entry,command,preview_command,&sep)==5 && sep=='}') {
        const char *_line = line + 8 + std::strlen(label) + std::strlen(entry) + std::strlen(command) +
          std::strlen(preview_command);
        int entry_found = -1, command_found = -1, preview_found = -1;
        unsigned int filter = 0;
        for (filter = 1; filter<indice_faves; ++filter) {
          const bool
            is_entry_match = !std::strcmp(gmic_entries[filter].data(),entry),
            is_command_match = !std::strcmp(gmic_commands[filter].data(),command),
            is_preview_match = !std::strcmp(gmic_preview_commands[filter].data(),preview_command);
          if (is_entry_match) entry_found = filter;
          if (is_command_match) command_found = filter;
          if (is_preview_match) preview_found = filter;
          if (is_command_match && is_preview_match) break;
        }

        CImg<char>::string(line).move_to(gmic_faves);
        // Get back '}' if necessary.
        for (char *p = std::strchr(label,_rbrace); p; p = std::strchr(p,_rbrace)) *p = '}';
        for (char *p = std::strchr(entry,_rbrace); p; p = std::strchr(p,_rbrace)) *p = '}';

        if (filter>=indice_faves) { // Entry not found.
          CImg<char>::string(label).move_to(gmic_entries);
          CImg<char>::string("_none_").move_to(gmic_commands);
          CImg<char>::string("_none_").move_to(gmic_preview_commands);
          std::sprintf(line,"note = note{\"<span foreground=\"red\"><b>Warning : </b></span>This fave links to an "
                       "unreferenced entry/set of G'MIC commands :\n\n"
                       "   - '<span foreground=\"purple\">%s</span>' as the entry name (%s%s%s%s%s).\n\n"
                       "   - '<span foreground=\"purple\">%s</span>' as the command to compute the filter "
                       "(%s%s%s%s%s).\n\n"
                       "   - '<span foreground=\"purple\">%s</span>' as the command to preview the filter "
                       "(%s%s%s%s%s)."
                       "\"}",
                       entry,
                       entry_found>=0?"recognized, associated to <i>":"<b>not recognized</b>",
                       entry_found>=0?gmic_commands[entry_found].data():"",
                       entry_found>=0?", ":"",
                       entry_found>=0?gmic_preview_commands[entry_found].data():"",
                       entry_found>=0?"</i>":"",
                       command,
                       command_found>=0?"recognized, associated to <i>":"<b>not recognized</b>",
                       command_found>=0?gmic_entries[command_found].data():"",
                       command_found>=0?", ":"",
                       command_found>=0?gmic_preview_commands[command_found].data():"",
                       command_found>=0?"</i>":"",
                       preview_command,
                       preview_found>=0?"recognized, associated to <i>":"<b>not recognized</b>",
                       preview_found>=0?gmic_entries[preview_found].data():"",
                       preview_found>=0?", ":"",
                       preview_found>=0?gmic_commands[preview_found].data():"",
                       preview_found>=0?"</i>":"");

          CImg<char>::string(line).move_to(gmic_arguments);
          CImg<double>::vector(0).move_to(gmic_preview_factors);
          set_filter_nbparams(gmic_entries.size()-1,0);
        } else { // Entry found.
          CImg<char>::string(label).move_to(gmic_entries);
          gmic_commands.insert(gmic_commands[filter]);
          gmic_preview_commands.insert(gmic_preview_commands[filter]);
          gmic_arguments.insert(gmic_arguments[filter]);
          gmic_preview_factors.insert(gmic_preview_factors[filter]);
          unsigned int nbp = 0;
          for (nbp = 0; std::sscanf(_line,"{%65533[^}]%c",arguments,&sep)==2 && sep=='}'; ++nbp) {
            // Get back '}' if necessary.
            for (char *p = std::strchr(arguments,_rbrace); p; p = std::strchr(p,_rbrace)) *p = '}';
            // Get back '\n' if necessary.
            for (char *p = std::strchr(arguments,_newline); p; p = std::strchr(p,_newline)) *p = '\n';
            set_fave_parameter(gmic_entries.size()-1,nbp,arguments);
            _line+=2 + std::strlen(arguments);
          }
          set_filter_nbparams(gmic_entries.size()-1,nbp);
        }
        gtk_tree_store_append(tree_view_store,&iter,&fave_iter);
        gtk_tree_store_set(tree_view_store,&iter,0,gmic_entries.size()-1,1,label,-1);
      } else if (get_verbosity_mode()>1)
        std::fprintf(cimg::output(),
                     "\n[gmic_gimp]./error/ Malformed line %u in fave file '%s' : '%s'.\n",
                     line_nb,filename_gmic_faves,line);
    }
    std::fclose(file_gmic_faves);
  }

  if (try_net_update) {
    gimp_progress_update(1);
    gimp_progress_end();
  }
  return invalid_servers;
}

// 'Convert' a CImg<T> image to a RGB[A] CImg<unsigned char> image, withing the same buffer.
//------------------------------------------------------------------------------------------
template<typename T>
void convert_image2uchar(CImg<T>& img) {
  const unsigned int siz = img.width()*img.height();
  unsigned char *ptrd = (unsigned char*)img.data();
  switch (img.spectrum()) {
  case 1 : {
    const T *ptr0 = img.data(0,0,0,0);
    for (unsigned int i = 0; i<siz; ++i) *(ptrd++) = (unsigned char)*(ptr0++);
  } break;
  case 2 : {
    const T *ptr0 = img.data(0,0,0,0), *ptr1 = img.data(0,0,0,1);
    for (unsigned int i = 0; i<siz; ++i) {
      *(ptrd++) = (unsigned char)*(ptr0++);
      *(ptrd++) = (unsigned char)*(ptr1++);
    }
  } break;
  case 3 : {
    const T *ptr0 = img.data(0,0,0,0), *ptr1 = img.data(0,0,0,1), *ptr2 = img.data(0,0,0,2);
    for (unsigned int i = 0; i<siz; ++i) {
      *(ptrd++) = (unsigned char)*(ptr0++);
      *(ptrd++) = (unsigned char)*(ptr1++);
      *(ptrd++) = (unsigned char)*(ptr2++);
    }
  } break;
  case 4 : {
    const T *ptr0 = img.data(0,0,0,0), *ptr1 = img.data(0,0,0,1),
      *ptr2 = img.data(0,0,0,2), *ptr3 = img.data(0,0,0,3);
    for (unsigned int i = 0; i<siz; ++i) {
      *(ptrd++) = (unsigned char)*(ptr0++);
      *(ptrd++) = (unsigned char)*(ptr1++);
      *(ptrd++) = (unsigned char)*(ptr2++);
      *(ptrd++) = (unsigned char)*(ptr3++);
    }
  } break;
  default: return;
  }
}

// Calibrate any image to fit the number of required channels (GRAY,GRAYA, RGB or RGBA).
//---------------------------------------------------------------------------------------
template<typename T>
void calibrate_image(CImg<T>& img, const unsigned int channels, const bool is_preview) {
  if (!img || !channels) return;
  switch (channels) {

  case 1 : // To GRAY
    switch (img.spectrum()) {
    case 1 : // from GRAY
      break;
    case 2 : // from GRAYA
      if (is_preview) {
        T *ptr_r = img.data(0,0,0,0), *ptr_a = img.data(0,0,0,1);
        cimg_forXY(img,x,y) {
          const unsigned int a = (unsigned int)*(ptr_a++), i = 96 + (((x^y)&8)<<3);
          *ptr_r = (T)((a*(unsigned int)*ptr_r + (255-a)*i)>>8);
          ++ptr_r;
        }
      }
      img.channel(0);
      break;
    case 3 : // from RGB
      img.RGBtoYCbCr().channel(0);
      break;
    case 4 : // from RGBA
      img.get_shared_channels(0,2).RGBtoYCbCr();
      if (is_preview) {
        T *ptr_r = img.data(0,0,0,0), *ptr_a = img.data(0,0,0,3);
        cimg_forXY(img,x,y) {
          const unsigned int a = (unsigned int)*(ptr_a++), i = 96 + (((x^y)&8)<<3);
          *ptr_r = (T)((a*(unsigned int)*ptr_r + (255-a)*i)>>8);
          ++ptr_r;
        }
      }
      img.channel(0);
      break;
    default : // from multi-channel (>4)
      img.channel(0);
    } break;

  case 2: // To GRAYA
    switch (img.spectrum()) {
    case 1: // from GRAY
      img.resize(-100,-100,1,2,0).get_shared_channel(1).fill(255);
      break;
    case 2: // from GRAYA
      break;
    case 3: // from RGB
      img.RGBtoYCbCr().channels(0,1).get_shared_channel(1).fill(255);
      break;
    case 4: // from RGBA
      img.get_shared_channels(0,2).RGBtoYCbCr();
      img.get_shared_channel(1) = img.get_shared_channel(3);
      img.channels(0,1);
      break;
    default: // from multi-channel (>4)
      img.channels(0,1);
    } break;

  case 3: // to RGB
    switch (img.spectrum()) {
    case 1: // from GRAY
      img.resize(-100,-100,1,3);
      break;
    case 2: // from GRAYA
      if (is_preview) {
        T *ptr_r = img.data(0,0,0,0), *ptr_a = img.data(0,0,0,1);
        cimg_forXY(img,x,y) {
          const unsigned int a = (unsigned int)*(ptr_a++), i = 96 + (((x^y)&8)<<3);
          *ptr_r = (T)((a*(unsigned int)*ptr_r + (255-a)*i)>>8);
          ++ptr_r;
        }
      }
      img.channel(0).resize(-100,-100,1,3);
      break;
    case 3: // from RGB
      break;
    case 4: // from RGBA
      if (is_preview) {
        T *ptr_r = img.data(0,0,0,0), *ptr_g = img.data(0,0,0,1),
          *ptr_b = img.data(0,0,0,2), *ptr_a = img.data(0,0,0,3);
        cimg_forXY(img,x,y) {
          const unsigned int a = (unsigned int)*(ptr_a++), i = 96 + (((x^y)&8)<<3);
          *ptr_r = (T)((a*(unsigned int)*ptr_r + (255-a)*i)>>8);
          *ptr_g = (T)((a*(unsigned int)*ptr_g + (255-a)*i)>>8);
          *ptr_b = (T)((a*(unsigned int)*ptr_b + (255-a)*i)>>8);
          ++ptr_r; ++ptr_g; ++ptr_b;
        }
      }
      img.channels(0,2);
      break;
    default: // from multi-channel (>4)
      img.channels(0,2);
    } break;

  case 4: // to RGBA
    switch (img.spectrum()) {
    case 1: // from GRAY
      img.resize(-100,-100,1,4).get_shared_channel(3).fill(255);
      break;
    case 2: // from GRAYA
      img.resize(-100,-100,1,4,0);
      img.get_shared_channel(3) = img.get_shared_channel(1);
      img.get_shared_channel(1) = img.get_shared_channel(0);
      img.get_shared_channel(2) = img.get_shared_channel(0);
      break;
    case 3: // from RGB
      img.resize(-100,-100,1,4,0).get_shared_channel(3).fill(255);
      break;
    case 4: // from RGBA
      break;
    default: // from multi-channel (>4)
      img.channels(0,3);
    } break;
  }
}

// Get the input layers of a GIMP image as a list of CImg<T>.
//-----------------------------------------------------------
template<typename T>
CImg<int> get_input_layers(CImgList<T>& images) {

  // Retrieve the list of desired layers.
  int
    nb_layers = 0,
    *layers = gimp_image_get_layers(image_id,&nb_layers),
    active_layer = gimp_image_get_active_layer(image_id);
  CImg<int> input_layers;
  const unsigned int input_mode = get_input_mode();
  switch (input_mode) {
  case 0 : // Input none
    break;
  case 1 : // Input active layer
    input_layers = CImg<int>::vector(active_layer);
    break;
  case 2 : case 9 : // Input all image layers
    input_layers = CImg<int>(layers,1,nb_layers);
    if (input_mode==9) input_layers.mirror('y');
    break;
  case 3 : { // Input active & below layers
    int i = 0; for (i = 0; i<nb_layers; ++i) if (layers[i]==active_layer) break;
    if (i<nb_layers-1) input_layers = CImg<int>::vector(active_layer,layers[i+1]);
    else input_layers = CImg<int>::vector(active_layer);
  } break;
  case 4 : { // Input active & above layers
    int i = 0; for (i = 0; i<nb_layers; ++i) if (layers[i]==active_layer) break;
    if (i>0) input_layers = CImg<int>::vector(active_layer,layers[i-1]);
    else input_layers = CImg<int>::vector(active_layer);
  } break;
  case 5 : case 7 : { // Input all visible image layers
    CImgList<int> visible_layers;
    for (int i = 0; i<nb_layers; ++i)
      if (gimp_drawable_get_visible(layers[i])) CImg<int>::vector(layers[i]).move_to(visible_layers);
    input_layers = visible_layers>'y';
    if (input_mode==7) input_layers.mirror('y');
  } break;
  default : { // Input all invisible image layers
    CImgList<int> invisible_layers;
    for (int i = 0; i<nb_layers; ++i)
      if (!gimp_drawable_get_visible(layers[i])) CImg<int>::vector(layers[i]).move_to(invisible_layers);
    input_layers = invisible_layers>'y';
    if (input_mode==8) input_layers.mirror('y');
  } break;
  }

  // Read input image data into a CImgList<T>.
  images.assign(input_layers.height());
  GimpPixelRgn region;
  gint x1, y1, x2, y2;
  cimglist_for(images,l) {
    GimpDrawable *drawable = gimp_drawable_get(input_layers[l]);
    if (!gmic_drawable_is_valid(drawable)) continue;
    gimp_drawable_mask_bounds(drawable->drawable_id,&x1,&y1,&x2,&y2);
    const int channels = drawable->bpp;
    gimp_pixel_rgn_init(&region,drawable,x1,y1,x2-x1,y2-y1,false,false);
    guchar *const row = g_new(guchar,(x2-x1)*channels), *ptrs = 0;
    CImg<T> img(x2-x1,y2-y1,1,channels);
    switch (channels) {
    case 1 : {
      T *ptr_r = img.data(0,0,0,0);
      cimg_forY(img,y) {
        gimp_pixel_rgn_get_row(&region,ptrs=row,x1,y1+y,img.width());
        cimg_forX(img,x) *(ptr_r++) = (T)*(ptrs++);
      }
    } break;
    case 2 : {
      T *ptr_r = img.data(0,0,0,0), *ptr_g = img.data(0,0,0,1);
      cimg_forY(img,y) {
        gimp_pixel_rgn_get_row(&region,ptrs=row,x1,y1+y,img.width());
        cimg_forX(img,x) {
          *(ptr_r++) = (T)*(ptrs++);
          *(ptr_g++) = (T)*(ptrs++);
        }
      }
    } break;
    case 3 : {
      T *ptr_r = img.data(0,0,0,0), *ptr_g = img.data(0,0,0,1), *ptr_b = img.data(0,0,0,2);
      cimg_forY(img,y) {
        gimp_pixel_rgn_get_row(&region,ptrs=row,x1,y1+y,img.width());
        cimg_forX(img,x) {
          *(ptr_r++) = (T)*(ptrs++);
          *(ptr_g++) = (T)*(ptrs++);
          *(ptr_b++) = (T)*(ptrs++);
        }
      }
    } break;
    case 4 : {
      T *ptr_r = img.data(0,0,0,0), *ptr_g = img.data(0,0,0,1),
        *ptr_b = img.data(0,0,0,2), *ptr_a = img.data(0,0,0,3);
      cimg_forY(img,y) {
        gimp_pixel_rgn_get_row(&region,ptrs=row,x1,y1+y,img.width());
        cimg_forX(img,x) {
          *(ptr_r++) = (T)*(ptrs++);
          *(ptr_g++) = (T)*(ptrs++);
          *(ptr_b++) = (T)*(ptrs++);
          *(ptr_a++) = (T)*(ptrs++);
        }
      }
    } break;
    }
    g_free(row);
    gimp_drawable_detach(drawable);
    img.move_to(images[l]);
  }
  return input_layers;
}

// Return the G'MIC command line needed to run the selected filter.
//-----------------------------------------------------------------
const char* get_commands_line(const bool is_preview) {
  const unsigned int
    filter = get_current_filter(),
    nbparams = get_filter_nbparams(filter);
  if (!filter) return 0;

  static CImg<char> res;
  CImgList<char> lres;
  switch (get_verbosity_mode()) {
  case 0: case 1: case 2: case 3: CImg<char>("-v -99 -",8).move_to(lres); break;  // Quiet or Verbose.
  case 4: case 5 : CImg<char>("-",1).move_to(lres); break;                // Very verbose.
  default: CImg<char>("-debug -",8).move_to(lres);                        // Debug.
  }
  const CImg<char> &command_item = (is_preview?gmic_preview_commands[filter]:gmic_commands[filter]);
  if (command_item) {
    lres.insert(command_item);
    if (nbparams) {
      lres[1].back() = ' ';
      for (unsigned int p = 0; p<nbparams; ++p) {
        const char *ss = get_filter_parameter(filter,p);
        const unsigned int l = std::strlen(ss);
        CImg<char> nparam(l+1);
        *nparam = 0;
        char *sd = nparam.data();
        if (l>=2 && ss[0]=='\"' && ss[l-1]=='\"') { // Replace special characters in a string or a filename.
          ++ss; *(sd++) = '\"';
          for (unsigned int i = 1; i<l-1; ++i, ++ss) { const char c = *ss; *(sd++) = c=='\"'?_dquote:c; }
          *(sd++) = '\"'; *(sd++) = 0;
          nparam.move_to(lres);
        } else CImg<char>(ss,l+1).move_to(lres);
        lres.back().back() = ',';
      }
    }
    (res=lres>'x').back() = 0;
  }
  return res.data();
}

// Set defaut zoom factor for preview of the current filter.
//----------------------------------------------------------
void set_preview_factor() {
  const unsigned int filter = get_current_filter();
  if (filter && gmic_preview_factors[filter] && GIMP_IS_PREVIEW(gui_preview)) {
    double factor = gmic_preview_factors(filter,0);
    if (factor>=0) {
      if (!factor) { // Compute factor so that 1:1 preview of the image is displayed.
        int _pw = 0, _ph = 0;
        gimp_preview_get_size(GIMP_PREVIEW(gui_preview),&_pw,&_ph);
        const float
          pw = (float)_pw,
          ph = (float)_ph,
          dw = (float)drawable_preview->width,
          dh = (float)drawable_preview->height;
        factor = std::sqrt((dw*dw+dh*dh)/(pw*pw+ph*ph));
      }
      gimp_zoom_model_zoom(gimp_zoom_preview_get_model(GIMP_ZOOM_PREVIEW(gui_preview)),GIMP_ZOOM_TO,factor);
    }
  }
}

// Handle GUI event functions.
//----------------------------
void create_parameters_gui(const bool);
void process_image(const char *const, const bool is_apply);
void process_preview();

// Secure function for invalidate preview.
void _gimp_preview_invalidate() {
  computed_preview.assign();
  if (GIMP_IS_PREVIEW(gui_preview) && gmic_drawable_is_valid(drawable_preview))
    gimp_preview_invalidate(GIMP_PREVIEW(gui_preview));
  else {
    if (GTK_IS_WIDGET(gui_preview)) gtk_widget_destroy(gui_preview);
    drawable_preview = gimp_drawable_get(gimp_image_get_active_drawable(image_id));
    gui_preview = gimp_zoom_preview_new(drawable_preview);
    gtk_widget_show(gui_preview);
    gtk_box_pack_end(GTK_BOX(left_pane),gui_preview,true,true,0);
    g_signal_connect(gui_preview,"invalidated",G_CALLBACK(process_preview),0);
  }
}

// Resize preview widget.
void resize_preview(const unsigned int size=2) {
  char tmp[256] = { 0 };
  std::sprintf(tmp,
               "style \"gimp-large-preview\"\n"
               "{\n"
               "  GimpPreview::size = %u\n"
               "}\n"
               "class \"GimpPreview\" style \"gimp-large-preview\"",
               200+size*120);
  gtk_rc_parse_string(tmp);
  if (GIMP_IS_PREVIEW(gui_preview)) {
    gtk_widget_destroy(gui_preview);
    gui_preview=0;
    _gimp_preview_invalidate();
  }
}

// Reset value of all button parameters for current filter.
void reset_button_parameters() {
  const unsigned int filter = get_current_filter();
  cimglist_for(gmic_button_parameters,l) set_filter_parameter(filter,gmic_button_parameters(l,0),"0");
}

// Handle preview resize event.
void on_dialog_resized() {
  reset_button_parameters();
  static int opw = 0, oph = 0;
  int pw = 0, ph = 0;
  if (GIMP_IS_PREVIEW(gui_preview)) {
    gimp_preview_get_size(GIMP_PREVIEW(gui_preview),&pw,&ph);
    if (!opw || !oph) { opw = pw; oph = ph; computed_preview.assign(); } else {
      if (pw!=opw || ph!=oph) { set_preview_factor(); opw = pw; oph = ph; computed_preview.assign(); }
    }
  }
}

// Handle widgets events related to parameter changes.
void on_float_parameter_changed(GtkAdjustment *const adjustment, const void *const event_infos) {
  reset_button_parameters();
  double value = 0;
  gimp_double_adjustment_update(adjustment,&value);
  char s_value[32] = { 0 };
  cimg_snprintf(s_value,sizeof(s_value),"%g",value);
  set_filter_parameter(get_current_filter(),*(int*)event_infos,s_value);
  _create_dialog_gui = true;
}

void on_int_parameter_changed(GtkAdjustment *const adjustment, const void *const event_infos) {
  reset_button_parameters();
  int value = 0;
  gimp_int_adjustment_update(adjustment,&value);
  char s_value[32] = { 0 };
  cimg_snprintf(s_value,sizeof(s_value),"%d",value);
  set_filter_parameter(get_current_filter(),*(int*)event_infos,s_value);
  _create_dialog_gui = true;
}

void on_bool_parameter_changed(GtkCheckButton *const checkbutton, const void *const event_infos) {
  reset_button_parameters();
  int value = 0;
  g_object_get(checkbutton,"active",&value,NULL);
  char s_value[4] = { 0 };
  cimg_snprintf(s_value,sizeof(s_value),"%d",value?1:0);
  set_filter_parameter(get_current_filter(),*(int*)event_infos,s_value);
  _create_dialog_gui = true;
}

void on_button_parameter_clicked(GtkCheckButton *const button, const void *const event_infos) {
  reset_button_parameters();
  cimg::unused(button);
  const unsigned int filter = get_current_filter();
  set_filter_parameter(filter,*(int*)event_infos,"1");
  _create_dialog_gui = true;
}

void on_list_parameter_changed(GtkComboBox *const combobox, const void *const event_infos) {
  reset_button_parameters();
  int value = 0;
  g_object_get(combobox,"active",&value,NULL);
  char s_value[32] = { 0 };
  cimg_snprintf(s_value,sizeof(s_value),"%d",value);
  set_filter_parameter(get_current_filter(),*(int*)event_infos,s_value);
  _create_dialog_gui = true;
}

void on_text_parameter_changed(const void *const event_infos) {
  reset_button_parameters();
  GtkWidget *entry = *((GtkWidget**)event_infos+1);
  const char *const s_value = gtk_entry_get_text(GTK_ENTRY(entry));
  CImg<char> s_param;
  if (s_value && *s_value) {
    CImg<char> _s_value = CImg<char>::string(s_value);
    cimg_for(_s_value,ptr,char) if (*ptr=='\"') *ptr = _dquote;
    s_param.assign(_s_value._width+2);
    cimg_snprintf(s_param,s_param._width,"\"%s\"",_s_value.data());
  } else std::strcpy(s_param.assign(3),"\"\"");
  set_filter_parameter(get_current_filter(),*(int*)event_infos,s_param);
  _create_dialog_gui = true;
}

void on_multitext_parameter_changed(const void *const event_infos) {
  reset_button_parameters();
  GtkWidget *text_view = *((GtkWidget**)event_infos+1);
  GtkTextBuffer *const text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
  GtkTextIter it_start, it_end;
  gtk_text_buffer_get_bounds(text_buffer,&it_start,&it_end);
  const char *const s_value = gtk_text_buffer_get_text(text_buffer,&it_start,&it_end,false);
  CImg<char> s_param;
  if (s_value && *s_value) {
    CImg<char> _s_value = CImg<char>::string(s_value);
    cimg_for(_s_value,ptr,char) if (*ptr=='\"') *ptr = _dquote;
    s_param.assign(_s_value._width+2);
    cimg_snprintf(s_param,s_param._width,"\"%s\"",_s_value.data());
  } else std::strcpy(s_param.assign(3),"\"\"");
  set_filter_parameter(get_current_filter(),*(int*)event_infos,s_param);
  _create_dialog_gui = true;
}

void on_file_parameter_changed(GtkFileChooser *const file_chooser, const void *const event_infos) {
  reset_button_parameters();
  const char
    *const _s_value = gtk_file_chooser_get_filename(file_chooser),
    *const s_value = _s_value?_s_value:"",
    *const _o_value = get_filter_parameter(get_current_filter(),*(int*)event_infos);
  CImg<char> o_value = CImg<char>::string(_o_value);
  cimg::strpare(o_value,'\"',true);
  const bool
    is_same_file = !std::strcmp(cimg::basename(s_value),cimg::basename(o_value)),
    is_silent_argument = (bool)*((void**)event_infos+1);
  CImg<char> s_param;
  if (s_value && *s_value) {
    s_param.assign(std::strlen(s_value)+3);
    cimg_snprintf(s_param,s_param._width,"\"%s\"",s_value);
  } else std::strcpy(s_param.assign(3),"\"\"");
  set_filter_parameter(get_current_filter(),*(int*)event_infos,s_param);
  if (!is_same_file && !is_silent_argument)
    _gimp_preview_invalidate(); // Invalidate preview only if filename has changed.
  _create_dialog_gui = true;
}

void on_color_parameter_changed(GtkColorButton *const color_button, const void *const event_infos) {
  reset_button_parameters();
  GdkColor color;
  gtk_color_button_get_color(color_button,&color);
  char s_value[256] = { 0 };
  if (gtk_color_button_get_use_alpha(color_button))
    cimg_snprintf(s_value,sizeof(s_value),"%d,%d,%d,%d",
                  color.red/257,color.green/257,color.blue/257,gtk_color_button_get_alpha(color_button)/257);
  else cimg_snprintf(s_value,sizeof(s_value),"%d,%d,%d",
                     color.red/257,color.green/257,color.blue/257);
  set_filter_parameter(get_current_filter(),*(int*)event_infos,s_value);
  _create_dialog_gui = true;
}

// Handle responses to the dialog window buttons.
void on_dialog_input_mode_changed(GtkComboBox *const combobox) {
  reset_button_parameters();
  int value = 0;
  g_object_get(combobox,"active",&value,NULL);
  if (value<2) gtk_combo_box_set_active(combobox,value=3);
  set_input_mode((unsigned int)value);
  _gimp_preview_invalidate();
}

void on_dialog_output_mode_changed(GtkComboBox *const combobox) {
  reset_button_parameters();
  int value = 0;
  g_object_get(combobox,"active",&value,NULL);
  if (value<2) gtk_combo_box_set_active(combobox,value=2);
  set_output_mode((unsigned int)value);
}

void on_dialog_verbosity_mode_changed(GtkComboBox *const combobox) {
  reset_button_parameters();
  int value = 0;
  g_object_get(combobox,"active",&value,NULL);
  if (value<2) gtk_combo_box_set_active(combobox,value=2);
  set_verbosity_mode((unsigned int)value);
  set_logfile();
  if (value>3) _gimp_preview_invalidate();
}

void on_dialog_preview_mode_changed(GtkComboBox *const combobox) {
  reset_button_parameters();
  int value = 0;
  g_object_get(combobox,"active",&value,NULL);
  if (value<2) gtk_combo_box_set_active(combobox,value=2);
  set_preview_mode((unsigned int)value);
  _gimp_preview_invalidate();
}

void on_dialog_preview_size_changed(GtkComboBox *const combobox) {
  reset_button_parameters();
  int value = 0;
  g_object_get(combobox,"active",&value,NULL);
  if (value<2) gtk_combo_box_set_active(combobox,value=2);
  set_preview_size((unsigned int)value);
  resize_preview(value-2);
}

void on_dialog_maximize_button_clicked(GtkButton *const button) {
  reset_button_parameters();
  static int ow = 0, oh = 0;
  GdkScreen* screen = gtk_window_get_screen(GTK_WINDOW(dialog_window));
  int
    width = gdk_screen_get_width(screen),
    height = gdk_screen_get_height(screen);
  if (width>0 && height>0 && !ow && !oh) {
    gtk_window_get_size(GTK_WINDOW(dialog_window),&ow,&oh);
#if cimg_OS==1
    height-=64;
#elif cimg_OS==2
    // Subtract the height of the taskbar on windows.
    RECT rect;
    HWND taskBar = FindWindow("Shell_traywnd",NULL);
    if (taskBar && GetWindowRect(taskBar,&rect)) height-=rect.bottom - rect.top;
#endif
    if (height>0) gtk_window_resize(GTK_WINDOW(dialog_window),width,height);
    gtk_window_move(GTK_WINDOW(dialog_window),0,0);
    gtk_button_set_label(button,GTK_STOCK_LEAVE_FULLSCREEN);
  } else if (ow>0 && oh>0) {
    gtk_window_resize(GTK_WINDOW(dialog_window),ow,oh);
    ow = oh = 0;
    gtk_button_set_label(button,GTK_STOCK_FULLSCREEN);
  }
  _gimp_preview_invalidate();
}

void on_dialog_reset_clicked() {
  create_parameters_gui(true);
  _create_dialog_gui = true;
  _gimp_preview_invalidate();
}

void on_dialog_cancel_clicked() {
  reset_button_parameters();
  _create_dialog_gui = false;
  gtk_main_quit();
}

void on_dialog_apply_clicked() {
  process_image(0,true);
  _create_dialog_gui = false;
  _gimp_preview_invalidate();
  const char *const commands_line = get_commands_line(false);
  if (commands_line) { // Remember command line for the next use of the filter.
    char s_tmp[48] = { 0 };
    cimg_snprintf(s_tmp,sizeof(s_tmp),"gmic_commands_line%u",get_current_filter());
    gimp_set_data(s_tmp,commands_line,std::strlen(commands_line)+1);
  }
}

void on_dialog_net_update_toggled(GtkToggleButton *const toggle_button) {
  reset_button_parameters();
  set_net_update(gtk_toggle_button_get_active(toggle_button));
}

void on_dialog_tree_mode_clicked(GtkWidget *const tree_view) {
  reset_button_parameters();
  set_tree_mode(!get_tree_mode());
  flush_tree_view(tree_view);
}

void on_dialog_add_fave_clicked(GtkWidget *const tree_view) {
  reset_button_parameters();
  const unsigned int filter = get_current_filter();
  gtk_widget_hide(relabel_hbox);
  gtk_widget_hide(fave_delete_button);
  if (filter) {
    char filename[1024] = { 0 };
    cimg_snprintf(filename,sizeof(filename),"%s%c%sgmic_faves",
                  get_conf_path(),cimg_file_separator,_gmic_file_prefix);
    std::FILE *file = std::fopen(filename,"wb");
    if (file) {
      char basename[256] = { 0 }, label[256] = { 0 };
      unsigned int ind = 0;
      std::strcpy(basename,gmic_entries[filter].data());
      char *last_space = 0;
      for (char *p = basename; p; p = std::strchr(p+1,' ')) last_space = p;
      if (last_space>basename) {
        char sep = 0, end = 0;
        if (std::sscanf(last_space+1,"(%u%c%c",&ind,&sep,&end)==2 && sep==')') *last_space = 0;
      }
      std::strcpy(label,basename);
      cimglist_for(gmic_faves,l) {
        std::fprintf(file,"%s\n",gmic_faves[l].data());
        if (!std::strcmp(label,gmic_entries[indice_faves+l].data()))
          std::sprintf(label,"%s (%u)",basename,++ind);
      }
      CImg<char> entry = gmic_entries[filter];
      for (char *p = std::strchr(label,'}'); p; p = std::strchr(p,'}')) *p = _rbrace;  // Convert '}' if necessary.
      for (char *p = std::strchr(entry,'}'); p; p = std::strchr(p,'}')) *p = _rbrace;
      std::fprintf(file,"{%s}{%s}{%s}{%s}",
                   label,
                   entry.data(),
                   gmic_commands[filter].data(),
                   gmic_preview_commands[filter].data());
      const unsigned int nbp = get_filter_nbparams(filter);
      for (unsigned int n = 0; n<nbp; ++n) {
        char *const param = get_filter_parameter(filter,n);
        set_filter_parameter(gmic_entries.size(),n,param);
        for (char *p = std::strchr(param,'}'); p; p = std::strchr(p,'}')) *p = _rbrace; // Convert '}' if necessary.
        for (char *p = std::strchr(param,'\n'); p; p = std::strchr(p,'\n')) *p = _newline; // Convert '\n' if necessary.
        std::fprintf(file,"{%s}",param);
      }
      std::fputc('\n',file);
      std::fclose(file);
      update_filters(false);
    } else if (get_verbosity_mode()>1)
      std::fprintf(cimg::output(),
                   "\n[gmic_gimp]./error/ Unable to write fave file '%s'.\n",
                   filename);
  }
  gtk_tree_view_set_model(GTK_TREE_VIEW(tree_view),GTK_TREE_MODEL(tree_view_store));
  gimp_set_data("gmic_current_treepath","0",2);
  set_current_filter(0);
  flush_tree_view(tree_view);
  GtkTreePath *path = gtk_tree_path_new_from_string("0");
  gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(tree_view),path,NULL,FALSE,0,0);
  GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view));
  gtk_tree_selection_select_path(selection,path);
  gtk_tree_path_free(path);
  create_parameters_gui(false);
  _gimp_preview_invalidate();
}

void on_dialog_remove_fave_clicked(GtkWidget *const tree_view) {
  reset_button_parameters();
  const unsigned int filter = get_current_filter();
  gtk_widget_hide(relabel_hbox);
  gtk_widget_hide(fave_delete_button);
  if (filter) {
    char filename[1024] = { 0 };
    cimg_snprintf(filename,sizeof(filename),"%s%c%sgmic_faves",
                  get_conf_path(),cimg_file_separator,_gmic_file_prefix);
    std::FILE *file = std::fopen(filename,"wb");
    if (file) {
      const unsigned int _filter = filter - indice_faves;
      if (gmic_faves.size()==1) { std::fclose(file); std::remove(filename); }
      else {
        cimglist_for(gmic_faves,l) if (l!=(int)_filter) std::fprintf(file,"%s\n",gmic_faves[l].data());
        std::fclose(file);
        for (unsigned int i = filter; i<gmic_entries.size()-1; ++i) { // Shift current parameters.
          const unsigned int nbp = get_filter_nbparams(i+1);
          for (unsigned int n = 0; n<nbp; ++n)
            set_filter_parameter(i,n,get_filter_parameter(i+1,n));
        }
      }
      update_filters(false);
    } else if (get_verbosity_mode()>1)
      std::fprintf(cimg::output(),
                   "\n[gmic_gimp]./error/ Unable to write fave file '%s'.\n",
                   filename);
  }
  gtk_tree_view_set_model(GTK_TREE_VIEW(tree_view),GTK_TREE_MODEL(tree_view_store));
  gimp_set_data("gmic_current_treepath","0",2);
  set_current_filter(0);
  flush_tree_view(tree_view);
  GtkTreePath *path = gtk_tree_path_new_from_string("0");
  gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(tree_view),path,NULL,FALSE,0,0);
  GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view));
  gtk_tree_selection_select_path(selection,path);
  gtk_tree_path_free(path);
  create_parameters_gui(false);
  _gimp_preview_invalidate();
}

void on_dialog_refresh_clicked(GtkWidget *const tree_view) {
  reset_button_parameters();
  gtk_widget_hide(relabel_hbox);
  gtk_widget_hide(fave_delete_button);
  CImgList<char> invalid_servers = update_filters(get_net_update());
  if (invalid_servers) {
    if (get_verbosity_mode()>1) cimglist_for(invalid_servers,l) {
        std::fprintf(cimg::output(),
                     "\n[gmic_gimp]./update/ Unable to reach filters source '%s'.\n",
                     invalid_servers[l].data());
        std::fflush(cimg::output());
      }
    CImg<char>::string(t(0)).move_to(invalid_servers,0);
    cimglist_for(invalid_servers,l) {
      CImg<char>& server = invalid_servers[l];
      if (l) {
        server.resize(2+server.width(),1,1,1,0,0,1);
        server[0] = '*';
        server[1] = ' ';
      }
      if (l!=invalid_servers.width()-1) server.back() = '\n';
    }
    const CImg<char> error_message = invalid_servers>'x';

    GtkWidget *const
      message = gtk_message_dialog_new_with_markup(0,GTK_DIALOG_MODAL,GTK_MESSAGE_ERROR,GTK_BUTTONS_OK,
                                                   "%s",error_message.data());
    gtk_widget_show(message);
    gtk_dialog_run(GTK_DIALOG(message));
    gtk_widget_destroy(message);
  } else reset_filters_parameters();
  gtk_tree_view_set_model(GTK_TREE_VIEW(tree_view),GTK_TREE_MODEL(tree_view_store));
  flush_tree_view(tree_view);
  create_parameters_gui(true);
  _gimp_preview_invalidate();
}

void on_filter_selected(GtkWidget *const tree_view) {
  reset_button_parameters();
  GtkTreeSelection *const selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view));
  GtkTreeIter iter;
  GtkTreeModel *model;
  unsigned int filter = 0;
  if (gtk_tree_selection_get_selected(selection,&model,&iter)) {
    gtk_tree_model_get(model,&iter,0,&filter,-1);
    const char *const treepath = gtk_tree_model_get_string_from_iter(GTK_TREE_MODEL(tree_view_store),&iter);
    gimp_set_data("gmic_current_treepath",treepath,std::strlen(treepath)+1);
  }
  if (filter!=get_current_filter()) {
    gtk_widget_hide(relabel_hbox);
    gtk_widget_hide(fave_delete_button);
    set_current_filter(filter);
    create_parameters_gui(false);
    _create_dialog_gui = true;
    _gimp_preview_invalidate();
  }
}

void _on_filter_doubleclicked(GtkWidget *const entry) {
  reset_button_parameters();
  const unsigned int filter = get_current_filter();
  cimg::unused(entry);
  gtk_widget_hide(relabel_hbox);
  gtk_widget_hide(fave_delete_button);
  if (filter>=indice_faves) {
    const char *const __label = gtk_entry_get_text(GTK_ENTRY(relabel_entry));
    char *const label = g_markup_escape_text(__label,std::strlen(__label));
    if (*label) {
      char filename[1024] = { 0 };
      cimg_snprintf(filename,sizeof(filename),"%s%c%sgmic_faves",
                    get_conf_path(),cimg_file_separator,_gmic_file_prefix);
      std::FILE *file = std::fopen(filename,"wb");
      if (file) {
        const unsigned int _filter = filter - indice_faves;
        cimglist_for(gmic_faves,l) if (l!=(int)_filter) std::fprintf(file,"%s\n",gmic_faves[l].data());
        else {
          CImg<char> _label = CImg<char>::string(label);
          for (char *p = std::strchr(_label,'}'); p; p = std::strchr(p,'}')) *p = _rbrace; // Convert '}' if necessary.
          std::fprintf(file,"{%s%s\n",_label.data(),std::strchr(gmic_faves[l].data(),'}'));
        }
        std::fclose(file);
        update_filters(false);
      } else if (get_verbosity_mode()>1)
        std::fprintf(cimg::output(),
                     "\n[gmic_gimp]./error/ Unable to write fave file '%s'.\n",
                     filename);
      gtk_tree_view_set_model(GTK_TREE_VIEW(tree_view),GTK_TREE_MODEL(tree_view_store));
      gimp_set_data("gmic_current_treepath","0",2);
      set_current_filter(0);
      flush_tree_view(tree_view);
      GtkTreePath *path = gtk_tree_path_new_from_string("0");
      gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(tree_view),path,NULL,FALSE,0,0);
      GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view));
      gtk_tree_selection_select_path(selection,path);
      gtk_tree_path_free(path);
      create_parameters_gui(false);
    }
    g_free(label);
  }
}

void on_filter_doubleclicked(GtkWidget *const tree_view) {
  reset_button_parameters();
  const unsigned int filter = get_current_filter();
  if (!filter) { // Expand/collapse folder.
    char current_path[64] = { 0 };
    gimp_get_data("gmic_current_treepath",current_path);
    if (*current_path) {
      GtkTreePath *path = gtk_tree_path_new_from_string(current_path);
      const bool is_expanded = gtk_tree_view_row_expanded(GTK_TREE_VIEW(tree_view),path);
      if (is_expanded) gtk_tree_view_collapse_row(GTK_TREE_VIEW(tree_view),path);
      else gtk_tree_view_expand_row(GTK_TREE_VIEW(tree_view),path,false);
      gtk_tree_path_free(path);
    }
  } else if (filter>=indice_faves) { // Rename fave filter.
    is_block_preview = true;
    GtkWidget *const markup2ascii = gtk_label_new(0);
    gtk_label_set_markup(GTK_LABEL(markup2ascii),gmic_entries[filter].data());
    gtk_entry_set_text(GTK_ENTRY(relabel_entry),gtk_label_get_text(GTK_LABEL(markup2ascii)));
    gtk_widget_destroy(markup2ascii);
    gtk_widget_show(relabel_hbox);
    gtk_widget_grab_focus(relabel_entry);
  }
}

// Process image data with the G'MIC interpreter.
//-----------------------------------------------

// This structure stores the arguments required by the processing thread.
struct st_process_thread {
  CImgList<gmic_pixel_type> images;
  CImgList<char> images_names;
  CImg<char> error_message, status;
  bool is_thread, is_preview;
  unsigned int verbosity_mode;
  const char *commands_line;
  float progress;
#if !defined(__MACOSX__) && !defined(__APPLE__)
  pthread_mutex_t is_running, wait_lock;
  pthread_cond_t wait_cond;
  pthread_t thread;
#endif
};

// Thread that runs the G'MIC interpreter.
void *process_thread(void *arg) {
  st_process_thread &spt = *(st_process_thread*)arg;
#if !defined(__MACOSX__) && !defined(__APPLE__)
  if (spt.is_thread) {
    pthread_mutex_lock(&spt.is_running);
    pthread_mutex_lock(&spt.wait_lock);
    pthread_cond_signal(&spt.wait_cond);
    pthread_mutex_unlock(&spt.wait_lock);
  }
#endif
  try {
    if (spt.verbosity_mode>1) {
      CImg<char> cl = CImg<char>::string(spt.commands_line);
      std::fprintf(cimg::output(),
                   "\n[gmic_gimp]./%s/ %s\n",
                   spt.is_preview?"preview":"apply",
                   cl.data());
      std::fflush(cimg::output());
    }
    gmic gmic_instance(spt.commands_line,spt.images,spt.images_names,gmic_additional_commands,true,&spt.progress);
    gmic_instance.status.move_to(spt.status);
  } catch (gmic_exception &e) {
    spt.images.assign();
    spt.images_names.assign();
    CImg<char>::string(e.what()).move_to(spt.error_message);
    if (spt.verbosity_mode>1) {
      std::fprintf(cimg::output(),
                   "\n[gmic_gimp]./error/ %s\n",
                   spt.error_message.data());
      std::fflush(cimg::output());
    }
  }
#if !defined(__MACOSX__) && !defined(__APPLE__)
  if (spt.is_thread) {
    pthread_mutex_unlock(&spt.is_running);
    pthread_exit(0);
  }
#endif
  return 0;
}

// Process the selected image/layers.
//------------------------------------
void process_image(const char *const commands_line, const bool is_apply) {
  if (!gimp_image_is_valid(image_id)) return;
  const unsigned int
    filter = get_current_filter(),
    _output_mode = get_output_mode(),
    output_mode = get_input_mode()==0?cimg::max(1U,_output_mode):_output_mode,
    verbosity_mode = get_verbosity_mode();

  if (!commands_line && !filter) return;
  bool update_parameters = false;
  const char *const _commands_line = commands_line?commands_line:get_commands_line(false);
  if (!_commands_line || std::strstr(_commands_line,"-_none_")) return;

  CImg<char> new_label(256);
  *new_label = 0;
  if (run_mode!=GIMP_RUN_NONINTERACTIVE) {
    GtkWidget *const markup2ascii = gtk_label_new(0);
    gtk_label_set_markup(GTK_LABEL(markup2ascii),gmic_entries[filter].data());
    gimp_progress_init_printf(" G'MIC : %s...",gtk_label_get_text(GTK_LABEL(markup2ascii)));
    const char *const cl = _commands_line +
      (!std::strncmp(_commands_line,"-v -99 ",7) || !std::strncmp(_commands_line,"-debug ",7)?7:0);
    cimg_snprintf(new_label,new_label.width(),"[G'MIC] %s : %s",gtk_label_get_text(GTK_LABEL(markup2ascii)),cl);
    gmic_ellipsize(new_label,240,false);
    gtk_widget_destroy(markup2ascii);
  } else {
    cimg_snprintf(new_label,new_label.width(),"G'MIC : %s...",_commands_line);
    gmic_ellipsize(new_label,240,false);
    gimp_progress_init_printf("%s",new_label.data());
    cimg_snprintf(new_label,new_label.width(),"[G'MIC] : %s",_commands_line);
    gmic_ellipsize(new_label,240,false);
  }

  // Get input layers for the chosen filter.
  st_process_thread spt;
  spt.is_preview = false;
  spt.commands_line = _commands_line;
  spt.verbosity_mode = get_verbosity_mode();
  spt.images_names.assign();
  spt.progress = -1;

  const CImg<int> layers = get_input_layers(spt.images);
  CImg<int> layer_dimensions(spt.images.size(),4);
  CImg<char> layer_name(256);

  const int image_id = gimp_item_get_image(layers[0]);
  int is_selection = 0, sel_x0 = 0, sel_y0 = 0, sel_x1 = 0, sel_y1 = 0;
  if (!gimp_selection_bounds(image_id,&is_selection,&sel_x0,&sel_y0,&sel_x1,&sel_y1)) is_selection = 0;
  else if (output_mode<1 || output_mode>2) sel_x0 = sel_y0 = 0;

  cimglist_for(spt.images,p) {
    const CImg<gmic_pixel_type>& img = spt.images[p];
    layer_dimensions(p,0) = img.width(); layer_dimensions(p,1) = img.height();
    layer_dimensions(p,2) = img.depth(); layer_dimensions(p,3) = img.spectrum();
    const GimpLayerModeEffects blendmode = gimp_layer_get_mode(layers[p]);
    const float opacity = gimp_layer_get_opacity(layers[p]);
    int posx = 0, posy = 0;
    if (is_selection && output_mode) {
      if (output_mode>=1 && output_mode<=2) { posx = sel_x0; posy = sel_y0; }
    } else gimp_drawable_offsets(layers[p],&posx,&posy);
    cimg_snprintf(layer_name,layer_name.width(),"mode(%s),opacity(%g),pos(%d,%d),name(%s)",
                  s_blendmode[blendmode],opacity,posx,posy,
                  gimp_item_get_name(layers[p]));
    CImg<char>::string(layer_name).move_to(spt.images_names);
  }

  // Get original image dimensions.
  unsigned int image_width = 0, image_height = 0;
  if (layers.height()) {
    image_width = gimp_image_width(image_id);
    image_height = gimp_image_height(image_id);
  }

  // Reset values for button parameters.
  cimglist_for(gmic_button_parameters,l) set_filter_parameter(filter,gmic_button_parameters(l,0),"0");

  // Create processing thread and wait for its completion.
  if (run_mode!=GIMP_RUN_NONINTERACTIVE) {
#if !defined(__MACOSX__) && !defined(__APPLE__)
    spt.is_thread = true;
    pthread_mutex_init(&spt.is_running,0);
    pthread_mutex_init(&spt.wait_lock,0);
    pthread_cond_init(&spt.wait_cond,0);
    pthread_mutex_lock(&spt.wait_lock);
    pthread_create(&(spt.thread),0,process_thread,(void*)&spt);
    pthread_cond_wait(&spt.wait_cond,&spt.wait_lock);  // Wait for the thread to lock the mutex.
    pthread_mutex_unlock(&spt.wait_lock);
    pthread_mutex_destroy(&spt.wait_lock);

    while (pthread_mutex_trylock(&spt.is_running)) {
      if (spt.progress>=0) gimp_progress_update(cimg::min(1.0,spt.progress/100.0));
      else gimp_progress_pulse();
      cimg::wait(350);
    }

    gimp_progress_update(1.0);
    pthread_join(spt.thread,0);
    pthread_mutex_unlock(&spt.is_running);
    pthread_mutex_destroy(&spt.is_running);
#else
    gimp_progress_update(0.5);
    process_thread(&spt);
    gimp_progress_update(1.0);
#endif
  } else {
    spt.is_thread = false;
    process_thread(&spt);
  }

  // Check that everything went fine, else display an error dialog.
  if (spt.error_message) {

    if (run_mode!=GIMP_RUN_NONINTERACTIVE) {
      GtkWidget *const
        message = gtk_message_dialog_new(0,GTK_DIALOG_MODAL,GTK_MESSAGE_ERROR,GTK_BUTTONS_OK,"%s",
                                         spt.error_message.data());
      gtk_widget_show(message);
      gtk_dialog_run(GTK_DIALOG(message));
      gtk_widget_destroy(message);
    } else {
      std::fprintf(cimg::output(),"\n[gmic_gimp]./error/ When running command '%s', this error occured :\n%s\n",
                   _commands_line,spt.error_message.data());
      std::fflush(cimg::output());
    }
    status = GIMP_PDB_CALLING_ERROR;
  } else {

    // Get output layers dimensions and check if input/output layers have compatible dimensions.
    unsigned int max_width = 0, max_height = 0, max_channels = 0;
    cimglist_for(spt.images,l) {
      if (spt.images[l].is_empty()) { spt.images.remove(l--); continue; }          // Discard possible empty images.
      if (spt.images[l]._width>max_width) max_width = spt.images[l]._width;
      if (spt.images[l]._height>max_height) max_height = spt.images[l]._height;
      if (spt.images[l]._spectrum>max_channels) max_channels = spt.images[l]._spectrum;
    }
    bool is_compatible_dimensions = (spt.images.size()==layers._height);
    for (unsigned int p = 0; p<spt.images.size() && is_compatible_dimensions; ++p) {
      const CImg<gmic_pixel_type>& img = spt.images[p];
      const bool
        source_is_alpha = (layer_dimensions(p,3)==2 || layer_dimensions(p,3)>=4),
        dest_is_alpha = (img.spectrum()==2 || img.spectrum()>=4);
      if (dest_is_alpha && !source_is_alpha) { gimp_layer_add_alpha(layers[p]); ++layer_dimensions(p,3); }
      if (img.width()!=layer_dimensions(p,0) ||
          img.height()!=layer_dimensions(p,1) ||
          img.spectrum()>layer_dimensions(p,3)) is_compatible_dimensions = false;
    }

    // Transfer the output layers back into GIMP.
    GimpLayerModeEffects layer_blendmode = GIMP_NORMAL_MODE;
    gint x1, y1, x2, y2, layer_posx = 0, layer_posy = 0;
    double layer_opacity = 100;
    GimpPixelRgn region;
    CImg<char> layer_name;

    switch (output_mode) {
    case 0 : { // Output in 'Replace' mode.
      gimp_image_undo_group_start(image_id);
      if (is_compatible_dimensions) cimglist_for(spt.images,p) { // Direct replacement of the layer data.
          layer_blendmode = gimp_layer_get_mode(layers[p]);
          layer_opacity = gimp_layer_get_opacity(layers[p]);
          gimp_drawable_offsets(layers[p],&layer_posx,&layer_posy);
          CImg<char>::string(gimp_item_get_name(layers[p])).move_to(layer_name);
          get_output_layer_props(spt.images_names[p],layer_blendmode,layer_opacity,layer_posx,layer_posy,layer_name);
          CImg<gmic_pixel_type> &img = spt.images[p];
          calibrate_image(img,layer_dimensions(p,3),false);
          GimpDrawable *drawable = gimp_drawable_get(layers[p]);
          gimp_drawable_mask_bounds(drawable->drawable_id,&x1,&y1,&x2,&y2);
          gimp_pixel_rgn_init(&region,drawable,x1,y1,x2-x1,y2-y1,true,true);
          convert_image2uchar(img);
          gimp_pixel_rgn_set_rect(&region,(guchar*)img.data(),x1,y1,x2-x1,y2-y1);
          img.assign();
          gimp_layer_set_mode(layers[p],layer_blendmode);
          gimp_layer_set_opacity(layers[p],layer_opacity);
          gimp_layer_set_offsets(layers[p],layer_posx,layer_posy);
          if (verbosity_mode==1) gimp_item_set_name(layers[p],new_label);
          else if (layer_name) gimp_item_set_name(layers[p],layer_name);
          gimp_drawable_flush(drawable);
          gimp_drawable_merge_shadow(drawable->drawable_id,true);
          gimp_drawable_update(drawable->drawable_id,x1,y1,x2-x1,y2-y1);
          gimp_drawable_detach(drawable);
        } else { // Indirect replacement : create new layers.
          gimp_selection_none(image_id);
#if GIMP_MINOR_VERSION<=6
        const int layer_pos = gimp_image_get_layer_position(image_id,layers[0]);
#else
        const int layer_pos = gimp_image_get_item_position(image_id,layers[0]);
#endif
        cimglist_for(spt.images,p) {
          layer_posx = layer_posy = 0;
          if (p<layers.height()) {
            layer_blendmode = gimp_layer_get_mode(layers[p]);
            layer_opacity = gimp_layer_get_opacity(layers[p]);
            if (!is_selection) gimp_drawable_offsets(layers[p],&layer_posx,&layer_posy);
            CImg<char>::string(gimp_item_get_name(layers[p])).move_to(layer_name);
            gimp_image_remove_layer(image_id,layers[p]);
          } else {
            layer_blendmode = GIMP_NORMAL_MODE;
            layer_opacity = 100;
            layer_name.assign();
          }
          get_output_layer_props(spt.images_names[p],layer_blendmode,layer_opacity,layer_posx,layer_posy,layer_name);

          CImg<gmic_pixel_type> &img = spt.images[p];
          if (gimp_image_base_type(image_id)==GIMP_GRAY) calibrate_image(img,(img.spectrum()==1 ||
                                                                              img.spectrum()==3)?1:2,false);
          else calibrate_image(img,(img.spectrum()==1 || img.spectrum()==3)?3:4,false);
          gint layer_id = gimp_layer_new(image_id,new_label,img.width(),img.height(),
                                         img.spectrum()==1?GIMP_GRAY_IMAGE:
                                         img.spectrum()==2?GIMP_GRAYA_IMAGE:
                                         img.spectrum()==3?GIMP_RGB_IMAGE:GIMP_RGBA_IMAGE,
                                         layer_opacity,layer_blendmode);
          gimp_layer_set_offsets(layer_id,layer_posx,layer_posy);
          if (verbosity_mode==1) gimp_item_set_name(layer_id,new_label);
          else if (layer_name) gimp_item_set_name(layer_id,layer_name);
#if GIMP_MINOR_VERSION<=6
          gimp_image_add_layer(image_id,layer_id,layer_pos+p);
#else
          gimp_image_insert_layer(image_id,layer_id,-1,layer_pos+p);
#endif
          GimpDrawable *drawable = gimp_drawable_get(layer_id);
          gimp_pixel_rgn_init(&region,drawable,0,0,drawable->width,drawable->height,true,true);
          convert_image2uchar(img);
          gimp_pixel_rgn_set_rect(&region,(guchar*)img.data(),0,0,img.width(),img.height());
          img.assign();
          gimp_drawable_flush(drawable);
          gimp_drawable_merge_shadow(drawable->drawable_id,true);
          gimp_drawable_update(drawable->drawable_id,0,0,drawable->width,drawable->height);
          gimp_drawable_detach(drawable);
        }

        for (unsigned int p = spt.images._width; p<layers._height; ++p) gimp_image_remove_layer(image_id,layers[p]);
        gimp_image_resize(image_id,max_width,max_height,0,0);
      }
      gimp_image_undo_group_end(image_id);
    } break;

    case 1 : case 2 : { // Output in 'New layer(s)' mode.
      gimp_image_undo_group_start(image_id);
      const gint active_layer_id = gimp_image_get_active_layer(image_id);
      gint layer_id = 0;
      cimglist_for(spt.images,p) {
        layer_blendmode = GIMP_NORMAL_MODE;
        layer_opacity = 100;
        layer_posx = layer_posy = 0;
        if (layers.height()==1) {
          if (!is_selection) gimp_drawable_offsets(active_layer_id,&layer_posx,&layer_posy);
          CImg<char>::string(gimp_item_get_name(active_layer_id)).move_to(layer_name);
        } else layer_name.assign();
        get_output_layer_props(spt.images_names[p],layer_blendmode,layer_opacity,layer_posx,layer_posy,layer_name);

        CImg<gmic_pixel_type> &img = spt.images[p];
        if (gimp_image_base_type(image_id)==GIMP_GRAY)
          calibrate_image(img,!is_selection && (img.spectrum()==1 || img.spectrum()==3)?1:2,false);
        else
          calibrate_image(img,!is_selection && (img.spectrum()==1 || img.spectrum()==3)?3:4,false);
        layer_id = gimp_layer_new(image_id,new_label,img.width(),img.height(),
                                  img.spectrum()==1?GIMP_GRAY_IMAGE:
                                  img.spectrum()==2?GIMP_GRAYA_IMAGE:
                                  img.spectrum()==3?GIMP_RGB_IMAGE:GIMP_RGBA_IMAGE,
                                  layer_opacity,layer_blendmode);
        gimp_layer_set_offsets(layer_id,layer_posx,layer_posy);
        if (verbosity_mode==1) gimp_item_set_name(layer_id,new_label);
        else if (layer_name) gimp_item_set_name(layer_id,layer_name);
#if GIMP_MINOR_VERSION<=6
        gimp_image_add_layer(image_id,layer_id,p);
#else
        gimp_image_insert_layer(image_id,layer_id,-1,p);
#endif
        GimpDrawable *drawable = gimp_drawable_get(layer_id);
        gimp_pixel_rgn_init(&region,drawable,0,0,drawable->width,drawable->height,true,true);
        convert_image2uchar(img);
        gimp_pixel_rgn_set_rect(&region,(guchar*)img.data(),0,0,img.width(),img.height());
        img.assign();
        gimp_drawable_flush(drawable);
        gimp_drawable_merge_shadow(drawable->drawable_id,true);
        gimp_drawable_update(drawable->drawable_id,0,0,drawable->width,drawable->height);
        gimp_drawable_detach(drawable);
      }
      gimp_image_resize(image_id,cimg::max(image_width,max_width),cimg::max(image_height,max_height),0,0);
      if (output_mode==1) gimp_image_set_active_layer(image_id,active_layer_id);
      else gtk_widget_destroy(gui_preview);  // Will force the preview to refresh on the new active layer.
      gimp_image_undo_group_end(image_id);
    } break;

    default : { // Output in 'New image' mode.
      if (spt.images.size()) {
        const int nimage_id = gimp_image_new(max_width,max_height,max_channels<=2?GIMP_GRAY:GIMP_RGB);
        const gint active_layer_id = gimp_image_get_active_layer(image_id);
        gimp_image_undo_group_start(nimage_id);
        cimglist_for(spt.images,p) {
          layer_blendmode = GIMP_NORMAL_MODE;
          layer_opacity = 100;
          layer_posx = layer_posy = 0;
          if (layers.height()==1) {
            if (!is_selection) gimp_drawable_offsets(active_layer_id,&layer_posx,&layer_posy);
            CImg<char>::string(gimp_item_get_name(active_layer_id)).move_to(layer_name);
          } else layer_name.assign();
          get_output_layer_props(spt.images_names[p],layer_blendmode,layer_opacity,layer_posx,layer_posy,layer_name);

          CImg<gmic_pixel_type> &img = spt.images[p];
          if (gimp_image_base_type(nimage_id)!=GIMP_GRAY)
            calibrate_image(img,(img.spectrum()==1 || img.spectrum()==3)?3:4,false);
          gint layer_id = gimp_layer_new(nimage_id,new_label,img.width(),img.height(),
                                         img.spectrum()==1?GIMP_GRAY_IMAGE:
                                         img.spectrum()==2?GIMP_GRAYA_IMAGE:
                                         img.spectrum()==3?GIMP_RGB_IMAGE:GIMP_RGBA_IMAGE,
                                         layer_opacity,layer_blendmode);
          gimp_layer_set_offsets(layer_id,layer_posx,layer_posy);
          if (verbosity_mode==1) gimp_item_set_name(layer_id,new_label);
          else if (layer_name) gimp_item_set_name(layer_id,layer_name);
#if GIMP_MINOR_VERSION<=6
          gimp_image_add_layer(nimage_id,layer_id,p);
#else
          gimp_image_insert_layer(nimage_id,layer_id,-1,p);
#endif
          GimpDrawable *drawable = gimp_drawable_get(layer_id);
          GimpPixelRgn dest_region;
          gimp_pixel_rgn_init(&dest_region,drawable,0,0,drawable->width,drawable->height,true,true);
          convert_image2uchar(img);
          gimp_pixel_rgn_set_rect(&dest_region,(guchar*)img.data(),0,0,img.width(),img.height());
          img.assign();
          gimp_drawable_flush(drawable);
          gimp_drawable_merge_shadow(drawable->drawable_id,true);
          gimp_drawable_update(drawable->drawable_id,0,0,drawable->width,drawable->height);
          gimp_drawable_detach(drawable);
        }
        gimp_display_new(nimage_id);
        gimp_image_undo_group_end(nimage_id);
      }
    }
    }

    // Update values of parameters if invoked command requests it (using status value).
    const unsigned int l_status = spt.status._width;
    if (l_status>3 && spt.status[0]==_lbrace && spt.status[l_status-2]==_rbrace) {
      spt.status.crop(1,l_status-3);
      CImgList<char> return_values = spt.status.get_split(CImg<char>::vector(_rbrace,_lbrace),false,false);
      if (return_values._width==get_filter_nbparams(filter)) {
        cimglist_for(return_values,l)
          set_filter_parameter(filter,l,gmic_strreplace(return_values[l].resize(1,return_values[l]._height+1,1,1,0)));
        update_parameters = true;
      }
    }
  }
  if (run_mode!=GIMP_RUN_NONINTERACTIVE) {
    gimp_progress_end();
    gimp_displays_flush();
    if (update_parameters && is_apply) create_parameters_gui(false);
  }
}

// Process the preview image.
//---------------------------
void process_preview() {
  if (is_block_preview) { is_block_preview = false; return; }
  if (!gimp_image_is_valid(image_id)) return;
  const unsigned int filter = get_current_filter();
  if (!filter) return;
  const char *const commands_line = get_commands_line(true);
  if (!commands_line || std::strstr(commands_line,"-_none_")) return;
  bool update_parameters = false;
  int wp, hp, sp, xp, yp;
  static int _xp = -1, _yp = -1;

  guchar *const ptr0 = gimp_zoom_preview_get_source(GIMP_ZOOM_PREVIEW(gui_preview),&wp,&hp,&sp);
  const double factor = gimp_zoom_preview_get_factor(GIMP_ZOOM_PREVIEW(gui_preview));
  gimp_preview_get_position(GIMP_PREVIEW(gui_preview),&xp,&yp);
  if (xp!=_xp || _yp!=yp) { _xp = xp; _yp = yp; computed_preview.assign(); }
  if (!computed_preview) {

    // Get input layers for the chosen filter and convert then to the preview size if necessary.
    st_process_thread spt;
    spt.is_preview = true;
    spt.is_thread = false;
    spt.commands_line = commands_line;
    spt.verbosity_mode = get_verbosity_mode();
    spt.progress = -1;

    CImg<char> layer_name(256);
    const unsigned int input_mode = get_input_mode();
    int nb_layers = 0, *const layers = gimp_image_get_layers(image_id,&nb_layers);

    if (nb_layers && input_mode) {
      if (input_mode==1 ||
          (input_mode==2 && nb_layers==1) ||
          (input_mode==3 && nb_layers==1 && gimp_drawable_get_visible(*layers)) ||
          (input_mode==4 && nb_layers==1 && !gimp_drawable_get_visible(*layers)) ||
          (input_mode==5 && nb_layers==1)) {

        // Single input layer : get the default thumbnail provided by GIMP.
        spt.images.assign(1);
        spt.images_names.assign(1);

        const guchar *ptrs = ptr0;
        spt.images.assign(1,wp,hp,1,sp);
        const int whp = wp*hp;
        switch (sp) {
        case 1 : {
          float *ptr_r = spt.images[0].data(0,0,0,0);
          for (int xy = 0; xy<whp; ++xy) *(ptr_r++) = (float)*(ptrs++);
        } break;
        case 2 : {
          float
            *ptr_r = spt.images[0].data(0,0,0,0),
            *ptr_g = spt.images[0].data(0,0,0,1);
          for (int xy = 0; xy<whp; ++xy) {
            *(ptr_r++) = (float)*(ptrs++);
            *(ptr_g++) = (float)*(ptrs++);
          }
        } break;
        case 3 : {
          float
            *ptr_r = spt.images[0].data(0,0,0,0),
            *ptr_g = spt.images[0].data(0,0,0,1),
            *ptr_b = spt.images[0].data(0,0,0,2);
          for (int xy = 0; xy<whp; ++xy) {
            *(ptr_r++) = (float)*(ptrs++);
            *(ptr_g++) = (float)*(ptrs++);
            *(ptr_b++) = (float)*(ptrs++);
          }
        } break;
        case 4 : {
          float
            *ptr_r = spt.images[0].data(0,0,0,0),
            *ptr_g = spt.images[0].data(0,0,0,1),
            *ptr_b = spt.images[0].data(0,0,0,2),
            *ptr_a = spt.images[0].data(0,0,0,3);
          for (int xy = 0; xy<whp; ++xy) {
            *(ptr_r++) = (float)*(ptrs++);
            *(ptr_g++) = (float)*(ptrs++);
            *(ptr_b++) = (float)*(ptrs++);
            *(ptr_a++) = (float)*(ptrs++);
          }
        } break;
        }

        const float opacity = gimp_layer_get_opacity(*layers);
        const GimpLayerModeEffects blendmode = gimp_layer_get_mode(*layers);
        int posx = 0, posy = 0;
        gimp_drawable_offsets(*layers,&posx,&posy);
        const int
          w = gimp_drawable_width(*layers),
          h = gimp_drawable_height(*layers),
          ox = (int)(posx*wp/w),
          oy = (int)(posy*hp/h);
        cimg_snprintf(layer_name,layer_name.width(),"mode(%s),opacity(%g),pos(%d,%d),name(%s)",
                      s_blendmode[blendmode],opacity,ox,oy,
                      gimp_item_get_name(*layers));
        CImg<char>::string(layer_name).move_to(spt.images_names[0]);
      } else {

        // Multiple input layers : compute a 'hand-made' set of thumbnails.
        CImgList<unsigned char> images_uchar;
        const CImg<int> layers = get_input_layers(images_uchar);
        if (images_uchar) {
          spt.images.assign(images_uchar.size());
          spt.images_names.assign(images_uchar.size());

          // Retrieve global preview ratio.
          const int
            active_layer = gimp_image_get_active_layer(image_id),
            w0 = gimp_drawable_width(active_layer),
            h0 = gimp_drawable_height(active_layer);
          const double ratio = cimg::max((double)wp/w0,(double)hp/h0);

          // Retrieve resized and cropped preview layers.
          cimg_forY(layers,p) {
            const float opacity = gimp_layer_get_opacity(layers[p]);
            const GimpLayerModeEffects blendmode = gimp_layer_get_mode(layers[p]);
            int posx = 0, posy = 0;
            gimp_drawable_offsets(layers[p],&posx,&posy);

            CImg<unsigned char>& img = images_uchar[p];
            const int
              w = img.width(),
              h = img.height(),
              x0 = (int)(xp*w/wp/factor),
              y0 = (int)(yp*h/hp/factor),
              x1 = x0 + (int)(w/factor) - 1,
              y1 = y0 + (int)(h/factor) - 1,
              ox = (int)(posx*wp/w0),
              oy = (int)(posy*hp/h0);
            img.get_crop(x0,y0,x1,y1).resize(cimg::round(img.width()*ratio),
                                             cimg::round(img.height()*ratio)).
              move_to(spt.images[p]);
            cimg_snprintf(layer_name,layer_name.width(),"mode(%s),opacity(%g),pos(%d,%d)",
                          s_blendmode[blendmode],opacity,ox,oy);
            CImg<char>::string(layer_name).move_to(spt.images_names[p]);
          }
        }
      }
    }

    // Run G'MIC.
    CImg<unsigned char> original_preview;
    if (spt.images) original_preview = spt.images[0];
    else original_preview.assign(wp,hp,1,4,0);
    process_thread(&spt);

    // Manage possible errors.
    if (spt.error_message) {
      spt.images.assign();
      spt.images_names.assign();
      original_preview.move_to(spt.images);
      char command[1024] = { 0 };
      cimg_snprintf(command,sizeof(command),"%s-gimp_error_preview \"%s\"",
                    get_verbosity_mode()>5?"-debug ":get_verbosity_mode()>3?"":"-v -99 ",
                    spt.error_message.data());
      try {
        gmic(command,spt.images,spt.images_names,gmic_additional_commands,true);

      } catch (...) {  // Fallback for '-gimp_error_preview'.
        const unsigned char white[] = { 155,155,155 };
        spt.images.assign(1).back().draw_text(0,0," Preview\n  error ",white,0,1,57).
          resize(-100,-100,1,4).get_shared_channel(3).dilate(5);
        spt.images[0].resize(wp,hp,1,4,0,0,0.5,0.5)+=100;
      }
      spt.status.assign();
    }

    // Transfer the output layers back into GIMP preview.
    CImgList<gmic_pixel_type> preview_images;
    computed_preview.assign();

    switch (get_preview_mode()) {
    case 0 : // Preview 1st layer
      if (spt.images && spt.images.size()>0) spt.images[0].move_to(preview_images);
      break;
    case 1 : // Preview 2nd layer
      if (spt.images && spt.images.size()>1) spt.images[1].move_to(preview_images);
      break;
    case 2 : // Preview 3rd layer
      if (spt.images && spt.images.size()>2) spt.images[2].move_to(preview_images);
      break;
    case 3 : // Preview 4th layer
      if (spt.images && spt.images.size()>3) spt.images[3].move_to(preview_images);
      break;
    case 4 : { // Preview 1st->2nd layers
      cimglist_for_in(spt.images,0,1,l) spt.images[l].move_to(preview_images);
    } break;
    case 5 : { // Preview 1st->3nd layers
      cimglist_for_in(spt.images,0,2,l) spt.images[l].move_to(preview_images);
    } break;
    case 6 : { // Preview 1st->4nd layers
      cimglist_for_in(spt.images,0,3,l) spt.images[l].move_to(preview_images);
    } break;
    default : // Preview all layers
      spt.images.move_to(preview_images);
    }
    spt.images.assign();
    spt.images_names.assign();

    unsigned int _sp = 0;
    cimglist_for(preview_images,l) if (preview_images[l]._spectrum>_sp) _sp = preview_images[l]._spectrum;
    if (_sp==1 || _sp==3) ++_sp;
    cimglist_for(preview_images,l) calibrate_image(preview_images[l],_sp,true);
    (preview_images>'x').move_to(computed_preview);

    if (!computed_preview) computed_preview.assign(wp,hp,1,4,0);

    if (computed_preview.width()!=wp || computed_preview.height()!=hp) {
      const double ratio = cimg::min((double)wp/computed_preview.width(),
                                     (double)hp/computed_preview.height());
      computed_preview.resize((int)cimg::round(computed_preview.width()*ratio),
                              (int)cimg::round(computed_preview.height()*ratio),
                              1,-100,2);
    }
    if (computed_preview.width()!=wp || computed_preview.height()!=hp)
      computed_preview.resize(wp,hp,1,-100,0,0,0.5,0.5);

    calibrate_image(computed_preview,sp,true);
    convert_image2uchar(computed_preview);
    computed_preview.channel(0);

    // Update values of parameters if invoked command requests it (using status value).
    const unsigned int l_status = spt.status._width;
    if (l_status>3 && spt.status[0]==_lbrace && spt.status[l_status-2]==_rbrace) {
      spt.status.crop(1,l_status-3);
      CImgList<char> return_values = spt.status.get_split(CImg<char>::vector(_rbrace,_lbrace),false,false);
      if (return_values._width==get_filter_nbparams(filter)) {
        cimglist_for(return_values,l)
          set_filter_parameter(filter,l,gmic_strreplace(return_values[l].resize(1,return_values[l]._height+1,1,1,0)));
        update_parameters = true;
      }
    }
  }

  std::memcpy(ptr0,computed_preview.data(),wp*hp*sp*sizeof(unsigned char));
  gimp_preview_draw_buffer(GIMP_PREVIEW(gui_preview),ptr0,wp*sp);
  g_free(ptr0);
  if (update_parameters) create_parameters_gui(false);
}

// Create the parameters GUI for the chosen filter.
//--------------------------------------------------
void create_parameters_gui(const bool reset_params) {
  const unsigned int filter = get_current_filter();
  gmic_button_parameters.assign();

  // Remove existing table in the parameters frame if necessary.
  GtkWidget *const child = GTK_WIDGET(gtk_bin_get_child(GTK_BIN(right_frame)));
  if (child) gtk_container_remove(GTK_CONTAINER(right_frame),child);

  // Create new table for the parameters frame.
  GtkWidget *table = 0;
  if (!filter) {  // No filter selected -> 1x1 table with default message.
    table = gtk_table_new(1,1,false);
    gtk_widget_show(table);
    GtkWidget *const label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label),t("<i>Select a filter...</i>"));
    gtk_widget_show(label);
    gtk_table_attach(GTK_TABLE(table),label,0,1,0,1,
                     (GtkAttachOptions)(GTK_EXPAND),(GtkAttachOptions)(GTK_EXPAND),0,0);
    gtk_misc_set_alignment (GTK_MISC(label),0,0.5);
    gtk_frame_set_label(GTK_FRAME(right_frame),NULL);
  } else { // Filter selected -> Build the table for setting the parameters.
    char s_label[256] = { 0 };
    cimg_snprintf(s_label,sizeof(s_label),"<b>  %s :  </b>",gmic_entries[filter].data());
    GtkWidget *const frame_title = gtk_label_new(NULL);
    gtk_widget_show(frame_title);
    gtk_label_set_markup(GTK_LABEL(frame_title),s_label);
    gtk_frame_set_label_widget(GTK_FRAME(right_frame),frame_title);

    // Count number of filter arguments.
    char argument_name[256] = { 0 }, _argument_type[32] = { 0 }, argument_arg[65536] = { 0 };
    unsigned int nb_arguments = 0;
    for (const char *argument = gmic_arguments[filter].data(); *argument; ) {
      int err = std::sscanf(argument,"%4095[^=]=%4095[ a-zA-z](%65535[^)]",
                            argument_name,_argument_type,&(argument_arg[0]=0));
      if (err!=3) err = std::sscanf(argument,"%4095[^=]=%4095[ a-zA-z]{%65535[^}]",
                                    argument_name,_argument_type,argument_arg);
      if (err!=3) err = std::sscanf(argument,"%4095[^=]=%4095[ a-zA-z][%65535[^]]",
                                    argument_name,_argument_type,argument_arg);
      if (err>=2) {
        argument += std::strlen(argument_name) + std::strlen(_argument_type) + std::strlen(argument_arg) + 3;
        if (*argument) ++argument;
        ++nb_arguments;
      } else break;
    }

    if (!nb_arguments) { // Filter requires no parameters -> 1x1 table with default message.
      table = gtk_table_new(1,1,false);
      gtk_widget_show(table);
      GtkWidget *label = gtk_label_new(NULL);
      gtk_label_set_markup(GTK_LABEL(label),t("<i>No parameters to set...</i>"));
      gtk_widget_show(label);
      gtk_table_attach(GTK_TABLE(table),label,0,1,0,1,
                       (GtkAttachOptions)(GTK_EXPAND),(GtkAttachOptions)(GTK_EXPAND),0,0);
      gtk_misc_set_alignment(GTK_MISC(label),0,0.5);
      if (event_infos) delete[] event_infos; event_infos = 0;
      set_filter_nbparams(filter,0);

    } else { // Filter requires parameters -> Create parameters table.

      // Create new table for putting parameters inside.
      table = gtk_table_new(nb_arguments,3,false);
      gtk_widget_show(table);
      gtk_table_set_row_spacings(GTK_TABLE(table),6);
      gtk_table_set_col_spacings(GTK_TABLE(table),6);
      gtk_container_set_border_width(GTK_CONTAINER(table),8);

      // Parse arguments list and add recognized one to the table.
      if (event_infos) delete[] event_infos; event_infos = new void*[2*nb_arguments];
      unsigned int current_argument = 0, current_table_line = 0;
      const bool is_fave = filter>=indice_faves;
      for (const char *argument = gmic_arguments[filter].data(); *argument; ) {
        int err = std::sscanf(argument,"%4095[^=]=%4095[ a-zA-Z_](%65535[^)]",
                              argument_name,_argument_type,&(argument_arg[0]=0));
        if (err!=3) err = std::sscanf(argument,"%4095[^=]=%4095[ a-zA-Z_][%65535[^]]",
                                      argument_name,_argument_type,argument_arg);
        if (err!=3) err = std::sscanf(argument,"%4095[^=]=%4095[ a-zA-Z_]{%65535[^}]",
                                      argument_name,_argument_type,argument_arg);
        if (err>=2) {
          argument += std::strlen(argument_name) + std::strlen(_argument_type) + std::strlen(argument_arg) + 3;
          if (*argument) ++argument;
          cimg::strpare(argument_name,' ',false,true);
          cimg::strpare(argument_name,'\"',true);
          cimg::strunescape(argument_name);
          cimg::strpare(_argument_type,' ',false,true);
          cimg::strpare(argument_arg,' ',false,true);

          const bool is_silent_argument = (*_argument_type=='_');
          char
            *const argument_type = _argument_type + (is_silent_argument?1:0),
            *const argument_value = get_filter_parameter(filter,current_argument),
            *const argument_fave = get_fave_parameter(filter,current_argument);

#if defined(_WIN64)
          typedef unsigned long long pint;
#else
          typedef unsigned long pint;
#endif

          // Check for a float-valued argument.
          bool found_valid_argument = false;
          if (!found_valid_argument && !cimg::strcasecmp(argument_type,"float")) {
            float value = 0, min_value = 0, max_value = 100;
            setlocale(LC_NUMERIC,"C");
            std::sscanf(argument_arg,"%f%*c%f%*c%f",&value,&min_value,&max_value);
            if (is_fave) std::sscanf(argument_fave,"%f",&value);
            if (!reset_params) std::sscanf(argument_value,"%f",&value);
            GtkObject *const
              scale = gimp_scale_entry_new(GTK_TABLE(table),0,(int)current_table_line,argument_name,50,6,
                                           (double)value,(double)min_value,(double)max_value,
                                           (double)(max_value-min_value)/100,
                                           (double)(max_value-min_value)/20,
                                           2,true,0,0,0,0);
            event_infos[2*current_argument] = (void*)(pint)current_argument;
            event_infos[2*current_argument+1] = (void*)0;
            on_float_parameter_changed(GTK_ADJUSTMENT(scale),event_infos+2*current_argument);
            g_signal_connect(scale,"value_changed",G_CALLBACK(on_float_parameter_changed),
                             event_infos+2*current_argument);
            if (!is_silent_argument)
              g_signal_connect(scale,"value_changed",G_CALLBACK(_gimp_preview_invalidate),0);
            found_valid_argument = true; ++current_argument;
          }

          // Check for an int-valued argument.
          if (!found_valid_argument && !cimg::strcasecmp(argument_type,"int")) {
            float value = 0, min_value = 0, max_value = 100;
            setlocale(LC_NUMERIC,"C");
            std::sscanf(argument_arg,"%f%*c%f%*c%f",&value,&min_value,&max_value);
            if (is_fave) std::sscanf(argument_fave,"%f",&value);
            if (!reset_params) std::sscanf(argument_value,"%f",&value);
            GtkObject *const
              scale = gimp_scale_entry_new(GTK_TABLE(table),0,(int)current_table_line,argument_name,50,6,
                                           (double)(int)cimg::round(value,1.0f),
                                           (double)(int)cimg::round(min_value,1.0f),
                                           (double)(int)cimg::round(max_value,1.0f),
                                           (double)1,
                                           (double)cimg::max(1.0,cimg::round((max_value-min_value)/20,1,1)),
                                           0,true,0,0,0,0);
            event_infos[2*current_argument] = (void*)(pint)current_argument;
            event_infos[2*current_argument+1] = (void*)0;
            on_int_parameter_changed(GTK_ADJUSTMENT(scale),event_infos+2*current_argument);
            g_signal_connect(scale,"value_changed",G_CALLBACK(on_int_parameter_changed),
                             event_infos+2*current_argument);
            if (!is_silent_argument)
              g_signal_connect(scale,"value_changed",G_CALLBACK(_gimp_preview_invalidate),0);
            found_valid_argument = true; ++current_argument;
          }

          // Check for a bool-valued argument.
          if (!found_valid_argument && !cimg::strcasecmp(argument_type,"bool")) {
            cimg::strpare(argument_arg,' ',false,true); cimg::strpare(argument_arg,'\"',true);
            bool
              value = !(!*argument_arg || !cimg::strcasecmp(argument_arg,"false") ||
                        (argument_arg[0]=='0' && argument_arg[1]==0));
            if (is_fave && *argument_fave)
              value = !(!cimg::strcasecmp(argument_fave,"false") ||
                        (argument_fave[0]=='0' && argument_fave[1]==0));
            if (!reset_params && *argument_value)
              value = !(!cimg::strcasecmp(argument_value,"false") ||
                        (argument_value[0]=='0' && argument_value[1]==0));
            GtkWidget *const checkbutton = gtk_check_button_new_with_label(argument_name);
            gtk_widget_show(checkbutton);
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbutton),value);
            gtk_table_attach(GTK_TABLE(table),checkbutton,0,3,(int)current_table_line,(int)current_table_line+1,
                             (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),GTK_SHRINK,0,0);
            event_infos[2*current_argument] = (void*)(pint)current_argument;
            event_infos[2*current_argument+1] = (void*)0;
            on_bool_parameter_changed(GTK_CHECK_BUTTON(checkbutton),event_infos+2*current_argument);
            g_signal_connect(checkbutton,"toggled",G_CALLBACK(on_bool_parameter_changed),
                             event_infos+2*current_argument);
            if (!is_silent_argument)
              g_signal_connect(checkbutton,"toggled",G_CALLBACK(_gimp_preview_invalidate),0);
            found_valid_argument = true; ++current_argument;
          }

          // Check for a button argument.
          if (!found_valid_argument && !cimg::strcasecmp(argument_type,"button")) {
            float alignment = 0;
            setlocale(LC_NUMERIC,"C");
            if (std::sscanf(argument_arg,"%f",&alignment)!=1) alignment = 0;
            GtkWidget
              *const button = gtk_button_new_with_label(argument_name),
              *const align = gtk_alignment_new(alignment,0.5f,0,0);
            gtk_widget_show(button);
            gtk_container_add(GTK_CONTAINER(align),button);
            gtk_widget_show(align);
            gtk_table_attach(GTK_TABLE(table),align,0,3,(int)current_table_line,(int)current_table_line+1,
                             (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),GTK_SHRINK,0,0);
            event_infos[2*current_argument] = (void*)(pint)current_argument;
            event_infos[2*current_argument+1] = (void*)0;
            g_signal_connect(button,"clicked",G_CALLBACK(on_button_parameter_clicked),
                             event_infos+2*current_argument);
            if (!is_silent_argument)
              g_signal_connect(button,"clicked",G_CALLBACK(_gimp_preview_invalidate),0);
            set_filter_parameter(filter,current_argument,"0");
            CImg<unsigned int>::vector(current_argument).move_to(gmic_button_parameters);
            found_valid_argument = true; ++current_argument;
          }

          // Check for a choice-valued argument.
          if (!found_valid_argument && !cimg::strcasecmp(argument_type,"choice")) {
            GtkWidget *const label = gtk_label_new(argument_name);
            gtk_widget_show(label);
            gtk_table_attach(GTK_TABLE(table),label,0,1,(int)current_table_line,(int)current_table_line+1,
                             GTK_FILL,GTK_SHRINK,0,0);
            gtk_misc_set_alignment(GTK_MISC(label),0,0.5);
            GtkWidget *const combobox = gtk_combo_box_new_text();
            gtk_widget_show(combobox);
            char s_entry[256] = { 0 }, end = 0; int err = 0;
            unsigned int value = 0;
            const char *entries = argument_arg;
            if (std::sscanf(entries,"%u",&value)==1)
              entries+=cimg_snprintf(s_entry,sizeof(s_entry),"%u",value) + 1;
            while (*entries) {
              if ((err = std::sscanf(entries,"%4095[^,]%c",s_entry,&end))>0) {
                entries += std::strlen(s_entry) + (err==2?1:0);
                cimg::strpare(s_entry,' ',false,true); cimg::strpare(s_entry,'\"',true);
                gtk_combo_box_append_text(GTK_COMBO_BOX(combobox),s_entry);
              } else break;
            }
            if (is_fave) std::sscanf(argument_fave,"%u",&value);
            if (!reset_params) std::sscanf(argument_value,"%u",&value);
            gtk_combo_box_set_active(GTK_COMBO_BOX(combobox),value);
            gtk_table_attach(GTK_TABLE(table),combobox,1,3,(int)current_table_line,(int)current_table_line+1,
                             (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),GTK_FILL,0,0);
            event_infos[2*current_argument] = (void*)(pint)current_argument;
            event_infos[2*current_argument+1] = (void*)0;
            on_list_parameter_changed(GTK_COMBO_BOX(combobox),event_infos+2*current_argument);
            g_signal_connect(combobox,"changed",G_CALLBACK(on_list_parameter_changed),
                             event_infos+2*current_argument);
            if (!is_silent_argument)
              g_signal_connect(combobox,"changed",G_CALLBACK(_gimp_preview_invalidate),0);
            found_valid_argument = true; ++current_argument;
          }

          // Check for a single or multi-line text-valued argument.
          if (!found_valid_argument && !cimg::strcasecmp(argument_type,"text")) {
            int line_number = 0;
            char sep = 0;
            if (std::sscanf(argument_arg,"%d%c",&line_number,&sep)==2 && sep==',' && line_number==1) {
              // Multi-line entry
              GtkWidget *const frame = gtk_frame_new(NULL);
              gtk_widget_show(frame);
              gtk_frame_set_shadow_type(GTK_FRAME(frame),GTK_SHADOW_IN);
              gtk_container_set_border_width(GTK_CONTAINER(frame),4);

              GtkWidget *const hbox = gtk_hbox_new(false,0);
              gtk_widget_show(hbox);

              char s_label[256] = { 0 };
              cimg_snprintf(s_label,sizeof(s_label),"  %s       ",argument_name);
              GtkWidget *const label = gtk_label_new(s_label);
              gtk_widget_show(label);
              gtk_box_pack_start(GTK_BOX(hbox),label,false,false,0);

              GtkWidget *const button = gtk_button_new_with_label(t("Update"));
              gtk_widget_show(button);
              gtk_box_pack_start(GTK_BOX(hbox),button,false,false,0);

              gtk_frame_set_label_widget(GTK_FRAME(frame),hbox);

              GtkWidget *const alignment2 = gtk_alignment_new(0,0,1,1);
              gtk_widget_show(alignment2);
              gtk_alignment_set_padding(GTK_ALIGNMENT(alignment2),3,3,3,3);
              gtk_container_add(GTK_CONTAINER(frame),alignment2);

              GtkWidget *const view = gtk_text_view_new();
              gtk_widget_show(view);
              gtk_text_view_set_editable(GTK_TEXT_VIEW(view),true);
              gtk_text_view_set_left_margin(GTK_TEXT_VIEW(view),8);
              gtk_text_view_set_right_margin(GTK_TEXT_VIEW(view),8);
              gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(view),GTK_WRAP_CHAR);
              gtk_container_add(GTK_CONTAINER(alignment2),view);

              GtkTextBuffer *const buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
              char *value = std::strchr(argument_arg,',') + 1;
              if (is_fave) value = argument_fave;
              if (!reset_params && *argument_value) value = argument_value;
              else if (!is_fave) cimg::strunescape(value);
              cimg::strpare(value,' ',false,true);
              cimg::strpare(value,'\"',true);
              for (char *p = value; *p; ++p) if (*p==_dquote) *p='\"';
              gtk_text_buffer_set_text(buffer,value,-1);

              gtk_table_attach(GTK_TABLE(table),frame,0,3,(int)current_table_line,(int)current_table_line+1,
                               (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),(GtkAttachOptions)0,0,0);

              event_infos[2*current_argument] = (void*)(pint)current_argument;
              event_infos[2*current_argument+1] = (void*)view;
              on_multitext_parameter_changed(event_infos+2*current_argument);
              g_signal_connect_swapped(button,"clicked",G_CALLBACK(on_multitext_parameter_changed),
                                       event_infos+2*current_argument);
              g_signal_connect_swapped(view,"key-release-event",G_CALLBACK(on_multitext_parameter_changed),
                                       event_infos+2*current_argument);
              if (!is_silent_argument)
                g_signal_connect(button,"clicked",G_CALLBACK(_gimp_preview_invalidate),0);

            } else { // Single-line entry
              GtkWidget *const label = gtk_label_new(argument_name);
              gtk_widget_show(label);
              gtk_table_attach(GTK_TABLE(table),label,0,1,(int)current_table_line,(int)current_table_line+1,
                               GTK_FILL,GTK_SHRINK,0,0);
              gtk_misc_set_alignment(GTK_MISC(label),0,0.5);
              GtkWidget *const entry = gtk_entry_new_with_max_length(1023);
              gtk_widget_show(entry);
              char *value = (line_number!=0 || sep!=',')?argument_arg:(std::strchr(argument_arg,',') + 1);
              if (is_fave) value = argument_fave;
              if (!reset_params && *argument_value) value = argument_value;
              cimg::strpare(value,' ',false,true);
              cimg::strpare(value,'\"',true);
              for (char *p = value; *p; ++p) if (*p==_dquote) *p='\"';
              gtk_entry_set_text(GTK_ENTRY(entry),value);
              gtk_table_attach(GTK_TABLE(table),entry,1,2,(int)current_table_line,(int)current_table_line+1,
                               (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),(GtkAttachOptions)0,0,0);
              GtkWidget *const button = gtk_button_new_with_label(t("Update"));
              gtk_widget_show(button);
              gtk_table_attach(GTK_TABLE(table),button,2,3,(int)current_table_line,(int)current_table_line+1,
                               GTK_FILL,GTK_SHRINK,0,0);
              event_infos[2*current_argument] = (void*)(pint)current_argument;
              event_infos[2*current_argument+1] = (void*)entry;
              on_text_parameter_changed(event_infos+2*current_argument);
              g_signal_connect_swapped(button,"clicked",G_CALLBACK(on_text_parameter_changed),
                                       event_infos+2*current_argument);
              g_signal_connect_swapped(entry,"changed",G_CALLBACK(on_text_parameter_changed),
                                       event_infos+2*current_argument);
              if (!is_silent_argument) {
                g_signal_connect(button,"clicked",G_CALLBACK(_gimp_preview_invalidate),0);
                g_signal_connect(entry,"activate",G_CALLBACK(_gimp_preview_invalidate),0);
              }
            }

            found_valid_argument = true; ++current_argument;
          }

          // Check for a filename or folder name argument.
          if (!found_valid_argument && (!cimg::strcasecmp(argument_type,"file") ||
                                        !cimg::strcasecmp(argument_type,"folder"))) {
            GtkWidget *const label = gtk_label_new(argument_name);
            gtk_widget_show(label);
            gtk_table_attach(GTK_TABLE(table),label,0,1,(int)current_table_line,(int)current_table_line+1,
                             GTK_FILL,GTK_SHRINK,0,0);
            gtk_misc_set_alignment(GTK_MISC(label),0,0.5);
            GtkWidget *const
              file_chooser = gtk_file_chooser_button_new(argument_name,
                                                         cimg::uncase(argument_type[1])=='i'?
                                                         GTK_FILE_CHOOSER_ACTION_OPEN:
                                                         GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
            gtk_widget_show(file_chooser);

            char *value = argument_arg;
            if (is_fave) value = argument_fave;
            if (!reset_params && *argument_value) value = argument_value;
            cimg::strpare(value,' ',false,true); cimg::strpare(value,'\"',true);
            gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(file_chooser),value);
            gtk_table_attach(GTK_TABLE(table),file_chooser,1,3,(int)current_table_line,(int)current_table_line+1,
                             (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),(GtkAttachOptions)0,0,0);
            event_infos[2*current_argument] = (void*)(pint)current_argument;
            event_infos[2*current_argument+1] = (void*)is_silent_argument; // Silent property handled by event.
            on_file_parameter_changed(GTK_FILE_CHOOSER(file_chooser),event_infos+2*current_argument);
            g_signal_connect(file_chooser,"selection-changed",G_CALLBACK(on_file_parameter_changed),
                             event_infos+2*current_argument);
            found_valid_argument = true; ++current_argument;
          }

          // Check for a color argument.
          if (!found_valid_argument && !cimg::strcasecmp(argument_type,"color")) {
            GtkWidget *const label = gtk_label_new(argument_name);
            gtk_widget_show(label);
            gtk_misc_set_alignment(GTK_MISC(label),0,0.5);
            gtk_table_attach(GTK_TABLE(table),label,0,1,(int)current_table_line,(int)current_table_line+1,
                             GTK_FILL,GTK_SHRINK,0,0);
            GtkWidget *const hbox = gtk_hbox_new(false,6);
            gtk_widget_show(hbox);
            gtk_table_attach(GTK_TABLE(table),hbox,1,2,(int)current_table_line,(int)current_table_line+1,
                             GTK_FILL,GTK_SHRINK,0,0);
            GtkWidget *const color_chooser = gtk_color_button_new();
            gtk_widget_show(color_chooser);
            gtk_color_button_set_title(GTK_COLOR_BUTTON(color_chooser),argument_name);
            gtk_button_set_alignment(GTK_BUTTON(color_chooser),0,0.5);
            gtk_box_pack_start(GTK_BOX(hbox),color_chooser,false,false,0);

            float red = 0, green = 0, blue = 0, alpha = 255;
            setlocale(LC_NUMERIC,"C");

            const int err = std::sscanf(argument_arg,"%f%*c%f%*c%f%*c%f",&red,&green,&blue,&alpha);
            if (is_fave) std::sscanf(argument_fave,"%f%*c%f%*c%f%*c%f",&red,&green,&blue,&alpha);
            if (!reset_params) std::sscanf(argument_value,"%f%*c%f%*c%f%*c%f",&red,&green,&blue,&alpha);
            red = red<0?0:red>255?255:red;
            green = green<0?0:green>255?255:green;
            blue = blue<0?0:blue>255?255:blue;
            GdkColor color;
            color.pixel = 0;
            color.red = (unsigned int)(red*257);
            color.green = (unsigned int)(green*257);
            color.blue = (unsigned int)(blue*257);
            gtk_color_button_set_color(GTK_COLOR_BUTTON(color_chooser),&color);
            if (err==4) {
              gtk_color_button_set_use_alpha(GTK_COLOR_BUTTON(color_chooser),true);
              gtk_color_button_set_alpha(GTK_COLOR_BUTTON(color_chooser),(unsigned int)(alpha*257));
            } else gtk_color_button_set_use_alpha(GTK_COLOR_BUTTON(color_chooser),false);
            event_infos[2*current_argument] = (void*)(pint)current_argument;
            event_infos[2*current_argument+1] = (void*)0;
            on_color_parameter_changed(GTK_COLOR_BUTTON(color_chooser),event_infos+2*current_argument);
            g_signal_connect(color_chooser,"color-set",G_CALLBACK(on_color_parameter_changed),
                             event_infos+2*current_argument);
            if (!is_silent_argument)
              g_signal_connect(color_chooser,"color-set",G_CALLBACK(_gimp_preview_invalidate),0);
            found_valid_argument = true; ++current_argument;
          }

          // Check for a constant value.
          if (!found_valid_argument && !cimg::strcasecmp(argument_type,"const")) {
            const char *value = argument_arg;
            if (is_fave) value = argument_fave;
            if (!reset_params && *argument_value) value = argument_value;
            set_filter_parameter(filter,current_argument,value);
            found_valid_argument = true; ++current_argument;
          }

          // Check for a note.
          if (!found_valid_argument && !cimg::strcasecmp(argument_type,"note")) {
            cimg::strpare(argument_arg,' ',false,true);
            cimg::strpare(argument_arg,'\"',true);
            cimg::strunescape(argument_arg);
            GtkWidget *const label = gtk_label_new(NULL);
            gtk_label_set_markup(GTK_LABEL(label),argument_arg);
            gtk_label_set_line_wrap(GTK_LABEL(label),true);
            gtk_widget_show(label);
            gtk_table_attach(GTK_TABLE(table),label,0,3,(int)current_table_line,(int)current_table_line+1,
                             (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),GTK_SHRINK,0,0);
            gtk_misc_set_alignment(GTK_MISC(label),0,0.5);
            found_valid_argument = true;
          }

          // Check for a link.
          if (!found_valid_argument && !cimg::strcasecmp(argument_type,"link")) {
            char label[1024] = { 0 }, url[1024] = { 0 };
            float alignment = 0.5f;
            switch (std::sscanf(argument_arg,"%f,%1023[^,],%1023s",&alignment,label,url)) {
            case 2 : std::strcpy(url,label); break;
            case 1 : cimg_snprintf(url,sizeof(url),"%g",alignment); break;
            case 0 : if (std::sscanf(argument_arg,"%1023[^,],%1023s",label,url)==1) std::strcpy(url,label); break;
            }
            cimg::strpare(label,' ',false,true);
            cimg::strpare(label,'\"',true);
            cimg::strunescape(label);
            cimg::strpare(url,' ',false,true);
            cimg::strpare(url,'\"',true);
            GtkWidget *const link = gtk_link_button_new_with_label(url,label);
            gtk_widget_show(link);
            gtk_button_set_alignment(GTK_BUTTON(link),alignment,0.5);
            gtk_table_attach(GTK_TABLE(table),link,0,3,(int)current_table_line,(int)current_table_line+1,
                             (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),GTK_SHRINK,0,0);
            found_valid_argument = true;
          }

          // Check for an horizontal separator.
          if (!found_valid_argument && !cimg::strcasecmp(argument_type,"separator")) {
            GtkWidget *const separator = gtk_hseparator_new();
            gtk_widget_show(separator);
            gtk_table_attach(GTK_TABLE(table),separator,0,3,(int)current_table_line,(int)current_table_line+1,
                             (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),GTK_SHRINK,0,0);
            found_valid_argument = true;
          }

          if (!found_valid_argument) {
            if (get_verbosity_mode()>1) {
              std::fprintf(cimg::output(),
                           "\n[gmic_gimp]./error/ Found invalid parameter type '%s' for argument '%s'.\n",
                           argument_type,argument_name);
              std::fflush(cimg::output());
            }
          } else ++current_table_line;
        } else break;
      }
      set_filter_nbparams(filter,current_argument);
    }
  }

  gtk_container_add(GTK_CONTAINER(right_frame),table);

  // Take care of the size of the parameter table.
  GtkRequisition requisition; gtk_widget_size_request(table,&requisition);
  gtk_widget_set_size_request(right_pane,cimg::max(450,requisition.width),-1);
  gtk_widget_show(dialog_window);
  set_preview_factor();

  // Set correct icon for fave button.
  if (fave_stock) gtk_widget_destroy(fave_stock);
  fave_stock = gtk_button_new_from_stock(!filter?GTK_STOCK_ABOUT:GTK_STOCK_ADD);
  GtkWidget *const fave_image = gtk_button_get_image(GTK_BUTTON(fave_stock));
  gtk_button_set_image(GTK_BUTTON(fave_add_button),fave_image);
  gtk_widget_show(fave_add_button);
  if (filter && filter>=indice_faves) gtk_widget_show(fave_delete_button);
  else gtk_widget_hide(fave_delete_button);
}

// Create main plug-in dialog window and wait for events.
//-------------------------------------------------------
bool create_dialog_gui() {

  // Init GUI_specific variables
  _create_dialog_gui = true;
  gimp_ui_init("gmic",true);
  event_infos = 0;

  // Create main dialog window with buttons.
  char dialog_title[64] = { 0 };
  cimg_snprintf(dialog_title,sizeof(dialog_title),"%s %d.%d.%d.%d%s%s%s - %s %u bits",
                t("G'MIC for GIMP"),
                gmic_version/1000,(gmic_version/100)%10,(gmic_version/10)%10,gmic_version%10,
                gmic_is_beta?" [beta - ":"",
                gmic_is_beta?__DATE__:"",
                gmic_is_beta?"]":"",
                cimg::stros(),
                sizeof(void*)==8?64:32);

  dialog_window = gimp_dialog_new(dialog_title,"gmic",0,(GtkDialogFlags)0,0,0,NULL);
  gimp_window_set_transient(GTK_WINDOW(dialog_window));

  g_signal_connect(dialog_window,"close",G_CALLBACK(on_dialog_cancel_clicked),0);
  g_signal_connect(dialog_window,"delete-event",G_CALLBACK(on_dialog_cancel_clicked),0);

  GtkWidget *const cancel_button = gtk_dialog_add_button(GTK_DIALOG(dialog_window),
                                                         GTK_STOCK_CANCEL,GTK_RESPONSE_CANCEL);
  g_signal_connect(cancel_button,"clicked",G_CALLBACK(on_dialog_cancel_clicked),0);

  GtkWidget *const maximize_button = gtk_dialog_add_button(GTK_DIALOG(dialog_window),
                                                           GTK_STOCK_FULLSCREEN,GTK_RESPONSE_NONE);
  g_signal_connect(maximize_button,"clicked",G_CALLBACK(on_dialog_maximize_button_clicked),0);

  GtkWidget *const reset_button = gtk_dialog_add_button(GTK_DIALOG(dialog_window),
                                                        GIMP_STOCK_RESET,GTK_RESPONSE_NONE);
  g_signal_connect(reset_button,"clicked",G_CALLBACK(on_dialog_reset_clicked),0);

  GtkWidget *apply_button = gtk_dialog_add_button(GTK_DIALOG(dialog_window),
                                                  GTK_STOCK_APPLY,GTK_RESPONSE_APPLY);
  g_signal_connect(apply_button,"clicked",G_CALLBACK(on_dialog_apply_clicked),0);

  GtkWidget *ok_button = gtk_dialog_add_button(GTK_DIALOG(dialog_window),
                                               GTK_STOCK_OK,GTK_RESPONSE_OK);
  g_signal_connect(ok_button,"clicked",G_CALLBACK(gtk_main_quit),0);

  GtkWidget *const dialog_hpaned = gtk_hpaned_new();
  gtk_widget_show(dialog_hpaned);
  gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog_window)->vbox),dialog_hpaned);

  // Create the left pane.
  left_pane = gtk_vbox_new(false,4);
  gtk_widget_show(left_pane);
  gtk_paned_pack1(GTK_PANED(dialog_hpaned),left_pane,true,false);

  GtkWidget *const image_align = gtk_alignment_new(0.1,0,0,0);
  gtk_widget_show(image_align);
  gtk_box_pack_end(GTK_BOX(left_pane),image_align,false,false,0);
  const unsigned int logo_width = 102, logo_height = 22;
  GdkPixbuf *const pixbuf = gdk_pixbuf_new_from_data(data_gmic_logo,GDK_COLORSPACE_RGB,
                                                     false,8,logo_width,logo_height,3*logo_width,0,0);
  GtkWidget *const image = gtk_image_new_from_pixbuf(pixbuf);
  gtk_widget_show(image);
  gtk_container_add(GTK_CONTAINER(image_align),image);

  GtkWidget *const left_align = gtk_alignment_new(0,0,0,0);
  gtk_widget_show(left_align);
  gtk_box_pack_end(GTK_BOX(left_pane),left_align,false,false,0);

  GtkWidget *const left_frame = gtk_frame_new(NULL);
  gtk_widget_show(left_frame);
  gtk_container_set_border_width(GTK_CONTAINER(left_frame),4);
  gtk_container_add(GTK_CONTAINER(left_align),left_frame);

  GtkWidget *const frame_title = gtk_label_new(NULL);
  gtk_widget_show(frame_title);
  gtk_label_set_markup(GTK_LABEL(frame_title),t("<b> Input / Output : </b>"));
  gtk_frame_set_label_widget(GTK_FRAME(left_frame),frame_title);

  GtkWidget *const left_table = gtk_table_new(5,1,false);
  gtk_widget_show(left_table);
  gtk_table_set_row_spacings(GTK_TABLE(left_table),6);
  gtk_table_set_col_spacings(GTK_TABLE(left_table),6);
  gtk_container_set_border_width(GTK_CONTAINER(left_table),8);
  gtk_container_add(GTK_CONTAINER(left_frame),left_table);

  GtkWidget *const input_combobox = gtk_combo_box_new_text();
  gtk_widget_show(input_combobox);
  gtk_combo_box_append_text(GTK_COMBO_BOX(input_combobox),t("Input layers..."));
  gtk_combo_box_append_text(GTK_COMBO_BOX(input_combobox),"-");
  gtk_combo_box_append_text(GTK_COMBO_BOX(input_combobox),t("None"));
  gtk_combo_box_append_text(GTK_COMBO_BOX(input_combobox),t("Active (default)"));
  gtk_combo_box_append_text(GTK_COMBO_BOX(input_combobox),t("All"));
  gtk_combo_box_append_text(GTK_COMBO_BOX(input_combobox),t("Active & below"));
  gtk_combo_box_append_text(GTK_COMBO_BOX(input_combobox),t("Active & above"));
  gtk_combo_box_append_text(GTK_COMBO_BOX(input_combobox),t("All visibles"));
  gtk_combo_box_append_text(GTK_COMBO_BOX(input_combobox),t("All invisibles"));
  gtk_combo_box_append_text(GTK_COMBO_BOX(input_combobox),t("All visibles (decr.)"));
  gtk_combo_box_append_text(GTK_COMBO_BOX(input_combobox),t("All invisibles (decr.)"));
  gtk_combo_box_append_text(GTK_COMBO_BOX(input_combobox),t("All (decr.)"));
  gtk_combo_box_set_active(GTK_COMBO_BOX(input_combobox),get_input_mode(false));
  gtk_table_attach_defaults(GTK_TABLE(left_table),input_combobox,0,1,0,1);
  g_signal_connect(input_combobox,"changed",G_CALLBACK(on_dialog_input_mode_changed),0);

  GtkWidget *const output_combobox = gtk_combo_box_new_text();
  gtk_widget_show(output_combobox);
  gtk_combo_box_append_text(GTK_COMBO_BOX(output_combobox),t("Output mode..."));
  gtk_combo_box_append_text(GTK_COMBO_BOX(output_combobox),"-");
  gtk_combo_box_append_text(GTK_COMBO_BOX(output_combobox),t("In place (default)"));
  gtk_combo_box_append_text(GTK_COMBO_BOX(output_combobox),t("New layer(s)"));
  gtk_combo_box_append_text(GTK_COMBO_BOX(output_combobox),t("New active layer(s)"));
  gtk_combo_box_append_text(GTK_COMBO_BOX(output_combobox),t("New image"));
  gtk_combo_box_set_active(GTK_COMBO_BOX(output_combobox),get_output_mode(false));
  gtk_table_attach_defaults(GTK_TABLE(left_table),output_combobox,0,1,1,2);
  g_signal_connect(output_combobox,"changed",G_CALLBACK(on_dialog_output_mode_changed),0);

  GtkWidget *const verbosity_combobox = gtk_combo_box_new_text();
  gtk_widget_show(verbosity_combobox);
  gtk_combo_box_append_text(GTK_COMBO_BOX(verbosity_combobox),t("Output messages..."));
  gtk_combo_box_append_text(GTK_COMBO_BOX(verbosity_combobox),"-");
  gtk_combo_box_append_text(GTK_COMBO_BOX(verbosity_combobox),t("Quiet (default)"));
  gtk_combo_box_append_text(GTK_COMBO_BOX(verbosity_combobox),t("Verbose (layer name)"));
  gtk_combo_box_append_text(GTK_COMBO_BOX(verbosity_combobox),t("Verbose (console)"));
  gtk_combo_box_append_text(GTK_COMBO_BOX(verbosity_combobox),t("Verbose (logfile)"));
  gtk_combo_box_append_text(GTK_COMBO_BOX(verbosity_combobox),t("Very verbose (console)"));
  gtk_combo_box_append_text(GTK_COMBO_BOX(verbosity_combobox),t("Very verbose (logfile)"));
  gtk_combo_box_append_text(GTK_COMBO_BOX(verbosity_combobox),t("Debug mode (console)"));
  gtk_combo_box_append_text(GTK_COMBO_BOX(verbosity_combobox),t("Debug mode (logfile)"));
  gtk_combo_box_set_active(GTK_COMBO_BOX(verbosity_combobox),get_verbosity_mode(false));
  gtk_table_attach_defaults(GTK_TABLE(left_table),verbosity_combobox,0,1,2,3);
  g_signal_connect(verbosity_combobox,"changed",G_CALLBACK(on_dialog_verbosity_mode_changed),0);

  GtkWidget *const preview_mode_combobox = gtk_combo_box_new_text();
  gtk_widget_show(preview_mode_combobox);
  gtk_combo_box_append_text(GTK_COMBO_BOX(preview_mode_combobox),t("Preview mode..."));
  gtk_combo_box_append_text(GTK_COMBO_BOX(preview_mode_combobox),"-");
  gtk_combo_box_append_text(GTK_COMBO_BOX(preview_mode_combobox),t("1st output (default)"));
  gtk_combo_box_append_text(GTK_COMBO_BOX(preview_mode_combobox),t("2nd output"));
  gtk_combo_box_append_text(GTK_COMBO_BOX(preview_mode_combobox),t("3rd output"));
  gtk_combo_box_append_text(GTK_COMBO_BOX(preview_mode_combobox),t("4th output"));
  gtk_combo_box_append_text(GTK_COMBO_BOX(preview_mode_combobox),t("1st -> 2nd"));
  gtk_combo_box_append_text(GTK_COMBO_BOX(preview_mode_combobox),t("1st -> 3rd"));
  gtk_combo_box_append_text(GTK_COMBO_BOX(preview_mode_combobox),t("1st -> 4th"));
  gtk_combo_box_append_text(GTK_COMBO_BOX(preview_mode_combobox),t("All outputs"));
  gtk_combo_box_set_active(GTK_COMBO_BOX(preview_mode_combobox),get_preview_mode(false));
  gtk_table_attach_defaults(GTK_TABLE(left_table),preview_mode_combobox,0,1,3,4);
  g_signal_connect(preview_mode_combobox,"changed",G_CALLBACK(on_dialog_preview_mode_changed),0);

  GtkWidget *const preview_size_combobox = gtk_combo_box_new_text();
  gtk_widget_show(preview_size_combobox);
  gtk_combo_box_append_text(GTK_COMBO_BOX(preview_size_combobox),t("Preview size..."));
  gtk_combo_box_append_text(GTK_COMBO_BOX(preview_size_combobox),"-");
  gtk_combo_box_append_text(GTK_COMBO_BOX(preview_size_combobox),t("Tiny"));
  gtk_combo_box_append_text(GTK_COMBO_BOX(preview_size_combobox),t("Small"));
  gtk_combo_box_append_text(GTK_COMBO_BOX(preview_size_combobox),t("Normal"));
  gtk_combo_box_append_text(GTK_COMBO_BOX(preview_size_combobox),t("Large"));
  gtk_combo_box_set_active(GTK_COMBO_BOX(preview_size_combobox),get_preview_size(false));
  gtk_table_attach_defaults(GTK_TABLE(left_table),preview_size_combobox,0,1,4,5);
  g_signal_connect(preview_size_combobox,"changed",G_CALLBACK(on_dialog_preview_size_changed),0);

  drawable_preview = gimp_drawable_get(gimp_image_get_active_drawable(image_id));
  gui_preview = 0;
  _gimp_preview_invalidate();
  g_signal_connect(dialog_window,"size-request",G_CALLBACK(on_dialog_resized),0);

  // Create the middle pane.
  GtkWidget *const mr_hpaned = gtk_hpaned_new();
  gtk_widget_show(mr_hpaned);
  gtk_paned_pack2(GTK_PANED(dialog_hpaned),mr_hpaned,true,true);

  GtkWidget *const middle_frame = gtk_frame_new(NULL);
  gtk_widget_show(middle_frame);
  gtk_container_set_border_width(GTK_CONTAINER(middle_frame),4);
  gtk_paned_add1(GTK_PANED(mr_hpaned),middle_frame);

  GtkWidget *const middle_pane = gtk_vbox_new(false,4);
  gtk_widget_show(middle_pane);
  gtk_container_add(GTK_CONTAINER(middle_frame),middle_pane);

  relabel_hbox = gtk_hbox_new(false,3);
  gtk_box_pack_start(GTK_BOX(middle_pane),relabel_hbox,false,false,0);
  relabel_entry = gtk_entry_new_with_max_length(255);
  gtk_widget_show(relabel_entry);
  gtk_box_pack_start(GTK_BOX(relabel_hbox),relabel_entry,false,true,0);
  GtkWidget *const relabel_button = gtk_button_new_with_label(t("Rename"));
  gtk_widget_show(relabel_button);
  gtk_box_pack_start(GTK_BOX(relabel_hbox),relabel_button,false,true,0);
  g_signal_connect(relabel_button,"clicked",G_CALLBACK(_on_filter_doubleclicked),0);
  g_signal_connect(relabel_entry,"activate",G_CALLBACK(_on_filter_doubleclicked),0);

  GtkWidget *const scrolled_window = gtk_scrolled_window_new(NULL,NULL);
  gtk_widget_show(scrolled_window);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start(GTK_BOX(middle_pane),scrolled_window,true,true,0);

  tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(tree_view_store));
  gtk_tree_view_set_enable_search(GTK_TREE_VIEW(tree_view),true);
#if GTK_CHECK_VERSION(2,18,0)
  gtk_widget_set_can_focus(tree_view,false);
#else
  GValue val = {0, {{0}, {0}}};
  g_value_init(&val,G_TYPE_BOOLEAN);
  g_value_set_boolean(&val,FALSE);
  g_object_set_property(G_OBJECT(tree_view),"can-focus",&val);
#endif
  gtk_widget_show(tree_view);
  gtk_container_add(GTK_CONTAINER(scrolled_window),tree_view);

  GtkWidget *const tree_hbox = gtk_hbox_new(false,6);
  gtk_widget_show(tree_hbox);
  gtk_box_pack_start(GTK_BOX(middle_pane),tree_hbox,false,false,0);

  fave_add_button = gtk_button_new();
  gtk_box_pack_start(GTK_BOX(tree_hbox),fave_add_button,false,false,0);
  g_signal_connect_swapped(fave_add_button,"clicked",G_CALLBACK(on_dialog_add_fave_clicked),tree_view);
  fave_delete_button = gtk_button_new();
  gtk_box_pack_start(GTK_BOX(tree_hbox),fave_delete_button,false,false,0);
  g_signal_connect_swapped(fave_delete_button,"clicked",G_CALLBACK(on_dialog_remove_fave_clicked),tree_view);
  delete_stock = gtk_button_new_from_stock(GTK_STOCK_DELETE);
  GtkWidget *const delete_image = gtk_button_get_image(GTK_BUTTON(delete_stock));
  gtk_button_set_image(GTK_BUTTON(fave_delete_button),delete_image);

  GtkWidget *const refresh_button = gtk_button_new();
  refresh_stock = gtk_button_new_from_stock(GTK_STOCK_REFRESH);
  GtkWidget *const refresh_image = gtk_button_get_image(GTK_BUTTON(refresh_stock));
  gtk_button_set_image(GTK_BUTTON(refresh_button),refresh_image);
  gtk_widget_show(refresh_button);
  gtk_box_pack_start(GTK_BOX(tree_hbox),refresh_button,false,false,0);
  g_signal_connect_swapped(refresh_button,"clicked",G_CALLBACK(on_dialog_refresh_clicked),tree_view);

  GtkWidget *const internet_checkbutton = gtk_check_button_new_with_mnemonic(t("Internet"));
  gtk_widget_show(internet_checkbutton);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(internet_checkbutton),get_net_update());
  gtk_box_pack_start(GTK_BOX(tree_hbox),internet_checkbutton,false,false,0);
  g_signal_connect(internet_checkbutton,"toggled",G_CALLBACK(on_dialog_net_update_toggled),0);

  tree_mode_button = gtk_button_new();
  gtk_box_pack_start(GTK_BOX(tree_hbox),tree_mode_button,false,false,0);
  g_signal_connect_swapped(tree_mode_button,"clicked",G_CALLBACK(on_dialog_tree_mode_clicked),tree_view);

  GtkTreeViewColumn *const column = gtk_tree_view_column_new();
  gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view),column);
  flush_tree_view(tree_view);
  GtkRequisition requisition; gtk_widget_size_request((GtkWidget*)tree_view,&requisition);
  gtk_widget_set_size_request((GtkWidget*)tree_view,cimg::max(210,requisition.width),-1);
  g_signal_connect(tree_view,"cursor-changed",G_CALLBACK(on_filter_selected),0);
  g_signal_connect(tree_view,"row-activated",G_CALLBACK(on_filter_doubleclicked),0);

  // Create the right pane.
  right_pane = gtk_scrolled_window_new(NULL,NULL);
  gtk_widget_show(right_pane);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(right_pane),
                                 GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
  gtk_paned_add2(GTK_PANED(mr_hpaned),right_pane);

  right_frame = gtk_frame_new(NULL);
  gtk_widget_show(right_frame);
  gtk_container_set_border_width(GTK_CONTAINER(right_frame),4);
  gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(right_pane),right_frame);

  // Show dialog window and wait for user response.
  create_parameters_gui(false);
  gtk_main();

  // Destroy dialog box widget and free resources.
  gtk_widget_destroy(dialog_window);
  if (tree_mode_stock) gtk_widget_destroy(tree_mode_stock);
  if (fave_stock) gtk_widget_destroy(fave_stock);
  if (delete_stock) gtk_widget_destroy(delete_stock);
  if (refresh_stock) gtk_widget_destroy(refresh_stock);
  if (event_infos) delete[] event_infos;
  return _create_dialog_gui;
}

// 'Run' function, required by the GIMP plug-in API.
//--------------------------------------------------
void gmic_run(const gchar *name, gint nparams, const GimpParam *param,
              gint *nreturn_vals, GimpParam **return_vals) {

  // Init plug-in variables.
  static GimpParam return_values[1];
  *return_vals  = return_values;
  *nreturn_vals = 1;
  return_values[0].type = GIMP_PDB_STATUS;
  cimg::unused(name,nparams);
  run_mode = (GimpRunMode)param[0].data.d_int32;
  set_logfile();
  set_locale();
  status = GIMP_PDB_SUCCESS;
#if cimg_OS==2
  cimg::curl_path("_gmic\\curl",true);
#endif
  const unsigned int ps = get_preview_size(false);
  if (ps) resize_preview(get_preview_size(true));
  else { // Try to guess best preview size.
    unsigned int h = 0;
    try { h = CImgDisplay::screen_height(); } catch (...) {}
    const unsigned int bps = h>=1024?4:h>=800?3:0;
    if (bps) { set_preview_size(bps); resize_preview(get_preview_size(true)); }
  }

  try {

    // Init filters and images.
    update_filters(false);
    image_id = param[1].data.d_drawable;
    gimp_tile_cache_ntiles(2*(gimp_image_width(image_id)/gimp_tile_width()+1));

    // Check for run mode.
    switch (run_mode) {

    case GIMP_RUN_INTERACTIVE : {
      if (create_dialog_gui()) {
        process_image(0,false);
        const char *const commands_line = get_commands_line(false);
        if (commands_line) { // Remember command line for the next use of the filter.
          char s_tmp[48] = { 0 };
          cimg_snprintf(s_tmp,sizeof(s_tmp),"gmic_commands_line%u",get_current_filter());
          gimp_set_data(s_tmp,commands_line,std::strlen(commands_line)+1);
        }
      }
    } break;

    case GIMP_RUN_WITH_LAST_VALS : {
      const unsigned int filter = get_current_filter();
      if (filter) {
        char s_tmp[48] = { 0 };
        cimg_snprintf(s_tmp,sizeof(s_tmp),"gmic_commands_line%u",filter);
        const unsigned int siz = 1U + gimp_get_data_size(s_tmp);
        CImg<char> commands_line(2*siz);
        *commands_line = 0;
        gimp_get_data(s_tmp,commands_line);
        process_image(commands_line,false);
        const char *const next_commands_line = get_commands_line(false);
        // Remember command line for the next use of the filter.
        if (next_commands_line) gimp_set_data(s_tmp,next_commands_line,std::strlen(next_commands_line)+1);
      }
    } break;

    case GIMP_RUN_NONINTERACTIVE : {
      const unsigned int _input_mode = get_input_mode(), _output_mode = get_output_mode();
      set_input_mode(param[3].data.d_int32 + 2);
      set_output_mode(0);
      process_image(param[4].data.d_string,false);
      set_input_mode(_input_mode + 2);
      set_output_mode(_output_mode + 2);
    } break;
    }

  } catch (CImgException &e) {
    GtkWidget *const
      message = gtk_message_dialog_new(0,GTK_DIALOG_MODAL,GTK_MESSAGE_ERROR,GTK_BUTTONS_OK,"%s",e.what());
    gtk_widget_show(message);
    gtk_dialog_run(GTK_DIALOG(message));
    gtk_widget_destroy(message);
    status = GIMP_PDB_CALLING_ERROR;
  }

  if (logfile) std::fclose(logfile);
  return_values[0].data.d_status = status;
}

// 'Query' function, required by the GIMP plug-in API.
//----------------------------------------------------
void gmic_query() {
  static const GimpParamDef args[] = {
    {GIMP_PDB_INT32,    (gchar*)"run_mode", (gchar*)"Interactive, non-interactive"},
    {GIMP_PDB_IMAGE,    (gchar*)"image",    (gchar*)"Input image"},
    {GIMP_PDB_DRAWABLE, (gchar*)"drawable", (gchar*)"Input drawable (unused)"},
    {GIMP_PDB_INT32,    (gchar*)"input",    (gchar*)"Input layers mode, when non-interactive"
     "(0=none, 1=active, 2=all, 3=active & below, 4=active & above, 5=all visibles, 6=all invisibles, "
     "7=all visibles (decr.), 8=all invisibles (decr.), 9=all (decr.))"},
    {GIMP_PDB_STRING,   (gchar*)"command",  (gchar*)"G'MIC command string, when non-interactive"},
  };

  set_locale();
  gimp_install_procedure("plug-in-gmic",             // name
                         "G'MIC",                    // blurb
                         "G'MIC",                    // help
                         "David Tschumperl\303\251", // author
                         "David Tschumperl\303\251", // copyright
                         "2014",                     // date
                         "_G'MIC...",                // menu_path
                         "RGB*, GRAY*",              // image_types
                         GIMP_PLUGIN,                // type
                         G_N_ELEMENTS(args),         // nparams
                         0,                          // nreturn_vals
                         args,                       // params
                         0);                         // return_vals

  gimp_plugin_menu_register("plug-in-gmic", "<Image>/Filters");
}

GimpPlugInInfo PLUG_IN_INFO = { 0, 0, gmic_query, gmic_run };
MAIN()
