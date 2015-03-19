/*             itex2MML 1.4.10
 *   itex2MML.l last modified 6/16/2012
 */

%{
%}
%option noyywrap
%x MATH ATTRIBUTELIST BBOLD FRAKTUR CALLIGRAPHIC ROMAN PLAINTEXT INT1 INT2 INT3 MATHENV SVGENV SVGEND OPTARG ARRAYOPTIONS COLOPTS TEXTOPTARG TEXTOPTARG1 RAISEBOX1 RAISEBOX2
%{
#include <string.h>

#define YYSTYPE char *

#include "y.tab.h"
#include "itex2MML.h"

#define YY_NEVER_INTERACTIVE 1
#define YY_INPUT(buffer,result,max_size) result = s_itex2MML_read (buffer, max_size)

static int s_itex2MML_read (char * buffer, int max_size);

#define yylval itex2MML_yylval
#define itex2MML_optarg_depth 100

extern char * yylval;

int itex2MML_lineno = 1;
int itex2MML_inoptarg[itex2MML_optarg_depth];
int itex2MML_optarg_ind = 0;
int itex2MML_env_start = 0;

int itex2MML_rowposn = 0;
int itex2MML_displaymode = 0;

%} 
%%
%{ if (itex2MML_rowposn==1) itex2MML_rowposn=0;
   if (itex2MML_rowposn==2) itex2MML_rowposn=1;
%}

<INITIAL>"$"{1,2}   BEGIN(MATH); itex2MML_inoptarg[0] = 0; itex2MML_rowposn=2; if (yyleng==2) { itex2MML_displaymode=1; return STARTDMATH;}; itex2MML_displaymode=0; return STARTMATH;
<INITIAL>"\\[" BEGIN(MATH); itex2MML_inoptarg[0] = 0; itex2MML_rowposn=2; itex2MML_displaymode=1; return STARTDMATH;
<INITIAL>EOF    return 0;
<INITIAL>"\n" {itex2MML_lineno++; yylval=itex2MML_copy_string(yytext); return CHAR;}
<INITIAL>.     {yylval=itex2MML_copy_string(yytext); return CHAR;}


