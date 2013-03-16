/* A Bison parser, made by GNU Bison 2.5.  */

/* Bison implementation for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2011 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.5"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0

/* Substitute the variable and function names.  */
#define yyparse         itex2MML_yyparse
#define yylex           itex2MML_yylex
#define yyerror         itex2MML_yyerror
#define yylval          itex2MML_yylval
#define yychar          itex2MML_yychar
#define yydebug         itex2MML_yydebug
#define yynerrs         itex2MML_yynerrs


/* Copy the first part of user declarations.  */

/* Line 268 of yacc.c  */
#line 5 "itex2MML.y"

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



/* Line 268 of yacc.c  */
#line 354 "y.tab.c"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     TEXATOP = 258,
     TEXOVER = 259,
     CHAR = 260,
     STARTMATH = 261,
     STARTDMATH = 262,
     ENDMATH = 263,
     MI = 264,
     MIB = 265,
     MN = 266,
     MO = 267,
     SUP = 268,
     SUB = 269,
     MROWOPEN = 270,
     MROWCLOSE = 271,
     LEFT = 272,
     RIGHT = 273,
     BIG = 274,
     BBIG = 275,
     BIGG = 276,
     BBIGG = 277,
     BIGL = 278,
     BBIGL = 279,
     BIGGL = 280,
     BBIGGL = 281,
     FRAC = 282,
     TFRAC = 283,
     OPERATORNAME = 284,
     MATHOP = 285,
     MATHBIN = 286,
     MATHREL = 287,
     MOP = 288,
     MOL = 289,
     MOLL = 290,
     MOF = 291,
     MOR = 292,
     PERIODDELIM = 293,
     OTHERDELIM = 294,
     LEFTDELIM = 295,
     RIGHTDELIM = 296,
     MOS = 297,
     MOB = 298,
     SQRT = 299,
     ROOT = 300,
     BINOM = 301,
     TBINOM = 302,
     UNDER = 303,
     OVER = 304,
     OVERBRACE = 305,
     UNDERLINE = 306,
     UNDERBRACE = 307,
     UNDEROVER = 308,
     TENSOR = 309,
     MULTI = 310,
     ARRAYALIGN = 311,
     COLUMNALIGN = 312,
     ARRAY = 313,
     COLSEP = 314,
     ROWSEP = 315,
     ARRAYOPTS = 316,
     COLLAYOUT = 317,
     COLALIGN = 318,
     ROWALIGN = 319,
     ALIGN = 320,
     EQROWS = 321,
     EQCOLS = 322,
     ROWLINES = 323,
     COLLINES = 324,
     FRAME = 325,
     PADDING = 326,
     ATTRLIST = 327,
     ITALICS = 328,
     BOLD = 329,
     BOXED = 330,
     SLASHED = 331,
     RM = 332,
     BB = 333,
     ST = 334,
     END = 335,
     BBLOWERCHAR = 336,
     BBUPPERCHAR = 337,
     BBDIGIT = 338,
     CALCHAR = 339,
     FRAKCHAR = 340,
     CAL = 341,
     FRAK = 342,
     CLAP = 343,
     LLAP = 344,
     RLAP = 345,
     ROWOPTS = 346,
     TEXTSIZE = 347,
     SCSIZE = 348,
     SCSCSIZE = 349,
     DISPLAY = 350,
     TEXTSTY = 351,
     TEXTBOX = 352,
     TEXTSTRING = 353,
     XMLSTRING = 354,
     CELLOPTS = 355,
     ROWSPAN = 356,
     COLSPAN = 357,
     THINSPACE = 358,
     MEDSPACE = 359,
     THICKSPACE = 360,
     QUAD = 361,
     QQUAD = 362,
     NEGSPACE = 363,
     PHANTOM = 364,
     HREF = 365,
     UNKNOWNCHAR = 366,
     EMPTYMROW = 367,
     STATLINE = 368,
     TOOLTIP = 369,
     TOGGLE = 370,
     FGHIGHLIGHT = 371,
     BGHIGHLIGHT = 372,
     SPACE = 373,
     INTONE = 374,
     INTTWO = 375,
     INTTHREE = 376,
     BAR = 377,
     WIDEBAR = 378,
     VEC = 379,
     WIDEVEC = 380,
     HAT = 381,
     WIDEHAT = 382,
     CHECK = 383,
     WIDECHECK = 384,
     TILDE = 385,
     WIDETILDE = 386,
     DOT = 387,
     DDOT = 388,
     DDDOT = 389,
     DDDDOT = 390,
     UNARYMINUS = 391,
     UNARYPLUS = 392,
     BEGINENV = 393,
     ENDENV = 394,
     MATRIX = 395,
     PMATRIX = 396,
     BMATRIX = 397,
     BBMATRIX = 398,
     VMATRIX = 399,
     VVMATRIX = 400,
     SVG = 401,
     ENDSVG = 402,
     SMALLMATRIX = 403,
     CASES = 404,
     ALIGNED = 405,
     GATHERED = 406,
     SUBSTACK = 407,
     PMOD = 408,
     RMCHAR = 409,
     COLOR = 410,
     BGCOLOR = 411,
     XARROW = 412,
     OPTARGOPEN = 413,
     OPTARGCLOSE = 414,
     ITEXNUM = 415,
     RAISEBOX = 416,
     NEG = 417
   };
