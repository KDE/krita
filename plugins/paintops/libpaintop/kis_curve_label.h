/*
 *  SPDX-FileCopyrightText: 2010 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _KIS_CURVE_LABEL_H_
#define _KIS_CURVE_LABEL_H_

#include <kritapaintop_export.h>

class QString;
class QImage;

struct PAINTOP_EXPORT KisCurveLabel
{
public:
    KisCurveLabel();
    KisCurveLabel(const QString&);
    KisCurveLabel(const QImage&);
    KisCurveLabel(const KisCurveLabel&);
    KisCurveLabel& operator=(const KisCurveLabel&);
    ~KisCurveLabel();

    QString name() const;
    QImage icon() const;
private:
    struct Private;
    Private* const d;
};


#endif
