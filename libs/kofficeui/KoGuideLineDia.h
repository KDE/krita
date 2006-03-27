// -*- Mode: c++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4; -*-
/* This file is part of the KDE project
   Copyright (C)  2002 Montel Laurent <lmontel@mandrakesoft.com>
   Copyright (C)  2005 Thorsten Zachmann <zachmann@kde.org>

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
 * Boston, MA 02110-1301, USA.
*/

#ifndef KOGUIDELINEDIA_H
#define KOGUIDELINEDIA_H

#include <kdialogbase.h>
#include <KoUnit.h>
#include "KoRect.h"
#include "KoPoint.h"


class KoUnitDoubleSpinBox;
class QRadioButton;

/**
 * @brief Class for setting a guide line position.
 */
class KoGuideLineDia : public KDialogBase
{
    Q_OBJECT
public:
    /**
     * @brief Constructor
     *
     * @param parent The parent widget
     * @param pos the actual position of the guide line
     * @param minPos the minimal position of the guide line
     * @param maxPos the maximal position of the guide line
     * @param unit The unit used in the document
     * @param name The name is send to the QObject constructor
     */
    KoGuideLineDia( QWidget *parent, double pos, double minPos, double maxPos,
                    KoUnit::Unit unit, const char *name = 0L );

    /**
     * @brief Constructor
     *
     * This shows a dialog to add a guide line. As long the position is not changed 
     * and the orientation of the guide line is changed the value will be set to pos.x()
     * or pos.y() according to the orientation.
     *
     * @param parent the parent widget
     * @param pos the actual position of cursor
     * @param rect the rect in where the guide can be placed
     * @param unit the unit used in the document
     * @param name the name is send to the QObject constructor
     */
    KoGuideLineDia( QWidget *parent, KoPoint &pos, KoRect &rect,
                    KoUnit::Unit unit, const char *name = 0L );
    /**
     * @brief the position
     *
     * @return the value of the position input
     */
    double pos() const;

    /**
     * @brief the orientation
     *
     * @return the orientation of the added guide line
     */
    Qt::Orientation orientation() const;

protected slots:
    void slotOrientationChanged();
    void slotPositionChanged();

protected:
    KoRect m_rect;
    KoPoint m_pos;
    bool m_positionChanged;
    QRadioButton * m_hButton;
    QRadioButton * m_vButton;
    KoUnitDoubleSpinBox* m_position;
};

#endif // KOGUIDELINEDIA_H
