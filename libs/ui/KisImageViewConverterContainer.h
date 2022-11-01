/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISIMAGEVIEWCONVERTERCONTAINER_H
#define KISIMAGEVIEWCONVERTERCONTAINER_H

#include "kritaui_export.h"
#include "kis_types.h"

class KoViewConverter;

class KisImageViewConverterContainer : public QObject
{
    Q_OBJECT
public:
    KisImageViewConverterContainer(KisImageWSP image);
    KisImageViewConverterContainer(const KisImageViewConverterContainer &rhs);
    ~KisImageViewConverterContainer();

    void setImage(KisImageWSP image);
    void detach();

    KoViewConverter* viewConverter() const;

private Q_SLOTS:
    void slotImageResolutionChanged(qreal xRes, qreal yRes);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KISIMAGEVIEWCONVERTERCONTAINER_H