<MATH>{



"$"{1,2} |
"\\]"           {BEGIN(INITIAL); return ENDMATH;}


"^"            return SUP;
"_"            return SUB; 
"{"[\t\r\n ]*"}"   return EMPTYMROW;
"{"            {itex2MML_rowposn = 2; if(itex2MML_inoptarg[itex2MML_optarg_ind] != 0) itex2MML_inoptarg[itex2MML_optarg_ind]++; return MROWOPEN;}
"}"            {if(itex2MML_inoptarg[itex2MML_optarg_ind] != 0) itex2MML_inoptarg[itex2MML_optarg_ind]--; return MROWCLOSE;}

"\\left"       itex2MML_rowposn=2; return LEFT;
"\\right"      return RIGHT;

"\\big"   |
"\\bigr"  {return BIG;}
"\\Big"   |
"\\Bigr"  {return BBIG;}
"\\bigg"  |
"\\biggr" {return BIGG;}
"\\Bigg"  |
"\\Biggr" {return BBIGG;}

"\\bigl"  {return BIGL;}
"\\Bigl"  {return BBIGL;}
"\\biggl" {return BIGGL;}
"\\Biggl" {return BBIGGL;}

"\\mathrlap"  {return RLAP;}
"\\mathllap"  {return LLAP;}
"\\mathclap"  {return CLAP;}
"\\rlap" |
"\\llap" |
"\\ulap" |
"\\dlap"               {/* Ignore */ ;}


"&amp;" |
"&"         {itex2MML_rowposn=2; return COLSEP;}

"\\&"           {yylval=itex2MML_copy_string("&amp;"); return MI;}

"\\$"           {yylval=itex2MML_copy_string("$"); return MI;}
"\\%"           {yylval=itex2MML_copy_string("%"); return MI;}

"\\\\"          itex2MML_rowposn=2; return ROWSEP;

[a-zA-Z]+       {yylval=itex2MML_copy_string(yytext); return MI;}

[0-9]{1,3}(,[0-9]{3,3})*(\.[0-9]+)? |
[0-9]*[\.][0-9]+                   |
[0-9]+           {yylval=itex2MML_copy_string(yytext); return MN;}
"\\infty"   |
"\\infinity"     {yylval=itex2MML_copy_string("&infin;"); return MN;}

"("   |
"["             {yylval=itex2MML_copy_string(yytext); return LEFTDELIM;}

")"   {yylval=itex2MML_copy_string(yytext); return RIGHTDELIM;}
"]"   {if(itex2MML_inoptarg[itex2MML_optarg_ind]==1 && itex2MML_optarg_ind > 0) {itex2MML_inoptarg[itex2MML_optarg_ind]=0; itex2MML_optarg_ind--; return OPTARGCLOSE;} else {yylval=itex2MML_copy_string(yytext); return RIGHTDELIM;}}

"\\lbrace" |
"\\{"           {yylval=itex2MML_copy_string("{"); return LEFTDELIM;}
"\\rbrace" |
"\\}"           {yylval=itex2MML_copy_string("}"); return RIGHTDELIM;}

"\\vert" |
"|"             {yylval=itex2MML_copy_string("&VerticalBar;"); return OTHERDELIM;}
"\\Vert" |
"\\|"           {yylval=itex2MML_copy_string("&DoubleVerticalBar;"); return OTHERDELIM;}

"\\setminus"   {yylval=itex2MML_copy_string("&setminus;"); return MO;}
"\\backslash"   {yylval=itex2MML_copy_string("&bsol;"); return MO;}
"\\smallsetminus"   {yylval=itex2MML_copy_string("&#xFE68;"); return MOF;}

"\\sslash" {yylval=itex2MML_copy_string("&parsl;"); return MO;}

"\\lfloor" |
"\\lceil"  |
"\\lang"   |
"\\langle"      {yylval=itex2MML_copy2(yytext, ";"); if (yylval != itex2MML_empty_string) yylval[0]='&'; return LEFTDELIM;}

"\\rfloor" |
"\\rceil"  |
"\\rang"   |
"\\rangle"      {yylval=itex2MML_copy2(yytext, ";"); if (yylval != itex2MML_empty_string) yylval[0]='&'; return RIGHTDELIM;}

"/"             {yylval=itex2MML_copy_string(yytext); return OTHERDELIM;}

"\\uparrow"   |
"\\downarrow" |
"\\updownarrow" {yylval=itex2MML_copy2(yytext, ";"); if (yylval != itex2MML_empty_string) yylval[0]='&'; return OTHERDELIM;}

"."             {yylval=itex2MML_copy_string(yytext); return PERIODDELIM;}

"-"    {if (itex2MML_rowposn==1) return UNARYMINUS; else {yylval=itex2MML_copy_string("&minus;"); return MO;}}

"+"    {if (itex2MML_rowposn==1) return UNARYPLUS; else {yylval=itex2MML_copy_string("+"); return MO;}}

"\\#" {yylval=itex2MML_copy_string("#"); return MOL;}

","   |
":"   |
"!"   |
"="   |
"~"   |
";"   |
"?"   |
"#"   |
"`"   |
"*"        {yylval=itex2MML_copy_string(yytext); return MOL;}

"\\prime"  {yylval=itex2MML_copy_string("&prime;"); return MOLL;}
"'"        {yylval=itex2MML_copy_string("&prime;"); return MOL;}
"''"     {yylval=itex2MML_copy_string("&Prime;"); return MOL;}
"'''"  {yylval=itex2MML_copy_string("&#8244;"); return MOL;}
"''''"  {yylval=itex2MML_copy_string("&#8279;"); return MOL;}

"\\alpha"   |
"\\beta" |
"\\gamma" |
"\\delta" |
"\\zeta" |
"\\eta"  |
"\\theta" |
"\\iota"  |
"\\kappa" |
"\\lambda" |
"\\mu"     |
"\\nu"    |
"\\xi"   |
"\\pi"     |
"\\rho"   |
"\\sigma"  |
"\\tau"   |
"\\upsilon" |
"\\chi"  |
"\\psi"  |
"\\omega" |
"\\backepsilon"   |
"\\varkappa" |
"\\varpi" |
"\\varrho" |
"\\varsigma" |
"\\vartheta"      {yylval=itex2MML_copy2(yytext, ";"); if (yylval != itex2MML_empty_string) yylval[0]='&'; return MI;}

"\\varepsilon" {yylval=itex2MML_copy_string("&#x03B5;"); return MI;}
"\\phi"  {yylval=itex2MML_copy_string("&#x03D5;"); return MI;}
"\\varphi"  {yylval=itex2MML_copy_string("&#x03C6;"); return MI;}

"\\arccos"  |
"\\arcsin"  |
"\\arctan"  |
"\\arg"     |
"\\cos"     |
"\\cosh"    |
"\\cot"     |
"\\coth"    |
"\\csc"     |
"\\deg"     |
"\\dim"     |
"\\exp"     |
"\\hom"     |
"\\ker"     |
"\\lg"      |
"\\ln"      |
"\\log"     |
"\\sec"     |
"\\sin"     |
"\\sinh"    |
"\\tan"     |
"\\tanh"    {yylval=itex2MML_copy_string(yytext+1); return MI;}

"\\det"     |
"\\gcd"     |
"\\inf"     |
"\\lim"     |
"\\liminf"  |
"\\limsup"  |
"\\max"     |
"\\min"     |
"\\Pr"      |
"\\sup"    {yylval=itex2MML_copy_string(yytext+1); return MIB;}

"\\omicron"  {yylval=itex2MML_copy_string("&oscr;"); return MI;}

"\\epsilon"  {yylval=itex2MML_copy_string("&epsi;"); return MI;}

"\\cdot"   {yylval=itex2MML_copy_string("&sdot;"); return MO;}

"\\Alpha"   |
"\\Beta" |
"\\Delta"  |
"\\Gamma"  |
"\\digamma"  |
"\\Lambda" |
"\\Pi"     |
"\\Phi"    |
"\\Psi"    |
"\\Sigma"  |
"\\Theta"  |
"\\Xi"     |
"\\Zeta" |
"\\Eta"  |
"\\Iota"  |
"\\Kappa" |
"\\Mu"     |
"\\Nu"    |
"\\Rho"   |
"\\Tau"  | 
"\\mho"  | 
"\\Omega"     {yylval=itex2MML_copy2(yytext, ";"); if (yylval != itex2MML_empty_string) yylval[0]='&'; return MI;}

"\\Upsilon" |
"\\Upsi" {yylval=itex2MML_copy_string("&Upsi;"); return MI;}

"\\iff" |
"\\Longleftrightarrow"  {yylval=itex2MML_copy_string("&iff;"); return MOL;}

"\\Leftrightarrow"  {yylval=itex2MML_copy_string("&Leftrightarrow;"); return MOL;}

"\\impliedby" |
"\\Leftarrow"   {yylval=itex2MML_copy_string("&Leftarrow;"); return MOL;}

"\\implies" |
"\\Rightarrow"   {yylval=itex2MML_copy_string("&Rightarrow;"); return MOL;}

"\\hookleftarrow" {yylval=itex2MML_copy_string("&hookleftarrow;"); return MOL;}

"\\embedsin" |
"\\hookrightarrow"  {yylval=itex2MML_copy_string("&hookrightarrow;"); return MOL;}

"\\longleftarrow"   {yylval=itex2MML_copy_string("&longleftarrow;"); return MOL;}

"\\longrightarrow"  {yylval=itex2MML_copy_string("&longrightarrow;"); return MOL;}

"\\leftarrow"   {yylval=itex2MML_copy_string("&leftarrow;"); return MOL;}

"\\to"    |
"\\rightarrow"  {yylval=itex2MML_copy_string("&rightarrow;"); return MOL;}

"\\leftrightarrow"  {yylval=itex2MML_copy_string("&leftrightarrow;"); return MOL;}

"\\mapsto"  |
"\\map"       {yylval=itex2MML_copy_string("&map;"); return MOL;}

"\\nearrow" |
"\\nearr"    {yylval=itex2MML_copy_string("&nearr;"); return MOL;}

"\\nwarrow" |
"\\nwarr"     {yylval=itex2MML_copy_string("&nwarr;"); return MOL;}

"\\searrow" |
"\\searr"     {yylval=itex2MML_copy_string("&searr;"); return MOL;}

"\\swarrow" |
"\\swarr"     {yylval=itex2MML_copy_string("&swarr;"); return MOL;}

"\\neArrow" |
"\\neArr"    {yylval=itex2MML_copy_string("&neArr;"); return MOL;}

"\\nwArrow" |
"\\nwArr"     {yylval=itex2MML_copy_string("&nwArr;"); return MOL;}

"\\seArrow" |
"\\seArr"     {yylval=itex2MML_copy_string("&seArr;"); return MOL;}

"\\swArrow" |
"\\swArr"     {yylval=itex2MML_copy_string("&swArr;"); return MOL;}

"\\darr"     {yylval=itex2MML_copy_string("&darr;"); return MOL;}

"\\Downarrow" {yylval=itex2MML_copy_string("&dArr;"); return MOL;}

"\\uparr"    {yylval=itex2MML_copy_string("&uarr;"); return MOL;}

"\\Uparrow" {yylval=itex2MML_copy_string("&uArr;"); return MOL;}

"\\downuparrow" |
"\\duparr" |
"\\updarr"  {yylval=itex2MML_copy_string("&updownarrow;"); return MOL;}

"\\Updownarrow"  {yylval=itex2MML_copy_string("&Updownarrow;"); return MOL;}

"\\leftsquigarrow"       {yylval=itex2MML_copy_string("&#x21DC;"); return MOL;}
"\\rightsquigarrow"      {yylval=itex2MML_copy_string("&zigrarr;"); return MOL;}
"\\dashleftarrow"        {yylval=itex2MML_copy_string("&#x290E;"); return MOL;}
"\\dashrightarrow"       {yylval=itex2MML_copy_string("&#x290F;"); return MOL;}
"\\curvearrowbotright"   {yylval=itex2MML_copy_string("&#x293B;"); return MOL;}
"\\righttoleftarrow"     {yylval=itex2MML_copy_string("&#x27F2;"); return MOL;}
"\\lefttorightarrow"     {yylval=itex2MML_copy_string("&#x27F3;"); return MOL;}
"\\leftrightsquigarrow" |
"\\upuparrows"          |
"\\rightleftarrows"     |
"\\rightrightarrows"    |
"\\curvearrowleft"      |
"\\curvearrowright"     |
"\\downdownarrows"      |
"\\leftarrowtail"       |
"\\rightarrowtail"      |
"\\leftleftarrows"      |
"\\leftrightarrows"     |
"\\Lleftarrow"          |
"\\Rrightarrow"         |
"\\looparrowleft"       |
"\\looparrowright"      |
"\\Lsh"                 |
"\\Rsh"                 |
"\\circlearrowleft"     |
"\\circlearrowright"    |
"\\twoheadleftarrow"    |
"\\twoheadrightarrow"   |
"\\nLeftarrow"          |
"\\nleftarrow"          |
"\\nLeftrightarrow"     |
"\\nleftrightarrow"     |
"\\nRightarrow"         |
"\\nrightarrow"         |
"\\downharpoonleft"     |
"\\downharpoonright"    |
"\\leftrightharpoons"   |
"\\rightleftharpoons"   |
"\\upharpoonleft"       |
"\\upharpoonright"   {yylval=itex2MML_copy2(yytext, ";"); if (yylval != itex2MML_empty_string) yylval[0]='&'; return MOL;}

"\\xrightarrow"        |
"\\xleftarrow"         |
"\\xleftrightarrow"    |
"\\xLeftarrow"         |
"\\xRightarrow"        |
"\\xLeftrightarrow"    |
"\\xleftrightharpoons" |
"\\xrightleftharpoons" |
"\\xhookleftarrow"     |
"\\xhookrightarrow"    |
"\\xmapsto" {BEGIN(OPTARG); char * s1 = itex2MML_copy_string(yytext+2); yylval = itex2MML_copy3("&", s1, ";"); itex2MML_free_string(s1); return XARROW;}

"\\dots" |
"\\ldots"  {yylval=itex2MML_copy_string("&hellip;"); return MI;}
"\\cdots"   {yylval=itex2MML_copy_string("&ctdot;"); return MI;}
"\\ddots"   {yylval=itex2MML_copy_string("&dtdot;"); return MI;}
"\\udots"   {yylval=itex2MML_copy_string("&utdot;"); return MI;}
"\\vdots"   {yylval=itex2MML_copy_string("&vellip;"); return MI;}
"\\colon"   {yylval=itex2MML_copy_string("&colon;"); return MOR;}

"\\cup"  |
"\&union;" |
"\\union"          {yylval=itex2MML_copy_string("&cup;"); return MOL;}

"\\bigcup" |
"\\Union"  |
"\&Union;"      {yylval=itex2MML_copy_string("&Union;"); return MOB;}

"\\cap"  |
"\\intersection"          {yylval=itex2MML_copy_string("&cap;"); return MOL;}

"\\bigcap" |
"\\Intersection"       {yylval=itex2MML_copy_string("&Intersection;"); return MOB;}

"\\in"           {yylval=itex2MML_copy_string("&Element;"); return MOL;}

 /* For some reason, Firefox does not put in the right spacing around some of these operators
  * unless we ask for it, explicitly.
  */
"\\coloneqq"     {yylval=itex2MML_copy_string("&colone;"); return MOL;}
"\\Coloneqq"     {yylval=itex2MML_copy_string("&Colone;"); return MOS;}
"\\coloneq"      {yylval=itex2MML_copy_string("&#x2236;&#x2212;"); return MOS;}
"\\Coloneq"      {yylval=itex2MML_copy_string("&#x2237;&#x2212;"); return MOS;}
"\\eqqcolon"     {yylval=itex2MML_copy_string("&ecolon;"); return MOS;}
"\\Eqqcolon"     {yylval=itex2MML_copy_string("&#x003d;&#x2237;"); return MOS;}
"\\eqcolon"      {yylval=itex2MML_copy_string("&#x2239;"); return MOS;}
"\\Eqcolon"      {yylval=itex2MML_copy_string("&#x2212;&#x2237;"); return MOS;}
"\\colonapprox"  {yylval=itex2MML_copy_string("&#x2236;&#x2248;"); return MOS;}
"\\Colonapprox"  {yylval=itex2MML_copy_string("&#x2237;&#x2248;"); return MOS;}
"\\colonsim"     {yylval=itex2MML_copy_string("&#x2236;&#x223C;"); return MOS;}
"\\Colonsim"     {yylval=itex2MML_copy_string("&#x2237;&#x223C;"); return MOS;}
"\\dblcolon"     {yylval=itex2MML_copy_string("&Colon;"); return MOL;}

"\\ast"  |
"\\Cap"  |
"\\Cup"  |
"\\circledast"  |
"\\circledcirc" |
"\\curlyvee"    |
"\\curlywedge"  |
"\\divideontimes" |
"\\dotplus" |
"\\leftthreetimes" |
"\\rightthreetimes" |
"\\veebar" |
"\\gt"   |
"\\lt"   |
"\\approxeq"   |
"\\backsim"   |
"\\backsimeq"   |
"\\barwedge"    |
"\\doublebarwedge" |
"\\subset"  |
"\\subseteq" |
"\\subseteqq" |
"\\subsetneq" |
"\\subsetneqq" |
"\\varsubsetneq" |
"\\varsubsetneqq" |
"\\prec"     |
"\\parallel"  |
"\\nparallel" |
"\\shortparallel" |
"\\nshortparallel" |
"\\perp"  |
"\\eqslantgtr" |
"\\eqslantless" |
"\\gg" |
"\\ggg" |
"\\geq" |
"\\geqq" |
"\\geqslant" |
"\\gneq" |
"\\gneqq" |
"\\gnapprox" |
"\\gnsim" |
"\\gtrapprox" |
"\\ge"  |
"\\le"  |
"\\leq" |
"\\leqq" |
"\\leqslant" |
"\\lessapprox" |
"\\lessdot" |
"\\lesseqgtr" |
"\\lesseqqgtr" |
"\\lessgtr" |
"\\lneq" |
"\\lneqq" |
"\\lnsim" |
"\\lvertneqq" |
"\\gtrsim" |
"\\gtrdot" |
"\\gtreqless" |
"\\gtreqqless" |
"\\gtrless" |
"\\gvertneqq" |
"\\lesssim" |
"\\lnapprox" |
"\\nsubset" |
"\\nsubseteq" |
"\\nsubseteqq" |
"\\notin" |
"\\ni" |
"\\notni" |
"\\nmid" |
"\\nshortmid" |
"\\preceq" |
"\\npreceq" |
"\\ll" |
"\\ngeq" |
"\\ngeqq" |
"\\ngeqslant" |
"\\nleq" |
"\\nleqq" |
"\\nleqslant" |
"\\nless" |
"\\supset" |
"\\supseteq" |
"\\supseteqq" |
"\\supsetneq" |
"\\supsetneqq" |
"\\varsupsetneq" |
"\\varsupsetneqq" |
"\\approx" |
"\\asymp" |
"\\bowtie" |
"\\dashv" |
"\\Vdash" |
"\\vDash" |
"\\VDash" |
"\\vdash" |
"\\Vvdash" |
"\\models" |
"\\sim" |
"\\simeq" |
"\\nsim" |
"\\smile" |
"\\triangle" |
"\\triangledown" |
"\\triangleleft" |
"\\cong" |
"\\succ" |
"\\nsucc" |
"\\ngtr" |
"\\nsupset" |
"\\nsupseteq" |
"\\propto" |
"\\equiv" |
"\\nequiv" |
"\\frown" |
"\\triangleright" |
"\\ncong" |
"\\succeq" |
"\\succapprox" |
"\\succnapprox" |
"\\succcurlyeq" |
"\\succsim" |
"\\succnsim" |
"\\nsucceq" |
"\\nvDash" |
"\\nvdash" |
"\\nVDash" |
"\\amalg" |
"\\pm" |
"\\mp" |
"\\bigcirc" |
"\\wr" |
"\\odot" |
"\\uplus" |
"\\clubsuit" |
"\\spadesuit" |
"\\Diamond" |
"\\diamond" |
"\\sqcup" |
"\\sqcap" |
"\\sqsubset" |
"\\sqsubseteq" |
"\\sqsupset" |
"\\sqsupseteq" |
"\\Subset" |
"\\Supset" |
"\\ltimes" |
"\\div" |
"\\rtimes" |
"\\bot" |
"\\therefore" |
"\\thickapprox" |
"\\thicksim" |
"\\varpropto" |
"\\varnothing" |
"\\flat" |
"\\vee" |
"\\because" |
"\\between" |
"\\Bumpeq" |
"\\bumpeq" |
"\\circeq" |
"\\curlyeqprec" |
"\\curlyeqsucc" |
"\\doteq" |
"\\doteqdot" |
"\\eqcirc" |
"\\fallingdotseq" |
"\\multimap" |
"\\pitchfork" |
"\\precapprox" |
"\\precnapprox" |
"\\preccurlyeq" |
"\\precsim" |
"\\precnsim" |
"\\risingdotseq" |
"\\sharp" |
"\\bullet" |
"\\nexists" |
"\\dagger" |
"\\ddagger" |
"\\not" |
"\\top" |
"\\natural" |
"\\angle" |
"\\measuredangle" |
"\\backprime" |
"\\bigstar" |
"\\blacklozenge" |
"\\lozenge" |
"\\blacksquare" |
"\\blacktriangle" |
"\\blacktriangleleft" |
"\\blacktriangleright" |
"\\blacktriangledown" |
"\\ntriangleleft" |
"\\ntriangleright" |
"\\ntrianglelefteq" |
"\\ntrianglerighteq" |
"\\trianglelefteq" |
"\\trianglerighteq" |
"\\triangleq" |
"\\vartriangleleft" |
"\\vartriangleright" |
"\\forall" |
"\\bigtriangleup" |
"\\bigtriangledown" |
"\\nprec"  {yylval=itex2MML_copy2(yytext, ";"); if (yylval != itex2MML_empty_string) yylval[0]='&'; return MOL;}

"\\aleph" |
"\\beth" |
"\\eth" |
"\\ell" |
"\\hbar" |
"\\Im" |
"\\imath" |
"\\jmath" |
"\\wp" |
"\\Re" {yylval=itex2MML_copy2(yytext, ";"); if (yylval != itex2MML_empty_string) yylval[0]='&'; return MI;}

"\\Perp" |
"\\Vbar" {yylval=itex2MML_copy_string("&Vbar;"); return MOL;}
"\\boxdot" {yylval=itex2MML_copy_string("&sdotb;"); return MOL;}
"\\Box" |
"\\square" {yylval=itex2MML_copy_string("&square;"); return MOL;}
"\\emptyset" |
"\\empty" {yylval=itex2MML_copy_string("&emptyv;"); return MI;}
"\\exists" {yylval=itex2MML_copy_string("&Exists;"); return MOL;}
"\\circ" {yylval=itex2MML_copy_string("&SmallCircle;"); return MOL;}
"\\rhd" {yylval=itex2MML_copy_string("&RightTriangle;"); return MOL;}
"\\lhd" {yylval=itex2MML_copy_string("&LeftTriangle;"); return MOL;}
"\\lll" {yylval=itex2MML_copy_string("&Ll;"); return MOL;}
"\\unrhd" {yylval=itex2MML_copy_string("&RightTriangleEqual;"); return MOL;}
"\\unlhd" {yylval=itex2MML_copy_string("&LeftTriangleEqual;"); return MOL;}
"\\Del" |
"\\nabla" {yylval=itex2MML_copy_string("&Del;"); return MOL;}
"\\sphericalangle" {yylval=itex2MML_copy_string("&angsph;"); return MOL;}

"\\heartsuit" {yylval=itex2MML_copy_string("&#x2661;"); return MOL;}
"\\diamondsuit" {yylval=itex2MML_copy_string("&#x2662;"); return MOL;}

"\\partial" {yylval=itex2MML_copy_string("&PartialD;"); return MOL;}

"\\qed" {yylval=itex2MML_copy_string("&squf;"); return MO;}

"\\mod" {yylval=itex2MML_copy_string("mod"); return MOS;}
"\\pmod" {return PMOD;}

"\\bottom" {yylval=itex2MML_copy_string("&perp;"); return MOL;}

"\\neg" {yylval=itex2MML_copy_string("&not;"); return MOL;}

"\\neq" |
"\\ne"   {yylval=itex2MML_copy_string("&ne;"); return MOL;}

"\\shortmid" |
"\\mid"  {yylval=itex2MML_copy_string("&VerticalBar;"); return MOS;}

"\\int"    |
"\\integral"     {yylval=itex2MML_copy_string("&Integral;"); return MOL;}

"\\iint"   |
"\\doubleintegral" {yylval=itex2MML_copy_string("&Int;"); return MOL;}

"\\iiint"   |
"\\tripleintegral" {yylval=itex2MML_copy_string("&tint;"); return MOL;}

"\\iiiint"   |
"\\quadrupleintegral" {yylval=itex2MML_copy_string("&qint;"); return MOL;}

"\\oint"    |
"\\conint"  |
"\\contourintegral"   {yylval=itex2MML_copy_string("&conint;"); return MOL;}

"\\times"   {yylval=itex2MML_copy_string("&times;"); return MOL;}
"\\star"   {yylval=itex2MML_copy_string("&sstarf;"); return MOL;}
"\\circleddash" |
"\\odash"   {yylval=itex2MML_copy_string("&odash;"); return MOL;}

"\\intercal"   {yylval=itex2MML_copy_string("&intcal;"); return MOL;}
"\\smallfrown"   {yylval=itex2MML_copy_string("&sfrown;"); return MOL;}
"\\smallsmile"   {yylval=itex2MML_copy_string("&ssmile;"); return MOL;}

"\\boxminus" |
"\\minusb"   {yylval=itex2MML_copy_string("&boxminus;"); return MOL;}
"\\boxplus" |
"\\plusb"   {yylval=itex2MML_copy_string("&boxplus;"); return MOL;}
"\\boxtimes" |
"\\timesb"   {yylval=itex2MML_copy_string("&boxtimes;"); return MOL;}

"\\sum"          {yylval=itex2MML_copy_string("&Sum;"); return MOB;}

"\\prod" |
"\\product"      {yylval=itex2MML_copy_string("&prod;"); return MOB;}

"\\coprod"  |
"\\coproduct"    {yylval=itex2MML_copy_string("&coprod;"); return MOB;}

"\\otimes"       {yylval=itex2MML_copy_string("&otimes;"); return MOL;}
"\\Otimes"       {yylval=itex2MML_copy_string("&Otimes;"); return MOL;}
"\\bigotimes"     {yylval=itex2MML_copy_string("&bigotimes;"); return MOL;}

"\\ominus"       {yylval=itex2MML_copy_string("&ominus;"); return MOL;}
"\\oslash"       {yylval=itex2MML_copy_string("&osol;"); return MOL;}

"\\oplus"       {yylval=itex2MML_copy_string("&oplus;"); return MOL;}
"\\Oplus"   |
"\\bigoplus"    {yylval=itex2MML_copy_string("&bigoplus;"); return MOB;}

"\\bigodot"    {yylval=itex2MML_copy_string("&bigodot;"); return MOB;}
"\\bigsqcup"    {yylval=itex2MML_copy_string("&bigsqcup;"); return MOB;}
"\\bigsqcap"    {yylval=itex2MML_copy_string("&#x2A05;"); return MOB;}
"\\biginterleave" {yylval=itex2MML_copy_string("&#x2AFC;"); return MOB;}
"\\biguplus"    {yylval=itex2MML_copy_string("&biguplus;"); return MOB;}

"\\wedge"       {yylval=itex2MML_copy_string("&wedge;"); return MOL;}
"\\Wedge"  |
"\\bigwedge"       {yylval=itex2MML_copy_string("&xwedge;"); return MOB;}

"\\Vee"  |
"\\bigvee"       {yylval=itex2MML_copy_string("&bigvee;"); return MOB;}

"\\invamp"  |
"\\parr"       {yylval=itex2MML_copy_string("&#8523;"); return MOL;}

"\\frac"         {return FRAC;}
"\\tfrac"         {return TFRAC;}

"\\binom"        {return BINOM;}
"\\tbinom"        {return TBINOM;}

"\\tensor"       {return TENSOR;}

"\\multiscripts" {return MULTI;}

"\\overbrace"     {return OVERBRACE;}

"\\underbrace"   {return UNDERBRACE;}

"\\underline"   {return UNDERLINE;}

"\\bar"          {return BAR;}
"\\overline" |
"\\closure" |
"\\widebar"      {return WIDEBAR;}

"\\vec"       {return VEC;} 
"\\widevec"   {return WIDEVEC;}

"\\dot"       {return DOT;}
"\\ddot"      {return DDOT;}
"\\dddot"      {return DDDOT;}
"\\ddddot"      {return DDDDOT;}

"\\tilde"       {return TILDE;} 
"\\widetilde"   {return WIDETILDE;}


"\\check"       {return CHECK;} 
"\\widecheck"   {return WIDECHECK;}


"\\hat"       {return HAT;} 
"\\widehat"   {return WIDEHAT;}

"\\underset"     {return UNDER;}

"\\stackrel" |
"\\overset"      {return OVER;}

"\\over"         {return TEXOVER;}
"\\atop"         {return TEXATOP;}

"\\underoverset"  {return UNDEROVER;}

"\\sqrt"         {BEGIN(OPTARG); return SQRT;}

"\\root"         {return ROOT;}

"\\space"        {BEGIN(INT1); return SPACE;}

"\\text"          {BEGIN(PLAINTEXT); return TEXTBOX;}

"\\statusline"    {BEGIN(PLAINTEXT); return STATLINE;}
"\\tooltip"       {BEGIN(PLAINTEXT); return TOOLTIP;}

"\\toggle"        {return TOGGLE;}

"\\mathraisebox"  {BEGIN(RAISEBOX1); return RAISEBOX;}

"\\fghilight" |
"\\fghighlight"   {BEGIN(ATTRIBUTELIST); return FGHIGHLIGHT;}

"\\bghilight" |
"\\bghighlight"   {BEGIN(ATTRIBUTELIST); return BGHIGHLIGHT;}

"\\color"        {BEGIN(ATTRIBUTELIST); return COLOR;}
"\\bgcolor"        {BEGIN(ATTRIBUTELIST); return BGCOLOR;}

"\\displaystyle"  {itex2MML_displaymode=1; itex2MML_rowposn=2; return DISPLAY;}

"\\textstyle"     {itex2MML_displaymode=0; itex2MML_rowposn=2; return TEXTSTY;}

"\\textsize"     {return TEXTSIZE;}

"\\scriptsize"     {return SCSIZE;}

"\\scriptscriptsize"     {return SCSCSIZE;}

"\\mathit"       {return ITALICS;}

"\\boldsymbol" |
"\\mathbf"       {return BOLD;}

"\\mathrm"       {BEGIN(ROMAN); return RM;}

"\\mathbb"       {BEGIN(BBOLD); return BB;}

"\\mathfrak" |
"\\mathfr"       {BEGIN(FRAKTUR); return FRAK;}

"\\slash"       {return SLASHED;}

"\\boxed"       {return BOXED;}

"\\mathcal"       {BEGIN(CALLIGRAPHIC); return CAL;}

"\\begin"        {itex2MML_env_start = 1; BEGIN(MATHENV); return BEGINENV;}
"\\end"        {BEGIN(MATHENV); return ENDENV;}

"\\substack"     {return SUBSTACK;}

"\\array"        {return ARRAY;}

"\\arrayopts"    {return ARRAYOPTS;}

"\\colalign"     {BEGIN(ATTRIBUTELIST); return COLALIGN;}

"\\collayout"   {BEGIN(ATTRIBUTELIST); return COLLAYOUT;}

"\\rowalign"     {BEGIN(ATTRIBUTELIST); return ROWALIGN;}

"\\align"     {BEGIN(ATTRIBUTELIST); return ALIGN;}

"\\equalrows"     {BEGIN(ATTRIBUTELIST); return EQROWS;}

"\\equalcols"     {BEGIN(ATTRIBUTELIST); return EQCOLS;}

"\\rowlines"     {BEGIN(ATTRIBUTELIST); return ROWLINES;}

"\\collines"     {BEGIN(ATTRIBUTELIST); return COLLINES;}

"\\frame"     {BEGIN(ATTRIBUTELIST); return FRAME;}

"\\padding"     {BEGIN(ATTRIBUTELIST); return PADDING;}

"\\rowopts"    {return ROWOPTS;}

"\\cellopts"   {return CELLOPTS;} 

"\\rowspan"     {BEGIN(ATTRIBUTELIST); return ROWSPAN;}

"\\colspan"     {BEGIN(ATTRIBUTELIST); return COLSPAN;}


"\\," |
"\\thinspace"   {return THINSPACE;}

"\\:" |
"\\medspace"    {return MEDSPACE;}

"\\;" |
"\\thickspace"  {return THICKSPACE;}

"\\quad"        {return QUAD;}
"\\qquad"       {return QQUAD;}

"\\!" |
"\\negspace"    {return NEGSPACE;}

"\\phantom"     {return PHANTOM;}

"\\href"        {BEGIN(PLAINTEXT); return HREF;}

"\\operatorname"  {BEGIN(PLAINTEXT); return OPERATORNAME;}
"\\mathop"        {BEGIN(PLAINTEXT); return MATHOP;}
"\\mathbin"       {BEGIN(PLAINTEXT); return MATHBIN;}
"\\mathrel"       {BEGIN(PLAINTEXT); return MATHREL;}
"\\itexnum"       {BEGIN(PLAINTEXT); return ITEXNUM;}

"\\includegraphics"(\[[^\]]*\])?\{[^}]*\}	;

