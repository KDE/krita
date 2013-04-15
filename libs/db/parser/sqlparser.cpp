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



/* Copy the first part of user declarations.  */

/* Line 268 of yacc.c  */
#line 436 "sqlparser.y"

#ifndef YYDEBUG /* compat. */
# define YYDEBUG 0
#endif
#include <stdio.h>
#include <string.h>
#include <string>
#include <iostream>
#include <assert.h>
#include <limits.h>
//TODO OK?
#ifdef Q_WS_WIN
//workaround for bug on msvc
# undef LLONG_MIN
#endif
#ifndef LLONG_MAX
# define LLONG_MAX     0x7fffffffffffffffLL
#endif
#ifndef LLONG_MIN
# define LLONG_MIN     0x8000000000000000LL
#endif
#ifndef LLONG_MAX
# define ULLONG_MAX    0xffffffffffffffffLL
#endif

#ifdef _WIN32
# include <malloc.h>
#endif

#include <QObject>
#include <QList>
#include <QVariant>

#include <kdebug.h>
#include <klocale.h>

#include <db/connection.h>
#include <db/queryschema.h>
#include <db/field.h>
#include <db/tableschema.h>

#include "parser.h"
#include "parser_p.h"
#include "sqltypes.h"
#ifdef Q_OS_SOLARIS
#include <alloca.h>
#endif

int yylex();

//	using namespace std;
using namespace KexiDB;

#define YY_NO_UNPUT
#define YYSTACK_USE_ALLOCA 1
#define YYMAXDEPTH 255

	extern "C"
	{
		int yywrap()
		{
			return 1;
		}
	}

#if 0
	struct yyval
	{
		QString parserUserName;
		int integerValue;
		KexiDBField::ColumnType coltype;
	}
#endif



/* Line 268 of yacc.c  */
#line 148 "sqlparser.cpp"

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
     SQL_TYPE = 258,
     AS = 259,
     ASC = 260,
     AUTO_INCREMENT = 261,
     BIT = 262,
     BITWISE_SHIFT_LEFT = 263,
     BITWISE_SHIFT_RIGHT = 264,
     BY = 265,
     CHARACTER_STRING_LITERAL = 266,
     CONCATENATION = 267,
     CREATE = 268,
     DESC = 269,
     DISTINCT = 270,
     DOUBLE_QUOTED_STRING = 271,
     FROM = 272,
     JOIN = 273,
     KEY = 274,
     LEFT = 275,
     LESS_OR_EQUAL = 276,
     SQL_NULL = 277,
     SQL_IS = 278,
     SQL_IS_NULL = 279,
     SQL_IS_NOT_NULL = 280,
     ORDER = 281,
     PRIMARY = 282,
     SELECT = 283,
     INTEGER_CONST = 284,
     REAL_CONST = 285,
     RIGHT = 286,
     SQL_ON = 287,
     DATE_CONST = 288,
     DATETIME_CONST = 289,
     TIME_CONST = 290,
     TABLE = 291,
     IDENTIFIER = 292,
     IDENTIFIER_DOT_ASTERISK = 293,
     QUERY_PARAMETER = 294,
     VARCHAR = 295,
     WHERE = 296,
     SCAN_ERROR = 297,
     __LAST_TOKEN = 298,
     EXCEPT = 299,
     UNION = 300,
     INTERSECT = 301,
     OR = 302,
     XOR = 303,
     AND = 304,
     NOT = 305,
     GREATER_OR_EQUAL = 306,
     NOT_EQUAL2 = 307,
     NOT_EQUAL = 308,
     NOT_SIMILAR_TO = 309,
     SIMILAR_TO = 310,
     ILIKE = 311,
     LIKE = 312,
     SQL_IN = 313,
     BETWEEN = 314,
     UMINUS = 315
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 293 of yacc.c  */
#line 512 "sqlparser.y"

	QString* stringValue;
	qint64 integerValue;
	bool booleanValue;
	struct realType realValue;
	KexiDB::Field::Type colType;
	KexiDB::Field *field;
	KexiDB::BaseExpr *expr;
	KexiDB::NArgExpr *exprList;
	KexiDB::ConstExpr *constExpr;
	KexiDB::QuerySchema *querySchema;
	SelectOptionsInternal *selectOptions;
	OrderByColumnInternal::List *orderByColumns;
	QVariant *variantValue;



