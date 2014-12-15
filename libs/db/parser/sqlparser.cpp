/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2013 Free Software Foundation, Inc.

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
#define YYBISON_VERSION "3.0.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* Copy the first part of user declarations.  */
#line 422 "sqlparser.y" /* yacc.c:339  */

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


#line 139 "sqlparser.cpp" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "sqlparser.tab.h".  */
#ifndef YY_YY_SQLPARSER_TAB_H_INCLUDED
# define YY_YY_SQLPARSER_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
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
    UNION = 299,
    EXCEPT = 300,
    INTERSECT = 301,
    OR = 302,
    AND = 303,
    XOR = 304,
    NOT = 305,
    GREATER_OR_EQUAL = 306,
    NOT_EQUAL = 307,
    NOT_EQUAL2 = 308,
    SQL_IN = 309,
    LIKE = 310,
    NOT_LIKE = 311,
    ILIKE = 312,
    SIMILAR_TO = 313,
    NOT_SIMILAR_TO = 314,
    SIMILAR = 315,
    BETWEEN = 316,
    UMINUS = 317
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE YYSTYPE;
union YYSTYPE
{
#line 495 "sqlparser.y" /* yacc.c:355  */

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

#line 258 "sqlparser.cpp" /* yacc.c:355  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_SQLPARSER_TAB_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 273 "sqlparser.cpp" /* yacc.c:358  */

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
#else
typedef signed char yytype_int8;
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
# elif ! defined YYSIZE_T
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
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
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
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
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
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
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
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  10
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   188

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  86
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  37
/* YYNRULES -- Number of rules.  */
#define YYNRULES  111
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  184

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   317

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,    52,    47,    83,    56,
      53,    54,    46,    45,    50,    44,    51,    57,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    49,
      66,    65,    67,    55,    48,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    81,     2,    82,    79,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    84,     2,    85,     2,     2,     2,
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
      72,    73,    74,    75,    76,    77,    78,    80
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   583,   583,   593,   597,   598,   608,   612,   620,   619,
     629,   629,   635,   643,   659,   659,   665,   670,   675,   683,
     688,   695,   702,   710,   717,   722,   728,   734,   743,   753,
     759,   765,   772,   782,   791,   800,   810,   818,   830,   836,
     843,   850,   854,   861,   866,   871,   875,   880,   885,   889,
     893,   897,   901,   906,   911,   916,   920,   924,   928,   932,
     936,   940,   949,   954,   958,   963,   968,   972,   977,   982,
     987,   991,   995,   999,  1004,  1009,  1013,  1017,  1022,  1028,
    1032,  1036,  1040,  1044,  1052,  1058,  1065,  1072,  1079,  1085,
    1102,  1108,  1113,  1121,  1131,  1136,  1145,  1190,  1195,  1203,
    1231,  1242,  1258,  1264,  1273,  1282,  1287,  1296,  1308,  1352,
    1361,  1370
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
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
  "';'", "','", "'.'", "'$'", "'('", "')'", "'?'", "'\\''", "'/'", "UNION",
  "EXCEPT", "INTERSECT", "OR", "AND", "XOR", "NOT", "'='", "'<'", "'>'",
  "GREATER_OR_EQUAL", "NOT_EQUAL", "NOT_EQUAL2", "SQL_IN", "LIKE",
  "NOT_LIKE", "ILIKE", "SIMILAR_TO", "NOT_SIMILAR_TO", "SIMILAR",
  "BETWEEN", "'^'", "UMINUS", "'['", "']'", "'&'", "'|'", "'~'", "$accept",
  "TopLevelStatement", "StatementList", "Statement",
  "CreateTableStatement", "$@1", "ColDefs", "ColDef", "ColKeys", "ColKey",
  "ColType", "SelectStatement", "Select", "SelectOptions", "WhereClause",
  "OrderByClause", "OrderByColumnId", "OrderByOption", "aExpr", "aExpr2",
  "aExpr3", "aExpr4", "aExpr5", "aExpr6", "aExpr7", "aExpr8", "aExpr9",
  "aExpr10", "aExprList", "aExprList2", "Tables", "FlatTableList",
  "FlatTable", "ColViews", "ColItem", "ColExpression", "ColWildCard", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,    45,    43,    42,    37,    64,    59,
      44,    46,    36,    40,    41,    63,    39,    47,   299,   300,
     301,   302,   303,   304,   305,    61,    60,    62,   306,   307,
     308,   309,   310,   311,   312,   313,   314,   315,   316,    94,
     317,    91,    93,    38,   124,   126
};
# endif

#define YYPACT_NINF -73

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-73)))

