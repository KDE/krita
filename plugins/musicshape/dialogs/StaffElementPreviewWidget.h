/* This file is part of the KDE project
 * Copyright (C) 2007 Marijn Kruisselbrink <mkruisselbrink@kde.org>
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
#ifndef STAFFELEMENTPREVIEWWIDGET_H
#define STAFFELEMENTPREVIEWWIDGET_H

#include <QWidget>
class MusicStyle;
class MusicRenderer;
namespace MusicCore {
    class StaffElement;
    class Staff;
    class Clef;
}

class StaffElementPreviewWidget : public QWidget
{
    Q_OBJECT
public:
    explicit StaffElementPreviewWidget(QWidget* parent = 0);
    virtual ~StaffElementPreviewWidget();
    
    virtual QSize sizeHint() const;
    
    void setMusicStyle(MusicStyle* style);
    void setStaffElement(MusicCore::StaffElement* se);
    MusicCore::Staff* staff();
protected:
    virtual void paintEvent(QPaintEvent * event);
private:
    MusicStyle* m_style;
    MusicRenderer* m_renderer;
    MusicCore::StaffElement* m_element;
    MusicCore::Staff* m_staff;
    MusicCore::Clef* m_clef;
};

#endif // STAFFELEMENTPREVIEWWIDGET_H
