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
#line 438 "sqlparser.y"

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

#include <KDebug>
#include <KLocale>

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
     UMINUS = 258,
     SQL_TYPE = 259,
     SQL_ABS = 260,
     ACOS = 261,
     AMPERSAND = 262,
     SQL_ABSOLUTE = 263,
     ADA = 264,
     ADD = 265,
     ADD_DAYS = 266,
     ADD_HOURS = 267,
     ADD_MINUTES = 268,
     ADD_MONTHS = 269,
     ADD_SECONDS = 270,
     ADD_YEARS = 271,
     ALL = 272,
     ALLOCATE = 273,
     ALTER = 274,
     AND = 275,
     ANY = 276,
     ARE = 277,
     AS = 278,
     ASIN = 279,
     ASC = 280,
     ASCII = 281,
     ASSERTION = 282,
     ATAN = 283,
     ATAN2 = 284,
     AUTHORIZATION = 285,
     AUTO_INCREMENT = 286,
     AVG = 287,
     BEFORE = 288,
     SQL_BEGIN = 289,
     BETWEEN = 290,
     BIGINT = 291,
     BINARY = 292,
     BIT = 293,
     BIT_LENGTH = 294,
     BITWISE_SHIFT_LEFT = 295,
     BITWISE_SHIFT_RIGHT = 296,
     BREAK = 297,
     BY = 298,
     CASCADE = 299,
     CASCADED = 300,
     CASE = 301,
     CAST = 302,
     CATALOG = 303,
     CEILING = 304,
     CENTER = 305,
     SQL_CHAR = 306,
     CHAR_LENGTH = 307,
     CHARACTER_STRING_LITERAL = 308,
     CHECK = 309,
     CLOSE = 310,
     COALESCE = 311,
     COBOL = 312,
     COLLATE = 313,
     COLLATION = 314,
     COLUMN = 315,
     COMMIT = 316,
     COMPUTE = 317,
     CONCAT = 318,
     CONCATENATION = 319,
     CONNECT = 320,
     CONNECTION = 321,
     CONSTRAINT = 322,
     CONSTRAINTS = 323,
     CONTINUE = 324,
     CONVERT = 325,
     CORRESPONDING = 326,
     COS = 327,
     COT = 328,
     COUNT = 329,
     CREATE = 330,
     CURDATE = 331,
     CURRENT = 332,
     CURRENT_DATE = 333,
     CURRENT_TIME = 334,
     CURRENT_TIMESTAMP = 335,
     CURTIME = 336,
     CURSOR = 337,
     DATABASE = 338,
     SQL_DATE = 339,
     DATE_FORMAT = 340,
     DATE_REMAINDER = 341,
     DATE_VALUE = 342,
     DAY = 343,
     DAYOFMONTH = 344,
     DAYOFWEEK = 345,
     DAYOFYEAR = 346,
     DAYS_BETWEEN = 347,
     DEALLOCATE = 348,
     DEC = 349,
     DECLARE = 350,
     DEFAULT = 351,
     DEFERRABLE = 352,
     DEFERRED = 353,
     SQL_DELETE = 354,
     DESC = 355,
     DESCRIBE = 356,
     DESCRIPTOR = 357,
     DIAGNOSTICS = 358,
     DICTIONARY = 359,
     DIRECTORY = 360,
     DISCONNECT = 361,
     DISPLACEMENT = 362,
     DISTINCT = 363,
     DOMAIN_TOKEN = 364,
     SQL_DOUBLE = 365,
     DOUBLE_QUOTED_STRING = 366,
     DROP = 367,
     ELSE = 368,
     END = 369,
     END_EXEC = 370,
     EQUAL = 371,
     ESCAPE = 372,
     EXCEPT = 373,
     SQL_EXCEPTION = 374,
     EXEC = 375,
     EXECUTE = 376,
     EXISTS = 377,
     EXP = 378,
     EXPONENT = 379,
     EXTERNAL = 380,
     EXTRACT = 381,
     SQL_FALSE = 382,
     FETCH = 383,
     FIRST = 384,
     SQL_FLOAT = 385,
     FLOOR = 386,
     FN = 387,
     FOR = 388,
     FOREIGN = 389,
     FORTRAN = 390,
     FOUND = 391,
     FOUR_DIGITS = 392,
     FROM = 393,
     FULL = 394,
     GET = 395,
     GLOBAL = 396,
     GO = 397,
     GOTO = 398,
     GRANT = 399,
     GREATER_OR_EQUAL = 400,
     HAVING = 401,
     HOUR = 402,
     HOURS_BETWEEN = 403,
     IDENTITY = 404,
     IFNULL = 405,
     SQL_IGNORE = 406,
     IMMEDIATE = 407,
     SQL_IN = 408,
     INCLUDE = 409,
     INDEX = 410,
     INDICATOR = 411,
     INITIALLY = 412,
     INNER = 413,
     SQL_INPUT = 414,
     INSENSITIVE = 415,
     INSERT = 416,
     INTEGER = 417,
     INTERSECT = 418,
     INTERVAL = 419,
     INTO = 420,
     IS = 421,
     ISOLATION = 422,
     JOIN = 423,
     JUSTIFY = 424,
     KEY = 425,
     LANGUAGE = 426,
     LAST = 427,
     LCASE = 428,
     LEFT = 429,
     LENGTH = 430,
     LESS_OR_EQUAL = 431,
     LEVEL = 432,
     LIKE = 433,
     LINE_WIDTH = 434,
     LOCAL = 435,
     LOCATE = 436,
     LOG = 437,
     SQL_LONG = 438,
     LOWER = 439,
     LTRIM = 440,
     LTRIP = 441,
     MATCH = 442,
     SQL_MAX = 443,
     MICROSOFT = 444,
     SQL_MIN = 445,
     MINUS = 446,
     MINUTE = 447,
     MINUTES_BETWEEN = 448,
     MOD = 449,
     MODIFY = 450,
     MODULE = 451,
     MONTH = 452,
     MONTHS_BETWEEN = 453,
     MUMPS = 454,
     NAMES = 455,
     NATIONAL = 456,
     NCHAR = 457,
     NEXT = 458,
     NODUP = 459,
     NONE = 460,
     NOT = 461,
     NOT_EQUAL = 462,
     NOT_EQUAL2 = 463,
     NOW = 464,
     SQL_NULL = 465,
     SQL_IS = 466,
     SQL_IS_NULL = 467,
     SQL_IS_NOT_NULL = 468,
     NULLIF = 469,
     NUMERIC = 470,
     OCTET_LENGTH = 471,
     ODBC = 472,
     OF = 473,
     SQL_OFF = 474,
     SQL_ON = 475,
     ONLY = 476,
     OPEN = 477,
     OPTION = 478,
     OR = 479,
     ORDER = 480,
     OUTER = 481,
     OUTPUT = 482,
     OVERLAPS = 483,
     PAGE = 484,
     PARTIAL = 485,
     SQL_PASCAL = 486,
     PERSISTENT = 487,
     CQL_PI = 488,
     PLI = 489,
     POSITION = 490,
     PRECISION = 491,
     PREPARE = 492,
     PRESERVE = 493,
     PRIMARY = 494,
     PRIOR = 495,
     PRIVILEGES = 496,
     PROCEDURE = 497,
     PRODUCT = 498,
     PUBLIC = 499,
     QUARTER = 500,
     QUIT = 501,
     RAND = 502,
     READ_ONLY = 503,
     REAL = 504,
     REFERENCES = 505,
     REPEAT = 506,
     REPLACE = 507,
     RESTRICT = 508,
     REVOKE = 509,
     RIGHT = 510,
     ROLLBACK = 511,
     ROWS = 512,
     RPAD = 513,
     RTRIM = 514,
     SCHEMA = 515,
     SCREEN_WIDTH = 516,
     SCROLL = 517,
     SECOND = 518,
     SECONDS_BETWEEN = 519,
     SELECT = 520,
     SEQUENCE = 521,
     SETOPT = 522,
     SET = 523,
     SHOWOPT = 524,
     SIGN = 525,
     SIMILAR_TO = 526,
     NOT_SIMILAR_TO = 527,
     INTEGER_CONST = 528,
     REAL_CONST = 529,
     DATE_CONST = 530,
     DATETIME_CONST = 531,
     TIME_CONST = 532,
     SIN = 533,
     SQL_SIZE = 534,
     SMALLINT = 535,
     SOME = 536,
     SPACE = 537,
     SQL = 538,
     SQL_TRUE = 539,
     SQLCA = 540,
     SQLCODE = 541,
     SQLERROR = 542,
     SQLSTATE = 543,
     SQLWARNING = 544,
     SQRT = 545,
     STDEV = 546,
     SUBSTRING = 547,
     SUM = 548,
     SYSDATE = 549,
     SYSDATE_FORMAT = 550,
     SYSTEM = 551,
     TABLE = 552,
     TAN = 553,
     TEMPORARY = 554,
     THEN = 555,
     THREE_DIGITS = 556,
     TIME = 557,
     TIMESTAMP = 558,
     TIMEZONE_HOUR = 559,
     TIMEZONE_MINUTE = 560,
     TINYINT = 561,
     TO = 562,
     TO_CHAR = 563,
     TO_DATE = 564,
     TRANSACTION = 565,
     TRANSLATE = 566,
     TRANSLATION = 567,
     TRUNCATE = 568,
     GENERAL_TITLE = 569,
     TWO_DIGITS = 570,
     UCASE = 571,
     UNION = 572,
     UNIQUE = 573,
     SQL_UNKNOWN = 574,
     UPDATE = 575,
     UPPER = 576,
     USAGE = 577,
     USER = 578,
     IDENTIFIER = 579,
     IDENTIFIER_DOT_ASTERISK = 580,
     QUERY_PARAMETER = 581,
     USING = 582,
     VALUE = 583,
     VALUES = 584,
     VARBINARY = 585,
     VARCHAR = 586,
     VARYING = 587,
     VENDOR = 588,
     VIEW = 589,
     WEEK = 590,
     WHEN = 591,
     WHENEVER = 592,
     WHERE = 593,
     WHERE_CURRENT_OF = 594,
     WITH = 595,
     WORD_WRAPPED = 596,
     WORK = 597,
     WRAPPED = 598,
     XOR = 599,
     YEAR = 600,
     YEARS_BETWEEN = 601,
     SCAN_ERROR = 602,
     __LAST_TOKEN = 603,
     ILIKE = 604
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 293 of yacc.c  */
#line 514 "sqlparser.y"

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
#line 551 "sqlparser.cpp"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 343 of yacc.c  */
#line 563 "sqlparser.cpp"

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
#define YYLAST   335

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  373
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  37
/* YYNRULES -- Number of rules.  */
#define YYNRULES  109
/* YYNRULES -- Number of states.  */
#define YYNSTATES  178

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   604

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint16 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,   357,   352,   370,   361,
     358,   359,   351,   350,   355,   349,   356,   362,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,   354,
     364,   363,   365,   360,   353,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   368,     2,   369,   367,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,   371,     2,   372,     2,     2,     2,
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
     155,   156,   157,   158,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   187,   188,   189,   190,   191,   192,   193,   194,
     195,   196,   197,   198,   199,   200,   201,   202,   203,   204,
     205,   206,   207,   208,   209,   210,   211,   212,   213,   214,
     215,   216,   217,   218,   219,   220,   221,   222,   223,   224,
     225,   226,   227,   228,   229,   230,   231,   232,   233,   234,
     235,   236,   237,   238,   239,   240,   241,   242,   243,   244,
     245,   246,   247,   248,   249,   250,   251,   252,   253,   254,
     255,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   366
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
static const yytype_int16 yyrhs[] =
{
     374,     0,    -1,   375,    -1,   376,   354,   375,    -1,   376,
      -1,   376,   354,    -1,   377,    -1,   384,    -1,    -1,    75,
     297,   324,   378,   358,   379,   359,    -1,   379,   355,   380,
      -1,   380,    -1,   324,   383,    -1,   324,   383,   381,    -1,
     381,   382,    -1,   382,    -1,   239,   170,    -1,   206,   210,
      -1,    31,    -1,     4,    -1,     4,   358,   273,   359,    -1,
     331,   358,   273,   359,    -1,    -1,   385,   406,    -1,   385,
     406,   403,    -1,   385,   403,    -1,   385,   406,   386,    -1,
     385,   406,   403,   386,    -1,   265,    -1,   387,    -1,   225,
      43,   388,    -1,   387,   225,    43,   388,    -1,   225,    43,
     388,   387,    -1,   338,   391,    -1,   389,    -1,   389,   390,
      -1,   389,   355,   388,    -1,   389,   390,   355,   388,    -1,
     324,    -1,   324,   356,   324,    -1,   273,    -1,    25,    -1,
     100,    -1,   392,    -1,   393,    20,   392,    -1,   393,   224,
     392,    -1,   393,   344,   392,    -1,   393,    -1,   394,   365,
     393,    -1,   394,   145,   393,    -1,   394,   364,   393,    -1,
     394,   176,   393,    -1,   394,   363,   393,    -1,   394,    -1,
     395,   207,   394,    -1,   395,   208,   394,    -1,   395,   178,
     394,    -1,   395,   153,   394,    -1,   395,   271,   394,    -1,
     395,   272,   394,    -1,   395,    -1,   395,   212,    -1,   395,
     213,    -1,   396,    -1,   397,    40,   396,    -1,   397,    41,
     396,    -1,   397,    -1,   398,   350,   397,    -1,   398,    64,
     397,    -1,   398,   349,   397,    -1,   398,   370,   397,    -1,
     398,   371,   397,    -1,   398,    -1,   399,   362,   398,    -1,
     399,   351,   398,    -1,   399,   352,   398,    -1,   399,    -1,
     349,   399,    -1,   350,   399,    -1,   372,   399,    -1,   206,
     399,    -1,   324,    -1,   326,    -1,   324,   401,    -1,   324,
     356,   324,    -1,   210,    -1,    53,    -1,   273,    -1,   274,
      -1,   400,    -1,   358,   391,   359,    -1,   358,   402,   359,
      -1,   391,   355,   402,    -1,   391,   355,   391,    -1,   138,
     404,    -1,   404,   355,   405,    -1,   405,    -1,   324,    -1,
     324,   324,    -1,   324,    23,   324,    -1,   406,   355,   407,
      -1,   407,    -1,   408,    -1,   409,    -1,   408,    23,   324,
      -1,   408,   324,    -1,   391,    -1,   108,   358,   408,   359,
      -1,   351,    -1,   324,   356,   351,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   583,   583,   593,   597,   598,   608,   612,   620,   619,
     629,   629,   635,   643,   659,   659,   665,   670,   675,   683,
     688,   695,   702,   710,   717,   722,   728,   734,   743,   753,
     759,   765,   772,   782,   791,   800,   810,   818,   830,   836,
     843,   850,   854,   861,   866,   871,   875,   880,   885,   889,
     893,   897,   901,   906,   911,   916,   920,   924,   928,   932,
     937,   942,   946,   951,   956,   960,   965,   970,   975,   979,
     983,   987,   992,   997,  1001,  1005,  1010,  1016,  1020,  1024,
    1028,  1032,  1040,  1046,  1053,  1060,  1067,  1073,  1090,  1096,
    1101,  1109,  1119,  1124,  1133,  1178,  1183,  1191,  1219,  1230,
    1246,  1252,  1261,  1270,  1275,  1284,  1296,  1340,  1349,  1358
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "UMINUS", "SQL_TYPE", "SQL_ABS", "ACOS",
  "AMPERSAND", "SQL_ABSOLUTE", "ADA", "ADD", "ADD_DAYS", "ADD_HOURS",
  "ADD_MINUTES", "ADD_MONTHS", "ADD_SECONDS", "ADD_YEARS", "ALL",
  "ALLOCATE", "ALTER", "AND", "ANY", "ARE", "AS", "ASIN", "ASC", "ASCII",
  "ASSERTION", "ATAN", "ATAN2", "AUTHORIZATION", "AUTO_INCREMENT", "AVG",
  "BEFORE", "SQL_BEGIN", "BETWEEN", "BIGINT", "BINARY", "BIT",
  "BIT_LENGTH", "BITWISE_SHIFT_LEFT", "BITWISE_SHIFT_RIGHT", "BREAK", "BY",
  "CASCADE", "CASCADED", "CASE", "CAST", "CATALOG", "CEILING", "CENTER",
  "SQL_CHAR", "CHAR_LENGTH", "CHARACTER_STRING_LITERAL", "CHECK", "CLOSE",
  "COALESCE", "COBOL", "COLLATE", "COLLATION", "COLUMN", "COMMIT",
  "COMPUTE", "CONCAT", "CONCATENATION", "CONNECT", "CONNECTION",
  "CONSTRAINT", "CONSTRAINTS", "CONTINUE", "CONVERT", "CORRESPONDING",
  "COS", "COT", "COUNT", "CREATE", "CURDATE", "CURRENT", "CURRENT_DATE",
  "CURRENT_TIME", "CURRENT_TIMESTAMP", "CURTIME", "CURSOR", "DATABASE",
  "SQL_DATE", "DATE_FORMAT", "DATE_REMAINDER", "DATE_VALUE", "DAY",
  "DAYOFMONTH", "DAYOFWEEK", "DAYOFYEAR", "DAYS_BETWEEN", "DEALLOCATE",
  "DEC", "DECLARE", "DEFAULT", "DEFERRABLE", "DEFERRED", "SQL_DELETE",
  "DESC", "DESCRIBE", "DESCRIPTOR", "DIAGNOSTICS", "DICTIONARY",
  "DIRECTORY", "DISCONNECT", "DISPLACEMENT", "DISTINCT", "DOMAIN_TOKEN",
  "SQL_DOUBLE", "DOUBLE_QUOTED_STRING", "DROP", "ELSE", "END", "END_EXEC",
  "EQUAL", "ESCAPE", "EXCEPT", "SQL_EXCEPTION", "EXEC", "EXECUTE",
  "EXISTS", "EXP", "EXPONENT", "EXTERNAL", "EXTRACT", "SQL_FALSE", "FETCH",
  "FIRST", "SQL_FLOAT", "FLOOR", "FN", "FOR", "FOREIGN", "FORTRAN",
  "FOUND", "FOUR_DIGITS", "FROM", "FULL", "GET", "GLOBAL", "GO", "GOTO",
  "GRANT", "GREATER_OR_EQUAL", "HAVING", "HOUR", "HOURS_BETWEEN",
  "IDENTITY", "IFNULL", "SQL_IGNORE", "IMMEDIATE", "SQL_IN", "INCLUDE",
  "INDEX", "INDICATOR", "INITIALLY", "INNER", "SQL_INPUT", "INSENSITIVE",
  "INSERT", "INTEGER", "INTERSECT", "INTERVAL", "INTO", "IS", "ISOLATION",
  "JOIN", "JUSTIFY", "KEY", "LANGUAGE", "LAST", "LCASE", "LEFT", "LENGTH",
  "LESS_OR_EQUAL", "LEVEL", "LIKE", "LINE_WIDTH", "LOCAL", "LOCATE", "LOG",
  "SQL_LONG", "LOWER", "LTRIM", "LTRIP", "MATCH", "SQL_MAX", "MICROSOFT",
  "SQL_MIN", "MINUS", "MINUTE", "MINUTES_BETWEEN", "MOD", "MODIFY",
  "MODULE", "MONTH", "MONTHS_BETWEEN", "MUMPS", "NAMES", "NATIONAL",
  "NCHAR", "NEXT", "NODUP", "NONE", "NOT", "NOT_EQUAL", "NOT_EQUAL2",
  "NOW", "SQL_NULL", "SQL_IS", "SQL_IS_NULL", "SQL_IS_NOT_NULL", "NULLIF",
  "NUMERIC", "OCTET_LENGTH", "ODBC", "OF", "SQL_OFF", "SQL_ON", "ONLY",
  "OPEN", "OPTION", "OR", "ORDER", "OUTER", "OUTPUT", "OVERLAPS", "PAGE",
  "PARTIAL", "SQL_PASCAL", "PERSISTENT", "CQL_PI", "PLI", "POSITION",
  "PRECISION", "PREPARE", "PRESERVE", "PRIMARY", "PRIOR", "PRIVILEGES",
  "PROCEDURE", "PRODUCT", "PUBLIC", "QUARTER", "QUIT", "RAND", "READ_ONLY",
  "REAL", "REFERENCES", "REPEAT", "REPLACE", "RESTRICT", "REVOKE", "RIGHT",
  "ROLLBACK", "ROWS", "RPAD", "RTRIM", "SCHEMA", "SCREEN_WIDTH", "SCROLL",
  "SECOND", "SECONDS_BETWEEN", "SELECT", "SEQUENCE", "SETOPT", "SET",
  "SHOWOPT", "SIGN", "SIMILAR_TO", "NOT_SIMILAR_TO", "INTEGER_CONST",
  "REAL_CONST", "DATE_CONST", "DATETIME_CONST", "TIME_CONST", "SIN",
  "SQL_SIZE", "SMALLINT", "SOME", "SPACE", "SQL", "SQL_TRUE", "SQLCA",
  "SQLCODE", "SQLERROR", "SQLSTATE", "SQLWARNING", "SQRT", "STDEV",
  "SUBSTRING", "SUM", "SYSDATE", "SYSDATE_FORMAT", "SYSTEM", "TABLE",
  "TAN", "TEMPORARY", "THEN", "THREE_DIGITS", "TIME", "TIMESTAMP",
  "TIMEZONE_HOUR", "TIMEZONE_MINUTE", "TINYINT", "TO", "TO_CHAR",
  "TO_DATE", "TRANSACTION", "TRANSLATE", "TRANSLATION", "TRUNCATE",
  "GENERAL_TITLE", "TWO_DIGITS", "UCASE", "UNION", "UNIQUE", "SQL_UNKNOWN",
  "UPDATE", "UPPER", "USAGE", "USER", "IDENTIFIER",
  "IDENTIFIER_DOT_ASTERISK", "QUERY_PARAMETER", "USING", "VALUE", "VALUES",
  "VARBINARY", "VARCHAR", "VARYING", "VENDOR", "VIEW", "WEEK", "WHEN",
  "WHENEVER", "WHERE", "WHERE_CURRENT_OF", "WITH", "WORD_WRAPPED", "WORK",
  "WRAPPED", "XOR", "YEAR", "YEARS_BETWEEN", "SCAN_ERROR", "__LAST_TOKEN",
  "'-'", "'+'", "'*'", "'%'", "'@'", "';'", "','", "'.'", "'$'", "'('",
  "')'", "'?'", "'\\''", "'/'", "'='", "'<'", "'>'", "ILIKE", "'^'", "'['",
  "']'", "'&'", "'|'", "'~'", "$accept", "TopLevelStatement",
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
     415,   416,   417,   418,   419,   420,   421,   422,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439,   440,   441,   442,   443,   444,
     445,   446,   447,   448,   449,   450,   451,   452,   453,   454,
     455,   456,   457,   458,   459,   460,   461,   462,   463,   464,
     465,   466,   467,   468,   469,   470,   471,   472,   473,   474,
     475,   476,   477,   478,   479,   480,   481,   482,   483,   484,
     485,   486,   487,   488,   489,   490,   491,   492,   493,   494,
     495,   496,   497,   498,   499,   500,   501,   502,   503,   504,
     505,   506,   507,   508,   509,   510,   511,   512,   513,   514,
     515,   516,   517,   518,   519,   520,   521,   522,   523,   524,
     525,   526,   527,   528,   529,   530,   531,   532,   533,   534,
     535,   536,   537,   538,   539,   540,   541,   542,   543,   544,
     545,   546,   547,   548,   549,   550,   551,   552,   553,   554,
     555,   556,   557,   558,   559,   560,   561,   562,   563,   564,
     565,   566,   567,   568,   569,   570,   571,   572,   573,   574,
     575,   576,   577,   578,   579,   580,   581,   582,   583,   584,
     585,   586,   587,   588,   589,   590,   591,   592,   593,   594,
     595,   596,   597,   598,   599,   600,   601,   602,   603,    45,
      43,    42,    37,    64,    59,    44,    46,    36,    40,    41,
      63,    39,    47,    61,    60,    62,   604,    94,    91,    93,
      38,   124,   126
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   373,   374,   375,   375,   375,   376,   376,   378,   377,
     379,   379,   380,   380,   381,   381,   382,   382,   382,   383,
     383,   383,   383,   384,   384,   384,   384,   384,   385,   386,
     386,   386,   386,   387,   388,   388,   388,   388,   389,   389,
     389,   390,   390,   391,   392,   392,   392,   392,   393,   393,
     393,   393,   393,   393,   394,   394,   394,   394,   394,   394,
     394,   395,   395,   395,   396,   396,   396,   397,   397,   397,
     397,   397,   397,   398,   398,   398,   398,   399,   399,   399,
     399,   399,   399,   399,   399,   399,   399,   399,   399,   399,
     400,   401,   402,   402,   403,   404,   404,   405,   405,   405,
     406,   406,   407,   407,   407,   407,   408,   408,   409,   409
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
       1,     5,    86,     0,     0,     0,    85,    87,    88,    81,
      82,     0,     0,   108,     0,     0,   106,    43,    47,    53,
      60,    63,    66,    72,    76,    89,    25,    23,   101,   102,
     103,     8,     3,     0,    97,    94,    96,    81,    80,     0,
       0,    83,    77,    78,     0,    79,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    61,    62,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    26,    29,    24,     0,   105,
       0,     0,     0,    98,     0,     0,    84,   109,     0,     0,
      90,    44,    45,    46,    49,    51,    52,    50,    48,    57,
      56,    54,    55,    58,    59,    64,    65,    68,    69,    67,
      70,    71,    74,    75,    73,     0,    33,   100,     0,    27,
     104,     0,   107,    99,    95,     0,    91,    40,    38,    30,
      34,     0,    22,     0,    11,    93,    92,     0,    32,    41,
      42,     0,    35,    31,    19,     0,    12,     0,     9,    39,
      36,     0,     0,     0,    18,     0,     0,    13,    15,    10,
      37,     0,     0,    17,    16,    14,    20,    21
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     3,     4,     5,     6,    90,   143,   144,   167,   168,
     156,     7,     8,    85,    86,   139,   140,   152,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    51,    99,
      36,    45,    46,    37,    38,    39,    40
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -333
static const yytype_int16 yypact[] =
{
     -66,  -267,  -333,    51,  -333,  -302,  -333,  -333,   -50,  -265,
    -333,   -66,  -333,  -298,  -262,   -37,  -333,  -333,  -333,  -323,
    -333,   -37,   -37,  -333,   -37,   -37,  -333,  -333,   -18,  -135,
    -142,  -333,   -15,   -56,  -331,  -333,  -333,  -133,  -333,   -19,
    -333,  -333,  -333,   -40,    -8,  -292,  -333,  -316,  -333,  -305,
     -37,  -333,  -333,  -333,  -295,  -333,   -37,   -37,   -37,   -37,
     -37,   -37,   -37,   -37,   -37,   -37,   -37,   -37,  -333,  -333,
     -37,   -37,   -37,   -37,   -37,   -37,   -37,   -37,   -37,   -37,
     -37,   -37,    24,   -37,   -47,  -333,  -153,  -213,  -251,  -333,
    -284,  -272,  -235,  -333,  -262,  -234,  -333,  -333,  -264,  -266,
    -333,  -333,  -333,  -333,  -333,  -333,  -333,  -333,  -333,  -333,
    -333,  -333,  -333,  -333,  -333,  -333,  -333,  -333,  -333,  -333,
    -333,  -333,  -333,  -333,  -333,  -255,  -333,  -333,    52,  -333,
    -333,  -230,  -333,  -333,  -333,   -37,  -333,  -333,  -260,  -241,
     -25,  -255,    -3,  -332,  -333,  -264,  -333,  -226,  -333,  -333,
    -333,  -255,  -256,  -333,  -258,  -257,   -24,  -230,  -333,  -333,
    -333,  -255,  -171,  -170,  -333,  -106,   -65,   -24,  -333,  -333,
    -333,  -253,  -252,  -333,  -333,  -333,  -333,  -333
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -333,  -333,    97,  -333,  -333,  -333,  -333,   -48,  -333,   -57,
    -333,  -333,  -333,    25,   -26,  -127,  -333,  -333,    -7,    -1,
      18,   -17,  -333,   -28,     8,   -42,     7,  -333,  -333,   -21,
      74,  -333,    21,  -333,    32,    75,  -333
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint8 yytable[] =
{
     149,   154,    56,    12,    88,    14,    12,   164,    74,     1,
      59,    64,    82,    12,   153,    92,    12,    54,   137,    96,
      79,    80,    48,   157,   160,    72,    73,   158,    52,    53,
       9,    81,    55,    49,   170,    50,    65,   122,   123,   124,
      95,    60,    50,    98,   115,   116,    97,   109,   110,   111,
     112,    10,    11,   113,   114,   101,   102,   103,    13,    41,
      43,    13,    44,    94,   100,    66,    67,   125,    13,   138,
      68,    69,   128,   130,   131,   150,   126,   104,   105,   106,
     107,   108,   117,   118,   119,   120,   121,   132,    14,   133,
      96,   135,    82,   136,   142,   141,   147,    83,   159,   161,
     162,   163,   171,   172,   173,   174,   176,   177,    42,   169,
     175,    87,   129,   148,   146,   134,   127,     0,    91,     0,
       0,     0,     0,     0,     0,    83,     0,     0,   145,    70,
      71,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    15,     0,     0,    15,
      16,     0,     0,    16,     0,     0,    15,     0,     0,    15,
      16,     0,     0,    16,     0,     0,     0,     0,     0,     0,
       0,     0,   165,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     2,
       0,     0,     0,     0,     0,    83,    57,     0,     0,     0,
       0,     0,     0,     0,     0,   166,     0,     0,     0,     0,
       0,     0,    84,    17,    18,     0,    17,    18,    61,    62,
      63,     0,     0,    17,    18,     0,    17,    18,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    19,     0,    20,    19,     0,    20,
       0,     0,     0,     0,    47,     0,    20,    47,     0,    20,
       0,     0,     0,    75,    76,     0,     0,     0,     0,    21,
      22,    23,    21,    22,    23,    89,     0,     0,    24,    21,
      22,    24,    21,    22,    77,    78,    93,     0,    24,     0,
       0,    24,    25,     0,     0,    25,    58,     0,   155,     0,
     151,     0,    25,     0,     0,    25
};

#define yypact_value_is_default(yystate) \
  ((yystate) == (-333))

#define yytable_value_is_error(yytable_value) \
  YYID (0)

static const yytype_int16 yycheck[] =
{
      25,     4,    20,    53,    23,   138,    53,    31,    64,    75,
     145,   153,   225,    53,   141,    23,    53,    24,   273,   324,
     351,   352,    15,   355,   151,    40,    41,   359,    21,    22,
     297,   362,    25,   356,   161,   358,   178,    79,    80,    81,
     356,   176,   358,    50,    72,    73,   351,    64,    65,    66,
      67,     0,   354,    70,    71,    56,    57,    58,   108,   324,
     358,   108,   324,   355,   359,   207,   208,    43,   108,   324,
     212,   213,   225,   324,   358,   100,    83,    59,    60,    61,
      62,    63,    74,    75,    76,    77,    78,   359,   138,   324,
     324,   355,   225,   359,   324,    43,   356,   338,   324,   355,
     358,   358,   273,   273,   210,   170,   359,   359,    11,   157,
     167,    37,    87,   139,   135,    94,    84,    -1,    43,    -1,
      -1,    -1,    -1,    -1,    -1,   338,    -1,    -1,   135,   271,
     272,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   206,    -1,    -1,   206,
     210,    -1,    -1,   210,    -1,    -1,   206,    -1,    -1,   206,
     210,    -1,    -1,   210,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   206,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   265,
      -1,    -1,    -1,    -1,    -1,   338,   224,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   239,    -1,    -1,    -1,    -1,
      -1,    -1,   355,   273,   274,    -1,   273,   274,   363,   364,
     365,    -1,    -1,   273,   274,    -1,   273,   274,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   324,    -1,   326,   324,    -1,   326,
      -1,    -1,    -1,    -1,   324,    -1,   326,   324,    -1,   326,
      -1,    -1,    -1,   349,   350,    -1,    -1,    -1,    -1,   349,
     350,   351,   349,   350,   351,   324,    -1,    -1,   358,   349,
     350,   358,   349,   350,   370,   371,   324,    -1,   358,    -1,
      -1,   358,   372,    -1,    -1,   372,   344,    -1,   331,    -1,
     355,    -1,   372,    -1,    -1,   372
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,    75,   265,   374,   375,   376,   377,   384,   385,   297,
       0,   354,    53,   108,   138,   206,   210,   273,   274,   324,
     326,   349,   350,   351,   358,   372,   391,   392,   393,   394,
     395,   396,   397,   398,   399,   400,   403,   406,   407,   408,
     409,   324,   375,   358,   324,   404,   405,   324,   399,   356,
     358,   401,   399,   399,   391,   399,    20,   224,   344,   145,
     176,   363,   364,   365,   153,   178,   207,   208,   212,   213,
     271,   272,    40,    41,    64,   349,   350,   370,   371,   351,
     352,   362,   225,   338,   355,   386,   387,   403,    23,   324,
     378,   408,    23,   324,   355,   356,   324,   351,   391,   402,
     359,   392,   392,   392,   393,   393,   393,   393,   393,   394,
     394,   394,   394,   394,   394,   396,   396,   397,   397,   397,
     397,   397,   398,   398,   398,    43,   391,   407,   225,   386,
     324,   358,   359,   324,   405,   355,   359,   273,   324,   388,
     389,    43,   324,   379,   380,   391,   402,   356,   387,    25,
     100,   355,   390,   388,     4,   331,   383,   355,   359,   324,
     388,   355,   358,   358,    31,   206,   239,   381,   382,   380,
     388,   273,   273,   210,   170,   382,   359,   359
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
#line 584 "sqlparser.y"
    {
//todo: multiple statements
//todo: not only "select" statements
	parser->setOperation(Parser::OP_Select);
	parser->setQuerySchema((yyvsp[(1) - (1)].querySchema));
}
    break;

  case 3:

/* Line 1806 of yacc.c  */
#line 594 "sqlparser.y"
    {
//todo: multiple statements
}
    break;

  case 5:

/* Line 1806 of yacc.c  */
#line 599 "sqlparser.y"
    {
	(yyval.querySchema) = (yyvsp[(1) - (2)].querySchema);
}
    break;

  case 6:

/* Line 1806 of yacc.c  */
#line 609 "sqlparser.y"
    {
YYACCEPT;
}
    break;

  case 7:

/* Line 1806 of yacc.c  */
#line 613 "sqlparser.y"
    {
	(yyval.querySchema) = (yyvsp[(1) - (1)].querySchema);
}
    break;

  case 8:

/* Line 1806 of yacc.c  */
#line 620 "sqlparser.y"
    {
	parser->setOperation(Parser::OP_CreateTable);
	parser->createTable((yyvsp[(3) - (3)].stringValue)->toLatin1());
	delete (yyvsp[(3) - (3)].stringValue);
}
    break;

  case 11:

/* Line 1806 of yacc.c  */
#line 630 "sqlparser.y"
    {
}
    break;

  case 12:

/* Line 1806 of yacc.c  */
#line 636 "sqlparser.y"
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
#line 644 "sqlparser.y"
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
#line 660 "sqlparser.y"
    {
}
    break;

  case 16:

/* Line 1806 of yacc.c  */
#line 666 "sqlparser.y"
    {
	field->setPrimaryKey(true);
	KexiDBDbg << "primary";
}
    break;

  case 17:

/* Line 1806 of yacc.c  */
#line 671 "sqlparser.y"
    {
	field->setNotNull(true);
	KexiDBDbg << "not_null";
}
    break;

  case 18:

/* Line 1806 of yacc.c  */
#line 676 "sqlparser.y"
    {
	field->setAutoIncrement(true);
	KexiDBDbg << "ainc";
}
    break;

  case 19:

/* Line 1806 of yacc.c  */
#line 684 "sqlparser.y"
    {
	field = new Field();
	field->setType((yyvsp[(1) - (1)].colType));
}
    break;

  case 20:

/* Line 1806 of yacc.c  */
#line 689 "sqlparser.y"
    {
	KexiDBDbg << "sql + length";
	field = new Field();
	field->setPrecision((yyvsp[(3) - (4)].integerValue));
	field->setType((yyvsp[(1) - (4)].colType));
}
    break;

  case 21:

/* Line 1806 of yacc.c  */
#line 696 "sqlparser.y"
    {
	field = new Field();
	field->setPrecision((yyvsp[(3) - (4)].integerValue));
	field->setType(Field::Text);
}
    break;

  case 22:

/* Line 1806 of yacc.c  */
#line 702 "sqlparser.y"
    {
	// SQLITE compatibillity
	field = new Field();
	field->setType(Field::InvalidType);
}
    break;

  case 23:

/* Line 1806 of yacc.c  */
#line 711 "sqlparser.y"
    {
	KexiDBDbg << "Select ColViews=" << (yyvsp[(2) - (2)].exprList)->debugString();

	if (!((yyval.querySchema) = buildSelectQuery( (yyvsp[(1) - (2)].querySchema), (yyvsp[(2) - (2)].exprList) )))
		return 0;
}
    break;

  case 24:

/* Line 1806 of yacc.c  */
#line 718 "sqlparser.y"
    {
	if (!((yyval.querySchema) = buildSelectQuery( (yyvsp[(1) - (3)].querySchema), (yyvsp[(2) - (3)].exprList), (yyvsp[(3) - (3)].exprList) )))
		return 0;
}
    break;

  case 25:

/* Line 1806 of yacc.c  */
#line 723 "sqlparser.y"
    {
	KexiDBDbg << "Select ColViews Tables";
	if (!((yyval.querySchema) = buildSelectQuery( (yyvsp[(1) - (2)].querySchema), 0, (yyvsp[(2) - (2)].exprList) )))
		return 0;
}
    break;

  case 26:

/* Line 1806 of yacc.c  */
#line 729 "sqlparser.y"
    {
	KexiDBDbg << "Select ColViews Conditions";
	if (!((yyval.querySchema) = buildSelectQuery( (yyvsp[(1) - (3)].querySchema), (yyvsp[(2) - (3)].exprList), 0, (yyvsp[(3) - (3)].selectOptions) )))
		return 0;
}
    break;

  case 27:

/* Line 1806 of yacc.c  */
#line 735 "sqlparser.y"
    {
	KexiDBDbg << "Select ColViews Tables SelectOptions";
	if (!((yyval.querySchema) = buildSelectQuery( (yyvsp[(1) - (4)].querySchema), (yyvsp[(2) - (4)].exprList), (yyvsp[(3) - (4)].exprList), (yyvsp[(4) - (4)].selectOptions) )))
		return 0;
}
    break;

  case 28:

/* Line 1806 of yacc.c  */
#line 744 "sqlparser.y"
    {
	KexiDBDbg << "SELECT";
//	parser->createSelect();
//	parser->setOperation(Parser::OP_Select);
	(yyval.querySchema) = new QuerySchema();
}
    break;

  case 29:

/* Line 1806 of yacc.c  */
#line 754 "sqlparser.y"
    {
	KexiDBDbg << "WhereClause";
	(yyval.selectOptions) = new SelectOptionsInternal;
	(yyval.selectOptions)->whereExpr = (yyvsp[(1) - (1)].expr);
}
    break;

  case 30:

/* Line 1806 of yacc.c  */
#line 760 "sqlparser.y"
    {
	KexiDBDbg << "OrderByClause";
	(yyval.selectOptions) = new SelectOptionsInternal;
	(yyval.selectOptions)->orderByColumns = (yyvsp[(3) - (3)].orderByColumns);
}
    break;

  case 31:

/* Line 1806 of yacc.c  */
#line 766 "sqlparser.y"
    {
	KexiDBDbg << "WhereClause ORDER BY OrderByClause";
	(yyval.selectOptions) = new SelectOptionsInternal;
	(yyval.selectOptions)->whereExpr = (yyvsp[(1) - (4)].expr);
	(yyval.selectOptions)->orderByColumns = (yyvsp[(4) - (4)].orderByColumns);
}
    break;

  case 32:

/* Line 1806 of yacc.c  */
#line 773 "sqlparser.y"
    {
	KexiDBDbg << "OrderByClause WhereClause";
	(yyval.selectOptions) = new SelectOptionsInternal;
	(yyval.selectOptions)->whereExpr = (yyvsp[(4) - (4)].expr);
	(yyval.selectOptions)->orderByColumns = (yyvsp[(3) - (4)].orderByColumns);
}
    break;

  case 33:

/* Line 1806 of yacc.c  */
#line 783 "sqlparser.y"
    {
	(yyval.expr) = (yyvsp[(2) - (2)].expr);
}
    break;

  case 34:

/* Line 1806 of yacc.c  */
#line 792 "sqlparser.y"
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
#line 801 "sqlparser.y"
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
#line 811 "sqlparser.y"
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
#line 819 "sqlparser.y"
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
#line 831 "sqlparser.y"
    {
	(yyval.variantValue) = new QVariant( *(yyvsp[(1) - (1)].stringValue) );
	KexiDBDbg << "OrderByColumnId: " << *(yyval.variantValue);
	delete (yyvsp[(1) - (1)].stringValue);
}
    break;

  case 39:

/* Line 1806 of yacc.c  */
#line 837 "sqlparser.y"
    {
	(yyval.variantValue) = new QVariant( *(yyvsp[(1) - (3)].stringValue) + "." + *(yyvsp[(3) - (3)].stringValue) );
	KexiDBDbg << "OrderByColumnId: " << *(yyval.variantValue);
	delete (yyvsp[(1) - (3)].stringValue);
	delete (yyvsp[(3) - (3)].stringValue);
}
    break;

  case 40:

/* Line 1806 of yacc.c  */
#line 844 "sqlparser.y"
    {
	(yyval.variantValue) = new QVariant((yyvsp[(1) - (1)].integerValue));
	KexiDBDbg << "OrderByColumnId: " << *(yyval.variantValue);
}
    break;

  case 41:

/* Line 1806 of yacc.c  */
#line 851 "sqlparser.y"
    {
	(yyval.booleanValue) = true;
}
    break;

  case 42:

/* Line 1806 of yacc.c  */
#line 855 "sqlparser.y"
    {
	(yyval.booleanValue) = false;
}
    break;

  case 44:

/* Line 1806 of yacc.c  */
#line 867 "sqlparser.y"
    {
//	KexiDBDbg << "AND " << $3.debugString();
	(yyval.expr) = new BinaryExpr( KexiDBExpr_Logical, (yyvsp[(1) - (3)].expr), AND, (yyvsp[(3) - (3)].expr) );
}
    break;

  case 45:

/* Line 1806 of yacc.c  */
#line 872 "sqlparser.y"
    {
	(yyval.expr) = new BinaryExpr( KexiDBExpr_Logical, (yyvsp[(1) - (3)].expr), OR, (yyvsp[(3) - (3)].expr) );
}
    break;

  case 46:

/* Line 1806 of yacc.c  */
#line 876 "sqlparser.y"
    {
	(yyval.expr) = new BinaryExpr( KexiDBExpr_Arithm, (yyvsp[(1) - (3)].expr), XOR, (yyvsp[(3) - (3)].expr) );
}
    break;

  case 48:

/* Line 1806 of yacc.c  */
#line 886 "sqlparser.y"
    {
	(yyval.expr) = new BinaryExpr(KexiDBExpr_Relational, (yyvsp[(1) - (3)].expr), '>', (yyvsp[(3) - (3)].expr));
}
    break;

  case 49:

/* Line 1806 of yacc.c  */
#line 890 "sqlparser.y"
    {
	(yyval.expr) = new BinaryExpr(KexiDBExpr_Relational, (yyvsp[(1) - (3)].expr), GREATER_OR_EQUAL, (yyvsp[(3) - (3)].expr));
}
    break;

  case 50:

/* Line 1806 of yacc.c  */
#line 894 "sqlparser.y"
    {
	(yyval.expr) = new BinaryExpr(KexiDBExpr_Relational, (yyvsp[(1) - (3)].expr), '<', (yyvsp[(3) - (3)].expr));
}
    break;

  case 51:

/* Line 1806 of yacc.c  */
#line 898 "sqlparser.y"
    {
	(yyval.expr) = new BinaryExpr(KexiDBExpr_Relational, (yyvsp[(1) - (3)].expr), LESS_OR_EQUAL, (yyvsp[(3) - (3)].expr));
}
    break;

  case 52:

/* Line 1806 of yacc.c  */
#line 902 "sqlparser.y"
    {
	(yyval.expr) = new BinaryExpr(KexiDBExpr_Relational, (yyvsp[(1) - (3)].expr), '=', (yyvsp[(3) - (3)].expr));
}
    break;

  case 54:

/* Line 1806 of yacc.c  */
#line 912 "sqlparser.y"
    {
	(yyval.expr) = new BinaryExpr(KexiDBExpr_Relational, (yyvsp[(1) - (3)].expr), NOT_EQUAL, (yyvsp[(3) - (3)].expr));
}
    break;

  case 55:

/* Line 1806 of yacc.c  */
#line 917 "sqlparser.y"
    {
	(yyval.expr) = new BinaryExpr(KexiDBExpr_Relational, (yyvsp[(1) - (3)].expr), NOT_EQUAL2, (yyvsp[(3) - (3)].expr));
}
    break;

  case 56:

/* Line 1806 of yacc.c  */
#line 921 "sqlparser.y"
    {
	(yyval.expr) = new BinaryExpr(KexiDBExpr_Relational, (yyvsp[(1) - (3)].expr), LIKE, (yyvsp[(3) - (3)].expr));
}
    break;

  case 57:

/* Line 1806 of yacc.c  */
#line 925 "sqlparser.y"
    {
	(yyval.expr) = new BinaryExpr(KexiDBExpr_Relational, (yyvsp[(1) - (3)].expr), SQL_IN, (yyvsp[(3) - (3)].expr));
}
    break;

  case 58:

/* Line 1806 of yacc.c  */
#line 929 "sqlparser.y"
    {
	(yyval.expr) = new BinaryExpr(KexiDBExpr_Relational, (yyvsp[(1) - (3)].expr), SIMILAR_TO, (yyvsp[(3) - (3)].expr));
}
    break;

  case 59:

/* Line 1806 of yacc.c  */
#line 933 "sqlparser.y"
    {
	(yyval.expr) = new BinaryExpr(KexiDBExpr_Relational, (yyvsp[(1) - (3)].expr), NOT_SIMILAR_TO, (yyvsp[(3) - (3)].expr));
}
    break;

  case 61:

/* Line 1806 of yacc.c  */
#line 943 "sqlparser.y"
    {
	(yyval.expr) = new UnaryExpr( SQL_IS_NULL, (yyvsp[(1) - (2)].expr) );
}
    break;

  case 62:

/* Line 1806 of yacc.c  */
#line 947 "sqlparser.y"
    {
	(yyval.expr) = new UnaryExpr( SQL_IS_NOT_NULL, (yyvsp[(1) - (2)].expr) );
}
    break;

  case 64:

/* Line 1806 of yacc.c  */
#line 957 "sqlparser.y"
    {
	(yyval.expr) = new BinaryExpr(KexiDBExpr_Arithm, (yyvsp[(1) - (3)].expr), BITWISE_SHIFT_LEFT, (yyvsp[(3) - (3)].expr));
}
    break;

  case 65:

/* Line 1806 of yacc.c  */
#line 961 "sqlparser.y"
    {
	(yyval.expr) = new BinaryExpr(KexiDBExpr_Arithm, (yyvsp[(1) - (3)].expr), BITWISE_SHIFT_RIGHT, (yyvsp[(3) - (3)].expr));
}
    break;

  case 67:

/* Line 1806 of yacc.c  */
#line 971 "sqlparser.y"
    {
	(yyval.expr) = new BinaryExpr(KexiDBExpr_Arithm, (yyvsp[(1) - (3)].expr), '+', (yyvsp[(3) - (3)].expr));
	(yyval.expr)->debug();
}
    break;

  case 68:

/* Line 1806 of yacc.c  */
#line 976 "sqlparser.y"
    {
    (yyval.expr) = new BinaryExpr(KexiDBExpr_Arithm, (yyvsp[(1) - (3)].expr), CONCATENATION, (yyvsp[(3) - (3)].expr));
}
    break;

  case 69:

/* Line 1806 of yacc.c  */
#line 980 "sqlparser.y"
    {
	(yyval.expr) = new BinaryExpr(KexiDBExpr_Arithm, (yyvsp[(1) - (3)].expr), '-', (yyvsp[(3) - (3)].expr));
}
    break;

  case 70:

/* Line 1806 of yacc.c  */
#line 984 "sqlparser.y"
    {
	(yyval.expr) = new BinaryExpr(KexiDBExpr_Arithm, (yyvsp[(1) - (3)].expr), '&', (yyvsp[(3) - (3)].expr));
}
    break;

  case 71:

/* Line 1806 of yacc.c  */
#line 988 "sqlparser.y"
    {
	(yyval.expr) = new BinaryExpr(KexiDBExpr_Arithm, (yyvsp[(1) - (3)].expr), '|', (yyvsp[(3) - (3)].expr));
}
    break;

  case 73:

/* Line 1806 of yacc.c  */
#line 998 "sqlparser.y"
    {
	(yyval.expr) = new BinaryExpr(KexiDBExpr_Arithm, (yyvsp[(1) - (3)].expr), '/', (yyvsp[(3) - (3)].expr));
}
    break;

  case 74:

/* Line 1806 of yacc.c  */
#line 1002 "sqlparser.y"
    {
	(yyval.expr) = new BinaryExpr(KexiDBExpr_Arithm, (yyvsp[(1) - (3)].expr), '*', (yyvsp[(3) - (3)].expr));
}
    break;

  case 75:

/* Line 1806 of yacc.c  */
#line 1006 "sqlparser.y"
    {
	(yyval.expr) = new BinaryExpr(KexiDBExpr_Arithm, (yyvsp[(1) - (3)].expr), '%', (yyvsp[(3) - (3)].expr));
}
    break;

  case 77:

/* Line 1806 of yacc.c  */
#line 1017 "sqlparser.y"
    {
	(yyval.expr) = new UnaryExpr( '-', (yyvsp[(2) - (2)].expr) );
}
    break;

  case 78:

/* Line 1806 of yacc.c  */
#line 1021 "sqlparser.y"
    {
	(yyval.expr) = new UnaryExpr( '+', (yyvsp[(2) - (2)].expr) );
}
    break;

  case 79:

/* Line 1806 of yacc.c  */
#line 1025 "sqlparser.y"
    {
	(yyval.expr) = new UnaryExpr( '~', (yyvsp[(2) - (2)].expr) );
}
    break;

  case 80:

/* Line 1806 of yacc.c  */
#line 1029 "sqlparser.y"
    {
	(yyval.expr) = new UnaryExpr( NOT, (yyvsp[(2) - (2)].expr) );
}
    break;

  case 81:

/* Line 1806 of yacc.c  */
#line 1033 "sqlparser.y"
    {
	(yyval.expr) = new VariableExpr( *(yyvsp[(1) - (1)].stringValue) );
	
//TODO: simplify this later if that's 'only one field name' expression
	KexiDBDbg << "  + identifier: " << *(yyvsp[(1) - (1)].stringValue);
	delete (yyvsp[(1) - (1)].stringValue);
}
    break;

  case 82:

/* Line 1806 of yacc.c  */
#line 1041 "sqlparser.y"
    {
	(yyval.expr) = new QueryParameterExpr( *(yyvsp[(1) - (1)].stringValue) );
	KexiDBDbg << "  + query parameter: " << (yyval.expr)->debugString();
	delete (yyvsp[(1) - (1)].stringValue);
}
    break;

  case 83:

/* Line 1806 of yacc.c  */
#line 1047 "sqlparser.y"
    {
	KexiDBDbg << "  + function: " << *(yyvsp[(1) - (2)].stringValue) << "(" << (yyvsp[(2) - (2)].exprList)->debugString() << ")";
	(yyval.expr) = new FunctionExpr(*(yyvsp[(1) - (2)].stringValue), (yyvsp[(2) - (2)].exprList));
	delete (yyvsp[(1) - (2)].stringValue);
}
    break;

  case 84:

/* Line 1806 of yacc.c  */
#line 1054 "sqlparser.y"
    {
	(yyval.expr) = new VariableExpr( *(yyvsp[(1) - (3)].stringValue) + "." + *(yyvsp[(3) - (3)].stringValue) );
	KexiDBDbg << "  + identifier.identifier: " << *(yyvsp[(1) - (3)].stringValue) << "." << *(yyvsp[(3) - (3)].stringValue);
	delete (yyvsp[(1) - (3)].stringValue);
	delete (yyvsp[(3) - (3)].stringValue);
}
    break;

  case 85:

/* Line 1806 of yacc.c  */
#line 1061 "sqlparser.y"
    {
	(yyval.expr) = new ConstExpr( SQL_NULL, QVariant() );
	KexiDBDbg << "  + NULL";
//	$$ = new Field();
	//$$->setName(QString());
}
    break;

  case 86:

/* Line 1806 of yacc.c  */
#line 1068 "sqlparser.y"
    {
	(yyval.expr) = new ConstExpr( CHARACTER_STRING_LITERAL, *(yyvsp[(1) - (1)].stringValue) );
	KexiDBDbg << "  + constant " << (yyvsp[(1) - (1)].stringValue);
	delete (yyvsp[(1) - (1)].stringValue);
}
    break;

  case 87:

/* Line 1806 of yacc.c  */
#line 1074 "sqlparser.y"
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
#line 1091 "sqlparser.y"
    {
	(yyval.expr) = new ConstExpr( REAL_CONST, QPoint( (yyvsp[(1) - (1)].realValue).integer, (yyvsp[(1) - (1)].realValue).fractional ) );
	KexiDBDbg << "  + real constant: " << (yyvsp[(1) - (1)].realValue).integer << "." << (yyvsp[(1) - (1)].realValue).fractional;
}
    break;

  case 90:

/* Line 1806 of yacc.c  */
#line 1102 "sqlparser.y"
    {
	KexiDBDbg << "(expr)";
	(yyval.expr) = new UnaryExpr('(', (yyvsp[(2) - (3)].expr));
}
    break;

  case 91:

/* Line 1806 of yacc.c  */
#line 1110 "sqlparser.y"
    {
//	$$ = new NArgExpr(0, 0);
//	$$->add( $1 );
//	$$->add( $3 );
	(yyval.exprList) = (yyvsp[(2) - (3)].exprList);
}
    break;

  case 92:

/* Line 1806 of yacc.c  */
#line 1120 "sqlparser.y"
    {
	(yyval.exprList) = (yyvsp[(3) - (3)].exprList);
	(yyval.exprList)->prepend( (yyvsp[(1) - (3)].expr) );
}
    break;

  case 93:

/* Line 1806 of yacc.c  */
#line 1125 "sqlparser.y"
    {
	(yyval.exprList) = new NArgExpr(0, 0);
	(yyval.exprList)->add( (yyvsp[(1) - (3)].expr) );
	(yyval.exprList)->add( (yyvsp[(3) - (3)].expr) );
}
    break;

  case 94:

/* Line 1806 of yacc.c  */
#line 1134 "sqlparser.y"
    {
	(yyval.exprList) = (yyvsp[(2) - (2)].exprList);
}
    break;

  case 95:

/* Line 1806 of yacc.c  */
#line 1179 "sqlparser.y"
    {
	(yyval.exprList) = (yyvsp[(1) - (3)].exprList);
	(yyval.exprList)->add((yyvsp[(3) - (3)].expr));
}
    break;

  case 96:

/* Line 1806 of yacc.c  */
#line 1184 "sqlparser.y"
    {
	(yyval.exprList) = new NArgExpr(KexiDBExpr_TableList, IDENTIFIER); //ok?
	(yyval.exprList)->add((yyvsp[(1) - (1)].expr));
}
    break;

  case 97:

/* Line 1806 of yacc.c  */
#line 1192 "sqlparser.y"
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
#line 1220 "sqlparser.y"
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
#line 1231 "sqlparser.y"
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
#line 1247 "sqlparser.y"
    {
	(yyval.exprList) = (yyvsp[(1) - (3)].exprList);
	(yyval.exprList)->add( (yyvsp[(3) - (3)].expr) );
	KexiDBDbg << "ColViews: ColViews , ColItem";
}
    break;

  case 101:

/* Line 1806 of yacc.c  */
#line 1253 "sqlparser.y"
    {
	(yyval.exprList) = new NArgExpr(0,0);
	(yyval.exprList)->add( (yyvsp[(1) - (1)].expr) );
	KexiDBDbg << "ColViews: ColItem";
}
    break;

  case 102:

/* Line 1806 of yacc.c  */
#line 1262 "sqlparser.y"
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
#line 1271 "sqlparser.y"
    {
	(yyval.expr) = (yyvsp[(1) - (1)].expr);
	KexiDBDbg << " added column wildcard: '" << (yyvsp[(1) - (1)].expr)->debugString() << "'";
}
    break;

  case 104:

/* Line 1806 of yacc.c  */
#line 1276 "sqlparser.y"
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
#line 1285 "sqlparser.y"
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
#line 1297 "sqlparser.y"
    {
	(yyval.expr) = (yyvsp[(1) - (1)].expr);
}
    break;

  case 107:

/* Line 1806 of yacc.c  */
#line 1341 "sqlparser.y"
    {
	(yyval.expr) = (yyvsp[(3) - (4)].expr);
//TODO
//	$$->setName("DISTINCT(" + $3->name() + ")");
}
    break;

  case 108:

/* Line 1806 of yacc.c  */
#line 1350 "sqlparser.y"
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
#line 1359 "sqlparser.y"
    {
	QString s( *(yyvsp[(1) - (3)].stringValue) );
	s += ".*";
	(yyval.expr) = new VariableExpr(s);
	KexiDBDbg << "  + all columns from " << s;
	delete (yyvsp[(1) - (3)].stringValue);
}
    break;



/* Line 1806 of yacc.c  */
#line 3147 "sqlparser.cpp"
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
#line 1374 "sqlparser.y"



const char* tname(int offset) { return yytname[offset]; }