"\\"[a-zA-Z]+    {yylval=itex2MML_copy_string(yytext+1); return MOP;}

"\&"[a-zA-Z_][a-zA-Z0-9_]*";" |
"\&#"[0-9]+";" |
"\&#x"[a-fA-F0-9]+";"  {yylval=itex2MML_copy_string(yytext); return MI;}

"\n"             itex2MML_lineno++;
[\r\t ]+           ;


.              {return UNKNOWNCHAR;}
}

<ATTRIBUTELIST>"{"[ \t\n\r #a-zA-Z0-9.\-]*"}"  {BEGIN(MATH); yylval=itex2MML_copy_string(yytext); if (yylval != itex2MML_empty_string) { yylval[0]='"'; yylval[yyleng-1]='"'; } return ATTRLIST;}

<OPTARG>{
"[" {if(itex2MML_optarg_ind < itex2MML_optarg_depth - 1 && itex2MML_inoptarg[itex2MML_optarg_ind] !=1 ){itex2MML_optarg_ind++; itex2MML_inoptarg[itex2MML_optarg_ind]=1;}; BEGIN(MATH); return OPTARGOPEN;}
[\n\r\t ]+    ;
.   {BEGIN(MATH); unput(yytext[0]);}
}

<RAISEBOX1>{
\{[\n\r\t ]*- {BEGIN(RAISEBOX2); return NEG;}
\{[\n\r\t ]* {BEGIN(RAISEBOX2);}
}

<RAISEBOX2>{
[^\}]*?\}  {yylval=itex2MML_copy_string(yytext); if ((yyleng > 1) && (yylval != itex2MML_empty_string)) yylval[yyleng-1]='\0'; BEGIN(TEXTOPTARG1); return TEXTSTRING;}
}


<TEXTOPTARG1>{
\[[^\]]*?\] {yylval=itex2MML_copy_string(yytext+1); if ((yyleng > 1) && (yylval != itex2MML_empty_string)) yylval[yyleng-2]='\0'; BEGIN(TEXTOPTARG); return TEXTSTRING;}
. {BEGIN(MATH); unput(yytext[0]);}
}

