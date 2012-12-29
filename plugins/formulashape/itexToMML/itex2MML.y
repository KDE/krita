/*             itex2MML 1.4.8
 *   itex2MML.y last modified 9/21/2011
 */

%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "itex2MML.h"

#define YYSTYPE char *
#define YYPARSE_PARAM_TYPE char **
#define YYPARSE_PARAM ret_str

#define yytext itex2MML_yytext

 extern int yylex ();

 extern char * yytext;

 static void itex2MML_default_error (const char * msg)
   {
     if (msg)
       fprintf(stderr, "Line: %d Error: %s\n", itex2MML_lineno, msg);
   }

 void (*itex2MML_error) (const char * msg) = itex2MML_default_error;

 static void yyerror (char * s)
   {
     char * msg = itex2MML_copy3 (s, " at token ", yytext);
     if (itex2MML_error)
       (*itex2MML_error) (msg);
     itex2MML_free_string (msg);
   }

 /* Note: If length is 0, then buffer is treated like a string; otherwise only length bytes are written.
  */
 static void itex2MML_default_write (const char * buffer, unsigned long length)
   {
     if (buffer)
       {
	 if (length)
	   fwrite (buffer, 1, length, stdout);
	 else
	   fputs (buffer, stdout);
       }
   }

 static void itex2MML_default_write_mathml (const char * mathml)
   {
     if (itex2MML_write)
       (*itex2MML_write) (mathml, 0);
   }

#ifdef itex2MML_CAPTURE
    static char * itex2MML_output_string = "" ;

    const char * itex2MML_output ()
    {
        char * copy = (char *) malloc((itex2MML_output_string ? strlen(itex2MML_output_string) : 0) + 1);
        if (copy)
          {
           if (itex2MML_output_string)
             {
               strcpy(copy, itex2MML_output_string);
               if (*itex2MML_output_string != '\0')
                   free(itex2MML_output_string);
             }
           else
             copy[0] = 0;
           itex2MML_output_string = "";
          }
        return copy;
    }

 static void itex2MML_capture (const char * buffer, unsigned long length)
    {
     if (buffer)
       {
         if (length)
           {
              unsigned long first_length = itex2MML_output_string ? strlen(itex2MML_output_string) : 0;
              char * copy  = (char *) malloc(first_length + length + 1);
              if (copy)
                {
                  if (itex2MML_output_string)
                    {
                       strcpy(copy, itex2MML_output_string);
                       if (*itex2MML_output_string != '\0')
                          free(itex2MML_output_string);
                    }
                  else
                     copy[0] = 0;
                  strncat(copy, buffer, length);
                  itex2MML_output_string = copy;
                 }
            }
         else
            {
              char * copy = itex2MML_copy2(itex2MML_output_string, buffer);
              if (*itex2MML_output_string != '\0')
                 free(itex2MML_output_string);
              itex2MML_output_string = copy;
            }
        }
    }

    static void itex2MML_capture_mathml (const char * buffer)
    {
       char * temp = itex2MML_copy2(itex2MML_output_string, buffer);
       if (*itex2MML_output_string != '\0')
         free(itex2MML_output_string);
       itex2MML_output_string = temp;
    }
    void (*itex2MML_write) (const char * buffer, unsigned long length) = itex2MML_capture;
    void (*itex2MML_write_mathml) (const char * mathml) = itex2MML_capture_mathml;
#else
    void (*itex2MML_write) (const char * buffer, unsigned long length) = itex2MML_default_write;
    void (*itex2MML_write_mathml) (const char * mathml) = itex2MML_default_write_mathml;
#endif 

 char * itex2MML_empty_string = "";

 /* Create a copy of a string, adding space for extra chars
  */
 char * itex2MML_copy_string_extra (const char * str, unsigned extra)
   {
     char * copy = (char *) malloc(extra + (str ? strlen (str) : 0) + 1);
     if (copy)
       {
	 if (str)
	   strcpy(copy, str);
	 else
	   copy[0] = 0;
       }
     return copy ? copy : itex2MML_empty_string;
   }

 /* Create a copy of a string, appending two strings
  */
 char * itex2MML_copy3 (const char * first, const char * second, const char * third)
   {
     int  first_length =  first ? strlen( first) : 0;
     int second_length = second ? strlen(second) : 0;
     int  third_length =  third ? strlen( third) : 0;

     char * copy = (char *) malloc(first_length + second_length + third_length + 1);

     if (copy)
       {
	 if (first)
	   strcpy(copy, first);
	 else
	   copy[0] = 0;

	 if (second) strcat(copy, second);
	 if ( third) strcat(copy,  third);
       }
     return copy ? copy : itex2MML_empty_string;
   }

 /* Create a copy of a string, appending a second string
  */
 char * itex2MML_copy2 (const char * first, const char * second)
   {
     return itex2MML_copy3(first, second, 0);
   }

 /* Create a copy of a string
  */
 char * itex2MML_copy_string (const char * str)
   {
     return itex2MML_copy3(str, 0, 0);
   }

 /* Create a copy of a string, escaping unsafe characters for XML
  */
 char * itex2MML_copy_escaped (const char * str)
   {
     unsigned long length = 0;

     const char * ptr1 = str;

     char * ptr2 = 0;
     char * copy = 0;

     if ( str == 0) return itex2MML_empty_string;
     if (*str == 0) return itex2MML_empty_string;

     while (*ptr1)
       {
	 switch (*ptr1)
	   {
	   case '<':  /* &lt;   */
	   case '>':  /* &gt;   */
	     length += 4;
	     break;
	   case '&':  /* &amp;  */
	     length += 5;
	     break;
	   case '\'': /* &apos; */
	   case '"':  /* &quot; */
	   case '-':  /* &#x2d; */
	     length += 6;
	     break;
	   default:
	     length += 1;
	     break;
	   }
	 ++ptr1;
       }

     copy = (char *) malloc (length + 1);

     if (copy)
       {
	 ptr1 = str;
	 ptr2 = copy;

	 while (*ptr1)
	   {
	     switch (*ptr1)
	       {
	       case '<':
		 strcpy (ptr2, "&lt;");
		 ptr2 += 4;
		 break;
	       case '>':
		 strcpy (ptr2, "&gt;");
		 ptr2 += 4;
		 break;
	       case '&':  /* &amp;  */
		 strcpy (ptr2, "&amp;");
		 ptr2 += 5;
		 break;
	       case '\'': /* &apos; */
		 strcpy (ptr2, "&apos;");
		 ptr2 += 6;
		 break;
	       case '"':  /* &quot; */
		 strcpy (ptr2, "&quot;");
		 ptr2 += 6;
		 break;
	       case '-':  /* &#x2d; */
		 strcpy (ptr2, "&#x2d;");
		 ptr2 += 6;
		 break;
	       default:
		 *ptr2++ = *ptr1;
		 break;
	       }
	     ++ptr1;
	   }
	 *ptr2 = 0;
       }
     return copy ? copy : itex2MML_empty_string;
   }

 /* Create a hex character reference string corresponding to code
  */
 char * itex2MML_character_reference (unsigned long int code)
   {
#define ENTITY_LENGTH 10
     char * entity = (char *) malloc(ENTITY_LENGTH);
     sprintf(entity, "&#x%05lx;", code);
     return entity;
   }

 void itex2MML_free_string (char * str)
   {
     if (str && str != itex2MML_empty_string)
       free(str);
   }

%}

%left TEXOVER TEXATOP
%token CHAR STARTMATH STARTDMATH ENDMATH MI MIB MN MO SUP SUB MROWOPEN MROWCLOSE LEFT RIGHT BIG BBIG BIGG BBIGG BIGL BBIGL BIGGL BBIGGL FRAC TFRAC OPERATORNAME MATHOP MATHBIN MATHREL MOP MOL MOLL MOF MOR PERIODDELIM OTHERDELIM LEFTDELIM RIGHTDELIM MOS MOB SQRT ROOT BINOM TBINOM UNDER OVER OVERBRACE UNDERLINE UNDERBRACE UNDEROVER TENSOR MULTI ARRAYALIGN COLUMNALIGN ARRAY COLSEP ROWSEP ARRAYOPTS COLLAYOUT COLALIGN ROWALIGN ALIGN EQROWS EQCOLS ROWLINES COLLINES FRAME PADDING ATTRLIST ITALICS BOLD BOXED SLASHED RM BB ST END BBLOWERCHAR BBUPPERCHAR BBDIGIT CALCHAR FRAKCHAR CAL FRAK CLAP LLAP RLAP ROWOPTS TEXTSIZE SCSIZE SCSCSIZE DISPLAY TEXTSTY TEXTBOX TEXTSTRING XMLSTRING CELLOPTS ROWSPAN COLSPAN THINSPACE MEDSPACE THICKSPACE QUAD QQUAD NEGSPACE PHANTOM HREF UNKNOWNCHAR EMPTYMROW STATLINE TOOLTIP TOGGLE FGHIGHLIGHT BGHIGHLIGHT SPACE INTONE INTTWO INTTHREE BAR WIDEBAR VEC WIDEVEC HAT WIDEHAT CHECK WIDECHECK TILDE WIDETILDE DOT DDOT DDDOT DDDDOT UNARYMINUS UNARYPLUS BEGINENV ENDENV MATRIX PMATRIX BMATRIX BBMATRIX VMATRIX VVMATRIX SVG ENDSVG SMALLMATRIX CASES ALIGNED GATHERED SUBSTACK PMOD RMCHAR COLOR BGCOLOR XARROW OPTARGOPEN OPTARGCLOSE ITEXNUM RAISEBOX NEG

