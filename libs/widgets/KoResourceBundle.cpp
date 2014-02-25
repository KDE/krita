/*
 *  Copyright (c) 2014 Victor Lafon metabolic.ewilan@hotmail.fr
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

#include "KoResourceBundle.h"

KoResourceBundle::KoResourceBundle(QString const& file):KoResource(file)
{

}

bool KoResourceBundle::load()
{
    setValid(true);
    return true;
}

bool KoResourceBundle::save()
{
    return true;
}


QString KoResourceBundle::defaultFileExtension() const{
    return QString(".zip");
}

QImage KoResourceBundle::image() const{
	return thumbnail;
}
