/* Generic support functions for Xcftools
 *
 * This file was written by Henning Makholm <henning@makholm.net>
 * It is hereby in the public domain.
 * 
 * In jurisdictions that do not recognise grants of copyright to the
 * public domain: I, the author and (presumably, in those jurisdictions)
 * copyright holder, hereby permit anyone to distribute and use this code,
 * in source code or binary form, with or without modifications. This
 * permission is world-wide and irrevocable.
 *
 * Of course, I will not be liable for any errors or shortcomings in the
 * code, since I give it away without asking any compenstations.
 *
 * If you use or distribute this code, I would appreciate receiving
 * credit for writing it, in whichever way you find proper and customary.
 */

#include "xcftools.h"
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>

const char *progname = "$0" ;
int verboseFlag = 0 ;


static void  __ATTRIBUTE__((noreturn))
vFatalGeneric(int status,const char *format,va_list args)
{
  if( format ) {
    if( *format == '!' ) {
      vfprintf(stderr,format+1,args);
      fprintf(stderr,": %s\n",strerror(errno));
    } else {
      vfprintf(stderr,format,args);
      fputc('\n',stderr);
    }
  }
  exit(status);
}

void
FatalGeneric(int status,const char* format,...)
{
  va_list v; va_start(v,format);
  if( format ) fprintf(stderr,"%s: ",progname);
  vFatalGeneric(status,format,v);
}

void
FatalUnexpected(const char* format,...)
{
  va_list v; va_start(v,format);
  fprintf(stderr,"%s: ",progname);
  vFatalGeneric(127,format,v) ;
}

void
FatalBadXCF(const char* format,...)
{
  va_list v; va_start(v,format);
  fprintf(stderr,"%s: %s:\n ",progname,_("Corrupted or malformed XCF file"));
  vFatalGeneric(125,format,v) ;
}

void
xcfCheckspace(uint32_t addr,int spaceafter,const char *format,...)
{
  if( xcf_length < spaceafter || addr > xcf_length - spaceafter ) {
    va_list v; va_start(v,format);
    fprintf(stderr,"%s: %s\n ",progname,_("Corrupted or truncated XCF file"));
    fprintf(stderr,"(0x%" PRIXPTR " bytes): ",(uintptr_t)xcf_length);
    vFatalGeneric(125,format,v) ;
  }
}


void
FatalUnsupportedXCF(const char* format,...)
{
  va_list v; va_start(v,format);
  fprintf(stderr,"%s: %s\n ",progname,
          _("The image contains features not understood by this program:"));
  vFatalGeneric(123,format,v) ;
}

void
gpl_blurb(void)
{
  fprintf(stderr,PACKAGE_STRING "\n");
  fprintf(stderr,
          _("Type \"%s -h\" to get an option summary.\n"),progname);
  exit(1) ;
}

/* ******************************************************* */

void *
xcfmalloc(size_t size)
{
  void *ptr = malloc(size);
  if( !ptr )
    FatalUnexpected(_("Out of memory"));
  return ptr ;
}

void
xcffree(void *block)
{
  if( xcf_file &&
      (uint8_t*)block >= xcf_file &&
      (uint8_t*)block < xcf_file + xcf_length )
    ;
  else
    free(block);
}

/* ******************************************************* */

FILE *
openout(const char *name)
{
  FILE *newfile ;
  if( strcmp(name,"-") == 0 )
    return stdout ;
  newfile = fopen(name,"wb") ;
  if( newfile == NULL )
    FatalUnexpected(_("!Cannot create file %s"),name);
  return newfile ;
}

void
closeout(FILE *f,const char *name)
{
  if( f == NULL )
    return ;
  if( fflush(f) == 0 ) {
    errno = 0 ;
    if( !ferror(f) ) {
      if( fclose(f) == 0 )
        return ;
    } else if( errno == 0 ) {
      /* Attempt to coax a valid errno out of the standard library,
       * following an idea by Bruno Haible
       * http://lists.gnu.org/archive/html/bug-gnulib/2003-09/msg00157.html
       */
      if( fputc('\0', f) != EOF &&
          fflush(f) == 0 )
        errno = EIO ; /* Argh, everything succeds. Just call it an I/O error */
    }
  }
  FatalUnexpected(_("!Error writing file %s"),name);
}

        
      
    