%%

doc:  xmlmmlTermList {/* all processing done in body*/};

xmlmmlTermList:
{/* nothing - do nothing*/}
| char {/* proc done in body*/}
| expression {/* all proc. in body*/}
| xmlmmlTermList char {/* all proc. in body*/}
| xmlmmlTermList expression {/* all proc. in body*/};

char: CHAR {printf("%s", $1);};

expression: STARTMATH ENDMATH {/* empty math group - ignore*/}
| STARTDMATH ENDMATH {/* ditto */}
| STARTMATH compoundTermList ENDMATH {
  char ** r = (char **) ret_str;
  char * s = itex2MML_copy3("<math xmlns='http://www.w3.org/1998/Math/MathML' display='inline'>", $2, "</math>");
  itex2MML_free_string($2);
  if (r) {
    (*r) = (s == itex2MML_empty_string) ? 0 : s;
  }
  else {
    if (itex2MML_write_mathml)
      (*itex2MML_write_mathml) (s);
    itex2MML_free_string(s);
  }
}
| STARTDMATH compoundTermList ENDMATH {
  char ** r = (char **) ret_str;
  char * s = itex2MML_copy3("<math xmlns='http://www.w3.org/1998/Math/MathML' display='block'>", $2, "</math>");
  itex2MML_free_string($2);
  if (r) {
    (*r) = (s == itex2MML_empty_string) ? 0 : s;
  }
  else {
    if (itex2MML_write_mathml)
      (*itex2MML_write_mathml) (s);
    itex2MML_free_string(s);
  }
};

compoundTermList: compoundTerm {
  $$ = itex2MML_copy_string($1);
  itex2MML_free_string($1);
}
| compoundTermList compoundTerm {
  $$ = itex2MML_copy2($1, $2);
  itex2MML_free_string($1);
  itex2MML_free_string($2);
};

compoundTerm: mob SUB closedTerm SUP closedTerm {
  if (itex2MML_displaymode == 1) {
    char * s1 = itex2MML_copy3("<munderover>", $1, " ");
    char * s2 = itex2MML_copy3($3, " ", $5);
    $$ = itex2MML_copy3(s1, s2, "</munderover>");
    itex2MML_free_string(s1);
    itex2MML_free_string(s2);
  }
  else {
    char * s1 = itex2MML_copy3("<msubsup>", $1, " ");
    char * s2 = itex2MML_copy3($3, " ", $5);
    $$ = itex2MML_copy3(s1, s2, "</msubsup>");
    itex2MML_free_string(s1);
    itex2MML_free_string(s2);
  }
  itex2MML_free_string($1);
  itex2MML_free_string($3);
  itex2MML_free_string($5);
}
| mob SUB closedTerm {
  if (itex2MML_displaymode == 1) {
    char * s1 = itex2MML_copy3("<munder>", $1, " ");
    $$ = itex2MML_copy3(s1, $3, "</munder>");
    itex2MML_free_string(s1);
  }
  else {
    char * s1 = itex2MML_copy3("<msub>", $1, " ");
    $$ = itex2MML_copy3(s1, $3, "</msub>");
    itex2MML_free_string(s1);
  }
  itex2MML_free_string($1);
  itex2MML_free_string($3);
}
| mob SUP closedTerm SUB closedTerm {
  if (itex2MML_displaymode == 1) {
    char * s1 = itex2MML_copy3("<munderover>", $1, " ");
    char * s2 = itex2MML_copy3($5, " ", $3);
    $$ = itex2MML_copy3(s1, s2, "</munderover>");
    itex2MML_free_string(s1);
    itex2MML_free_string(s2);
  }
  else {
    char * s1 = itex2MML_copy3("<msubsup>", $1, " ");
    char * s2 = itex2MML_copy3($5, " ", $3);
    $$ = itex2MML_copy3(s1, s2, "</msubsup>");
    itex2MML_free_string(s1);
    itex2MML_free_string(s2);
  }
  itex2MML_free_string($1);
  itex2MML_free_string($3);
  itex2MML_free_string($5);
}
| mob SUP closedTerm {
  if (itex2MML_displaymode == 1) {
    char * s1 = itex2MML_copy3("<mover>", $1, " ");
    $$ = itex2MML_copy3(s1, $3, "</mover>");
    itex2MML_free_string(s1);
  }
  else {
    char * s1 = itex2MML_copy3("<msup>", $1, " ");
    $$ = itex2MML_copy3(s1, $3, "</msup>");
    itex2MML_free_string(s1);
  }
  itex2MML_free_string($1);
  itex2MML_free_string($3);
}
|mib SUB closedTerm SUP closedTerm {
  if (itex2MML_displaymode == 1) {
    char * s1 = itex2MML_copy3("<munderover>", $1, " ");
    char * s2 = itex2MML_copy3($3, " ", $5);
    $$ = itex2MML_copy3(s1, s2, "</munderover>");
    itex2MML_free_string(s1);
    itex2MML_free_string(s2);
  }
  else {
    char * s1 = itex2MML_copy3("<msubsup>", $1, " ");
    char * s2 = itex2MML_copy3($3, " ", $5);
    $$ = itex2MML_copy3(s1, s2, "</msubsup>");
    itex2MML_free_string(s1);
    itex2MML_free_string(s2);
  }
  itex2MML_free_string($1);
  itex2MML_free_string($3);
  itex2MML_free_string($5);
}
| mib SUB closedTerm {
  if (itex2MML_displaymode == 1) {
    char * s1 = itex2MML_copy3("<munder>", $1, " ");
    $$ = itex2MML_copy3(s1, $3, "</munder>");
    itex2MML_free_string(s1);
  }
  else {
    char * s1 = itex2MML_copy3("<msub>", $1, " ");
    $$ = itex2MML_copy3(s1, $3, "</msub>");
    itex2MML_free_string(s1);
  }
  itex2MML_free_string($1);
  itex2MML_free_string($3);
}
| mib SUP closedTerm SUB closedTerm {
  if (itex2MML_displaymode == 1) {
    char * s1 = itex2MML_copy3("<munderover>", $1, " ");
    char * s2 = itex2MML_copy3($5, " ", $3);
    $$ = itex2MML_copy3(s1, s2, "</munderover>");
    itex2MML_free_string(s1);
    itex2MML_free_string(s2);
  }
  else {
    char * s1 = itex2MML_copy3("<msubsup>", $1, " ");
    char * s2 = itex2MML_copy3($5, " ", $3);
    $$ = itex2MML_copy3(s1, s2, "</msubsup>");
    itex2MML_free_string(s1);
    itex2MML_free_string(s2);
  }
  itex2MML_free_string($1);
  itex2MML_free_string($3);
  itex2MML_free_string($5);
}
| mib SUP closedTerm {
  if (itex2MML_displaymode == 1) {
    char * s1 = itex2MML_copy3("<mover>", $1, " ");
    $$ = itex2MML_copy3(s1, $3, "</mover>");
    itex2MML_free_string(s1);
  }
  else {
    char * s1 = itex2MML_copy3("<msup>", $1, " ");
    $$ = itex2MML_copy3(s1, $3, "</msup>");
    itex2MML_free_string(s1);
  }
  itex2MML_free_string($1);
  itex2MML_free_string($3);
}
| closedTerm SUB closedTerm SUP closedTerm {
  char * s1 = itex2MML_copy3("<msubsup>", $1, " ");
  char * s2 = itex2MML_copy3($3, " ", $5);
  $$ = itex2MML_copy3(s1, s2, "</msubsup>");
  itex2MML_free_string(s1);
  itex2MML_free_string(s2);
  itex2MML_free_string($1);
  itex2MML_free_string($3);
  itex2MML_free_string($5);
}
| closedTerm SUP closedTerm SUB closedTerm {
  char * s1 = itex2MML_copy3("<msubsup>", $1, " ");
  char * s2 = itex2MML_copy3($5, " ", $3);
  $$ = itex2MML_copy3(s1, s2, "</msubsup>");
  itex2MML_free_string(s1);
  itex2MML_free_string(s2);
  itex2MML_free_string($1);
  itex2MML_free_string($3);
  itex2MML_free_string($5);
}
| closedTerm SUB closedTerm {
  char * s1 = itex2MML_copy3("<msub>", $1, " ");
  $$ = itex2MML_copy3(s1, $3, "</msub>");
  itex2MML_free_string(s1);
  itex2MML_free_string($1);
  itex2MML_free_string($3);
}
| closedTerm SUP closedTerm {
  char * s1 = itex2MML_copy3("<msup>", $1, " ");
  $$ = itex2MML_copy3(s1, $3, "</msup>");
  itex2MML_free_string(s1);
  itex2MML_free_string($1);
  itex2MML_free_string($3);
}
| SUB closedTerm {
  $$ = itex2MML_copy3("<msub><mo></mo>", $2, "</msub>");
  itex2MML_free_string($2);
}
| SUP closedTerm {
  $$ = itex2MML_copy3("<msup><mo></mo>", $2, "</msup>");
  itex2MML_free_string($2);
}
| closedTerm {
  $$ = itex2MML_copy_string($1);
  itex2MML_free_string($1);
};

