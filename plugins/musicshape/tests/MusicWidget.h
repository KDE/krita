/* This file is part of the KDE project
 * Copyright (C) 2009 Marijn Kruisselbrink <mkruisselbrink@kde.org>
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
#ifndef MUSICWIDGET_H
#define MUSICWIDGET_H

#include <QWidget>
#include "../Engraver.h"
#include "../Renderer.h"
#include "../MusicStyle.h"

namespace MusicCore {
    class Sheet;
}
class Engraver;

class MusicWidget : public QWidget {
    Q_OBJECT
public:
    explicit MusicWidget(QWidget* parent = 0);

    /**
     * Set the sheet that is displayed in this MusicWidget. The widget doesn't take ownership of the sheet,
     * (unless it is already the parent QObject of it), but it is still probably not a good idea to have the
     * same sheet in multiple widgets, as the layout of the sheet is stored in the sheet itself.
     */
    void setSheet(MusicCore::Sheet* sheet);
    
    /**
     * Get the sheet that is currently displayed in this widget.
     */
    MusicCore::Sheet* sheet() const;

    /**
     * Sets the scale at which the music should be painted in this widget.
     */
    void setScale(qreal scale);
    
    /**
     * Gets the scale at which the music is painted in this widget.
     */
    qreal scale() const;
public slots:
    /**
     * Call this method when the contents of the sheet have changed in such a way that a re-engraving
     * of the music is required. This is done automatically if the widget changes size or scale.
     */
    void engrave();
protected:
    void paintEvent(QPaintEvent* event);
    void resizeEvent(QResizeEvent* event);
private:
    Engraver m_engraver;
    MusicStyle m_style;
    MusicRenderer m_renderer;
    MusicCore::Sheet* m_sheet;
    qreal m_scale;
    int m_lastSystem;
};

#endif // MUSICWIDGET_H
