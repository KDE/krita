/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __KIS_DERIVED_RESOURCES_H
#define __KIS_DERIVED_RESOURCES_H

#include "KoDerivedResourceConverter.h"
#include "KoResourceUpdateMediator.h"
#include <QScopedPointer>

class KisPresetUpdateMediator : public KoResourceUpdateMediator
{
    Q_OBJECT
public:
    KisPresetUpdateMediator();
    ~KisPresetUpdateMediator() override;
    void connectResource(QVariant sourceResource) override;

private Q_SLOTS:
    void slotSettingsChanged();

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

class KisCompositeOpResourceConverter : public KoDerivedResourceConverter
{
public:
    KisCompositeOpResourceConverter();

    QVariant fromSource(const QVariant &value) override;
    QVariant toSource(const QVariant &value, const QVariant &sourceValue) override;
};

class KisEffectiveCompositeOpResourceConverter : public KoDerivedResourceConverter
{
public:
    KisEffectiveCompositeOpResourceConverter();

    QVariant fromSource(const QVariant &value) override;
    QVariant toSource(const QVariant &value, const QVariant &sourceValue) override;
};

class KisOpacityResourceConverter : public KoDerivedResourceConverter, public QObject
{
public:
    KisOpacityResourceConverter();

    QVariant fromSource(const QVariant &value) override;
    QVariant toSource(const QVariant &value, const QVariant &sourceValue) override;
};

class KisFlowResourceConverter : public KoDerivedResourceConverter, public QObject
{
public:
    KisFlowResourceConverter();

    QVariant fromSource(const QVariant &value) override;
    QVariant toSource(const QVariant &value, const QVariant &sourceValue) override;
};

class KisSizeResourceConverter : public KoDerivedResourceConverter, public QObject
{
public:
    KisSizeResourceConverter();

    QVariant fromSource(const QVariant &value) override;
    QVariant toSource(const QVariant &value, const QVariant &sourceValue) override;
};

class KisLodAvailabilityResourceConverter : public KoDerivedResourceConverter
{
public:
    KisLodAvailabilityResourceConverter();

    QVariant fromSource(const QVariant &value) override;
    QVariant toSource(const QVariant &value, const QVariant &sourceValue) override;
};

class KisLodSizeThresholdResourceConverter : public KoDerivedResourceConverter
{
public:
    KisLodSizeThresholdResourceConverter();

    QVariant fromSource(const QVariant &value) override;
    QVariant toSource(const QVariant &value, const QVariant &sourceValue) override;
};

class KisLodSizeThresholdSupportedResourceConverter : public KoDerivedResourceConverter
{
public:
    KisLodSizeThresholdSupportedResourceConverter();

    QVariant fromSource(const QVariant &value) override;
    QVariant toSource(const QVariant &value, const QVariant &sourceValue) override;
};

class KisEraserModeResourceConverter : public KoDerivedResourceConverter
{
public:
    KisEraserModeResourceConverter();

    QVariant fromSource(const QVariant &value) override;
    QVariant toSource(const QVariant &value, const QVariant &sourceValue) override;
};



#endif /* __KIS_DERIVED_RESOURCES_H */