#define YYTABLE_NINF -1

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      -2,   -21,   -73,    42,   -73,     1,   -73,   -73,    -1,    27,
     -73,    -2,   -73,    12,    35,   -73,   -73,   -73,    62,   -73,
      67,    67,   -73,    67,    67,    67,   -73,   -73,    86,    -9,
      68,   -73,    71,    25,   -24,   -73,   -73,    64,   -73,    36,
     -73,   -73,   -73,    38,    57,    60,   -73,    61,    67,   -73,
     100,   -73,   -73,    32,   -73,   -73,    67,    67,    67,    67,
      67,    67,    67,    67,   -73,   -73,    67,    67,    67,    67,
      67,    67,    67,    67,    67,    67,    67,    67,    67,    67,
      67,    67,    67,    67,   109,    67,     2,   -73,   103,    -6,
      93,   -73,    89,    91,   113,   -73,    35,   -73,   -73,   110,
     107,   125,   -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,
     -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,   101,   -73,
     -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,   -73,   -10,
     -73,   -73,   154,   -73,   -73,   128,   -73,   -73,   -73,    67,
     -73,    67,   -73,   115,   126,     4,   -10,    31,    49,   -73,
     110,   -73,   -73,   131,   -73,   -73,   -73,   -10,   119,   -73,
     117,   118,    24,   128,   -73,   -73,   -73,   -10,   143,   144,
     -73,   155,   153,    24,   -73,   -73,   -73,   122,   123,   -73,
     -73,   -73,   -73,   -73
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,    28,     0,     2,     4,     6,     7,     0,     0,
       1,     5,    88,     0,     0,    87,    89,    90,    83,    84,
       0,     0,   110,     0,     0,     0,   108,    43,    47,    53,
      62,    65,    68,    74,    78,    91,    25,    23,   103,   104,
     105,     8,     3,     0,    99,    96,    98,     0,     0,    85,
      83,    79,    80,     0,    82,    81,     0,     0,     0,     0,
       0,     0,     0,     0,    63,    64,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    26,    29,    24,
       0,   107,     0,     0,     0,   100,     0,    86,   111,     0,
       0,     0,    92,    45,    44,    46,    51,    52,    50,    48,
      49,    54,    55,    58,    56,    57,    59,    60,     0,    66,
      67,    70,    71,    69,    72,    73,    76,    77,    75,     0,
      33,   102,     0,    27,   106,     0,   109,   101,    97,     0,
      93,     0,    40,    38,    30,    34,     0,    22,     0,    11,
      95,    94,    61,     0,    32,    41,    42,     0,    35,    31,
      19,     0,    12,     0,     9,    39,    36,     0,     0,     0,
      18,     0,     0,    13,    15,    10,    37,     0,     0,    16,
      17,    14,    20,    21
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
     -73,   -73,   167,   -73,   -73,   -73,   -73,    16,   -73,     7,
     -73,   -73,   -73,    92,    39,   -72,   -73,   -73,   -23,    98,
      65,   -65,   -73,    26,    56,    76,    97,   -73,   -73,    43,
     147,   -73,    90,   -73,    99,   145,   -73
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     3,     4,     5,     6,    92,   148,   149,   173,   174,
     162,     7,     8,    87,    88,   144,   145,   158,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    49,   100,
      36,    45,    46,    37,    38,    39,    40
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
      53,   111,   112,   113,   114,   115,   116,   117,   118,   155,
      12,     1,    59,    12,    13,     9,    14,    13,   156,   142,
      84,    15,    81,    82,    15,    99,     2,   143,    16,    17,
     170,    16,    17,    83,   160,    85,    18,    76,    19,    18,
      90,    19,    10,    20,    21,    22,    20,    21,    22,    12,
      11,   171,    23,    13,   157,    23,    60,    61,    62,    63,
      15,    94,   130,    24,    41,    43,    24,    16,    17,    77,
      78,   161,    44,    91,   159,    50,   152,    19,    12,    74,
      75,    14,    20,    21,    25,   166,   102,    25,   172,    15,
      84,    23,    64,    65,    95,   176,    16,    17,    97,   163,
     119,   120,    24,   164,    50,    85,    19,    98,    79,    80,
      96,    20,    21,    47,    86,    48,   150,    51,    52,   129,
      23,    54,    55,    25,   106,   107,   108,   109,   110,   132,
     134,    24,   121,   122,   123,   124,   125,    66,    67,    68,
      69,    70,   135,    71,    72,   136,    73,    56,    57,    58,
     137,   101,    25,    48,   103,   104,   105,   126,   127,   128,
     139,   140,    97,   141,   146,   147,   153,    85,   165,   167,
     168,   169,   177,   178,   179,   180,   182,   183,    42,   175,
     181,   133,   151,   154,    89,   131,   138,     0,    93
};