<TEXTOPTARG>{
\[[^\]]*?\] {yylval=itex2MML_copy_string(yytext+1); if ((yyleng > 1) && (yylval != itex2MML_empty_string)) yylval[yyleng-2]='\0'; BEGIN(MATH); return TEXTSTRING;}
. {BEGIN(MATH); unput(yytext[0]);}
}

<MATHENV>{
"\{matrix\}" {itex2MML_env_start = 0; BEGIN(MATH); return MATRIX;}
"\{pmatrix\}" {itex2MML_env_start = 0; BEGIN(MATH); return PMATRIX;}
"\{bmatrix\}" {itex2MML_env_start = 0; BEGIN(MATH); return BMATRIX;}
"\{Bmatrix\}" {itex2MML_env_start = 0; BEGIN(MATH); return BBMATRIX;}
"\{vmatrix\}" {itex2MML_env_start = 0; BEGIN(MATH); return VMATRIX;}
"\{Vmatrix\}" {itex2MML_env_start = 0; BEGIN(MATH); return VVMATRIX;}
"\{smallmatrix\}" {itex2MML_env_start = 0; BEGIN(MATH); return SMALLMATRIX;}
"\{cases\}" {itex2MML_env_start = 0; BEGIN(MATH); return CASES;}
"\{aligned\}" {itex2MML_env_start = 0; BEGIN(MATH); return ALIGNED;}
"\{gathered\}" {itex2MML_env_start = 0; BEGIN(MATH); return GATHERED;}
"\{split\}" {itex2MML_env_start = 0; BEGIN(MATH); return ALIGNED;}
"\{array\}" {if (itex2MML_env_start == 1) BEGIN(ARRAYOPTIONS); else BEGIN(MATH); return ARRAY;}
"\{svg\}" {itex2MML_env_start = 0; BEGIN(SVGENV); return SVG;}
}