closedTerm: array
| unaryminus
| unaryplus
| mib
| mi {
  $$ = itex2MML_copy3("<mi>", $1, "</mi>");
  itex2MML_free_string($1);
}
| mn {
  $$ = itex2MML_copy3("<mn>", $1, "</mn>");
  itex2MML_free_string($1);
}
| mo 
| tensor
| multi
| mfrac
| binom
| msqrt 
| mroot
| raisebox
| munder
| mover
| bar
| vec
| hat
| dot
| ddot
| dddot
| ddddot
| check
| tilde
| moverbrace
| munderbrace
| munderline
| munderover
| emptymrow
| mathclap
| mathllap
| mathrlap
| displaystyle
| textstyle
| textsize
| scriptsize
| scriptscriptsize
| italics
| bold
| roman
| rmchars
| bbold
| frak
| slashed
| boxed
| cal
| space
| textstring
| thinspace
| medspace
| thickspace
| quad
| qquad
| negspace
| phantom
| href
| statusline
| tooltip
| toggle
| fghighlight
| bghighlight
| color
| texover
| texatop
| MROWOPEN closedTerm MROWCLOSE {
  $$ = itex2MML_copy_string($2);
  itex2MML_free_string($2);
}
| MROWOPEN compoundTermList MROWCLOSE {
  $$ = itex2MML_copy3("<mrow>", $2, "</mrow>");
  itex2MML_free_string($2);
}
| left compoundTermList right {
  char * s1 = itex2MML_copy3("<mrow>", $1, $2);
  $$ = itex2MML_copy3(s1, $3, "</mrow>");
  itex2MML_free_string(s1);
  itex2MML_free_string($1);
  itex2MML_free_string($2);
  itex2MML_free_string($3);
}
| mathenv
| substack
| pmod
| unrecognized;

left: LEFT LEFTDELIM {
  itex2MML_rowposn = 2;
  $$ = itex2MML_copy3("<mo>", $2, "</mo>");
  itex2MML_free_string($2);
}
| LEFT OTHERDELIM {
  itex2MML_rowposn = 2;
  $$ = itex2MML_copy3("<mo>", $2, "</mo>");
  itex2MML_free_string($2);
}
| LEFT PERIODDELIM {
  itex2MML_rowposn = 2;
  $$ = itex2MML_copy_string("");
  itex2MML_free_string($2);
};

right: RIGHT RIGHTDELIM {
  $$ = itex2MML_copy3("<mo>", $2, "</mo>");
  itex2MML_free_string($2);
}
| RIGHT OTHERDELIM {
  $$ = itex2MML_copy3("<mo>", $2, "</mo>");
  itex2MML_free_string($2);
}
| RIGHT PERIODDELIM {
  $$ = itex2MML_copy_string("");
  itex2MML_free_string($2);
};

bigdelim: BIG LEFTDELIM {
  itex2MML_rowposn = 2;
  $$ = itex2MML_copy3("<mo maxsize=\"1.2em\" minsize=\"1.2em\">", $2, "</mo>");
  itex2MML_free_string($2);
} 
| BIG RIGHTDELIM {
  $$ = itex2MML_copy3("<mo maxsize=\"1.2em\" minsize=\"1.2em\">", $2, "</mo>");
  itex2MML_free_string($2);
}
| BIG OTHERDELIM {
  $$ = itex2MML_copy3("<mo maxsize=\"1.2em\" minsize=\"1.2em\">", $2, "</mo>");
  itex2MML_free_string($2);
}
| BBIG LEFTDELIM {
  itex2MML_rowposn = 2;
  $$ = itex2MML_copy3("<mo maxsize=\"1.8em\" minsize=\"1.8em\">", $2, "</mo>");
  itex2MML_free_string($2);
}
| BBIG RIGHTDELIM {
  $$ = itex2MML_copy3("<mo maxsize=\"1.8em\" minsize=\"1.8em\">", $2, "</mo>");
  itex2MML_free_string($2);
}
| BBIG OTHERDELIM {
  $$ = itex2MML_copy3("<mo maxsize=\"1.8em\" minsize=\"1.8em\">", $2, "</mo>");
  itex2MML_free_string($2);
}
| BIGG LEFTDELIM {
  itex2MML_rowposn = 2;
  $$ = itex2MML_copy3("<mo maxsize=\"2.4em\" minsize=\"2.4em\">", $2, "</mo>");
  itex2MML_free_string($2);
} 
| BIGG RIGHTDELIM {
  $$ = itex2MML_copy3("<mo maxsize=\"2.4em\" minsize=\"2.4em\">", $2, "</mo>");
  itex2MML_free_string($2);
}
| BIGG OTHERDELIM {
  $$ = itex2MML_copy3("<mo maxsize=\"2.4em\" minsize=\"2.4em\">", $2, "</mo>");
  itex2MML_free_string($2);
}
| BBIGG LEFTDELIM {
  itex2MML_rowposn = 2;
  $$ = itex2MML_copy3("<mo maxsize=\"3em\" minsize=\"3em\">", $2, "</mo>");
  itex2MML_free_string($2);
}
| BBIGG RIGHTDELIM {
  $$ = itex2MML_copy3("<mo maxsize=\"3em\" minsize=\"3em\">", $2, "</mo>");
  itex2MML_free_string($2);
}
| BBIGG OTHERDELIM {
  $$ = itex2MML_copy3("<mo maxsize=\"3em\" minsize=\"3em\">", $2, "</mo>");
  itex2MML_free_string($2);
}
|BIGL LEFTDELIM {
  itex2MML_rowposn = 2;
  $$ = itex2MML_copy3("<mo maxsize=\"1.2em\" minsize=\"1.2em\">", $2, "</mo>");
  itex2MML_free_string($2);
}
| BIGL OTHERDELIM {
  itex2MML_rowposn = 2;
  $$ = itex2MML_copy3("<mo maxsize=\"1.2em\" minsize=\"1.2em\">", $2, "</mo>");
  itex2MML_free_string($2);
}
| BBIGL LEFTDELIM {
  itex2MML_rowposn = 2;
  $$ = itex2MML_copy3("<mo maxsize=\"1.8em\" minsize=\"1.8em\">", $2, "</mo>");
  itex2MML_free_string($2);
}
| BBIGL OTHERDELIM {
  itex2MML_rowposn = 2;
  $$ = itex2MML_copy3("<mo maxsize=\"1.8em\" minsize=\"1.8em\">", $2, "</mo>");
  itex2MML_free_string($2);
}
| BIGGL LEFTDELIM {
  itex2MML_rowposn = 2;
  $$ = itex2MML_copy3("<mo maxsize=\"2.4em\" minsize=\"2.4em\">", $2, "</mo>");
  itex2MML_free_string($2);
} 
| BIGGL OTHERDELIM {
  itex2MML_rowposn = 2;
  $$ = itex2MML_copy3("<mo maxsize=\"2.4em\" minsize=\"2.4em\">", $2, "</mo>");
  itex2MML_free_string($2);
}
| BBIGGL LEFTDELIM {
  itex2MML_rowposn = 2;
  $$ = itex2MML_copy3("<mo maxsize=\"3em\" minsize=\"3em\">", $2, "</mo>");
  itex2MML_free_string($2);
}
| BBIGGL OTHERDELIM {
  itex2MML_rowposn = 2;
  $$ = itex2MML_copy3("<mo maxsize=\"3em\" minsize=\"3em\">", $2, "</mo>");
  itex2MML_free_string($2);
};

unrecognized: UNKNOWNCHAR {
  $$ = itex2MML_copy_string("<merror><mtext>Unknown character</mtext></merror>");
};

unaryminus: UNARYMINUS {
  $$ = itex2MML_copy_string("<mo lspace=\"verythinmathspace\" rspace=\"0em\">&minus;</mo>");
};

unaryplus: UNARYPLUS {
  $$ = itex2MML_copy_string("<mo lspace=\"verythinmathspace\" rspace=\"0em\">+</mo>");
};

mi: MI;

mib: MIB {
  itex2MML_rowposn=2;
  $$ = itex2MML_copy3("<mi>", $1, "</mi>");
  itex2MML_free_string($1);
};

mn: MN
| ITEXNUM TEXTSTRING {
  itex2MML_rowposn = 2;
  $$ = itex2MML_copy_string($2);
  itex2MML_free_string($2);
};

mob: MOB {
  itex2MML_rowposn = 2;
  $$ = itex2MML_copy3("<mo lspace=\"thinmathspace\" rspace=\"thinmathspace\">", $1, "</mo>");
  itex2MML_free_string($1);
};