#endif
/* Tokens.  */
#define TEXATOP 258
#define TEXOVER 259
#define CHAR 260
#define STARTMATH 261
#define STARTDMATH 262
#define ENDMATH 263
#define MI 264
#define MIB 265
#define MN 266
#define MO 267
#define SUP 268
#define SUB 269
#define MROWOPEN 270
#define MROWCLOSE 271
#define LEFT 272
#define RIGHT 273
#define BIG 274
#define BBIG 275
#define BIGG 276
#define BBIGG 277
#define BIGL 278
#define BBIGL 279
#define BIGGL 280
#define BBIGGL 281
#define FRAC 282
#define TFRAC 283
#define OPERATORNAME 284
#define MATHOP 285
#define MATHBIN 286
#define MATHREL 287
#define MOP 288
#define MOL 289
#define MOLL 290
#define MOF 291
#define MOR 292
#define PERIODDELIM 293
#define OTHERDELIM 294
#define LEFTDELIM 295
#define RIGHTDELIM 296
#define MOS 297
#define MOB 298
#define SQRT 299
#define ROOT 300
#define BINOM 301
#define TBINOM 302
#define UNDER 303
#define OVER 304
#define OVERBRACE 305
#define UNDERLINE 306
#define UNDERBRACE 307
#define UNDEROVER 308
#define TENSOR 309
#define MULTI 310
#define ARRAYALIGN 311
#define COLUMNALIGN 312
#define ARRAY 313
#define COLSEP 314
#define ROWSEP 315
#define ARRAYOPTS 316
#define COLLAYOUT 317
#define COLALIGN 318
#define ROWALIGN 319
#define ALIGN 320
#define EQROWS 321
#define EQCOLS 322
#define ROWLINES 323
#define COLLINES 324
#define FRAME 325
#define PADDING 326
#define ATTRLIST 327
#define ITALICS 328
#define BOLD 329
#define BOXED 330
#define SLASHED 331
#define RM 332
#define BB 333
#define ST 334
#define END 335
#define BBLOWERCHAR 336
#define BBUPPERCHAR 337
#define BBDIGIT 338
#define CALCHAR 339
#define FRAKCHAR 340
#define CAL 341
#define FRAK 342
#define CLAP 343
#define LLAP 344
#define RLAP 345
#define ROWOPTS 346
#define TEXTSIZE 347
#define SCSIZE 348
#define SCSCSIZE 349
#define DISPLAY 350
#define TEXTSTY 351
#define TEXTBOX 352
#define TEXTSTRING 353
#define XMLSTRING 354
#define CELLOPTS 355
#define ROWSPAN 356
#define COLSPAN 357
#define THINSPACE 358
#define MEDSPACE 359
#define THICKSPACE 360
#define QUAD 361
#define QQUAD 362
#define NEGSPACE 363
#define PHANTOM 364
#define HREF 365
#define UNKNOWNCHAR 366
#define EMPTYMROW 367
#define STATLINE 368
#define TOOLTIP 369
#define TOGGLE 370
#define FGHIGHLIGHT 371
#define BGHIGHLIGHT 372
#define SPACE 373
#define INTONE 374
#define INTTWO 375
#define INTTHREE 376
#define BAR 377
#define WIDEBAR 378
#define VEC 379
#define WIDEVEC 380
#define HAT 381
#define WIDEHAT 382
#define CHECK 383
#define WIDECHECK 384
#define TILDE 385
#define WIDETILDE 386
#define DOT 387
#define DDOT 388
#define DDDOT 389
#define DDDDOT 390
#define UNARYMINUS 391
#define UNARYPLUS 392
#define BEGINENV 393
#define ENDENV 394
#define MATRIX 395
#define PMATRIX 396
#define BMATRIX 397
#define BBMATRIX 398
#define VMATRIX 399
#define VVMATRIX 400
#define SVG 401
#define ENDSVG 402
#define SMALLMATRIX 403
#define CASES 404
#define ALIGNED 405
#define GATHERED 406
#define SUBSTACK 407
#define PMOD 408
#define RMCHAR 409
#define COLOR 410
#define BGCOLOR 411
#define XARROW 412
#define OPTARGOPEN 413
#define OPTARGCLOSE 414
#define ITEXNUM 415
#define RAISEBOX 416
#define NEG 417




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 343 of yacc.c  */
#line 720 "y.tab.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  190
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   4668

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  163
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  113
/* YYNRULES -- Number of rules.  */
#define YYNRULES  314
/* YYNRULES -- Number of states.  */
#define YYNSTATES  561

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   417

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   157,   158,   159,   160,   161,   162
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     6,     8,    10,    13,    16,    18,
      21,    24,    28,    32,    34,    37,    43,    47,    53,    57,
      63,    67,    73,    77,    83,    89,    93,    97,   100,   103,
     105,   107,   109,   111,   113,   115,   117,   119,   121,   123,
     125,   127,   129,   131,   133,   135,   137,   139,   141,   143,
     145,   147,   149,   151,   153,   155,   157,   159,   161,   163,
     165,   167,   169,   171,   173,   175,   177,   179,   181,   183,
     185,   187,   189,   191,   193,   195,   197,   199,   201,   203,
     205,   207,   209,   211,   213,   215,   217,   219,   221,   223,
     225,   227,   229,   231,   233,   235,   239,   243,   247,   249,
     251,   253,   255,   258,   261,   264,   267,   270,   273,   276,
     279,   282,   285,   288,   291,   294,   297,   300,   303,   306,
     309,   312,   315,   318,   321,   324,   327,   330,   333,   335,
     337,   339,   341,   343,   345,   348,   350,   352,   354,   356,
     358,   360,   362,   364,   366,   368,   370,   372,   374,   376,
     379,   382,   385,   388,   399,   403,   407,   411,   415,   419,
     423,   427,   430,   433,   436,   439,   442,   445,   448,   451,
     454,   457,   460,   463,   466,   471,   473,   476,   481,   483,
     486,   488,   490,   492,   497,   499,   502,   504,   509,   511,
     514,   516,   518,   520,   522,   524,   526,   528,   531,   535,
     541,   545,   554,   561,   568,   570,   573,   578,   581,   584,
     588,   592,   596,   599,   605,   611,   617,   623,   627,   631,
     634,   637,   640,   643,   646,   649,   652,   655,   658,   661,
     664,   667,   670,   673,   676,   679,   682,   685,   691,   695,
     701,   708,   713,   719,   723,   728,   734,   738,   741,   745,
     751,   756,   758,   764,   770,   776,   782,   788,   794,   800,
     806,   812,   818,   828,   837,   842,   846,   849,   851,   856,
     861,   870,   872,   875,   877,   879,   881,   883,   885,   887,
     889,   891,   893,   895,   898,   901,   904,   907,   910,   913,
     916,   919,   922,   925,   927,   931,   933,   935,   937,   941,
     947,   949,   952,   954,   956,   957,   959,   965,   967,   970,
     972,   974,   976,   978,   981
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     164,     0,    -1,   165,    -1,    -1,   166,    -1,   167,    -1,
     165,   166,    -1,   165,   167,    -1,     5,    -1,     6,     8,
      -1,     7,     8,    -1,     6,   168,     8,    -1,     7,   168,
       8,    -1,   169,    -1,   168,   169,    -1,   180,    14,   170,
      13,   170,    -1,   180,    14,   170,    -1,   180,    13,   170,
      14,   170,    -1,   180,    13,   170,    -1,   178,    14,   170,
      13,   170,    -1,   178,    14,   170,    -1,   178,    13,   170,
      14,   170,    -1,   178,    13,   170,    -1,   170,    14,   170,
      13,   170,    -1,   170,    13,   170,    14,   170,    -1,   170,
      14,   170,    -1,   170,    13,   170,    -1,    14,   170,    -1,
      13,   170,    -1,   170,    -1,   252,    -1,   175,    -1,   176,
      -1,   178,    -1,   177,    -1,   179,    -1,   181,    -1,   221,
      -1,   222,    -1,   225,    -1,   229,    -1,   242,    -1,   243,
      -1,   244,    -1,   245,    -1,   246,    -1,   233,    -1,   234,
      -1,   241,    -1,   235,    -1,   236,    -1,   237,    -1,   238,
      -1,   240,    -1,   239,    -1,   232,    -1,   230,    -1,   231,
      -1,   247,    -1,   248,    -1,   191,    -1,   190,    -1,   189,
      -1,   193,    -1,   194,    -1,   195,    -1,   196,    -1,   197,
      -1,   198,    -1,   201,    -1,   202,    -1,   203,    -1,   204,
      -1,   207,    -1,   199,    -1,   200,    -1,   210,    -1,   182,
      -1,   192,    -1,   213,    -1,   214,    -1,   215,    -1,   216,
      -1,   217,    -1,   218,    -1,   219,    -1,   220,    -1,   183,
      -1,   184,    -1,   185,    -1,   186,    -1,   187,    -1,   188,
      -1,   227,    -1,   228,    -1,    15,   170,    16,    -1,    15,
     168,    16,    -1,   171,   168,   172,    -1,   249,    -1,   251,
      -1,   226,    -1,   174,    -1,    17,    40,    -1,    17,    39,
      -1,    17,    38,    -1,    18,    41,    -1,    18,    39,    -1,
      18,    38,    -1,    19,    40,    -1,    19,    41,    -1,    19,
      39,    -1,    20,    40,    -1,    20,    41,    -1,    20,    39,
      -1,    21,    40,    -1,    21,    41,    -1,    21,    39,    -1,
      22,    40,    -1,    22,    41,    -1,    22,    39,    -1,    23,
      40,    -1,    23,    39,    -1,    24,    40,    -1,    24,    39,
      -1,    25,    40,    -1,    25,    39,    -1,    26,    40,    -1,
      26,    39,    -1,   111,    -1,   136,    -1,   137,    -1,     9,
      -1,    10,    -1,    11,    -1,   160,    98,    -1,    43,    -1,
     180,    -1,   173,    -1,    12,    -1,    34,    -1,    35,    -1,
      41,    -1,    40,    -1,    39,    -1,    36,    -1,    38,    -1,
      42,    -1,    33,    -1,    37,    -1,    29,    98,    -1,    30,
      98,    -1,    31,    98,    -1,    32,    98,    -1,   118,    79,
     119,    80,    79,   120,    80,    79,   121,    80,    -1,   113,
      98,   170,    -1,   114,    98,   170,    -1,   115,   170,   170,
      -1,   116,    72,   170,    -1,   117,    72,   170,    -1,   155,
      72,   168,    -1,   156,    72,   168,    -1,    90,   170,    -1,
      89,   170,    -1,    88,   170,    -1,    97,    98,    -1,    95,
     168,    -1,    96,   168,    -1,    92,   168,    -1,    93,   168,
      -1,    94,   168,    -1,    73,   170,    -1,    76,   170,    -1,
      75,   170,    -1,    74,   170,    -1,    77,    79,   203,    80,
      -1,   154,    -1,   203,   154,    -1,    78,    79,   205,    80,
      -1,   206,    -1,   205,   206,    -1,    81,    -1,    82,    -1,
      83,    -1,    87,    79,   208,    80,    -1,   209,    -1,   208,
     209,    -1,    85,    -1,    86,    79,   211,    80,    -1,   212,
      -1,   211,   212,    -1,    84,    -1,   103,    -1,   104,    -1,
     105,    -1,   106,    -1,   107,    -1,   108,    -1,   109,   170,
      -1,   110,    98,   170,    -1,    54,   170,    15,   223,    16,
      -1,    54,   170,   223,    -1,    55,    15,   223,    16,   170,
      15,   223,    16,    -1,    55,    15,   223,    16,   170,   112,
      -1,    55,   112,   170,    15,   223,    16,    -1,   224,    -1,
     223,   224,    -1,    14,   170,    13,   170,    -1,    14,   170,
      -1,    13,   170,    -1,    14,    13,   170,    -1,    27,   170,
     170,    -1,    28,   170,   170,    -1,   153,   170,    -1,    15,
     168,     4,   168,    16,    -1,   171,   168,     4,   168,   172,
      -1,    15,   168,     3,   168,    16,    -1,   171,   168,     3,
     168,   172,    -1,    46,   170,   170,    -1,    47,   170,   170,
      -1,    52,   170,    -1,    51,   170,    -1,    50,   170,    -1,
     122,   170,    -1,   123,   170,    -1,   124,   170,    -1,   125,
     170,    -1,   132,   170,    -1,   133,   170,    -1,   134,   170,
      -1,   135,   170,    -1,   130,   170,    -1,   131,   170,    -1,
     128,   170,    -1,   129,   170,    -1,   126,   170,    -1,   127,
     170,    -1,    44,   170,    -1,    44,   158,   168,   159,   170,
      -1,    45,   170,   170,    -1,   161,    98,    98,    98,   170,
      -1,   161,   162,    98,    98,    98,   170,    -1,   161,    98,
      98,   170,    -1,   161,   162,    98,    98,   170,    -1,   161,
      98,   170,    -1,   161,   162,    98,   170,    -1,   157,   158,
     168,   159,   112,    -1,    48,   170,   170,    -1,   157,   170,
      -1,    49,   170,   170,    -1,   157,   158,   168,   159,   170,
      -1,    53,   170,   170,   170,    -1,   112,    -1,   138,   140,
     265,   139,   140,    -1,   138,   151,   265,   139,   151,    -1,
     138,   141,   265,   139,   141,    -1,   138,   142,   265,   139,
     142,    -1,   138,   144,   265,   139,   144,    -1,   138,   143,
     265,   139,   143,    -1,   138,   145,   265,   139,   145,    -1,
     138,   148,   265,   139,   148,    -1,   138,   149,   265,   139,
     149,    -1,   138,   150,   265,   139,   150,    -1,   138,    58,
      56,    79,   250,    80,   265,   139,    58,    -1,   138,    58,
      79,   250,    80,   265,   139,    58,    -1,   138,   146,    99,
     147,    -1,   138,   146,   147,    -1,   250,    57,    -1,    57,
      -1,   152,    15,   265,    16,    -1,    58,    15,   265,    16,
      -1,    58,    15,    61,    15,   253,    16,   265,    16,    -1,
     254,    -1,   253,   254,    -1,   255,    -1,   256,    -1,   257,
      -1,   258,    -1,   259,    -1,   260,    -1,   261,    -1,   262,
      -1,   263,    -1,   264,    -1,    62,    72,    -1,    63,    72,
      -1,    64,    72,    -1,    65,    72,    -1,    66,    72,    -1,
      67,    72,    -1,    68,    72,    -1,    69,    72,    -1,    70,
      72,    -1,    71,    72,    -1,   266,    -1,   265,    60,   266,
      -1,   267,    -1,   268,    -1,   271,    -1,   267,    59,   271,
      -1,    91,    15,   269,    16,   267,    -1,   270,    -1,   269,
     270,    -1,   256,    -1,   257,    -1,    -1,   168,    -1,   100,
      15,   272,    16,   168,    -1,   273,    -1,   272,   273,    -1,
     256,    -1,   257,    -1,   274,    -1,   275,    -1,   101,    72,
      -1,   102,    72,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   284,   284,   287,   288,   289,   290,   291,   293,   295,
     296,   297,   310,   324,   328,   334,   353,   367,   386,   400,
     419,   433,   452,   466,   476,   486,   493,   500,   504,   508,
     513,   514,   515,   516,   517,   521,   525,   526,   527,   528,
     529,   530,   531,   532,   533,   534,   535,   536,   537,   538,
     539,   540,   541,   542,   543,   544,   545,   546,   547,   548,
     549,   550,   551,   552,   553,   554,   555,   556,   557,   558,
     559,   560,   561,   562,   563,   564,   565,   566,   567,   568,
     569,   570,   571,   572,   573,   574,   575,   576,   577,   578,
     579,   580,   581,   582,   583,   584,   588,   592,   600,   601,
     602,   603,   605,   610,   615,   621,   625,   629,   634,   639,
     643,   647,   652,   656,   660,   665,   669,   673,   678,   682,
     686,   691,   696,   701,   706,   711,   716,   721,   727,   731,
     735,   739,   741,   747,   748,   754,   760,   761,   762,   767,
     772,   777,   781,   786,   790,   794,   798,   803,   808,   813,
     818,   823,   828,   834,   845,   853,   861,   869,   877,   885,
     892,   900,   905,   910,   915,   920,   925,   930,   935,   940,
     945,   950,   955,   960,   965,   970,   974,   980,   985,   989,
     995,   999,  1003,  1011,  1016,  1020,  1026,  1031,  1036,  1040,
    1046,  1051,  1055,  1059,  1063,  1067,  1071,  1075,  1080,  1090,
    1097,  1105,  1115,  1124,  1132,  1136,  1142,  1147,  1151,  1155,
    1160,  1167,  1175,  1180,  1187,  1201,  1208,  1222,  1229,  1237,
    1242,  1247,  1252,  1256,  1261,  1265,  1270,  1275,  1280,  1285,
    1290,  1294,  1299,  1303,  1308,  1312,  1317,  1322,  1329,  1337,
    1350,  1363,  1373,  1385,  1394,  1404,  1411,  1419,  1426,  1434,
    1444,  1453,  1457,  1461,  1465,  1469,  1473,  1477,  1481,  1485,
    1489,  1493,  1497,  1507,  1514,  1518,  1522,  1527,  1532,  1537,
    1541,  1549,  1553,  1559,  1563,  1567,  1571,  1575,  1579,  1583,
    1587,  1591,  1595,  1600,  1605,  1610,  1615,  1620,  1625,  1630,
    1635,  1640,  1645,  1652,  1656,  1662,  1666,  1671,  1675,  1681,
    1689,  1693,  1699,  1703,  1708,  1711,  1715,  1723,  1727,  1733,
    1737,  1741,  1745,  1750,  1755
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "TEXATOP", "TEXOVER", "CHAR",
  "STARTMATH", "STARTDMATH", "ENDMATH", "MI", "MIB", "MN", "MO", "SUP",
  "SUB", "MROWOPEN", "MROWCLOSE", "LEFT", "RIGHT", "BIG", "BBIG", "BIGG",
  "BBIGG", "BIGL", "BBIGL", "BIGGL", "BBIGGL", "FRAC", "TFRAC",
  "OPERATORNAME", "MATHOP", "MATHBIN", "MATHREL", "MOP", "MOL", "MOLL",
  "MOF", "MOR", "PERIODDELIM", "OTHERDELIM", "LEFTDELIM", "RIGHTDELIM",
  "MOS", "MOB", "SQRT", "ROOT", "BINOM", "TBINOM", "UNDER", "OVER",
  "OVERBRACE", "UNDERLINE", "UNDERBRACE", "UNDEROVER", "TENSOR", "MULTI",
  "ARRAYALIGN", "COLUMNALIGN", "ARRAY", "COLSEP", "ROWSEP", "ARRAYOPTS",
  "COLLAYOUT", "COLALIGN", "ROWALIGN", "ALIGN", "EQROWS", "EQCOLS",
  "ROWLINES", "COLLINES", "FRAME", "PADDING", "ATTRLIST", "ITALICS",
  "BOLD", "BOXED", "SLASHED", "RM", "BB", "ST", "END", "BBLOWERCHAR",
  "BBUPPERCHAR", "BBDIGIT", "CALCHAR", "FRAKCHAR", "CAL", "FRAK", "CLAP",
  "LLAP", "RLAP", "ROWOPTS", "TEXTSIZE", "SCSIZE", "SCSCSIZE", "DISPLAY",
  "TEXTSTY", "TEXTBOX", "TEXTSTRING", "XMLSTRING", "CELLOPTS", "ROWSPAN",
  "COLSPAN", "THINSPACE", "MEDSPACE", "THICKSPACE", "QUAD", "QQUAD",
  "NEGSPACE", "PHANTOM", "HREF", "UNKNOWNCHAR", "EMPTYMROW", "STATLINE",
  "TOOLTIP", "TOGGLE", "FGHIGHLIGHT", "BGHIGHLIGHT", "SPACE", "INTONE",
  "INTTWO", "INTTHREE", "BAR", "WIDEBAR", "VEC", "WIDEVEC", "HAT",
  "WIDEHAT", "CHECK", "WIDECHECK", "TILDE", "WIDETILDE", "DOT", "DDOT",
  "DDDOT", "DDDDOT", "UNARYMINUS", "UNARYPLUS", "BEGINENV", "ENDENV",
  "MATRIX", "PMATRIX", "BMATRIX", "BBMATRIX", "VMATRIX", "VVMATRIX", "SVG",
  "ENDSVG", "SMALLMATRIX", "CASES", "ALIGNED", "GATHERED", "SUBSTACK",
  "PMOD", "RMCHAR", "COLOR", "BGCOLOR", "XARROW", "OPTARGOPEN",
  "OPTARGCLOSE", "ITEXNUM", "RAISEBOX", "NEG", "$accept", "doc",
  "xmlmmlTermList", "char", "expression", "compoundTermList",
  "compoundTerm", "closedTerm", "left", "right", "bigdelim",
  "unrecognized", "unaryminus", "unaryplus", "mi", "mib", "mn", "mob",
  "mo", "space", "statusline", "tooltip", "toggle", "fghighlight",
  "bghighlight", "color", "mathrlap", "mathllap", "mathclap", "textstring",
  "displaystyle", "textstyle", "textsize", "scriptsize",
  "scriptscriptsize", "italics", "slashed", "boxed", "bold", "roman",
  "rmchars", "bbold", "bbchars", "bbchar", "frak", "frakletters",
  "frakletter", "cal", "calletters", "calletter", "thinspace", "medspace",
  "thickspace", "quad", "qquad", "negspace", "phantom", "href", "tensor",
  "multi", "subsupList", "subsupTerm", "mfrac", "pmod", "texover",
  "texatop", "binom", "munderbrace", "munderline", "moverbrace", "bar",
  "vec", "dot", "ddot", "dddot", "ddddot", "tilde", "check", "hat",
  "msqrt", "mroot", "raisebox", "munder", "mover", "munderover",
  "emptymrow", "mathenv", "columnAlignList", "substack", "array",
  "arrayopts", "anarrayopt", "collayout", "colalign", "rowalign", "align",
  "eqrows", "eqcols", "rowlines", "collines", "frame", "padding",
  "tableRowList", "tableRow", "simpleTableRow", "optsTableRow", "rowopts",
  "arowopt", "tableCell", "cellopts", "acellopt", "rowspan", "colspan", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,   386,   387,   388,   389,   390,   391,   392,   393,   394,
     395,   396,   397,   398,   399,   400,   401,   402,   403,   404,
     405,   406,   407,   408,   409,   410,   411,   412,   413,   414,
     415,   416,   417
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   163,   164,   165,   165,   165,   165,   165,   166,   167,
     167,   167,   167,   168,   168,   169,   169,   169,   169,   169,
     169,   169,   169,   169,   169,   169,   169,   169,   169,   169,
     170,   170,   170,   170,   170,   170,   170,   170,   170,   170,
     170,   170,   170,   170,   170,   170,   170,   170,   170,   170,
     170,   170,   170,   170,   170,   170,   170,   170,   170,   170,
     170,   170,   170,   170,   170,   170,   170,   170,   170,   170,
     170,   170,   170,   170,   170,   170,   170,   170,   170,   170,
     170,   170,   170,   170,   170,   170,   170,   170,   170,   170,
     170,   170,   170,   170,   170,   170,   170,   170,   170,   170,
     170,   170,   171,   171,   171,   172,   172,   172,   173,   173,
     173,   173,   173,   173,   173,   173,   173,   173,   173,   173,
     173,   173,   173,   173,   173,   173,   173,   173,   174,   175,
     176,   177,   178,   179,   179,   180,   181,   181,   181,   181,
     181,   181,   181,   181,   181,   181,   181,   181,   181,   181,
     181,   181,   181,   182,   183,   184,   185,   186,   187,   188,
     188,   189,   190,   191,   192,   193,   194,   195,   196,   197,
     198,   199,   200,   201,   202,   203,   203,   204,   205,   205,
     206,   206,   206,   207,   208,   208,   209,   210,   211,   211,
     212,   213,   214,   215,   216,   217,   218,   219,   220,   221,
     221,   222,   222,   222,   223,   223,   224,   224,   224,   224,
     225,   225,   226,   227,   227,   228,   228,   229,   229,   230,
     231,   232,   233,   233,   234,   234,   235,   236,   237,   238,
     239,   239,   240,   240,   241,   241,   242,   243,   243,   244,
     244,   244,   244,   244,   244,   245,   245,   246,   246,   247,
     247,   248,   249,   249,   249,   249,   249,   249,   249,   249,
     249,   249,   249,   249,   249,   249,   250,   250,   251,   252,
     252,   253,   253,   254,   254,   254,   254,   254,   254,   254,
     254,   254,   254,   255,   256,   257,   258,   259,   260,   261,
     262,   263,   264,   265,   265,   266,   266,   267,   267,   268,
     269,   269,   270,   270,   271,   271,   271,   272,   272,   273,
     273,   273,   273,   274,   275
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     0,     1,     1,     2,     2,     1,     2,
       2,     3,     3,     1,     2,     5,     3,     5,     3,     5,
       3,     5,     3,     5,     5,     3,     3,     2,     2,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     3,     3,     1,     1,
       1,     1,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     1,     1,
       1,     1,     1,     1,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     2,
       2,     2,     2,    10,     3,     3,     3,     3,     3,     3,
       3,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     4,     1,     2,     4,     1,     2,
       1,     1,     1,     4,     1,     2,     1,     4,     1,     2,
       1,     1,     1,     1,     1,     1,     1,     2,     3,     5,
       3,     8,     6,     6,     1,     2,     4,     2,     2,     3,
       3,     3,     2,     5,     5,     5,     5,     3,     3,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     5,     3,     5,
       6,     4,     5,     3,     4,     5,     3,     2,     3,     5,
       4,     1,     5,     5,     5,     5,     5,     5,     5,     5,
       5,     5,     9,     8,     4,     3,     2,     1,     4,     4,
       8,     1,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     1,     3,     1,     1,     1,     3,     5,
       1,     2,     1,     1,     0,     1,     5,     1,     2,     1,
       1,     1,     1,     2,     2
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       3,     8,     0,     0,     0,     2,     4,     5,     9,   131,
     132,   133,   138,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   147,   139,   140,   144,   148,   145,   143,   142,   141,
     146,   135,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   191,   192,   193,   194,   195,   196,     0,     0,
     128,   251,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   129,   130,     0,     0,     0,   175,     0,     0,
       0,     0,     0,     0,    13,    29,     0,   137,   101,    31,
      32,    34,    33,    35,   136,    36,    77,    87,    88,    89,
      90,    91,    92,    62,    61,    60,    78,    63,    64,    65,
      66,    67,    68,    74,    75,    69,    70,    71,    72,    73,
      76,    79,    80,    81,    82,    83,    84,    85,    86,    37,
      38,    39,   100,    93,    94,    40,    56,    57,    55,    46,
      47,    49,    50,    51,    52,    54,    53,    48,    41,    42,
      43,    44,    45,    58,    59,    98,    99,    30,    10,     0,
       1,     6,     7,    28,    33,   136,    27,     0,    29,   104,
     103,   102,   110,   108,   109,   113,   111,   112,   116,   114,
     115,   119,   117,   118,   121,   120,   123,   122,   125,   124,
     127,   126,     0,     0,   149,   150,   151,   152,     0,   236,
       0,     0,     0,     0,     0,   221,   220,   219,     0,     0,
       0,     0,   304,   170,   173,   172,   171,     0,     0,     0,
       0,   163,   162,   161,   167,   168,   169,   165,   166,   164,
     197,     0,     0,     0,     0,     0,     0,     0,   222,   223,
     224,   225,   234,   235,   232,   233,   230,   231,   226,   227,
     228,   229,     0,   304,   304,   304,   304,   304,   304,     0,
     304,   304,   304,   304,   304,   212,     0,     0,     0,   247,
     134,     0,     0,    11,    14,     0,     0,     0,     0,     0,
       0,     0,   176,    12,     0,     0,    96,    95,   210,   211,
       0,   238,   217,   218,   246,   248,     0,     0,     0,     0,
     200,   204,     0,     0,     0,     0,     0,   305,     0,   293,
     295,   296,   297,     0,   180,   181,   182,     0,   178,   190,
       0,   188,   186,     0,   184,   198,   154,   155,   156,   157,
     158,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   265,     0,     0,     0,     0,     0,   159,   160,     0,
       0,   243,     0,    26,    25,     0,     0,     0,    97,    22,
      20,    18,    16,     0,     0,     0,   250,   208,     0,   207,
       0,   205,     0,     0,     0,     0,     0,   269,   304,   304,
     174,   177,   179,   187,   189,   183,   185,     0,     0,   267,
       0,     0,     0,     0,     0,     0,     0,   264,     0,     0,
       0,     0,   268,     0,     0,   241,     0,   244,     0,     0,
       0,     0,   107,   106,   105,     0,     0,     0,     0,   215,
     213,   237,   209,     0,   199,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   271,   273,
     274,   275,   276,   277,   278,   279,   280,   281,   282,   302,
     303,     0,   300,     0,     0,   309,   310,     0,   307,   311,
     312,   294,   298,     0,     0,   266,   304,   252,   254,   255,
     257,   256,   258,   259,   260,   261,   253,   245,   249,   239,
       0,   242,    24,    23,   216,   214,    21,    19,    17,    15,
     206,     0,   202,   203,   283,   284,   285,   286,   287,   288,
     289,   290,   291,   292,   304,   272,   304,   301,   313,   314,
       0,   308,     0,   304,     0,   240,     0,     0,   299,   306,
       0,     0,     0,   201,   270,     0,     0,   263,     0,   262,
     153
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     4,     5,     6,     7,   337,   114,   115,   116,   388,
     117,   118,   119,   120,   121,   194,   123,   195,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,   139,   140,   141,   142,   143,   144,   145,   146,
     147,   148,   347,   348,   149,   353,   354,   150,   350,   351,
     151,   152,   153,   154,   155,   156,   157,   158,   159,   160,
     330,   331,   161,   162,   163,   164,   165,   166,   167,   168,
     169,   170,   171,   172,   173,   174,   175,   176,   177,   178,
     179,   180,   181,   182,   183,   184,   185,   420,   186,   187,
     467,   468,   469,   470,   471,   472,   473,   474,   475,   476,
     477,   478,   338,   339,   340,   341,   481,   482,   342,   487,
     488,   489,   490
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -396
static const yytype_int16 yypact[] =
{
     203,  -396,  1291,  1445,    27,   203,  -396,  -396,  -396,  -396,
    -396,  -396,  -396,  4354,  4354,  3130,   173,   180,   189,   192,
     195,    22,    92,   107,   141,  4354,  4354,   -20,   -18,    -9,
       3,  -396,  -396,  -396,  -396,  -396,  -396,  -396,  -396,  -396,
    -396,  -396,  3283,  4354,  4354,  4354,  4354,  4354,  4354,  4354,
    4354,  4354,  4354,   -13,    39,  4354,  4354,  4354,  4354,    14,
      58,    72,    76,  4354,  4354,  4354,  3130,  3130,  3130,  3130,
    3130,    70,  -396,  -396,  -396,  -396,  -396,  -396,  4354,    90,
    -396,  -396,   100,   104,  4354,    24,    38,   127,  4354,  4354,
    4354,  4354,  4354,  4354,  4354,  4354,  4354,  4354,  4354,  4354,
    4354,  4354,  -396,  -396,   -25,    67,  4354,  -396,   122,   150,
    3436,   148,   -64,  1599,  -396,    35,  3130,  -396,  -396,  -396,
    -396,  -396,   171,  -396,   211,  -396,  -396,  -396,  -396,  -396,
    -396,  -396,  -396,  -396,  -396,  -396,  -396,  -396,  -396,  -396,
    -396,  -396,  -396,  -396,  -396,  -396,  -396,    97,  -396,  -396,
    -396,  -396,  -396,  -396,  -396,  -396,  -396,  -396,  -396,  -396,
    -396,  -396,  -396,  -396,  -396,  -396,  -396,  -396,  -396,  -396,
    -396,  -396,  -396,  -396,  -396,  -396,  -396,  -396,  -396,  -396,
    -396,  -396,  -396,  -396,  -396,  -396,  -396,  -396,  -396,  1753,
    -396,  -396,  -396,  -396,  -396,  -396,  -396,   982,    31,  -396,
    -396,  -396,  -396,  -396,  -396,  -396,  -396,  -396,  -396,  -396,
    -396,  -396,  -396,  -396,  -396,  -396,  -396,  -396,  -396,  -396,
    -396,  -396,  4354,  4354,  -396,  -396,  -396,  -396,  3130,  -396,
    4354,  4354,  4354,  4354,  4354,  -396,  -396,  -396,  4354,   224,
     231,  4354,  1906,  -396,  -396,  -396,  -396,   103,   160,   174,
     175,  -396,  -396,  -396,  3130,  3130,  3130,  3130,  3130,  -396,
    -396,  4354,  4354,  4354,  4354,  4354,  4354,   140,  -396,  -396,
    -396,  -396,  -396,  -396,  -396,  -396,  -396,  -396,  -396,  -396,
    -396,  -396,   -16,  2059,  2059,  2059,  2059,  2059,  2059,   -87,
    2059,  2059,  2059,  2059,  2059,  -396,  3130,  3130,  3130,  -396,
    -396,  3589,   163,  -396,  -396,  4354,  4354,  1137,  4354,  4354,
    4354,  4354,  -396,  -396,  3130,  3130,  -396,  -396,  -396,  -396,
    2212,  -396,  -396,  -396,  -396,  -396,  4354,  4354,  3742,   231,
     231,  -396,   114,   248,   251,   253,   254,  3130,    10,  -396,
     212,  -396,  -396,   -60,  -396,  -396,  -396,   110,  -396,  -396,
     -38,  -396,  -396,   -44,  -396,  -396,  -396,  -396,  -396,  -396,
    -396,   193,   196,   215,   -56,   -52,   -31,   -30,   -17,   -10,
     129,  -396,    -5,    -4,    -3,    -1,    12,  3130,  3130,  2365,
    3895,  -396,  4048,   263,   265,  3130,  3130,   115,  -396,   279,
     266,   284,   286,  2518,  2671,  4354,  -396,  -396,  4354,   287,
     183,  -396,  4354,   231,   108,   184,    81,  -396,  2059,  2824,
    -396,  -396,  -396,  -396,  -396,  -396,  -396,   223,   215,  -396,
       1,   164,   162,   165,   166,   167,   161,  -396,   168,   159,
     169,   170,  -396,  4507,  4354,  -396,  4201,  -396,  4354,  4354,
    2977,  2977,  -396,  -396,  -396,  4354,  4354,  4354,  4354,  -396,
    -396,  -396,  -396,  4354,  -396,   -12,   187,   238,   242,   245,
     250,   252,   264,   268,   269,   270,   271,    96,  -396,  -396,
    -396,  -396,  -396,  -396,  -396,  -396,  -396,  -396,  -396,  -396,
    -396,    21,  -396,   272,   273,  -396,  -396,   -11,  -396,  -396,
    -396,  -396,  -396,   200,    17,  -396,  2059,  -396,  -396,  -396,
    -396,  -396,  -396,  -396,  -396,  -396,  -396,  -396,  -396,  -396,
    4354,  -396,  -396,  -396,  -396,  -396,  -396,  -396,  -396,  -396,
    -396,   231,  -396,  -396,  -396,  -396,  -396,  -396,  -396,  -396,
    -396,  -396,  -396,  -396,  2059,  -396,  2824,  -396,  -396,  -396,
    3130,  -396,   243,  2059,     9,  -396,   191,    19,   212,  3130,
     246,    11,   288,  -396,  -396,   205,   289,  -396,   275,  -396,
    -396
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -396,  -396,  -396,   343,   344,    36,   -40,   536,  -396,  -191,
    -396,  -396,  -396,  -396,  -396,    -2,  -396,    73,  -396,  -396,
    -396,  -396,  -396,  -396,  -396,  -396,  -396,  -396,  -396,  -396,
    -396,  -396,  -396,  -396,  -396,  -396,  -396,  -396,  -396,  -396,
     105,  -396,  -396,    15,  -396,  -396,    20,  -396,  -396,    18,
    -396,  -396,  -396,  -396,  -396,  -396,  -396,  -396,  -396,  -396,
    -234,  -323,  -396,  -396,  -396,  -396,  -396,  -396,  -396,  -396,
    -396,  -396,  -396,  -396,  -396,  -396,  -396,  -396,  -396,  -396,
    -396,  -396,  -396,  -396,  -396,  -396,  -396,   -46,  -396,  -396,
    -396,   -93,  -396,  -395,  -374,  -396,  -396,  -396,  -396,  -396,
    -396,  -396,  -269,   -29,  -158,  -396,  -396,  -100,   -27,  -396,
    -102,  -396,  -396
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint16 yytable[] =
{
     122,   122,   240,   521,   408,   540,   332,   401,   408,   401,
     479,   485,   370,   122,   364,   365,   366,   367,   368,   369,
     410,   372,   373,   374,   375,   376,   407,   190,   432,   408,
     408,   480,   486,   282,   301,   554,   415,   536,   113,   189,
     362,   352,   413,   408,   305,   306,   349,   317,   305,   306,
     408,   197,   458,   459,   242,   408,   408,   408,   495,   408,
     371,   214,   215,   363,   122,   122,   122,   122,   122,   408,
     408,   408,   408,   304,   495,   124,   124,   401,   224,   408,
     225,   496,   294,   421,   458,   459,   479,   422,   124,   226,
     483,   484,   485,   247,   312,   400,   265,   543,   302,   241,
     522,   227,   254,   255,   256,   257,   258,   480,   423,   424,
     266,   122,   534,   486,   122,   283,   284,   285,   286,   287,
     288,   289,   425,   290,   291,   292,   293,   327,   328,   426,
     402,   216,   217,   401,   428,   429,   430,   248,   431,   124,
     124,   124,   124,   124,   458,   459,   218,   219,   552,   304,
     556,   249,   307,   442,   443,   250,   444,   304,   457,   458,
     459,   460,   461,   462,   463,   464,   465,   466,   259,   456,
     457,   458,   459,   460,   461,   462,   463,   464,   465,   466,
     220,   221,   483,   484,   308,   309,   124,   122,   261,   124,
     411,   344,   345,   346,   296,   122,   327,   328,   262,   454,
     327,   328,   263,   523,   327,   328,   267,   553,     1,     2,
       3,   199,   200,   201,   304,   304,   304,   304,   304,   202,
     203,   204,   297,   401,   310,   311,   122,   544,   205,   206,
     207,   208,   209,   210,   211,   212,   213,   327,   328,   329,
     122,   344,   345,   346,   327,   328,   300,   458,   459,   514,
     515,   312,   122,   122,   122,   122,   122,   107,   349,   361,
     352,   382,   124,   403,   320,   547,   404,   304,   405,   406,
     124,   409,   419,   417,   551,   418,   427,   438,   439,   446,
     304,   122,   122,   122,   122,   122,   122,   546,   122,   122,
     122,   122,   122,   445,   122,   122,   122,   304,   447,   448,
     453,   124,   493,   498,   497,   122,   502,   499,   504,   500,
     524,   501,   122,   122,   525,   124,   503,   526,   122,   505,
     542,   506,   527,   550,   528,   555,   558,   124,   124,   124,
     124,   124,   377,   378,   379,   122,   529,   304,   304,   304,
     530,   531,   532,   533,   538,   539,   557,   559,   191,   192,
     393,   394,   343,   304,   304,   560,   124,   124,   124,   124,
     124,   124,   412,   124,   124,   124,   124,   124,   414,   124,
     124,   124,   494,   416,   535,   122,   122,   122,   548,   491,
     124,   537,   492,   122,   122,   541,     0,   124,   124,     0,
       0,   122,   122,   124,     0,     0,     0,     0,     0,     0,
     304,   304,     0,     0,     0,     0,   122,   122,     0,     0,
     124,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   440,   441,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   122,   122,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     124,   124,   124,     0,     0,     0,     0,     0,   124,   124,
       0,     0,     0,     0,     0,     0,   124,   124,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   124,   124,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   122,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   304,
       0,     0,     0,   124,   124,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   122,     0,   122,     0,     0,     0,   122,     0,
       0,   122,     0,     0,     0,     0,     0,   122,     0,   193,
     196,   198,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   222,   223,     0,     0,     0,     0,     0,     0,   124,
       0,     0,     0,     0,     0,     0,   549,     0,   229,   230,
     231,   232,   233,   234,   235,   236,   237,   238,   239,     0,
       0,   243,   244,   245,   246,     0,     0,     0,     0,   251,
     252,   253,     0,     0,     0,     0,     0,   124,     0,   124,
       0,     0,     0,   124,   260,     0,   124,     0,     0,     0,
     264,     0,   124,     0,   268,   269,   270,   271,   272,   273,
     274,   275,   276,   277,   278,   279,   280,   281,     0,     0,
       0,     0,   295,     0,     0,     0,   299,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   318,   319,
       0,     0,     0,     0,     0,     0,   321,   322,   323,   324,
     325,     0,     0,     0,   326,     0,     0,   333,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   355,   356,   357,
     358,   359,   360,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   381,     0,     0,
       0,   383,   384,     0,   389,   390,   391,   392,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   396,   397,   399,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   435,     0,   437,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   451,     0,     0,   452,     0,     0,     0,   455,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   508,
     509,     0,   511,     0,   512,   513,     0,     0,     0,     0,
       0,   516,   517,   518,   519,   314,   315,     0,     0,   520,
       0,     9,    10,    11,    12,    13,    14,    15,   316,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,     0,     0,
      54,     0,     0,     0,     0,     0,   545,     0,     0,     0,
       0,     0,     0,     0,     0,    55,    56,    57,    58,    59,
      60,     0,     0,     0,     0,     0,     0,     0,    61,    62,
      63,    64,    65,     0,    66,    67,    68,    69,    70,    71,
       0,     0,     0,     0,     0,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,     0,     0,     0,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   105,   106,   107,   108,   109,   110,
     385,   386,   111,   112,     0,     0,     9,    10,    11,    12,
      13,    14,    15,     0,    16,   387,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,     0,     0,    54,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      55,    56,    57,    58,    59,    60,     0,     0,     0,     0,
       0,     0,     0,    61,    62,    63,    64,    65,     0,    66,
      67,    68,    69,    70,    71,     0,     0,     0,     0,     0,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,     0,     0,     0,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   105,
     106,   107,   108,   109,   110,     0,     0,   111,   112,     8,
       9,    10,    11,    12,    13,    14,    15,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,     0,     0,    54,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    55,    56,    57,    58,    59,    60,
       0,     0,     0,     0,     0,     0,     0,    61,    62,    63,
      64,    65,     0,    66,    67,    68,    69,    70,    71,     0,
       0,     0,     0,     0,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
       0,     0,     0,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   105,   106,   107,   108,   109,   110,     0,
       0,   111,   112,   188,     9,    10,    11,    12,    13,    14,
      15,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,     0,     0,    54,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    55,    56,
      57,    58,    59,    60,     0,     0,     0,     0,     0,     0,
       0,    61,    62,    63,    64,    65,     0,    66,    67,    68,
      69,    70,    71,     0,     0,     0,     0,     0,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,     0,     0,     0,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   105,   106,   107,
     108,   109,   110,     0,     0,   111,   112,   303,     9,    10,
      11,    12,    13,    14,    15,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,     0,     0,    54,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    55,    56,    57,    58,    59,    60,     0,     0,
       0,     0,     0,     0,     0,    61,    62,    63,    64,    65,
       0,    66,    67,    68,    69,    70,    71,     0,     0,     0,
       0,     0,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,     0,     0,
       0,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   105,   106,   107,   108,   109,   110,     0,     0,   111,
     112,   313,     9,    10,    11,    12,    13,    14,    15,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,     0,
       0,    54,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    55,    56,    57,    58,
      59,    60,     0,     0,     0,     0,     0,     0,     0,    61,
      62,    63,    64,    65,     0,    66,    67,    68,    69,    70,
      71,     0,     0,     0,     0,     0,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,     0,     0,     0,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   105,   106,   107,   108,   109,
     110,     0,     0,   111,   112,     9,    10,    11,    12,    13,
      14,    15,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,     0,     0,    54,     0,     0,   334,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    55,
      56,    57,    58,    59,    60,     0,     0,     0,     0,     0,
       0,     0,    61,    62,    63,    64,    65,   335,    66,    67,
      68,    69,    70,    71,     0,     0,   336,     0,     0,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,     0,     0,     0,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   105,   106,
     107,   108,   109,   110,     0,     0,   111,   112,     9,    10,
      11,    12,    13,    14,    15,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,     0,     0,    54,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    55,    56,    57,    58,    59,    60,     0,     0,
       0,     0,     0,     0,     0,    61,    62,    63,    64,    65,
     335,    66,    67,    68,    69,    70,    71,     0,     0,   336,
       0,     0,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,     0,     0,
       0,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   105,   106,   107,   108,   109,   110,     0,     0,   111,
     112,     9,    10,    11,    12,    13,    14,    15,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,     0,     0,
      54,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    55,    56,    57,    58,    59,
      60,     0,     0,     0,     0,     0,     0,     0,    61,    62,
      63,    64,    65,     0,    66,    67,    68,    69,    70,    71,
       0,     0,     0,     0,     0,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,     0,     0,     0,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   105,   106,   107,   108,   109,   110,
       0,   395,   111,   112,     9,    10,    11,    12,    13,    14,
      15,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,     0,     0,    54,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    55,    56,
      57,    58,    59,    60,     0,     0,     0,     0,     0,     0,
       0,    61,    62,    63,    64,    65,     0,    66,    67,    68,
      69,    70,    71,     0,     0,     0,     0,     0,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,     0,     0,     0,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   105,   106,   107,
     108,   109,   110,     0,   433,   111,   112,     9,    10,    11,
      12,    13,    14,    15,   449,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,     0,     0,    54,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    55,    56,    57,    58,    59,    60,     0,     0,     0,
       0,     0,     0,     0,    61,    62,    63,    64,    65,     0,
      66,    67,    68,    69,    70,    71,     0,     0,     0,     0,
       0,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,     0,     0,     0,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     105,   106,   107,   108,   109,   110,     0,     0,   111,   112,
       9,    10,    11,    12,    13,    14,    15,   450,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,     0,     0,    54,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    55,    56,    57,    58,    59,    60,
       0,     0,     0,     0,     0,     0,     0,    61,    62,    63,
      64,    65,     0,    66,    67,    68,    69,    70,    71,     0,
       0,     0,     0,     0,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
       0,     0,     0,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   105,   106,   107,   108,   109,   110,     0,
       0,   111,   112,     9,    10,    11,    12,    13,    14,    15,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
       0,     0,    54,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    55,    56,    57,
      58,    59,    60,     0,     0,     0,     0,     0,     0,     0,
      61,    62,    63,    64,    65,     0,    66,    67,    68,    69,
      70,    71,     0,     0,   336,     0,     0,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,     0,     0,     0,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   105,   106,   107,   108,
     109,   110,     0,     0,   111,   112,     9,    10,    11,    12,
      13,    14,    15,     0,    16,   387,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,     0,     0,    54,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      55,    56,    57,    58,    59,    60,     0,     0,     0,     0,
       0,     0,     0,    61,    62,    63,    64,    65,     0,    66,
      67,    68,    69,    70,    71,     0,     0,     0,     0,     0,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    86,    87,     0,     0,     0,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   105,
     106,   107,   108,   109,   110,     0,     0,   111,   112,     9,
      10,    11,    12,    13,    14,    15,     0,    16,     0,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,     0,     0,    54,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    55,    56,    57,    58,    59,    60,     0,
       0,     0,     0,     0,     0,     0,    61,    62,    63,    64,
      65,     0,    66,    67,    68,    69,    70,    71,     0,     0,
       0,     0,     0,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,    85,    86,    87,     0,
       0,     0,    88,    89,    90,    91,    92,    93,    94,    95,
      96,    97,    98,    99,   100,   101,   102,   103,   104,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   105,   106,   107,   108,   109,   110,     0,     0,
     111,   112,     9,    10,    11,    12,     0,     0,    15,     0,
      16,     0,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,     0,
       0,    54,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    55,    56,    57,    58,
      59,    60,     0,     0,     0,     0,     0,     0,     0,    61,
      62,    63,    64,    65,     0,    66,    67,    68,    69,    70,
      71,     0,     0,     0,     0,     0,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    85,
      86,    87,     0,     0,     0,    88,    89,    90,    91,    92,
      93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
     103,   104,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   105,   106,   107,   108,   109,
     110,   228,     0,   111,   112,     9,    10,    11,    12,     0,
       0,    15,     0,    16,     0,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,     0,     0,    54,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    55,
      56,    57,    58,    59,    60,     0,     0,     0,     0,     0,
       0,     0,    61,    62,    63,    64,    65,     0,    66,    67,
      68,    69,    70,    71,     0,     0,     0,     0,     0,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,     0,     0,     0,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   104,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   105,   106,
     107,   108,   109,   110,   298,     0,   111,   112,     9,    10,
      11,    12,     0,     0,    15,     0,    16,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,     0,     0,    54,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    55,    56,    57,    58,    59,    60,     0,     0,
       0,     0,     0,     0,     0,    61,    62,    63,    64,    65,
       0,    66,    67,    68,    69,    70,    71,   380,     0,     0,
       0,     0,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,    87,     0,     0,
       0,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   105,   106,   107,   108,   109,   110,     0,     0,   111,
     112,     9,    10,    11,    12,   398,     0,    15,     0,    16,
       0,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,     0,     0,
      54,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    55,    56,    57,    58,    59,
      60,     0,     0,     0,     0,     0,     0,     0,    61,    62,
      63,    64,    65,     0,    66,    67,    68,    69,    70,    71,
       0,     0,     0,     0,     0,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
      87,     0,     0,     0,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     104,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   105,   106,   107,   108,   109,   110,
       0,     0,   111,   112,     9,    10,    11,    12,     0,     0,
      15,     0,    16,     0,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,     0,     0,    54,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    55,    56,
      57,    58,    59,    60,     0,     0,     0,     0,     0,     0,
       0,    61,    62,    63,    64,    65,     0,    66,    67,    68,
      69,    70,    71,   434,     0,     0,     0,     0,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,    87,     0,     0,     0,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   105,   106,   107,
     108,   109,   110,     0,     0,   111,   112,     9,    10,    11,
      12,     0,     0,    15,     0,    16,     0,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,     0,     0,    54,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    55,    56,    57,    58,    59,    60,     0,     0,     0,
       0,     0,     0,     0,    61,    62,    63,    64,    65,     0,
      66,    67,    68,    69,    70,    71,   436,     0,     0,     0,
       0,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,    87,     0,     0,     0,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   104,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     105,   106,   107,   108,   109,   110,     0,     0,   111,   112,
       9,    10,    11,    12,     0,     0,    15,     0,    16,     0,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,     0,     0,    54,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    55,    56,    57,    58,    59,    60,
       0,     0,     0,     0,     0,     0,     0,    61,    62,    63,
      64,    65,     0,    66,    67,    68,    69,    70,    71,   510,
       0,     0,     0,     0,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,    87,
       0,     0,     0,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   105,   106,   107,   108,   109,   110,     0,
       0,   111,   112,     9,    10,    11,    12,     0,     0,    15,
       0,    16,     0,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
       0,     0,    54,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    55,    56,    57,
      58,    59,    60,     0,     0,     0,     0,     0,     0,     0,
      61,    62,    63,    64,    65,     0,    66,    67,    68,    69,
      70,    71,     0,     0,     0,     0,     0,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,     0,     0,     0,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,   104,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   105,   106,   107,   108,
     109,   110,     0,     0,   111,   112,     9,    10,    11,    12,
       0,     0,    15,     0,    16,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,     0,     0,    54,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      55,    56,    57,    58,    59,    60,     0,     0,     0,     0,
       0,     0,     0,    61,    62,    63,    64,    65,     0,    66,
      67,    68,    69,    70,    71,     0,     0,     0,     0,     0,
      72,    73,    74,    75,    76,    77,    78,    79,    80,   507,
      82,    83,    84,    85,    86,    87,     0,     0,     0,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   105,
     106,   107,   108,   109,   110,     0,     0,   111,   112
};

#define yypact_value_is_default(yystate) \
  ((yystate) == (-396))

#define yytable_value_is_error(yytable_value) \
  YYID (0)

static const yytype_int16 yycheck[] =
{
       2,     3,    15,    15,    60,    16,   240,   330,    60,   332,
     405,   406,    99,    15,   283,   284,   285,   286,   287,   288,
      80,   290,   291,   292,   293,   294,    16,     0,    16,    60,
      60,   405,   406,    58,    98,    16,    80,    16,     2,     3,
      56,    85,    80,    60,    13,    14,    84,    16,    13,    14,
      60,    15,    63,    64,    15,    60,    60,    60,    57,    60,
     147,    39,    40,    79,    66,    67,    68,    69,    70,    60,
      60,    60,    60,   113,    57,     2,     3,   400,    98,    60,
      98,    80,    15,   139,    63,    64,   481,   139,    15,    98,
     101,   102,   487,    79,   154,   329,    72,    80,   162,   112,
     112,    98,    66,    67,    68,    69,    70,   481,   139,   139,
      72,   113,    16,   487,   116,   140,   141,   142,   143,   144,
     145,   146,   139,   148,   149,   150,   151,    13,    14,   139,
      16,    39,    40,   456,   139,   139,   139,    79,   139,    66,
      67,    68,    69,    70,    63,    64,    39,    40,   139,   189,
     139,    79,   116,    38,    39,    79,    41,   197,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    98,   403,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      39,    40,   101,   102,    13,    14,   113,   189,    98,   116,
      80,    81,    82,    83,    72,   197,    13,    14,    98,    16,
      13,    14,    98,    16,    13,    14,    79,    16,     5,     6,
       7,    38,    39,    40,   254,   255,   256,   257,   258,    39,
      40,    41,    72,   546,    13,    14,   228,   496,    39,    40,
      41,    39,    40,    41,    39,    40,    41,    13,    14,    15,
     242,    81,    82,    83,    13,    14,    98,    63,    64,   440,
     441,   154,   254,   255,   256,   257,   258,   154,    84,   119,
      85,    98,   189,    15,   228,   534,    15,   307,    15,    15,
     197,    59,    57,    80,   543,    79,   147,    14,    13,    13,
     320,   283,   284,   285,   286,   287,   288,   521,   290,   291,
     292,   293,   294,    14,   296,   297,   298,   337,    14,    13,
      13,   228,    79,   141,   140,   307,   145,   142,   149,   143,
      72,   144,   314,   315,    72,   242,   148,    72,   320,   150,
     120,   151,    72,    80,    72,    79,   121,   254,   255,   256,
     257,   258,   296,   297,   298,   337,    72,   377,   378,   379,
      72,    72,    72,    72,    72,    72,    58,    58,     5,     5,
     314,   315,   247,   393,   394,    80,   283,   284,   285,   286,
     287,   288,   347,   290,   291,   292,   293,   294,   350,   296,
     297,   298,   418,   353,   467,   377,   378,   379,   536,   408,
     307,   481,   409,   385,   386,   487,    -1,   314,   315,    -1,
      -1,   393,   394,   320,    -1,    -1,    -1,    -1,    -1,    -1,
     440,   441,    -1,    -1,    -1,    -1,   408,   409,    -1,    -1,
     337,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   385,   386,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   440,   441,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     377,   378,   379,    -1,    -1,    -1,    -1,    -1,   385,   386,
      -1,    -1,    -1,    -1,    -1,    -1,   393,   394,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   408,   409,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   496,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   549,
      -1,    -1,    -1,   440,   441,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   534,    -1,   536,    -1,    -1,    -1,   540,    -1,
      -1,   543,    -1,    -1,    -1,    -1,    -1,   549,    -1,    13,
      14,    15,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    25,    26,    -1,    -1,    -1,    -1,    -1,    -1,   496,
      -1,    -1,    -1,    -1,    -1,    -1,   540,    -1,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    -1,
      -1,    55,    56,    57,    58,    -1,    -1,    -1,    -1,    63,
      64,    65,    -1,    -1,    -1,    -1,    -1,   534,    -1,   536,
      -1,    -1,    -1,   540,    78,    -1,   543,    -1,    -1,    -1,
      84,    -1,   549,    -1,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,    -1,    -1,
      -1,    -1,   106,    -1,    -1,    -1,   110,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   222,   223,
      -1,    -1,    -1,    -1,    -1,    -1,   230,   231,   232,   233,
     234,    -1,    -1,    -1,   238,    -1,    -1,   241,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   261,   262,   263,
     264,   265,   266,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   301,    -1,    -1,
      -1,   305,   306,    -1,   308,   309,   310,   311,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   326,   327,   328,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   380,    -1,   382,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   395,    -1,    -1,   398,    -1,    -1,    -1,   402,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   433,
     434,    -1,   436,    -1,   438,   439,    -1,    -1,    -1,    -1,
      -1,   445,   446,   447,   448,     3,     4,    -1,    -1,   453,
      -1,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      -1,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    -1,
      58,    -1,    -1,    -1,    -1,    -1,   510,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    73,    74,    75,    76,    77,
      78,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    86,    87,
      88,    89,    90,    -1,    92,    93,    94,    95,    96,    97,
      -1,    -1,    -1,    -1,    -1,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,    -1,    -1,    -1,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,   135,   136,   137,
     138,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   152,   153,   154,   155,   156,   157,
       3,     4,   160,   161,    -1,    -1,     9,    10,    11,    12,
      13,    14,    15,    -1,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    -1,    58,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      73,    74,    75,    76,    77,    78,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    86,    87,    88,    89,    90,    -1,    92,
      93,    94,    95,    96,    97,    -1,    -1,    -1,    -1,    -1,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,    -1,    -1,    -1,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   134,   135,   136,   137,   138,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   152,
     153,   154,   155,   156,   157,    -1,    -1,   160,   161,     8,
       9,    10,    11,    12,    13,    14,    15,    -1,    17,    -1,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    -1,    58,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    73,    74,    75,    76,    77,    78,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    86,    87,    88,
      89,    90,    -1,    92,    93,    94,    95,    96,    97,    -1,
      -1,    -1,    -1,    -1,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
      -1,    -1,    -1,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   152,   153,   154,   155,   156,   157,    -1,
      -1,   160,   161,     8,     9,    10,    11,    12,    13,    14,
      15,    -1,    17,    -1,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    73,    74,
      75,    76,    77,    78,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    86,    87,    88,    89,    90,    -1,    92,    93,    94,
      95,    96,    97,    -1,    -1,    -1,    -1,    -1,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,    -1,    -1,    -1,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   152,   153,   154,
     155,   156,   157,    -1,    -1,   160,   161,     8,     9,    10,
      11,    12,    13,    14,    15,    -1,    17,    -1,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    -1,    58,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    73,    74,    75,    76,    77,    78,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    86,    87,    88,    89,    90,
      -1,    92,    93,    94,    95,    96,    97,    -1,    -1,    -1,
      -1,    -1,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,    -1,    -1,
      -1,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   152,   153,   154,   155,   156,   157,    -1,    -1,   160,
     161,     8,     9,    10,    11,    12,    13,    14,    15,    -1,
      17,    -1,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    73,    74,    75,    76,
      77,    78,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    86,
      87,    88,    89,    90,    -1,    92,    93,    94,    95,    96,
      97,    -1,    -1,    -1,    -1,    -1,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,    -1,    -1,    -1,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   152,   153,   154,   155,   156,
     157,    -1,    -1,   160,   161,     9,    10,    11,    12,    13,
      14,    15,    -1,    17,    -1,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    -1,    58,    -1,    -1,    61,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    73,
      74,    75,    76,    77,    78,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    -1,    -1,   100,    -1,    -1,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,    -1,    -1,    -1,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   152,   153,
     154,   155,   156,   157,    -1,    -1,   160,   161,     9,    10,
      11,    12,    13,    14,    15,    -1,    17,    -1,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    -1,    58,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    73,    74,    75,    76,    77,    78,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    -1,    -1,   100,
      -1,    -1,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,    -1,    -1,
      -1,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   152,   153,   154,   155,   156,   157,    -1,    -1,   160,
     161,     9,    10,    11,    12,    13,    14,    15,    -1,    17,
      -1,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    -1,
      58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    73,    74,    75,    76,    77,
      78,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    86,    87,
      88,    89,    90,    -1,    92,    93,    94,    95,    96,    97,
      -1,    -1,    -1,    -1,    -1,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,    -1,    -1,    -1,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,   135,   136,   137,
     138,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   152,   153,   154,   155,   156,   157,
      -1,   159,   160,   161,     9,    10,    11,    12,    13,    14,
      15,    -1,    17,    -1,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    73,    74,
      75,    76,    77,    78,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    86,    87,    88,    89,    90,    -1,    92,    93,    94,
      95,    96,    97,    -1,    -1,    -1,    -1,    -1,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,    -1,    -1,    -1,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   152,   153,   154,
     155,   156,   157,    -1,   159,   160,   161,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    -1,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    -1,    58,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    73,    74,    75,    76,    77,    78,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    86,    87,    88,    89,    90,    -1,
      92,    93,    94,    95,    96,    97,    -1,    -1,    -1,    -1,
      -1,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,    -1,    -1,    -1,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     152,   153,   154,   155,   156,   157,    -1,    -1,   160,   161,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    -1,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    -1,    58,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    73,    74,    75,    76,    77,    78,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    86,    87,    88,
      89,    90,    -1,    92,    93,    94,    95,    96,    97,    -1,
      -1,    -1,    -1,    -1,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
      -1,    -1,    -1,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   152,   153,   154,   155,   156,   157,    -1,
      -1,   160,   161,     9,    10,    11,    12,    13,    14,    15,
      -1,    17,    -1,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    73,    74,    75,
      76,    77,    78,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      86,    87,    88,    89,    90,    -1,    92,    93,    94,    95,
      96,    97,    -1,    -1,   100,    -1,    -1,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,    -1,    -1,    -1,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   132,   133,   134,   135,
     136,   137,   138,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   152,   153,   154,   155,
     156,   157,    -1,    -1,   160,   161,     9,    10,    11,    12,
      13,    14,    15,    -1,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    -1,    58,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      73,    74,    75,    76,    77,    78,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    86,    87,    88,    89,    90,    -1,    92,
      93,    94,    95,    96,    97,    -1,    -1,    -1,    -1,    -1,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,    -1,    -1,    -1,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   134,   135,   136,   137,   138,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   152,
     153,   154,   155,   156,   157,    -1,    -1,   160,   161,     9,
      10,    11,    12,    13,    14,    15,    -1,    17,    -1,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    -1,    -1,    58,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    73,    74,    75,    76,    77,    78,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    86,    87,    88,    89,
      90,    -1,    92,    93,    94,    95,    96,    97,    -1,    -1,
      -1,    -1,    -1,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,    -1,
      -1,    -1,   122,   123,   124,   125,   126,   127,   128,   129,
     130,   131,   132,   133,   134,   135,   136,   137,   138,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   152,   153,   154,   155,   156,   157,    -1,    -1,
     160,   161,     9,    10,    11,    12,    -1,    -1,    15,    -1,
      17,    -1,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    -1,
      -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    73,    74,    75,    76,
      77,    78,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    86,
      87,    88,    89,    90,    -1,    92,    93,    94,    95,    96,
      97,    -1,    -1,    -1,    -1,    -1,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117,   118,    -1,    -1,    -1,   122,   123,   124,   125,   126,
     127,   128,   129,   130,   131,   132,   133,   134,   135,   136,
     137,   138,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   152,   153,   154,   155,   156,
     157,   158,    -1,   160,   161,     9,    10,    11,    12,    -1,
      -1,    15,    -1,    17,    -1,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    -1,    -1,    58,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    73,
      74,    75,    76,    77,    78,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    86,    87,    88,    89,    90,    -1,    92,    93,
      94,    95,    96,    97,    -1,    -1,    -1,    -1,    -1,   103,
     104,   105,   106,   107,   108,   109,   110,   111,   112,   113,
     114,   115,   116,   117,   118,    -1,    -1,    -1,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   152,   153,
     154,   155,   156,   157,   158,    -1,   160,   161,     9,    10,
      11,    12,    -1,    -1,    15,    -1,    17,    -1,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    -1,    -1,    58,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    73,    74,    75,    76,    77,    78,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    86,    87,    88,    89,    90,
      -1,    92,    93,    94,    95,    96,    97,    98,    -1,    -1,
      -1,    -1,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,    -1,    -1,
      -1,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,   132,   133,   134,   135,   136,   137,   138,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   152,   153,   154,   155,   156,   157,    -1,    -1,   160,
     161,     9,    10,    11,    12,    13,    -1,    15,    -1,    17,
      -1,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    -1,    -1,
      58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    73,    74,    75,    76,    77,
      78,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    86,    87,
      88,    89,    90,    -1,    92,    93,    94,    95,    96,    97,
      -1,    -1,    -1,    -1,    -1,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   114,   115,   116,   117,
     118,    -1,    -1,    -1,   122,   123,   124,   125,   126,   127,
     128,   129,   130,   131,   132,   133,   134,   135,   136,   137,
     138,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   152,   153,   154,   155,   156,   157,
      -1,    -1,   160,   161,     9,    10,    11,    12,    -1,    -1,
      15,    -1,    17,    -1,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    -1,    -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    73,    74,
      75,    76,    77,    78,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    86,    87,    88,    89,    90,    -1,    92,    93,    94,
      95,    96,    97,    98,    -1,    -1,    -1,    -1,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,    -1,    -1,    -1,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   152,   153,   154,
     155,   156,   157,    -1,    -1,   160,   161,     9,    10,    11,
      12,    -1,    -1,    15,    -1,    17,    -1,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    -1,    -1,    58,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    73,    74,    75,    76,    77,    78,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    86,    87,    88,    89,    90,    -1,
      92,    93,    94,    95,    96,    97,    98,    -1,    -1,    -1,
      -1,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,   113,   114,   115,   116,   117,   118,    -1,    -1,    -1,
     122,   123,   124,   125,   126,   127,   128,   129,   130,   131,
     132,   133,   134,   135,   136,   137,   138,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     152,   153,   154,   155,   156,   157,    -1,    -1,   160,   161,
       9,    10,    11,    12,    -1,    -1,    15,    -1,    17,    -1,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    -1,    -1,    58,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    73,    74,    75,    76,    77,    78,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    86,    87,    88,
      89,    90,    -1,    92,    93,    94,    95,    96,    97,    98,
      -1,    -1,    -1,    -1,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
      -1,    -1,    -1,   122,   123,   124,   125,   126,   127,   128,
     129,   130,   131,   132,   133,   134,   135,   136,   137,   138,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   152,   153,   154,   155,   156,   157,    -1,
      -1,   160,   161,     9,    10,    11,    12,    -1,    -1,    15,
      -1,    17,    -1,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      -1,    -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    73,    74,    75,
      76,    77,    78,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      86,    87,    88,    89,    90,    -1,    92,    93,    94,    95,
      96,    97,    -1,    -1,    -1,    -1,    -1,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,   113,   114,   115,
     116,   117,   118,    -1,    -1,    -1,   122,   123,   124,   125,
     126,   127,   128,   129,   130,   131,   132,   133,   134,   135,
     136,   137,   138,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   152,   153,   154,   155,
     156,   157,    -1,    -1,   160,   161,     9,    10,    11,    12,
      -1,    -1,    15,    -1,    17,    -1,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    -1,    -1,    58,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      73,    74,    75,    76,    77,    78,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    86,    87,    88,    89,    90,    -1,    92,
      93,    94,    95,    96,    97,    -1,    -1,    -1,    -1,    -1,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
     113,   114,   115,   116,   117,   118,    -1,    -1,    -1,   122,
     123,   124,   125,   126,   127,   128,   129,   130,   131,   132,
     133,   134,   135,   136,   137,   138,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   152,
     153,   154,   155,   156,   157,    -1,    -1,   160,   161
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,     5,     6,     7,   164,   165,   166,   167,     8,     9,
      10,    11,    12,    13,    14,    15,    17,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    58,    73,    74,    75,    76,    77,
      78,    86,    87,    88,    89,    90,    92,    93,    94,    95,
      96,    97,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,   133,
     134,   135,   136,   137,   138,   152,   153,   154,   155,   156,
     157,   160,   161,   168,   169,   170,   171,   173,   174,   175,
     176,   177,   178,   179,   180,   181,   182,   183,   184,   185,
     186,   187,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,   199,   200,   201,   202,   203,   204,   207,
     210,   213,   214,   215,   216,   217,   218,   219,   220,   221,
     222,   225,   226,   227,   228,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,   241,   242,   243,
     244,   245,   246,   247,   248,   249,   251,   252,     8,   168,
       0,   166,   167,   170,   178,   180,   170,   168,   170,    38,
      39,    40,    39,    40,    41,    39,    40,    41,    39,    40,
      41,    39,    40,    41,    39,    40,    39,    40,    39,    40,
      39,    40,   170,   170,    98,    98,    98,    98,   158,   170,
     170,   170,   170,   170,   170,   170,   170,   170,   170,   170,
      15,   112,    15,   170,   170,   170,   170,    79,    79,    79,
      79,   170,   170,   170,   168,   168,   168,   168,   168,    98,
     170,    98,    98,    98,   170,    72,    72,    79,   170,   170,
     170,   170,   170,   170,   170,   170,   170,   170,   170,   170,
     170,   170,    58,   140,   141,   142,   143,   144,   145,   146,
     148,   149,   150,   151,    15,   170,    72,    72,   158,   170,
      98,    98,   162,     8,   169,    13,    14,   168,    13,    14,
      13,    14,   154,     8,     3,     4,    16,    16,   170,   170,
     168,   170,   170,   170,   170,   170,   170,    13,    14,    15,
     223,   224,   223,   170,    61,    91,   100,   168,   265,   266,
     267,   268,   271,   203,    81,    82,    83,   205,   206,    84,
     211,   212,    85,   208,   209,   170,   170,   170,   170,   170,
     170,   119,    56,    79,   265,   265,   265,   265,   265,   265,
      99,   147,   265,   265,   265,   265,   265,   168,   168,   168,
      98,   170,    98,   170,   170,     3,     4,    18,   172,   170,
     170,   170,   170,   168,   168,   159,   170,   170,    13,   170,
     223,   224,    16,    15,    15,    15,    15,    16,    60,    59,
      80,    80,   206,    80,   212,    80,   209,    80,    79,    57,
     250,   139,   139,   139,   139,   139,   139,   147,   139,   139,
     139,   139,    16,   159,    98,   170,    98,   170,    14,    13,
     168,   168,    38,    39,    41,    14,    13,    14,    13,    16,
      16,   170,   170,    13,    16,   170,   223,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,   253,   254,   255,
     256,   257,   258,   259,   260,   261,   262,   263,   264,   256,
     257,   269,   270,   101,   102,   256,   257,   272,   273,   274,
     275,   266,   271,    79,   250,    57,    80,   140,   141,   142,
     143,   144,   145,   148,   149,   150,   151,   112,   170,   170,
      98,   170,   170,   170,   172,   172,   170,   170,   170,   170,
     170,    15,   112,    16,    72,    72,    72,    72,    72,    72,
      72,    72,    72,    72,    16,   254,    16,   270,    72,    72,
      16,   273,   120,    80,   265,   170,   223,   265,   267,   168,
      80,   265,   139,    16,    16,    79,   139,    58,   121,    58,
      80
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  However,
   YYFAIL appears to be in use.  Nevertheless, it is formally deprecated
   in Bison 2.4.2's NEWS entry, where a plan to phase it out is
   discussed.  */

#define YYFAIL		goto yyerrlab
#if defined YYFAIL
  /* This is here to suppress warnings from the GCC cpp's
     -Wunused-macros.  Normally we don't worry about that warning, but
     some users do, and we want to make it easy for users to remove
     YYFAIL uses, which will produce warnings from Bison 2.5.  */
#endif

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* This macro is provided for backward compatibility. */

#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (0, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  YYSIZE_T yysize1;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = 0;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - Assume YYFAIL is not used.  It's too flawed to consider.  See
       <http://lists.gnu.org/archive/html/bison-patches/2009-12/msg00024.html>
       for details.  YYERROR is fine as it does not invoke this
       function.
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                yysize1 = yysize + yytnamerr (0, yytname[yyx]);
                if (! (yysize <= yysize1
                       && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                  return 2;
                yysize = yysize1;
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  yysize1 = yysize + yystrlen (yyformat);
  if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
    return 2;
  yysize = yysize1;

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */


/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:

/* Line 1806 of yacc.c  */
#line 284 "itex2MML.y"
    {/* all processing done in body*/}
    break;

  case 3:

/* Line 1806 of yacc.c  */
#line 287 "itex2MML.y"
    {/* nothing - do nothing*/}
    break;

  case 4:

/* Line 1806 of yacc.c  */
#line 288 "itex2MML.y"
    {/* proc done in body*/}
    break;

  case 5:

/* Line 1806 of yacc.c  */
#line 289 "itex2MML.y"
    {/* all proc. in body*/}
    break;

  case 6:

/* Line 1806 of yacc.c  */
#line 290 "itex2MML.y"
    {/* all proc. in body*/}
    break;

  case 7:

/* Line 1806 of yacc.c  */
#line 291 "itex2MML.y"
    {/* all proc. in body*/}
    break;

  case 8:

/* Line 1806 of yacc.c  */
#line 293 "itex2MML.y"
    {printf("%s", (yyvsp[(1) - (1)]));}
    break;

  case 9:

/* Line 1806 of yacc.c  */
#line 295 "itex2MML.y"
    {/* empty math group - ignore*/}
    break;

  case 10:

/* Line 1806 of yacc.c  */
#line 296 "itex2MML.y"
    {/* ditto */}
    break;

  case 11:

/* Line 1806 of yacc.c  */
#line 297 "itex2MML.y"
    {
  char ** r = (char **) ret_str;
  char * s = itex2MML_copy3("<math xmlns='http://www.w3.org/1998/Math/MathML' display='inline'>", (yyvsp[(2) - (3)]), "</math>");
  itex2MML_free_string((yyvsp[(2) - (3)]));
  if (r) {
    (*r) = (s == itex2MML_empty_string) ? 0 : s;
  }
  else {
    if (itex2MML_write_mathml)
      (*itex2MML_write_mathml) (s);
    itex2MML_free_string(s);
  }
}
    break;

  case 12:

/* Line 1806 of yacc.c  */
#line 310 "itex2MML.y"
    {
  char ** r = (char **) ret_str;
  char * s = itex2MML_copy3("<math xmlns='http://www.w3.org/1998/Math/MathML' display='block'>", (yyvsp[(2) - (3)]), "</math>");
  itex2MML_free_string((yyvsp[(2) - (3)]));
  if (r) {
    (*r) = (s == itex2MML_empty_string) ? 0 : s;
  }
  else {
    if (itex2MML_write_mathml)
      (*itex2MML_write_mathml) (s);
    itex2MML_free_string(s);
  }
}
    break;

  case 13:

/* Line 1806 of yacc.c  */
#line 324 "itex2MML.y"
    {
  (yyval) = itex2MML_copy_string((yyvsp[(1) - (1)]));
  itex2MML_free_string((yyvsp[(1) - (1)]));
}
    break;

  case 14:

/* Line 1806 of yacc.c  */
#line 328 "itex2MML.y"
    {
  (yyval) = itex2MML_copy2((yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));
  itex2MML_free_string((yyvsp[(1) - (2)]));
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 15:

/* Line 1806 of yacc.c  */
#line 334 "itex2MML.y"
    {
  if (itex2MML_displaymode == 1) {
    char * s1 = itex2MML_copy3("<munderover>", (yyvsp[(1) - (5)]), " ");
    char * s2 = itex2MML_copy3((yyvsp[(3) - (5)]), " ", (yyvsp[(5) - (5)]));
    (yyval) = itex2MML_copy3(s1, s2, "</munderover>");
    itex2MML_free_string(s1);
    itex2MML_free_string(s2);
  }
  else {
    char * s1 = itex2MML_copy3("<msubsup>", (yyvsp[(1) - (5)]), " ");
    char * s2 = itex2MML_copy3((yyvsp[(3) - (5)]), " ", (yyvsp[(5) - (5)]));
    (yyval) = itex2MML_copy3(s1, s2, "</msubsup>");
    itex2MML_free_string(s1);
    itex2MML_free_string(s2);
  }
  itex2MML_free_string((yyvsp[(1) - (5)]));
  itex2MML_free_string((yyvsp[(3) - (5)]));
  itex2MML_free_string((yyvsp[(5) - (5)]));
}
    break;

  case 16:

/* Line 1806 of yacc.c  */
#line 353 "itex2MML.y"
    {
  if (itex2MML_displaymode == 1) {
    char * s1 = itex2MML_copy3("<munder>", (yyvsp[(1) - (3)]), " ");
    (yyval) = itex2MML_copy3(s1, (yyvsp[(3) - (3)]), "</munder>");
    itex2MML_free_string(s1);
  }
  else {
    char * s1 = itex2MML_copy3("<msub>", (yyvsp[(1) - (3)]), " ");
    (yyval) = itex2MML_copy3(s1, (yyvsp[(3) - (3)]), "</msub>");
    itex2MML_free_string(s1);
  }
  itex2MML_free_string((yyvsp[(1) - (3)]));
  itex2MML_free_string((yyvsp[(3) - (3)]));
}
    break;

  case 17:

/* Line 1806 of yacc.c  */
#line 367 "itex2MML.y"
    {
  if (itex2MML_displaymode == 1) {
    char * s1 = itex2MML_copy3("<munderover>", (yyvsp[(1) - (5)]), " ");
    char * s2 = itex2MML_copy3((yyvsp[(5) - (5)]), " ", (yyvsp[(3) - (5)]));
    (yyval) = itex2MML_copy3(s1, s2, "</munderover>");
    itex2MML_free_string(s1);
    itex2MML_free_string(s2);
  }
  else {
    char * s1 = itex2MML_copy3("<msubsup>", (yyvsp[(1) - (5)]), " ");
    char * s2 = itex2MML_copy3((yyvsp[(5) - (5)]), " ", (yyvsp[(3) - (5)]));
    (yyval) = itex2MML_copy3(s1, s2, "</msubsup>");
    itex2MML_free_string(s1);
    itex2MML_free_string(s2);
  }
  itex2MML_free_string((yyvsp[(1) - (5)]));
  itex2MML_free_string((yyvsp[(3) - (5)]));
  itex2MML_free_string((yyvsp[(5) - (5)]));
}
    break;

  case 18:

/* Line 1806 of yacc.c  */
#line 386 "itex2MML.y"
    {
  if (itex2MML_displaymode == 1) {
    char * s1 = itex2MML_copy3("<mover>", (yyvsp[(1) - (3)]), " ");
    (yyval) = itex2MML_copy3(s1, (yyvsp[(3) - (3)]), "</mover>");
    itex2MML_free_string(s1);
  }
  else {
    char * s1 = itex2MML_copy3("<msup>", (yyvsp[(1) - (3)]), " ");
    (yyval) = itex2MML_copy3(s1, (yyvsp[(3) - (3)]), "</msup>");
    itex2MML_free_string(s1);
  }
  itex2MML_free_string((yyvsp[(1) - (3)]));
  itex2MML_free_string((yyvsp[(3) - (3)]));
}
    break;

  case 19:

/* Line 1806 of yacc.c  */
#line 400 "itex2MML.y"
    {
  if (itex2MML_displaymode == 1) {
    char * s1 = itex2MML_copy3("<munderover>", (yyvsp[(1) - (5)]), " ");
    char * s2 = itex2MML_copy3((yyvsp[(3) - (5)]), " ", (yyvsp[(5) - (5)]));
    (yyval) = itex2MML_copy3(s1, s2, "</munderover>");
    itex2MML_free_string(s1);
    itex2MML_free_string(s2);
  }
  else {
    char * s1 = itex2MML_copy3("<msubsup>", (yyvsp[(1) - (5)]), " ");
    char * s2 = itex2MML_copy3((yyvsp[(3) - (5)]), " ", (yyvsp[(5) - (5)]));
    (yyval) = itex2MML_copy3(s1, s2, "</msubsup>");
    itex2MML_free_string(s1);
    itex2MML_free_string(s2);
  }
  itex2MML_free_string((yyvsp[(1) - (5)]));
  itex2MML_free_string((yyvsp[(3) - (5)]));
  itex2MML_free_string((yyvsp[(5) - (5)]));
}
    break;

  case 20:

/* Line 1806 of yacc.c  */
#line 419 "itex2MML.y"
    {
  if (itex2MML_displaymode == 1) {
    char * s1 = itex2MML_copy3("<munder>", (yyvsp[(1) - (3)]), " ");
    (yyval) = itex2MML_copy3(s1, (yyvsp[(3) - (3)]), "</munder>");
    itex2MML_free_string(s1);
  }
  else {
    char * s1 = itex2MML_copy3("<msub>", (yyvsp[(1) - (3)]), " ");
    (yyval) = itex2MML_copy3(s1, (yyvsp[(3) - (3)]), "</msub>");
    itex2MML_free_string(s1);
  }
  itex2MML_free_string((yyvsp[(1) - (3)]));
  itex2MML_free_string((yyvsp[(3) - (3)]));
}
    break;

  case 21:

/* Line 1806 of yacc.c  */
#line 433 "itex2MML.y"
    {
  if (itex2MML_displaymode == 1) {
    char * s1 = itex2MML_copy3("<munderover>", (yyvsp[(1) - (5)]), " ");
    char * s2 = itex2MML_copy3((yyvsp[(5) - (5)]), " ", (yyvsp[(3) - (5)]));
    (yyval) = itex2MML_copy3(s1, s2, "</munderover>");
    itex2MML_free_string(s1);
    itex2MML_free_string(s2);
  }
  else {
    char * s1 = itex2MML_copy3("<msubsup>", (yyvsp[(1) - (5)]), " ");
    char * s2 = itex2MML_copy3((yyvsp[(5) - (5)]), " ", (yyvsp[(3) - (5)]));
    (yyval) = itex2MML_copy3(s1, s2, "</msubsup>");
    itex2MML_free_string(s1);
    itex2MML_free_string(s2);
  }
  itex2MML_free_string((yyvsp[(1) - (5)]));
  itex2MML_free_string((yyvsp[(3) - (5)]));
  itex2MML_free_string((yyvsp[(5) - (5)]));
}
    break;

  case 22:

/* Line 1806 of yacc.c  */
#line 452 "itex2MML.y"
    {
  if (itex2MML_displaymode == 1) {
    char * s1 = itex2MML_copy3("<mover>", (yyvsp[(1) - (3)]), " ");
    (yyval) = itex2MML_copy3(s1, (yyvsp[(3) - (3)]), "</mover>");
    itex2MML_free_string(s1);
  }
  else {
    char * s1 = itex2MML_copy3("<msup>", (yyvsp[(1) - (3)]), " ");
    (yyval) = itex2MML_copy3(s1, (yyvsp[(3) - (3)]), "</msup>");
    itex2MML_free_string(s1);
  }
  itex2MML_free_string((yyvsp[(1) - (3)]));
  itex2MML_free_string((yyvsp[(3) - (3)]));
}
    break;

  case 23:

/* Line 1806 of yacc.c  */
#line 466 "itex2MML.y"
    {
  char * s1 = itex2MML_copy3("<msubsup>", (yyvsp[(1) - (5)]), " ");
  char * s2 = itex2MML_copy3((yyvsp[(3) - (5)]), " ", (yyvsp[(5) - (5)]));
  (yyval) = itex2MML_copy3(s1, s2, "</msubsup>");
  itex2MML_free_string(s1);
  itex2MML_free_string(s2);
  itex2MML_free_string((yyvsp[(1) - (5)]));
  itex2MML_free_string((yyvsp[(3) - (5)]));
  itex2MML_free_string((yyvsp[(5) - (5)]));
}
    break;

  case 24:

/* Line 1806 of yacc.c  */
#line 476 "itex2MML.y"
    {
  char * s1 = itex2MML_copy3("<msubsup>", (yyvsp[(1) - (5)]), " ");
  char * s2 = itex2MML_copy3((yyvsp[(5) - (5)]), " ", (yyvsp[(3) - (5)]));
  (yyval) = itex2MML_copy3(s1, s2, "</msubsup>");
  itex2MML_free_string(s1);
  itex2MML_free_string(s2);
  itex2MML_free_string((yyvsp[(1) - (5)]));
  itex2MML_free_string((yyvsp[(3) - (5)]));
  itex2MML_free_string((yyvsp[(5) - (5)]));
}
    break;

  case 25:

/* Line 1806 of yacc.c  */
#line 486 "itex2MML.y"
    {
  char * s1 = itex2MML_copy3("<msub>", (yyvsp[(1) - (3)]), " ");
  (yyval) = itex2MML_copy3(s1, (yyvsp[(3) - (3)]), "</msub>");
  itex2MML_free_string(s1);
  itex2MML_free_string((yyvsp[(1) - (3)]));
  itex2MML_free_string((yyvsp[(3) - (3)]));
}
    break;

  case 26:

/* Line 1806 of yacc.c  */
#line 493 "itex2MML.y"
    {
  char * s1 = itex2MML_copy3("<msup>", (yyvsp[(1) - (3)]), " ");
  (yyval) = itex2MML_copy3(s1, (yyvsp[(3) - (3)]), "</msup>");
  itex2MML_free_string(s1);
  itex2MML_free_string((yyvsp[(1) - (3)]));
  itex2MML_free_string((yyvsp[(3) - (3)]));
}
    break;

  case 27:

/* Line 1806 of yacc.c  */
#line 500 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<msub><mo></mo>", (yyvsp[(2) - (2)]), "</msub>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 28:

/* Line 1806 of yacc.c  */
#line 504 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<msup><mo></mo>", (yyvsp[(2) - (2)]), "</msup>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 29:

/* Line 1806 of yacc.c  */
#line 508 "itex2MML.y"
    {
  (yyval) = itex2MML_copy_string((yyvsp[(1) - (1)]));
  itex2MML_free_string((yyvsp[(1) - (1)]));
}
    break;

  case 34:

/* Line 1806 of yacc.c  */
#line 517 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mi>", (yyvsp[(1) - (1)]), "</mi>");
  itex2MML_free_string((yyvsp[(1) - (1)]));
}
    break;

  case 35:

/* Line 1806 of yacc.c  */
#line 521 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mn>", (yyvsp[(1) - (1)]), "</mn>");
  itex2MML_free_string((yyvsp[(1) - (1)]));
}
    break;

  case 95:

/* Line 1806 of yacc.c  */
#line 584 "itex2MML.y"
    {
  (yyval) = itex2MML_copy_string((yyvsp[(2) - (3)]));
  itex2MML_free_string((yyvsp[(2) - (3)]));
}
    break;

  case 96:

/* Line 1806 of yacc.c  */
#line 588 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mrow>", (yyvsp[(2) - (3)]), "</mrow>");
  itex2MML_free_string((yyvsp[(2) - (3)]));
}
    break;

  case 97:

/* Line 1806 of yacc.c  */
#line 592 "itex2MML.y"
    {
  char * s1 = itex2MML_copy3("<mrow>", (yyvsp[(1) - (3)]), (yyvsp[(2) - (3)]));
  (yyval) = itex2MML_copy3(s1, (yyvsp[(3) - (3)]), "</mrow>");
  itex2MML_free_string(s1);
  itex2MML_free_string((yyvsp[(1) - (3)]));
  itex2MML_free_string((yyvsp[(2) - (3)]));
  itex2MML_free_string((yyvsp[(3) - (3)]));
}
    break;

  case 102:

/* Line 1806 of yacc.c  */
#line 605 "itex2MML.y"
    {
  itex2MML_rowposn = 2;
  (yyval) = itex2MML_copy3("<mo>", (yyvsp[(2) - (2)]), "</mo>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 103:

/* Line 1806 of yacc.c  */
#line 610 "itex2MML.y"
    {
  itex2MML_rowposn = 2;
  (yyval) = itex2MML_copy3("<mo>", (yyvsp[(2) - (2)]), "</mo>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 104:

/* Line 1806 of yacc.c  */
#line 615 "itex2MML.y"
    {
  itex2MML_rowposn = 2;
  (yyval) = itex2MML_copy_string("");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 105:

/* Line 1806 of yacc.c  */
#line 621 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mo>", (yyvsp[(2) - (2)]), "</mo>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 106:

/* Line 1806 of yacc.c  */
#line 625 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mo>", (yyvsp[(2) - (2)]), "</mo>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 107:

/* Line 1806 of yacc.c  */
#line 629 "itex2MML.y"
    {
  (yyval) = itex2MML_copy_string("");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 108:

/* Line 1806 of yacc.c  */
#line 634 "itex2MML.y"
    {
  itex2MML_rowposn = 2;
  (yyval) = itex2MML_copy3("<mo maxsize=\"1.2em\" minsize=\"1.2em\">", (yyvsp[(2) - (2)]), "</mo>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 109:

/* Line 1806 of yacc.c  */
#line 639 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mo maxsize=\"1.2em\" minsize=\"1.2em\">", (yyvsp[(2) - (2)]), "</mo>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 110:

/* Line 1806 of yacc.c  */
#line 643 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mo maxsize=\"1.2em\" minsize=\"1.2em\">", (yyvsp[(2) - (2)]), "</mo>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 111:

/* Line 1806 of yacc.c  */
#line 647 "itex2MML.y"
    {
  itex2MML_rowposn = 2;
  (yyval) = itex2MML_copy3("<mo maxsize=\"1.8em\" minsize=\"1.8em\">", (yyvsp[(2) - (2)]), "</mo>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 112:

/* Line 1806 of yacc.c  */
#line 652 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mo maxsize=\"1.8em\" minsize=\"1.8em\">", (yyvsp[(2) - (2)]), "</mo>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 113:

/* Line 1806 of yacc.c  */
#line 656 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mo maxsize=\"1.8em\" minsize=\"1.8em\">", (yyvsp[(2) - (2)]), "</mo>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 114:

/* Line 1806 of yacc.c  */
#line 660 "itex2MML.y"
    {
  itex2MML_rowposn = 2;
  (yyval) = itex2MML_copy3("<mo maxsize=\"2.4em\" minsize=\"2.4em\">", (yyvsp[(2) - (2)]), "</mo>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 115:

/* Line 1806 of yacc.c  */
#line 665 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mo maxsize=\"2.4em\" minsize=\"2.4em\">", (yyvsp[(2) - (2)]), "</mo>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 116:

/* Line 1806 of yacc.c  */
#line 669 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mo maxsize=\"2.4em\" minsize=\"2.4em\">", (yyvsp[(2) - (2)]), "</mo>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 117:

/* Line 1806 of yacc.c  */
#line 673 "itex2MML.y"
    {
  itex2MML_rowposn = 2;
  (yyval) = itex2MML_copy3("<mo maxsize=\"3em\" minsize=\"3em\">", (yyvsp[(2) - (2)]), "</mo>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 118:

/* Line 1806 of yacc.c  */
#line 678 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mo maxsize=\"3em\" minsize=\"3em\">", (yyvsp[(2) - (2)]), "</mo>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 119:

/* Line 1806 of yacc.c  */
#line 682 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mo maxsize=\"3em\" minsize=\"3em\">", (yyvsp[(2) - (2)]), "</mo>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 120:

/* Line 1806 of yacc.c  */
#line 686 "itex2MML.y"
    {
  itex2MML_rowposn = 2;
  (yyval) = itex2MML_copy3("<mo maxsize=\"1.2em\" minsize=\"1.2em\">", (yyvsp[(2) - (2)]), "</mo>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 121:

/* Line 1806 of yacc.c  */
#line 691 "itex2MML.y"
    {
  itex2MML_rowposn = 2;
  (yyval) = itex2MML_copy3("<mo maxsize=\"1.2em\" minsize=\"1.2em\">", (yyvsp[(2) - (2)]), "</mo>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 122:

/* Line 1806 of yacc.c  */
#line 696 "itex2MML.y"
    {
  itex2MML_rowposn = 2;
  (yyval) = itex2MML_copy3("<mo maxsize=\"1.8em\" minsize=\"1.8em\">", (yyvsp[(2) - (2)]), "</mo>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 123:

/* Line 1806 of yacc.c  */
#line 701 "itex2MML.y"
    {
  itex2MML_rowposn = 2;
  (yyval) = itex2MML_copy3("<mo maxsize=\"1.8em\" minsize=\"1.8em\">", (yyvsp[(2) - (2)]), "</mo>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 124:

/* Line 1806 of yacc.c  */
#line 706 "itex2MML.y"
    {
  itex2MML_rowposn = 2;
  (yyval) = itex2MML_copy3("<mo maxsize=\"2.4em\" minsize=\"2.4em\">", (yyvsp[(2) - (2)]), "</mo>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 125:

/* Line 1806 of yacc.c  */
#line 711 "itex2MML.y"
    {
  itex2MML_rowposn = 2;
  (yyval) = itex2MML_copy3("<mo maxsize=\"2.4em\" minsize=\"2.4em\">", (yyvsp[(2) - (2)]), "</mo>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 126:

/* Line 1806 of yacc.c  */
#line 716 "itex2MML.y"
    {
  itex2MML_rowposn = 2;
  (yyval) = itex2MML_copy3("<mo maxsize=\"3em\" minsize=\"3em\">", (yyvsp[(2) - (2)]), "</mo>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 127:

/* Line 1806 of yacc.c  */
#line 721 "itex2MML.y"
    {
  itex2MML_rowposn = 2;
  (yyval) = itex2MML_copy3("<mo maxsize=\"3em\" minsize=\"3em\">", (yyvsp[(2) - (2)]), "</mo>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 128:

/* Line 1806 of yacc.c  */
#line 727 "itex2MML.y"
    {
  (yyval) = itex2MML_copy_string("<merror><mtext>Unknown character</mtext></merror>");
}
    break;

  case 129:

/* Line 1806 of yacc.c  */
#line 731 "itex2MML.y"
    {
  (yyval) = itex2MML_copy_string("<mo lspace=\"verythinmathspace\" rspace=\"0em\">&minus;</mo>");
}
    break;

  case 130:

/* Line 1806 of yacc.c  */
#line 735 "itex2MML.y"
    {
  (yyval) = itex2MML_copy_string("<mo lspace=\"verythinmathspace\" rspace=\"0em\">+</mo>");
}
    break;

  case 132:

/* Line 1806 of yacc.c  */
#line 741 "itex2MML.y"
    {
  itex2MML_rowposn=2;
  (yyval) = itex2MML_copy3("<mi>", (yyvsp[(1) - (1)]), "</mi>");
  itex2MML_free_string((yyvsp[(1) - (1)]));
}
    break;

  case 134:

/* Line 1806 of yacc.c  */
#line 748 "itex2MML.y"
    {
  itex2MML_rowposn = 2;
  (yyval) = itex2MML_copy_string((yyvsp[(2) - (2)]));
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 135:

/* Line 1806 of yacc.c  */
#line 754 "itex2MML.y"
    {
  itex2MML_rowposn = 2;
  (yyval) = itex2MML_copy3("<mo lspace=\"thinmathspace\" rspace=\"thinmathspace\">", (yyvsp[(1) - (1)]), "</mo>");
  itex2MML_free_string((yyvsp[(1) - (1)]));
}
    break;

  case 138:

/* Line 1806 of yacc.c  */
#line 762 "itex2MML.y"
    {
  itex2MML_rowposn = 2;
  (yyval) = itex2MML_copy3("<mo>", (yyvsp[(1) - (1)]), "</mo>");
  itex2MML_free_string((yyvsp[(1) - (1)]));
}
    break;

  case 139:

/* Line 1806 of yacc.c  */
#line 767 "itex2MML.y"
    {
  itex2MML_rowposn = 2;
  (yyval) = itex2MML_copy3("<mo>", (yyvsp[(1) - (1)]), "</mo>");
  itex2MML_free_string((yyvsp[(1) - (1)]));
}
    break;

  case 140:

/* Line 1806 of yacc.c  */
#line 772 "itex2MML.y"
    {
  itex2MML_rowposn = 2;
  (yyval) = itex2MML_copy3("<mstyle scriptlevel=\"0\"><mo>", (yyvsp[(1) - (1)]), "</mo></mstyle>");
  itex2MML_free_string((yyvsp[(1) - (1)]));
}
    break;

  case 141:

/* Line 1806 of yacc.c  */
#line 777 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mo stretchy=\"false\">", (yyvsp[(1) - (1)]), "</mo>");
  itex2MML_free_string((yyvsp[(1) - (1)]));
}
    break;

  case 142:

/* Line 1806 of yacc.c  */
#line 781 "itex2MML.y"
    {
  itex2MML_rowposn = 2;
  (yyval) = itex2MML_copy3("<mo stretchy=\"false\">", (yyvsp[(1) - (1)]), "</mo>");
  itex2MML_free_string((yyvsp[(1) - (1)]));
}
    break;

  case 143:

/* Line 1806 of yacc.c  */
#line 786 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mo stretchy=\"false\">", (yyvsp[(1) - (1)]), "</mo>");
  itex2MML_free_string((yyvsp[(1) - (1)]));
}
    break;

  case 144:

/* Line 1806 of yacc.c  */
#line 790 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mo stretchy=\"false\">", (yyvsp[(1) - (1)]), "</mo>");
  itex2MML_free_string((yyvsp[(1) - (1)]));
}
    break;

  case 145:

/* Line 1806 of yacc.c  */
#line 794 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mo>", (yyvsp[(1) - (1)]), "</mo>");
  itex2MML_free_string((yyvsp[(1) - (1)]));
}
    break;

  case 146:

/* Line 1806 of yacc.c  */
#line 798 "itex2MML.y"
    {
  itex2MML_rowposn=2;
  (yyval) = itex2MML_copy3("<mo lspace=\"mediummathspace\" rspace=\"mediummathspace\">", (yyvsp[(1) - (1)]), "</mo>");
  itex2MML_free_string((yyvsp[(1) - (1)]));
}
    break;

  case 147:

/* Line 1806 of yacc.c  */
#line 803 "itex2MML.y"
    {
  itex2MML_rowposn = 2;
  (yyval) = itex2MML_copy3("<mo lspace=\"0em\" rspace=\"thinmathspace\">", (yyvsp[(1) - (1)]), "</mo>");
  itex2MML_free_string((yyvsp[(1) - (1)]));
}
    break;

  case 148:

/* Line 1806 of yacc.c  */
#line 808 "itex2MML.y"
    {
  itex2MML_rowposn = 2;
  (yyval) = itex2MML_copy3("<mo lspace=\"verythinmathspace\">", (yyvsp[(1) - (1)]), "</mo>");
  itex2MML_free_string((yyvsp[(1) - (1)]));
}
    break;

  case 149:

/* Line 1806 of yacc.c  */
#line 813 "itex2MML.y"
    {
  itex2MML_rowposn = 2;
  (yyval) = itex2MML_copy3("<mo lspace=\"0em\" rspace=\"thinmathspace\">", (yyvsp[(2) - (2)]), "</mo>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 150:

/* Line 1806 of yacc.c  */
#line 818 "itex2MML.y"
    {
  itex2MML_rowposn = 2;
  (yyval) = itex2MML_copy3("<mo lspace=\"thinmathspace\" rspace=\"thinmathspace\">", (yyvsp[(2) - (2)]), "</mo>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 151:

/* Line 1806 of yacc.c  */
#line 823 "itex2MML.y"
    {
  itex2MML_rowposn = 2;
  (yyval) = itex2MML_copy3("<mo lspace=\"mediummathspace\" rspace=\"mediummathspace\">", (yyvsp[(2) - (2)]), "</mo>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 152:

/* Line 1806 of yacc.c  */
#line 828 "itex2MML.y"
    {
  itex2MML_rowposn = 2;
  (yyval) = itex2MML_copy3("<mo lspace=\"thickmathspace\" rspace=\"thickmathspace\">", (yyvsp[(2) - (2)]), "</mo>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 153:

/* Line 1806 of yacc.c  */
#line 834 "itex2MML.y"
    {
  char * s1 = itex2MML_copy3("<mspace height=\"", (yyvsp[(3) - (10)]), "ex\" depth=\"");
  char * s2 = itex2MML_copy3((yyvsp[(6) - (10)]), "ex\" width=\"", (yyvsp[(9) - (10)]));
  (yyval) = itex2MML_copy3(s1, s2, "em\"></mspace>");
  itex2MML_free_string(s1);
  itex2MML_free_string(s2);
  itex2MML_free_string((yyvsp[(3) - (10)]));
  itex2MML_free_string((yyvsp[(6) - (10)]));
  itex2MML_free_string((yyvsp[(9) - (10)]));
}
    break;

  case 154:

/* Line 1806 of yacc.c  */
#line 845 "itex2MML.y"
    {
  char * s1 = itex2MML_copy3("<maction actiontype=\"statusline\">", (yyvsp[(3) - (3)]), "<mtext>");
  (yyval) = itex2MML_copy3(s1, (yyvsp[(2) - (3)]), "</mtext></maction>");
  itex2MML_free_string(s1);
  itex2MML_free_string((yyvsp[(2) - (3)]));
  itex2MML_free_string((yyvsp[(3) - (3)]));
}
    break;

  case 155:

/* Line 1806 of yacc.c  */
#line 853 "itex2MML.y"
    {
  char * s1 = itex2MML_copy3("<maction actiontype=\"tooltip\">", (yyvsp[(3) - (3)]), "<mtext>");
  (yyval) = itex2MML_copy3(s1, (yyvsp[(2) - (3)]), "</mtext></maction>");
  itex2MML_free_string(s1);
  itex2MML_free_string((yyvsp[(2) - (3)]));
  itex2MML_free_string((yyvsp[(3) - (3)]));
}
    break;

  case 156:

/* Line 1806 of yacc.c  */
#line 861 "itex2MML.y"
    {
  char * s1 = itex2MML_copy3("<maction actiontype=\"toggle\" selection=\"2\">", (yyvsp[(2) - (3)]), " ");
  (yyval) = itex2MML_copy3(s1, (yyvsp[(3) - (3)]), "</maction>");
  itex2MML_free_string(s1);
  itex2MML_free_string((yyvsp[(2) - (3)]));
  itex2MML_free_string((yyvsp[(3) - (3)]));
}
    break;

  case 157:

/* Line 1806 of yacc.c  */
#line 869 "itex2MML.y"
    {
  char * s1 = itex2MML_copy3("<maction actiontype=\"highlight\" other='color=", (yyvsp[(2) - (3)]), "'>");
  (yyval) = itex2MML_copy3(s1, (yyvsp[(3) - (3)]), "</maction>");
  itex2MML_free_string(s1);
  itex2MML_free_string((yyvsp[(2) - (3)]));
  itex2MML_free_string((yyvsp[(3) - (3)]));
}
    break;

  case 158:

/* Line 1806 of yacc.c  */
#line 877 "itex2MML.y"
    {
  char * s1 = itex2MML_copy3("<maction actiontype=\"highlight\" other='background=", (yyvsp[(2) - (3)]), "'>");
  (yyval) = itex2MML_copy3(s1, (yyvsp[(3) - (3)]), "</maction>");
  itex2MML_free_string(s1);
  itex2MML_free_string((yyvsp[(2) - (3)]));
  itex2MML_free_string((yyvsp[(3) - (3)]));
}
    break;

  case 159:

/* Line 1806 of yacc.c  */
#line 885 "itex2MML.y"
    {
  char * s1 = itex2MML_copy3("<mstyle mathcolor=", (yyvsp[(2) - (3)]), ">");
  (yyval) = itex2MML_copy3(s1, (yyvsp[(3) - (3)]), "</mstyle>");
  itex2MML_free_string(s1);
  itex2MML_free_string((yyvsp[(2) - (3)]));
  itex2MML_free_string((yyvsp[(3) - (3)]));
}
    break;

  case 160:

/* Line 1806 of yacc.c  */
#line 892 "itex2MML.y"
    {
  char * s1 = itex2MML_copy3("<mstyle mathbackground=", (yyvsp[(2) - (3)]), ">");
  (yyval) = itex2MML_copy3(s1, (yyvsp[(3) - (3)]), "</mstyle>");
  itex2MML_free_string(s1);
  itex2MML_free_string((yyvsp[(2) - (3)]));
  itex2MML_free_string((yyvsp[(3) - (3)]));
}
    break;

  case 161:

/* Line 1806 of yacc.c  */
#line 900 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mpadded width=\"0\">", (yyvsp[(2) - (2)]), "</mpadded>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 162:

/* Line 1806 of yacc.c  */
#line 905 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mpadded width=\"0\" lspace=\"-100%width\">", (yyvsp[(2) - (2)]), "</mpadded>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 163:

/* Line 1806 of yacc.c  */
#line 910 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mpadded width=\"0\" lspace=\"-50%width\">", (yyvsp[(2) - (2)]), "</mpadded>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 164:

/* Line 1806 of yacc.c  */
#line 915 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mtext>", (yyvsp[(2) - (2)]), "</mtext>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 165:

/* Line 1806 of yacc.c  */
#line 920 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mstyle displaystyle=\"true\">", (yyvsp[(2) - (2)]), "</mstyle>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 166:

/* Line 1806 of yacc.c  */
#line 925 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mstyle displaystyle=\"false\">", (yyvsp[(2) - (2)]), "</mstyle>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 167:

/* Line 1806 of yacc.c  */
#line 930 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mstyle scriptlevel=\"0\">", (yyvsp[(2) - (2)]), "</mstyle>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 168:

/* Line 1806 of yacc.c  */
#line 935 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mstyle scriptlevel=\"1\">", (yyvsp[(2) - (2)]), "</mstyle>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 169:

/* Line 1806 of yacc.c  */
#line 940 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mstyle scriptlevel=\"2\">", (yyvsp[(2) - (2)]), "</mstyle>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 170:

/* Line 1806 of yacc.c  */
#line 945 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mstyle mathvariant=\"italic\">", (yyvsp[(2) - (2)]), "</mstyle>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 171:

/* Line 1806 of yacc.c  */
#line 950 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<menclose notation=\"updiagonalstrike\">", (yyvsp[(2) - (2)]), "</menclose>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 172:

/* Line 1806 of yacc.c  */
#line 955 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<menclose notation=\"box\">", (yyvsp[(2) - (2)]), "</menclose>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 173:

/* Line 1806 of yacc.c  */
#line 960 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mstyle mathvariant=\"bold\">", (yyvsp[(2) - (2)]), "</mstyle>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 174:

/* Line 1806 of yacc.c  */
#line 965 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mi mathvariant=\"normal\">", (yyvsp[(3) - (4)]), "</mi>");
  itex2MML_free_string((yyvsp[(3) - (4)]));
}
    break;

  case 175:

/* Line 1806 of yacc.c  */
#line 970 "itex2MML.y"
    {
  (yyval) = itex2MML_copy_string((yyvsp[(1) - (1)]));
  itex2MML_free_string((yyvsp[(1) - (1)]));
}
    break;

  case 176:

/* Line 1806 of yacc.c  */
#line 974 "itex2MML.y"
    {
  (yyval) = itex2MML_copy2((yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));
  itex2MML_free_string((yyvsp[(1) - (2)]));
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 177:

/* Line 1806 of yacc.c  */
#line 980 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mi>", (yyvsp[(3) - (4)]), "</mi>");
  itex2MML_free_string((yyvsp[(3) - (4)]));
}
    break;

  case 178:

/* Line 1806 of yacc.c  */
#line 985 "itex2MML.y"
    {
  (yyval) = itex2MML_copy_string((yyvsp[(1) - (1)]));
  itex2MML_free_string((yyvsp[(1) - (1)]));
}
    break;

  case 179:

/* Line 1806 of yacc.c  */
#line 989 "itex2MML.y"
    {
  (yyval) = itex2MML_copy2((yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));
  itex2MML_free_string((yyvsp[(1) - (2)]));
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 180:

/* Line 1806 of yacc.c  */
#line 995 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("&", (yyvsp[(1) - (1)]), "opf;");
  itex2MML_free_string((yyvsp[(1) - (1)]));
}
    break;

  case 181:

/* Line 1806 of yacc.c  */
#line 999 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("&", (yyvsp[(1) - (1)]), "opf;");
  itex2MML_free_string((yyvsp[(1) - (1)]));
}
    break;

  case 182:

/* Line 1806 of yacc.c  */
#line 1003 "itex2MML.y"
    {
  /* Blackboard digits 0-9 correspond to Unicode characters 0x1D7D8-0x1D7E1 */
  char * end = (yyvsp[(1) - (1)]) + 1;
  int code = 0x1D7D8 + strtoul((yyvsp[(1) - (1)]), &end, 10);
  (yyval) = itex2MML_character_reference(code);
  itex2MML_free_string((yyvsp[(1) - (1)]));
}
    break;

  case 183:

/* Line 1806 of yacc.c  */
#line 1011 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mi>", (yyvsp[(3) - (4)]), "</mi>");
  itex2MML_free_string((yyvsp[(3) - (4)]));
}
    break;

  case 184:

/* Line 1806 of yacc.c  */
#line 1016 "itex2MML.y"
    {
  (yyval) = itex2MML_copy_string((yyvsp[(1) - (1)]));
  itex2MML_free_string((yyvsp[(1) - (1)]));
}
    break;

  case 185:

/* Line 1806 of yacc.c  */
#line 1020 "itex2MML.y"
    {
  (yyval) = itex2MML_copy2((yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));
  itex2MML_free_string((yyvsp[(1) - (2)]));
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 186:

/* Line 1806 of yacc.c  */
#line 1026 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("&", (yyvsp[(1) - (1)]), "fr;");
  itex2MML_free_string((yyvsp[(1) - (1)]));
}
    break;

  case 187:

/* Line 1806 of yacc.c  */
#line 1031 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mi>", (yyvsp[(3) - (4)]), "</mi>");
  itex2MML_free_string((yyvsp[(3) - (4)]));
}
    break;

  case 188:

/* Line 1806 of yacc.c  */
#line 1036 "itex2MML.y"
    {
  (yyval) = itex2MML_copy_string((yyvsp[(1) - (1)]));
  itex2MML_free_string((yyvsp[(1) - (1)]));
}
    break;

  case 189:

/* Line 1806 of yacc.c  */
#line 1040 "itex2MML.y"
    {
  (yyval) = itex2MML_copy2((yyvsp[(1) - (2)]), (yyvsp[(2) - (2)]));
  itex2MML_free_string((yyvsp[(1) - (2)]));
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 190:

/* Line 1806 of yacc.c  */
#line 1046 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("&", (yyvsp[(1) - (1)]), "scr;");
  itex2MML_free_string((yyvsp[(1) - (1)]));
}
    break;

  case 191:

/* Line 1806 of yacc.c  */
#line 1051 "itex2MML.y"
    {
  (yyval) = itex2MML_copy_string("<mspace width=\"thinmathspace\"></mspace>");
}
    break;

  case 192:

/* Line 1806 of yacc.c  */
#line 1055 "itex2MML.y"
    {
  (yyval) = itex2MML_copy_string("<mspace width=\"mediummathspace\"></mspace>");
}
    break;

  case 193:

/* Line 1806 of yacc.c  */
#line 1059 "itex2MML.y"
    {
  (yyval) = itex2MML_copy_string("<mspace width=\"thickmathspace\"></mspace>");
}
    break;

  case 194:

/* Line 1806 of yacc.c  */
#line 1063 "itex2MML.y"
    {
  (yyval) = itex2MML_copy_string("<mspace width=\"1em\"></mspace>");
}
    break;

  case 195:

/* Line 1806 of yacc.c  */
#line 1067 "itex2MML.y"
    {
  (yyval) = itex2MML_copy_string("<mspace width=\"2em\"></mspace>");
}
    break;

  case 196:

/* Line 1806 of yacc.c  */
#line 1071 "itex2MML.y"
    {
  (yyval) = itex2MML_copy_string("<mspace width=\"-0.1667 em\"></mspace>");
}
    break;

  case 197:

/* Line 1806 of yacc.c  */
#line 1075 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mphantom>", (yyvsp[(2) - (2)]), "</mphantom>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 198:

/* Line 1806 of yacc.c  */
#line 1080 "itex2MML.y"
    {
  char * s1 = itex2MML_copy3("<mrow href=\"", (yyvsp[(2) - (3)]), "\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" xlink:type=\"simple\" xlink:href=\"");
  char * s2 = itex2MML_copy3(s1, (yyvsp[(2) - (3)]), "\">");
  (yyval) = itex2MML_copy3(s2, (yyvsp[(3) - (3)]), "</mrow>");
  itex2MML_free_string(s1);
  itex2MML_free_string(s2);
  itex2MML_free_string((yyvsp[(2) - (3)]));
  itex2MML_free_string((yyvsp[(3) - (3)]));
}
    break;

  case 199:

/* Line 1806 of yacc.c  */
#line 1090 "itex2MML.y"
    {
  char * s1 = itex2MML_copy3("<mmultiscripts>", (yyvsp[(2) - (5)]), (yyvsp[(4) - (5)]));
  (yyval) = itex2MML_copy2(s1, "</mmultiscripts>");
  itex2MML_free_string(s1);
  itex2MML_free_string((yyvsp[(2) - (5)]));
  itex2MML_free_string((yyvsp[(4) - (5)]));
}
    break;

  case 200:

/* Line 1806 of yacc.c  */
#line 1097 "itex2MML.y"
    {
  char * s1 = itex2MML_copy3("<mmultiscripts>", (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));
  (yyval) = itex2MML_copy2(s1, "</mmultiscripts>");
  itex2MML_free_string(s1);
  itex2MML_free_string((yyvsp[(2) - (3)]));
  itex2MML_free_string((yyvsp[(3) - (3)]));
}
    break;

  case 201:

/* Line 1806 of yacc.c  */
#line 1105 "itex2MML.y"
    {
  char * s1 = itex2MML_copy3("<mmultiscripts>", (yyvsp[(5) - (8)]), (yyvsp[(7) - (8)]));
  char * s2 = itex2MML_copy3("<mprescripts></mprescripts>", (yyvsp[(3) - (8)]), "</mmultiscripts>");
  (yyval) = itex2MML_copy2(s1, s2);
  itex2MML_free_string(s1);
  itex2MML_free_string(s2);
  itex2MML_free_string((yyvsp[(3) - (8)]));
  itex2MML_free_string((yyvsp[(5) - (8)]));
  itex2MML_free_string((yyvsp[(7) - (8)]));
}
    break;

  case 202:

/* Line 1806 of yacc.c  */
#line 1115 "itex2MML.y"
    {
  char * s1 = itex2MML_copy2("<mmultiscripts>", (yyvsp[(5) - (6)]));
  char * s2 = itex2MML_copy3("<mprescripts></mprescripts>", (yyvsp[(3) - (6)]), "</mmultiscripts>");
  (yyval) = itex2MML_copy2(s1, s2);
  itex2MML_free_string(s1);
  itex2MML_free_string(s2);
  itex2MML_free_string((yyvsp[(3) - (6)]));
  itex2MML_free_string((yyvsp[(5) - (6)]));
}
    break;

  case 203:

/* Line 1806 of yacc.c  */
#line 1124 "itex2MML.y"
    {
  char * s1 = itex2MML_copy3("<mmultiscripts>", (yyvsp[(3) - (6)]), (yyvsp[(5) - (6)]));
  (yyval) = itex2MML_copy2(s1, "</mmultiscripts>");
  itex2MML_free_string(s1);
  itex2MML_free_string((yyvsp[(3) - (6)]));
  itex2MML_free_string((yyvsp[(5) - (6)])); 
}
    break;

  case 204:

/* Line 1806 of yacc.c  */
#line 1132 "itex2MML.y"
    {
  (yyval) = itex2MML_copy_string((yyvsp[(1) - (1)]));
  itex2MML_free_string((yyvsp[(1) - (1)]));
}
    break;

  case 205:

/* Line 1806 of yacc.c  */
#line 1136 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3((yyvsp[(1) - (2)]), " ", (yyvsp[(2) - (2)]));
  itex2MML_free_string((yyvsp[(1) - (2)]));
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 206:

/* Line 1806 of yacc.c  */
#line 1142 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3((yyvsp[(2) - (4)]), " ", (yyvsp[(4) - (4)]));
  itex2MML_free_string((yyvsp[(2) - (4)]));
  itex2MML_free_string((yyvsp[(4) - (4)]));
}
    break;

  case 207:

/* Line 1806 of yacc.c  */
#line 1147 "itex2MML.y"
    {
  (yyval) = itex2MML_copy2((yyvsp[(2) - (2)]), " <none></none>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 208:

/* Line 1806 of yacc.c  */
#line 1151 "itex2MML.y"
    {
  (yyval) = itex2MML_copy2("<none></none> ", (yyvsp[(2) - (2)]));
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 209:

/* Line 1806 of yacc.c  */
#line 1155 "itex2MML.y"
    {
  (yyval) = itex2MML_copy2("<none></none> ", (yyvsp[(3) - (3)]));
  itex2MML_free_string((yyvsp[(3) - (3)]));
}
    break;

  case 210:

/* Line 1806 of yacc.c  */
#line 1160 "itex2MML.y"
    {
  char * s1 = itex2MML_copy3("<mfrac>", (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));
  (yyval) = itex2MML_copy2(s1, "</mfrac>");
  itex2MML_free_string(s1);
  itex2MML_free_string((yyvsp[(2) - (3)]));
  itex2MML_free_string((yyvsp[(3) - (3)]));
}
    break;

  case 211:

/* Line 1806 of yacc.c  */
#line 1167 "itex2MML.y"
    {
  char * s1 = itex2MML_copy3("<mstyle displaystyle=\"false\"><mfrac>", (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));
  (yyval) = itex2MML_copy2(s1, "</mfrac></mstyle>");
  itex2MML_free_string(s1);
  itex2MML_free_string((yyvsp[(2) - (3)]));
  itex2MML_free_string((yyvsp[(3) - (3)]));
}
    break;

  case 212:

/* Line 1806 of yacc.c  */
#line 1175 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3( "<mo lspace=\"mediummathspace\">(</mo><mo rspace=\"thinmathspace\">mod</mo>", (yyvsp[(2) - (2)]), "<mo rspace=\"mediummathspace\">)</mo>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 213:

/* Line 1806 of yacc.c  */
#line 1180 "itex2MML.y"
    {
  char * s1 = itex2MML_copy3("<mfrac><mrow>", (yyvsp[(2) - (5)]), "</mrow><mrow>");
  (yyval) = itex2MML_copy3(s1, (yyvsp[(4) - (5)]), "</mrow></mfrac>");
  itex2MML_free_string(s1);
  itex2MML_free_string((yyvsp[(2) - (5)]));
  itex2MML_free_string((yyvsp[(4) - (5)]));
}
    break;

  case 214:

/* Line 1806 of yacc.c  */
#line 1187 "itex2MML.y"
    {
  char * s1 = itex2MML_copy3("<mrow>", (yyvsp[(1) - (5)]), "<mfrac><mrow>");
  char * s2 = itex2MML_copy3((yyvsp[(2) - (5)]), "</mrow><mrow>", (yyvsp[(4) - (5)]));
  char * s3 = itex2MML_copy3("</mrow></mfrac>", (yyvsp[(5) - (5)]), "</mrow>");
  (yyval) = itex2MML_copy3(s1, s2, s3);
  itex2MML_free_string(s1);
  itex2MML_free_string(s2);
  itex2MML_free_string(s3);
  itex2MML_free_string((yyvsp[(1) - (5)]));
  itex2MML_free_string((yyvsp[(2) - (5)]));
  itex2MML_free_string((yyvsp[(4) - (5)]));
  itex2MML_free_string((yyvsp[(5) - (5)]));
}
    break;

  case 215:

/* Line 1806 of yacc.c  */
#line 1201 "itex2MML.y"
    {
  char * s1 = itex2MML_copy3("<mfrac linethickness=\"0\"><mrow>", (yyvsp[(2) - (5)]), "</mrow><mrow>");
  (yyval) = itex2MML_copy3(s1, (yyvsp[(4) - (5)]), "</mrow></mfrac>");
  itex2MML_free_string(s1);
  itex2MML_free_string((yyvsp[(2) - (5)]));
  itex2MML_free_string((yyvsp[(4) - (5)]));
}
    break;

  case 216:

/* Line 1806 of yacc.c  */
#line 1208 "itex2MML.y"
    {
  char * s1 = itex2MML_copy3("<mrow>", (yyvsp[(1) - (5)]), "<mfrac linethickness=\"0\"><mrow>");
  char * s2 = itex2MML_copy3((yyvsp[(2) - (5)]), "</mrow><mrow>", (yyvsp[(4) - (5)]));
  char * s3 = itex2MML_copy3("</mrow></mfrac>", (yyvsp[(5) - (5)]), "</mrow>");
  (yyval) = itex2MML_copy3(s1, s2, s3);
  itex2MML_free_string(s1);
  itex2MML_free_string(s2);
  itex2MML_free_string(s3);
  itex2MML_free_string((yyvsp[(1) - (5)]));
  itex2MML_free_string((yyvsp[(2) - (5)]));
  itex2MML_free_string((yyvsp[(4) - (5)]));
  itex2MML_free_string((yyvsp[(5) - (5)]));
}
    break;

  case 217:

/* Line 1806 of yacc.c  */
#line 1222 "itex2MML.y"
    {
  char * s1 = itex2MML_copy3("<mrow><mo>(</mo><mfrac linethickness=\"0\">", (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));
  (yyval) = itex2MML_copy2(s1, "</mfrac><mo>)</mo></mrow>");
  itex2MML_free_string(s1);
  itex2MML_free_string((yyvsp[(2) - (3)]));
  itex2MML_free_string((yyvsp[(3) - (3)]));
}
    break;

  case 218:

/* Line 1806 of yacc.c  */
#line 1229 "itex2MML.y"
    {
  char * s1 = itex2MML_copy3("<mrow><mo>(</mo><mstyle displaystyle=\"false\"><mfrac linethickness=\"0\">", (yyvsp[(2) - (3)]), (yyvsp[(3) - (3)]));
  (yyval) = itex2MML_copy2(s1, "</mfrac></mstyle><mo>)</mo></mrow>");
  itex2MML_free_string(s1);
  itex2MML_free_string((yyvsp[(2) - (3)]));
  itex2MML_free_string((yyvsp[(3) - (3)]));
}
    break;

  case 219:

/* Line 1806 of yacc.c  */
#line 1237 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<munder>", (yyvsp[(2) - (2)]), "<mo>&UnderBrace;</mo></munder>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 220:

/* Line 1806 of yacc.c  */
#line 1242 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<munder>", (yyvsp[(2) - (2)]), "<mo>&#x00332;</mo></munder>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 221:

/* Line 1806 of yacc.c  */
#line 1247 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mover>", (yyvsp[(2) - (2)]), "<mo>&OverBrace;</mo></mover>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 222:

/* Line 1806 of yacc.c  */
#line 1252 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mover>", (yyvsp[(2) - (2)]), "<mo stretchy=\"false\">&#x000AF;</mo></mover>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 223:

/* Line 1806 of yacc.c  */
#line 1256 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mover>", (yyvsp[(2) - (2)]), "<mo>&#x000AF;</mo></mover>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 224:

/* Line 1806 of yacc.c  */
#line 1261 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mover>", (yyvsp[(2) - (2)]), "<mo stretchy=\"false\">&RightVector;</mo></mover>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 225:

/* Line 1806 of yacc.c  */
#line 1265 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mover>", (yyvsp[(2) - (2)]), "<mo>&RightVector;</mo></mover>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 226:

/* Line 1806 of yacc.c  */
#line 1270 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mover>", (yyvsp[(2) - (2)]), "<mo>&dot;</mo></mover>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 227:

/* Line 1806 of yacc.c  */
#line 1275 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mover>", (yyvsp[(2) - (2)]), "<mo>&Dot;</mo></mover>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 228:

/* Line 1806 of yacc.c  */
#line 1280 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mover>", (yyvsp[(2) - (2)]), "<mo>&tdot;</mo></mover>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 229:

/* Line 1806 of yacc.c  */
#line 1285 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mover>", (yyvsp[(2) - (2)]), "<mo>&DotDot;</mo></mover>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 230:

/* Line 1806 of yacc.c  */
#line 1290 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mover>", (yyvsp[(2) - (2)]), "<mo stretchy=\"false\">&tilde;</mo></mover>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 231:

/* Line 1806 of yacc.c  */
#line 1294 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mover>", (yyvsp[(2) - (2)]), "<mo>&tilde;</mo></mover>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 232:

/* Line 1806 of yacc.c  */
#line 1299 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mover>", (yyvsp[(2) - (2)]), "<mo stretchy=\"false\">&#x2c7;</mo></mover>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 233:

/* Line 1806 of yacc.c  */
#line 1303 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mover>", (yyvsp[(2) - (2)]), "<mo>&#x2c7;</mo></mover>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 234:

/* Line 1806 of yacc.c  */
#line 1308 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mover>", (yyvsp[(2) - (2)]), "<mo stretchy=\"false\">&#x5E;</mo></mover>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 235:

/* Line 1806 of yacc.c  */
#line 1312 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mover>", (yyvsp[(2) - (2)]), "<mo>&#x5E;</mo></mover>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 236:

/* Line 1806 of yacc.c  */
#line 1317 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<msqrt>", (yyvsp[(2) - (2)]), "</msqrt>");
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 237:

/* Line 1806 of yacc.c  */
#line 1322 "itex2MML.y"
    {
  char * s1 = itex2MML_copy3("<mroot>", (yyvsp[(5) - (5)]), (yyvsp[(3) - (5)]));
  (yyval) = itex2MML_copy2(s1, "</mroot>");
  itex2MML_free_string(s1);
  itex2MML_free_string((yyvsp[(3) - (5)]));
  itex2MML_free_string((yyvsp[(5) - (5)]));
}
    break;

  case 238:

/* Line 1806 of yacc.c  */
#line 1329 "itex2MML.y"
    {
  char * s1 = itex2MML_copy3("<mroot>", (yyvsp[(3) - (3)]), (yyvsp[(2) - (3)]));
  (yyval) = itex2MML_copy2(s1, "</mroot>");
  itex2MML_free_string(s1);
  itex2MML_free_string((yyvsp[(2) - (3)]));
  itex2MML_free_string((yyvsp[(3) - (3)]));
}
    break;

  case 239:

/* Line 1806 of yacc.c  */
#line 1337 "itex2MML.y"
    {
  char * s1 = itex2MML_copy3("<mpadded voffset='", (yyvsp[(2) - (5)]), "' height='");
  char * s2 = itex2MML_copy3(s1, (yyvsp[(3) - (5)]), "' depth='");
  char * s3 = itex2MML_copy3(s2, (yyvsp[(4) - (5)]), "'>");
  (yyval) = itex2MML_copy3(s3, (yyvsp[(5) - (5)]), "</mpadded>");
  itex2MML_free_string(s1);
  itex2MML_free_string(s2);
  itex2MML_free_string(s3);
  itex2MML_free_string((yyvsp[(2) - (5)]));
  itex2MML_free_string((yyvsp[(3) - (5)]));
  itex2MML_free_string((yyvsp[(4) - (5)]));
  itex2MML_free_string((yyvsp[(5) - (5)]));
}
    break;

  case 240:

/* Line 1806 of yacc.c  */
#line 1350 "itex2MML.y"
    {
  char * s1 = itex2MML_copy3("<mpadded voffset='-", (yyvsp[(3) - (6)]), "' height='");
  char * s2 = itex2MML_copy3(s1, (yyvsp[(4) - (6)]), "' depth='");
  char * s3 = itex2MML_copy3(s2, (yyvsp[(5) - (6)]), "'>");
  (yyval) = itex2MML_copy3(s3, (yyvsp[(6) - (6)]), "</mpadded>");
  itex2MML_free_string(s1);
  itex2MML_free_string(s2);
  itex2MML_free_string(s3);
  itex2MML_free_string((yyvsp[(3) - (6)]));
  itex2MML_free_string((yyvsp[(4) - (6)]));
  itex2MML_free_string((yyvsp[(5) - (6)]));
  itex2MML_free_string((yyvsp[(6) - (6)]));
}
    break;

  case 241:

/* Line 1806 of yacc.c  */
#line 1363 "itex2MML.y"
    {
  char * s1 = itex2MML_copy3("<mpadded voffset='", (yyvsp[(2) - (4)]), "' height='");
  char * s2 = itex2MML_copy3(s1, (yyvsp[(3) - (4)]), "' depth='depth'>");
  (yyval) = itex2MML_copy3(s2, (yyvsp[(4) - (4)]), "</mpadded>");
  itex2MML_free_string(s1);
  itex2MML_free_string(s2);
  itex2MML_free_string((yyvsp[(2) - (4)]));
  itex2MML_free_string((yyvsp[(3) - (4)]));
  itex2MML_free_string((yyvsp[(4) - (4)]));
}
    break;

  case 242:

/* Line 1806 of yacc.c  */
#line 1373 "itex2MML.y"
    {
  char * s1 = itex2MML_copy3("<mpadded voffset='-", (yyvsp[(3) - (5)]), "' height='");
  char * s2 = itex2MML_copy3(s1, (yyvsp[(4) - (5)]), "' depth='+");
  char * s3 = itex2MML_copy3(s2, (yyvsp[(3) - (5)]), "'>");
  (yyval) = itex2MML_copy3(s3, (yyvsp[(5) - (5)]), "</mpadded>");
  itex2MML_free_string(s1);
  itex2MML_free_string(s2);
  itex2MML_free_string(s3);
  itex2MML_free_string((yyvsp[(3) - (5)]));
  itex2MML_free_string((yyvsp[(4) - (5)]));
  itex2MML_free_string((yyvsp[(5) - (5)]));
}
    break;

  case 243:

/* Line 1806 of yacc.c  */
#line 1385 "itex2MML.y"
    {
  char * s1 = itex2MML_copy3("<mpadded voffset='", (yyvsp[(2) - (3)]), "' height='+");
  char * s2 = itex2MML_copy3(s1, (yyvsp[(2) - (3)]), "' depth='depth'>");
  (yyval) = itex2MML_copy3(s2, (yyvsp[(3) - (3)]), "</mpadded>");
  itex2MML_free_string(s1);
  itex2MML_free_string(s2);
  itex2MML_free_string((yyvsp[(2) - (3)]));
  itex2MML_free_string((yyvsp[(3) - (3)]));
}
    break;

  case 244:

/* Line 1806 of yacc.c  */
#line 1394 "itex2MML.y"
    {
  char * s1 = itex2MML_copy3("<mpadded voffset='-", (yyvsp[(3) - (4)]), "' height='0pt' depth='+");
  char * s2 = itex2MML_copy3(s1, (yyvsp[(3) - (4)]), "'>");
  (yyval) = itex2MML_copy3(s2, (yyvsp[(4) - (4)]), "</mpadded>");
  itex2MML_free_string(s1);
  itex2MML_free_string(s2);
  itex2MML_free_string((yyvsp[(3) - (4)]));
  itex2MML_free_string((yyvsp[(4) - (4)]));
}
    break;

  case 245:

/* Line 1806 of yacc.c  */
#line 1404 "itex2MML.y"
    {
  char * s1 = itex2MML_copy3("<munder><mo>", (yyvsp[(1) - (5)]), "</mo><mrow>");
  (yyval) = itex2MML_copy3(s1, (yyvsp[(3) - (5)]), "</mrow></munder>");
  itex2MML_free_string(s1);
  itex2MML_free_string((yyvsp[(1) - (5)]));
  itex2MML_free_string((yyvsp[(3) - (5)]));
}
    break;

  case 246:

/* Line 1806 of yacc.c  */
#line 1411 "itex2MML.y"
    {
  char * s1 = itex2MML_copy3("<munder>", (yyvsp[(3) - (3)]), (yyvsp[(2) - (3)]));
  (yyval) = itex2MML_copy2(s1, "</munder>");
  itex2MML_free_string(s1);
  itex2MML_free_string((yyvsp[(2) - (3)]));
  itex2MML_free_string((yyvsp[(3) - (3)]));
}
    break;

  case 247:

/* Line 1806 of yacc.c  */
#line 1419 "itex2MML.y"
    {
  char * s1 = itex2MML_copy3("<mover><mo>", (yyvsp[(1) - (2)]), "</mo>");
  (yyval) =  itex2MML_copy3(s1, (yyvsp[(2) - (2)]), "</mover>");
  itex2MML_free_string(s1);
  itex2MML_free_string((yyvsp[(1) - (2)]));
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 248:

/* Line 1806 of yacc.c  */
#line 1426 "itex2MML.y"
    {
  char * s1 = itex2MML_copy3("<mover>", (yyvsp[(3) - (3)]), (yyvsp[(2) - (3)]));
  (yyval) = itex2MML_copy2(s1, "</mover>");
  itex2MML_free_string(s1);
  itex2MML_free_string((yyvsp[(2) - (3)]));
  itex2MML_free_string((yyvsp[(3) - (3)]));
}
    break;

  case 249:

/* Line 1806 of yacc.c  */
#line 1434 "itex2MML.y"
    {
  char * s1 = itex2MML_copy3("<munderover><mo>", (yyvsp[(1) - (5)]), "</mo><mrow>");
  char * s2 = itex2MML_copy3(s1, (yyvsp[(3) - (5)]), "</mrow>");
  (yyval) = itex2MML_copy3(s2, (yyvsp[(5) - (5)]), "</munderover>");
  itex2MML_free_string(s1);
  itex2MML_free_string(s2);
  itex2MML_free_string((yyvsp[(1) - (5)]));
  itex2MML_free_string((yyvsp[(3) - (5)]));
  itex2MML_free_string((yyvsp[(5) - (5)]));
}
    break;

  case 250:

/* Line 1806 of yacc.c  */
#line 1444 "itex2MML.y"
    {
  char * s1 = itex2MML_copy3("<munderover>", (yyvsp[(4) - (4)]), (yyvsp[(2) - (4)]));
  (yyval) = itex2MML_copy3(s1, (yyvsp[(3) - (4)]), "</munderover>");
  itex2MML_free_string(s1);
  itex2MML_free_string((yyvsp[(2) - (4)]));
  itex2MML_free_string((yyvsp[(3) - (4)]));
  itex2MML_free_string((yyvsp[(4) - (4)]));
}
    break;

  case 251:

/* Line 1806 of yacc.c  */
#line 1453 "itex2MML.y"
    {
  (yyval) = itex2MML_copy_string("<mrow></mrow>");
}
    break;

  case 252:

/* Line 1806 of yacc.c  */
#line 1457 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mrow><mtable rowspacing=\"0.5ex\">", (yyvsp[(3) - (5)]), "</mtable></mrow>");
  itex2MML_free_string((yyvsp[(3) - (5)]));
}
    break;

  case 253:

/* Line 1806 of yacc.c  */
#line 1461 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mrow><mtable rowspacing=\"1.0ex\">", (yyvsp[(3) - (5)]), "</mtable></mrow>");
  itex2MML_free_string((yyvsp[(3) - (5)]));
}
    break;

  case 254:

/* Line 1806 of yacc.c  */
#line 1465 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mrow><mo>(</mo><mrow><mtable rowspacing=\"0.5ex\">", (yyvsp[(3) - (5)]), "</mtable></mrow><mo>)</mo></mrow>");
  itex2MML_free_string((yyvsp[(3) - (5)]));
}
    break;

  case 255:

/* Line 1806 of yacc.c  */
#line 1469 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mrow><mo>[</mo><mrow><mtable rowspacing=\"0.5ex\">", (yyvsp[(3) - (5)]), "</mtable></mrow><mo>]</mo></mrow>");
  itex2MML_free_string((yyvsp[(3) - (5)]));
}
    break;

  case 256:

/* Line 1806 of yacc.c  */
#line 1473 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mrow><mo>&VerticalBar;</mo><mrow><mtable rowspacing=\"0.5ex\">", (yyvsp[(3) - (5)]), "</mtable></mrow><mo>&VerticalBar;</mo></mrow>");
  itex2MML_free_string((yyvsp[(3) - (5)]));
}
    break;

  case 257:

/* Line 1806 of yacc.c  */
#line 1477 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mrow><mo>{</mo><mrow><mtable rowspacing=\"0.5ex\">", (yyvsp[(3) - (5)]), "</mtable></mrow><mo>}</mo></mrow>");
  itex2MML_free_string((yyvsp[(3) - (5)]));
}
    break;

  case 258:

/* Line 1806 of yacc.c  */
#line 1481 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mrow><mo>&DoubleVerticalBar;</mo><mrow><mtable rowspacing=\"0.5ex\">", (yyvsp[(3) - (5)]), "</mtable></mrow><mo>&DoubleVerticalBar;</mo></mrow>");
  itex2MML_free_string((yyvsp[(3) - (5)]));
}
    break;

  case 259:

/* Line 1806 of yacc.c  */
#line 1485 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mstyle scriptlevel=\"2\"><mrow><mtable rowspacing=\"0.5ex\">", (yyvsp[(3) - (5)]), "</mtable></mrow></mstyle>");
  itex2MML_free_string((yyvsp[(3) - (5)]));
}
    break;

  case 260:

/* Line 1806 of yacc.c  */
#line 1489 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mrow><mo>{</mo><mrow><mtable columnalign=\"left left\">", (yyvsp[(3) - (5)]), "</mtable></mrow></mrow>");
  itex2MML_free_string((yyvsp[(3) - (5)]));
}
    break;

  case 261:

/* Line 1806 of yacc.c  */
#line 1493 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mrow><mtable columnalign=\"right left right left right left right left right left\" columnspacing=\"0em\">", (yyvsp[(3) - (5)]), "</mtable></mrow>");
  itex2MML_free_string((yyvsp[(3) - (5)]));
}
    break;

  case 262:

/* Line 1806 of yacc.c  */
#line 1497 "itex2MML.y"
    {
  char * s1 = itex2MML_copy3("<mtable rowspacing=\"0.5ex\" align=\"", (yyvsp[(3) - (9)]), "\" columnalign=\"");
  char * s2 = itex2MML_copy3(s1, (yyvsp[(5) - (9)]), "\">");
  (yyval) = itex2MML_copy3(s2, (yyvsp[(7) - (9)]), "</mtable>");
  itex2MML_free_string(s1);
  itex2MML_free_string(s2);
  itex2MML_free_string((yyvsp[(3) - (9)]));
  itex2MML_free_string((yyvsp[(5) - (9)]));
  itex2MML_free_string((yyvsp[(7) - (9)]));
}
    break;

  case 263:

/* Line 1806 of yacc.c  */
#line 1507 "itex2MML.y"
    {
  char * s1 = itex2MML_copy3("<mtable rowspacing=\"0.5ex\" columnalign=\"", (yyvsp[(4) - (8)]), "\">");
  (yyval) = itex2MML_copy3(s1, (yyvsp[(6) - (8)]), "</mtable>");
  itex2MML_free_string(s1);
  itex2MML_free_string((yyvsp[(4) - (8)]));
  itex2MML_free_string((yyvsp[(6) - (8)]));
}
    break;

  case 264:

/* Line 1806 of yacc.c  */
#line 1514 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<semantics><annotation-xml encoding=\"SVG1.1\">", (yyvsp[(3) - (4)]), "</annotation-xml></semantics>");
  itex2MML_free_string((yyvsp[(3) - (4)]));
}
    break;

  case 265:

/* Line 1806 of yacc.c  */
#line 1518 "itex2MML.y"
    {
  (yyval) = itex2MML_copy_string(" ");
}
    break;

  case 266:

/* Line 1806 of yacc.c  */
#line 1522 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3((yyvsp[(1) - (2)]), " ", (yyvsp[(2) - (2)]));
  itex2MML_free_string((yyvsp[(1) - (2)]));
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 267:

/* Line 1806 of yacc.c  */
#line 1527 "itex2MML.y"
    {
  (yyval) = itex2MML_copy_string((yyvsp[(1) - (1)]));
  itex2MML_free_string((yyvsp[(1) - (1)]));
}
    break;

  case 268:

/* Line 1806 of yacc.c  */
#line 1532 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mrow><mtable columnalign=\"center\" rowspacing=\"0.5ex\">", (yyvsp[(3) - (4)]), "</mtable></mrow>");
  itex2MML_free_string((yyvsp[(3) - (4)]));
}
    break;

  case 269:

/* Line 1806 of yacc.c  */
#line 1537 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mrow><mtable>", (yyvsp[(3) - (4)]), "</mtable></mrow>");
  itex2MML_free_string((yyvsp[(3) - (4)]));
}
    break;

  case 270:

/* Line 1806 of yacc.c  */
#line 1541 "itex2MML.y"
    {
  char * s1 = itex2MML_copy3("<mrow><mtable ", (yyvsp[(5) - (8)]), ">");
  (yyval) = itex2MML_copy3(s1, (yyvsp[(7) - (8)]), "</mtable></mrow>");
  itex2MML_free_string(s1);
  itex2MML_free_string((yyvsp[(5) - (8)]));
  itex2MML_free_string((yyvsp[(7) - (8)]));
}
    break;

  case 271:

/* Line 1806 of yacc.c  */
#line 1549 "itex2MML.y"
    {
  (yyval) = itex2MML_copy_string((yyvsp[(1) - (1)]));
  itex2MML_free_string((yyvsp[(1) - (1)]));
}
    break;

  case 272:

/* Line 1806 of yacc.c  */
#line 1553 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3((yyvsp[(1) - (2)]), " ", (yyvsp[(2) - (2)]));
  itex2MML_free_string((yyvsp[(1) - (2)]));
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 273:

/* Line 1806 of yacc.c  */
#line 1559 "itex2MML.y"
    {
  (yyval) = itex2MML_copy_string((yyvsp[(1) - (1)]));
  itex2MML_free_string((yyvsp[(1) - (1)]));
}
    break;

  case 274:

/* Line 1806 of yacc.c  */
#line 1563 "itex2MML.y"
    {
  (yyval) = itex2MML_copy_string((yyvsp[(1) - (1)]));
  itex2MML_free_string((yyvsp[(1) - (1)]));
}
    break;

  case 275:

/* Line 1806 of yacc.c  */
#line 1567 "itex2MML.y"
    {
  (yyval) = itex2MML_copy_string((yyvsp[(1) - (1)]));
  itex2MML_free_string((yyvsp[(1) - (1)]));
}
    break;

  case 276:

/* Line 1806 of yacc.c  */
#line 1571 "itex2MML.y"
    {
  (yyval) = itex2MML_copy_string((yyvsp[(1) - (1)]));
  itex2MML_free_string((yyvsp[(1) - (1)]));
}
    break;

  case 277:

/* Line 1806 of yacc.c  */
#line 1575 "itex2MML.y"
    {
  (yyval) = itex2MML_copy_string((yyvsp[(1) - (1)]));
  itex2MML_free_string((yyvsp[(1) - (1)]));
}
    break;

  case 278:

/* Line 1806 of yacc.c  */
#line 1579 "itex2MML.y"
    {
  (yyval) = itex2MML_copy_string((yyvsp[(1) - (1)]));
  itex2MML_free_string((yyvsp[(1) - (1)]));
}
    break;

  case 279:

/* Line 1806 of yacc.c  */
#line 1583 "itex2MML.y"
    {
  (yyval) = itex2MML_copy_string((yyvsp[(1) - (1)]));
  itex2MML_free_string((yyvsp[(1) - (1)]));
}
    break;

  case 280:

/* Line 1806 of yacc.c  */
#line 1587 "itex2MML.y"
    {
  (yyval) = itex2MML_copy_string((yyvsp[(1) - (1)]));
  itex2MML_free_string((yyvsp[(1) - (1)]));
}
    break;

  case 281:

/* Line 1806 of yacc.c  */
#line 1591 "itex2MML.y"
    {
  (yyval) = itex2MML_copy_string((yyvsp[(1) - (1)]));
  itex2MML_free_string((yyvsp[(1) - (1)]));
}
    break;

  case 282:

/* Line 1806 of yacc.c  */
#line 1595 "itex2MML.y"
    {
  (yyval) = itex2MML_copy_string((yyvsp[(1) - (1)]));
  itex2MML_free_string((yyvsp[(1) - (1)]));
}
    break;

  case 283:

/* Line 1806 of yacc.c  */
#line 1600 "itex2MML.y"
    {
  (yyval) = itex2MML_copy2("columnalign=", (yyvsp[(2) - (2)]));
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 284:

/* Line 1806 of yacc.c  */
#line 1605 "itex2MML.y"
    {
  (yyval) = itex2MML_copy2("columnalign=", (yyvsp[(2) - (2)]));
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 285:

/* Line 1806 of yacc.c  */
#line 1610 "itex2MML.y"
    {
  (yyval) = itex2MML_copy2("rowalign=", (yyvsp[(2) - (2)]));
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 286:

/* Line 1806 of yacc.c  */
#line 1615 "itex2MML.y"
    {
  (yyval) = itex2MML_copy2("align=", (yyvsp[(2) - (2)]));
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 287:

/* Line 1806 of yacc.c  */
#line 1620 "itex2MML.y"
    {
  (yyval) = itex2MML_copy2("equalrows=", (yyvsp[(2) - (2)]));
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 288:

/* Line 1806 of yacc.c  */
#line 1625 "itex2MML.y"
    {
  (yyval) = itex2MML_copy2("equalcolumns=", (yyvsp[(2) - (2)]));
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 289:

/* Line 1806 of yacc.c  */
#line 1630 "itex2MML.y"
    {
  (yyval) = itex2MML_copy2("rowlines=", (yyvsp[(2) - (2)]));
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 290:

/* Line 1806 of yacc.c  */
#line 1635 "itex2MML.y"
    {
  (yyval) = itex2MML_copy2("columnlines=", (yyvsp[(2) - (2)]));
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 291:

/* Line 1806 of yacc.c  */
#line 1640 "itex2MML.y"
    {
  (yyval) = itex2MML_copy2("frame=", (yyvsp[(2) - (2)]));
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 292:

/* Line 1806 of yacc.c  */
#line 1645 "itex2MML.y"
    {
  char * s1 = itex2MML_copy3("rowspacing=", (yyvsp[(2) - (2)]), " columnspacing=");
  (yyval) = itex2MML_copy2(s1, (yyvsp[(2) - (2)]));
  itex2MML_free_string(s1);
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 293:

/* Line 1806 of yacc.c  */
#line 1652 "itex2MML.y"
    {
  (yyval) = itex2MML_copy_string((yyvsp[(1) - (1)]));
  itex2MML_free_string((yyvsp[(1) - (1)]));
}
    break;

  case 294:

/* Line 1806 of yacc.c  */
#line 1656 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3((yyvsp[(1) - (3)]), " ", (yyvsp[(3) - (3)]));
  itex2MML_free_string((yyvsp[(1) - (3)]));
  itex2MML_free_string((yyvsp[(3) - (3)]));
}
    break;

  case 295:

/* Line 1806 of yacc.c  */
#line 1662 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mtr>", (yyvsp[(1) - (1)]), "</mtr>");
  itex2MML_free_string((yyvsp[(1) - (1)]));
}
    break;

  case 296:

/* Line 1806 of yacc.c  */
#line 1666 "itex2MML.y"
    {
  (yyval) = itex2MML_copy_string((yyvsp[(1) - (1)]));
  itex2MML_free_string((yyvsp[(1) - (1)]));
}
    break;

  case 297:

/* Line 1806 of yacc.c  */
#line 1671 "itex2MML.y"
    {
  (yyval) = itex2MML_copy_string((yyvsp[(1) - (1)]));
  itex2MML_free_string((yyvsp[(1) - (1)]));
}
    break;

  case 298:

/* Line 1806 of yacc.c  */
#line 1675 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3((yyvsp[(1) - (3)]), " ", (yyvsp[(3) - (3)]));
  itex2MML_free_string((yyvsp[(1) - (3)]));
  itex2MML_free_string((yyvsp[(3) - (3)]));
}
    break;

  case 299:

/* Line 1806 of yacc.c  */
#line 1681 "itex2MML.y"
    {
  char * s1 = itex2MML_copy3("<mtr ", (yyvsp[(3) - (5)]), ">");
  (yyval) = itex2MML_copy3(s1, (yyvsp[(5) - (5)]), "</mtr>");
  itex2MML_free_string(s1);
  itex2MML_free_string((yyvsp[(3) - (5)]));
  itex2MML_free_string((yyvsp[(5) - (5)]));
}
    break;

  case 300:

/* Line 1806 of yacc.c  */
#line 1689 "itex2MML.y"
    {
  (yyval) = itex2MML_copy_string((yyvsp[(1) - (1)]));
  itex2MML_free_string((yyvsp[(1) - (1)]));
}
    break;

  case 301:

/* Line 1806 of yacc.c  */
#line 1693 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3((yyvsp[(1) - (2)]), " ", (yyvsp[(2) - (2)]));
  itex2MML_free_string((yyvsp[(1) - (2)]));
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 302:

/* Line 1806 of yacc.c  */
#line 1699 "itex2MML.y"
    {
  (yyval) = itex2MML_copy_string((yyvsp[(1) - (1)]));
  itex2MML_free_string((yyvsp[(1) - (1)]));
}
    break;

  case 303:

/* Line 1806 of yacc.c  */
#line 1703 "itex2MML.y"
    {
  (yyval) = itex2MML_copy_string((yyvsp[(1) - (1)]));
  itex2MML_free_string((yyvsp[(1) - (1)]));
}
    break;

  case 304:

/* Line 1806 of yacc.c  */
#line 1708 "itex2MML.y"
    {
  (yyval) = itex2MML_copy_string("<mtd></mtd>");
}
    break;

  case 305:

/* Line 1806 of yacc.c  */
#line 1711 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3("<mtd>", (yyvsp[(1) - (1)]), "</mtd>");
  itex2MML_free_string((yyvsp[(1) - (1)]));
}
    break;

  case 306:

/* Line 1806 of yacc.c  */
#line 1715 "itex2MML.y"
    {
  char * s1 = itex2MML_copy3("<mtd ", (yyvsp[(3) - (5)]), ">");
  (yyval) = itex2MML_copy3(s1, (yyvsp[(5) - (5)]), "</mtd>");
  itex2MML_free_string(s1);
  itex2MML_free_string((yyvsp[(3) - (5)]));
  itex2MML_free_string((yyvsp[(5) - (5)]));
}
    break;

  case 307:

/* Line 1806 of yacc.c  */
#line 1723 "itex2MML.y"
    {
  (yyval) = itex2MML_copy_string((yyvsp[(1) - (1)]));
  itex2MML_free_string((yyvsp[(1) - (1)]));
}
    break;

  case 308:

/* Line 1806 of yacc.c  */
#line 1727 "itex2MML.y"
    {
  (yyval) = itex2MML_copy3((yyvsp[(1) - (2)]), " ", (yyvsp[(2) - (2)]));
  itex2MML_free_string((yyvsp[(1) - (2)]));
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 309:

/* Line 1806 of yacc.c  */
#line 1733 "itex2MML.y"
    {
  (yyval) = itex2MML_copy_string((yyvsp[(1) - (1)]));
  itex2MML_free_string((yyvsp[(1) - (1)]));
}
    break;

  case 310:

/* Line 1806 of yacc.c  */
#line 1737 "itex2MML.y"
    {
  (yyval) = itex2MML_copy_string((yyvsp[(1) - (1)]));
  itex2MML_free_string((yyvsp[(1) - (1)]));
}
    break;

  case 311:

/* Line 1806 of yacc.c  */
#line 1741 "itex2MML.y"
    {
  (yyval) = itex2MML_copy_string((yyvsp[(1) - (1)]));
  itex2MML_free_string((yyvsp[(1) - (1)]));
}
    break;

  case 312:

/* Line 1806 of yacc.c  */
#line 1745 "itex2MML.y"
    {
  (yyval) = itex2MML_copy_string((yyvsp[(1) - (1)]));
  itex2MML_free_string((yyvsp[(1) - (1)]));
}
    break;

  case 313:

/* Line 1806 of yacc.c  */
#line 1750 "itex2MML.y"
    {
  (yyval) = itex2MML_copy2("rowspan=", (yyvsp[(2) - (2)]));
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;

  case 314:

/* Line 1806 of yacc.c  */
#line 1755 "itex2MML.y"
    {
  (yyval) = itex2MML_copy2("columnspan=", (yyvsp[(2) - (2)]));
  itex2MML_free_string((yyvsp[(2) - (2)]));
}
    break;



/* Line 1806 of yacc.c  */
#line 6091 "y.tab.c"
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



/* Line 2067 of yacc.c  */
#line 1760 "itex2MML.y"


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
  return itex2MML_do_html_filter (buffer, length, 0);
}

int itex2MML_strict_html_filter (const char * buffer, unsigned long length)
{
  return itex2MML_do_html_filter (buffer, length, 1);
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