<ARRAYOPTIONS>{
"[t]" {yylval=itex2MML_copy_string("axis 1"); return ARRAYALIGN;}
"[c]" {yylval=itex2MML_copy_string("center"); return ARRAYALIGN;}
"[b]" {yylval=itex2MML_copy_string("axis -1"); return ARRAYALIGN;}
"\{"  {BEGIN(COLOPTS); return ST;}
[\t\n\r ]+       ;
}

<COLOPTS>{
"\}"  {itex2MML_env_start = 0; BEGIN(MATH); return END;}
"c"   {yylval=itex2MML_copy_string("center"); return COLUMNALIGN;}
"l"   {yylval=itex2MML_copy_string("left"); return COLUMNALIGN;}
"r"   {yylval=itex2MML_copy_string("right"); return COLUMNALIGN;}
[\t\n\r ]+       ;
}

<SVGENV>{
"\\end\{svg" {yylval=itex2MML_copy_string(yytext); yylval[yyleng-8]=0; BEGIN(SVGEND); return XMLSTRING;}
(.|\n) {yymore();}
}

<SVGEND>{
"\}" {BEGIN(MATH); return ENDSVG;}
}

<ROMAN>{
"\{"  {return ST;}
"\}"  {BEGIN(MATH); return END;}
[a-zA-Z]   {yylval=itex2MML_copy_string(yytext); return RMCHAR;}
[\t\n\r ]+       ;
.           {return yytext[0];}
}

