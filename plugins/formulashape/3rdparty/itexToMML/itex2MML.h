/*             itex2MML 1.4.10
 *   itex2MML.h last modified 6/16/2012
 */

#ifndef ITEX2MML_H
#define ITEX2MML_H

#define ITEX2MML_VERSION "1.4.10"

#ifdef __cplusplus
extern "C" {
#endif

  /* Step 1. Parse a buffer with itex source; return value is mathml, or 0 on failure (e.g., parse error).
   */
  extern char * itex2MML_parse (const char * buffer, unsigned long length);

  /* Step 2. Free the string from Step 1.
   */
  extern void   itex2MML_free_string (char * str);


  /* Alternatively, to filter generic source and converting embedded equations, use:
   */
  extern int    itex2MML_filter (const char * buffer, unsigned long length);

  extern int    itex2MML_html_filter (const char * buffer, unsigned long length);
  extern int    itex2MML_strict_html_filter (const char * buffer, unsigned long length);
  extern int itex2MML_do_html_filter (const char * buffer, unsigned long length, const int forbid_markup);


  /* To change output methods:
   *
   * Note: If length is 0, then buffer is treated like a string; otherwise only length bytes are written.
   */
  extern void (*itex2MML_write) (const char * buffer, unsigned long length); /* default writes to stdout */
  extern void (*itex2MML_write_mathml) (const char * mathml);                /* default calls itex2MML_write(mathml,0) */
  extern void (*itex2MML_error) (const char * msg);                          /* default writes to stderr */


  /* Other stuff:
   */
  extern void   itex2MML_setup (const char * buffer, unsigned long length);

  extern void   itex2MML_restart ();

  extern char * itex2MML_copy_string (const char * str);
  extern char * itex2MML_copy_string_extra (const char * str, unsigned extra);
  extern char * itex2MML_copy2 (const char * first, const char * second);
  extern char * itex2MML_copy3 (const char * first, const char * second, const char * third);
  extern char * itex2MML_copy_escaped (const char * str);

  extern char * itex2MML_empty_string;

  extern int    itex2MML_lineno;

  extern int    itex2MML_rowposn;
  extern int    itex2MML_displaymode;

#ifdef __cplusplus
}
#endif

#endif /* ! ITEX2MML_H */