mo: mob
| bigdelim
| MO {
  itex2MML_rowposn = 2;
  $$ = itex2MML_copy3("<mo>", $1, "</mo>");
  itex2MML_free_string($1);
}
| MOL {
  itex2MML_rowposn = 2;
  $$ = itex2MML_copy3("<mo>", $1, "</mo>");
  itex2MML_free_string($1);
}
| MOLL {
  itex2MML_rowposn = 2;
  $$ = itex2MML_copy3("<mstyle scriptlevel=\"0\"><mo>", $1, "</mo></mstyle>");
  itex2MML_free_string($1);
}
| RIGHTDELIM {
  $$ = itex2MML_copy3("<mo stretchy=\"false\">", $1, "</mo>");
  itex2MML_free_string($1);
}
| LEFTDELIM {
  itex2MML_rowposn = 2;
  $$ = itex2MML_copy3("<mo stretchy=\"false\">", $1, "</mo>");
  itex2MML_free_string($1);
}
| OTHERDELIM {
  $$ = itex2MML_copy3("<mo stretchy=\"false\">", $1, "</mo>");
  itex2MML_free_string($1);
}
| MOF {
  $$ = itex2MML_copy3("<mo stretchy=\"false\">", $1, "</mo>");
  itex2MML_free_string($1);
}
| PERIODDELIM {
  $$ = itex2MML_copy3("<mo>", $1, "</mo>");
  itex2MML_free_string($1);
}
| MOS {
  itex2MML_rowposn=2;
  $$ = itex2MML_copy3("<mo lspace=\"mediummathspace\" rspace=\"mediummathspace\">", $1, "</mo>");
  itex2MML_free_string($1);
}
| MOP {
  itex2MML_rowposn = 2;
  $$ = itex2MML_copy3("<mo lspace=\"0em\" rspace=\"thinmathspace\">", $1, "</mo>");
  itex2MML_free_string($1);
}
| MOR {
  itex2MML_rowposn = 2;
  $$ = itex2MML_copy3("<mo lspace=\"verythinmathspace\">", $1, "</mo>");
  itex2MML_free_string($1);
}
| OPERATORNAME TEXTSTRING {
  itex2MML_rowposn = 2;
  $$ = itex2MML_copy3("<mo lspace=\"0em\" rspace=\"thinmathspace\">", $2, "</mo>");
  itex2MML_free_string($2);
}
| MATHOP TEXTSTRING {
  itex2MML_rowposn = 2;
  $$ = itex2MML_copy3("<mo lspace=\"thinmathspace\" rspace=\"thinmathspace\">", $2, "</mo>");
  itex2MML_free_string($2);
}
| MATHBIN TEXTSTRING {
  itex2MML_rowposn = 2;
  $$ = itex2MML_copy3("<mo lspace=\"mediummathspace\" rspace=\"mediummathspace\">", $2, "</mo>");
  itex2MML_free_string($2);
}
| MATHREL TEXTSTRING {
  itex2MML_rowposn = 2;
  $$ = itex2MML_copy3("<mo lspace=\"thickmathspace\" rspace=\"thickmathspace\">", $2, "</mo>");
  itex2MML_free_string($2);
};

space: SPACE ST INTONE END ST INTTWO END ST INTTHREE END {
  char * s1 = itex2MML_copy3("<mspace height=\"", $3, "ex\" depth=\"");
  char * s2 = itex2MML_copy3($6, "ex\" width=\"", $9);
  $$ = itex2MML_copy3(s1, s2, "em\"></mspace>");
  itex2MML_free_string(s1);
  itex2MML_free_string(s2);
  itex2MML_free_string($3);
  itex2MML_free_string($6);
  itex2MML_free_string($9);
};

statusline: STATLINE TEXTSTRING closedTerm {
  char * s1 = itex2MML_copy3("<maction actiontype=\"statusline\">", $3, "<mtext>");
  $$ = itex2MML_copy3(s1, $2, "</mtext></maction>");
  itex2MML_free_string(s1);
  itex2MML_free_string($2);
  itex2MML_free_string($3);
};

tooltip: TOOLTIP TEXTSTRING closedTerm {
  char * s1 = itex2MML_copy3("<maction actiontype=\"tooltip\">", $3, "<mtext>");
  $$ = itex2MML_copy3(s1, $2, "</mtext></maction>");
  itex2MML_free_string(s1);
  itex2MML_free_string($2);
  itex2MML_free_string($3);
};

toggle: TOGGLE closedTerm closedTerm {
  char * s1 = itex2MML_copy3("<maction actiontype=\"toggle\" selection=\"2\">", $2, " ");
  $$ = itex2MML_copy3(s1, $3, "</maction>");
  itex2MML_free_string(s1);
  itex2MML_free_string($2);
  itex2MML_free_string($3);
};

fghighlight: FGHIGHLIGHT ATTRLIST closedTerm {
  char * s1 = itex2MML_copy3("<maction actiontype=\"highlight\" other='color=", $2, "'>");
  $$ = itex2MML_copy3(s1, $3, "</maction>");
  itex2MML_free_string(s1);
  itex2MML_free_string($2);
  itex2MML_free_string($3);
};

bghighlight: BGHIGHLIGHT ATTRLIST closedTerm {
  char * s1 = itex2MML_copy3("<maction actiontype=\"highlight\" other='background=", $2, "'>");
  $$ = itex2MML_copy3(s1, $3, "</maction>");
  itex2MML_free_string(s1);
  itex2MML_free_string($2);
  itex2MML_free_string($3);
};

color: COLOR ATTRLIST compoundTermList {
  char * s1 = itex2MML_copy3("<mstyle mathcolor=", $2, ">");
  $$ = itex2MML_copy3(s1, $3, "</mstyle>");
  itex2MML_free_string(s1);
  itex2MML_free_string($2);
  itex2MML_free_string($3);
}
| BGCOLOR ATTRLIST compoundTermList {
  char * s1 = itex2MML_copy3("<mstyle mathbackground=", $2, ">");
  $$ = itex2MML_copy3(s1, $3, "</mstyle>");
  itex2MML_free_string(s1);
  itex2MML_free_string($2);
  itex2MML_free_string($3);
};

mathrlap: RLAP closedTerm {
  $$ = itex2MML_copy3("<mpadded width=\"0\">", $2, "</mpadded>");
  itex2MML_free_string($2);
};

mathllap: LLAP closedTerm {
  $$ = itex2MML_copy3("<mpadded width=\"0\" lspace=\"-100%width\">", $2, "</mpadded>");
  itex2MML_free_string($2);
};

mathclap: CLAP closedTerm {
  $$ = itex2MML_copy3("<mpadded width=\"0\" lspace=\"-50%width\">", $2, "</mpadded>");
  itex2MML_free_string($2);
};

textstring: TEXTBOX TEXTSTRING {
  $$ = itex2MML_copy3("<mtext>", $2, "</mtext>");
  itex2MML_free_string($2);
};

displaystyle: DISPLAY compoundTermList {
  $$ = itex2MML_copy3("<mstyle displaystyle=\"true\">", $2, "</mstyle>");
  itex2MML_free_string($2);
};

textstyle: TEXTSTY compoundTermList {
  $$ = itex2MML_copy3("<mstyle displaystyle=\"false\">", $2, "</mstyle>");
  itex2MML_free_string($2);
};

textsize: TEXTSIZE compoundTermList {
  $$ = itex2MML_copy3("<mstyle scriptlevel=\"0\">", $2, "</mstyle>");
  itex2MML_free_string($2);
};

scriptsize: SCSIZE compoundTermList {
  $$ = itex2MML_copy3("<mstyle scriptlevel=\"1\">", $2, "</mstyle>");
  itex2MML_free_string($2);
};

scriptscriptsize: SCSCSIZE compoundTermList {
  $$ = itex2MML_copy3("<mstyle scriptlevel=\"2\">", $2, "</mstyle>");
  itex2MML_free_string($2);
};

italics: ITALICS closedTerm {
  $$ = itex2MML_copy3("<mstyle mathvariant=\"italic\">", $2, "</mstyle>");
  itex2MML_free_string($2);
};

slashed: SLASHED closedTerm {
  $$ = itex2MML_copy3("<menclose notation=\"updiagonalstrike\">", $2, "</menclose>");
  itex2MML_free_string($2);
};

boxed: BOXED closedTerm {
  $$ = itex2MML_copy3("<menclose notation=\"box\">", $2, "</menclose>");
  itex2MML_free_string($2);
};

bold: BOLD closedTerm {
  $$ = itex2MML_copy3("<mstyle mathvariant=\"bold\">", $2, "</mstyle>");
  itex2MML_free_string($2);
};

roman: RM ST rmchars END {
  $$ = itex2MML_copy3("<mi mathvariant=\"normal\">", $3, "</mi>");
  itex2MML_free_string($3);
};

rmchars: RMCHAR {
  $$ = itex2MML_copy_string($1);
  itex2MML_free_string($1);
}
| rmchars RMCHAR {
  $$ = itex2MML_copy2($1, $2);
  itex2MML_free_string($1);
  itex2MML_free_string($2);
};

bbold: BB ST bbchars END {
  $$ = itex2MML_copy3("<mi>", $3, "</mi>");
  itex2MML_free_string($3);
};

bbchars: bbchar {
  $$ = itex2MML_copy_string($1);
  itex2MML_free_string($1);
}
| bbchars bbchar {
  $$ = itex2MML_copy2($1, $2);
  itex2MML_free_string($1);
  itex2MML_free_string($2);
};

bbchar: BBLOWERCHAR {
  $$ = itex2MML_copy3("&", $1, "opf;");
  itex2MML_free_string($1);
}
| BBUPPERCHAR {
  $$ = itex2MML_copy3("&", $1, "opf;");
  itex2MML_free_string($1);
}
| BBDIGIT {
  /* Blackboard digits 0-9 correspond to Unicode characters 0x1D7D8-0x1D7E1 */
  char * end = $1 + 1;
  int code = 0x1D7D8 + strtoul($1, &end, 10);
  $$ = itex2MML_character_reference(code);
  itex2MML_free_string($1);
};

