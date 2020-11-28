/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_ASL_XML_WRITER_H
#define __KIS_ASL_XML_WRITER_H

#include <QScopedPointer>
#include <QVector>

#include <KoPattern.h>
#include <KoSegmentGradient.h>

#include "kritapsd_export.h"

class QString;
class QColor;
class QPointF;
class QDomDocument;

class KoStopGradient;
class KoSegmentGradient;


class KRITAPSD_EXPORT KisAslXmlWriter
{
public:
    KisAslXmlWriter();
    ~KisAslXmlWriter();

    QDomDocument document() const;

    void enterDescriptor(const QString &key, const QString &name, const QString &classId);
    void leaveDescriptor();

    void enterList(const QString &key);
    void leaveList();

    void writeDouble(const QString &key, double value);
    void writeInteger(const QString &key, int value);
    void writeEnum(const QString &key, const QString &typeId, const QString &value);
    void writeUnitFloat(const QString &key, const QString &unit, double value);
    void writeText(const QString &key, const QString &value);
    void writeBoolean(const QString &key, bool value);
    void writeColor(const QString &key, const QColor &value);
    void writePoint(const QString &key, const QPointF &value);
    void writePhasePoint(const QString &key, const QPointF &value);
    void writeOffsetPoint(const QString &key, const QPointF &value);
    void writeCurve(const QString &key, const QString &name, const QVector<QPointF> &points);
    QString writePattern(const QString &key, const KoPatternSP pattern);
    void writePatternRef(const QString &key, const KoPatternSP pattern, const QString &uuid);
    void writeSegmentGradient(const QString &key, const KoSegmentGradient *gradient);
    void writeStopGradient(const QString &key, const KoStopGradient *gradient);

private:
    QString getSegmentEndpointTypeString(KoGradientSegmentEndpointType segtype);
    void writeGradientImpl(const QString &key,
                           const QString &name,
                           QVector<QColor> colors,
                           QVector<qreal> transparencies,
                           QVector<qreal> positions,
                           QVector<QString> types,
                           QVector<qreal> middleOffsets);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_ASL_XML_WRITER_H */