/* Line 293 of yacc.c  */
#line 262 "sqlparser.cpp"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 343 of yacc.c  */
#line 274 "sqlparser.cpp"

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
#define YYFINAL  10
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   181

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  84
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  37
/* YYNRULES -- Number of rules.  */
#define YYNRULES  109
/* YYNRULES -- Number of states.  */
#define YYNSTATES  178

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   315

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,    52,    47,    81,    56,
      53,    54,    46,    45,    50,    44,    51,    57,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    49,
      66,    65,    67,    55,    48,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    79,     2,    80,    77,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    82,     2,    83,     2,     2,     2,
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
      35,    36,    37,    38,    39,    40,    41,    42,    43,    58,
      59,    60,    61,    62,    63,    64,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    78
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     9,    11,    14,    16,    18,    19,
      27,    31,    33,    36,    40,    43,    45,    48,    51,    53,
      55,    60,    65,    66,    69,    73,    76,    80,    85,    87,
      89,    93,    98,   103,   106,   108,   111,   115,   120,   122,
     126,   128,   130,   132,   134,   138,   142,   146,   148,   152,
     156,   160,   164,   168,   170,   174,   178,   182,   186,   190,
     194,   196,   199,   202,   204,   208,   212,   214,   218,   222,
     226,   230,   234,   236,   240,   244,   248,   250,   253,   256,
     259,   262,   264,   266,   269,   273,   275,   277,   279,   281,
     283,   287,   291,   295,   299,   302,   306,   308,   310,   313,
     317,   321,   323,   325,   327,   331,   334,   336,   341,   343
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      85,     0,    -1,    86,    -1,    87,    49,    86,    -1,    87,
      -1,    87,    49,    -1,    88,    -1,    95,    -1,    -1,    13,
      36,    37,    89,    53,    90,    54,    -1,    90,    50,    91,
      -1,    91,    -1,    37,    94,    -1,    37,    94,    92,    -1,
      92,    93,    -1,    93,    -1,    27,    19,    -1,    64,    22,
      -1,     6,    -1,     3,    -1,     3,    53,    29,    54,    -1,
      40,    53,    29,    54,    -1,    -1,    96,   117,    -1,    96,
     117,   114,    -1,    96,   114,    -1,    96,   117,    97,    -1,
      96,   117,   114,    97,    -1,    28,    -1,    98,    -1,    26,
      10,    99,    -1,    98,    26,    10,    99,    -1,    26,    10,
      99,    98,    -1,    41,   102,    -1,   100,    -1,   100,   101,
      -1,   100,    50,    99,    -1,   100,   101,    50,    99,    -1,
      37,    -1,    37,    51,    37,    -1,    29,    -1,     5,    -1,
      14,    -1,   103,    -1,   104,    63,   103,    -1,   104,    61,
     103,    -1,   104,    62,   103,    -1,   104,    -1,   105,    67,
     104,    -1,   105,    68,   104,    -1,   105,    66,   104,    -1,
     105,    21,   104,    -1,   105,    65,   104,    -1,   105,    -1,
     106,    70,   105,    -1,   106,    69,   105,    -1,   106,    74,
     105,    -1,   106,    75,   105,    -1,   106,    72,   105,    -1,
     106,    71,   105,    -1,   106,    -1,   106,    24,    -1,   106,
      25,    -1,   107,    -1,   108,     8,   107,    -1,   108,     9,
     107,    -1,   108,    -1,   109,    45,   108,    -1,   109,    12,
     108,    -1,   109,    44,   108,    -1,   109,    81,   108,    -1,
     109,    82,   108,    -1,   109,    -1,   110,    57,   109,    -1,
     110,    46,   109,    -1,   110,    47,   109,    -1,   110,    -1,
      44,   110,    -1,    45,   110,    -1,    83,   110,    -1,    64,
     110,    -1,    37,    -1,    39,    -1,    37,   112,    -1,    37,
      51,    37,    -1,    22,    -1,    11,    -1,    29,    -1,    30,
      -1,   111,    -1,    53,   102,    54,    -1,    53,   113,    54,
      -1,   102,    50,   113,    -1,   102,    50,   102,    -1,    17,
     115,    -1,   115,    50,   116,    -1,   116,    -1,    37,    -1,
      37,    37,    -1,    37,     4,    37,    -1,   117,    50,   118,
      -1,   118,    -1,   119,    -1,   120,    -1,   119,     4,    37,
      -1,   119,    37,    -1,   102,    -1,    15,    53,   119,    54,
      -1,    46,    -1,    37,    51,    46,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   581,   581,   591,   595,   596,   606,   610,   618,   617,
     627,   627,   633,   641,   657,   657,   663,   668,   673,   681,
     686,   693,   700,   708,   715,   720,   726,   732,   741,   751,
     757,   763,   770,   780,   789,   798,   808,   816,   828,   834,
     841,   848,   852,   859,   864,   869,   873,   878,   883,   887,
     891,   895,   899,   904,   909,   914,   918,   922,   926,   930,
     935,   940,   944,   949,   954,   958,   963,   968,   973,   977,
     981,   985,   990,   995,   999,  1003,  1008,  1014,  1018,  1022,
    1026,  1030,  1038,  1044,  1051,  1058,  1065,  1071,  1088,  1094,
    1099,  1107,  1117,  1122,  1131,  1176,  1181,  1189,  1217,  1228,
    1244,  1250,  1259,  1268,  1273,  1282,  1294,  1338,  1347,  1356
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "SQL_TYPE", "AS", "ASC",
  "AUTO_INCREMENT", "BIT", "BITWISE_SHIFT_LEFT", "BITWISE_SHIFT_RIGHT",
  "BY", "CHARACTER_STRING_LITERAL", "CONCATENATION", "CREATE", "DESC",
  "DISTINCT", "DOUBLE_QUOTED_STRING", "FROM", "JOIN", "KEY", "LEFT",
  "LESS_OR_EQUAL", "SQL_NULL", "SQL_IS", "SQL_IS_NULL", "SQL_IS_NOT_NULL",
  "ORDER", "PRIMARY", "SELECT", "INTEGER_CONST", "REAL_CONST", "RIGHT",
  "SQL_ON", "DATE_CONST", "DATETIME_CONST", "TIME_CONST", "TABLE",
  "IDENTIFIER", "IDENTIFIER_DOT_ASTERISK", "QUERY_PARAMETER", "VARCHAR",
  "WHERE", "SCAN_ERROR", "__LAST_TOKEN", "'-'", "'+'", "'*'", "'%'", "'@'",
  "';'", "','", "'.'", "'$'", "'('", "')'", "'?'", "'\\''", "'/'",
  "EXCEPT", "UNION", "INTERSECT", "OR", "XOR", "AND", "NOT", "'='", "'<'",
  "'>'", "GREATER_OR_EQUAL", "NOT_EQUAL2", "NOT_EQUAL", "NOT_SIMILAR_TO",
  "SIMILAR_TO", "ILIKE", "LIKE", "SQL_IN", "BETWEEN", "'^'", "UMINUS",
  "'['", "']'", "'&'", "'|'", "'~'", "$accept", "TopLevelStatement",
  "StatementList", "Statement", "CreateTableStatement", "$@1", "ColDefs",
  "ColDef", "ColKeys", "ColKey", "ColType", "SelectStatement", "Select",
  "SelectOptions", "WhereClause", "OrderByClause", "OrderByColumnId",
  "OrderByOption", "aExpr", "aExpr2", "aExpr3", "aExpr4", "aExpr5",
  "aExpr6", "aExpr7", "aExpr8", "aExpr9", "aExpr10", "aExprList",
  "aExprList2", "Tables", "FlatTableList", "FlatTable", "ColViews",
  "ColItem", "ColExpression", "ColWildCard", 0
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
     295,   296,   297,   298,    45,    43,    42,    37,    64,    59,
      44,    46,    36,    40,    41,    63,    39,    47,   299,   300,
     301,   302,   303,   304,   305,    61,    60,    62,   306,   307,
     308,   309,   310,   311,   312,   313,   314,    94,   315,    91,
      93,    38,   124,   126
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    84,    85,    86,    86,    86,    87,    87,    89,    88,
      90,    90,    91,    91,    92,    92,    93,    93,    93,    94,
      94,    94,    94,    95,    95,    95,    95,    95,    96,    97,
      97,    97,    97,    98,    99,    99,    99,    99,   100,   100,
     100,   101,   101,   102,   103,   103,   103,   103,   104,   104,
     104,   104,   104,   104,   105,   105,   105,   105,   105,   105,
     105,   106,   106,   106,   107,   107,   107,   108,   108,   108,
     108,   108,   108,   109,   109,   109,   109,   110,   110,   110,
     110,   110,   110,   110,   110,   110,   110,   110,   110,   110,
     111,   112,   113,   113,   114,   115,   115,   116,   116,   116,
     117,   117,   118,   118,   118,   118,   119,   119,   120,   120
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     3,     1,     2,     1,     1,     0,     7,
       3,     1,     2,     3,     2,     1,     2,     2,     1,     1,
       4,     4,     0,     2,     3,     2,     3,     4,     1,     1,
       3,     4,     4,     2,     1,     2,     3,     4,     1,     3,
       1,     1,     1,     1,     3,     3,     3,     1,     3,     3,
       3,     3,     3,     1,     3,     3,     3,     3,     3,     3,
       1,     2,     2,     1,     3,     3,     1,     3,     3,     3,
       3,     3,     1,     3,     3,     3,     1,     2,     2,     2,
       2,     1,     1,     2,     3,     1,     1,     1,     1,     1,
       3,     3,     3,     3,     2,     3,     1,     1,     2,     3,
       3,     1,     1,     1,     3,     2,     1,     4,     1,     3
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,    28,     0,     2,     4,     6,     7,     0,     0,
       1,     5,    86,     0,     0,    85,    87,    88,    81,    82,
       0,     0,   108,     0,     0,     0,   106,    43,    47,    53,
      60,    63,    66,    72,    76,    89,    25,    23,   101,   102,
     103,     8,     3,     0,    97,    94,    96,     0,     0,    83,
      81,    77,    78,     0,    80,    79,     0,     0,     0,     0,
       0,     0,     0,     0,    61,    62,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    26,    29,    24,     0,   105,
       0,     0,     0,    98,     0,    84,   109,     0,     0,     0,
      90,    45,    46,    44,    51,    52,    50,    48,    49,    55,
      54,    59,    58,    56,    57,    64,    65,    68,    69,    67,
      70,    71,    74,    75,    73,     0,    33,   100,     0,    27,
     104,     0,   107,    99,    95,     0,    91,    40,    38,    30,
      34,     0,    22,     0,    11,    93,    92,     0,    32,    41,
      42,     0,    35,    31,    19,     0,    12,     0,     9,    39,
      36,     0,     0,     0,    18,     0,     0,    13,    15,    10,
      37,     0,     0,    16,    17,    14,    20,    21
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     3,     4,     5,     6,    90,   143,   144,   167,   168,
     156,     7,     8,    85,    86,   139,   140,   152,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    49,    98,
      36,    45,    46,    37,    38,    39,    40
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -131
static const yytype_int16 yypact[] =
{
     -11,   -22,  -131,    52,  -131,    -4,  -131,  -131,   -10,    21,
    -131,   -11,  -131,    18,    29,  -131,  -131,  -131,    30,  -131,
      45,    45,  -131,    45,    45,    45,  -131,  -131,    41,    26,
      55,  -131,   140,     6,    49,  -131,  -131,    82,  -131,     5,
    -131,  -131,  -131,    33,    22,    50,  -131,    64,    45,  -131,
      54,  -131,  -131,    77,  -131,  -131,    45,    45,    45,    45,
      45,    45,    45,    45,  -131,  -131,    45,    45,    45,    45,
      45,    45,    45,    45,    45,    45,    45,    45,    45,    45,
      45,    45,   101,    45,    -7,  -131,   112,   -13,   107,  -131,
      99,   100,   116,  -131,    29,  -131,  -131,   105,   102,   120,
    -131,  -131,  -131,  -131,  -131,  -131,  -131,  -131,  -131,  -131,
    -131,  -131,  -131,  -131,  -131,  -131,  -131,  -131,  -131,  -131,
    -131,  -131,  -131,  -131,  -131,     4,  -131,  -131,   148,  -131,
    -131,   122,  -131,  -131,  -131,    45,  -131,  -131,   109,   121,
      35,     4,    13,   -44,  -131,   105,  -131,   124,  -131,  -131,
    -131,     4,   113,  -131,   111,   114,    -3,   122,  -131,  -131,
    -131,     4,   136,   137,  -131,   149,   147,    -3,  -131,  -131,
    -131,   117,   118,  -131,  -131,  -131,  -131,  -131
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -131,  -131,   159,  -131,  -131,  -131,  -131,    16,  -131,     7,
    -131,  -131,  -131,    88,    37,  -130,  -131,  -131,   -23,    57,
      74,    51,  -131,    78,    65,    66,    44,  -131,  -131,    42,
     141,  -131,    85,  -131,    96,   138,  -131
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint8 yytable[] =
{
      53,    12,     1,   164,    12,    13,   157,    14,    13,    88,
     158,   153,    15,    82,     9,    15,   154,     2,    74,    16,
      17,   160,    16,    17,   165,    97,    92,    18,    83,    19,
      18,   170,    19,   137,    20,    21,    22,    20,    21,    22,
     149,   138,    89,    23,    12,    11,    23,    59,    13,   150,
      75,    76,    10,   155,    24,    15,    12,    24,    41,    93,
     126,   166,    16,    17,    51,    52,    44,    15,    54,    55,
      50,    43,    19,    25,    16,    17,    25,    20,    21,    64,
      65,    47,    50,    48,    19,   151,    23,    77,    78,    20,
      21,    60,    61,    62,    63,    79,    80,    24,    23,    14,
      94,    95,    56,    57,    58,    99,    81,    48,    82,    24,
      96,   125,   145,   101,   102,   103,    25,   109,   110,   111,
     112,   113,   114,    83,    66,    67,    68,    69,    25,    70,
      71,   100,    84,   104,   105,   106,   107,   108,   128,   117,
     118,   119,   120,   121,   130,   122,   123,   124,    72,    73,
     115,   116,   131,   133,   132,   135,   136,    95,   141,   142,
     147,   159,    83,   161,   162,   171,   172,   163,   173,   174,
      42,   176,   177,   169,   175,   129,   148,   146,    87,   134,
     127,    91
};

#define yypact_value_is_default(yystate) \
  ((yystate) == (-131))

#define yytable_value_is_error(yytable_value) \
  YYID (0)

static const yytype_uint8 yycheck[] =
{
      23,    11,    13,     6,    11,    15,    50,    17,    15,     4,
      54,   141,    22,    26,    36,    22,     3,    28,    12,    29,
      30,   151,    29,    30,    27,    48,     4,    37,    41,    39,
      37,   161,    39,    29,    44,    45,    46,    44,    45,    46,
       5,    37,    37,    53,    11,    49,    53,    21,    15,    14,
      44,    45,     0,    40,    64,    22,    11,    64,    37,    37,
      83,    64,    29,    30,    20,    21,    37,    22,    24,    25,
      37,    53,    39,    83,    29,    30,    83,    44,    45,    24,
      25,    51,    37,    53,    39,    50,    53,    81,    82,    44,
      45,    65,    66,    67,    68,    46,    47,    64,    53,    17,
      50,    37,    61,    62,    63,    51,    57,    53,    26,    64,
      46,    10,   135,    56,    57,    58,    83,    66,    67,    68,
      69,    70,    71,    41,    69,    70,    71,    72,    83,    74,
      75,    54,    50,    59,    60,    61,    62,    63,    26,    74,
      75,    76,    77,    78,    37,    79,    80,    81,     8,     9,
      72,    73,    53,    37,    54,    50,    54,    37,    10,    37,
      51,    37,    41,    50,    53,    29,    29,    53,    19,    22,
      11,    54,    54,   157,   167,    87,   139,   135,    37,    94,
      84,    43
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    13,    28,    85,    86,    87,    88,    95,    96,    36,
       0,    49,    11,    15,    17,    22,    29,    30,    37,    39,
      44,    45,    46,    53,    64,    83,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   114,   117,   118,   119,
     120,    37,    86,    53,    37,   115,   116,    51,    53,   112,
      37,   110,   110,   102,   110,   110,    61,    62,    63,    21,
      65,    66,    67,    68,    24,    25,    69,    70,    71,    72,
      74,    75,     8,     9,    12,    44,    45,    81,    82,    46,
      47,    57,    26,    41,    50,    97,    98,   114,     4,    37,
      89,   119,     4,    37,    50,    37,    46,   102,   113,    51,
      54,   103,   103,   103,   104,   104,   104,   104,   104,   105,
     105,   105,   105,   105,   105,   107,   107,   108,   108,   108,
     108,   108,   109,   109,   109,    10,   102,   118,    26,    97,
      37,    53,    54,    37,   116,    50,    54,    29,    37,    99,
     100,    10,    37,    90,    91,   102,   113,    51,    98,     5,
      14,    50,   101,    99,     3,    40,    94,    50,    54,    37,
      99,    50,    53,    53,     6,    27,    64,    92,    93,    91,
      99,    29,    29,    19,    22,    93,    54,    54
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
#line 582 "sqlparser.y"
    {
//todo: multiple statements
//todo: not only "select" statements
	parser->setOperation(Parser::OP_Select);
	parser->setQuerySchema((yyvsp[(1) - (1)].querySchema));
}
    break;

  case 3:

/* Line 1806 of yacc.c  */
#line 592 "sqlparser.y"
    {
//todo: multiple statements
}
    break;

  case 5:

/* Line 1806 of yacc.c  */
#line 597 "sqlparser.y"
    {
	(yyval.querySchema) = (yyvsp[(1) - (2)].querySchema);
}
    break;

  case 6:

/* Line 1806 of yacc.c  */
#line 607 "sqlparser.y"
    {
YYACCEPT;
}
    break;

  case 7:

/* Line 1806 of yacc.c  */
#line 611 "sqlparser.y"
    {
	(yyval.querySchema) = (yyvsp[(1) - (1)].querySchema);
}
    break;

  case 8:

/* Line 1806 of yacc.c  */
#line 618 "sqlparser.y"
    {
	parser->setOperation(Parser::OP_CreateTable);
	parser->createTable((yyvsp[(3) - (3)].stringValue)->toLatin1());
	delete (yyvsp[(3) - (3)].stringValue);
}
    break;

  case 11:

/* Line 1806 of yacc.c  */
#line 628 "sqlparser.y"
    {
}
    break;

  case 12:

/* Line 1806 of yacc.c  */
#line 634 "sqlparser.y"
    {
	KexiDBDbg << "adding field " << *(yyvsp[(1) - (2)].stringValue);
	field->setName((yyvsp[(1) - (2)].stringValue)->toLatin1());
	parser->table()->addField(field);
	field = 0;
	delete (yyvsp[(1) - (2)].stringValue);
}
    break;

  case 13:

/* Line 1806 of yacc.c  */
#line 642 "sqlparser.y"
    {
	KexiDBDbg << "adding field " << *(yyvsp[(1) - (3)].stringValue);
	field->setName(*(yyvsp[(1) - (3)].stringValue));
	delete (yyvsp[(1) - (3)].stringValue);
	parser->table()->addField(field);

//	if(field->isPrimaryKey())
//		parser->table()->addPrimaryKey(field->name());

//	delete field;
//	field = 0;
}
    break;

  case 15:

/* Line 1806 of yacc.c  */
#line 658 "sqlparser.y"
    {
}
    break;

  case 16:

/* Line 1806 of yacc.c  */
#line 664 "sqlparser.y"
    {
	field->setPrimaryKey(true);
	KexiDBDbg << "primary";
}
    break;

  case 17:

/* Line 1806 of yacc.c  */
#line 669 "sqlparser.y"
    {
	field->setNotNull(true);
	KexiDBDbg << "not_null";
}
    break;

  case 18:

/* Line 1806 of yacc.c  */
#line 674 "sqlparser.y"
    {
	field->setAutoIncrement(true);
	KexiDBDbg << "ainc";
}
    break;

  case 19:

/* Line 1806 of yacc.c  */
#line 682 "sqlparser.y"
    {
	field = new Field();
	field->setType((yyvsp[(1) - (1)].colType));
}
    break;

  case 20:

/* Line 1806 of yacc.c  */
#line 687 "sqlparser.y"
    {
	KexiDBDbg << "sql + length";
	field = new Field();
	field->setPrecision((yyvsp[(3) - (4)].integerValue));
	field->setType((yyvsp[(1) - (4)].colType));
}
    break;

  case 21:

/* Line 1806 of yacc.c  */
#line 694 "sqlparser.y"
    {
	field = new Field();
	field->setPrecision((yyvsp[(3) - (4)].integerValue));
	field->setType(Field::Text);
}
    break;

  case 22:

/* Line 1806 of yacc.c  */
#line 700 "sqlparser.y"
    {
	// SQLITE compatibillity
	field = new Field();
	field->setType(Field::InvalidType);
}
    break;

  case 23:

/* Line 1806 of yacc.c  */
#line 709 "sqlparser.y"
    {
	KexiDBDbg << "Select ColViews=" << (yyvsp[(2) - (2)].exprList)->debugString();

	if (!((yyval.querySchema) = buildSelectQuery( (yyvsp[(1) - (2)].querySchema), (yyvsp[(2) - (2)].exprList) )))
		return 0;
}
    break;

  case 24:

/* Line 1806 of yacc.c  */
#line 716 "sqlparser.y"
    {
	if (!((yyval.querySchema) = buildSelectQuery( (yyvsp[(1) - (3)].querySchema), (yyvsp[(2) - (3)].exprList), (yyvsp[(3) - (3)].exprList) )))
		return 0;
}
    break;

  case 25:

/* Line 1806 of yacc.c  */
#line 721 "sqlparser.y"
    {
	KexiDBDbg << "Select ColViews Tables";
	if (!((yyval.querySchema) = buildSelectQuery( (yyvsp[(1) - (2)].querySchema), 0, (yyvsp[(2) - (2)].exprList) )))
		return 0;
}
    break;

  case 26:

/* Line 1806 of yacc.c  */
#line 727 "sqlparser.y"
    {
	KexiDBDbg << "Select ColViews Conditions";
	if (!((yyval.querySchema) = buildSelectQuery( (yyvsp[(1) - (3)].querySchema), (yyvsp[(2) - (3)].exprList), 0, (yyvsp[(3) - (3)].selectOptions) )))
		return 0;
}
    break;

  case 27:

/* Line 1806 of yacc.c  */
#line 733 "sqlparser.y"
    {
	KexiDBDbg << "Select ColViews Tables SelectOptions";
	if (!((yyval.querySchema) = buildSelectQuery( (yyvsp[(1) - (4)].querySchema), (yyvsp[(2) - (4)].exprList), (yyvsp[(3) - (4)].exprList), (yyvsp[(4) - (4)].selectOptions) )))
		return 0;
}
    break;

  case 28:

/* Line 1806 of yacc.c  */
#line 742 "sqlparser.y"
    {
	KexiDBDbg << "SELECT";
//	parser->createSelect();
//	parser->setOperation(Parser::OP_Select);
	(yyval.querySchema) = new QuerySchema();
}
    break;

  case 29:

/* Line 1806 of yacc.c  */
#line 752 "sqlparser.y"
    {
	KexiDBDbg << "WhereClause";
	(yyval.selectOptions) = new SelectOptionsInternal;
	(yyval.selectOptions)->whereExpr = (yyvsp[(1) - (1)].expr);
}
    break;

  case 30:

/* Line 1806 of yacc.c  */
#line 758 "sqlparser.y"
    {
	KexiDBDbg << "OrderByClause";
	(yyval.selectOptions) = new SelectOptionsInternal;
	(yyval.selectOptions)->orderByColumns = (yyvsp[(3) - (3)].orderByColumns);
}
    break;

  case 31:

/* Line 1806 of yacc.c  */
#line 764 "sqlparser.y"
    {
	KexiDBDbg << "WhereClause ORDER BY OrderByClause";
	(yyval.selectOptions) = new SelectOptionsInternal;
	(yyval.selectOptions)->whereExpr = (yyvsp[(1) - (4)].expr);
	(yyval.selectOptions)->orderByColumns = (yyvsp[(4) - (4)].orderByColumns);
}
    break;

  case 32:

/* Line 1806 of yacc.c  */
#line 771 "sqlparser.y"
    {
	KexiDBDbg << "OrderByClause WhereClause";
	(yyval.selectOptions) = new SelectOptionsInternal;
	(yyval.selectOptions)->whereExpr = (yyvsp[(4) - (4)].expr);
	(yyval.selectOptions)->orderByColumns = (yyvsp[(3) - (4)].orderByColumns);
}
    break;

  case 33:

/* Line 1806 of yacc.c  */
#line 781 "sqlparser.y"
    {
	(yyval.expr) = (yyvsp[(2) - (2)].expr);
}
    break;

  case 34:

/* Line 1806 of yacc.c  */
#line 790 "sqlparser.y"
    {
	KexiDBDbg << "ORDER BY IDENTIFIER";
	(yyval.orderByColumns) = new OrderByColumnInternal::List;
	OrderByColumnInternal orderByColumn;
	orderByColumn.setColumnByNameOrNumber( *(yyvsp[(1) - (1)].variantValue) );
	(yyval.orderByColumns)->append( orderByColumn );
	delete (yyvsp[(1) - (1)].variantValue);
}
    break;

  case 35:

/* Line 1806 of yacc.c  */
#line 799 "sqlparser.y"
    {
	KexiDBDbg << "ORDER BY IDENTIFIER OrderByOption";
	(yyval.orderByColumns) = new OrderByColumnInternal::List;
	OrderByColumnInternal orderByColumn;
	orderByColumn.setColumnByNameOrNumber( *(yyvsp[(1) - (2)].variantValue) );
	orderByColumn.ascending = (yyvsp[(2) - (2)].booleanValue);
	(yyval.orderByColumns)->append( orderByColumn );
	delete (yyvsp[(1) - (2)].variantValue);
}
    break;

  case 36:

/* Line 1806 of yacc.c  */
#line 809 "sqlparser.y"
    {
	(yyval.orderByColumns) = (yyvsp[(3) - (3)].orderByColumns);
	OrderByColumnInternal orderByColumn;
	orderByColumn.setColumnByNameOrNumber( *(yyvsp[(1) - (3)].variantValue) );
	(yyval.orderByColumns)->append( orderByColumn );
	delete (yyvsp[(1) - (3)].variantValue);
}
    break;

  case 37:

/* Line 1806 of yacc.c  */
#line 817 "sqlparser.y"
    {
	(yyval.orderByColumns) = (yyvsp[(4) - (4)].orderByColumns);
	OrderByColumnInternal orderByColumn;
	orderByColumn.setColumnByNameOrNumber( *(yyvsp[(1) - (4)].variantValue) );
	orderByColumn.ascending = (yyvsp[(2) - (4)].booleanValue);
	(yyval.orderByColumns)->append( orderByColumn );
	delete (yyvsp[(1) - (4)].variantValue);
}
    break;

  case 38:

/* Line 1806 of yacc.c  */
#line 829 "sqlparser.y"
    {
	(yyval.variantValue) = new QVariant( *(yyvsp[(1) - (1)].stringValue) );
	KexiDBDbg << "OrderByColumnId: " << *(yyval.variantValue);
	delete (yyvsp[(1) - (1)].stringValue);
}
    break;

  case 39:

/* Line 1806 of yacc.c  */
#line 835 "sqlparser.y"
    {
	(yyval.variantValue) = new QVariant( *(yyvsp[(1) - (3)].stringValue) + "." + *(yyvsp[(3) - (3)].stringValue) );
	KexiDBDbg << "OrderByColumnId: " << *(yyval.variantValue);
	delete (yyvsp[(1) - (3)].stringValue);
	delete (yyvsp[(3) - (3)].stringValue);
}
    break;

  case 40:

/* Line 1806 of yacc.c  */
#line 842 "sqlparser.y"
    {
	(yyval.variantValue) = new QVariant((yyvsp[(1) - (1)].integerValue));
	KexiDBDbg << "OrderByColumnId: " << *(yyval.variantValue);
}
    break;

  case 41:

/* Line 1806 of yacc.c  */
#line 849 "sqlparser.y"
    {
	(yyval.booleanValue) = true;
}
    break;

  case 42:

/* Line 1806 of yacc.c  */
#line 853 "sqlparser.y"
    {
	(yyval.booleanValue) = false;
}
    break;

  case 44:

/* Line 1806 of yacc.c  */
#line 865 "sqlparser.y"
    {
//	KexiDBDbg << "AND " << $3.debugString();
	(yyval.expr) = new BinaryExpr( KexiDBExpr_Logical, (yyvsp[(1) - (3)].expr), AND, (yyvsp[(3) - (3)].expr) );
}
    break;

  case 45:

/* Line 1806 of yacc.c  */
#line 870 "sqlparser.y"
    {
	(yyval.expr) = new BinaryExpr( KexiDBExpr_Logical, (yyvsp[(1) - (3)].expr), OR, (yyvsp[(3) - (3)].expr) );
}
    break;

  case 46:

/* Line 1806 of yacc.c  */
#line 874 "sqlparser.y"
    {
	(yyval.expr) = new BinaryExpr( KexiDBExpr_Arithm, (yyvsp[(1) - (3)].expr), XOR, (yyvsp[(3) - (3)].expr) );
}
    break;

  case 48:

/* Line 1806 of yacc.c  */
#line 884 "sqlparser.y"
    {
	(yyval.expr) = new BinaryExpr(KexiDBExpr_Relational, (yyvsp[(1) - (3)].expr), '>', (yyvsp[(3) - (3)].expr));
}
    break;

  case 49:

/* Line 1806 of yacc.c  */
#line 888 "sqlparser.y"
    {
	(yyval.expr) = new BinaryExpr(KexiDBExpr_Relational, (yyvsp[(1) - (3)].expr), GREATER_OR_EQUAL, (yyvsp[(3) - (3)].expr));
}
    break;

  case 50:

/* Line 1806 of yacc.c  */
#line 892 "sqlparser.y"
    {
	(yyval.expr) = new BinaryExpr(KexiDBExpr_Relational, (yyvsp[(1) - (3)].expr), '<', (yyvsp[(3) - (3)].expr));
}
    break;

  case 51:

/* Line 1806 of yacc.c  */
#line 896 "sqlparser.y"
    {
	(yyval.expr) = new BinaryExpr(KexiDBExpr_Relational, (yyvsp[(1) - (3)].expr), LESS_OR_EQUAL, (yyvsp[(3) - (3)].expr));
}
    break;

  case 52:

/* Line 1806 of yacc.c  */
#line 900 "sqlparser.y"
    {
	(yyval.expr) = new BinaryExpr(KexiDBExpr_Relational, (yyvsp[(1) - (3)].expr), '=', (yyvsp[(3) - (3)].expr));
}
    break;

  case 54:

/* Line 1806 of yacc.c  */
#line 910 "sqlparser.y"
    {
	(yyval.expr) = new BinaryExpr(KexiDBExpr_Relational, (yyvsp[(1) - (3)].expr), NOT_EQUAL, (yyvsp[(3) - (3)].expr));
}
    break;

  case 55:

/* Line 1806 of yacc.c  */
#line 915 "sqlparser.y"
    {
	(yyval.expr) = new BinaryExpr(KexiDBExpr_Relational, (yyvsp[(1) - (3)].expr), NOT_EQUAL2, (yyvsp[(3) - (3)].expr));
}
    break;

  case 56:

/* Line 1806 of yacc.c  */
#line 919 "sqlparser.y"
    {
	(yyval.expr) = new BinaryExpr(KexiDBExpr_Relational, (yyvsp[(1) - (3)].expr), LIKE, (yyvsp[(3) - (3)].expr));
}
    break;

  case 57:

/* Line 1806 of yacc.c  */
#line 923 "sqlparser.y"
    {
	(yyval.expr) = new BinaryExpr(KexiDBExpr_Relational, (yyvsp[(1) - (3)].expr), SQL_IN, (yyvsp[(3) - (3)].expr));
}
    break;

  case 58:

/* Line 1806 of yacc.c  */
#line 927 "sqlparser.y"
    {
	(yyval.expr) = new BinaryExpr(KexiDBExpr_Relational, (yyvsp[(1) - (3)].expr), SIMILAR_TO, (yyvsp[(3) - (3)].expr));
}
    break;

  case 59:

/* Line 1806 of yacc.c  */
#line 931 "sqlparser.y"
    {
	(yyval.expr) = new BinaryExpr(KexiDBExpr_Relational, (yyvsp[(1) - (3)].expr), NOT_SIMILAR_TO, (yyvsp[(3) - (3)].expr));
}
    break;

  case 61:

/* Line 1806 of yacc.c  */
#line 941 "sqlparser.y"
    {
	(yyval.expr) = new UnaryExpr( SQL_IS_NULL, (yyvsp[(1) - (2)].expr) );
}
    break;

  case 62:

/* Line 1806 of yacc.c  */
#line 945 "sqlparser.y"
    {
	(yyval.expr) = new UnaryExpr( SQL_IS_NOT_NULL, (yyvsp[(1) - (2)].expr) );
}
    break;

  case 64:

/* Line 1806 of yacc.c  */
#line 955 "sqlparser.y"
    {
	(yyval.expr) = new BinaryExpr(KexiDBExpr_Arithm, (yyvsp[(1) - (3)].expr), BITWISE_SHIFT_LEFT, (yyvsp[(3) - (3)].expr));
}
    break;

  case 65:

/* Line 1806 of yacc.c  */
#line 959 "sqlparser.y"
    {
	(yyval.expr) = new BinaryExpr(KexiDBExpr_Arithm, (yyvsp[(1) - (3)].expr), BITWISE_SHIFT_RIGHT, (yyvsp[(3) - (3)].expr));
}
    break;

  case 67:

/* Line 1806 of yacc.c  */
#line 969 "sqlparser.y"
    {
	(yyval.expr) = new BinaryExpr(KexiDBExpr_Arithm, (yyvsp[(1) - (3)].expr), '+', (yyvsp[(3) - (3)].expr));
	(yyval.expr)->debug();
}
    break;

  case 68:

/* Line 1806 of yacc.c  */
#line 974 "sqlparser.y"
    {
    (yyval.expr) = new BinaryExpr(KexiDBExpr_Arithm, (yyvsp[(1) - (3)].expr), CONCATENATION, (yyvsp[(3) - (3)].expr));
}
    break;

  case 69:

/* Line 1806 of yacc.c  */
#line 978 "sqlparser.y"
    {
	(yyval.expr) = new BinaryExpr(KexiDBExpr_Arithm, (yyvsp[(1) - (3)].expr), '-', (yyvsp[(3) - (3)].expr));
}
    break;

  case 70:

/* Line 1806 of yacc.c  */
#line 982 "sqlparser.y"
    {
	(yyval.expr) = new BinaryExpr(KexiDBExpr_Arithm, (yyvsp[(1) - (3)].expr), '&', (yyvsp[(3) - (3)].expr));
}
    break;

  case 71:

/* Line 1806 of yacc.c  */
#line 986 "sqlparser.y"
    {
	(yyval.expr) = new BinaryExpr(KexiDBExpr_Arithm, (yyvsp[(1) - (3)].expr), '|', (yyvsp[(3) - (3)].expr));
}
    break;

  case 73:

/* Line 1806 of yacc.c  */
#line 996 "sqlparser.y"
    {
	(yyval.expr) = new BinaryExpr(KexiDBExpr_Arithm, (yyvsp[(1) - (3)].expr), '/', (yyvsp[(3) - (3)].expr));
}
    break;

  case 74:

/* Line 1806 of yacc.c  */
#line 1000 "sqlparser.y"
    {
	(yyval.expr) = new BinaryExpr(KexiDBExpr_Arithm, (yyvsp[(1) - (3)].expr), '*', (yyvsp[(3) - (3)].expr));
}
    break;

  case 75:

/* Line 1806 of yacc.c  */
#line 1004 "sqlparser.y"
    {
	(yyval.expr) = new BinaryExpr(KexiDBExpr_Arithm, (yyvsp[(1) - (3)].expr), '%', (yyvsp[(3) - (3)].expr));
}
    break;

  case 77:

/* Line 1806 of yacc.c  */
#line 1015 "sqlparser.y"
    {
	(yyval.expr) = new UnaryExpr( '-', (yyvsp[(2) - (2)].expr) );
}
    break;

  case 78:

/* Line 1806 of yacc.c  */
#line 1019 "sqlparser.y"
    {
	(yyval.expr) = new UnaryExpr( '+', (yyvsp[(2) - (2)].expr) );
}
    break;

  case 79:

/* Line 1806 of yacc.c  */
#line 1023 "sqlparser.y"
    {
	(yyval.expr) = new UnaryExpr( '~', (yyvsp[(2) - (2)].expr) );
}
    break;

  case 80:

/* Line 1806 of yacc.c  */
#line 1027 "sqlparser.y"
    {
	(yyval.expr) = new UnaryExpr( NOT, (yyvsp[(2) - (2)].expr) );
}
    break;

  case 81:

/* Line 1806 of yacc.c  */
#line 1031 "sqlparser.y"
    {
	(yyval.expr) = new VariableExpr( *(yyvsp[(1) - (1)].stringValue) );
	
//TODO: simplify this later if that's 'only one field name' expression
	KexiDBDbg << "  + identifier: " << *(yyvsp[(1) - (1)].stringValue);
	delete (yyvsp[(1) - (1)].stringValue);
}
    break;

  case 82:

/* Line 1806 of yacc.c  */
#line 1039 "sqlparser.y"
    {
	(yyval.expr) = new QueryParameterExpr( *(yyvsp[(1) - (1)].stringValue) );
	KexiDBDbg << "  + query parameter: " << (yyval.expr)->debugString();
	delete (yyvsp[(1) - (1)].stringValue);
}
    break;

  case 83:

/* Line 1806 of yacc.c  */
#line 1045 "sqlparser.y"
    {
	KexiDBDbg << "  + function: " << *(yyvsp[(1) - (2)].stringValue) << "(" << (yyvsp[(2) - (2)].exprList)->debugString() << ")";
	(yyval.expr) = new FunctionExpr(*(yyvsp[(1) - (2)].stringValue), (yyvsp[(2) - (2)].exprList));
	delete (yyvsp[(1) - (2)].stringValue);
}
    break;

  case 84:

/* Line 1806 of yacc.c  */
#line 1052 "sqlparser.y"
    {
	(yyval.expr) = new VariableExpr( *(yyvsp[(1) - (3)].stringValue) + "." + *(yyvsp[(3) - (3)].stringValue) );
	KexiDBDbg << "  + identifier.identifier: " << *(yyvsp[(1) - (3)].stringValue) << "." << *(yyvsp[(3) - (3)].stringValue);
	delete (yyvsp[(1) - (3)].stringValue);
	delete (yyvsp[(3) - (3)].stringValue);
}
    break;

  case 85:

/* Line 1806 of yacc.c  */
#line 1059 "sqlparser.y"
    {
	(yyval.expr) = new ConstExpr( SQL_NULL, QVariant() );
	KexiDBDbg << "  + NULL";
//	$$ = new Field();
	//$$->setName(QString());
}
    break;

  case 86:

/* Line 1806 of yacc.c  */
#line 1066 "sqlparser.y"
    {
	(yyval.expr) = new ConstExpr( CHARACTER_STRING_LITERAL, *(yyvsp[(1) - (1)].stringValue) );
	KexiDBDbg << "  + constant " << (yyvsp[(1) - (1)].stringValue);
	delete (yyvsp[(1) - (1)].stringValue);
}
    break;

  case 87:

/* Line 1806 of yacc.c  */
#line 1072 "sqlparser.y"
    {
	QVariant val;
	if ((yyvsp[(1) - (1)].integerValue) <= INT_MAX && (yyvsp[(1) - (1)].integerValue) >= INT_MIN)
		val = (int)(yyvsp[(1) - (1)].integerValue);
	else if ((yyvsp[(1) - (1)].integerValue) <= UINT_MAX && (yyvsp[(1) - (1)].integerValue) >= 0)
		val = (uint)(yyvsp[(1) - (1)].integerValue);
	else if ((yyvsp[(1) - (1)].integerValue) <= LLONG_MAX && (yyvsp[(1) - (1)].integerValue) >= LLONG_MIN)
		val = (qint64)(yyvsp[(1) - (1)].integerValue);

//	if ($1 < ULLONG_MAX)
//		val = (quint64)$1;
//TODO ok?

	(yyval.expr) = new ConstExpr( INTEGER_CONST, val );
	KexiDBDbg << "  + int constant: " << val.toString();
}
    break;

  case 88:

/* Line 1806 of yacc.c  */
#line 1089 "sqlparser.y"
    {
	(yyval.expr) = new ConstExpr( REAL_CONST, QPoint( (yyvsp[(1) - (1)].realValue).integer, (yyvsp[(1) - (1)].realValue).fractional ) );
	KexiDBDbg << "  + real constant: " << (yyvsp[(1) - (1)].realValue).integer << "." << (yyvsp[(1) - (1)].realValue).fractional;
}
    break;

  case 90:

/* Line 1806 of yacc.c  */
#line 1100 "sqlparser.y"
    {
	KexiDBDbg << "(expr)";
	(yyval.expr) = new UnaryExpr('(', (yyvsp[(2) - (3)].expr));
}
    break;

  case 91:

/* Line 1806 of yacc.c  */
#line 1108 "sqlparser.y"
    {
//	$$ = new NArgExpr(0, 0);
//	$$->add( $1 );
//	$$->add( $3 );
	(yyval.exprList) = (yyvsp[(2) - (3)].exprList);
}
    break;

  case 92:

/* Line 1806 of yacc.c  */
#line 1118 "sqlparser.y"
    {
	(yyval.exprList) = (yyvsp[(3) - (3)].exprList);
	(yyval.exprList)->prepend( (yyvsp[(1) - (3)].expr) );
}
    break;

  case 93:

/* Line 1806 of yacc.c  */
#line 1123 "sqlparser.y"
    {
	(yyval.exprList) = new NArgExpr(0, 0);
	(yyval.exprList)->add( (yyvsp[(1) - (3)].expr) );
	(yyval.exprList)->add( (yyvsp[(3) - (3)].expr) );
}
    break;

  case 94:

/* Line 1806 of yacc.c  */
#line 1132 "sqlparser.y"
    {
	(yyval.exprList) = (yyvsp[(2) - (2)].exprList);
}
    break;

  case 95:

/* Line 1806 of yacc.c  */
#line 1177 "sqlparser.y"
    {
	(yyval.exprList) = (yyvsp[(1) - (3)].exprList);
	(yyval.exprList)->add((yyvsp[(3) - (3)].expr));
}
    break;

  case 96:

/* Line 1806 of yacc.c  */
#line 1182 "sqlparser.y"
    {
	(yyval.exprList) = new NArgExpr(KexiDBExpr_TableList, IDENTIFIER); //ok?
	(yyval.exprList)->add((yyvsp[(1) - (1)].expr));
}
    break;

  case 97:

/* Line 1806 of yacc.c  */
#line 1190 "sqlparser.y"
    {
	KexiDBDbg << "FROM: '" << *(yyvsp[(1) - (1)].stringValue) << "'";
	(yyval.expr) = new VariableExpr(*(yyvsp[(1) - (1)].stringValue));

	/*
//TODO: this isn't ok for more tables:
	Field::ListIterator it = parser->select()->fieldsIterator();
	for(Field *item; (item = it.current()); ++it)
	{
		if(item->table() == dummy)
		{
			item->setTable(schema);
		}

		if(item->table() && !item->isQueryAsterisk())
		{
			Field *f = item->table()->field(item->name());
			if(!f)
			{
				ParserError err(i18n("Field List Error"), i18n("Unknown column '%1' in table '%2'",item->name(),schema->name()), ctoken, current);
				parser->setError(err);
				yyerror("fieldlisterror");
			}	
		}
	}*/
	delete (yyvsp[(1) - (1)].stringValue);
}
    break;

  case 98:

/* Line 1806 of yacc.c  */
#line 1218 "sqlparser.y"
    {
	//table + alias
	(yyval.expr) = new BinaryExpr(
		KexiDBExpr_SpecialBinary, 
		new VariableExpr(*(yyvsp[(1) - (2)].stringValue)), 0,
		new VariableExpr(*(yyvsp[(2) - (2)].stringValue))
	);
	delete (yyvsp[(1) - (2)].stringValue);
	delete (yyvsp[(2) - (2)].stringValue);
}
    break;

  case 99:

/* Line 1806 of yacc.c  */
#line 1229 "sqlparser.y"
    {
	//table + alias
	(yyval.expr) = new BinaryExpr(
		KexiDBExpr_SpecialBinary,
		new VariableExpr(*(yyvsp[(1) - (3)].stringValue)), AS,
		new VariableExpr(*(yyvsp[(3) - (3)].stringValue))
	);
	delete (yyvsp[(1) - (3)].stringValue);
	delete (yyvsp[(3) - (3)].stringValue);
}
    break;

  case 100:

/* Line 1806 of yacc.c  */
#line 1245 "sqlparser.y"
    {
	(yyval.exprList) = (yyvsp[(1) - (3)].exprList);
	(yyval.exprList)->add( (yyvsp[(3) - (3)].expr) );
	KexiDBDbg << "ColViews: ColViews , ColItem";
}
    break;

  case 101:

/* Line 1806 of yacc.c  */
#line 1251 "sqlparser.y"
    {
	(yyval.exprList) = new NArgExpr(0,0);
	(yyval.exprList)->add( (yyvsp[(1) - (1)].expr) );
	KexiDBDbg << "ColViews: ColItem";
}
    break;

  case 102:

/* Line 1806 of yacc.c  */
#line 1260 "sqlparser.y"
    {
//	$$ = new Field();
//	dummy->addField($$);
//	$$->setExpression( $1 );
//	parser->select()->addField($$);
	(yyval.expr) = (yyvsp[(1) - (1)].expr);
	KexiDBDbg << " added column expr: '" << (yyvsp[(1) - (1)].expr)->debugString() << "'";
}
    break;

  case 103:

/* Line 1806 of yacc.c  */
#line 1269 "sqlparser.y"
    {
	(yyval.expr) = (yyvsp[(1) - (1)].expr);
	KexiDBDbg << " added column wildcard: '" << (yyvsp[(1) - (1)].expr)->debugString() << "'";
}
    break;

  case 104:

/* Line 1806 of yacc.c  */
#line 1274 "sqlparser.y"
    {
	(yyval.expr) = new BinaryExpr(
		KexiDBExpr_SpecialBinary, (yyvsp[(1) - (3)].expr), AS,
		new VariableExpr(*(yyvsp[(3) - (3)].stringValue))
	);
	KexiDBDbg << " added column expr: " << (yyval.expr)->debugString();
	delete (yyvsp[(3) - (3)].stringValue);
}
    break;

  case 105:

/* Line 1806 of yacc.c  */
#line 1283 "sqlparser.y"
    {
	(yyval.expr) = new BinaryExpr(
		KexiDBExpr_SpecialBinary, (yyvsp[(1) - (2)].expr), 0, 
		new VariableExpr(*(yyvsp[(2) - (2)].stringValue))
	);
	KexiDBDbg << " added column expr: " << (yyval.expr)->debugString();
	delete (yyvsp[(2) - (2)].stringValue);
}
    break;

  case 106:

/* Line 1806 of yacc.c  */
#line 1295 "sqlparser.y"
    {
	(yyval.expr) = (yyvsp[(1) - (1)].expr);
}
    break;

  case 107:

/* Line 1806 of yacc.c  */
#line 1339 "sqlparser.y"
    {
	(yyval.expr) = (yyvsp[(3) - (4)].expr);
//TODO
//	$$->setName("DISTINCT(" + $3->name() + ")");
}
    break;

  case 108:

/* Line 1806 of yacc.c  */
#line 1348 "sqlparser.y"
    {
	(yyval.expr) = new VariableExpr("*");
	KexiDBDbg << "all columns";

//	QueryAsterisk *ast = new QueryAsterisk(parser->select(), dummy);
//	parser->select()->addAsterisk(ast);
//	requiresTable = true;
}
    break;

  case 109:

/* Line 1806 of yacc.c  */
#line 1357 "sqlparser.y"
    {
	QString s( *(yyvsp[(1) - (3)].stringValue) );
	s += ".*";
	(yyval.expr) = new VariableExpr(s);
	KexiDBDbg << "  + all columns from " << s;
	delete (yyvsp[(1) - (3)].stringValue);
}
    break;



/* Line 1806 of yacc.c  */
#line 2724 "sqlparser.cpp"
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
#line 1372 "sqlparser.y"



const char* tname(int offset) { return yytname[offset]; }
