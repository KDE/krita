/*
 *  Copyright (c) 2005 Adrian Page <adrian@pagenet.plus.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include <klocale.h>

#include "KoCompositeOp.h"

//KoIDCompositeOpMap
std::map<KoID, CompositeOp> KoCompositeOp::s_idOpMap;

KoCompositeOp::KoCompositeOp()
{
    m_valid = false;
}

KoCompositeOp::KoCompositeOp(const QString& id)
{
    if (s_idOpMap.empty()) {
        fillMap();
    }

    KoIDCompositeOpMap::const_iterator it;
    m_valid = false;

    for (it = s_idOpMap.begin(); it != s_idOpMap.end(); ++it) {

        const KoID& kisId = (*it).first;

        if (kisId.id() == id) {

            m_id = (*it).first;
            m_op = (*it).second;
            m_valid = true;
            break;
        }
    }
}

KoCompositeOp::KoCompositeOp(CompositeOp compositeOp)
{
    if (s_idOpMap.empty()) {
        fillMap();
    }

    KoIDCompositeOpMap::const_iterator it;
    m_valid = false;

    for (it = s_idOpMap.begin(); it != s_idOpMap.end(); ++it) {

        CompositeOp compOp = (*it).second;

        if (compOp == compositeOp) {

            m_id = (*it).first;
            m_op = compositeOp;
            m_valid = true;
            break;
        }
    }
}

bool KoCompositeOp::operator==(const KoCompositeOp& other) const
{
    if (isValid() && other.isValid()) {
        return op() == other.op();
    }
    return false;
}

bool KoCompositeOp::operator!=(const KoCompositeOp& other) const
{
    return !(*this == other);
}

void KoCompositeOp::fillMap()
{
    s_idOpMap[KoID("normal",     i18n("Normal"))] =        COMPOSITE_OVER;
    s_idOpMap[KoID("in",         i18n("In"))] =                   COMPOSITE_IN;                 
    s_idOpMap[KoID("out",         i18n("Out"))] =            COMPOSITE_OUT;                
    s_idOpMap[KoID("atop",     i18n("Atop"))] =        COMPOSITE_ATOP;               
    s_idOpMap[KoID("xor",         i18n("Xor"))] =            COMPOSITE_XOR;                
    s_idOpMap[KoID("plus",        i18n("Plus"))] =        COMPOSITE_PLUS;               
    s_idOpMap[KoID("minus",    i18n("Minus"))] =        COMPOSITE_MINUS;              
    s_idOpMap[KoID("add",         i18n("Add"))] =            COMPOSITE_ADD;                
    s_idOpMap[KoID("subtract",    i18n("Subtract"))] =        COMPOSITE_SUBTRACT;           
    s_idOpMap[KoID("diff",     i18n("Diff"))] =        COMPOSITE_DIFF;               
    s_idOpMap[KoID("multiply",    i18n("Multiply"))] =        COMPOSITE_MULT;               
    s_idOpMap[KoID("divide",    i18n("Divide"))] =        COMPOSITE_DIVIDE;             
    s_idOpMap[KoID("dodge",     i18n("Dodge"))] =        COMPOSITE_DODGE;              
    s_idOpMap[KoID("burn",     i18n("Burn"))] =        COMPOSITE_BURN;               
    s_idOpMap[KoID("bumpmap",     i18n("Bumpmap"))] =        COMPOSITE_BUMPMAP;            
    s_idOpMap[KoID("copy",     i18n("Copy"))] =        COMPOSITE_COPY;               
    s_idOpMap[KoID("copyred",     i18n("Copy Red"))] =        COMPOSITE_COPY_RED;           
    s_idOpMap[KoID("copygreen",     i18n("Copy Green"))] =        COMPOSITE_COPY_GREEN;         
    s_idOpMap[KoID("copyblue",     i18n("Copy Blue"))] =        COMPOSITE_COPY_BLUE;          
    s_idOpMap[KoID("copyopacity",     i18n("Copy Opacity"))] =    COMPOSITE_COPY_OPACITY;       
    s_idOpMap[KoID("clear",     i18n("Clear"))] =        COMPOSITE_CLEAR;              
    s_idOpMap[KoID("dissolve",     i18n("Dissolve"))] =        COMPOSITE_DISSOLVE;           
    s_idOpMap[KoID("displace",     i18n("Displace"))] =        COMPOSITE_DISPLACE;           
#if 0                                                                                     
    s_idOpMap[KoID("modulate",     i18n("Modulate"))] =        COMPOSITE_MODULATE;           
    s_idOpMap[KoID("threshold",     i18n("Threshold"))] =        COMPOSITE_THRESHOLD;          
#endif                                                                                     
    s_idOpMap[KoID("nocomposition",i18n("No Composition"))] =    COMPOSITE_NO;                 
    s_idOpMap[KoID("darken",     i18n("Darken"))] =        COMPOSITE_DARKEN;             
    s_idOpMap[KoID("lighten",     i18n("Lighten"))] =        COMPOSITE_LIGHTEN;            
    s_idOpMap[KoID("hue",         i18n("Hue"))] =            COMPOSITE_HUE;                
    s_idOpMap[KoID("saturation",     i18n("Saturation"))] =        COMPOSITE_SATURATION;         
    s_idOpMap[KoID("value",     i18n("Value"))] =        COMPOSITE_VALUE;              
    s_idOpMap[KoID("color",     i18n("Color"))] =        COMPOSITE_COLOR;              
    s_idOpMap[KoID("colorize",     i18n("Colorize"))] =        COMPOSITE_COLORIZE;           
    s_idOpMap[KoID("luminize",     i18n("Luminize"))] =        COMPOSITE_LUMINIZE;           
    s_idOpMap[KoID("screen",     i18n("Screen"))] =        COMPOSITE_SCREEN;             
    s_idOpMap[KoID("overlay",     i18n("Overlay"))] =        COMPOSITE_OVERLAY;            
    s_idOpMap[KoID("copycyan",     i18n("Copy Cyan"))] =        COMPOSITE_COPY_CYAN;          
    s_idOpMap[KoID("copymagenta",     i18n("Copy Magenta"))] =    COMPOSITE_COPY_MAGENTA;       
    s_idOpMap[KoID("copyyellow",     i18n("Copy Yellow"))] =        COMPOSITE_COPY_YELLOW;        
    s_idOpMap[KoID("copyblack",     i18n("Copy Black"))] =        COMPOSITE_COPY_BLACK;         
    s_idOpMap[KoID("erase",     i18n("Erase"))] =        COMPOSITE_ERASE;              
    s_idOpMap[KoID("undefined",     i18n("Undefined"))] =            COMPOSITE_UNDEF;              
}
                 