<BBOLD>{
"\{"  {return ST;}
"\}"  {BEGIN(MATH); return END;}
[a-z] {yylval=itex2MML_copy_string(yytext); return BBLOWERCHAR;}
[A-Z]   {yylval=itex2MML_copy_string(yytext); return BBUPPERCHAR;}
[0-9]   {yylval=itex2MML_copy_string(yytext); return BBDIGIT;}
[\t\n\r ]+       ;
.           {return yytext[0];}
}

<CALLIGRAPHIC>{
"\{"  {return ST;}
"\}"  {BEGIN(MATH); return END;}
[a-zA-Z]   {yylval=itex2MML_copy_string(yytext); return CALCHAR;}
[\t\n\r ]+       ;
.           {return yytext[0];}
}

<FRAKTUR>{
"\{"  {return ST;}
"\}"  {BEGIN(MATH); return END;}
[a-zA-Z]   {yylval=itex2MML_copy_string(yytext); return FRAKCHAR;}
[\t\n\r ]+       ;
.           {return yytext[0];}
}

<PLAINTEXT>{
\{[^\}]*?\}  {yylval=itex2MML_copy_string(yytext+1); if ((yyleng > 1) && (yylval != itex2MML_empty_string)) yylval[yyleng-2]='\0'; BEGIN(MATH); return TEXTSTRING;}
}

