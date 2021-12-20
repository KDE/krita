/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef TestColorConversionSystem_H
#define TestColorConversionSystem_H

#include <QObject>
#include <QList>
#include <QString>

struct ModelDepthProfile {
    ModelDepthProfile(const QString& _model, const QString& _depth, const QString& _profile)
            : model(_model), depth(_depth), profile(_profile) {
    }
    QString model;
    QString depth;
    QString profile;
};

class TestColorConversionSystem : public QObject
{
    Q_OBJECT
public:
    TestColorConversionSystem();
private Q_SLOTS:
    void testConnections();
    void testGoodConnections();
    void testAlphaConnectionPaths();
    void testAlphaConversions();
    void testAlphaU16Conversions();
    void benchmarkAlphaToRgbConversion();
    void benchmarkRgbToAlphaConversion();

    void testCmykBitnessConversion();
private:
    QList< ModelDepthProfile > listModels;
};

#endif
