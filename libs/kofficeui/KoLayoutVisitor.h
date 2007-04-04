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
#ifndef KOLAYOUTVISITOR_H
#define KOLAYOUTVISITOR_H

#include <QWidget>

#include <kofficeui_export.h>

/**
 * This class can be used to loop over a number of widgets and then do a relayout to align
 * columns of all the different visited widgets.
 */
class KOFFICEUI_EXPORT KoLayoutVisitor {
public:
    /// Constructor
    KoLayoutVisitor();
    ~KoLayoutVisitor();

    /**
     * Visit looks at the widget and all the widget placed on it (as children) and checks to see
     * if they should be used for the layout process.
     * @param widget the widget to visit.
     */
    void visit(QWidget *widget);

    /**
     * This will do the actual work, which is to align the labels properly based on all the widgets visited.
     * Call relayout only once after all the widgets are visited.
     */
    void relayout();

private:
    class Private;
    Private * const d;
};

#endif
