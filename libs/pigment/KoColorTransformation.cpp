/*
 *  SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "KoColorTransformation.h"
#include <QDebug>

KoColorTransformation::~KoColorTransformation()
{
}

QList<QString> KoColorTransformation::parameters() const
{
    return QList<QString>();
}

int KoColorTransformation::parameterId(const QString& name) const
{
    Q_UNUSED(name);
    qFatal("No parameter for this transformation");
    return -1;
}

void KoColorTransformation::setParameter(int id, const QVariant& parameter)
{
    Q_UNUSED(id);
    Q_UNUSED(parameter);
    qFatal("No parameter for this transformation");
}

void KoColorTransformation::setParameters(const QHash<QString, QVariant> & parameters)
{
    for (QHash<QString, QVariant>::const_iterator it = parameters.begin(); it != parameters.end(); ++it) {
        setParameter( parameterId(it.key()), it.value());
    }

}