static const yytype_int16 yycheck[] =
{
      23,    66,    67,    68,    69,    70,    71,    72,    73,     5,
      11,    13,    21,    11,    15,    36,    17,    15,    14,    29,
      26,    22,    46,    47,    22,    48,    28,    37,    29,    30,
       6,    29,    30,    57,     3,    41,    37,    12,    39,    37,
       4,    39,     0,    44,    45,    46,    44,    45,    46,    11,
      49,    27,    53,    15,    50,    53,    65,    66,    67,    68,
      22,     4,    85,    64,    37,    53,    64,    29,    30,    44,
      45,    40,    37,    37,   146,    37,   141,    39,    11,     8,
       9,    17,    44,    45,    85,   157,    54,    85,    64,    22,
      26,    53,    24,    25,    37,   167,    29,    30,    37,    50,
      74,    75,    64,    54,    37,    41,    39,    46,    83,    84,
      50,    44,    45,    51,    50,    53,   139,    20,    21,    10,
      53,    24,    25,    85,    59,    60,    61,    62,    63,    26,
      37,    64,    76,    77,    78,    79,    80,    69,    70,    71,
      72,    73,    53,    75,    76,    54,    78,    61,    62,    63,
      37,    51,    85,    53,    56,    57,    58,    81,    82,    83,
      50,    54,    37,    62,    10,    37,    51,    41,    37,    50,
      53,    53,    29,    29,    19,    22,    54,    54,    11,   163,
     173,    89,   139,   144,    37,    86,    96,    -1,    43
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    13,    28,    87,    88,    89,    90,    97,    98,    36,
       0,    49,    11,    15,    17,    22,    29,    30,    37,    39,
      44,    45,    46,    53,    64,    85,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   113,   116,   119,   120,   121,
     122,    37,    88,    53,    37,   117,   118,    51,    53,   114,
      37,   112,   112,   104,   112,   112,    61,    62,    63,    21,
      65,    66,    67,    68,    24,    25,    69,    70,    71,    72,
      73,    75,    76,    78,     8,     9,    12,    44,    45,    83,
      84,    46,    47,    57,    26,    41,    50,    99,   100,   116,
       4,    37,    91,   121,     4,    37,    50,    37,    46,   104,
     115,    51,    54,   105,   105,   105,   106,   106,   106,   106,
     106,   107,   107,   107,   107,   107,   107,   107,   107,   109,
     109,   110,   110,   110,   110,   110,   111,   111,   111,    10,
     104,   120,    26,    99,    37,    53,    54,    37,   118,    50,
      54,    62,    29,    37,   101,   102,    10,    37,    92,    93,
     104,   115,   107,    51,   100,     5,    14,    50,   103,   101,
       3,    40,    96,    50,    54,    37,   101,    50,    53,    53,
       6,    27,    64,    94,    95,    93,   101,    29,    29,    19,
      22,    95,    54,    54
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    86,    87,    88,    88,    88,    89,    89,    91,    90,
      92,    92,    93,    93,    94,    94,    95,    95,    95,    96,
      96,    96,    96,    97,    97,    97,    97,    97,    98,    99,
      99,    99,    99,   100,   101,   101,   101,   101,   102,   102,
     102,   103,   103,   104,   105,   105,   105,   105,   106,   106,
     106,   106,   106,   106,   107,   107,   107,   107,   107,   107,
     107,   107,   107,   108,   108,   108,   109,   109,   109,   110,
     110,   110,   110,   110,   110,   111,   111,   111,   111,   112,
     112,   112,   112,   112,   112,   112,   112,   112,   112,   112,
     112,   112,   113,   114,   115,   115,   116,   117,   117,   118,
     118,   118,   119,   119,   120,   120,   120,   120,   121,   121,
     122,   122
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     3,     1,     2,     1,     1,     0,     7,
       3,     1,     2,     3,     2,     1,     2,     2,     1,     1,
       4,     4,     0,     2,     3,     2,     3,     4,     1,     1,
       3,     4,     4,     2,     1,     2,     3,     4,     1,     3,
       1,     1,     1,     1,     3,     3,     3,     1,     3,     3,
       3,     3,     3,     1,     3,     3,     3,     3,     3,     3,
       3,     5,     1,     2,     2,     1,     3,     3,     1,     3,
       3,     3,     3,     3,     1,     3,     3,     3,     1,     2,
       2,     2,     2,     1,     1,     2,     3,     1,     1,     1,
       1,     1,     3,     3,     3,     3,     2,     3,     1,     1,
       2,     3,     3,     1,     1,     1,     3,     2,     1,     4,
       1,     3
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                                              );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

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
#ifndef YYINITDEPTH
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
static YYSIZE_T
yystrlen (const char *yystr)
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
static char *
yystpcpy (char *yydest, const char *yysrc)
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
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
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
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
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

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

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

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
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
  int yytoken = 0;
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

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
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
      yychar = yylex ();
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
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

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
     '$$ = $1'.

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
#line 584 "sqlparser.y" /* yacc.c:1646  */
    {
//todo: multiple statements
//todo: not only "select" statements
	parser->setOperation(Parser::OP_Select);
	parser->setQuerySchema((yyvsp[0].querySchema));
}
#line 1505 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 3:
#line 594 "sqlparser.y" /* yacc.c:1646  */
    {
//todo: multiple statements
}
#line 1513 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 5:
#line 599 "sqlparser.y" /* yacc.c:1646  */
    {
	(yyval.querySchema) = (yyvsp[-1].querySchema);
}
#line 1521 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 6:
#line 609 "sqlparser.y" /* yacc.c:1646  */
    {
YYACCEPT;
}
#line 1529 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 7:
#line 613 "sqlparser.y" /* yacc.c:1646  */
    {
	(yyval.querySchema) = (yyvsp[0].querySchema);
}
#line 1537 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 8:
#line 620 "sqlparser.y" /* yacc.c:1646  */
    {
	parser->setOperation(Parser::OP_CreateTable);
	parser->createTable((yyvsp[0].stringValue)->toLatin1());
	delete (yyvsp[0].stringValue);
}
#line 1547 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 11:
#line 630 "sqlparser.y" /* yacc.c:1646  */
    {
}
#line 1554 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 12:
#line 636 "sqlparser.y" /* yacc.c:1646  */
    {
	KexiDBDbg << "adding field " << *(yyvsp[-1].stringValue);
	field->setName((yyvsp[-1].stringValue)->toLatin1());
	parser->table()->addField(field);
	field = 0;
	delete (yyvsp[-1].stringValue);
}
#line 1566 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 13:
#line 644 "sqlparser.y" /* yacc.c:1646  */
    {
	KexiDBDbg << "adding field " << *(yyvsp[-2].stringValue);
	field->setName(*(yyvsp[-2].stringValue));
	delete (yyvsp[-2].stringValue);
	parser->table()->addField(field);

//	if(field->isPrimaryKey())
//		parser->table()->addPrimaryKey(field->name());

//	delete field;
//	field = 0;
}
#line 1583 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 15:
#line 660 "sqlparser.y" /* yacc.c:1646  */
    {
}
#line 1590 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 16:
#line 666 "sqlparser.y" /* yacc.c:1646  */
    {
	field->setPrimaryKey(true);
	KexiDBDbg << "primary";
}
#line 1599 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 17:
#line 671 "sqlparser.y" /* yacc.c:1646  */
    {
	field->setNotNull(true);
	KexiDBDbg << "not_null";
}
#line 1608 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 18:
#line 676 "sqlparser.y" /* yacc.c:1646  */
    {
	field->setAutoIncrement(true);
	KexiDBDbg << "ainc";
}
#line 1617 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 19:
#line 684 "sqlparser.y" /* yacc.c:1646  */
    {
	field = new Field();
	field->setType((yyvsp[0].colType));
}
#line 1626 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 20:
#line 689 "sqlparser.y" /* yacc.c:1646  */
    {
	KexiDBDbg << "sql + length";
	field = new Field();
	field->setPrecision((yyvsp[-1].integerValue));
	field->setType((yyvsp[-3].colType));
}
#line 1637 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 21:
#line 696 "sqlparser.y" /* yacc.c:1646  */
    {
	field = new Field();
	field->setPrecision((yyvsp[-1].integerValue));
	field->setType(Field::Text);
}
#line 1647 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 22:
#line 702 "sqlparser.y" /* yacc.c:1646  */
    {
	// SQLITE compatibillity
	field = new Field();
	field->setType(Field::InvalidType);
}
#line 1657 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 23:
#line 711 "sqlparser.y" /* yacc.c:1646  */
    {
	KexiDBDbg << "Select ColViews=" << (yyvsp[0].exprList)->debugString();

	if (!((yyval.querySchema) = buildSelectQuery( (yyvsp[-1].querySchema), (yyvsp[0].exprList) )))
		return 0;
}
#line 1668 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 24:
#line 718 "sqlparser.y" /* yacc.c:1646  */
    {
	if (!((yyval.querySchema) = buildSelectQuery( (yyvsp[-2].querySchema), (yyvsp[-1].exprList), (yyvsp[0].exprList) )))
		return 0;
}
#line 1677 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 25:
#line 723 "sqlparser.y" /* yacc.c:1646  */
    {
	KexiDBDbg << "Select ColViews Tables";
	if (!((yyval.querySchema) = buildSelectQuery( (yyvsp[-1].querySchema), 0, (yyvsp[0].exprList) )))
		return 0;
}
#line 1687 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 26:
#line 729 "sqlparser.y" /* yacc.c:1646  */
    {
	KexiDBDbg << "Select ColViews Conditions";
	if (!((yyval.querySchema) = buildSelectQuery( (yyvsp[-2].querySchema), (yyvsp[-1].exprList), 0, (yyvsp[0].selectOptions) )))
		return 0;
}
#line 1697 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 27:
#line 735 "sqlparser.y" /* yacc.c:1646  */
    {
	KexiDBDbg << "Select ColViews Tables SelectOptions";
	if (!((yyval.querySchema) = buildSelectQuery( (yyvsp[-3].querySchema), (yyvsp[-2].exprList), (yyvsp[-1].exprList), (yyvsp[0].selectOptions) )))
		return 0;
}
#line 1707 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 28:
#line 744 "sqlparser.y" /* yacc.c:1646  */
    {
	KexiDBDbg << "SELECT";
//	parser->createSelect();
//	parser->setOperation(Parser::OP_Select);
	(yyval.querySchema) = new QuerySchema();
}
#line 1718 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 29:
#line 754 "sqlparser.y" /* yacc.c:1646  */
    {
	KexiDBDbg << "WhereClause";
	(yyval.selectOptions) = new SelectOptionsInternal;
	(yyval.selectOptions)->whereExpr = (yyvsp[0].expr);
}
#line 1728 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 30:
#line 760 "sqlparser.y" /* yacc.c:1646  */
    {
	KexiDBDbg << "OrderByClause";
	(yyval.selectOptions) = new SelectOptionsInternal;
	(yyval.selectOptions)->orderByColumns = (yyvsp[0].orderByColumns);
}
#line 1738 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 31:
#line 766 "sqlparser.y" /* yacc.c:1646  */
    {
	KexiDBDbg << "WhereClause ORDER BY OrderByClause";
	(yyval.selectOptions) = new SelectOptionsInternal;
	(yyval.selectOptions)->whereExpr = (yyvsp[-3].expr);
	(yyval.selectOptions)->orderByColumns = (yyvsp[0].orderByColumns);
}
#line 1749 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 32:
#line 773 "sqlparser.y" /* yacc.c:1646  */
    {
	KexiDBDbg << "OrderByClause WhereClause";
	(yyval.selectOptions) = new SelectOptionsInternal;
	(yyval.selectOptions)->whereExpr = (yyvsp[0].expr);
	(yyval.selectOptions)->orderByColumns = (yyvsp[-1].orderByColumns);
}
#line 1760 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 33:
#line 783 "sqlparser.y" /* yacc.c:1646  */
    {
	(yyval.expr) = (yyvsp[0].expr);
}
#line 1768 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 34:
#line 792 "sqlparser.y" /* yacc.c:1646  */
    {
	KexiDBDbg << "ORDER BY IDENTIFIER";
	(yyval.orderByColumns) = new OrderByColumnInternal::List;
	OrderByColumnInternal orderByColumn;
	orderByColumn.setColumnByNameOrNumber( *(yyvsp[0].variantValue) );
	(yyval.orderByColumns)->append( orderByColumn );
	delete (yyvsp[0].variantValue);
}
#line 1781 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 35:
#line 801 "sqlparser.y" /* yacc.c:1646  */
    {
	KexiDBDbg << "ORDER BY IDENTIFIER OrderByOption";
	(yyval.orderByColumns) = new OrderByColumnInternal::List;
	OrderByColumnInternal orderByColumn;
	orderByColumn.setColumnByNameOrNumber( *(yyvsp[-1].variantValue) );
	orderByColumn.ascending = (yyvsp[0].booleanValue);
	(yyval.orderByColumns)->append( orderByColumn );
	delete (yyvsp[-1].variantValue);
}
#line 1795 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 36:
#line 811 "sqlparser.y" /* yacc.c:1646  */
    {
	(yyval.orderByColumns) = (yyvsp[0].orderByColumns);
	OrderByColumnInternal orderByColumn;
	orderByColumn.setColumnByNameOrNumber( *(yyvsp[-2].variantValue) );
	(yyval.orderByColumns)->append( orderByColumn );
	delete (yyvsp[-2].variantValue);
}
#line 1807 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 37:
#line 819 "sqlparser.y" /* yacc.c:1646  */
    {
	(yyval.orderByColumns) = (yyvsp[0].orderByColumns);
	OrderByColumnInternal orderByColumn;
	orderByColumn.setColumnByNameOrNumber( *(yyvsp[-3].variantValue) );
	orderByColumn.ascending = (yyvsp[-2].booleanValue);
	(yyval.orderByColumns)->append( orderByColumn );
	delete (yyvsp[-3].variantValue);
}
#line 1820 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 38:
#line 831 "sqlparser.y" /* yacc.c:1646  */
    {
	(yyval.variantValue) = new QVariant( *(yyvsp[0].stringValue) );
	KexiDBDbg << "OrderByColumnId: " << *(yyval.variantValue);
	delete (yyvsp[0].stringValue);
}
#line 1830 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 39:
#line 837 "sqlparser.y" /* yacc.c:1646  */
    {
	(yyval.variantValue) = new QVariant( *(yyvsp[-2].stringValue) + "." + *(yyvsp[0].stringValue) );
	KexiDBDbg << "OrderByColumnId: " << *(yyval.variantValue);
	delete (yyvsp[-2].stringValue);
	delete (yyvsp[0].stringValue);
}
#line 1841 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 40:
#line 844 "sqlparser.y" /* yacc.c:1646  */
    {
	(yyval.variantValue) = new QVariant((yyvsp[0].integerValue));
	KexiDBDbg << "OrderByColumnId: " << *(yyval.variantValue);
}
#line 1850 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 41:
#line 851 "sqlparser.y" /* yacc.c:1646  */
    {
	(yyval.booleanValue) = true;
}
#line 1858 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 42:
#line 855 "sqlparser.y" /* yacc.c:1646  */
    {
	(yyval.booleanValue) = false;
}
#line 1866 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 44:
#line 867 "sqlparser.y" /* yacc.c:1646  */
    {
//	KexiDBDbg << "AND " << $3.debugString();
	(yyval.expr) = new BinaryExpr( KexiDBExpr_Logical, (yyvsp[-2].expr), AND, (yyvsp[0].expr) );
}
#line 1875 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 45:
#line 872 "sqlparser.y" /* yacc.c:1646  */
    {
	(yyval.expr) = new BinaryExpr( KexiDBExpr_Logical, (yyvsp[-2].expr), OR, (yyvsp[0].expr) );
}
#line 1883 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 46:
#line 876 "sqlparser.y" /* yacc.c:1646  */
    {
	(yyval.expr) = new BinaryExpr( KexiDBExpr_Arithm, (yyvsp[-2].expr), XOR, (yyvsp[0].expr) );
}
#line 1891 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 48:
#line 886 "sqlparser.y" /* yacc.c:1646  */
    {
	(yyval.expr) = new BinaryExpr(KexiDBExpr_Relational, (yyvsp[-2].expr), '>', (yyvsp[0].expr));
}
#line 1899 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 49:
#line 890 "sqlparser.y" /* yacc.c:1646  */
    {
	(yyval.expr) = new BinaryExpr(KexiDBExpr_Relational, (yyvsp[-2].expr), GREATER_OR_EQUAL, (yyvsp[0].expr));
}
#line 1907 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 50:
#line 894 "sqlparser.y" /* yacc.c:1646  */
    {
	(yyval.expr) = new BinaryExpr(KexiDBExpr_Relational, (yyvsp[-2].expr), '<', (yyvsp[0].expr));
}
#line 1915 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 51:
#line 898 "sqlparser.y" /* yacc.c:1646  */
    {
	(yyval.expr) = new BinaryExpr(KexiDBExpr_Relational, (yyvsp[-2].expr), LESS_OR_EQUAL, (yyvsp[0].expr));
}
#line 1923 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 52:
#line 902 "sqlparser.y" /* yacc.c:1646  */
    {
	(yyval.expr) = new BinaryExpr(KexiDBExpr_Relational, (yyvsp[-2].expr), '=', (yyvsp[0].expr));
}
#line 1931 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 54:
#line 912 "sqlparser.y" /* yacc.c:1646  */
    {
	(yyval.expr) = new BinaryExpr(KexiDBExpr_Relational, (yyvsp[-2].expr), NOT_EQUAL, (yyvsp[0].expr));
}
#line 1939 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 55:
#line 917 "sqlparser.y" /* yacc.c:1646  */
    {
	(yyval.expr) = new BinaryExpr(KexiDBExpr_Relational, (yyvsp[-2].expr), NOT_EQUAL2, (yyvsp[0].expr));
}
#line 1947 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 56:
#line 921 "sqlparser.y" /* yacc.c:1646  */
    {
	(yyval.expr) = new BinaryExpr(KexiDBExpr_Relational, (yyvsp[-2].expr), LIKE, (yyvsp[0].expr));
}
#line 1955 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 57:
#line 925 "sqlparser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new BinaryExpr(KexiDBExpr_Relational, (yyvsp[-2].expr), NOT_LIKE, (yyvsp[0].expr));
}
#line 1963 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 58:
#line 929 "sqlparser.y" /* yacc.c:1646  */
    {
	(yyval.expr) = new BinaryExpr(KexiDBExpr_Relational, (yyvsp[-2].expr), SQL_IN, (yyvsp[0].expr));
}
#line 1971 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 59:
#line 933 "sqlparser.y" /* yacc.c:1646  */
    {
	(yyval.expr) = new BinaryExpr(KexiDBExpr_Relational, (yyvsp[-2].expr), SIMILAR_TO, (yyvsp[0].expr));
}
#line 1979 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 60:
#line 937 "sqlparser.y" /* yacc.c:1646  */
    {
	(yyval.expr) = new BinaryExpr(KexiDBExpr_Relational, (yyvsp[-2].expr), NOT_SIMILAR_TO, (yyvsp[0].expr));
}
#line 1987 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 61:
#line 941 "sqlparser.y" /* yacc.c:1646  */
    {
	(yyval.expr) = new NArgExpr(KexiDBExpr_Relational, KEXIDB_TOKEN_BETWEEN_AND);

	(yyval.expr)->toNArg()->add( (yyvsp[-4].expr) );
	(yyval.expr)->toNArg()->add( (yyvsp[-2].expr) );
	(yyval.expr)->toNArg()->add( (yyvsp[0].expr) );
}
#line 1999 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 63:
#line 955 "sqlparser.y" /* yacc.c:1646  */
    {
	(yyval.expr) = new UnaryExpr( SQL_IS_NULL, (yyvsp[-1].expr) );
}
#line 2007 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 64:
#line 959 "sqlparser.y" /* yacc.c:1646  */
    {
	(yyval.expr) = new UnaryExpr( SQL_IS_NOT_NULL, (yyvsp[-1].expr) );
}
#line 2015 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 66:
#line 969 "sqlparser.y" /* yacc.c:1646  */
    {
	(yyval.expr) = new BinaryExpr(KexiDBExpr_Arithm, (yyvsp[-2].expr), BITWISE_SHIFT_LEFT, (yyvsp[0].expr));
}
#line 2023 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 67:
#line 973 "sqlparser.y" /* yacc.c:1646  */
    {
	(yyval.expr) = new BinaryExpr(KexiDBExpr_Arithm, (yyvsp[-2].expr), BITWISE_SHIFT_RIGHT, (yyvsp[0].expr));
}
#line 2031 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 69:
#line 983 "sqlparser.y" /* yacc.c:1646  */
    {
	(yyval.expr) = new BinaryExpr(KexiDBExpr_Arithm, (yyvsp[-2].expr), '+', (yyvsp[0].expr));
	(yyval.expr)->debug();
}
#line 2040 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 70:
#line 988 "sqlparser.y" /* yacc.c:1646  */
    {
    (yyval.expr) = new BinaryExpr(KexiDBExpr_Arithm, (yyvsp[-2].expr), CONCATENATION, (yyvsp[0].expr));
}
#line 2048 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 71:
#line 992 "sqlparser.y" /* yacc.c:1646  */
    {
	(yyval.expr) = new BinaryExpr(KexiDBExpr_Arithm, (yyvsp[-2].expr), '-', (yyvsp[0].expr));
}
#line 2056 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 72:
#line 996 "sqlparser.y" /* yacc.c:1646  */
    {
	(yyval.expr) = new BinaryExpr(KexiDBExpr_Arithm, (yyvsp[-2].expr), '&', (yyvsp[0].expr));
}
#line 2064 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 73:
#line 1000 "sqlparser.y" /* yacc.c:1646  */
    {
	(yyval.expr) = new BinaryExpr(KexiDBExpr_Arithm, (yyvsp[-2].expr), '|', (yyvsp[0].expr));
}
#line 2072 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 75:
#line 1010 "sqlparser.y" /* yacc.c:1646  */
    {
	(yyval.expr) = new BinaryExpr(KexiDBExpr_Arithm, (yyvsp[-2].expr), '/', (yyvsp[0].expr));
}
#line 2080 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 76:
#line 1014 "sqlparser.y" /* yacc.c:1646  */
    {
	(yyval.expr) = new BinaryExpr(KexiDBExpr_Arithm, (yyvsp[-2].expr), '*', (yyvsp[0].expr));
}
#line 2088 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 77:
#line 1018 "sqlparser.y" /* yacc.c:1646  */
    {
	(yyval.expr) = new BinaryExpr(KexiDBExpr_Arithm, (yyvsp[-2].expr), '%', (yyvsp[0].expr));
}
#line 2096 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 79:
#line 1029 "sqlparser.y" /* yacc.c:1646  */
    {
	(yyval.expr) = new UnaryExpr( '-', (yyvsp[0].expr) );
}
#line 2104 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 80:
#line 1033 "sqlparser.y" /* yacc.c:1646  */
    {
	(yyval.expr) = new UnaryExpr( '+', (yyvsp[0].expr) );
}
#line 2112 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 81:
#line 1037 "sqlparser.y" /* yacc.c:1646  */
    {
	(yyval.expr) = new UnaryExpr( '~', (yyvsp[0].expr) );
}
#line 2120 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 82:
#line 1041 "sqlparser.y" /* yacc.c:1646  */
    {
	(yyval.expr) = new UnaryExpr( NOT, (yyvsp[0].expr) );
}
#line 2128 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 83:
#line 1045 "sqlparser.y" /* yacc.c:1646  */
    {
	(yyval.expr) = new VariableExpr( *(yyvsp[0].stringValue) );
	
//TODO: simplify this later if that's 'only one field name' expression
	KexiDBDbg << "  + identifier: " << *(yyvsp[0].stringValue);
	delete (yyvsp[0].stringValue);
}
#line 2140 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 84:
#line 1053 "sqlparser.y" /* yacc.c:1646  */
    {
	(yyval.expr) = new QueryParameterExpr( *(yyvsp[0].stringValue) );
	KexiDBDbg << "  + query parameter: " << (yyval.expr)->debugString();
	delete (yyvsp[0].stringValue);
}
#line 2150 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 85:
#line 1059 "sqlparser.y" /* yacc.c:1646  */
    {
	KexiDBDbg << "  + function: " << *(yyvsp[-1].stringValue) << "(" << (yyvsp[0].exprList)->debugString() << ")";
	(yyval.expr) = new FunctionExpr(*(yyvsp[-1].stringValue), (yyvsp[0].exprList));
	delete (yyvsp[-1].stringValue);
}
#line 2160 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 86:
#line 1066 "sqlparser.y" /* yacc.c:1646  */
    {
	(yyval.expr) = new VariableExpr( *(yyvsp[-2].stringValue) + "." + *(yyvsp[0].stringValue) );
	KexiDBDbg << "  + identifier.identifier: " << *(yyvsp[-2].stringValue) << "." << *(yyvsp[0].stringValue);
	delete (yyvsp[-2].stringValue);
	delete (yyvsp[0].stringValue);
}
#line 2171 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 87:
#line 1073 "sqlparser.y" /* yacc.c:1646  */
    {
	(yyval.expr) = new ConstExpr( SQL_NULL, QVariant() );
	KexiDBDbg << "  + NULL";
//	$$ = new Field();
	//$$->setName(QString());
}
#line 2182 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 88:
#line 1080 "sqlparser.y" /* yacc.c:1646  */
    {
	(yyval.expr) = new ConstExpr( CHARACTER_STRING_LITERAL, *(yyvsp[0].stringValue) );
	KexiDBDbg << "  + constant " << (yyvsp[0].stringValue);
	delete (yyvsp[0].stringValue);
}
#line 2192 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 89:
#line 1086 "sqlparser.y" /* yacc.c:1646  */
    {
	QVariant val;
	if ((yyvsp[0].integerValue) <= INT_MAX && (yyvsp[0].integerValue) >= INT_MIN)
		val = (int)(yyvsp[0].integerValue);
	else if ((yyvsp[0].integerValue) <= UINT_MAX && (yyvsp[0].integerValue) >= 0)
		val = (uint)(yyvsp[0].integerValue);
	else if ((yyvsp[0].integerValue) <= LLONG_MAX && (yyvsp[0].integerValue) >= LLONG_MIN)
		val = (qint64)(yyvsp[0].integerValue);

//	if ($1 < ULLONG_MAX)
//		val = (quint64)$1;
//TODO ok?

	(yyval.expr) = new ConstExpr( INTEGER_CONST, val );
	KexiDBDbg << "  + int constant: " << val.toString();
}
#line 2213 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 90:
#line 1103 "sqlparser.y" /* yacc.c:1646  */
    {
	(yyval.expr) = new ConstExpr( REAL_CONST, QPoint( (yyvsp[0].realValue).integer, (yyvsp[0].realValue).fractional ) );
	KexiDBDbg << "  + real constant: " << (yyvsp[0].realValue).integer << "." << (yyvsp[0].realValue).fractional;
}
#line 2222 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 92:
#line 1114 "sqlparser.y" /* yacc.c:1646  */
    {
	KexiDBDbg << "(expr)";
	(yyval.expr) = new UnaryExpr('(', (yyvsp[-1].expr));
}
#line 2231 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 93:
#line 1122 "sqlparser.y" /* yacc.c:1646  */
    {
//	$$ = new NArgExpr(0, 0);
//	$$->add( $1 );
//	$$->add( $3 );
	(yyval.exprList) = (yyvsp[-1].exprList);
}
#line 2242 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 94:
#line 1132 "sqlparser.y" /* yacc.c:1646  */
    {
	(yyval.exprList) = (yyvsp[0].exprList);
	(yyval.exprList)->prepend( (yyvsp[-2].expr) );
}
#line 2251 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 95:
#line 1137 "sqlparser.y" /* yacc.c:1646  */
    {
	(yyval.exprList) = new NArgExpr(0, 0);
	(yyval.exprList)->add( (yyvsp[-2].expr) );
	(yyval.exprList)->add( (yyvsp[0].expr) );
}
#line 2261 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 96:
#line 1146 "sqlparser.y" /* yacc.c:1646  */
    {
	(yyval.exprList) = (yyvsp[0].exprList);
}
#line 2269 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 97:
#line 1191 "sqlparser.y" /* yacc.c:1646  */
    {
	(yyval.exprList) = (yyvsp[-2].exprList);
	(yyval.exprList)->add((yyvsp[0].expr));
}
#line 2278 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 98:
#line 1196 "sqlparser.y" /* yacc.c:1646  */
    {
	(yyval.exprList) = new NArgExpr(KexiDBExpr_TableList, IDENTIFIER); //ok?
	(yyval.exprList)->add((yyvsp[0].expr));
}
#line 2287 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 99:
#line 1204 "sqlparser.y" /* yacc.c:1646  */
    {
	KexiDBDbg << "FROM: '" << *(yyvsp[0].stringValue) << "'";
	(yyval.expr) = new VariableExpr(*(yyvsp[0].stringValue));

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
	delete (yyvsp[0].stringValue);
}
#line 2319 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 100:
#line 1232 "sqlparser.y" /* yacc.c:1646  */
    {
	//table + alias
	(yyval.expr) = new BinaryExpr(
		KexiDBExpr_SpecialBinary, 
		new VariableExpr(*(yyvsp[-1].stringValue)), 0,
		new VariableExpr(*(yyvsp[0].stringValue))
	);
	delete (yyvsp[-1].stringValue);
	delete (yyvsp[0].stringValue);
}
#line 2334 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 101:
#line 1243 "sqlparser.y" /* yacc.c:1646  */
    {
	//table + alias
	(yyval.expr) = new BinaryExpr(
		KexiDBExpr_SpecialBinary,
		new VariableExpr(*(yyvsp[-2].stringValue)), AS,
		new VariableExpr(*(yyvsp[0].stringValue))
	);
	delete (yyvsp[-2].stringValue);
	delete (yyvsp[0].stringValue);
}
#line 2349 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 102:
#line 1259 "sqlparser.y" /* yacc.c:1646  */
    {
	(yyval.exprList) = (yyvsp[-2].exprList);
	(yyval.exprList)->add( (yyvsp[0].expr) );
	KexiDBDbg << "ColViews: ColViews , ColItem";
}
#line 2359 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 103:
#line 1265 "sqlparser.y" /* yacc.c:1646  */
    {
	(yyval.exprList) = new NArgExpr(0,0);
	(yyval.exprList)->add( (yyvsp[0].expr) );
	KexiDBDbg << "ColViews: ColItem";
}
#line 2369 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 104:
#line 1274 "sqlparser.y" /* yacc.c:1646  */
    {
//	$$ = new Field();
//	dummy->addField($$);
//	$$->setExpression( $1 );
//	parser->select()->addField($$);
	(yyval.expr) = (yyvsp[0].expr);
	KexiDBDbg << " added column expr: '" << (yyvsp[0].expr)->debugString() << "'";
}
#line 2382 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 105:
#line 1283 "sqlparser.y" /* yacc.c:1646  */
    {
	(yyval.expr) = (yyvsp[0].expr);
	KexiDBDbg << " added column wildcard: '" << (yyvsp[0].expr)->debugString() << "'";
}
#line 2391 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 106:
#line 1288 "sqlparser.y" /* yacc.c:1646  */
    {
	(yyval.expr) = new BinaryExpr(
		KexiDBExpr_SpecialBinary, (yyvsp[-2].expr), AS,
		new VariableExpr(*(yyvsp[0].stringValue))
	);
	KexiDBDbg << " added column expr: " << (yyval.expr)->debugString();
	delete (yyvsp[0].stringValue);
}
#line 2404 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 107:
#line 1297 "sqlparser.y" /* yacc.c:1646  */
    {
	(yyval.expr) = new BinaryExpr(
		KexiDBExpr_SpecialBinary, (yyvsp[-1].expr), 0, 
		new VariableExpr(*(yyvsp[0].stringValue))
	);
	KexiDBDbg << " added column expr: " << (yyval.expr)->debugString();
	delete (yyvsp[0].stringValue);
}
#line 2417 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 108:
#line 1309 "sqlparser.y" /* yacc.c:1646  */
    {
	(yyval.expr) = (yyvsp[0].expr);
}
#line 2425 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 109:
#line 1353 "sqlparser.y" /* yacc.c:1646  */
    {
	(yyval.expr) = (yyvsp[-1].expr);
//TODO
//	$$->setName("DISTINCT(" + $3->name() + ")");
}
#line 2435 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 110:
#line 1362 "sqlparser.y" /* yacc.c:1646  */
    {
	(yyval.expr) = new VariableExpr("*");
	KexiDBDbg << "all columns";

//	QueryAsterisk *ast = new QueryAsterisk(parser->select(), dummy);
//	parser->select()->addAsterisk(ast);
//	requiresTable = true;
}
#line 2448 "sqlparser.cpp" /* yacc.c:1646  */
    break;

  case 111:
#line 1371 "sqlparser.y" /* yacc.c:1646  */
    {
	QString s( *(yyvsp[-2].stringValue) );
	s += ".*";
	(yyval.expr) = new VariableExpr(s);
	KexiDBDbg << "  + all columns from " << s;
	delete (yyvsp[-2].stringValue);
}
#line 2460 "sqlparser.cpp" /* yacc.c:1646  */
    break;


#line 2464 "sqlparser.cpp" /* yacc.c:1646  */
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

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
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

  /* Do not reclaim the symbols of the rule whose action triggered
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
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

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

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


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

#if !defined yyoverflow || YYERROR_VERBOSE
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
  /* Do not reclaim the symbols of the rule whose action triggered
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
  return yyresult;
}
#line 1386 "sqlparser.y" /* yacc.c:1906  */


const char* tname(int offset) { return yytname[offset]; }
