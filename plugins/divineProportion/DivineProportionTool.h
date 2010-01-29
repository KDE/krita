/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

#ifndef TOOL_H
#define TOOL_H

#include "DivineProportionShape.h"

#include <KoToolBase.h>

class QAction;

/**
 * This is the tool for the text-shape (which is a flake-based plugin).
 */
class DivineProportionTool : public KoToolBase {
    Q_OBJECT
public:
    explicit DivineProportionTool(KoCanvasBase *canvas);
    ~DivineProportionTool();

    /// reimplemented from superclass
    virtual void paint( QPainter &painter, const KoViewConverter &converter );

    /// reimplemented from superclass
    virtual void mousePressEvent( KoPointerEvent *event ) ;
    /// reimplemented from superclass
    virtual void mouseMoveEvent( KoPointerEvent *event );
    /// reimplemented from superclass
    virtual void mouseReleaseEvent( KoPointerEvent *event );
    /// reimplemented from superclass
    virtual void activate (bool temporary=false);
    /// reimplemented from superclass
    virtual void deactivate();
    /// reimplemented from superclass
    virtual QWidget *createOptionWidget();

private slots:
    void topLeftOrientationToggled(bool on);
    void topRightOrientationToggled(bool on);
    void bottomLeftOrientationToggled(bool on);
    void bottomRightOrientationToggled(bool on);
    void setPrintable(bool on);

private:
    void updateActions();

    DivineProportionShape *m_currentShape;

    QAction *m_topLeftOrientation;
    QAction *m_topRightOrientation;
    QAction *m_bottomLeftOrientation;
    QAction *m_bottomRightOrientation;
};

#endif
