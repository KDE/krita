// Created: Wed Sep 12 13:13:23 2007
// WARNING! All changes made in this file will be lost!

/* This file is part of the KDE project
   Copyright (C) 2007 <hubipete@gmx.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
 
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
 
   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "Dictionary.h"

Dictionary::Dictionary()
{
    m_lspace = "thickmathspace";
    m_rspace = "thickmathspace";
    m_maxsize = "infinity";
    m_minsize = "1";
    m_fence = false;
    m_separator = false;
    m_stretchy = false;
    m_symmetric = true;
    m_largeop = false;
    m_movablelimits = false;
    m_accent = false;
}
QString Dictionary::lSpace() const
{
    return m_lspace;
}

QString Dictionary::rSpace() const
{
    return m_rspace;
}

bool Dictionary::stretchy() const
{
    return m_stretchy;
}

QChar Dictionary::mapEntity( const QString& entity )
{
    if( entity.isEmpty() ) return QChar();
    switch( entity[0].toLower().toLatin1() ) {
        case 'a':
            if( entity == "Aacute" ) return QChar( 0x000C1 );
            if( entity == "aacute" ) return QChar( 0x000E1 );
            if( entity == "Abreve" ) return QChar( 0x00102 );
            if( entity == "abreve" ) return QChar( 0x00103 );
            if( entity == "ac" ) return QChar( 0x0223E );
            if( entity == "acd" ) return QChar( 0x0223F );
            if( entity == "Acirc" ) return QChar( 0x000C2 );
            if( entity == "acirc" ) return QChar( 0x000E2 );
            if( entity == "acute" ) return QChar( 0x000B4 );
            if( entity == "Acy" ) return QChar( 0x00410 );
            if( entity == "acy" ) return QChar( 0x00430 );
            if( entity == "AElig" ) return QChar( 0x000C6 );
            if( entity == "aelig" ) return QChar( 0x000E6 );
            if( entity == "af" ) return QChar( 0x02061 );
            if( entity == "Afr" ) return QChar( 0x1D504 );
            if( entity == "afr" ) return QChar( 0x1D51E );
            if( entity == "Agrave" ) return QChar( 0x000C0 );
            if( entity == "agrave" ) return QChar( 0x000E0 );
            if( entity == "aleph" ) return QChar( 0x02135 );
            if( entity == "alpha" ) return QChar( 0x003B1 );
            if( entity == "Amacr" ) return QChar( 0x00100 );
            if( entity == "amacr" ) return QChar( 0x00101 );
            if( entity == "amalg" ) return QChar( 0x02A3F );
            if( entity == "amp" ) return QChar( 0x00026 );
            if( entity == "And" ) return QChar( 0x02A53 );
            if( entity == "and" ) return QChar( 0x02227 );
            if( entity == "andand" ) return QChar( 0x02A55 );
            if( entity == "andd" ) return QChar( 0x02A5C );
            if( entity == "andslope" ) return QChar( 0x02A58 );
            if( entity == "andv" ) return QChar( 0x02A5A );
            if( entity == "ang" ) return QChar( 0x02220 );
            if( entity == "ange" ) return QChar( 0x029A4 );
            if( entity == "angle" ) return QChar( 0x02220 );
            if( entity == "angmsd" ) return QChar( 0x02221 );
            if( entity == "angmsdaa" ) return QChar( 0x029A8 );
            if( entity == "angmsdab" ) return QChar( 0x029A9 );
            if( entity == "angmsdac" ) return QChar( 0x029AA );
            if( entity == "angmsdad" ) return QChar( 0x029AB );
            if( entity == "angmsdae" ) return QChar( 0x029AC );
            if( entity == "angmsdaf" ) return QChar( 0x029AD );
            if( entity == "angmsdag" ) return QChar( 0x029AE );
            if( entity == "angmsdah" ) return QChar( 0x029AF );
            if( entity == "angrt" ) return QChar( 0x0221F );
            if( entity == "angrtvb" ) return QChar( 0x022BE );
            if( entity == "angrtvbd" ) return QChar( 0x0299D );
            if( entity == "angsph" ) return QChar( 0x02222 );
            if( entity == "angst" ) return QChar( 0x0212B );
            if( entity == "angzarr" ) return QChar( 0x0237C );
            if( entity == "Aogon" ) return QChar( 0x00104 );
            if( entity == "aogon" ) return QChar( 0x00105 );
            if( entity == "Aopf" ) return QChar( 0x1D538 );
            if( entity == "aopf" ) return QChar( 0x1D552 );
            if( entity == "ap" ) return QChar( 0x02248 );
            if( entity == "apacir" ) return QChar( 0x02A6F );
            if( entity == "apE" ) return QChar( 0x02A70 );
            if( entity == "ape" ) return QChar( 0x0224A );
            if( entity == "apid" ) return QChar( 0x0224B );
            if( entity == "apos" ) return QChar( 0x00027 );
            if( entity == "ApplyFunction" ) return QChar( 0x02061 );
            if( entity == "approx" ) return QChar( 0x02248 );
            if( entity == "approxeq" ) return QChar( 0x0224A );
            if( entity == "Aring" ) return QChar( 0x000C5 );
            if( entity == "aring" ) return QChar( 0x000E5 );
            if( entity == "Ascr" ) return QChar( 0x1D49C );
            if( entity == "ascr" ) return QChar( 0x1D4B6 );
            if( entity == "Assign" ) return QChar( 0x02254 );
            if( entity == "ast" ) return QChar( 0x0002A );
            if( entity == "asymp" ) return QChar( 0x02248 );
            if( entity == "asympeq" ) return QChar( 0x0224D );
            if( entity == "Atilde" ) return QChar( 0x000C3 );
            if( entity == "atilde" ) return QChar( 0x000E3 );
            if( entity == "Auml" ) return QChar( 0x000C4 );
            if( entity == "auml" ) return QChar( 0x000E4 );
            if( entity == "awconint" ) return QChar( 0x02233 );
            if( entity == "awint" ) return QChar( 0x02A11 );
            break;
        case 'b':
            if( entity == "backcong" ) return QChar( 0x0224C );
            if( entity == "backepsilon" ) return QChar( 0x003F6 );
            if( entity == "backprime" ) return QChar( 0x02035 );
            if( entity == "backsim" ) return QChar( 0x0223D );
            if( entity == "backsimeq" ) return QChar( 0x022CD );
            if( entity == "Backslash" ) return QChar( 0x02216 );
            if( entity == "Barv" ) return QChar( 0x02AE7 );
            if( entity == "barvee" ) return QChar( 0x022BD );
            if( entity == "Barwed" ) return QChar( 0x02306 );
            if( entity == "barwed" ) return QChar( 0x02305 );
            if( entity == "barwedge" ) return QChar( 0x02305 );
            if( entity == "bbrk" ) return QChar( 0x023B5 );
            if( entity == "bbrktbrk" ) return QChar( 0x023B6 );
            if( entity == "bcong" ) return QChar( 0x0224C );
            if( entity == "Bcy" ) return QChar( 0x00411 );
            if( entity == "bcy" ) return QChar( 0x00431 );
            if( entity == "becaus" ) return QChar( 0x02235 );
            if( entity == "Because" ) return QChar( 0x02235 );
            if( entity == "because" ) return QChar( 0x02235 );
            if( entity == "bemptyv" ) return QChar( 0x029B0 );
            if( entity == "bepsi" ) return QChar( 0x003F6 );
            if( entity == "bernou" ) return QChar( 0x0212C );
            if( entity == "Bernoullis" ) return QChar( 0x0212C );
            if( entity == "beta" ) return QChar( 0x003B2 );
            if( entity == "beth" ) return QChar( 0x02136 );
            if( entity == "between" ) return QChar( 0x0226C );
            if( entity == "Bfr" ) return QChar( 0x1D505 );
            if( entity == "bfr" ) return QChar( 0x1D51F );
            if( entity == "bigcap" ) return QChar( 0x022C2 );
            if( entity == "bigcirc" ) return QChar( 0x025EF );
            if( entity == "bigcup" ) return QChar( 0x022C3 );
            if( entity == "bigodot" ) return QChar( 0x02A00 );
            if( entity == "bigoplus" ) return QChar( 0x02A01 );
            if( entity == "bigotimes" ) return QChar( 0x02A02 );
            if( entity == "bigsqcup" ) return QChar( 0x02A06 );
            if( entity == "bigstar" ) return QChar( 0x02605 );
            if( entity == "bigtriangledown" ) return QChar( 0x025BD );
            if( entity == "bigtriangleup" ) return QChar( 0x025B3 );
            if( entity == "biguplus" ) return QChar( 0x02A04 );
            if( entity == "bigvee" ) return QChar( 0x022C1 );
            if( entity == "bigwedge" ) return QChar( 0x022C0 );
            if( entity == "bkarow" ) return QChar( 0x0290D );
            if( entity == "blacklozenge" ) return QChar( 0x029EB );
            if( entity == "blacksquare" ) return QChar( 0x025AA );
            if( entity == "blacktriangle" ) return QChar( 0x025B4 );
            if( entity == "blacktriangledown" ) return QChar( 0x025BE );
            if( entity == "blacktriangleleft" ) return QChar( 0x025C2 );
            if( entity == "blacktriangleright" ) return QChar( 0x025B8 );
            if( entity == "blank" ) return QChar( 0x02423 );
            if( entity == "blk12" ) return QChar( 0x02592 );
            if( entity == "blk14" ) return QChar( 0x02591 );
            if( entity == "blk34" ) return QChar( 0x02593 );
            if( entity == "block" ) return QChar( 0x02588 );
            if( entity == "bNot" ) return QChar( 0x02AED );
            if( entity == "bnot" ) return QChar( 0x02310 );
            if( entity == "Bopf" ) return QChar( 0x1D539 );
            if( entity == "bopf" ) return QChar( 0x1D553 );
            if( entity == "bot" ) return QChar( 0x022A5 );
            if( entity == "bottom" ) return QChar( 0x022A5 );
            if( entity == "bowtie" ) return QChar( 0x022C8 );
            if( entity == "boxbox" ) return QChar( 0x029C9 );
            if( entity == "boxDL" ) return QChar( 0x02557 );
            if( entity == "boxDl" ) return QChar( 0x02556 );
            if( entity == "boxdL" ) return QChar( 0x02555 );
            if( entity == "boxdl" ) return QChar( 0x02510 );
            if( entity == "boxDR" ) return QChar( 0x02554 );
            if( entity == "boxDr" ) return QChar( 0x02553 );
            if( entity == "boxdR" ) return QChar( 0x02552 );
            if( entity == "boxdr" ) return QChar( 0x0250C );
            if( entity == "boxH" ) return QChar( 0x02550 );
            if( entity == "boxh" ) return QChar( 0x02500 );
            if( entity == "boxHD" ) return QChar( 0x02566 );
            if( entity == "boxHd" ) return QChar( 0x02564 );
            if( entity == "boxhD" ) return QChar( 0x02565 );
            if( entity == "boxhd" ) return QChar( 0x0252C );
            if( entity == "boxHU" ) return QChar( 0x02569 );
            if( entity == "boxHu" ) return QChar( 0x02567 );
            if( entity == "boxhU" ) return QChar( 0x02568 );
            if( entity == "boxhu" ) return QChar( 0x02534 );
            if( entity == "boxminus" ) return QChar( 0x0229F );
            if( entity == "boxplus" ) return QChar( 0x0229E );
            if( entity == "boxtimes" ) return QChar( 0x022A0 );
            if( entity == "boxUL" ) return QChar( 0x0255D );
            if( entity == "boxUl" ) return QChar( 0x0255C );
            if( entity == "boxuL" ) return QChar( 0x0255B );
            if( entity == "boxul" ) return QChar( 0x02518 );
            if( entity == "boxUR" ) return QChar( 0x0255A );
            if( entity == "boxUr" ) return QChar( 0x02559 );
            if( entity == "boxuR" ) return QChar( 0x02558 );
            if( entity == "boxur" ) return QChar( 0x02514 );
            if( entity == "boxV" ) return QChar( 0x02551 );
            if( entity == "boxv" ) return QChar( 0x02502 );
            if( entity == "boxVH" ) return QChar( 0x0256C );
            if( entity == "boxVh" ) return QChar( 0x0256B );
            if( entity == "boxvH" ) return QChar( 0x0256A );
            if( entity == "boxvh" ) return QChar( 0x0253C );
            if( entity == "boxVL" ) return QChar( 0x02563 );
            if( entity == "boxVl" ) return QChar( 0x02562 );
            if( entity == "boxvL" ) return QChar( 0x02561 );
            if( entity == "boxvl" ) return QChar( 0x02524 );
            if( entity == "boxVR" ) return QChar( 0x02560 );
            if( entity == "boxVr" ) return QChar( 0x0255F );
            if( entity == "boxvR" ) return QChar( 0x0255E );
            if( entity == "boxvr" ) return QChar( 0x0251C );
            if( entity == "bprime" ) return QChar( 0x02035 );
            if( entity == "Breve" ) return QChar( 0x002D8 );
            if( entity == "breve" ) return QChar( 0x002D8 );
            if( entity == "brvbar" ) return QChar( 0x000A6 );
            if( entity == "Bscr" ) return QChar( 0x0212C );
            if( entity == "bscr" ) return QChar( 0x1D4B7 );
            if( entity == "bsemi" ) return QChar( 0x0204F );
            if( entity == "bsim" ) return QChar( 0x0223D );
            if( entity == "bsime" ) return QChar( 0x022CD );
            if( entity == "bsol" ) return QChar( 0x0005C );
            if( entity == "bsolb" ) return QChar( 0x029C5 );
            if( entity == "bull" ) return QChar( 0x02022 );
            if( entity == "bullet" ) return QChar( 0x02022 );
            if( entity == "bump" ) return QChar( 0x0224E );
            if( entity == "bumpE" ) return QChar( 0x02AAE );
            if( entity == "bumpe" ) return QChar( 0x0224F );
            if( entity == "Bumpeq" ) return QChar( 0x0224E );
            if( entity == "bumpeq" ) return QChar( 0x0224F );
            break;
        case 'c':
            if( entity == "Cacute" ) return QChar( 0x00106 );
            if( entity == "cacute" ) return QChar( 0x00107 );
            if( entity == "Cap" ) return QChar( 0x022D2 );
            if( entity == "cap" ) return QChar( 0x02229 );
            if( entity == "capand" ) return QChar( 0x02A44 );
            if( entity == "capbrcup" ) return QChar( 0x02A49 );
            if( entity == "capcap" ) return QChar( 0x02A4B );
            if( entity == "capcup" ) return QChar( 0x02A47 );
            if( entity == "capdot" ) return QChar( 0x02A40 );
            if( entity == "CapitalDifferentialD" ) return QChar( 0x02145 );
            if( entity == "caret" ) return QChar( 0x02041 );
            if( entity == "caron" ) return QChar( 0x002C7 );
            if( entity == "Cayleys" ) return QChar( 0x0212D );
            if( entity == "ccaps" ) return QChar( 0x02A4D );
            if( entity == "Ccaron" ) return QChar( 0x0010C );
            if( entity == "ccaron" ) return QChar( 0x0010D );
            if( entity == "Ccedil" ) return QChar( 0x000C7 );
            if( entity == "ccedil" ) return QChar( 0x000E7 );
            if( entity == "Ccirc" ) return QChar( 0x00108 );
            if( entity == "ccirc" ) return QChar( 0x00109 );
            if( entity == "Cconint" ) return QChar( 0x02230 );
            if( entity == "ccups" ) return QChar( 0x02A4C );
            if( entity == "ccupssm" ) return QChar( 0x02A50 );
            if( entity == "Cdot" ) return QChar( 0x0010A );
            if( entity == "cdot" ) return QChar( 0x0010B );
            if( entity == "cedil" ) return QChar( 0x000B8 );
            if( entity == "Cedilla" ) return QChar( 0x000B8 );
            if( entity == "cemptyv" ) return QChar( 0x029B2 );
            if( entity == "cent" ) return QChar( 0x000A2 );
            if( entity == "CenterDot" ) return QChar( 0x000B7 );
            if( entity == "centerdot" ) return QChar( 0x000B7 );
            if( entity == "Cfr" ) return QChar( 0x0212D );
            if( entity == "cfr" ) return QChar( 0x1D520 );
            if( entity == "CHcy" ) return QChar( 0x00427 );
            if( entity == "chcy" ) return QChar( 0x00447 );
            if( entity == "check" ) return QChar( 0x02713 );
            if( entity == "checkmark" ) return QChar( 0x02713 );
            if( entity == "chi" ) return QChar( 0x003C7 );
            if( entity == "cir" ) return QChar( 0x025CB );
            if( entity == "circ" ) return QChar( 0x002C6 );
            if( entity == "circeq" ) return QChar( 0x02257 );
            if( entity == "circlearrowleft" ) return QChar( 0x021BA );
            if( entity == "circlearrowright" ) return QChar( 0x021BB );
            if( entity == "circledast" ) return QChar( 0x0229B );
            if( entity == "circledcirc" ) return QChar( 0x0229A );
            if( entity == "circleddash" ) return QChar( 0x0229D );
            if( entity == "CircleDot" ) return QChar( 0x02299 );
            if( entity == "circledR" ) return QChar( 0x000AE );
            if( entity == "circledS" ) return QChar( 0x024C8 );
            if( entity == "CircleMinus" ) return QChar( 0x02296 );
            if( entity == "CirclePlus" ) return QChar( 0x02295 );
            if( entity == "CircleTimes" ) return QChar( 0x02297 );
            if( entity == "cirE" ) return QChar( 0x029C3 );
            if( entity == "cire" ) return QChar( 0x02257 );
            if( entity == "cirfnint" ) return QChar( 0x02A10 );
            if( entity == "cirmid" ) return QChar( 0x02AEF );
            if( entity == "cirscir" ) return QChar( 0x029C2 );
            if( entity == "ClockwiseContourIntegral" ) return QChar( 0x02232 );
            if( entity == "CloseCurlyDoubleQuote" ) return QChar( 0x0201D );
            if( entity == "CloseCurlyQuote" ) return QChar( 0x02019 );
            if( entity == "clubs" ) return QChar( 0x02663 );
            if( entity == "clubsuit" ) return QChar( 0x02663 );
            if( entity == "Colon" ) return QChar( 0x02237 );
            if( entity == "colon" ) return QChar( 0x0003A );
            if( entity == "Colone" ) return QChar( 0x02A74 );
            if( entity == "colone" ) return QChar( 0x02254 );
            if( entity == "coloneq" ) return QChar( 0x02254 );
            if( entity == "comma" ) return QChar( 0x0002C );
            if( entity == "commat" ) return QChar( 0x00040 );
            if( entity == "comp" ) return QChar( 0x02201 );
            if( entity == "compfn" ) return QChar( 0x02218 );
            if( entity == "complement" ) return QChar( 0x02201 );
            if( entity == "complexes" ) return QChar( 0x02102 );
            if( entity == "cong" ) return QChar( 0x02245 );
            if( entity == "congdot" ) return QChar( 0x02A6D );
            if( entity == "Congruent" ) return QChar( 0x02261 );
            if( entity == "Conint" ) return QChar( 0x0222F );
            if( entity == "conint" ) return QChar( 0x0222E );
            if( entity == "ContourIntegral" ) return QChar( 0x0222E );
            if( entity == "Copf" ) return QChar( 0x02102 );
            if( entity == "copf" ) return QChar( 0x1D554 );
            if( entity == "coprod" ) return QChar( 0x02210 );
            if( entity == "Coproduct" ) return QChar( 0x02210 );
            if( entity == "copy" ) return QChar( 0x000A9 );
            if( entity == "copysr" ) return QChar( 0x02117 );
            if( entity == "CounterClockwiseContourIntegral" ) return QChar( 0x02233 );
            if( entity == "Cross" ) return QChar( 0x02A2F );
            if( entity == "cross" ) return QChar( 0x02717 );
            if( entity == "Cscr" ) return QChar( 0x1D49E );
            if( entity == "cscr" ) return QChar( 0x1D4B8 );
            if( entity == "csub" ) return QChar( 0x02ACF );
            if( entity == "csube" ) return QChar( 0x02AD1 );
            if( entity == "csup" ) return QChar( 0x02AD0 );
            if( entity == "csupe" ) return QChar( 0x02AD2 );
            if( entity == "ctdot" ) return QChar( 0x022EF );
            if( entity == "cudarrl" ) return QChar( 0x02938 );
            if( entity == "cudarrr" ) return QChar( 0x02935 );
            if( entity == "cuepr" ) return QChar( 0x022DE );
            if( entity == "cuesc" ) return QChar( 0x022DF );
            if( entity == "cularr" ) return QChar( 0x021B6 );
            if( entity == "cularrp" ) return QChar( 0x0293D );
            if( entity == "Cup" ) return QChar( 0x022D3 );
            if( entity == "cup" ) return QChar( 0x0222A );
            if( entity == "cupbrcap" ) return QChar( 0x02A48 );
            if( entity == "CupCap" ) return QChar( 0x0224D );
            if( entity == "cupcap" ) return QChar( 0x02A46 );
            if( entity == "cupcup" ) return QChar( 0x02A4A );
            if( entity == "cupdot" ) return QChar( 0x0228D );
            if( entity == "cupor" ) return QChar( 0x02A45 );
            if( entity == "curarr" ) return QChar( 0x021B7 );
            if( entity == "curarrm" ) return QChar( 0x0293C );
            if( entity == "curlyeqprec" ) return QChar( 0x022DE );
            if( entity == "curlyeqsucc" ) return QChar( 0x022DF );
            if( entity == "curlyvee" ) return QChar( 0x022CE );
            if( entity == "curlywedge" ) return QChar( 0x022CF );
            if( entity == "curren" ) return QChar( 0x000A4 );
            if( entity == "curvearrowleft" ) return QChar( 0x021B6 );
            if( entity == "curvearrowright" ) return QChar( 0x021B7 );
            if( entity == "cuvee" ) return QChar( 0x022CE );
            if( entity == "cuwed" ) return QChar( 0x022CF );
            if( entity == "cwconint" ) return QChar( 0x02232 );
            if( entity == "cwint" ) return QChar( 0x02231 );
            if( entity == "cylcty" ) return QChar( 0x0232D );
            break;
        case 'd':
            if( entity == "Dagger" ) return QChar( 0x02021 );
            if( entity == "Dagger" ) return QChar( 0x02021 );
            if( entity == "dagger" ) return QChar( 0x02020 );
            if( entity == "dagger" ) return QChar( 0x02020 );
            if( entity == "daleth" ) return QChar( 0x02138 );
            if( entity == "Darr" ) return QChar( 0x021A1 );
            if( entity == "dArr" ) return QChar( 0x021D3 );
            if( entity == "darr" ) return QChar( 0x02193 );
            if( entity == "dash" ) return QChar( 0x02010 );
            if( entity == "Dashv" ) return QChar( 0x02AE4 );
            if( entity == "dashv" ) return QChar( 0x022A3 );
            if( entity == "dbkarow" ) return QChar( 0x0290F );
            if( entity == "dblac" ) return QChar( 0x002DD );
            if( entity == "Dcaron" ) return QChar( 0x0010E );
            if( entity == "dcaron" ) return QChar( 0x0010F );
            if( entity == "Dcy" ) return QChar( 0x00414 );
            if( entity == "dcy" ) return QChar( 0x00434 );
            if( entity == "DD" ) return QChar( 0x02145 );
            if( entity == "dd" ) return QChar( 0x02146 );
            if( entity == "ddagger" ) return QChar( 0x02021 );
            if( entity == "ddarr" ) return QChar( 0x021CA );
            if( entity == "DDotrahd" ) return QChar( 0x02911 );
            if( entity == "ddotseq" ) return QChar( 0x02A77 );
            if( entity == "deg" ) return QChar( 0x000B0 );
            if( entity == "Del" ) return QChar( 0x02207 );
            if( entity == "Delta" ) return QChar( 0x00394 );
            if( entity == "delta" ) return QChar( 0x003B4 );
            if( entity == "demptyv" ) return QChar( 0x029B1 );
            if( entity == "dfisht" ) return QChar( 0x0297F );
            if( entity == "Dfr" ) return QChar( 0x1D507 );
            if( entity == "dfr" ) return QChar( 0x1D521 );
            if( entity == "dHar" ) return QChar( 0x02965 );
            if( entity == "dharl" ) return QChar( 0x021C3 );
            if( entity == "dharr" ) return QChar( 0x021C2 );
            if( entity == "DiacriticalAcute" ) return QChar( 0x000B4 );
            if( entity == "DiacriticalDot" ) return QChar( 0x002D9 );
            if( entity == "DiacriticalDoubleAcute" ) return QChar( 0x002DD );
            if( entity == "DiacriticalGrave" ) return QChar( 0x00060 );
            if( entity == "DiacriticalTilde" ) return QChar( 0x002DC );
            if( entity == "diam" ) return QChar( 0x022C4 );
            if( entity == "Diamond" ) return QChar( 0x022C4 );
            if( entity == "diamond" ) return QChar( 0x022C4 );
            if( entity == "diamondsuit" ) return QChar( 0x02666 );
            if( entity == "diams" ) return QChar( 0x02666 );
            if( entity == "die" ) return QChar( 0x000A8 );
            if( entity == "DifferentialD" ) return QChar( 0x02146 );
            if( entity == "digamma" ) return QChar( 0x003DD );
            if( entity == "disin" ) return QChar( 0x022F2 );
            if( entity == "div" ) return QChar( 0x000F7 );
            if( entity == "divide" ) return QChar( 0x000F7 );
            if( entity == "divideontimes" ) return QChar( 0x022C7 );
            if( entity == "divonx" ) return QChar( 0x022C7 );
            if( entity == "DJcy" ) return QChar( 0x00402 );
            if( entity == "djcy" ) return QChar( 0x00452 );
            if( entity == "dlcorn" ) return QChar( 0x0231E );
            if( entity == "dlcrop" ) return QChar( 0x0230D );
            if( entity == "dollar" ) return QChar( 0x00024 );
            if( entity == "Dopf" ) return QChar( 0x1D53B );
            if( entity == "dopf" ) return QChar( 0x1D555 );
            if( entity == "Dot" ) return QChar( 0x000A8 );
            if( entity == "dot" ) return QChar( 0x002D9 );
            if( entity == "DotDot" ) return QChar( 0x020DC );
            if( entity == "doteq" ) return QChar( 0x02250 );
            if( entity == "doteqdot" ) return QChar( 0x02251 );
            if( entity == "DotEqual" ) return QChar( 0x02250 );
            if( entity == "dotminus" ) return QChar( 0x02238 );
            if( entity == "dotplus" ) return QChar( 0x02214 );
            if( entity == "dotsquare" ) return QChar( 0x022A1 );
            if( entity == "doublebarwedge" ) return QChar( 0x02306 );
            if( entity == "DoubleContourIntegral" ) return QChar( 0x0222F );
            if( entity == "DoubleDot" ) return QChar( 0x000A8 );
            if( entity == "DoubleDownArrow" ) return QChar( 0x021D3 );
            if( entity == "DoubleLeftArrow" ) return QChar( 0x021D0 );
            if( entity == "DoubleLeftRightArrow" ) return QChar( 0x021D4 );
            if( entity == "DoubleLeftTee" ) return QChar( 0x02AE4 );
            if( entity == "DoubleLongLeftArrow" ) return QChar( 0x027F8 );
            if( entity == "DoubleLongLeftRightArrow" ) return QChar( 0x027FA );
            if( entity == "DoubleLongRightArrow" ) return QChar( 0x027F9 );
            if( entity == "DoubleRightArrow" ) return QChar( 0x021D2 );
            if( entity == "DoubleRightTee" ) return QChar( 0x022A8 );
            if( entity == "DoubleUpArrow" ) return QChar( 0x021D1 );
            if( entity == "DoubleUpDownArrow" ) return QChar( 0x021D5 );
            if( entity == "DoubleVerticalBar" ) return QChar( 0x02225 );
            if( entity == "DownArrow" ) return QChar( 0x02193 );
            if( entity == "Downarrow" ) return QChar( 0x021D3 );
            if( entity == "downarrow" ) return QChar( 0x02193 );
            if( entity == "DownArrowBar" ) return QChar( 0x02913 );
            if( entity == "DownArrowUpArrow" ) return QChar( 0x021F5 );
            if( entity == "DownBreve" ) return QChar( 0x00311 );
            if( entity == "downdownarrows" ) return QChar( 0x021CA );
            if( entity == "downharpoonleft" ) return QChar( 0x021C3 );
            if( entity == "downharpoonright" ) return QChar( 0x021C2 );
            if( entity == "DownLeftRightVector" ) return QChar( 0x02950 );
            if( entity == "DownLeftTeeVector" ) return QChar( 0x0295E );
            if( entity == "DownLeftVector" ) return QChar( 0x021BD );
            if( entity == "DownLeftVectorBar" ) return QChar( 0x02956 );
            if( entity == "DownRightTeeVector" ) return QChar( 0x0295F );
            if( entity == "DownRightVector" ) return QChar( 0x021C1 );
            if( entity == "DownRightVectorBar" ) return QChar( 0x02957 );
            if( entity == "DownTee" ) return QChar( 0x022A4 );
            if( entity == "DownTeeArrow" ) return QChar( 0x021A7 );
            if( entity == "drbkarow" ) return QChar( 0x02910 );
            if( entity == "drcorn" ) return QChar( 0x0231F );
            if( entity == "drcrop" ) return QChar( 0x0230C );
            if( entity == "Dscr" ) return QChar( 0x1D49F );
            if( entity == "dscr" ) return QChar( 0x1D4B9 );
            if( entity == "DScy" ) return QChar( 0x00405 );
            if( entity == "dscy" ) return QChar( 0x00455 );
            if( entity == "dsol" ) return QChar( 0x029F6 );
            if( entity == "Dstrok" ) return QChar( 0x00110 );
            if( entity == "dstrok" ) return QChar( 0x00111 );
            if( entity == "dtdot" ) return QChar( 0x022F1 );
            if( entity == "dtri" ) return QChar( 0x025BF );
            if( entity == "dtrif" ) return QChar( 0x025BE );
            if( entity == "duarr" ) return QChar( 0x021F5 );
            if( entity == "duhar" ) return QChar( 0x0296F );
            if( entity == "dwangle" ) return QChar( 0x029A6 );
            if( entity == "DZcy" ) return QChar( 0x0040F );
            if( entity == "dzcy" ) return QChar( 0x0045F );
            if( entity == "dzigrarr" ) return QChar( 0x027FF );
            break;
        case 'e':
            if( entity == "Eacute" ) return QChar( 0x000C9 );
            if( entity == "eacute" ) return QChar( 0x000E9 );
            if( entity == "easter" ) return QChar( 0x02A6E );
            if( entity == "Ecaron" ) return QChar( 0x0011A );
            if( entity == "ecaron" ) return QChar( 0x0011B );
            if( entity == "ecir" ) return QChar( 0x02256 );
            if( entity == "Ecirc" ) return QChar( 0x000CA );
            if( entity == "ecirc" ) return QChar( 0x000EA );
            if( entity == "ecolon" ) return QChar( 0x02255 );
            if( entity == "Ecy" ) return QChar( 0x0042D );
            if( entity == "ecy" ) return QChar( 0x0044D );
            if( entity == "eDDot" ) return QChar( 0x02A77 );
            if( entity == "Edot" ) return QChar( 0x00116 );
            if( entity == "eDot" ) return QChar( 0x02251 );
            if( entity == "edot" ) return QChar( 0x00117 );
            if( entity == "ee" ) return QChar( 0x02147 );
            if( entity == "efDot" ) return QChar( 0x02252 );
            if( entity == "Efr" ) return QChar( 0x1D508 );
            if( entity == "efr" ) return QChar( 0x1D522 );
            if( entity == "eg" ) return QChar( 0x02A9A );
            if( entity == "Egrave" ) return QChar( 0x000C8 );
            if( entity == "egrave" ) return QChar( 0x000E8 );
            if( entity == "egs" ) return QChar( 0x02A96 );
            if( entity == "egsdot" ) return QChar( 0x02A98 );
            if( entity == "el" ) return QChar( 0x02A99 );
            if( entity == "Element" ) return QChar( 0x02208 );
            if( entity == "elinters" ) return QChar( 0x0FFFD );
            if( entity == "ell" ) return QChar( 0x02113 );
            if( entity == "els" ) return QChar( 0x02A95 );
            if( entity == "elsdot" ) return QChar( 0x02A97 );
            if( entity == "Emacr" ) return QChar( 0x00112 );
            if( entity == "emacr" ) return QChar( 0x00113 );
            if( entity == "empty" ) return QChar( 0x02205 );
            if( entity == "emptyset" ) return QChar( 0x02205 );
            if( entity == "EmptySmallSquare" ) return QChar( 0x025FB );
            if( entity == "emptyv" ) return QChar( 0x02205 );
            if( entity == "EmptyVerySmallSquare" ) return QChar( 0x025AB );
            if( entity == "emsp" ) return QChar( 0x02003 );
            if( entity == "emsp13" ) return QChar( 0x02004 );
            if( entity == "emsp14" ) return QChar( 0x02005 );
            if( entity == "ENG" ) return QChar( 0x0014A );
            if( entity == "eng" ) return QChar( 0x0014B );
            if( entity == "ensp" ) return QChar( 0x02002 );
            if( entity == "Eogon" ) return QChar( 0x00118 );
            if( entity == "eogon" ) return QChar( 0x00119 );
            if( entity == "Eopf" ) return QChar( 0x1D53C );
            if( entity == "eopf" ) return QChar( 0x1D556 );
            if( entity == "epar" ) return QChar( 0x022D5 );
            if( entity == "eparsl" ) return QChar( 0x029E3 );
            if( entity == "eplus" ) return QChar( 0x02A71 );
            if( entity == "epsi" ) return QChar( 0x003F5 );
            if( entity == "epsiv" ) return QChar( 0x003B5 );
            if( entity == "eqcirc" ) return QChar( 0x02256 );
            if( entity == "eqcolon" ) return QChar( 0x02255 );
            if( entity == "eqsim" ) return QChar( 0x02242 );
            if( entity == "eqslantgtr" ) return QChar( 0x02A96 );
            if( entity == "eqslantless" ) return QChar( 0x02A95 );
            if( entity == "Equal" ) return QChar( 0x02A75 );
            if( entity == "equals" ) return QChar( 0x0003D );
            if( entity == "EqualTilde" ) return QChar( 0x02242 );
            if( entity == "equest" ) return QChar( 0x0225F );
            if( entity == "Equilibrium" ) return QChar( 0x021CC );
            if( entity == "equiv" ) return QChar( 0x02261 );
            if( entity == "equivDD" ) return QChar( 0x02A78 );
            if( entity == "eqvparsl" ) return QChar( 0x029E5 );
            if( entity == "erarr" ) return QChar( 0x02971 );
            if( entity == "erDot" ) return QChar( 0x02253 );
            if( entity == "Escr" ) return QChar( 0x02130 );
            if( entity == "escr" ) return QChar( 0x0212F );
            if( entity == "esdot" ) return QChar( 0x02250 );
            if( entity == "Esim" ) return QChar( 0x02A73 );
            if( entity == "esim" ) return QChar( 0x02242 );
            if( entity == "eta" ) return QChar( 0x003B7 );
            if( entity == "ETH" ) return QChar( 0x000D0 );
            if( entity == "eth" ) return QChar( 0x000F0 );
            if( entity == "Euml" ) return QChar( 0x000CB );
            if( entity == "euml" ) return QChar( 0x000EB );
            if( entity == "excl" ) return QChar( 0x00021 );
            if( entity == "exist" ) return QChar( 0x02203 );
            if( entity == "Exists" ) return QChar( 0x02203 );
            if( entity == "expectation" ) return QChar( 0x02130 );
            if( entity == "ExponentialE" ) return QChar( 0x02147 );
            if( entity == "exponentiale" ) return QChar( 0x02147 );
            break;
        case 'f':
            if( entity == "fallingdotseq" ) return QChar( 0x02252 );
            if( entity == "Fcy" ) return QChar( 0x00424 );
            if( entity == "fcy" ) return QChar( 0x00444 );
            if( entity == "female" ) return QChar( 0x02640 );
            if( entity == "ffilig" ) return QChar( 0x0FB03 );
            if( entity == "fflig" ) return QChar( 0x0FB00 );
            if( entity == "ffllig" ) return QChar( 0x0FB04 );
            if( entity == "Ffr" ) return QChar( 0x1D509 );
            if( entity == "ffr" ) return QChar( 0x1D523 );
            if( entity == "filig" ) return QChar( 0x0FB01 );
            if( entity == "FilledSmallSquare" ) return QChar( 0x025FC );
            if( entity == "FilledVerySmallSquare" ) return QChar( 0x025AA );
            if( entity == "flat" ) return QChar( 0x0266D );
            if( entity == "fllig" ) return QChar( 0x0FB02 );
            if( entity == "fltns" ) return QChar( 0x025B1 );
            if( entity == "fnof" ) return QChar( 0x00192 );
            if( entity == "Fopf" ) return QChar( 0x1D53D );
            if( entity == "fopf" ) return QChar( 0x1D557 );
            if( entity == "ForAll" ) return QChar( 0x02200 );
            if( entity == "forall" ) return QChar( 0x02200 );
            if( entity == "fork" ) return QChar( 0x022D4 );
            if( entity == "forkv" ) return QChar( 0x02AD9 );
            if( entity == "Fouriertrf" ) return QChar( 0x02131 );
            if( entity == "fpartint" ) return QChar( 0x02A0D );
            if( entity == "frac12" ) return QChar( 0x000BD );
            if( entity == "frac13" ) return QChar( 0x02153 );
            if( entity == "frac14" ) return QChar( 0x000BC );
            if( entity == "frac15" ) return QChar( 0x02155 );
            if( entity == "frac16" ) return QChar( 0x02159 );
            if( entity == "frac18" ) return QChar( 0x0215B );
            if( entity == "frac23" ) return QChar( 0x02154 );
            if( entity == "frac25" ) return QChar( 0x02156 );
            if( entity == "frac34" ) return QChar( 0x000BE );
            if( entity == "frac35" ) return QChar( 0x02157 );
            if( entity == "frac38" ) return QChar( 0x0215C );
            if( entity == "frac45" ) return QChar( 0x02158 );
            if( entity == "frac56" ) return QChar( 0x0215A );
            if( entity == "frac58" ) return QChar( 0x0215D );
            if( entity == "frac78" ) return QChar( 0x0215E );
            if( entity == "frown" ) return QChar( 0x02322 );
            if( entity == "Fscr" ) return QChar( 0x02131 );
            if( entity == "fscr" ) return QChar( 0x1D4BB );
            break;
        case 'g':
            if( entity == "gacute" ) return QChar( 0x001F5 );
            if( entity == "Gamma" ) return QChar( 0x00393 );
            if( entity == "gamma" ) return QChar( 0x003B3 );
            if( entity == "Gammad" ) return QChar( 0x003DC );
            if( entity == "gammad" ) return QChar( 0x003DD );
            if( entity == "gap" ) return QChar( 0x02A86 );
            if( entity == "Gbreve" ) return QChar( 0x0011E );
            if( entity == "gbreve" ) return QChar( 0x0011F );
            if( entity == "Gcedil" ) return QChar( 0x00122 );
            if( entity == "Gcirc" ) return QChar( 0x0011C );
            if( entity == "gcirc" ) return QChar( 0x0011D );
            if( entity == "Gcy" ) return QChar( 0x00413 );
            if( entity == "gcy" ) return QChar( 0x00433 );
            if( entity == "Gdot" ) return QChar( 0x00120 );
            if( entity == "gdot" ) return QChar( 0x00121 );
            if( entity == "gE" ) return QChar( 0x02267 );
            if( entity == "ge" ) return QChar( 0x02265 );
            if( entity == "gEl" ) return QChar( 0x02A8C );
            if( entity == "gel" ) return QChar( 0x022DB );
            if( entity == "geq" ) return QChar( 0x02265 );
            if( entity == "geqq" ) return QChar( 0x02267 );
            if( entity == "geqslant" ) return QChar( 0x02A7E );
            if( entity == "ges" ) return QChar( 0x02A7E );
            if( entity == "gescc" ) return QChar( 0x02AA9 );
            if( entity == "gesdot" ) return QChar( 0x02A80 );
            if( entity == "gesdoto" ) return QChar( 0x02A82 );
            if( entity == "gesdotol" ) return QChar( 0x02A84 );
            if( entity == "gesles" ) return QChar( 0x02A94 );
            if( entity == "Gfr" ) return QChar( 0x1D50A );
            if( entity == "gfr" ) return QChar( 0x1D524 );
            if( entity == "Gg" ) return QChar( 0x022D9 );
            if( entity == "gg" ) return QChar( 0x0226B );
            if( entity == "ggg" ) return QChar( 0x022D9 );
            if( entity == "gimel" ) return QChar( 0x02137 );
            if( entity == "GJcy" ) return QChar( 0x00403 );
            if( entity == "gjcy" ) return QChar( 0x00453 );
            if( entity == "gl" ) return QChar( 0x02277 );
            if( entity == "gla" ) return QChar( 0x02AA5 );
            if( entity == "glE" ) return QChar( 0x02A92 );
            if( entity == "glj" ) return QChar( 0x02AA4 );
            if( entity == "gnap" ) return QChar( 0x02A8A );
            if( entity == "gnapprox" ) return QChar( 0x02A8A );
            if( entity == "gnE" ) return QChar( 0x02269 );
            if( entity == "gne" ) return QChar( 0x02A88 );
            if( entity == "gneq" ) return QChar( 0x02A88 );
            if( entity == "gneqq" ) return QChar( 0x02269 );
            if( entity == "gnsim" ) return QChar( 0x022E7 );
            if( entity == "Gopf" ) return QChar( 0x1D53E );
            if( entity == "gopf" ) return QChar( 0x1D558 );
            if( entity == "grave" ) return QChar( 0x00060 );
            if( entity == "GreaterEqual" ) return QChar( 0x02265 );
            if( entity == "GreaterEqualLess" ) return QChar( 0x022DB );
            if( entity == "GreaterFullEqual" ) return QChar( 0x02267 );
            if( entity == "GreaterGreater" ) return QChar( 0x02AA2 );
            if( entity == "GreaterLess" ) return QChar( 0x02277 );
            if( entity == "GreaterSlantEqual" ) return QChar( 0x02A7E );
            if( entity == "GreaterTilde" ) return QChar( 0x02273 );
            if( entity == "Gscr" ) return QChar( 0x1D4A2 );
            if( entity == "gscr" ) return QChar( 0x0210A );
            if( entity == "gsim" ) return QChar( 0x02273 );
            if( entity == "gsime" ) return QChar( 0x02A8E );
            if( entity == "gsiml" ) return QChar( 0x02A90 );
            if( entity == "Gt" ) return QChar( 0x0226B );
            if( entity == "gt" ) return QChar( 0x0003E );
            if( entity == "gtcc" ) return QChar( 0x02AA7 );
            if( entity == "gtcir" ) return QChar( 0x02A7A );
            if( entity == "gtdot" ) return QChar( 0x022D7 );
            if( entity == "gtlPar" ) return QChar( 0x02995 );
            if( entity == "gtquest" ) return QChar( 0x02A7C );
            if( entity == "gtrapprox" ) return QChar( 0x02A86 );
            if( entity == "gtrarr" ) return QChar( 0x02978 );
            if( entity == "gtrdot" ) return QChar( 0x022D7 );
            if( entity == "gtreqless" ) return QChar( 0x022DB );
            if( entity == "gtreqqless" ) return QChar( 0x02A8C );
            if( entity == "gtrless" ) return QChar( 0x02277 );
            if( entity == "gtrsim" ) return QChar( 0x02273 );
            break;
        case 'h':
            if( entity == "Hacek" ) return QChar( 0x002C7 );
            if( entity == "hairsp" ) return QChar( 0x0200A );
            if( entity == "half" ) return QChar( 0x000BD );
            if( entity == "hamilt" ) return QChar( 0x0210B );
            if( entity == "HARDcy" ) return QChar( 0x0042A );
            if( entity == "hardcy" ) return QChar( 0x0044A );
            if( entity == "hArr" ) return QChar( 0x021D4 );
            if( entity == "harr" ) return QChar( 0x02194 );
            if( entity == "harrcir" ) return QChar( 0x02948 );
            if( entity == "harrw" ) return QChar( 0x021AD );
            if( entity == "Hat" ) return QChar( 0x0005E );
            if( entity == "hbar" ) return QChar( 0x0210F );
            if( entity == "Hcirc" ) return QChar( 0x00124 );
            if( entity == "hcirc" ) return QChar( 0x00125 );
            if( entity == "hearts" ) return QChar( 0x02665 );
            if( entity == "heartsuit" ) return QChar( 0x02665 );
            if( entity == "hellip" ) return QChar( 0x02026 );
            if( entity == "hercon" ) return QChar( 0x022B9 );
            if( entity == "Hfr" ) return QChar( 0x0210C );
            if( entity == "hfr" ) return QChar( 0x1D525 );
            if( entity == "HilbertSpace" ) return QChar( 0x0210B );
            if( entity == "hksearow" ) return QChar( 0x02925 );
            if( entity == "hkswarow" ) return QChar( 0x02926 );
            if( entity == "hoarr" ) return QChar( 0x021FF );
            if( entity == "homtht" ) return QChar( 0x0223B );
            if( entity == "hookleftarrow" ) return QChar( 0x021A9 );
            if( entity == "hookrightarrow" ) return QChar( 0x021AA );
            if( entity == "Hopf" ) return QChar( 0x0210D );
            if( entity == "hopf" ) return QChar( 0x1D559 );
            if( entity == "horbar" ) return QChar( 0x02015 );
            if( entity == "HorizontalLine" ) return QChar( 0x02500 );
            if( entity == "Hscr" ) return QChar( 0x0210B );
            if( entity == "hscr" ) return QChar( 0x1D4BD );
            if( entity == "hslash" ) return QChar( 0x0210F );
            if( entity == "Hstrok" ) return QChar( 0x00126 );
            if( entity == "hstrok" ) return QChar( 0x00127 );
            if( entity == "HumpDownHump" ) return QChar( 0x0224E );
            if( entity == "HumpEqual" ) return QChar( 0x0224F );
            if( entity == "hybull" ) return QChar( 0x02043 );
            if( entity == "hyphen" ) return QChar( 0x02010 );
            break;
        case 'i':
            if( entity == "Iacute" ) return QChar( 0x000CD );
            if( entity == "iacute" ) return QChar( 0x000ED );
            if( entity == "ic" ) return QChar( 0x02063 );
            if( entity == "Icirc" ) return QChar( 0x000CE );
            if( entity == "icirc" ) return QChar( 0x000EE );
            if( entity == "Icy" ) return QChar( 0x00418 );
            if( entity == "icy" ) return QChar( 0x00438 );
            if( entity == "Idot" ) return QChar( 0x00130 );
            if( entity == "IEcy" ) return QChar( 0x00415 );
            if( entity == "iecy" ) return QChar( 0x00435 );
            if( entity == "iexcl" ) return QChar( 0x000A1 );
            if( entity == "iff" ) return QChar( 0x021D4 );
            if( entity == "Ifr" ) return QChar( 0x02111 );
            if( entity == "ifr" ) return QChar( 0x1D526 );
            if( entity == "Igrave" ) return QChar( 0x000CC );
            if( entity == "igrave" ) return QChar( 0x000EC );
            if( entity == "ii" ) return QChar( 0x02148 );
            if( entity == "iiiint" ) return QChar( 0x02A0C );
            if( entity == "iiint" ) return QChar( 0x0222D );
            if( entity == "iinfin" ) return QChar( 0x029DC );
            if( entity == "iiota" ) return QChar( 0x02129 );
            if( entity == "IJlig" ) return QChar( 0x00132 );
            if( entity == "ijlig" ) return QChar( 0x00133 );
            if( entity == "Im" ) return QChar( 0x02111 );
            if( entity == "Imacr" ) return QChar( 0x0012A );
            if( entity == "imacr" ) return QChar( 0x0012B );
            if( entity == "image" ) return QChar( 0x02111 );
            if( entity == "ImaginaryI" ) return QChar( 0x02148 );
            if( entity == "imagline" ) return QChar( 0x02110 );
            if( entity == "imagpart" ) return QChar( 0x02111 );
            if( entity == "imath" ) return QChar( 0x00131 );
            if( entity == "imof" ) return QChar( 0x022B7 );
            if( entity == "imped" ) return QChar( 0x001B5 );
            if( entity == "Implies" ) return QChar( 0x021D2 );
            if( entity == "in" ) return QChar( 0x02208 );
            if( entity == "incare" ) return QChar( 0x02105 );
            if( entity == "infin" ) return QChar( 0x0221E );
            if( entity == "infintie" ) return QChar( 0x029DD );
            if( entity == "inodot" ) return QChar( 0x00131 );
            if( entity == "Int" ) return QChar( 0x0222C );
            if( entity == "int" ) return QChar( 0x0222B );
            if( entity == "intcal" ) return QChar( 0x022BA );
            if( entity == "integers" ) return QChar( 0x02124 );
            if( entity == "Integral" ) return QChar( 0x0222B );
            if( entity == "intercal" ) return QChar( 0x022BA );
            if( entity == "Intersection" ) return QChar( 0x022C2 );
            if( entity == "intlarhk" ) return QChar( 0x02A17 );
            if( entity == "intprod" ) return QChar( 0x02A3C );
            if( entity == "InvisibleComma" ) return QChar( 0x02063 );
            if( entity == "InvisibleTimes" ) return QChar( 0x02062 );
            if( entity == "IOcy" ) return QChar( 0x00401 );
            if( entity == "iocy" ) return QChar( 0x00451 );
            if( entity == "Iogon" ) return QChar( 0x0012E );
            if( entity == "iogon" ) return QChar( 0x0012F );
            if( entity == "Iopf" ) return QChar( 0x1D540 );
            if( entity == "iopf" ) return QChar( 0x1D55A );
            if( entity == "iota" ) return QChar( 0x003B9 );
            if( entity == "iprod" ) return QChar( 0x02A3C );
            if( entity == "iquest" ) return QChar( 0x000BF );
            if( entity == "Iscr" ) return QChar( 0x02110 );
            if( entity == "iscr" ) return QChar( 0x1D4BE );
            if( entity == "isin" ) return QChar( 0x02208 );
            if( entity == "isindot" ) return QChar( 0x022F5 );
            if( entity == "isinE" ) return QChar( 0x022F9 );
            if( entity == "isins" ) return QChar( 0x022F4 );
            if( entity == "isinsv" ) return QChar( 0x022F3 );
            if( entity == "isinv" ) return QChar( 0x02208 );
            if( entity == "it" ) return QChar( 0x02062 );
            if( entity == "Itilde" ) return QChar( 0x00128 );
            if( entity == "itilde" ) return QChar( 0x00129 );
            if( entity == "Iukcy" ) return QChar( 0x00406 );
            if( entity == "iukcy" ) return QChar( 0x00456 );
            if( entity == "Iuml" ) return QChar( 0x000CF );
            if( entity == "iuml" ) return QChar( 0x000EF );
            break;
        case 'j':
            if( entity == "Jcirc" ) return QChar( 0x00134 );
            if( entity == "jcirc" ) return QChar( 0x00135 );
            if( entity == "Jcy" ) return QChar( 0x00419 );
            if( entity == "jcy" ) return QChar( 0x00439 );
            if( entity == "Jfr" ) return QChar( 0x1D50D );
            if( entity == "jfr" ) return QChar( 0x1D527 );
            if( entity == "jmath" ) return QChar( 0x0006A );
            if( entity == "Jopf" ) return QChar( 0x1D541 );
            if( entity == "jopf" ) return QChar( 0x1D55B );
            if( entity == "Jscr" ) return QChar( 0x1D4A5 );
            if( entity == "jscr" ) return QChar( 0x1D4BF );
            if( entity == "Jsercy" ) return QChar( 0x00408 );
            if( entity == "jsercy" ) return QChar( 0x00458 );
            if( entity == "Jukcy" ) return QChar( 0x00404 );
            if( entity == "jukcy" ) return QChar( 0x00454 );
            break;
        case 'k':
            if( entity == "kappa" ) return QChar( 0x003BA );
            if( entity == "kappav" ) return QChar( 0x003F0 );
            if( entity == "Kcedil" ) return QChar( 0x00136 );
            if( entity == "kcedil" ) return QChar( 0x00137 );
            if( entity == "Kcy" ) return QChar( 0x0041A );
            if( entity == "kcy" ) return QChar( 0x0043A );
            if( entity == "Kfr" ) return QChar( 0x1D50E );
            if( entity == "kfr" ) return QChar( 0x1D528 );
            if( entity == "kgreen" ) return QChar( 0x00138 );
            if( entity == "KHcy" ) return QChar( 0x00425 );
            if( entity == "khcy" ) return QChar( 0x00445 );
            if( entity == "KJcy" ) return QChar( 0x0040C );
            if( entity == "kjcy" ) return QChar( 0x0045C );
            if( entity == "Kopf" ) return QChar( 0x1D542 );
            if( entity == "kopf" ) return QChar( 0x1D55C );
            if( entity == "Kscr" ) return QChar( 0x1D4A6 );
            if( entity == "kscr" ) return QChar( 0x1D4C0 );
            break;
        case 'l':
            if( entity == "lAarr" ) return QChar( 0x021DA );
            if( entity == "Lacute" ) return QChar( 0x00139 );
            if( entity == "lacute" ) return QChar( 0x0013A );
            if( entity == "laemptyv" ) return QChar( 0x029B4 );
            if( entity == "lagran" ) return QChar( 0x02112 );
            if( entity == "Lambda" ) return QChar( 0x0039B );
            if( entity == "lambda" ) return QChar( 0x003BB );
            if( entity == "Lang" ) return QChar( 0x0300A );
            if( entity == "lang" ) return QChar( 0x02329 );
            if( entity == "langd" ) return QChar( 0x02991 );
            if( entity == "langle" ) return QChar( 0x02329 );
            if( entity == "lap" ) return QChar( 0x02A85 );
            if( entity == "Laplacetrf" ) return QChar( 0x02112 );
            if( entity == "laquo" ) return QChar( 0x000AB );
            if( entity == "Larr" ) return QChar( 0x0219E );
            if( entity == "lArr" ) return QChar( 0x021D0 );
            if( entity == "larr" ) return QChar( 0x02190 );
            if( entity == "larrb" ) return QChar( 0x021E4 );
            if( entity == "larrbfs" ) return QChar( 0x0291F );
            if( entity == "larrfs" ) return QChar( 0x0291D );
            if( entity == "larrhk" ) return QChar( 0x021A9 );
            if( entity == "larrlp" ) return QChar( 0x021AB );
            if( entity == "larrpl" ) return QChar( 0x02939 );
            if( entity == "larrsim" ) return QChar( 0x02973 );
            if( entity == "larrtl" ) return QChar( 0x021A2 );
            if( entity == "lat" ) return QChar( 0x02AAB );
            if( entity == "lAtail" ) return QChar( 0x0291B );
            if( entity == "latail" ) return QChar( 0x02919 );
            if( entity == "late" ) return QChar( 0x02AAD );
            if( entity == "lBarr" ) return QChar( 0x0290E );
            if( entity == "lbarr" ) return QChar( 0x0290C );
            if( entity == "lbbrk" ) return QChar( 0x03014 );
            if( entity == "lbrace" ) return QChar( 0x0007B );
            if( entity == "lbrack" ) return QChar( 0x0005B );
            if( entity == "lbrke" ) return QChar( 0x0298B );
            if( entity == "lbrksld" ) return QChar( 0x0298F );
            if( entity == "lbrkslu" ) return QChar( 0x0298D );
            if( entity == "Lcaron" ) return QChar( 0x0013D );
            if( entity == "lcaron" ) return QChar( 0x0013E );
            if( entity == "Lcedil" ) return QChar( 0x0013B );
            if( entity == "lcedil" ) return QChar( 0x0013C );
            if( entity == "lceil" ) return QChar( 0x02308 );
            if( entity == "lcub" ) return QChar( 0x0007B );
            if( entity == "Lcy" ) return QChar( 0x0041B );
            if( entity == "lcy" ) return QChar( 0x0043B );
            if( entity == "ldca" ) return QChar( 0x02936 );
            if( entity == "ldquo" ) return QChar( 0x0201C );
            if( entity == "ldquor" ) return QChar( 0x0201E );
            if( entity == "ldrdhar" ) return QChar( 0x02967 );
            if( entity == "ldrushar" ) return QChar( 0x0294B );
            if( entity == "ldsh" ) return QChar( 0x021B2 );
            if( entity == "lE" ) return QChar( 0x02266 );
            if( entity == "le" ) return QChar( 0x02264 );
            if( entity == "LeftAngleBracket" ) return QChar( 0x02329 );
            if( entity == "LeftArrow" ) return QChar( 0x02190 );
            if( entity == "Leftarrow" ) return QChar( 0x021D0 );
            if( entity == "leftarrow" ) return QChar( 0x02190 );
            if( entity == "LeftArrowBar" ) return QChar( 0x021E4 );
            if( entity == "LeftArrowRightArrow" ) return QChar( 0x021C6 );
            if( entity == "leftarrowtail" ) return QChar( 0x021A2 );
            if( entity == "LeftCeiling" ) return QChar( 0x02308 );
            if( entity == "LeftDoubleBracket" ) return QChar( 0x0301A );
            if( entity == "LeftDownTeeVector" ) return QChar( 0x02961 );
            if( entity == "LeftDownVector" ) return QChar( 0x021C3 );
            if( entity == "LeftDownVectorBar" ) return QChar( 0x02959 );
            if( entity == "LeftFloor" ) return QChar( 0x0230A );
            if( entity == "leftharpoondown" ) return QChar( 0x021BD );
            if( entity == "leftharpoonup" ) return QChar( 0x021BC );
            if( entity == "leftleftarrows" ) return QChar( 0x021C7 );
            if( entity == "LeftRightArrow" ) return QChar( 0x02194 );
            if( entity == "Leftrightarrow" ) return QChar( 0x021D4 );
            if( entity == "leftrightarrow" ) return QChar( 0x02194 );
            if( entity == "leftrightarrows" ) return QChar( 0x021C6 );
            if( entity == "leftrightharpoons" ) return QChar( 0x021CB );
            if( entity == "leftrightsquigarrow" ) return QChar( 0x021AD );
            if( entity == "LeftRightVector" ) return QChar( 0x0294E );
            if( entity == "LeftTee" ) return QChar( 0x022A3 );
            if( entity == "LeftTeeArrow" ) return QChar( 0x021A4 );
            if( entity == "LeftTeeVector" ) return QChar( 0x0295A );
            if( entity == "leftthreetimes" ) return QChar( 0x022CB );
            if( entity == "LeftTriangle" ) return QChar( 0x022B2 );
            if( entity == "LeftTriangleBar" ) return QChar( 0x029CF );
            if( entity == "LeftTriangleEqual" ) return QChar( 0x022B4 );
            if( entity == "LeftUpDownVector" ) return QChar( 0x02951 );
            if( entity == "LeftUpTeeVector" ) return QChar( 0x02960 );
            if( entity == "LeftUpVector" ) return QChar( 0x021BF );
            if( entity == "LeftUpVectorBar" ) return QChar( 0x02958 );
            if( entity == "LeftVector" ) return QChar( 0x021BC );
            if( entity == "LeftVectorBar" ) return QChar( 0x02952 );
            if( entity == "lEg" ) return QChar( 0x02A8B );
            if( entity == "leg" ) return QChar( 0x022DA );
            if( entity == "leq" ) return QChar( 0x02264 );
            if( entity == "leqq" ) return QChar( 0x02266 );
            if( entity == "leqslant" ) return QChar( 0x02A7D );
            if( entity == "les" ) return QChar( 0x02A7D );
            if( entity == "lescc" ) return QChar( 0x02AA8 );
            if( entity == "lesdot" ) return QChar( 0x02A7F );
            if( entity == "lesdoto" ) return QChar( 0x02A81 );
            if( entity == "lesdotor" ) return QChar( 0x02A83 );
            if( entity == "lesges" ) return QChar( 0x02A93 );
            if( entity == "lessapprox" ) return QChar( 0x02A85 );
            if( entity == "lessdot" ) return QChar( 0x022D6 );
            if( entity == "lesseqgtr" ) return QChar( 0x022DA );
            if( entity == "lesseqqgtr" ) return QChar( 0x02A8B );
            if( entity == "LessEqualGreater" ) return QChar( 0x022DA );
            if( entity == "LessFullEqual" ) return QChar( 0x02266 );
            if( entity == "LessGreater" ) return QChar( 0x02276 );
            if( entity == "lessgtr" ) return QChar( 0x02276 );
            if( entity == "LessLess" ) return QChar( 0x02AA1 );
            if( entity == "lesssim" ) return QChar( 0x02272 );
            if( entity == "LessSlantEqual" ) return QChar( 0x02A7D );
            if( entity == "LessTilde" ) return QChar( 0x02272 );
            if( entity == "lfisht" ) return QChar( 0x0297C );
            if( entity == "lfloor" ) return QChar( 0x0230A );
            if( entity == "Lfr" ) return QChar( 0x1D50F );
            if( entity == "lfr" ) return QChar( 0x1D529 );
            if( entity == "lg" ) return QChar( 0x02276 );
            if( entity == "lgE" ) return QChar( 0x02A91 );
            if( entity == "lHar" ) return QChar( 0x02962 );
            if( entity == "lhard" ) return QChar( 0x021BD );
            if( entity == "lharu" ) return QChar( 0x021BC );
            if( entity == "lharul" ) return QChar( 0x0296A );
            if( entity == "lhblk" ) return QChar( 0x02584 );
            if( entity == "LJcy" ) return QChar( 0x00409 );
            if( entity == "ljcy" ) return QChar( 0x00459 );
            if( entity == "Ll" ) return QChar( 0x022D8 );
            if( entity == "ll" ) return QChar( 0x0226A );
            if( entity == "llarr" ) return QChar( 0x021C7 );
            if( entity == "llcorner" ) return QChar( 0x0231E );
            if( entity == "Lleftarrow" ) return QChar( 0x021DA );
            if( entity == "llhard" ) return QChar( 0x0296B );
            if( entity == "lltri" ) return QChar( 0x025FA );
            if( entity == "Lmidot" ) return QChar( 0x0013F );
            if( entity == "lmidot" ) return QChar( 0x00140 );
            if( entity == "lmoust" ) return QChar( 0x023B0 );
            if( entity == "lmoustache" ) return QChar( 0x023B0 );
            if( entity == "lnap" ) return QChar( 0x02A89 );
            if( entity == "lnapprox" ) return QChar( 0x02A89 );
            if( entity == "lnE" ) return QChar( 0x02268 );
            if( entity == "lne" ) return QChar( 0x02A87 );
            if( entity == "lneq" ) return QChar( 0x02A87 );
            if( entity == "lneqq" ) return QChar( 0x02268 );
            if( entity == "lnsim" ) return QChar( 0x022E6 );
            if( entity == "loang" ) return QChar( 0x03018 );
            if( entity == "loarr" ) return QChar( 0x021FD );
            if( entity == "lobrk" ) return QChar( 0x0301A );
            if( entity == "LongLeftArrow" ) return QChar( 0x027F5 );
            if( entity == "Longleftarrow" ) return QChar( 0x027F8 );
            if( entity == "longleftarrow" ) return QChar( 0x027F5 );
            if( entity == "LongLeftRightArrow" ) return QChar( 0x027F7 );
            if( entity == "Longleftrightarrow" ) return QChar( 0x027FA );
            if( entity == "longleftrightarrow" ) return QChar( 0x027F7 );
            if( entity == "longmapsto" ) return QChar( 0x027FC );
            if( entity == "LongRightArrow" ) return QChar( 0x027F6 );
            if( entity == "Longrightarrow" ) return QChar( 0x027F9 );
            if( entity == "longrightarrow" ) return QChar( 0x027F6 );
            if( entity == "looparrowleft" ) return QChar( 0x021AB );
            if( entity == "looparrowright" ) return QChar( 0x021AC );
            if( entity == "lopar" ) return QChar( 0x02985 );
            if( entity == "Lopf" ) return QChar( 0x1D543 );
            if( entity == "lopf" ) return QChar( 0x1D55D );
            if( entity == "loplus" ) return QChar( 0x02A2D );
            if( entity == "lotimes" ) return QChar( 0x02A34 );
            if( entity == "lowast" ) return QChar( 0x02217 );
            if( entity == "lowbar" ) return QChar( 0x0005F );
            if( entity == "LowerLeftArrow" ) return QChar( 0x02199 );
            if( entity == "LowerRightArrow" ) return QChar( 0x02198 );
            if( entity == "loz" ) return QChar( 0x025CA );
            if( entity == "lozenge" ) return QChar( 0x025CA );
            if( entity == "lozf" ) return QChar( 0x029EB );
            if( entity == "lpar" ) return QChar( 0x00028 );
            if( entity == "lparlt" ) return QChar( 0x02993 );
            if( entity == "lrarr" ) return QChar( 0x021C6 );
            if( entity == "lrcorner" ) return QChar( 0x0231F );
            if( entity == "lrhar" ) return QChar( 0x021CB );
            if( entity == "lrhard" ) return QChar( 0x0296D );
            if( entity == "lrtri" ) return QChar( 0x022BF );
            if( entity == "Lscr" ) return QChar( 0x02112 );
            if( entity == "lscr" ) return QChar( 0x1D4C1 );
            if( entity == "Lsh" ) return QChar( 0x021B0 );
            if( entity == "lsh" ) return QChar( 0x021B0 );
            if( entity == "lsim" ) return QChar( 0x02272 );
            if( entity == "lsime" ) return QChar( 0x02A8D );
            if( entity == "lsimg" ) return QChar( 0x02A8F );
            if( entity == "lsqb" ) return QChar( 0x0005B );
            if( entity == "lsquo" ) return QChar( 0x02018 );
            if( entity == "lsquor" ) return QChar( 0x0201A );
            if( entity == "Lstrok" ) return QChar( 0x00141 );
            if( entity == "lstrok" ) return QChar( 0x00142 );
            if( entity == "Lt" ) return QChar( 0x0226A );
            if( entity == "lt" ) return QChar( 0x0003C );
            if( entity == "ltcc" ) return QChar( 0x02AA6 );
            if( entity == "ltcir" ) return QChar( 0x02A79 );
            if( entity == "ltdot" ) return QChar( 0x022D6 );
            if( entity == "lthree" ) return QChar( 0x022CB );
            if( entity == "ltimes" ) return QChar( 0x022C9 );
            if( entity == "ltlarr" ) return QChar( 0x02976 );
            if( entity == "ltquest" ) return QChar( 0x02A7B );
            if( entity == "ltri" ) return QChar( 0x025C3 );
            if( entity == "ltrie" ) return QChar( 0x022B4 );
            if( entity == "ltrif" ) return QChar( 0x025C2 );
            if( entity == "ltrPar" ) return QChar( 0x02996 );
            if( entity == "lurdshar" ) return QChar( 0x0294A );
            if( entity == "luruhar" ) return QChar( 0x02966 );
            break;
        case 'm':
            if( entity == "macr" ) return QChar( 0x000AF );
            if( entity == "male" ) return QChar( 0x02642 );
            if( entity == "malt" ) return QChar( 0x02720 );
            if( entity == "maltese" ) return QChar( 0x02720 );
            if( entity == "Map" ) return QChar( 0x02905 );
            if( entity == "map" ) return QChar( 0x021A6 );
            if( entity == "mapsto" ) return QChar( 0x021A6 );
            if( entity == "mapstodown" ) return QChar( 0x021A7 );
            if( entity == "mapstoleft" ) return QChar( 0x021A4 );
            if( entity == "mapstoup" ) return QChar( 0x021A5 );
            if( entity == "marker" ) return QChar( 0x025AE );
            if( entity == "mcomma" ) return QChar( 0x02A29 );
            if( entity == "Mcy" ) return QChar( 0x0041C );
            if( entity == "mcy" ) return QChar( 0x0043C );
            if( entity == "mdash" ) return QChar( 0x02014 );
            if( entity == "mDDot" ) return QChar( 0x0223A );
            if( entity == "measuredangle" ) return QChar( 0x02221 );
            if( entity == "MediumSpace" ) return QChar( 0x0205F );
            if( entity == "Mellintrf" ) return QChar( 0x02133 );
            if( entity == "Mfr" ) return QChar( 0x1D510 );
            if( entity == "mfr" ) return QChar( 0x1D52A );
            if( entity == "mho" ) return QChar( 0x02127 );
            if( entity == "micro" ) return QChar( 0x000B5 );
            if( entity == "mid" ) return QChar( 0x02223 );
            if( entity == "midast" ) return QChar( 0x0002A );
            if( entity == "midcir" ) return QChar( 0x02AF0 );
            if( entity == "middot" ) return QChar( 0x000B7 );
            if( entity == "minus" ) return QChar( 0x02212 );
            if( entity == "minusb" ) return QChar( 0x0229F );
            if( entity == "minusd" ) return QChar( 0x02238 );
            if( entity == "minusdu" ) return QChar( 0x02A2A );
            if( entity == "MinusPlus" ) return QChar( 0x02213 );
            if( entity == "mlcp" ) return QChar( 0x02ADB );
            if( entity == "mldr" ) return QChar( 0x02026 );
            if( entity == "mnplus" ) return QChar( 0x02213 );
            if( entity == "models" ) return QChar( 0x022A7 );
            if( entity == "Mopf" ) return QChar( 0x1D544 );
            if( entity == "mopf" ) return QChar( 0x1D55E );
            if( entity == "mp" ) return QChar( 0x02213 );
            if( entity == "Mscr" ) return QChar( 0x02133 );
            if( entity == "mscr" ) return QChar( 0x1D4C2 );
            if( entity == "mstpos" ) return QChar( 0x0223E );
            if( entity == "mu" ) return QChar( 0x003BC );
            if( entity == "multimap" ) return QChar( 0x022B8 );
            if( entity == "mumap" ) return QChar( 0x022B8 );
            break;
        case 'n':
            if( entity == "nabla" ) return QChar( 0x02207 );
            if( entity == "Nacute" ) return QChar( 0x00143 );
            if( entity == "nacute" ) return QChar( 0x00144 );
            if( entity == "nap" ) return QChar( 0x02249 );
            if( entity == "napos" ) return QChar( 0x00149 );
            if( entity == "napprox" ) return QChar( 0x02249 );
            if( entity == "natur" ) return QChar( 0x0266E );
            if( entity == "natural" ) return QChar( 0x0266E );
            if( entity == "naturals" ) return QChar( 0x02115 );
            if( entity == "nbsp" ) return QChar( 0x000A0 );
            if( entity == "ncap" ) return QChar( 0x02A43 );
            if( entity == "Ncaron" ) return QChar( 0x00147 );
            if( entity == "ncaron" ) return QChar( 0x00148 );
            if( entity == "Ncedil" ) return QChar( 0x00145 );
            if( entity == "ncedil" ) return QChar( 0x00146 );
            if( entity == "ncong" ) return QChar( 0x02247 );
            if( entity == "ncup" ) return QChar( 0x02A42 );
            if( entity == "Ncy" ) return QChar( 0x0041D );
            if( entity == "ncy" ) return QChar( 0x0043D );
            if( entity == "ndash" ) return QChar( 0x02013 );
            if( entity == "ne" ) return QChar( 0x02260 );
            if( entity == "nearhk" ) return QChar( 0x02924 );
            if( entity == "neArr" ) return QChar( 0x021D7 );
            if( entity == "nearr" ) return QChar( 0x02197 );
            if( entity == "nearrow" ) return QChar( 0x02197 );
            if( entity == "NegativeMediumSpace" ) return QChar( 0x0200B );
            if( entity == "NegativeThickSpace" ) return QChar( 0x0200B );
            if( entity == "NegativeThinSpace" ) return QChar( 0x0200B );
            if( entity == "NegativeVeryThinSpace" ) return QChar( 0x0200B );
            if( entity == "nequiv" ) return QChar( 0x02262 );
            if( entity == "nesear" ) return QChar( 0x02928 );
            if( entity == "NestedGreaterGreater" ) return QChar( 0x0226B );
            if( entity == "NestedLessLess" ) return QChar( 0x0226A );
            if( entity == "NewLine" ) return QChar( 0x0000A );
            if( entity == "nexist" ) return QChar( 0x02204 );
            if( entity == "nexists" ) return QChar( 0x02204 );
            if( entity == "Nfr" ) return QChar( 0x1D511 );
            if( entity == "nfr" ) return QChar( 0x1D52B );
            if( entity == "nge" ) return QChar( 0x02271 );
            if( entity == "ngeq" ) return QChar( 0x02271 );
            if( entity == "ngsim" ) return QChar( 0x02275 );
            if( entity == "ngt" ) return QChar( 0x0226F );
            if( entity == "ngtr" ) return QChar( 0x0226F );
            if( entity == "nhArr" ) return QChar( 0x021CE );
            if( entity == "nharr" ) return QChar( 0x021AE );
            if( entity == "nhpar" ) return QChar( 0x02AF2 );
            if( entity == "ni" ) return QChar( 0x0220B );
            if( entity == "nis" ) return QChar( 0x022FC );
            if( entity == "nisd" ) return QChar( 0x022FA );
            if( entity == "niv" ) return QChar( 0x0220B );
            if( entity == "NJcy" ) return QChar( 0x0040A );
            if( entity == "njcy" ) return QChar( 0x0045A );
            if( entity == "nlArr" ) return QChar( 0x021CD );
            if( entity == "nlarr" ) return QChar( 0x0219A );
            if( entity == "nldr" ) return QChar( 0x02025 );
            if( entity == "nle" ) return QChar( 0x02270 );
            if( entity == "nLeftarrow" ) return QChar( 0x021CD );
            if( entity == "nleftarrow" ) return QChar( 0x0219A );
            if( entity == "nLeftrightarrow" ) return QChar( 0x021CE );
            if( entity == "nleftrightarrow" ) return QChar( 0x021AE );
            if( entity == "nleq" ) return QChar( 0x02270 );
            if( entity == "nless" ) return QChar( 0x0226E );
            if( entity == "nlsim" ) return QChar( 0x02274 );
            if( entity == "nlt" ) return QChar( 0x0226E );
            if( entity == "nltri" ) return QChar( 0x022EA );
            if( entity == "nltrie" ) return QChar( 0x022EC );
            if( entity == "nmid" ) return QChar( 0x02224 );
            if( entity == "NoBreak" ) return QChar( 0x02060 );
            if( entity == "NonBreakingSpace" ) return QChar( 0x000A0 );
            if( entity == "Nopf" ) return QChar( 0x02115 );
            if( entity == "nopf" ) return QChar( 0x1D55F );
            if( entity == "Not" ) return QChar( 0x02AEC );
            if( entity == "not" ) return QChar( 0x000AC );
            if( entity == "NotCongruent" ) return QChar( 0x02262 );
            if( entity == "NotCupCap" ) return QChar( 0x0226D );
            if( entity == "NotDoubleVerticalBar" ) return QChar( 0x02226 );
            if( entity == "NotElement" ) return QChar( 0x02209 );
            if( entity == "NotEqual" ) return QChar( 0x02260 );
            if( entity == "NotExists" ) return QChar( 0x02204 );
            if( entity == "NotGreater" ) return QChar( 0x0226F );
            if( entity == "NotGreaterEqual" ) return QChar( 0x02271 );
            if( entity == "NotGreaterLess" ) return QChar( 0x02279 );
            if( entity == "NotGreaterTilde" ) return QChar( 0x02275 );
            if( entity == "notin" ) return QChar( 0x02209 );
            if( entity == "notinva" ) return QChar( 0x02209 );
            if( entity == "notinvb" ) return QChar( 0x022F7 );
            if( entity == "notinvc" ) return QChar( 0x022F6 );
            if( entity == "NotLeftTriangle" ) return QChar( 0x022EA );
            if( entity == "NotLeftTriangleEqual" ) return QChar( 0x022EC );
            if( entity == "NotLess" ) return QChar( 0x0226E );
            if( entity == "NotLessEqual" ) return QChar( 0x02270 );
            if( entity == "NotLessGreater" ) return QChar( 0x02278 );
            if( entity == "NotLessTilde" ) return QChar( 0x02274 );
            if( entity == "notni" ) return QChar( 0x0220C );
            if( entity == "notniva" ) return QChar( 0x0220C );
            if( entity == "notnivb" ) return QChar( 0x022FE );
            if( entity == "notnivc" ) return QChar( 0x022FD );
            if( entity == "NotPrecedes" ) return QChar( 0x02280 );
            if( entity == "NotPrecedesSlantEqual" ) return QChar( 0x022E0 );
            if( entity == "NotReverseElement" ) return QChar( 0x0220C );
            if( entity == "NotRightTriangle" ) return QChar( 0x022EB );
            if( entity == "NotRightTriangleEqual" ) return QChar( 0x022ED );
            if( entity == "NotSquareSubsetEqual" ) return QChar( 0x022E2 );
            if( entity == "NotSquareSupersetEqual" ) return QChar( 0x022E3 );
            if( entity == "NotSubsetEqual" ) return QChar( 0x02288 );
            if( entity == "NotSucceeds" ) return QChar( 0x02281 );
            if( entity == "NotSucceedsSlantEqual" ) return QChar( 0x022E1 );
            if( entity == "NotSupersetEqual" ) return QChar( 0x02289 );
            if( entity == "NotTilde" ) return QChar( 0x02241 );
            if( entity == "NotTildeEqual" ) return QChar( 0x02244 );
            if( entity == "NotTildeFullEqual" ) return QChar( 0x02247 );
            if( entity == "NotTildeTilde" ) return QChar( 0x02249 );
            if( entity == "NotVerticalBar" ) return QChar( 0x02224 );
            if( entity == "npar" ) return QChar( 0x02226 );
            if( entity == "nparallel" ) return QChar( 0x02226 );
            if( entity == "npolint" ) return QChar( 0x02A14 );
            if( entity == "npr" ) return QChar( 0x02280 );
            if( entity == "nprcue" ) return QChar( 0x022E0 );
            if( entity == "nprec" ) return QChar( 0x02280 );
            if( entity == "nrArr" ) return QChar( 0x021CF );
            if( entity == "nrarr" ) return QChar( 0x0219B );
            if( entity == "nRightarrow" ) return QChar( 0x021CF );
            if( entity == "nrightarrow" ) return QChar( 0x0219B );
            if( entity == "nrtri" ) return QChar( 0x022EB );
            if( entity == "nrtrie" ) return QChar( 0x022ED );
            if( entity == "nsc" ) return QChar( 0x02281 );
            if( entity == "nsccue" ) return QChar( 0x022E1 );
            if( entity == "Nscr" ) return QChar( 0x1D4A9 );
            if( entity == "nscr" ) return QChar( 0x1D4C3 );
            if( entity == "nshortmid" ) return QChar( 0x02224 );
            if( entity == "nshortparallel" ) return QChar( 0x02226 );
            if( entity == "nsim" ) return QChar( 0x02241 );
            if( entity == "nsime" ) return QChar( 0x02244 );
            if( entity == "nsimeq" ) return QChar( 0x02244 );
            if( entity == "nsmid" ) return QChar( 0x02224 );
            if( entity == "nspar" ) return QChar( 0x02226 );
            if( entity == "nsqsube" ) return QChar( 0x022E2 );
            if( entity == "nsqsupe" ) return QChar( 0x022E3 );
            if( entity == "nsub" ) return QChar( 0x02284 );
            if( entity == "nsube" ) return QChar( 0x02288 );
            if( entity == "nsubseteq" ) return QChar( 0x02288 );
            if( entity == "nsucc" ) return QChar( 0x02281 );
            if( entity == "nsup" ) return QChar( 0x02285 );
            if( entity == "nsupe" ) return QChar( 0x02289 );
            if( entity == "nsupseteq" ) return QChar( 0x02289 );
            if( entity == "ntgl" ) return QChar( 0x02279 );
            if( entity == "Ntilde" ) return QChar( 0x000D1 );
            if( entity == "ntilde" ) return QChar( 0x000F1 );
            if( entity == "ntlg" ) return QChar( 0x02278 );
            if( entity == "ntriangleleft" ) return QChar( 0x022EA );
            if( entity == "ntrianglelefteq" ) return QChar( 0x022EC );
            if( entity == "ntriangleright" ) return QChar( 0x022EB );
            if( entity == "ntrianglerighteq" ) return QChar( 0x022ED );
            if( entity == "nu" ) return QChar( 0x003BD );
            if( entity == "num" ) return QChar( 0x00023 );
            if( entity == "numero" ) return QChar( 0x02116 );
            if( entity == "numsp" ) return QChar( 0x02007 );
            if( entity == "nVDash" ) return QChar( 0x022AF );
            if( entity == "nVdash" ) return QChar( 0x022AE );
            if( entity == "nvDash" ) return QChar( 0x022AD );
            if( entity == "nvdash" ) return QChar( 0x022AC );
            if( entity == "nvHarr" ) return QChar( 0x02904 );
            if( entity == "nvinfin" ) return QChar( 0x029DE );
            if( entity == "nvlArr" ) return QChar( 0x02902 );
            if( entity == "nvrArr" ) return QChar( 0x02903 );
            if( entity == "nwarhk" ) return QChar( 0x02923 );
            if( entity == "nwArr" ) return QChar( 0x021D6 );
            if( entity == "nwarr" ) return QChar( 0x02196 );
            if( entity == "nwarrow" ) return QChar( 0x02196 );
            if( entity == "nwnear" ) return QChar( 0x02927 );
            break;
        case 'o':
            if( entity == "Oacute" ) return QChar( 0x000D3 );
            if( entity == "oacute" ) return QChar( 0x000F3 );
            if( entity == "oast" ) return QChar( 0x0229B );
            if( entity == "ocir" ) return QChar( 0x0229A );
            if( entity == "Ocirc" ) return QChar( 0x000D4 );
            if( entity == "ocirc" ) return QChar( 0x000F4 );
            if( entity == "Ocy" ) return QChar( 0x0041E );
            if( entity == "ocy" ) return QChar( 0x0043E );
            if( entity == "odash" ) return QChar( 0x0229D );
            if( entity == "Odblac" ) return QChar( 0x00150 );
            if( entity == "odblac" ) return QChar( 0x00151 );
            if( entity == "odiv" ) return QChar( 0x02A38 );
            if( entity == "odot" ) return QChar( 0x02299 );
            if( entity == "odsold" ) return QChar( 0x029BC );
            if( entity == "OElig" ) return QChar( 0x00152 );
            if( entity == "oelig" ) return QChar( 0x00153 );
            if( entity == "ofcir" ) return QChar( 0x029BF );
            if( entity == "Ofr" ) return QChar( 0x1D512 );
            if( entity == "ofr" ) return QChar( 0x1D52C );
            if( entity == "ogon" ) return QChar( 0x002DB );
            if( entity == "Ograve" ) return QChar( 0x000D2 );
            if( entity == "ograve" ) return QChar( 0x000F2 );
            if( entity == "ogt" ) return QChar( 0x029C1 );
            if( entity == "ohbar" ) return QChar( 0x029B5 );
            if( entity == "ohm" ) return QChar( 0x02126 );
            if( entity == "oint" ) return QChar( 0x0222E );
            if( entity == "olarr" ) return QChar( 0x021BA );
            if( entity == "olcir" ) return QChar( 0x029BE );
            if( entity == "olcross" ) return QChar( 0x029BB );
            if( entity == "olt" ) return QChar( 0x029C0 );
            if( entity == "Omacr" ) return QChar( 0x0014C );
            if( entity == "omacr" ) return QChar( 0x0014D );
            if( entity == "Omega" ) return QChar( 0x003A9 );
            if( entity == "omega" ) return QChar( 0x003C9 );
            if( entity == "omid" ) return QChar( 0x029B6 );
            if( entity == "ominus" ) return QChar( 0x02296 );
            if( entity == "Oopf" ) return QChar( 0x1D546 );
            if( entity == "oopf" ) return QChar( 0x1D560 );
            if( entity == "opar" ) return QChar( 0x029B7 );
            if( entity == "OpenCurlyDoubleQuote" ) return QChar( 0x0201C );
            if( entity == "OpenCurlyQuote" ) return QChar( 0x02018 );
            if( entity == "operp" ) return QChar( 0x029B9 );
            if( entity == "oplus" ) return QChar( 0x02295 );
            if( entity == "Or" ) return QChar( 0x02A54 );
            if( entity == "or" ) return QChar( 0x02228 );
            if( entity == "orarr" ) return QChar( 0x021BB );
            if( entity == "ord" ) return QChar( 0x02A5D );
            if( entity == "order" ) return QChar( 0x02134 );
            if( entity == "orderof" ) return QChar( 0x02134 );
            if( entity == "ordf" ) return QChar( 0x000AA );
            if( entity == "ordm" ) return QChar( 0x000BA );
            if( entity == "origof" ) return QChar( 0x022B6 );
            if( entity == "oror" ) return QChar( 0x02A56 );
            if( entity == "orslope" ) return QChar( 0x02A57 );
            if( entity == "orv" ) return QChar( 0x02A5B );
            if( entity == "oS" ) return QChar( 0x024C8 );
            if( entity == "Oscr" ) return QChar( 0x1D4AA );
            if( entity == "oscr" ) return QChar( 0x02134 );
            if( entity == "Oslash" ) return QChar( 0x000D8 );
            if( entity == "oslash" ) return QChar( 0x000F8 );
            if( entity == "osol" ) return QChar( 0x02298 );
            if( entity == "Otilde" ) return QChar( 0x000D5 );
            if( entity == "otilde" ) return QChar( 0x000F5 );
            if( entity == "Otimes" ) return QChar( 0x02A37 );
            if( entity == "otimes" ) return QChar( 0x02297 );
            if( entity == "otimesas" ) return QChar( 0x02A36 );
            if( entity == "Ouml" ) return QChar( 0x000D6 );
            if( entity == "ouml" ) return QChar( 0x000F6 );
            if( entity == "ovbar" ) return QChar( 0x0233D );
            if( entity == "OverBar" ) return QChar( 0x000AF );
            if( entity == "OverBrace" ) return QChar( 0x0FE37 );
            if( entity == "OverBracket" ) return QChar( 0x023B4 );
            if( entity == "OverParenthesis" ) return QChar( 0x0FE35 );
            break;
        case 'p':
            if( entity == "par" ) return QChar( 0x02225 );
            if( entity == "para" ) return QChar( 0x000B6 );
            if( entity == "parallel" ) return QChar( 0x02225 );
            if( entity == "parsim" ) return QChar( 0x02AF3 );
            if( entity == "parsl" ) return QChar( 0x02AFD );
            if( entity == "part" ) return QChar( 0x02202 );
            if( entity == "PartialD" ) return QChar( 0x02202 );
            if( entity == "Pcy" ) return QChar( 0x0041F );
            if( entity == "pcy" ) return QChar( 0x0043F );
            if( entity == "percnt" ) return QChar( 0x00025 );
            if( entity == "period" ) return QChar( 0x0002E );
            if( entity == "permil" ) return QChar( 0x02030 );
            if( entity == "perp" ) return QChar( 0x022A5 );
            if( entity == "pertenk" ) return QChar( 0x02031 );
            if( entity == "Pfr" ) return QChar( 0x1D513 );
            if( entity == "pfr" ) return QChar( 0x1D52D );
            if( entity == "Phi" ) return QChar( 0x003A6 );
            if( entity == "phi" ) return QChar( 0x003D5 );
            if( entity == "phiv" ) return QChar( 0x003C6 );
            if( entity == "phmmat" ) return QChar( 0x02133 );
            if( entity == "phone" ) return QChar( 0x0260E );
            if( entity == "Pi" ) return QChar( 0x003A0 );
            if( entity == "pi" ) return QChar( 0x003C0 );
            if( entity == "pitchfork" ) return QChar( 0x022D4 );
            if( entity == "piv" ) return QChar( 0x003D6 );
            if( entity == "planck" ) return QChar( 0x0210F );
            if( entity == "planckh" ) return QChar( 0x0210E );
            if( entity == "plankv" ) return QChar( 0x0210F );
            if( entity == "plus" ) return QChar( 0x0002B );
            if( entity == "plusacir" ) return QChar( 0x02A23 );
            if( entity == "plusb" ) return QChar( 0x0229E );
            if( entity == "pluscir" ) return QChar( 0x02A22 );
            if( entity == "plusdo" ) return QChar( 0x02214 );
            if( entity == "plusdu" ) return QChar( 0x02A25 );
            if( entity == "pluse" ) return QChar( 0x02A72 );
            if( entity == "PlusMinus" ) return QChar( 0x000B1 );
            if( entity == "plusmn" ) return QChar( 0x000B1 );
            if( entity == "plussim" ) return QChar( 0x02A26 );
            if( entity == "plustwo" ) return QChar( 0x02A27 );
            if( entity == "pm" ) return QChar( 0x000B1 );
            if( entity == "Poincareplane" ) return QChar( 0x0210C );
            if( entity == "pointint" ) return QChar( 0x02A15 );
            if( entity == "Popf" ) return QChar( 0x02119 );
            if( entity == "popf" ) return QChar( 0x1D561 );
            if( entity == "pound" ) return QChar( 0x000A3 );
            if( entity == "Pr" ) return QChar( 0x02ABB );
            if( entity == "pr" ) return QChar( 0x0227A );
            if( entity == "prap" ) return QChar( 0x02AB7 );
            if( entity == "prcue" ) return QChar( 0x0227C );
            if( entity == "prE" ) return QChar( 0x02AB3 );
            if( entity == "pre" ) return QChar( 0x02AAF );
            if( entity == "prec" ) return QChar( 0x0227A );
            if( entity == "precapprox" ) return QChar( 0x02AB7 );
            if( entity == "preccurlyeq" ) return QChar( 0x0227C );
            if( entity == "Precedes" ) return QChar( 0x0227A );
            if( entity == "PrecedesEqual" ) return QChar( 0x02AAF );
            if( entity == "PrecedesSlantEqual" ) return QChar( 0x0227C );
            if( entity == "PrecedesTilde" ) return QChar( 0x0227E );
            if( entity == "preceq" ) return QChar( 0x02AAF );
            if( entity == "precnapprox" ) return QChar( 0x02AB9 );
            if( entity == "precneqq" ) return QChar( 0x02AB5 );
            if( entity == "precnsim" ) return QChar( 0x022E8 );
            if( entity == "precsim" ) return QChar( 0x0227E );
            if( entity == "Prime" ) return QChar( 0x02033 );
            if( entity == "prime" ) return QChar( 0x02032 );
            if( entity == "primes" ) return QChar( 0x02119 );
            if( entity == "prnap" ) return QChar( 0x02AB9 );
            if( entity == "prnE" ) return QChar( 0x02AB5 );
            if( entity == "prnsim" ) return QChar( 0x022E8 );
            if( entity == "prod" ) return QChar( 0x0220F );
            if( entity == "Product" ) return QChar( 0x0220F );
            if( entity == "profalar" ) return QChar( 0x0232E );
            if( entity == "profline" ) return QChar( 0x02312 );
            if( entity == "profsurf" ) return QChar( 0x02313 );
            if( entity == "prop" ) return QChar( 0x0221D );
            if( entity == "Proportion" ) return QChar( 0x02237 );
            if( entity == "Proportional" ) return QChar( 0x0221D );
            if( entity == "propto" ) return QChar( 0x0221D );
            if( entity == "prsim" ) return QChar( 0x0227E );
            if( entity == "prurel" ) return QChar( 0x022B0 );
            if( entity == "Pscr" ) return QChar( 0x1D4AB );
            if( entity == "pscr" ) return QChar( 0x1D4C5 );
            if( entity == "Psi" ) return QChar( 0x003A8 );
            if( entity == "psi" ) return QChar( 0x003C8 );
            if( entity == "puncsp" ) return QChar( 0x02008 );
            break;
        case 'q':
            if( entity == "Qfr" ) return QChar( 0x1D514 );
            if( entity == "qfr" ) return QChar( 0x1D52E );
            if( entity == "qint" ) return QChar( 0x02A0C );
            if( entity == "Qopf" ) return QChar( 0x0211A );
            if( entity == "qopf" ) return QChar( 0x1D562 );
            if( entity == "qprime" ) return QChar( 0x02057 );
            if( entity == "Qscr" ) return QChar( 0x1D4AC );
            if( entity == "qscr" ) return QChar( 0x1D4C6 );
            if( entity == "quaternions" ) return QChar( 0x0210D );
            if( entity == "quatint" ) return QChar( 0x02A16 );
            if( entity == "quest" ) return QChar( 0x0003F );
            if( entity == "questeq" ) return QChar( 0x0225F );
            if( entity == "quot" ) return QChar( 0x00022 );
            break;
        case 'r':
            if( entity == "rAarr" ) return QChar( 0x021DB );
            if( entity == "race" ) return QChar( 0x029DA );
            if( entity == "Racute" ) return QChar( 0x00154 );
            if( entity == "racute" ) return QChar( 0x00155 );
            if( entity == "radic" ) return QChar( 0x0221A );
            if( entity == "raemptyv" ) return QChar( 0x029B3 );
            if( entity == "Rang" ) return QChar( 0x0300B );
            if( entity == "rang" ) return QChar( 0x0232A );
            if( entity == "rangd" ) return QChar( 0x02992 );
            if( entity == "range" ) return QChar( 0x029A5 );
            if( entity == "rangle" ) return QChar( 0x0232A );
            if( entity == "raquo" ) return QChar( 0x000BB );
            if( entity == "Rarr" ) return QChar( 0x021A0 );
            if( entity == "rArr" ) return QChar( 0x021D2 );
            if( entity == "rarr" ) return QChar( 0x02192 );
            if( entity == "rarrap" ) return QChar( 0x02975 );
            if( entity == "rarrb" ) return QChar( 0x021E5 );
            if( entity == "rarrbfs" ) return QChar( 0x02920 );
            if( entity == "rarrc" ) return QChar( 0x02933 );
            if( entity == "rarrfs" ) return QChar( 0x0291E );
            if( entity == "rarrhk" ) return QChar( 0x021AA );
            if( entity == "rarrlp" ) return QChar( 0x021AC );
            if( entity == "rarrpl" ) return QChar( 0x02945 );
            if( entity == "rarrsim" ) return QChar( 0x02974 );
            if( entity == "Rarrtl" ) return QChar( 0x02916 );
            if( entity == "rarrtl" ) return QChar( 0x021A3 );
            if( entity == "rarrw" ) return QChar( 0x0219D );
            if( entity == "rAtail" ) return QChar( 0x0291C );
            if( entity == "ratail" ) return QChar( 0x0291A );
            if( entity == "ratio" ) return QChar( 0x02236 );
            if( entity == "rationals" ) return QChar( 0x0211A );
            if( entity == "RBarr" ) return QChar( 0x02910 );
            if( entity == "rBarr" ) return QChar( 0x0290F );
            if( entity == "rbarr" ) return QChar( 0x0290D );
            if( entity == "rbbrk" ) return QChar( 0x03015 );
            if( entity == "rbrace" ) return QChar( 0x0007D );
            if( entity == "rbrack" ) return QChar( 0x0005D );
            if( entity == "rbrke" ) return QChar( 0x0298C );
            if( entity == "rbrksld" ) return QChar( 0x0298E );
            if( entity == "rbrkslu" ) return QChar( 0x02990 );
            if( entity == "Rcaron" ) return QChar( 0x00158 );
            if( entity == "rcaron" ) return QChar( 0x00159 );
            if( entity == "Rcedil" ) return QChar( 0x00156 );
            if( entity == "rcedil" ) return QChar( 0x00157 );
            if( entity == "rceil" ) return QChar( 0x02309 );
            if( entity == "rcub" ) return QChar( 0x0007D );
            if( entity == "Rcy" ) return QChar( 0x00420 );
            if( entity == "rcy" ) return QChar( 0x00440 );
            if( entity == "rdca" ) return QChar( 0x02937 );
            if( entity == "rdldhar" ) return QChar( 0x02969 );
            if( entity == "rdquo" ) return QChar( 0x0201D );
            if( entity == "rdquor" ) return QChar( 0x0201D );
            if( entity == "rdsh" ) return QChar( 0x021B3 );
            if( entity == "Re" ) return QChar( 0x0211C );
            if( entity == "real" ) return QChar( 0x0211C );
            if( entity == "realine" ) return QChar( 0x0211B );
            if( entity == "realpart" ) return QChar( 0x0211C );
            if( entity == "reals" ) return QChar( 0x0211D );
            if( entity == "rect" ) return QChar( 0x025AD );
            if( entity == "reg" ) return QChar( 0x000AE );
            if( entity == "ReverseElement" ) return QChar( 0x0220B );
            if( entity == "ReverseEquilibrium" ) return QChar( 0x021CB );
            if( entity == "ReverseUpEquilibrium" ) return QChar( 0x0296F );
            if( entity == "rfisht" ) return QChar( 0x0297D );
            if( entity == "rfloor" ) return QChar( 0x0230B );
            if( entity == "Rfr" ) return QChar( 0x0211C );
            if( entity == "rfr" ) return QChar( 0x1D52F );
            if( entity == "rHar" ) return QChar( 0x02964 );
            if( entity == "rhard" ) return QChar( 0x021C1 );
            if( entity == "rharu" ) return QChar( 0x021C0 );
            if( entity == "rharul" ) return QChar( 0x0296C );
            if( entity == "rho" ) return QChar( 0x003C1 );
            if( entity == "rhov" ) return QChar( 0x003F1 );
            if( entity == "RightAngleBracket" ) return QChar( 0x0232A );
            if( entity == "RightArrow" ) return QChar( 0x02192 );
            if( entity == "Rightarrow" ) return QChar( 0x021D2 );
            if( entity == "rightarrow" ) return QChar( 0x02192 );
            if( entity == "RightArrowBar" ) return QChar( 0x021E5 );
            if( entity == "RightArrowLeftArrow" ) return QChar( 0x021C4 );
            if( entity == "rightarrowtail" ) return QChar( 0x021A3 );
            if( entity == "RightCeiling" ) return QChar( 0x02309 );
            if( entity == "RightDoubleBracket" ) return QChar( 0x0301B );
            if( entity == "RightDownTeeVector" ) return QChar( 0x0295D );
            if( entity == "RightDownVector" ) return QChar( 0x021C2 );
            if( entity == "RightDownVectorBar" ) return QChar( 0x02955 );
            if( entity == "RightFloor" ) return QChar( 0x0230B );
            if( entity == "rightharpoondown" ) return QChar( 0x021C1 );
            if( entity == "rightharpoonup" ) return QChar( 0x021C0 );
            if( entity == "rightleftarrows" ) return QChar( 0x021C4 );
            if( entity == "rightleftharpoons" ) return QChar( 0x021CC );
            if( entity == "rightrightarrows" ) return QChar( 0x021C9 );
            if( entity == "rightsquigarrow" ) return QChar( 0x0219D );
            if( entity == "RightTee" ) return QChar( 0x022A2 );
            if( entity == "RightTeeArrow" ) return QChar( 0x021A6 );
            if( entity == "RightTeeVector" ) return QChar( 0x0295B );
            if( entity == "rightthreetimes" ) return QChar( 0x022CC );
            if( entity == "RightTriangle" ) return QChar( 0x022B3 );
            if( entity == "RightTriangleBar" ) return QChar( 0x029D0 );
            if( entity == "RightTriangleEqual" ) return QChar( 0x022B5 );
            if( entity == "RightUpDownVector" ) return QChar( 0x0294F );
            if( entity == "RightUpTeeVector" ) return QChar( 0x0295C );
            if( entity == "RightUpVector" ) return QChar( 0x021BE );
            if( entity == "RightUpVectorBar" ) return QChar( 0x02954 );
            if( entity == "RightVector" ) return QChar( 0x021C0 );
            if( entity == "RightVectorBar" ) return QChar( 0x02953 );
            if( entity == "ring" ) return QChar( 0x002DA );
            if( entity == "risingdotseq" ) return QChar( 0x02253 );
            if( entity == "rlarr" ) return QChar( 0x021C4 );
            if( entity == "rlhar" ) return QChar( 0x021CC );
            if( entity == "rmoust" ) return QChar( 0x023B1 );
            if( entity == "rmoustache" ) return QChar( 0x023B1 );
            if( entity == "rnmid" ) return QChar( 0x02AEE );
            if( entity == "roang" ) return QChar( 0x03019 );
            if( entity == "roarr" ) return QChar( 0x021FE );
            if( entity == "robrk" ) return QChar( 0x0301B );
            if( entity == "ropar" ) return QChar( 0x02986 );
            if( entity == "Ropf" ) return QChar( 0x0211D );
            if( entity == "ropf" ) return QChar( 0x1D563 );
            if( entity == "roplus" ) return QChar( 0x02A2E );
            if( entity == "rotimes" ) return QChar( 0x02A35 );
            if( entity == "RoundImplies" ) return QChar( 0x02970 );
            if( entity == "rpar" ) return QChar( 0x00029 );
            if( entity == "rpargt" ) return QChar( 0x02994 );
            if( entity == "rppolint" ) return QChar( 0x02A12 );
            if( entity == "rrarr" ) return QChar( 0x021C9 );
            if( entity == "Rrightarrow" ) return QChar( 0x021DB );
            if( entity == "Rscr" ) return QChar( 0x0211B );
            if( entity == "rscr" ) return QChar( 0x1D4C7 );
            if( entity == "Rsh" ) return QChar( 0x021B1 );
            if( entity == "rsh" ) return QChar( 0x021B1 );
            if( entity == "rsqb" ) return QChar( 0x0005D );
            if( entity == "rsquo" ) return QChar( 0x02019 );
            if( entity == "rsquor" ) return QChar( 0x02019 );
            if( entity == "rthree" ) return QChar( 0x022CC );
            if( entity == "rtimes" ) return QChar( 0x022CA );
            if( entity == "rtri" ) return QChar( 0x025B9 );
            if( entity == "rtrie" ) return QChar( 0x022B5 );
            if( entity == "rtrif" ) return QChar( 0x025B8 );
            if( entity == "rtriltri" ) return QChar( 0x029CE );
            if( entity == "RuleDelayed" ) return QChar( 0x029F4 );
            if( entity == "ruluhar" ) return QChar( 0x02968 );
            if( entity == "rx" ) return QChar( 0x0211E );
            break;
        case 's':
            if( entity == "Sacute" ) return QChar( 0x0015A );
            if( entity == "sacute" ) return QChar( 0x0015B );
            if( entity == "Sc" ) return QChar( 0x02ABC );
            if( entity == "sc" ) return QChar( 0x0227B );
            if( entity == "scap" ) return QChar( 0x02AB8 );
            if( entity == "Scaron" ) return QChar( 0x00160 );
            if( entity == "scaron" ) return QChar( 0x00161 );
            if( entity == "sccue" ) return QChar( 0x0227D );
            if( entity == "scE" ) return QChar( 0x02AB4 );
            if( entity == "sce" ) return QChar( 0x02AB0 );
            if( entity == "Scedil" ) return QChar( 0x0015E );
            if( entity == "scedil" ) return QChar( 0x0015F );
            if( entity == "Scirc" ) return QChar( 0x0015C );
            if( entity == "scirc" ) return QChar( 0x0015D );
            if( entity == "scnap" ) return QChar( 0x02ABA );
            if( entity == "scnE" ) return QChar( 0x02AB6 );
            if( entity == "scnsim" ) return QChar( 0x022E9 );
            if( entity == "scpolint" ) return QChar( 0x02A13 );
            if( entity == "scsim" ) return QChar( 0x0227F );
            if( entity == "Scy" ) return QChar( 0x00421 );
            if( entity == "scy" ) return QChar( 0x00441 );
            if( entity == "sdot" ) return QChar( 0x022C5 );
            if( entity == "sdotb" ) return QChar( 0x022A1 );
            if( entity == "sdote" ) return QChar( 0x02A66 );
            if( entity == "searhk" ) return QChar( 0x02925 );
            if( entity == "seArr" ) return QChar( 0x021D8 );
            if( entity == "searr" ) return QChar( 0x02198 );
            if( entity == "searrow" ) return QChar( 0x02198 );
            if( entity == "sect" ) return QChar( 0x000A7 );
            if( entity == "semi" ) return QChar( 0x0003B );
            if( entity == "seswar" ) return QChar( 0x02929 );
            if( entity == "setminus" ) return QChar( 0x02216 );
            if( entity == "setmn" ) return QChar( 0x02216 );
            if( entity == "sext" ) return QChar( 0x02736 );
            if( entity == "Sfr" ) return QChar( 0x1D516 );
            if( entity == "sfr" ) return QChar( 0x1D530 );
            if( entity == "sfrown" ) return QChar( 0x02322 );
            if( entity == "sharp" ) return QChar( 0x0266F );
            if( entity == "SHCHcy" ) return QChar( 0x00429 );
            if( entity == "shchcy" ) return QChar( 0x00449 );
            if( entity == "SHcy" ) return QChar( 0x00428 );
            if( entity == "shcy" ) return QChar( 0x00448 );
            if( entity == "ShortDownArrow" ) return QChar( 0x02193 );
            if( entity == "ShortLeftArrow" ) return QChar( 0x02190 );
            if( entity == "shortmid" ) return QChar( 0x02223 );
            if( entity == "shortparallel" ) return QChar( 0x02225 );
            if( entity == "ShortRightArrow" ) return QChar( 0x02192 );
            if( entity == "ShortUpArrow" ) return QChar( 0x02191 );
            if( entity == "shy" ) return QChar( 0x000AD );
            if( entity == "Sigma" ) return QChar( 0x003A3 );
            if( entity == "sigma" ) return QChar( 0x003C3 );
            if( entity == "sigmav" ) return QChar( 0x003C2 );
            if( entity == "sim" ) return QChar( 0x0223C );
            if( entity == "simdot" ) return QChar( 0x02A6A );
            if( entity == "sime" ) return QChar( 0x02243 );
            if( entity == "simeq" ) return QChar( 0x02243 );
            if( entity == "simg" ) return QChar( 0x02A9E );
            if( entity == "simgE" ) return QChar( 0x02AA0 );
            if( entity == "siml" ) return QChar( 0x02A9D );
            if( entity == "simlE" ) return QChar( 0x02A9F );
            if( entity == "simne" ) return QChar( 0x02246 );
            if( entity == "simplus" ) return QChar( 0x02A24 );
            if( entity == "simrarr" ) return QChar( 0x02972 );
            if( entity == "slarr" ) return QChar( 0x02190 );
            if( entity == "SmallCircle" ) return QChar( 0x02218 );
            if( entity == "smallsetminus" ) return QChar( 0x02216 );
            if( entity == "smashp" ) return QChar( 0x02A33 );
            if( entity == "smeparsl" ) return QChar( 0x029E4 );
            if( entity == "smid" ) return QChar( 0x02223 );
            if( entity == "smile" ) return QChar( 0x02323 );
            if( entity == "smt" ) return QChar( 0x02AAA );
            if( entity == "smte" ) return QChar( 0x02AAC );
            if( entity == "SOFTcy" ) return QChar( 0x0042C );
            if( entity == "softcy" ) return QChar( 0x0044C );
            if( entity == "sol" ) return QChar( 0x0002F );
            if( entity == "solb" ) return QChar( 0x029C4 );
            if( entity == "solbar" ) return QChar( 0x0233F );
            if( entity == "Sopf" ) return QChar( 0x1D54A );
            if( entity == "sopf" ) return QChar( 0x1D564 );
            if( entity == "spades" ) return QChar( 0x02660 );
            if( entity == "spadesuit" ) return QChar( 0x02660 );
            if( entity == "spar" ) return QChar( 0x02225 );
            if( entity == "sqcap" ) return QChar( 0x02293 );
            if( entity == "sqcup" ) return QChar( 0x02294 );
            if( entity == "Sqrt" ) return QChar( 0x0221A );
            if( entity == "sqsub" ) return QChar( 0x0228F );
            if( entity == "sqsube" ) return QChar( 0x02291 );
            if( entity == "sqsubset" ) return QChar( 0x0228F );
            if( entity == "sqsubseteq" ) return QChar( 0x02291 );
            if( entity == "sqsup" ) return QChar( 0x02290 );
            if( entity == "sqsupe" ) return QChar( 0x02292 );
            if( entity == "sqsupset" ) return QChar( 0x02290 );
            if( entity == "sqsupseteq" ) return QChar( 0x02292 );
            if( entity == "squ" ) return QChar( 0x025A1 );
            if( entity == "Square" ) return QChar( 0x025A1 );
            if( entity == "square" ) return QChar( 0x025A1 );
            if( entity == "SquareIntersection" ) return QChar( 0x02293 );
            if( entity == "SquareSubset" ) return QChar( 0x0228F );
            if( entity == "SquareSubsetEqual" ) return QChar( 0x02291 );
            if( entity == "SquareSuperset" ) return QChar( 0x02290 );
            if( entity == "SquareSupersetEqual" ) return QChar( 0x02292 );
            if( entity == "SquareUnion" ) return QChar( 0x02294 );
            if( entity == "squarf" ) return QChar( 0x025AA );
            if( entity == "squf" ) return QChar( 0x025AA );
            if( entity == "srarr" ) return QChar( 0x02192 );
            if( entity == "Sscr" ) return QChar( 0x1D4AE );
            if( entity == "sscr" ) return QChar( 0x1D4C8 );
            if( entity == "ssetmn" ) return QChar( 0x02216 );
            if( entity == "ssmile" ) return QChar( 0x02323 );
            if( entity == "sstarf" ) return QChar( 0x022C6 );
            if( entity == "Star" ) return QChar( 0x022C6 );
            if( entity == "star" ) return QChar( 0x02606 );
            if( entity == "starf" ) return QChar( 0x02605 );
            if( entity == "straightepsilon" ) return QChar( 0x003F5 );
            if( entity == "straightphi" ) return QChar( 0x003D5 );
            if( entity == "strns" ) return QChar( 0x000AF );
            if( entity == "Sub" ) return QChar( 0x022D0 );
            if( entity == "sub" ) return QChar( 0x02282 );
            if( entity == "subdot" ) return QChar( 0x02ABD );
            if( entity == "subE" ) return QChar( 0x02AC5 );
            if( entity == "sube" ) return QChar( 0x02286 );
            if( entity == "subedot" ) return QChar( 0x02AC3 );
            if( entity == "submult" ) return QChar( 0x02AC1 );
            if( entity == "subnE" ) return QChar( 0x02ACB );
            if( entity == "subne" ) return QChar( 0x0228A );
            if( entity == "subplus" ) return QChar( 0x02ABF );
            if( entity == "subrarr" ) return QChar( 0x02979 );
            if( entity == "Subset" ) return QChar( 0x022D0 );
            if( entity == "subset" ) return QChar( 0x02282 );
            if( entity == "subseteq" ) return QChar( 0x02286 );
            if( entity == "subseteqq" ) return QChar( 0x02AC5 );
            if( entity == "SubsetEqual" ) return QChar( 0x02286 );
            if( entity == "subsetneq" ) return QChar( 0x0228A );
            if( entity == "subsetneqq" ) return QChar( 0x02ACB );
            if( entity == "subsim" ) return QChar( 0x02AC7 );
            if( entity == "subsub" ) return QChar( 0x02AD5 );
            if( entity == "subsup" ) return QChar( 0x02AD3 );
            if( entity == "succ" ) return QChar( 0x0227B );
            if( entity == "succapprox" ) return QChar( 0x02AB8 );
            if( entity == "succcurlyeq" ) return QChar( 0x0227D );
            if( entity == "Succeeds" ) return QChar( 0x0227B );
            if( entity == "SucceedsEqual" ) return QChar( 0x02AB0 );
            if( entity == "SucceedsSlantEqual" ) return QChar( 0x0227D );
            if( entity == "SucceedsTilde" ) return QChar( 0x0227F );
            if( entity == "succeq" ) return QChar( 0x02AB0 );
            if( entity == "succnapprox" ) return QChar( 0x02ABA );
            if( entity == "succneqq" ) return QChar( 0x02AB6 );
            if( entity == "succnsim" ) return QChar( 0x022E9 );
            if( entity == "succsim" ) return QChar( 0x0227F );
            if( entity == "SuchThat" ) return QChar( 0x0220B );
            if( entity == "Sum" ) return QChar( 0x02211 );
            if( entity == "sum" ) return QChar( 0x02211 );
            if( entity == "sung" ) return QChar( 0x0266A );
            if( entity == "Sup" ) return QChar( 0x022D1 );
            if( entity == "sup" ) return QChar( 0x02283 );
            if( entity == "sup1" ) return QChar( 0x000B9 );
            if( entity == "sup2" ) return QChar( 0x000B2 );
            if( entity == "sup3" ) return QChar( 0x000B3 );
            if( entity == "supdot" ) return QChar( 0x02ABE );
            if( entity == "supdsub" ) return QChar( 0x02AD8 );
            if( entity == "supE" ) return QChar( 0x02AC6 );
            if( entity == "supe" ) return QChar( 0x02287 );
            if( entity == "supedot" ) return QChar( 0x02AC4 );
            if( entity == "Superset" ) return QChar( 0x02283 );
            if( entity == "SupersetEqual" ) return QChar( 0x02287 );
            if( entity == "suphsub" ) return QChar( 0x02AD7 );
            if( entity == "suplarr" ) return QChar( 0x0297B );
            if( entity == "supmult" ) return QChar( 0x02AC2 );
            if( entity == "supnE" ) return QChar( 0x02ACC );
            if( entity == "supne" ) return QChar( 0x0228B );
            if( entity == "supplus" ) return QChar( 0x02AC0 );
            if( entity == "Supset" ) return QChar( 0x022D1 );
            if( entity == "supset" ) return QChar( 0x02283 );
            if( entity == "supseteq" ) return QChar( 0x02287 );
            if( entity == "supseteqq" ) return QChar( 0x02AC6 );
            if( entity == "supsetneq" ) return QChar( 0x0228B );
            if( entity == "supsetneqq" ) return QChar( 0x02ACC );
            if( entity == "supsim" ) return QChar( 0x02AC8 );
            if( entity == "supsub" ) return QChar( 0x02AD4 );
            if( entity == "supsup" ) return QChar( 0x02AD6 );
            if( entity == "swarhk" ) return QChar( 0x02926 );
            if( entity == "swArr" ) return QChar( 0x021D9 );
            if( entity == "swarr" ) return QChar( 0x02199 );
            if( entity == "swarrow" ) return QChar( 0x02199 );
            if( entity == "swnwar" ) return QChar( 0x0292A );
            if( entity == "szlig" ) return QChar( 0x000DF );
            break;
        case 't':
            if( entity == "Tab" ) return QChar( 0x00009 );
            if( entity == "target" ) return QChar( 0x02316 );
            if( entity == "tau" ) return QChar( 0x003C4 );
            if( entity == "tbrk" ) return QChar( 0x023B4 );
            if( entity == "Tcaron" ) return QChar( 0x00164 );
            if( entity == "tcaron" ) return QChar( 0x00165 );
            if( entity == "Tcedil" ) return QChar( 0x00162 );
            if( entity == "tcedil" ) return QChar( 0x00163 );
            if( entity == "Tcy" ) return QChar( 0x00422 );
            if( entity == "tcy" ) return QChar( 0x00442 );
            if( entity == "tdot" ) return QChar( 0x020DB );
            if( entity == "telrec" ) return QChar( 0x02315 );
            if( entity == "Tfr" ) return QChar( 0x1D517 );
            if( entity == "tfr" ) return QChar( 0x1D531 );
            if( entity == "there4" ) return QChar( 0x02234 );
            if( entity == "Therefore" ) return QChar( 0x02234 );
            if( entity == "therefore" ) return QChar( 0x02234 );
            if( entity == "Theta" ) return QChar( 0x00398 );
            if( entity == "theta" ) return QChar( 0x003B8 );
            if( entity == "thetav" ) return QChar( 0x003D1 );
            if( entity == "thickapprox" ) return QChar( 0x02248 );
            if( entity == "thicksim" ) return QChar( 0x0223C );
            if( entity == "thinsp" ) return QChar( 0x02009 );
            if( entity == "ThinSpace" ) return QChar( 0x02009 );
            if( entity == "thkap" ) return QChar( 0x02248 );
            if( entity == "thksim" ) return QChar( 0x0223C );
            if( entity == "THORN" ) return QChar( 0x000DE );
            if( entity == "thorn" ) return QChar( 0x000FE );
            if( entity == "Tilde" ) return QChar( 0x0223C );
            if( entity == "tilde" ) return QChar( 0x002DC );
            if( entity == "TildeEqual" ) return QChar( 0x02243 );
            if( entity == "TildeFullEqual" ) return QChar( 0x02245 );
            if( entity == "TildeTilde" ) return QChar( 0x02248 );
            if( entity == "times" ) return QChar( 0x000D7 );
            if( entity == "timesb" ) return QChar( 0x022A0 );
            if( entity == "timesbar" ) return QChar( 0x02A31 );
            if( entity == "timesd" ) return QChar( 0x02A30 );
            if( entity == "tint" ) return QChar( 0x0222D );
            if( entity == "toea" ) return QChar( 0x02928 );
            if( entity == "top" ) return QChar( 0x022A4 );
            if( entity == "topbot" ) return QChar( 0x02336 );
            if( entity == "topcir" ) return QChar( 0x02AF1 );
            if( entity == "Topf" ) return QChar( 0x1D54B );
            if( entity == "topf" ) return QChar( 0x1D565 );
            if( entity == "topfork" ) return QChar( 0x02ADA );
            if( entity == "tosa" ) return QChar( 0x02929 );
            if( entity == "tprime" ) return QChar( 0x02034 );
            if( entity == "trade" ) return QChar( 0x02122 );
            if( entity == "triangle" ) return QChar( 0x025B5 );
            if( entity == "triangledown" ) return QChar( 0x025BF );
            if( entity == "triangleleft" ) return QChar( 0x025C3 );
            if( entity == "trianglelefteq" ) return QChar( 0x022B4 );
            if( entity == "triangleq" ) return QChar( 0x0225C );
            if( entity == "triangleright" ) return QChar( 0x025B9 );
            if( entity == "trianglerighteq" ) return QChar( 0x022B5 );
            if( entity == "tridot" ) return QChar( 0x025EC );
            if( entity == "trie" ) return QChar( 0x0225C );
            if( entity == "triminus" ) return QChar( 0x02A3A );
            if( entity == "TripleDot" ) return QChar( 0x020DB );
            if( entity == "triplus" ) return QChar( 0x02A39 );
            if( entity == "trisb" ) return QChar( 0x029CD );
            if( entity == "tritime" ) return QChar( 0x02A3B );
            if( entity == "trpezium" ) return QChar( 0x0FFFD );
            if( entity == "Tscr" ) return QChar( 0x1D4AF );
            if( entity == "tscr" ) return QChar( 0x1D4C9 );
            if( entity == "TScy" ) return QChar( 0x00426 );
            if( entity == "tscy" ) return QChar( 0x00446 );
            if( entity == "TSHcy" ) return QChar( 0x0040B );
            if( entity == "tshcy" ) return QChar( 0x0045B );
            if( entity == "Tstrok" ) return QChar( 0x00166 );
            if( entity == "tstrok" ) return QChar( 0x00167 );
            if( entity == "twixt" ) return QChar( 0x0226C );
            if( entity == "twoheadleftarrow" ) return QChar( 0x0219E );
            if( entity == "twoheadrightarrow" ) return QChar( 0x021A0 );
            break;
        case 'u':
            if( entity == "Uacute" ) return QChar( 0x000DA );
            if( entity == "uacute" ) return QChar( 0x000FA );
            if( entity == "Uarr" ) return QChar( 0x0219F );
            if( entity == "uArr" ) return QChar( 0x021D1 );
            if( entity == "uarr" ) return QChar( 0x02191 );
            if( entity == "Uarrocir" ) return QChar( 0x02949 );
            if( entity == "Ubrcy" ) return QChar( 0x0040E );
            if( entity == "ubrcy" ) return QChar( 0x0045E );
            if( entity == "Ubreve" ) return QChar( 0x0016C );
            if( entity == "ubreve" ) return QChar( 0x0016D );
            if( entity == "Ucirc" ) return QChar( 0x000DB );
            if( entity == "ucirc" ) return QChar( 0x000FB );
            if( entity == "Ucy" ) return QChar( 0x00423 );
            if( entity == "ucy" ) return QChar( 0x00443 );
            if( entity == "udarr" ) return QChar( 0x021C5 );
            if( entity == "Udblac" ) return QChar( 0x00170 );
            if( entity == "udblac" ) return QChar( 0x00171 );
            if( entity == "udhar" ) return QChar( 0x0296E );
            if( entity == "ufisht" ) return QChar( 0x0297E );
            if( entity == "Ufr" ) return QChar( 0x1D518 );
            if( entity == "ufr" ) return QChar( 0x1D532 );
            if( entity == "Ugrave" ) return QChar( 0x000D9 );
            if( entity == "ugrave" ) return QChar( 0x000F9 );
            if( entity == "uHar" ) return QChar( 0x02963 );
            if( entity == "uharl" ) return QChar( 0x021BF );
            if( entity == "uharr" ) return QChar( 0x021BE );
            if( entity == "uhblk" ) return QChar( 0x02580 );
            if( entity == "ulcorn" ) return QChar( 0x0231C );
            if( entity == "ulcorner" ) return QChar( 0x0231C );
            if( entity == "ulcrop" ) return QChar( 0x0230F );
            if( entity == "ultri" ) return QChar( 0x025F8 );
            if( entity == "Umacr" ) return QChar( 0x0016A );
            if( entity == "umacr" ) return QChar( 0x0016B );
            if( entity == "uml" ) return QChar( 0x000A8 );
            if( entity == "UnderBar" ) return QChar( 0x00332 );
            if( entity == "UnderBrace" ) return QChar( 0x0FE38 );
            if( entity == "UnderBracket" ) return QChar( 0x023B5 );
            if( entity == "UnderParenthesis" ) return QChar( 0x0FE36 );
            if( entity == "Union" ) return QChar( 0x022C3 );
            if( entity == "UnionPlus" ) return QChar( 0x0228E );
            if( entity == "Uogon" ) return QChar( 0x00172 );
            if( entity == "uogon" ) return QChar( 0x00173 );
            if( entity == "Uopf" ) return QChar( 0x1D54C );
            if( entity == "uopf" ) return QChar( 0x1D566 );
            if( entity == "UpArrow" ) return QChar( 0x02191 );
            if( entity == "Uparrow" ) return QChar( 0x021D1 );
            if( entity == "uparrow" ) return QChar( 0x02191 );
            if( entity == "UpArrowBar" ) return QChar( 0x02912 );
            if( entity == "UpArrowDownArrow" ) return QChar( 0x021C5 );
            if( entity == "UpDownArrow" ) return QChar( 0x02195 );
            if( entity == "Updownarrow" ) return QChar( 0x021D5 );
            if( entity == "updownarrow" ) return QChar( 0x02195 );
            if( entity == "UpEquilibrium" ) return QChar( 0x0296E );
            if( entity == "upharpoonleft" ) return QChar( 0x021BF );
            if( entity == "upharpoonright" ) return QChar( 0x021BE );
            if( entity == "uplus" ) return QChar( 0x0228E );
            if( entity == "UpperLeftArrow" ) return QChar( 0x02196 );
            if( entity == "UpperRightArrow" ) return QChar( 0x02197 );
            if( entity == "Upsi" ) return QChar( 0x003D2 );
            if( entity == "upsi" ) return QChar( 0x003C5 );
            if( entity == "Upsilon" ) return QChar( 0x003A5 );
            if( entity == "upsilon" ) return QChar( 0x003C5 );
            if( entity == "UpTee" ) return QChar( 0x022A5 );
            if( entity == "UpTeeArrow" ) return QChar( 0x021A5 );
            if( entity == "upuparrows" ) return QChar( 0x021C8 );
            if( entity == "urcorn" ) return QChar( 0x0231D );
            if( entity == "urcorner" ) return QChar( 0x0231D );
            if( entity == "urcrop" ) return QChar( 0x0230E );
            if( entity == "Uring" ) return QChar( 0x0016E );
            if( entity == "uring" ) return QChar( 0x0016F );
            if( entity == "urtri" ) return QChar( 0x025F9 );
            if( entity == "Uscr" ) return QChar( 0x1D4B0 );
            if( entity == "uscr" ) return QChar( 0x1D4CA );
            if( entity == "utdot" ) return QChar( 0x022F0 );
            if( entity == "Utilde" ) return QChar( 0x00168 );
            if( entity == "utilde" ) return QChar( 0x00169 );
            if( entity == "utri" ) return QChar( 0x025B5 );
            if( entity == "utrif" ) return QChar( 0x025B4 );
            if( entity == "uuarr" ) return QChar( 0x021C8 );
            if( entity == "Uuml" ) return QChar( 0x000DC );
            if( entity == "uuml" ) return QChar( 0x000FC );
            if( entity == "uwangle" ) return QChar( 0x029A7 );
            break;
        case 'v':
            if( entity == "vangrt" ) return QChar( 0x0299C );
            if( entity == "varepsilon" ) return QChar( 0x003B5 );
            if( entity == "varkappa" ) return QChar( 0x003F0 );
            if( entity == "varnothing" ) return QChar( 0x02205 );
            if( entity == "varphi" ) return QChar( 0x003C6 );
            if( entity == "varpi" ) return QChar( 0x003D6 );
            if( entity == "varpropto" ) return QChar( 0x0221D );
            if( entity == "vArr" ) return QChar( 0x021D5 );
            if( entity == "varr" ) return QChar( 0x02195 );
            if( entity == "varrho" ) return QChar( 0x003F1 );
            if( entity == "varsigma" ) return QChar( 0x003C2 );
            if( entity == "vartheta" ) return QChar( 0x003D1 );
            if( entity == "vartriangleleft" ) return QChar( 0x022B2 );
            if( entity == "vartriangleright" ) return QChar( 0x022B3 );
            if( entity == "Vbar" ) return QChar( 0x02AEB );
            if( entity == "vBar" ) return QChar( 0x02AE8 );
            if( entity == "vBarv" ) return QChar( 0x02AE9 );
            if( entity == "Vcy" ) return QChar( 0x00412 );
            if( entity == "vcy" ) return QChar( 0x00432 );
            if( entity == "VDash" ) return QChar( 0x022AB );
            if( entity == "Vdash" ) return QChar( 0x022A9 );
            if( entity == "vDash" ) return QChar( 0x022A8 );
            if( entity == "vdash" ) return QChar( 0x022A2 );
            if( entity == "Vdashl" ) return QChar( 0x02AE6 );
            if( entity == "Vee" ) return QChar( 0x022C1 );
            if( entity == "vee" ) return QChar( 0x02228 );
            if( entity == "veebar" ) return QChar( 0x022BB );
            if( entity == "veeeq" ) return QChar( 0x0225A );
            if( entity == "vellip" ) return QChar( 0x022EE );
            if( entity == "Verbar" ) return QChar( 0x02016 );
            if( entity == "verbar" ) return QChar( 0x0007C );
            if( entity == "Vert" ) return QChar( 0x02016 );
            if( entity == "vert" ) return QChar( 0x0007C );
            if( entity == "VerticalBar" ) return QChar( 0x02223 );
            if( entity == "VerticalLine" ) return QChar( 0x0007C );
            if( entity == "VerticalSeparator" ) return QChar( 0x02758 );
            if( entity == "VerticalTilde" ) return QChar( 0x02240 );
            if( entity == "VeryThinSpace" ) return QChar( 0x0200A );
            if( entity == "Vfr" ) return QChar( 0x1D519 );
            if( entity == "vfr" ) return QChar( 0x1D533 );
            if( entity == "vltri" ) return QChar( 0x022B2 );
            if( entity == "Vopf" ) return QChar( 0x1D54D );
            if( entity == "vopf" ) return QChar( 0x1D567 );
            if( entity == "vprop" ) return QChar( 0x0221D );
            if( entity == "vrtri" ) return QChar( 0x022B3 );
            if( entity == "Vscr" ) return QChar( 0x1D4B1 );
            if( entity == "vscr" ) return QChar( 0x1D4CB );
            if( entity == "Vvdash" ) return QChar( 0x022AA );
            if( entity == "vzigzag" ) return QChar( 0x0299A );
            break;
        case 'w':
            if( entity == "Wcirc" ) return QChar( 0x00174 );
            if( entity == "wcirc" ) return QChar( 0x00175 );
            if( entity == "wedbar" ) return QChar( 0x02A5F );
            if( entity == "Wedge" ) return QChar( 0x022C0 );
            if( entity == "wedge" ) return QChar( 0x02227 );
            if( entity == "wedgeq" ) return QChar( 0x02259 );
            if( entity == "weierp" ) return QChar( 0x02118 );
            if( entity == "Wfr" ) return QChar( 0x1D51A );
            if( entity == "wfr" ) return QChar( 0x1D534 );
            if( entity == "Wopf" ) return QChar( 0x1D54E );
            if( entity == "wopf" ) return QChar( 0x1D568 );
            if( entity == "wp" ) return QChar( 0x02118 );
            if( entity == "wr" ) return QChar( 0x02240 );
            if( entity == "wreath" ) return QChar( 0x02240 );
            if( entity == "Wscr" ) return QChar( 0x1D4B2 );
            if( entity == "wscr" ) return QChar( 0x1D4CC );
            break;
        case 'x':
            if( entity == "xcap" ) return QChar( 0x022C2 );
            if( entity == "xcirc" ) return QChar( 0x025EF );
            if( entity == "xcup" ) return QChar( 0x022C3 );
            if( entity == "xdtri" ) return QChar( 0x025BD );
            if( entity == "Xfr" ) return QChar( 0x1D51B );
            if( entity == "xfr" ) return QChar( 0x1D535 );
            if( entity == "xhArr" ) return QChar( 0x027FA );
            if( entity == "xharr" ) return QChar( 0x027F7 );
            if( entity == "Xi" ) return QChar( 0x0039E );
            if( entity == "xi" ) return QChar( 0x003BE );
            if( entity == "xlArr" ) return QChar( 0x027F8 );
            if( entity == "xlarr" ) return QChar( 0x027F5 );
            if( entity == "xmap" ) return QChar( 0x027FC );
            if( entity == "xnis" ) return QChar( 0x022FB );
            if( entity == "xodot" ) return QChar( 0x02A00 );
            if( entity == "Xopf" ) return QChar( 0x1D54F );
            if( entity == "xopf" ) return QChar( 0x1D569 );
            if( entity == "xoplus" ) return QChar( 0x02A01 );
            if( entity == "xotime" ) return QChar( 0x02A02 );
            if( entity == "xrArr" ) return QChar( 0x027F9 );
            if( entity == "xrarr" ) return QChar( 0x027F6 );
            if( entity == "Xscr" ) return QChar( 0x1D4B3 );
            if( entity == "xscr" ) return QChar( 0x1D4CD );
            if( entity == "xsqcup" ) return QChar( 0x02A06 );
            if( entity == "xuplus" ) return QChar( 0x02A04 );
            if( entity == "xutri" ) return QChar( 0x025B3 );
            if( entity == "xvee" ) return QChar( 0x022C1 );
            if( entity == "xwedge" ) return QChar( 0x022C0 );
            break;
        case 'y':
            if( entity == "Yacute" ) return QChar( 0x000DD );
            if( entity == "yacute" ) return QChar( 0x000FD );
            if( entity == "YAcy" ) return QChar( 0x0042F );
            if( entity == "yacy" ) return QChar( 0x0044F );
            if( entity == "Ycirc" ) return QChar( 0x00176 );
            if( entity == "ycirc" ) return QChar( 0x00177 );
            if( entity == "Ycy" ) return QChar( 0x0042B );
            if( entity == "ycy" ) return QChar( 0x0044B );
            if( entity == "yen" ) return QChar( 0x000A5 );
            if( entity == "Yfr" ) return QChar( 0x1D51C );
            if( entity == "yfr" ) return QChar( 0x1D536 );
            if( entity == "YIcy" ) return QChar( 0x00407 );
            if( entity == "yicy" ) return QChar( 0x00457 );
            if( entity == "Yopf" ) return QChar( 0x1D550 );
            if( entity == "yopf" ) return QChar( 0x1D56A );
            if( entity == "Yscr" ) return QChar( 0x1D4B4 );
            if( entity == "yscr" ) return QChar( 0x1D4CE );
            if( entity == "YUcy" ) return QChar( 0x0042E );
            if( entity == "yucy" ) return QChar( 0x0044E );
            if( entity == "Yuml" ) return QChar( 0x00178 );
            if( entity == "yuml" ) return QChar( 0x000FF );
            break;
        case 'z':
            if( entity == "Zacute" ) return QChar( 0x00179 );
            if( entity == "zacute" ) return QChar( 0x0017A );
            if( entity == "Zcaron" ) return QChar( 0x0017D );
            if( entity == "zcaron" ) return QChar( 0x0017E );
            if( entity == "Zcy" ) return QChar( 0x00417 );
            if( entity == "zcy" ) return QChar( 0x00437 );
            if( entity == "Zdot" ) return QChar( 0x0017B );
            if( entity == "zdot" ) return QChar( 0x0017C );
            if( entity == "zeetrf" ) return QChar( 0x02128 );
            if( entity == "ZeroWidthSpace" ) return QChar( 0x0200B );
            if( entity == "zeta" ) return QChar( 0x003B6 );
            if( entity == "Zfr" ) return QChar( 0x02128 );
            if( entity == "zfr" ) return QChar( 0x1D537 );
            if( entity == "ZHcy" ) return QChar( 0x00416 );
            if( entity == "zhcy" ) return QChar( 0x00436 );
            if( entity == "zigrarr" ) return QChar( 0x021DD );
            if( entity == "Zopf" ) return QChar( 0x02124 );
            if( entity == "zopf" ) return QChar( 0x1D56B );
            if( entity == "Zscr" ) return QChar( 0x1D4B5 );
            if( entity == "zscr" ) return QChar( 0x1D4CF );
            break;
        default:
            break;
    }
    return QChar();
}

bool Dictionary::queryOperator( const QString& queriedOperator, Form form )
{
    if( queriedOperator.isEmpty() || queriedOperator.isNull() )
        return false;
    if( queriedOperator == "(" && form == Prefix ) {
        m_fence = true;
        m_stretchy = true;
        m_lspace = "0em";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == ")" && form == Postfix ) {
        m_fence = true;
        m_stretchy = true;
        m_lspace = "0em";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == "[" && form == Prefix ) {
        m_fence = true;
        m_stretchy = true;
        m_lspace = "0em";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == "]" && form == Postfix ) {
        m_fence = true;
        m_stretchy = true;
        m_lspace = "0em";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == "{" && form == Prefix ) {
        m_fence = true;
        m_stretchy = true;
        m_lspace = "0em";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == "}" && form == Postfix ) {
        m_fence = true;
        m_stretchy = true;
        m_lspace = "0em";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == "&CloseCurlyDoubleQuote;" && form == Postfix ) {
        m_fence = true;
        m_lspace = "0em";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == "&CloseCurlyQuote;" && form == Postfix ) {
        m_fence = true;
        m_lspace = "0em";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == "&LeftAngleBracket;" && form == Prefix ) {
        m_fence = true;
        m_stretchy = true;
        m_lspace = "0em";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == "&LeftCeiling;" && form == Prefix ) {
        m_fence = true;
        m_stretchy = true;
        m_lspace = "0em";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == "&LeftDoubleBracket;" && form == Prefix ) {
        m_fence = true;
        m_stretchy = true;
        m_lspace = "0em";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == "&LeftFloor;" && form == Prefix ) {
        m_fence = true;
        m_stretchy = true;
        m_lspace = "0em";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == "&OpenCurlyDoubleQuote;" && form == Prefix ) {
        m_fence = true;
        m_lspace = "0em";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == "&OpenCurlyQuote;" && form == Prefix ) {
        m_fence = true;
        m_lspace = "0em";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == "&RightAngleBracket;" && form == Postfix ) {
        m_fence = true;
        m_stretchy = true;
        m_lspace = "0em";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == "&RightCeiling;" && form == Postfix ) {
        m_fence = true;
        m_stretchy = true;
        m_lspace = "0em";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == "&RightDoubleBracket;" && form == Postfix ) {
        m_fence = true;
        m_stretchy = true;
        m_lspace = "0em";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == "&RightFloor;" && form == Postfix ) {
        m_fence = true;
        m_stretchy = true;
        m_lspace = "0em";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == "&InvisibleComma;" && form == Infix ) {
        m_separator = true;
        m_lspace = "0em";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == "," && form == Infix ) {
        m_separator = true;
        m_lspace = "0em";
        m_rspace = "verythickmathspace";
        return true;
    }
    if( queriedOperator == "&HorizontalLine;" && form == Infix ) {
        m_stretchy = true;
        m_minsize = "0";
        m_lspace = "0em";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == "&VerticalLine;" && form == Infix ) {
        m_stretchy = true;
        m_minsize = "0";
        m_lspace = "0em";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == ";" && form == Infix ) {
        m_separator = true;
        m_lspace = "0em";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == ";" && form == Postfix ) {
        m_separator = true;
        m_lspace = "0em";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == ":=" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&Assign;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&Because;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&Therefore;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&VerticalSeparator;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "//" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&Colon;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&amp;" && form == Prefix ) {
        m_lspace = "0em";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&amp;" && form == Postfix ) {
        m_lspace = "thickmathspace";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == "*=" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "-=" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "+=" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "/=" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "->" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == ":" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == ".." && form == Postfix ) {
        m_lspace = "mediummathspace";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == "..." && form == Postfix ) {
        m_lspace = "mediummathspace";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == "&SuchThat;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&DoubleLeftTee;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&DoubleRightTee;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&DownTee;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&LeftTee;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&RightTee;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&Implies;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&RoundImplies;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "|" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "||" && form == Infix ) {
        m_lspace = "mediummathspace";
        m_rspace = "mediummathspace";
        return true;
    }
    if( queriedOperator == "&Or;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "mediummathspace";
        m_rspace = "mediummathspace";
        return true;
    }
    if( queriedOperator == "&amp;&amp;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&And;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "mediummathspace";
        m_rspace = "mediummathspace";
        return true;
    }
    if( queriedOperator == "&amp;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "!" && form == Prefix ) {
        m_lspace = "0em";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&Not;" && form == Prefix ) {
        m_lspace = "0em";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&Exists;" && form == Prefix ) {
        m_lspace = "0em";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&ForAll;" && form == Prefix ) {
        m_lspace = "0em";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&NotExists;" && form == Prefix ) {
        m_lspace = "0em";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&Element;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&NotElement;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&NotReverseElement;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&NotSquareSubset;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&NotSquareSubsetEqual;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&NotSquareSuperset;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&NotSquareSupersetEqual;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&NotSubset;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&NotSubsetEqual;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&NotSuperset;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&NotSupersetEqual;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&ReverseElement;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&SquareSubset;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&SquareSubsetEqual;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&SquareSuperset;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&SquareSupersetEqual;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&Subset;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&SubsetEqual;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&Superset;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&SupersetEqual;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&DoubleLeftArrow;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&DoubleLeftRightArrow;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&DoubleRightArrow;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&DownLeftRightVector;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&DownLeftTeeVector;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&DownLeftVector;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&DownLeftVectorBar;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&DownRightTeeVector;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&DownRightVector;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&DownRightVectorBar;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&LeftArrow;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&LeftArrowBar;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&LeftArrowRightArrow;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&LeftRightArrow;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&LeftRightVector;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&LeftTeeArrow;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&LeftTeeVector;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&LeftVector;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&LeftVectorBar;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&LowerLeftArrow;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&LowerRightArrow;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&RightArrow;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&RightArrowBar;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&RightArrowLeftArrow;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&RightTeeArrow;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&RightTeeVector;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&RightVector;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&RightVectorBar;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&ShortLeftArrow;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&ShortRightArrow;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&UpperLeftArrow;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&UpperRightArrow;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "=" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&lt;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == ">" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "!=" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "==" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&lt;=" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == ">=" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&Congruent;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&CupCap;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&DotEqual;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&DoubleVerticalBar;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&Equal;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&EqualTilde;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&Equilibrium;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&GreaterEqual;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&GreaterEqualLess;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&GreaterFullEqual;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&GreaterGreater;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&GreaterLess;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&GreaterSlantEqual;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&GreaterTilde;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&HumpDownHump;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&HumpEqual;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&LeftTriangle;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&LeftTriangleBar;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&LeftTriangleEqual;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&le;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&LessEqualGreater;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&LessFullEqual;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&LessGreater;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&LessLess;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&LessSlantEqual;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&LessTilde;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&NestedGreaterGreater;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&NestedLessLess;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&NotCongruent;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&NotCupCap;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&NotDoubleVerticalBar;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&NotEqual;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&NotEqualTilde;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&NotGreater;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&NotGreaterEqual;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&NotGreaterFullEqual;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&NotGreaterGreater;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&NotGreaterLess;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&NotGreaterSlantEqual;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&NotGreaterTilde;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&NotHumpDownHump;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&NotHumpEqual;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&NotLeftTriangle;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&NotLeftTriangleBar;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&NotLeftTriangleEqual;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&NotLess;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&NotLessEqual;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&NotLessGreater;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&NotLessLess;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&NotLessSlantEqual;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&NotLessTilde;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&NotNestedGreaterGreater;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&NotNestedLessLess;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&NotPrecedes;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&NotPrecedesEqual;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&NotPrecedesSlantEqual;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&NotRightTriangle;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&NotRightTriangleBar;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&NotRightTriangleEqual;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&NotSucceeds;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&NotSucceedsEqual;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&NotSucceedsSlantEqual;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&NotSucceedsTilde;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&NotTilde;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&NotTildeEqual;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&NotTildeFullEqual;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&NotTildeTilde;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&NotVerticalBar;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&Precedes;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&PrecedesEqual;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&PrecedesSlantEqual;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&PrecedesTilde;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&Proportion;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&Proportional;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&ReverseEquilibrium;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&RightTriangle;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&RightTriangleBar;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&RightTriangleEqual;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&Succeeds;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&SucceedsEqual;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&SucceedsSlantEqual;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&SucceedsTilde;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&Tilde;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&TildeEqual;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&TildeFullEqual;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&TildeTilde;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&UpTee;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&VerticalBar;" && form == Infix ) {
        m_lspace = "thickmathspace";
        m_rspace = "thickmathspace";
        return true;
    }
    if( queriedOperator == "&SquareUnion;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "mediummathspace";
        m_rspace = "mediummathspace";
        return true;
    }
    if( queriedOperator == "&Union;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "mediummathspace";
        m_rspace = "mediummathspace";
        return true;
    }
    if( queriedOperator == "&UnionPlus;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "mediummathspace";
        m_rspace = "mediummathspace";
        return true;
    }
    if( queriedOperator == "-" && form == Infix ) {
        m_lspace = "mediummathspace";
        m_rspace = "mediummathspace";
        return true;
    }
    if( queriedOperator == "+" && form == Infix ) {
        m_lspace = "mediummathspace";
        m_rspace = "mediummathspace";
        return true;
    }
    if( queriedOperator == "&Intersection;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "mediummathspace";
        m_rspace = "mediummathspace";
        return true;
    }
    if( queriedOperator == "&MinusPlus;" && form == Infix ) {
        m_lspace = "mediummathspace";
        m_rspace = "mediummathspace";
        return true;
    }
    if( queriedOperator == "&PlusMinus;" && form == Infix ) {
        m_lspace = "mediummathspace";
        m_rspace = "mediummathspace";
        return true;
    }
    if( queriedOperator == "&SquareIntersection;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "mediummathspace";
        m_rspace = "mediummathspace";
        return true;
    }
    if( queriedOperator == "&Vee;" && form == Prefix ) {
        m_largeop = true;
        m_movablelimits = true;
        m_stretchy = true;
        m_lspace = "0em";
        m_rspace = "thinmathspace";
        return true;
    }
    if( queriedOperator == "&CircleMinus;" && form == Prefix ) {
        m_largeop = true;
        m_movablelimits = true;
        m_lspace = "0em";
        m_rspace = "thinmathspace";
        return true;
    }
    if( queriedOperator == "&CirclePlus;" && form == Prefix ) {
        m_largeop = true;
        m_movablelimits = true;
        m_lspace = "0em";
        m_rspace = "thinmathspace";
        return true;
    }
    if( queriedOperator == "&Sum;" && form == Prefix ) {
        m_largeop = true;
        m_movablelimits = true;
        m_stretchy = true;
        m_lspace = "0em";
        m_rspace = "thinmathspace";
        return true;
    }
    if( queriedOperator == "&Union;" && form == Prefix ) {
        m_largeop = true;
        m_movablelimits = true;
        m_stretchy = true;
        m_lspace = "0em";
        m_rspace = "thinmathspace";
        return true;
    }
    if( queriedOperator == "&UnionPlus;" && form == Prefix ) {
        m_largeop = true;
        m_movablelimits = true;
        m_stretchy = true;
        m_lspace = "0em";
        m_rspace = "thinmathspace";
        return true;
    }
    if( queriedOperator == "lim" && form == Prefix ) {
        m_movablelimits = true;
        m_lspace = "0em";
        m_rspace = "thinmathspace";
        return true;
    }
    if( queriedOperator == "max" && form == Prefix ) {
        m_movablelimits = true;
        m_lspace = "0em";
        m_rspace = "thinmathspace";
        return true;
    }
    if( queriedOperator == "min" && form == Prefix ) {
        m_movablelimits = true;
        m_lspace = "0em";
        m_rspace = "thinmathspace";
        return true;
    }
    if( queriedOperator == "&CircleMinus;" && form == Infix ) {
        m_lspace = "thinmathspace";
        m_rspace = "thinmathspace";
        return true;
    }
    if( queriedOperator == "&CirclePlus;" && form == Infix ) {
        m_lspace = "thinmathspace";
        m_rspace = "thinmathspace";
        return true;
    }
    if( queriedOperator == "&ClockwiseContourIntegral;" && form == Prefix ) {
        m_largeop = true;
        m_stretchy = true;
        m_lspace = "0em";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == "&ContourIntegral;" && form == Prefix ) {
        m_largeop = true;
        m_stretchy = true;
        m_lspace = "0em";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == "&CounterClockwiseContourIntegral;" && form == Prefix ) {
        m_largeop = true;
        m_stretchy = true;
        m_lspace = "0em";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == "&DoubleContourIntegral;" && form == Prefix ) {
        m_largeop = true;
        m_stretchy = true;
        m_lspace = "0em";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == "&Integral;" && form == Prefix ) {
        m_largeop = true;
        m_stretchy = true;
        m_lspace = "0em";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == "&Cup;" && form == Infix ) {
        m_lspace = "thinmathspace";
        m_rspace = "thinmathspace";
        return true;
    }
    if( queriedOperator == "&Cap;" && form == Infix ) {
        m_lspace = "thinmathspace";
        m_rspace = "thinmathspace";
        return true;
    }
    if( queriedOperator == "&VerticalTilde;" && form == Infix ) {
        m_lspace = "thinmathspace";
        m_rspace = "thinmathspace";
        return true;
    }
    if( queriedOperator == "&Wedge;" && form == Prefix ) {
        m_largeop = true;
        m_movablelimits = true;
        m_stretchy = true;
        m_lspace = "0em";
        m_rspace = "thinmathspace";
        return true;
    }
    if( queriedOperator == "&CircleTimes;" && form == Prefix ) {
        m_largeop = true;
        m_movablelimits = true;
        m_lspace = "0em";
        m_rspace = "thinmathspace";
        return true;
    }
    if( queriedOperator == "&Coproduct;" && form == Prefix ) {
        m_largeop = true;
        m_movablelimits = true;
        m_stretchy = true;
        m_lspace = "0em";
        m_rspace = "thinmathspace";
        return true;
    }
    if( queriedOperator == "&Product;" && form == Prefix ) {
        m_largeop = true;
        m_movablelimits = true;
        m_stretchy = true;
        m_lspace = "0em";
        m_rspace = "thinmathspace";
        return true;
    }
    if( queriedOperator == "&Intersection;" && form == Prefix ) {
        m_largeop = true;
        m_movablelimits = true;
        m_stretchy = true;
        m_lspace = "0em";
        m_rspace = "thinmathspace";
        return true;
    }
    if( queriedOperator == "&Coproduct;" && form == Infix ) {
        m_lspace = "thinmathspace";
        m_rspace = "thinmathspace";
        return true;
    }
    if( queriedOperator == "&Star;" && form == Infix ) {
        m_lspace = "thinmathspace";
        m_rspace = "thinmathspace";
        return true;
    }
    if( queriedOperator == "&CircleDot;" && form == Prefix ) {
        m_largeop = true;
        m_movablelimits = true;
        m_lspace = "0em";
        m_rspace = "thinmathspace";
        return true;
    }
    if( queriedOperator == "*" && form == Infix ) {
        m_lspace = "thinmathspace";
        m_rspace = "thinmathspace";
        return true;
    }
    if( queriedOperator == "&InvisibleTimes;" && form == Infix ) {
        m_lspace = "0em";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == "&CenterDot;" && form == Infix ) {
        m_lspace = "thinmathspace";
        m_rspace = "thinmathspace";
        return true;
    }
    if( queriedOperator == "&CircleTimes;" && form == Infix ) {
        m_lspace = "thinmathspace";
        m_rspace = "thinmathspace";
        return true;
    }
    if( queriedOperator == "&Vee;" && form == Infix ) {
        m_lspace = "thinmathspace";
        m_rspace = "thinmathspace";
        return true;
    }
    if( queriedOperator == "&Wedge;" && form == Infix ) {
        m_lspace = "thinmathspace";
        m_rspace = "thinmathspace";
        return true;
    }
    if( queriedOperator == "&Diamond;" && form == Infix ) {
        m_lspace = "thinmathspace";
        m_rspace = "thinmathspace";
        return true;
    }
    if( queriedOperator == "&Backslash;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "thinmathspace";
        m_rspace = "thinmathspace";
        return true;
    }
    if( queriedOperator == "/" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "thinmathspace";
        m_rspace = "thinmathspace";
        return true;
    }
    if( queriedOperator == "-" && form == Prefix ) {
        m_lspace = "0em";
        m_rspace = "veryverythinmathspace";
        return true;
    }
    if( queriedOperator == "+" && form == Prefix ) {
        m_lspace = "0em";
        m_rspace = "veryverythinmathspace";
        return true;
    }
    if( queriedOperator == "&MinusPlus;" && form == Prefix ) {
        m_lspace = "0em";
        m_rspace = "veryverythinmathspace";
        return true;
    }
    if( queriedOperator == "&PlusMinus;" && form == Prefix ) {
        m_lspace = "0em";
        m_rspace = "veryverythinmathspace";
        return true;
    }
    if( queriedOperator == "." && form == Infix ) {
        m_lspace = "0em";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == "&Cross;" && form == Infix ) {
        m_lspace = "verythinmathspace";
        m_rspace = "verythinmathspace";
        return true;
    }
    if( queriedOperator == "**" && form == Infix ) {
        m_lspace = "verythinmathspace";
        m_rspace = "verythinmathspace";
        return true;
    }
    if( queriedOperator == "&CircleDot;" && form == Infix ) {
        m_lspace = "verythinmathspace";
        m_rspace = "verythinmathspace";
        return true;
    }
    if( queriedOperator == "&SmallCircle;" && form == Infix ) {
        m_lspace = "verythinmathspace";
        m_rspace = "verythinmathspace";
        return true;
    }
    if( queriedOperator == "&Square;" && form == Prefix ) {
        m_lspace = "0em";
        m_rspace = "verythinmathspace";
        return true;
    }
    if( queriedOperator == "&Del;" && form == Prefix ) {
        m_lspace = "0em";
        m_rspace = "verythinmathspace";
        return true;
    }
    if( queriedOperator == "&PartialD;" && form == Prefix ) {
        m_lspace = "0em";
        m_rspace = "verythinmathspace";
        return true;
    }
    if( queriedOperator == "&CapitalDifferentialD;" && form == Prefix ) {
        m_lspace = "0em";
        m_rspace = "verythinmathspace";
        return true;
    }
    if( queriedOperator == "&DifferentialD;" && form == Prefix ) {
        m_lspace = "0em";
        m_rspace = "verythinmathspace";
        return true;
    }
    if( queriedOperator == "&Sqrt;" && form == Prefix ) {
        m_stretchy = true;
        m_lspace = "0em";
        m_rspace = "verythinmathspace";
        return true;
    }
    if( queriedOperator == "&DoubleDownArrow;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "verythinmathspace";
        m_rspace = "verythinmathspace";
        return true;
    }
    if( queriedOperator == "&DoubleLongLeftArrow;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "verythinmathspace";
        m_rspace = "verythinmathspace";
        return true;
    }
    if( queriedOperator == "&DoubleLongLeftRightArrow;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "verythinmathspace";
        m_rspace = "verythinmathspace";
        return true;
    }
    if( queriedOperator == "&DoubleLongRightArrow;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "verythinmathspace";
        m_rspace = "verythinmathspace";
        return true;
    }
    if( queriedOperator == "&DoubleUpArrow;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "verythinmathspace";
        m_rspace = "verythinmathspace";
        return true;
    }
    if( queriedOperator == "&DoubleUpDownArrow;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "verythinmathspace";
        m_rspace = "verythinmathspace";
        return true;
    }
    if( queriedOperator == "&DownArrow;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "verythinmathspace";
        m_rspace = "verythinmathspace";
        return true;
    }
    if( queriedOperator == "&DownArrowBar;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "verythinmathspace";
        m_rspace = "verythinmathspace";
        return true;
    }
    if( queriedOperator == "&DownArrowUpArrow;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "verythinmathspace";
        m_rspace = "verythinmathspace";
        return true;
    }
    if( queriedOperator == "&DownTeeArrow;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "verythinmathspace";
        m_rspace = "verythinmathspace";
        return true;
    }
    if( queriedOperator == "&LeftDownTeeVector;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "verythinmathspace";
        m_rspace = "verythinmathspace";
        return true;
    }
    if( queriedOperator == "&LeftDownVector;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "verythinmathspace";
        m_rspace = "verythinmathspace";
        return true;
    }
    if( queriedOperator == "&LeftDownVectorBar;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "verythinmathspace";
        m_rspace = "verythinmathspace";
        return true;
    }
    if( queriedOperator == "&LeftUpDownVector;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "verythinmathspace";
        m_rspace = "verythinmathspace";
        return true;
    }
    if( queriedOperator == "&LeftUpTeeVector;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "verythinmathspace";
        m_rspace = "verythinmathspace";
        return true;
    }
    if( queriedOperator == "&LeftUpVector;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "verythinmathspace";
        m_rspace = "verythinmathspace";
        return true;
    }
    if( queriedOperator == "&LeftUpVectorBar;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "verythinmathspace";
        m_rspace = "verythinmathspace";
        return true;
    }
    if( queriedOperator == "&LongLeftArrow;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "verythinmathspace";
        m_rspace = "verythinmathspace";
        return true;
    }
    if( queriedOperator == "&LongLeftRightArrow;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "verythinmathspace";
        m_rspace = "verythinmathspace";
        return true;
    }
    if( queriedOperator == "&LongRightArrow;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "verythinmathspace";
        m_rspace = "verythinmathspace";
        return true;
    }
    if( queriedOperator == "&ReverseUpEquilibrium;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "verythinmathspace";
        m_rspace = "verythinmathspace";
        return true;
    }
    if( queriedOperator == "&RightDownTeeVector;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "verythinmathspace";
        m_rspace = "verythinmathspace";
        return true;
    }
    if( queriedOperator == "&RightDownVector;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "verythinmathspace";
        m_rspace = "verythinmathspace";
        return true;
    }
    if( queriedOperator == "&RightDownVectorBar;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "verythinmathspace";
        m_rspace = "verythinmathspace";
        return true;
    }
    if( queriedOperator == "&RightUpDownVector;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "verythinmathspace";
        m_rspace = "verythinmathspace";
        return true;
    }
    if( queriedOperator == "&RightUpTeeVector;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "verythinmathspace";
        m_rspace = "verythinmathspace";
        return true;
    }
    if( queriedOperator == "&RightUpVector;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "verythinmathspace";
        m_rspace = "verythinmathspace";
        return true;
    }
    if( queriedOperator == "&RightUpVectorBar;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "verythinmathspace";
        m_rspace = "verythinmathspace";
        return true;
    }
    if( queriedOperator == "&ShortDownArrow;" && form == Infix ) {
        m_lspace = "verythinmathspace";
        m_rspace = "verythinmathspace";
        return true;
    }
    if( queriedOperator == "&ShortUpArrow;" && form == Infix ) {
        m_lspace = "verythinmathspace";
        m_rspace = "verythinmathspace";
        return true;
    }
    if( queriedOperator == "&UpArrow;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "verythinmathspace";
        m_rspace = "verythinmathspace";
        return true;
    }
    if( queriedOperator == "&UpArrowBar;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "verythinmathspace";
        m_rspace = "verythinmathspace";
        return true;
    }
    if( queriedOperator == "&UpArrowDownArrow;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "verythinmathspace";
        m_rspace = "verythinmathspace";
        return true;
    }
    if( queriedOperator == "&UpDownArrow;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "verythinmathspace";
        m_rspace = "verythinmathspace";
        return true;
    }
    if( queriedOperator == "&UpEquilibrium;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "verythinmathspace";
        m_rspace = "verythinmathspace";
        return true;
    }
    if( queriedOperator == "&UpTeeArrow;" && form == Infix ) {
        m_stretchy = true;
        m_lspace = "verythinmathspace";
        m_rspace = "verythinmathspace";
        return true;
    }
    if( queriedOperator == "^" && form == Infix ) {
        m_lspace = "verythinmathspace";
        m_rspace = "verythinmathspace";
        return true;
    }
    if( queriedOperator == "&lt;>" && form == Infix ) {
        m_lspace = "verythinmathspace";
        m_rspace = "verythinmathspace";
        return true;
    }
    if( queriedOperator == "'" && form == Postfix ) {
        m_lspace = "verythinmathspace";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == "!" && form == Postfix ) {
        m_lspace = "verythinmathspace";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == "!!" && form == Postfix ) {
        m_lspace = "verythinmathspace";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == "~" && form == Infix ) {
        m_lspace = "verythinmathspace";
        m_rspace = "verythinmathspace";
        return true;
    }
    if( queriedOperator == "@" && form == Infix ) {
        m_lspace = "verythinmathspace";
        m_rspace = "verythinmathspace";
        return true;
    }
    if( queriedOperator == "--" && form == Postfix ) {
        m_lspace = "verythinmathspace";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == "--" && form == Prefix ) {
        m_lspace = "0em";
        m_rspace = "verythinmathspace";
        return true;
    }
    if( queriedOperator == "++" && form == Postfix ) {
        m_lspace = "verythinmathspace";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == "++" && form == Prefix ) {
        m_lspace = "0em";
        m_rspace = "verythinmathspace";
        return true;
    }
    if( queriedOperator == "&ApplyFunction;" && form == Infix ) {
        m_lspace = "0em";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == "?" && form == Infix ) {
        m_lspace = "verythinmathspace";
        m_rspace = "verythinmathspace";
        return true;
    }
    if( queriedOperator == "_" && form == Infix ) {
        m_lspace = "verythinmathspace";
        m_rspace = "verythinmathspace";
        return true;
    }
    if( queriedOperator == "&Breve;" && form == Postfix ) {
        m_accent = true;
        m_lspace = "0em";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == "&Cedilla;" && form == Postfix ) {
        m_accent = true;
        m_lspace = "0em";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == "&DiacriticalGrave;" && form == Postfix ) {
        m_accent = true;
        m_lspace = "0em";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == "&DiacriticalDot;" && form == Postfix ) {
        m_accent = true;
        m_lspace = "0em";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == "&DiacriticalDoubleAcute;" && form == Postfix ) {
        m_accent = true;
        m_lspace = "0em";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == "&LeftArrow;" && form == Postfix ) {
        m_accent = true;
        m_stretchy = true;
        m_lspace = "0em";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == "&LeftRightArrow;" && form == Postfix ) {
        m_accent = true;
        m_stretchy = true;
        m_lspace = "0em";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == "&LeftRightVector;" && form == Postfix ) {
        m_accent = true;
        m_stretchy = true;
        m_lspace = "0em";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == "&LeftVector;" && form == Postfix ) {
        m_accent = true;
        m_stretchy = true;
        m_lspace = "0em";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == "&DiacriticalAcute;" && form == Postfix ) {
        m_accent = true;
        m_lspace = "0em";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == "&RightArrow;" && form == Postfix ) {
        m_accent = true;
        m_stretchy = true;
        m_lspace = "0em";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == "&RightVector;" && form == Postfix ) {
        m_accent = true;
        m_stretchy = true;
        m_lspace = "0em";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == "&DiacriticalTilde;" && form == Postfix ) {
        m_accent = true;
        m_stretchy = true;
        m_lspace = "0em";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == "&DoubleDot;" && form == Postfix ) {
        m_accent = true;
        m_lspace = "0em";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == "&DownBreve;" && form == Postfix ) {
        m_accent = true;
        m_lspace = "0em";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == "&Hacek;" && form == Postfix ) {
        m_accent = true;
        m_stretchy = true;
        m_lspace = "0em";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == "&Hat;" && form == Postfix ) {
        m_accent = true;
        m_stretchy = true;
        m_lspace = "0em";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == "&OverBar;" && form == Postfix ) {
        m_accent = true;
        m_stretchy = true;
        m_lspace = "0em";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == "&OverBrace;" && form == Postfix ) {
        m_accent = true;
        m_stretchy = true;
        m_lspace = "0em";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == "&OverBracket;" && form == Postfix ) {
        m_accent = true;
        m_stretchy = true;
        m_lspace = "0em";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == "&OverParenthesis;" && form == Postfix ) {
        m_accent = true;
        m_stretchy = true;
        m_lspace = "0em";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == "&TripleDot;" && form == Postfix ) {
        m_accent = true;
        m_lspace = "0em";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == "&UnderBar;" && form == Postfix ) {
        m_accent = true;
        m_stretchy = true;
        m_lspace = "0em";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == "&UnderBrace;" && form == Postfix ) {
        m_accent = true;
        m_stretchy = true;
        m_lspace = "0em";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == "&UnderBracket;" && form == Postfix ) {
        m_accent = true;
        m_stretchy = true;
        m_lspace = "0em";
        m_rspace = "0em";
        return true;
    }
    if( queriedOperator == "&UnderParenthesis;" && form == Postfix ) {
        m_accent = true;
        m_stretchy = true;
        m_lspace = "0em";
        m_rspace = "0em";
        return true;
    }

    return false;
}
