/* This file is part of the KDE project

   Copyright 2007 Johannes Simon <johannes.simon@gmail.com>
   Copyright 2009 Inge Wallin    <inge@lysator.liu.se>

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


#ifndef KCHART_THREEDSCENE_H
#define KCHART_THREEDSCENE_H


namespace KChart {

/**
 * @brief The ThreeDScene class is used to store properties of a 3D scene.
 * 
 * Currently this is only used for storage so that if we load a file
 * with a real ODF 3D chart, we can save it back without losing
 * data. More will be implemented once we have a real 3D charting
 * backend.
 */

class ThreeDScene
{
public:
    ThreeDScene();
    ~ThreeDScene();

private:
    class Private;
    Private *const d;
};

} // Namespace KChart

#endif // KCHART_THREEDSCENE_H

