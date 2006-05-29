/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
	              Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>

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

#ifndef KFORMULAWIDGET_H
#define KFORMULAWIDGET_H

#include <qdom.h>
#include <QPixmap>
#include <QPoint>
#include <QWidget>

#include "kformuladefs.h"
#include "kformulaview.h"

KFORMULA_NAMESPACE_BEGIN

class BasicElement;
class FormulaCursor;
class FormulaElement;
class Container;

KFORMULA_NAMESPACE_END

using namespace KFormula;


/**
 * The view. A widget that shows the formula. There are methods
 * to move the cursor around. To edit the formula use the document.
 */
class KFormulaWidget : public QWidget
{
    Q_OBJECT

public:
    KFormulaWidget(Container*, QWidget* parent=0);
    ~KFormulaWidget();


    /**
     * @returns the point inside the formula widget where the cursor is.
     */
    QPoint getCursorPoint() const;

    /**
     * Puts the widget in read only mode.
     */
    void setReadOnly(bool ro);

public slots:

    void slotSelectAll();

signals:

    /**
     * Is emitted every time the cursor might have changed.
     */
    void cursorChanged(bool visible, bool selecting);

protected slots:

    /**
     * The formula has changed and needs to be redrawn.
     */
    void slotFormulaChanged(int width, int height);

    void slotCursorChanged(bool visible, bool selecting);

protected:

    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);
    virtual void mouseDoubleClickEvent(QMouseEvent* event);
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void wheelEvent(QWheelEvent* event);

    virtual void paintEvent(QPaintEvent* event);
    virtual void keyPressEvent(QKeyEvent* event);
    virtual void focusInEvent(QFocusEvent* event);
    virtual void focusOutEvent(QFocusEvent* event);

    /**
     * The document we show.
     */
    Container* getDocument();

    /**
     * Our cursor.
     */
    FormulaCursor* getCursor();

private:

    /**
     * This widget is a wrapper around the actual view.
     */
    View formulaView;

    QPixmap buffer;
};

#endif // KFORMULAWIDGET_H
