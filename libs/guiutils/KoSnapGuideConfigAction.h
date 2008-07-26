/* This file is part of the KDE project
 * Copyright (c) 2008 Jan Hambrecht <jaham@gmx.net>
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

#ifndef KOSNAPGUIDECONFIGACTION_H
#define KOSNAPGUIDECONFIGACTION_H

#include <QAction>

#include "koguiutils_export.h"

 /// KoSnapGuideConfigAction shows a dialog to configure snap guides.
class KOGUIUTILS_EXPORT KoSnapGuideConfigAction : public QAction
{
    Q_OBJECT

public:
    /**
     * Constructs a KoSnapGuideConfigAction with the specified parent.
     *
     * @param parent The parent for this action.
     */
    KoSnapGuideConfigAction(QObject *parent = 0);

    /// Destructor
    virtual ~KoSnapGuideConfigAction();

private slots:
    void showSnapGuideDialog();
};

#endif // KOSNAPGUIDECONFIGACTION_H