<INT1>{ 
"\{"      {return ST;}
"\}"      {BEGIN(INT2); return END;}
[0-9]+    {yylval=itex2MML_copy_string_extra(yytext, 1); if (yyleng && (yylval != itex2MML_empty_string)) { yylval[yyleng]=yylval[yyleng-1]; yylval[yyleng-1]='.'; yylval[yyleng+1]='\0'; } return INTONE;}
[\t\n\r ]+   ;
.         {return yytext[0];}
}

<INT2>{
"\{"      {return ST;}
"\}"      {BEGIN(INT3); return END;}
[0-9]+    {yylval=itex2MML_copy_string_extra(yytext, 1); if (yyleng && (yylval != itex2MML_empty_string)) { yylval[yyleng]=yylval[yyleng-1]; yylval[yyleng-1]='.'; yylval[yyleng+1]='\0'; } return INTTWO;}
[\t\n\r ]+   ;
.         {return yytext[0];}
}

<INT3>{
"\{"      {return ST;}
"\}"      {BEGIN(MATH); return END;}
[0-9]+    {yylval=itex2MML_copy_string_extra(yytext, 1); if (yyleng && (yylval != itex2MML_empty_string)) { yylval[yyleng]=yylval[yyleng-1]; yylval[yyleng-1]='.'; yylval[yyleng+1]='\0'; } return INTTHREE;}
[\t\n\r ]+   ;
.         {return yytext[0];}
}



%%

void itex2MML_restart ()
{
	BEGIN(INITIAL);
}

static const char *  s_itex2MML_buffer = 0;
static unsigned long s_itex2MML_length = 0;

void itex2MML_setup (const char * buffer, unsigned long length)
{
	yy_flush_buffer( YY_CURRENT_BUFFER );

	s_itex2MML_buffer = buffer;
	s_itex2MML_length = length;
}

static int s_itex2MML_read (char * buffer, int max_size)
{
	int result = 0;

	if (max_size < s_itex2MML_length)
	{
		result = max_size;

		memcpy (buffer, s_itex2MML_buffer, max_size);

		s_itex2MML_buffer += max_size;
		s_itex2MML_length -= max_size;
	}
	else
	{
		result = s_itex2MML_length;

		memcpy (buffer, s_itex2MML_buffer, s_itex2MML_length);

		s_itex2MML_buffer = 0;
		s_itex2MML_length = 0;
	}
	return result;
}
