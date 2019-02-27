/* This file is part of the KDE project

   Copyright (c) 2014 Sven Langkamp <sven.langkamp@gmail.com>
   Copyright (c) 2019 Boudewijn Rempt <boud@valdyas.org>   

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

#ifndef KISRESOURCEITEMCHOOSERSYNC_H
#define KISRESOURCEITEMCHOOSERSYNC_H

#include <QObject>
#include <QScopedPointer>

#include "kritaresourcewidgets_export.h"

/**
 * KisResourceItemChooserSync is a singleton that syncs the size of entries in the
 * resource item choosers between different choosers
 * To use the syncing it has to be turned on in the KisResourceItemChooser
 */
class KRITARESOURCEWIDGETS_EXPORT KisResourceItemChooserSync : public QObject
{
    Q_OBJECT
public:
    KisResourceItemChooserSync();
    ~KisResourceItemChooserSync() override;
    static KisResourceItemChooserSync* instance();

    /// Gets the base length
    /// @returns the base length of items
    int baseLength();

    /// Set the base length 
    /// @param length base length for the items, will be clamped if ouside range
    void setBaseLength(int length);
    
Q_SIGNALS:
    /// Signal is emitted when the base length is changed and will trigger and update in
    /// the resource item choosers
    void baseLengthChanged(int length);
    
private:

    KisResourceItemChooserSync(const KisResourceItemChooserSync&);
    KisResourceItemChooserSync operator=(const KisResourceItemChooserSync&);

private:
    struct Private;
    const QScopedPointer<Private> d;
};

#endif // KORESOURCEITEMCHOOSERSYNC_H
