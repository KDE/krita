/*
 *  Copyright (c) 2005 Adrian Page <adrian@pagenet.plus.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <klocale.h>

#include "kis_composite_op.h"

//KisIDCompositeOpMap
std::map<KisID, CompositeOp> KisCompositeOp::s_idOpMap;

KisCompositeOp::KisCompositeOp()
{
    m_valid = false;
}

KisCompositeOp::KisCompositeOp(const QString& id)
{
    if (s_idOpMap.empty()) {
        fillMap();
    }

    KisIDCompositeOpMap::const_iterator it;
    m_valid = false;

    for (it = s_idOpMap.begin(); it != s_idOpMap.end(); ++it) {

        const KisID& kisId = (*it).first;

        if (kisId.id() == id) {

            m_id = (*it).first;
            m_op = (*it).second;
            m_valid = true;
            break;
        }
    }
}

KisCompositeOp::KisCompositeOp(CompositeOp compositeOp)
{
    if (s_idOpMap.empty()) {
        fillMap();
    }

    KisIDCompositeOpMap::const_iterator it;
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

bool KisCompositeOp::operator==(const KisCompositeOp& other) const
{
    if (isValid() && other.isValid()) {
        return op() == other.op();
    }
    return false;
}

bool KisCompositeOp::operator!=(const KisCompositeOp& other) const
{
    return !(*this == other);
}

void KisCompositeOp::fillMap()
{
    s_idOpMap[KisID("normal",     i18n("Normal"))] =        COMPOSITE_OVER;
    s_idOpMap[KisID("in",         i18n("In"))] =                   COMPOSITE_IN;                 
    s_idOpMap[KisID("out",         i18n("Out"))] =            COMPOSITE_OUT;                
    s_idOpMap[KisID("atop",     i18n("Atop"))] =        COMPOSITE_ATOP;               
    s_idOpMap[KisID("xor",         i18n("Xor"))] =            COMPOSITE_XOR;                
    s_idOpMap[KisID("plus",        i18n("Plus"))] =        COMPOSITE_PLUS;               
    s_idOpMap[KisID("minus",    i18n("Minus"))] =        COMPOSITE_MINUS;              
    s_idOpMap[KisID("add",         i18n("Add"))] =            COMPOSITE_ADD;                
    s_idOpMap[KisID("subtract",    i18n("Subtract"))] =        COMPOSITE_SUBTRACT;           
    s_idOpMap[KisID("diff",     i18n("Diff"))] =        COMPOSITE_DIFF;               
    s_idOpMap[KisID("multiply",    i18n("Multiply"))] =        COMPOSITE_MULT;               
    s_idOpMap[KisID("divide",    i18n("Divide"))] =        COMPOSITE_DIVIDE;             
    s_idOpMap[KisID("dodge",     i18n("Dodge"))] =        COMPOSITE_DODGE;              
    s_idOpMap[KisID("burn",     i18n("Burn"))] =        COMPOSITE_BURN;               
    s_idOpMap[KisID("bumpmap",     i18n("Bumpmap"))] =        COMPOSITE_BUMPMAP;            
    s_idOpMap[KisID("copy",     i18n("Copy"))] =        COMPOSITE_COPY;               
    s_idOpMap[KisID("copyred",     i18n("Copy Red"))] =        COMPOSITE_COPY_RED;           
    s_idOpMap[KisID("copygreen",     i18n("Copy Green"))] =        COMPOSITE_COPY_GREEN;         
    s_idOpMap[KisID("copyblue",     i18n("Copy Blue"))] =        COMPOSITE_COPY_BLUE;          
    s_idOpMap[KisID("copyopacity",     i18n("Copy Opacity"))] =    COMPOSITE_COPY_OPACITY;       
    s_idOpMap[KisID("clear",     i18n("Clear"))] =        COMPOSITE_CLEAR;              
    s_idOpMap[KisID("dissolve",     i18n("Dissolve"))] =        COMPOSITE_DISSOLVE;           
    s_idOpMap[KisID("displace",     i18n("Displace"))] =        COMPOSITE_DISPLACE;           
#if 0                                                                                     
    s_idOpMap[KisID("modulate",     i18n("Modulate"))] =        COMPOSITE_MODULATE;           
    s_idOpMap[KisID("threshold",     i18n("Threshold"))] =        COMPOSITE_THRESHOLD;          
#endif                                                                                     
    s_idOpMap[KisID("nocomposition",i18n("No Composition"))] =    COMPOSITE_NO;                 
    s_idOpMap[KisID("darken",     i18n("Darken"))] =        COMPOSITE_DARKEN;             
    s_idOpMap[KisID("lighten",     i18n("Lighten"))] =        COMPOSITE_LIGHTEN;            
    s_idOpMap[KisID("hue",         i18n("Hue"))] =            COMPOSITE_HUE;                
    s_idOpMap[KisID("saturation",     i18n("Saturation"))] =        COMPOSITE_SATURATION;         
    s_idOpMap[KisID("value",     i18n("Value"))] =        COMPOSITE_VALUE;              
    s_idOpMap[KisID("color",     i18n("Color"))] =        COMPOSITE_COLOR;              
    s_idOpMap[KisID("colorize",     i18n("Colorize"))] =        COMPOSITE_COLORIZE;           
    s_idOpMap[KisID("luminize",     i18n("Luminize"))] =        COMPOSITE_LUMINIZE;           
    s_idOpMap[KisID("screen",     i18n("Screen"))] =        COMPOSITE_SCREEN;             
    s_idOpMap[KisID("overlay",     i18n("Overlay"))] =        COMPOSITE_OVERLAY;            
    s_idOpMap[KisID("copycyan",     i18n("Copy Cyan"))] =        COMPOSITE_COPY_CYAN;          
    s_idOpMap[KisID("copymagenta",     i18n("Copy Magenta"))] =    COMPOSITE_COPY_MAGENTA;       
    s_idOpMap[KisID("copyyellow",     i18n("Copy Yellow"))] =        COMPOSITE_COPY_YELLOW;        
    s_idOpMap[KisID("copyblack",     i18n("Copy Black"))] =        COMPOSITE_COPY_BLACK;         
    s_idOpMap[KisID("erase",     i18n("Erase"))] =        COMPOSITE_ERASE;              
    s_idOpMap[KisID("undefined",     i18n("Undefined"))] =            COMPOSITE_UNDEF;              
}
                 
