/* This file is part of the KDE project
 * Copyright (C) 2011 Thorsten Zachmann <zachmann@kde.org>
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

#ifndef KOMARKERSELECTOR_H
#define KOMARKERSELECTOR_H

#include "kritawidgets_export.h"

#include <KoFlake.h>
#include <QComboBox>

class KoMarker;


class KRITAWIDGETS_EXPORT KoMarkerSelector : public QComboBox
{
    Q_OBJECT
public:
    explicit KoMarkerSelector(KoFlake::MarkerPosition position, QWidget *parent = 0);
    ~KoMarkerSelector() override;

    // set the current marker style
    void setMarker(KoMarker *marker);
    // return the current marker style
    KoMarker *marker() const;

    /// reimplement
    QVariant itemData(int index, int role = Qt::UserRole) const;

    /**
     * Set the available markers in the document.
     */
    void updateMarkers(const QList<KoMarker*> markers);

protected:
    void paintEvent(QPaintEvent *pe) override;

private:
    class Private;
    Private * const d;
};

#endif /* KOMARKERSELECTOR_H */
