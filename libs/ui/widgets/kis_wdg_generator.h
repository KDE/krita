/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2008
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
#ifndef KIS_WDG_GENERATOR_H
#define KIS_WDG_GENERATOR_H

#include <QWidget>
#include <kis_types.h>

class KisFilterConfiguration;
class KisViewManager;
class KoColor;

/**
 * A widget that allows users to select a generator and
 * create a config object for it.
 *
 * XXX: make use of bookmarked configuration things, like
 *      in the filter widget.
 */
class KisWdgGenerator : public QWidget
{

    Q_OBJECT

public:

    KisWdgGenerator(QWidget * parent);

    KisWdgGenerator(QWidget * parent, KisPaintDeviceSP dev);

    ~KisWdgGenerator() override;

    void initialize(KisViewManager *view);

    void setConfiguration(const KisFilterConfigurationSP  config);

    KisFilterConfigurationSP  configuration();

Q_SIGNALS:
    void previewConfiguration();

private Q_SLOTS:
    void slotGeneratorActivated(int);

public Q_SLOTS:
    void showFilterGallery(bool);

private:

    struct Private;
    Private * const d;
};

#endif