frak: FRAK ST frakletters END {
  $$ = itex2MML_copy3("<mi>", $3, "</mi>");
  itex2MML_free_string($3);
};

frakletters: frakletter {
  $$ = itex2MML_copy_string($1);
  itex2MML_free_string($1);
}
| frakletters frakletter {
  $$ = itex2MML_copy2($1, $2);
  itex2MML_free_string($1);
  itex2MML_free_string($2);
};

frakletter: FRAKCHAR {
  $$ = itex2MML_copy3("&", $1, "fr;");
  itex2MML_free_string($1);
};

cal: CAL ST calletters END {
  $$ = itex2MML_copy3("<mi>", $3, "</mi>");
  itex2MML_free_string($3);
};

calletters: calletter {
  $$ = itex2MML_copy_string($1);
  itex2MML_free_string($1);
}
| calletters calletter {
  $$ = itex2MML_copy2($1, $2);
  itex2MML_free_string($1);
  itex2MML_free_string($2);
};

calletter: CALCHAR {
  $$ = itex2MML_copy3("&", $1, "scr;");
  itex2MML_free_string($1);
};

thinspace: THINSPACE {
  $$ = itex2MML_copy_string("<mspace width=\"thinmathspace\"></mspace>");
};

medspace: MEDSPACE {
  $$ = itex2MML_copy_string("<mspace width=\"mediummathspace\"></mspace>");
};

thickspace: THICKSPACE {
  $$ = itex2MML_copy_string("<mspace width=\"thickmathspace\"></mspace>");
};

quad: QUAD {
  $$ = itex2MML_copy_string("<mspace width=\"1em\"></mspace>");
};

qquad: QQUAD {
  $$ = itex2MML_copy_string("<mspace width=\"2em\"></mspace>");
};

negspace: NEGSPACE {
  $$ = itex2MML_copy_string("<mspace width=\"-0.1667 em\"></mspace>");
};

phantom: PHANTOM closedTerm {
  $$ = itex2MML_copy3("<mphantom>", $2, "</mphantom>");
  itex2MML_free_string($2);
};

href: HREF TEXTSTRING closedTerm {
  char * s1 = itex2MML_copy3("<mrow href=\"", $2, "\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" xlink:type=\"simple\" xlink:href=\"");
  char * s2 = itex2MML_copy3(s1, $2, "\">");
  $$ = itex2MML_copy3(s2, $3, "</mrow>");
  itex2MML_free_string(s1);
  itex2MML_free_string(s2);
  itex2MML_free_string($2);
  itex2MML_free_string($3);
};

tensor: TENSOR closedTerm MROWOPEN subsupList MROWCLOSE {
  char * s1 = itex2MML_copy3("<mmultiscripts>", $2, $4);
  $$ = itex2MML_copy2(s1, "</mmultiscripts>");
  itex2MML_free_string(s1);
  itex2MML_free_string($2);
  itex2MML_free_string($4);
}
| TENSOR closedTerm subsupList {
  char * s1 = itex2MML_copy3("<mmultiscripts>", $2, $3);
  $$ = itex2MML_copy2(s1, "</mmultiscripts>");
  itex2MML_free_string(s1);
  itex2MML_free_string($2);
  itex2MML_free_string($3);
};

multi: MULTI MROWOPEN subsupList MROWCLOSE closedTerm MROWOPEN subsupList MROWCLOSE {
  char * s1 = itex2MML_copy3("<mmultiscripts>", $5, $7);
  char * s2 = itex2MML_copy3("<mprescripts></mprescripts>", $3, "</mmultiscripts>");
  $$ = itex2MML_copy2(s1, s2);
  itex2MML_free_string(s1);
  itex2MML_free_string(s2);
  itex2MML_free_string($3);
  itex2MML_free_string($5);
  itex2MML_free_string($7);
}
| MULTI MROWOPEN subsupList MROWCLOSE closedTerm EMPTYMROW {
  char * s1 = itex2MML_copy2("<mmultiscripts>", $5);
  char * s2 = itex2MML_copy3("<mprescripts></mprescripts>", $3, "</mmultiscripts>");
  $$ = itex2MML_copy2(s1, s2);
  itex2MML_free_string(s1);
  itex2MML_free_string(s2);
  itex2MML_free_string($3);
  itex2MML_free_string($5);
}
| MULTI EMPTYMROW closedTerm MROWOPEN subsupList MROWCLOSE {
  char * s1 = itex2MML_copy3("<mmultiscripts>", $3, $5);
  $$ = itex2MML_copy2(s1, "</mmultiscripts>");
  itex2MML_free_string(s1);
  itex2MML_free_string($3);
  itex2MML_free_string($5); 
};

subsupList: subsupTerm {
  $$ = itex2MML_copy_string($1);
  itex2MML_free_string($1);
}
| subsupList subsupTerm {
  $$ = itex2MML_copy3($1, " ", $2);
  itex2MML_free_string($1);
  itex2MML_free_string($2);
};

subsupTerm: SUB closedTerm SUP closedTerm {
  $$ = itex2MML_copy3($2, " ", $4);
  itex2MML_free_string($2);
  itex2MML_free_string($4);
}
| SUB closedTerm {
  $$ = itex2MML_copy2($2, " <none></none>");
  itex2MML_free_string($2);
}
| SUP closedTerm {
  $$ = itex2MML_copy2("<none></none> ", $2);
  itex2MML_free_string($2);
}
| SUB SUP closedTerm {
  $$ = itex2MML_copy2("<none></none> ", $3);
  itex2MML_free_string($3);
};

mfrac: FRAC closedTerm closedTerm {
  char * s1 = itex2MML_copy3("<mfrac>", $2, $3);
  $$ = itex2MML_copy2(s1, "</mfrac>");
  itex2MML_free_string(s1);
  itex2MML_free_string($2);
  itex2MML_free_string($3);
}
| TFRAC closedTerm closedTerm {
  char * s1 = itex2MML_copy3("<mstyle displaystyle=\"false\"><mfrac>", $2, $3);
  $$ = itex2MML_copy2(s1, "</mfrac></mstyle>");
  itex2MML_free_string(s1);
  itex2MML_free_string($2);
  itex2MML_free_string($3);
};

pmod: PMOD closedTerm {
  $$ = itex2MML_copy3( "<mo lspace=\"mediummathspace\">(</mo><mo rspace=\"thinmathspace\">mod</mo>", $2, "<mo rspace=\"mediummathspace\">)</mo>");
  itex2MML_free_string($2);
}

texover: MROWOPEN compoundTermList TEXOVER compoundTermList MROWCLOSE {
  char * s1 = itex2MML_copy3("<mfrac><mrow>", $2, "</mrow><mrow>");
  $$ = itex2MML_copy3(s1, $4, "</mrow></mfrac>");
  itex2MML_free_string(s1);
  itex2MML_free_string($2);
  itex2MML_free_string($4);
}
| left compoundTermList TEXOVER compoundTermList right {
  char * s1 = itex2MML_copy3("<mrow>", $1, "<mfrac><mrow>");
  char * s2 = itex2MML_copy3($2, "</mrow><mrow>", $4);
  char * s3 = itex2MML_copy3("</mrow></mfrac>", $5, "</mrow>");
  $$ = itex2MML_copy3(s1, s2, s3);
  itex2MML_free_string(s1);
  itex2MML_free_string(s2);
  itex2MML_free_string(s3);
  itex2MML_free_string($1);
  itex2MML_free_string($2);
  itex2MML_free_string($4);
  itex2MML_free_string($5);
};

texatop: MROWOPEN compoundTermList TEXATOP compoundTermList MROWCLOSE {
  char * s1 = itex2MML_copy3("<mfrac linethickness=\"0\"><mrow>", $2, "</mrow><mrow>");
  $$ = itex2MML_copy3(s1, $4, "</mrow></mfrac>");
  itex2MML_free_string(s1);
  itex2MML_free_string($2);
  itex2MML_free_string($4);
}
| left compoundTermList TEXATOP compoundTermList right {
  char * s1 = itex2MML_copy3("<mrow>", $1, "<mfrac linethickness=\"0\"><mrow>");
  char * s2 = itex2MML_copy3($2, "</mrow><mrow>", $4);
  char * s3 = itex2MML_copy3("</mrow></mfrac>", $5, "</mrow>");
  $$ = itex2MML_copy3(s1, s2, s3);
  itex2MML_free_string(s1);
  itex2MML_free_string(s2);
  itex2MML_free_string(s3);
  itex2MML_free_string($1);
  itex2MML_free_string($2);
  itex2MML_free_string($4);
  itex2MML_free_string($5);
};

binom: BINOM closedTerm closedTerm {
  char * s1 = itex2MML_copy3("<mrow><mo>(</mo><mfrac linethickness=\"0\">", $2, $3);
  $$ = itex2MML_copy2(s1, "</mfrac><mo>)</mo></mrow>");
  itex2MML_free_string(s1);
  itex2MML_free_string($2);
  itex2MML_free_string($3);
}
| TBINOM closedTerm closedTerm {
  char * s1 = itex2MML_copy3("<mrow><mo>(</mo><mstyle displaystyle=\"false\"><mfrac linethickness=\"0\">", $2, $3);
  $$ = itex2MML_copy2(s1, "</mfrac></mstyle><mo>)</mo></mrow>");
  itex2MML_free_string(s1);
  itex2MML_free_string($2);
  itex2MML_free_string($3);
};

