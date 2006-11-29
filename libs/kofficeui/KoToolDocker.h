/* This file is part of the KDE project
 *
 * Copyright (c) 2005-2006 Boudewijn Rempt <boud@valdyas.org>
 * Copyright (c) 2006 Thomas Zander <zander@kde.org>
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
#ifndef KO_TOOL_DOCKER_H
#define KO_TOOL_DOCKER_H

#include <QDockWidget>

#include <koffice_export.h>

class QStackedWidget;
class QWidget;

/**
   The tool docker shows the tool option widget associtated with the
   current tool and the current canvas.
 */
class KOFFICEUI_EXPORT KoToolDocker : public QDockWidget
{
public:

    KoToolDocker();

    virtual ~KoToolDocker();

    /**
     * Update the option widget to the argument one, removing the currently set widget.
     */
    void setOptionWidget(QWidget * widget);

private:

    QStackedWidget * m_stack;
    QWidget * m_label;
};

#endif
