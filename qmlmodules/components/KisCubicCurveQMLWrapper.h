/*
 *  SPDX-FileCopyrightText: 2024 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISCUBICCURVEQML_H
#define KISCUBICCURVEQML_H

#include <QQmlEngine>
#include <QPointF>

#include <kis_cubic_curve.h>

class KisCubicCurveQml : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(KisCubicCurve)
    
    Q_PROPERTY(QList<QPointF> points READ points WRITE setPoints NOTIFY pointsChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)

public:
    KisCubicCurveQml(QObject *parent = nullptr);

    const QList<QPointF> points() const;
    const QString& name() const;

    Q_INVOKABLE qreal value(qreal x) const;
    Q_INVOKABLE bool isIdentity() const;
    Q_INVOKABLE bool isConstant(qreal c) const;
    Q_INVOKABLE qreal interpolateLinear(qreal normalizedValue, const QList<qreal> &transfer) const;
    Q_INVOKABLE QList<qreal> floatTransfer(int size) const;
    Q_INVOKABLE QString toString() const;
    Q_INVOKABLE void fromString(const QString&);

public Q_SLOTS:
    void setPoints(const QList<QPointF> points);
    void setPoint(int idx, const QPointF &point);
    int addPoint(const QPointF &point);
    void removePoint(int idx);
    void setName(const QString& name);

Q_SIGNALS:
    void pointsChanged(const QList<QPointF>&);
    void nameChanged(const QString&);

private:
    KisCubicCurve m_curve;
};

#endif