munderbrace: UNDERBRACE closedTerm {
  $$ = itex2MML_copy3("<munder>", $2, "<mo>&UnderBrace;</mo></munder>");
  itex2MML_free_string($2);
};

munderline: UNDERLINE closedTerm {
  $$ = itex2MML_copy3("<munder>", $2, "<mo>&#x00332;</mo></munder>");
  itex2MML_free_string($2);
};

moverbrace: OVERBRACE closedTerm {
  $$ = itex2MML_copy3("<mover>", $2, "<mo>&OverBrace;</mo></mover>");
  itex2MML_free_string($2);
};

bar: BAR closedTerm {
  $$ = itex2MML_copy3("<mover>", $2, "<mo stretchy=\"false\">&#x000AF;</mo></mover>");
  itex2MML_free_string($2);
}
| WIDEBAR closedTerm {
  $$ = itex2MML_copy3("<mover>", $2, "<mo>&#x000AF;</mo></mover>");
  itex2MML_free_string($2);
};

vec: VEC closedTerm {
  $$ = itex2MML_copy3("<mover>", $2, "<mo stretchy=\"false\">&RightVector;</mo></mover>");
  itex2MML_free_string($2);
}
| WIDEVEC closedTerm {
  $$ = itex2MML_copy3("<mover>", $2, "<mo>&RightVector;</mo></mover>");
  itex2MML_free_string($2);
};

dot: DOT closedTerm {
  $$ = itex2MML_copy3("<mover>", $2, "<mo>&dot;</mo></mover>");
  itex2MML_free_string($2);
};

ddot: DDOT closedTerm {
  $$ = itex2MML_copy3("<mover>", $2, "<mo>&Dot;</mo></mover>");
  itex2MML_free_string($2);
};

dddot: DDDOT closedTerm {
  $$ = itex2MML_copy3("<mover>", $2, "<mo>&tdot;</mo></mover>");
  itex2MML_free_string($2);
};

ddddot: DDDDOT closedTerm {
  $$ = itex2MML_copy3("<mover>", $2, "<mo>&DotDot;</mo></mover>");
  itex2MML_free_string($2);
};

tilde: TILDE closedTerm {
  $$ = itex2MML_copy3("<mover>", $2, "<mo stretchy=\"false\">&tilde;</mo></mover>");
  itex2MML_free_string($2);
}
| WIDETILDE closedTerm {
  $$ = itex2MML_copy3("<mover>", $2, "<mo>&tilde;</mo></mover>");
  itex2MML_free_string($2);
};

check: CHECK closedTerm {
  $$ = itex2MML_copy3("<mover>", $2, "<mo stretchy=\"false\">&#x2c7;</mo></mover>");
  itex2MML_free_string($2);
}
| WIDECHECK closedTerm {
  $$ = itex2MML_copy3("<mover>", $2, "<mo>&#x2c7;</mo></mover>");
  itex2MML_free_string($2);
};

hat: HAT closedTerm {
  $$ = itex2MML_copy3("<mover>", $2, "<mo stretchy=\"false\">&#x5E;</mo></mover>");
  itex2MML_free_string($2);
}
| WIDEHAT closedTerm {
  $$ = itex2MML_copy3("<mover>", $2, "<mo>&#x5E;</mo></mover>");
  itex2MML_free_string($2);
};

msqrt: SQRT closedTerm {
  $$ = itex2MML_copy3("<msqrt>", $2, "</msqrt>");
  itex2MML_free_string($2);
};

mroot: SQRT OPTARGOPEN compoundTermList OPTARGCLOSE closedTerm {
  char * s1 = itex2MML_copy3("<mroot>", $5, $3);
  $$ = itex2MML_copy2(s1, "</mroot>");
  itex2MML_free_string(s1);
  itex2MML_free_string($3);
  itex2MML_free_string($5);
}
| ROOT closedTerm closedTerm {
  char * s1 = itex2MML_copy3("<mroot>", $3, $2);
  $$ = itex2MML_copy2(s1, "</mroot>");
  itex2MML_free_string(s1);
  itex2MML_free_string($2);
  itex2MML_free_string($3);
};

raisebox: RAISEBOX TEXTSTRING TEXTSTRING TEXTSTRING closedTerm {
  char * s1 = itex2MML_copy3("<mpadded voffset='", $2, "' height='");
  char * s2 = itex2MML_copy3(s1, $3, "' depth='");
  char * s3 = itex2MML_copy3(s2, $4, "'>");
  $$ = itex2MML_copy3(s3, $5, "</mpadded>");
  itex2MML_free_string(s1);
  itex2MML_free_string(s2);
  itex2MML_free_string(s3);
  itex2MML_free_string($2);
  itex2MML_free_string($3);
  itex2MML_free_string($4);
  itex2MML_free_string($5);
}
| RAISEBOX NEG TEXTSTRING TEXTSTRING TEXTSTRING closedTerm {
  char * s1 = itex2MML_copy3("<mpadded voffset='-", $3, "' height='");
  char * s2 = itex2MML_copy3(s1, $4, "' depth='");
  char * s3 = itex2MML_copy3(s2, $5, "'>");
  $$ = itex2MML_copy3(s3, $6, "</mpadded>");
  itex2MML_free_string(s1);
  itex2MML_free_string(s2);
  itex2MML_free_string(s3);
  itex2MML_free_string($3);
  itex2MML_free_string($4);
  itex2MML_free_string($5);
  itex2MML_free_string($6);
}
| RAISEBOX TEXTSTRING TEXTSTRING closedTerm {
  char * s1 = itex2MML_copy3("<mpadded voffset='", $2, "' height='");
  char * s2 = itex2MML_copy3(s1, $3, "' depth='depth'>");
  $$ = itex2MML_copy3(s2, $4, "</mpadded>");
  itex2MML_free_string(s1);
  itex2MML_free_string(s2);
  itex2MML_free_string($2);
  itex2MML_free_string($3);
  itex2MML_free_string($4);
}
| RAISEBOX NEG TEXTSTRING TEXTSTRING closedTerm {
  char * s1 = itex2MML_copy3("<mpadded voffset='-", $3, "' height='");
  char * s2 = itex2MML_copy3(s1, $4, "' depth='+");
  char * s3 = itex2MML_copy3(s2, $3, "'>");
  $$ = itex2MML_copy3(s3, $5, "</mpadded>");
  itex2MML_free_string(s1);
  itex2MML_free_string(s2);
  itex2MML_free_string(s3);
  itex2MML_free_string($3);
  itex2MML_free_string($4);
  itex2MML_free_string($5);
}
| RAISEBOX TEXTSTRING closedTerm {
  char * s1 = itex2MML_copy3("<mpadded voffset='", $2, "' height='+");
  char * s2 = itex2MML_copy3(s1, $2, "' depth='depth'>");
  $$ = itex2MML_copy3(s2, $3, "</mpadded>");
  itex2MML_free_string(s1);
  itex2MML_free_string(s2);
  itex2MML_free_string($2);
  itex2MML_free_string($3);
}
| RAISEBOX NEG TEXTSTRING closedTerm {
  char * s1 = itex2MML_copy3("<mpadded voffset='-", $3, "' height='0pt' depth='+");
  char * s2 = itex2MML_copy3(s1, $3, "'>");
  $$ = itex2MML_copy3(s2, $4, "</mpadded>");
  itex2MML_free_string(s1);
  itex2MML_free_string(s2);
  itex2MML_free_string($3);
  itex2MML_free_string($4);
};

munder: XARROW OPTARGOPEN compoundTermList OPTARGCLOSE EMPTYMROW {
  char * s1 = itex2MML_copy3("<munder><mo>", $1, "</mo><mrow>");
  $$ = itex2MML_copy3(s1, $3, "</mrow></munder>");
  itex2MML_free_string(s1);
  itex2MML_free_string($1);
  itex2MML_free_string($3);
}
| UNDER closedTerm closedTerm {
  char * s1 = itex2MML_copy3("<munder>", $3, $2);
  $$ = itex2MML_copy2(s1, "</munder>");
  itex2MML_free_string(s1);
  itex2MML_free_string($2);
  itex2MML_free_string($3);
};

mover: XARROW closedTerm {
  char * s1 = itex2MML_copy3("<mover><mo>", $1, "</mo>");
  $$ =  itex2MML_copy3(s1, $2, "</mover>");
  itex2MML_free_string(s1);
  itex2MML_free_string($1);
  itex2MML_free_string($2);
}
| OVER closedTerm closedTerm {
  char * s1 = itex2MML_copy3("<mover>", $3, $2);
  $$ = itex2MML_copy2(s1, "</mover>");
  itex2MML_free_string(s1);
  itex2MML_free_string($2);
  itex2MML_free_string($3);
};

