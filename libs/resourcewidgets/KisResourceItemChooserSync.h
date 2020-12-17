/* This file is part of the KDE project

   SPDX-FileCopyrightText: 2014 Sven Langkamp <sven.langkamp@gmail.com>
   SPDX-FileCopyrightText: 2019 Boudewijn Rempt <boud@valdyas.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
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
    /// @param length base length for the items, will be clamped if outside range
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