munderover: XARROW OPTARGOPEN compoundTermList OPTARGCLOSE closedTerm {
  char * s1 = itex2MML_copy3("<munderover><mo>", $1, "</mo><mrow>");
  char * s2 = itex2MML_copy3(s1, $3, "</mrow>");
  $$ = itex2MML_copy3(s2, $5, "</munderover>");
  itex2MML_free_string(s1);
  itex2MML_free_string(s2);
  itex2MML_free_string($1);
  itex2MML_free_string($3);
  itex2MML_free_string($5);
}
| UNDEROVER closedTerm closedTerm closedTerm {
  char * s1 = itex2MML_copy3("<munderover>", $4, $2);
  $$ = itex2MML_copy3(s1, $3, "</munderover>");
  itex2MML_free_string(s1);
  itex2MML_free_string($2);
  itex2MML_free_string($3);
  itex2MML_free_string($4);
};

emptymrow: EMPTYMROW {
  $$ = itex2MML_copy_string("<mrow></mrow>");
};

mathenv: BEGINENV MATRIX tableRowList ENDENV MATRIX {
  $$ = itex2MML_copy3("<mrow><mtable rowspacing=\"0.5ex\">", $3, "</mtable></mrow>");
  itex2MML_free_string($3);
}
|  BEGINENV GATHERED tableRowList ENDENV GATHERED {
  $$ = itex2MML_copy3("<mrow><mtable rowspacing=\"1.0ex\">", $3, "</mtable></mrow>");
  itex2MML_free_string($3);
}
| BEGINENV PMATRIX tableRowList ENDENV PMATRIX {
  $$ = itex2MML_copy3("<mrow><mo>(</mo><mrow><mtable rowspacing=\"0.5ex\">", $3, "</mtable></mrow><mo>)</mo></mrow>");
  itex2MML_free_string($3);
}
| BEGINENV BMATRIX tableRowList ENDENV BMATRIX {
  $$ = itex2MML_copy3("<mrow><mo>[</mo><mrow><mtable rowspacing=\"0.5ex\">", $3, "</mtable></mrow><mo>]</mo></mrow>");
  itex2MML_free_string($3);
}
| BEGINENV VMATRIX tableRowList ENDENV VMATRIX {
  $$ = itex2MML_copy3("<mrow><mo>&VerticalBar;</mo><mrow><mtable rowspacing=\"0.5ex\">", $3, "</mtable></mrow><mo>&VerticalBar;</mo></mrow>");
  itex2MML_free_string($3);
}
| BEGINENV BBMATRIX tableRowList ENDENV BBMATRIX {
  $$ = itex2MML_copy3("<mrow><mo>{</mo><mrow><mtable rowspacing=\"0.5ex\">", $3, "</mtable></mrow><mo>}</mo></mrow>");
  itex2MML_free_string($3);
}
| BEGINENV VVMATRIX tableRowList ENDENV VVMATRIX {
  $$ = itex2MML_copy3("<mrow><mo>&DoubleVerticalBar;</mo><mrow><mtable rowspacing=\"0.5ex\">", $3, "</mtable></mrow><mo>&DoubleVerticalBar;</mo></mrow>");
  itex2MML_free_string($3);
}
| BEGINENV SMALLMATRIX tableRowList ENDENV SMALLMATRIX {
  $$ = itex2MML_copy3("<mstyle scriptlevel=\"2\"><mrow><mtable rowspacing=\"0.5ex\">", $3, "</mtable></mrow></mstyle>");
  itex2MML_free_string($3);
}
| BEGINENV CASES tableRowList ENDENV CASES {
  $$ = itex2MML_copy3("<mrow><mo>{</mo><mrow><mtable columnalign=\"left left\">", $3, "</mtable></mrow></mrow>");
  itex2MML_free_string($3);
}
| BEGINENV ALIGNED tableRowList ENDENV ALIGNED {
  $$ = itex2MML_copy3("<mrow><mtable columnalign=\"right left right left right left right left right left\" columnspacing=\"0em\">", $3, "</mtable></mrow>");
  itex2MML_free_string($3);
}
| BEGINENV ARRAY ARRAYALIGN ST columnAlignList END tableRowList ENDENV ARRAY {
  char * s1 = itex2MML_copy3("<mtable rowspacing=\"0.5ex\" align=\"", $3, "\" columnalign=\"");
  char * s2 = itex2MML_copy3(s1, $5, "\">");
  $$ = itex2MML_copy3(s2, $7, "</mtable>");
  itex2MML_free_string(s1);
  itex2MML_free_string(s2);
  itex2MML_free_string($3);
  itex2MML_free_string($5);
  itex2MML_free_string($7);
}
| BEGINENV ARRAY ST columnAlignList END tableRowList ENDENV ARRAY {
  char * s1 = itex2MML_copy3("<mtable rowspacing=\"0.5ex\" columnalign=\"", $4, "\">");
  $$ = itex2MML_copy3(s1, $6, "</mtable>");
  itex2MML_free_string(s1);
  itex2MML_free_string($4);
  itex2MML_free_string($6);
}
| BEGINENV SVG XMLSTRING ENDSVG {
  $$ = itex2MML_copy3("<semantics><annotation-xml encoding=\"SVG1.1\">", $3, "</annotation-xml></semantics>");
  itex2MML_free_string($3);
}
| BEGINENV SVG ENDSVG {
  $$ = itex2MML_copy_string(" ");
};

columnAlignList: columnAlignList COLUMNALIGN {
  $$ = itex2MML_copy3($1, " ", $2);
  itex2MML_free_string($1);
  itex2MML_free_string($2);
}
| COLUMNALIGN {
  $$ = itex2MML_copy_string($1);
  itex2MML_free_string($1);
};

substack: SUBSTACK MROWOPEN tableRowList MROWCLOSE {
  $$ = itex2MML_copy3("<mrow><mtable columnalign=\"center\" rowspacing=\"0.5ex\">", $3, "</mtable></mrow>");
  itex2MML_free_string($3);
};

array: ARRAY MROWOPEN tableRowList MROWCLOSE {
  $$ = itex2MML_copy3("<mrow><mtable>", $3, "</mtable></mrow>");
  itex2MML_free_string($3);
}
| ARRAY MROWOPEN ARRAYOPTS MROWOPEN arrayopts MROWCLOSE tableRowList MROWCLOSE {
  char * s1 = itex2MML_copy3("<mrow><mtable ", $5, ">");
  $$ = itex2MML_copy3(s1, $7, "</mtable></mrow>");
  itex2MML_free_string(s1);
  itex2MML_free_string($5);
  itex2MML_free_string($7);
};

arrayopts: anarrayopt {
  $$ = itex2MML_copy_string($1);
  itex2MML_free_string($1);
}
| arrayopts anarrayopt {
  $$ = itex2MML_copy3($1, " ", $2);
  itex2MML_free_string($1);
  itex2MML_free_string($2);
};

anarrayopt: collayout {
  $$ = itex2MML_copy_string($1);
  itex2MML_free_string($1);
}
| colalign {
  $$ = itex2MML_copy_string($1);
  itex2MML_free_string($1);
}
| rowalign {
  $$ = itex2MML_copy_string($1);
  itex2MML_free_string($1);
}
| align {
  $$ = itex2MML_copy_string($1);
  itex2MML_free_string($1);
}
| eqrows {
  $$ = itex2MML_copy_string($1);
  itex2MML_free_string($1);
}
| eqcols {
  $$ = itex2MML_copy_string($1);
  itex2MML_free_string($1);
}
| rowlines {
  $$ = itex2MML_copy_string($1);
  itex2MML_free_string($1);
}
| collines {
  $$ = itex2MML_copy_string($1);
  itex2MML_free_string($1);
}
| frame {
  $$ = itex2MML_copy_string($1);
  itex2MML_free_string($1);
}
| padding {
  $$ = itex2MML_copy_string($1);
  itex2MML_free_string($1);
};

collayout: COLLAYOUT ATTRLIST {
  $$ = itex2MML_copy2("columnalign=", $2);
  itex2MML_free_string($2);
};

colalign: COLALIGN ATTRLIST {
  $$ = itex2MML_copy2("columnalign=", $2);
  itex2MML_free_string($2);
};

rowalign: ROWALIGN ATTRLIST {
  $$ = itex2MML_copy2("rowalign=", $2);
  itex2MML_free_string($2);
};

align: ALIGN ATTRLIST {
  $$ = itex2MML_copy2("align=", $2);
  itex2MML_free_string($2);
};

eqrows: EQROWS ATTRLIST {
  $$ = itex2MML_copy2("equalrows=", $2);
  itex2MML_free_string($2);
};

eqcols: EQCOLS ATTRLIST {
  $$ = itex2MML_copy2("equalcolumns=", $2);
  itex2MML_free_string($2);
};

rowlines: ROWLINES ATTRLIST {
  $$ = itex2MML_copy2("rowlines=", $2);
  itex2MML_free_string($2);
};

collines: COLLINES ATTRLIST {
  $$ = itex2MML_copy2("columnlines=", $2);
  itex2MML_free_string($2);
};

frame: FRAME ATTRLIST {
  $$ = itex2MML_copy2("frame=", $2);
  itex2MML_free_string($2);
};

padding: PADDING ATTRLIST {
  char * s1 = itex2MML_copy3("rowspacing=", $2, " columnspacing=");
  $$ = itex2MML_copy2(s1, $2);
  itex2MML_free_string(s1);
  itex2MML_free_string($2);
};

tableRowList: tableRow {
  $$ = itex2MML_copy_string($1);
  itex2MML_free_string($1);
}
| tableRowList ROWSEP tableRow {
  $$ = itex2MML_copy3($1, " ", $3);
  itex2MML_free_string($1);
  itex2MML_free_string($3);
};

tableRow: simpleTableRow {
  $$ = itex2MML_copy3("<mtr>", $1, "</mtr>");
  itex2MML_free_string($1);
}
| optsTableRow {
  $$ = itex2MML_copy_string($1);
  itex2MML_free_string($1);
};

simpleTableRow: tableCell {
  $$ = itex2MML_copy_string($1);
  itex2MML_free_string($1);
}
| simpleTableRow COLSEP tableCell {
  $$ = itex2MML_copy3($1, " ", $3);
  itex2MML_free_string($1);
  itex2MML_free_string($3);
};

optsTableRow: ROWOPTS MROWOPEN rowopts MROWCLOSE simpleTableRow {
  char * s1 = itex2MML_copy3("<mtr ", $3, ">");
  $$ = itex2MML_copy3(s1, $5, "</mtr>");
  itex2MML_free_string(s1);
  itex2MML_free_string($3);
  itex2MML_free_string($5);
};

rowopts: arowopt {
  $$ = itex2MML_copy_string($1);
  itex2MML_free_string($1);
}
| rowopts arowopt {
  $$ = itex2MML_copy3($1, " ", $2);
  itex2MML_free_string($1);
  itex2MML_free_string($2);
};

arowopt: colalign {
  $$ = itex2MML_copy_string($1);
  itex2MML_free_string($1);
}
| rowalign {
  $$ = itex2MML_copy_string($1);
  itex2MML_free_string($1);
};

tableCell:   {
  $$ = itex2MML_copy_string("<mtd></mtd>");
}
| compoundTermList {
  $$ = itex2MML_copy3("<mtd>", $1, "</mtd>");
  itex2MML_free_string($1);
}
| CELLOPTS MROWOPEN cellopts MROWCLOSE compoundTermList {
  char * s1 = itex2MML_copy3("<mtd ", $3, ">");
  $$ = itex2MML_copy3(s1, $5, "</mtd>");
  itex2MML_free_string(s1);
  itex2MML_free_string($3);
  itex2MML_free_string($5);
};

cellopts: acellopt {
  $$ = itex2MML_copy_string($1);
  itex2MML_free_string($1);
}
| cellopts acellopt {
  $$ = itex2MML_copy3($1, " ", $2);
  itex2MML_free_string($1);
  itex2MML_free_string($2);
};

acellopt: colalign {
  $$ = itex2MML_copy_string($1);
  itex2MML_free_string($1);
}
| rowalign {
  $$ = itex2MML_copy_string($1);
  itex2MML_free_string($1);
}
| rowspan {
  $$ = itex2MML_copy_string($1);
  itex2MML_free_string($1);
}
| colspan {
  $$ = itex2MML_copy_string($1);
  itex2MML_free_string($1);
};

rowspan: ROWSPAN ATTRLIST {
  $$ = itex2MML_copy2("rowspan=", $2);
  itex2MML_free_string($2);
};

colspan: COLSPAN ATTRLIST {
  $$ = itex2MML_copy2("columnspan=", $2);
  itex2MML_free_string($2);
};

%%

char * itex2MML_parse (const char * buffer, unsigned long length)
{
  char * mathml = 0;

  int result;

  itex2MML_setup (buffer, length);
  itex2MML_restart ();

  result = itex2MML_yyparse (&mathml);

  if (result && mathml) /* shouldn't happen? */
    {
      itex2MML_free_string (mathml);
      mathml = 0;
    }
  return mathml;
}

int itex2MML_filter (const char * buffer, unsigned long length)
{
  itex2MML_setup (buffer, length);
  itex2MML_restart ();

  return itex2MML_yyparse (0);
}

#define ITEX_DELIMITER_DOLLAR 0
#define ITEX_DELIMITER_DOUBLE 1
#define ITEX_DELIMITER_SQUARE 2

static char * itex2MML_last_error = 0;

static void itex2MML_keep_error (const char * msg)
{
  if (itex2MML_last_error)
    {
      itex2MML_free_string (itex2MML_last_error);
      itex2MML_last_error = 0;
    }
  itex2MML_last_error = itex2MML_copy_escaped (msg);
}

int itex2MML_html_filter (const char * buffer, unsigned long length)
{
  itex2MML_do_html_filter (buffer, length, 0);
}

int itex2MML_strict_html_filter (const char * buffer, unsigned long length)
{
  itex2MML_do_html_filter (buffer, length, 1);
}

int itex2MML_do_html_filter (const char * buffer, unsigned long length, const int forbid_markup)
{
  int result = 0;

  int type = 0;
  int skip = 0;
  int match = 0;

  const char * ptr1 = buffer;
  const char * ptr2 = 0;

  const char * end = buffer + length;

  char * mathml = 0;

  void (*save_error_fn) (const char * msg) = itex2MML_error;

  itex2MML_error = itex2MML_keep_error;

 _until_math:
  ptr2 = ptr1;

  while (ptr2 < end)
    {
      if (*ptr2 == '$') break;
      if ((*ptr2 == '\\') && (ptr2 + 1 < end))
	{
	  if (*(ptr2+1) == '[') break;
	}
      ++ptr2;
    }
  if (itex2MML_write && ptr2 > ptr1)
    (*itex2MML_write) (ptr1, ptr2 - ptr1);

  if (ptr2 == end) goto _finish;

 _until_html:
  ptr1 = ptr2;

  if (ptr2 + 1 < end)
    {
      if ((*ptr2 == '\\') && (*(ptr2+1) == '['))
	{
	  type = ITEX_DELIMITER_SQUARE;
	  ptr2 += 2;
	}
      else if ((*ptr2 == '$') && (*(ptr2+1) == '$'))
	{
	  type = ITEX_DELIMITER_DOUBLE;
	  ptr2 += 2;
	}
      else
	{
	  type = ITEX_DELIMITER_DOLLAR;
	  ptr2 += 2;
	}
    }
  else goto _finish;

  skip = 0;
  match = 0;

  while (ptr2 < end)
    {
      switch (*ptr2)
	{
	case '<':
	case '>':
	  if (forbid_markup == 1) skip = 1;
	  break;

	case '\\':
	  if (ptr2 + 1 < end)
	    {
	      if (*(ptr2 + 1) == '[')
		{
		  skip = 1;
		}
	      else if (*(ptr2 + 1) == ']')
		{
		  if (type == ITEX_DELIMITER_SQUARE)
		    {
		      ptr2 += 2;
		      match = 1;
		    }
		  else
		    {
		      skip = 1;
		    }
		}
	    }
	  break;

	case '$':
	  if (type == ITEX_DELIMITER_SQUARE)
	    {
	      skip = 1;
	    }
	  else if (ptr2 + 1 < end)
	    {
	      if (*(ptr2 + 1) == '$')
		{
		  if (type == ITEX_DELIMITER_DOLLAR)
		    {
		      ptr2++;
		      match = 1;
		    }
		  else
		    {
		      ptr2 += 2;
		      match = 1;
		    }
		}
	      else
		{
		  if (type == ITEX_DELIMITER_DOLLAR)
		    {
		      ptr2++;
		      match = 1;
		    }
		  else
		    {
		      skip = 1;
		    }
		}
	    }
	  else
	    {
	      if (type == ITEX_DELIMITER_DOLLAR)
		{
		  ptr2++;
		  match = 1;
		}
	      else
		{
		  skip = 1;
		}
	    }
	  break;

	default:
	  break;
	}
      if (skip || match) break;

      ++ptr2;
    }
  if (skip)
    {
      if (type == ITEX_DELIMITER_DOLLAR)
	{
	  if (itex2MML_write)
	    (*itex2MML_write) (ptr1, 1);
	  ptr1++;
	}
      else
	{
	  if (itex2MML_write)
	    (*itex2MML_write) (ptr1, 2);
	  ptr1 += 2;
	}
      goto _until_math;
    }
  if (match)
    {
      mathml = itex2MML_parse (ptr1, ptr2 - ptr1);

      if (mathml)
	{
	  if (itex2MML_write_mathml)
	    (*itex2MML_write_mathml) (mathml);
	  itex2MML_free_string (mathml);
	  mathml = 0;
	}
      else
	{
	  ++result;
	  if (itex2MML_write)
	    {
	      if (type == ITEX_DELIMITER_DOLLAR)
		(*itex2MML_write) ("<math xmlns='http://www.w3.org/1998/Math/MathML' display='inline'><merror><mtext>", 0);
	      else
		(*itex2MML_write) ("<math xmlns='http://www.w3.org/1998/Math/MathML' display='block'><merror><mtext>", 0);

	      (*itex2MML_write) (itex2MML_last_error, 0);
	      (*itex2MML_write) ("</mtext></merror></math>", 0);
	    }
	}
      ptr1 = ptr2;

      goto _until_math;
    }
  if (itex2MML_write)
    (*itex2MML_write) (ptr1, ptr2 - ptr1);

 _finish:
  if (itex2MML_last_error)
    {
      itex2MML_free_string (itex2MML_last_error);
      itex2MML_last_error = 0;
    }
  itex2MML_error = save_error_fn;

  return result;
}
