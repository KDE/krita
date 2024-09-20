/*
 * SPDX-FileCopyrightText: 2009 Matthew Woehlke <mw_triad@users.sourceforge.net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_RANDOM_GENERATOR_2D_DEMO_H
#define KIS_RANDOM_GENERATOR_2D_DEMO_H

#include "ui_KisRandomGenerator2DDemo.h"

class Noise {
public:
    Noise(int wx, int wy);
    virtual ~Noise();

    void update(quint64 seed, int shift, int cutoff, bool chR, bool chG, bool chB);
    QImage image() const;
    QImage histogram() const;
    quint64 min() const;
    quint64 max() const;
    quint64 mean() const;

private:
    int _wx, _wy;
    QImage _image, _hist;
    quint64 _min, _max, _sum;
};

class KisRandomGenerator2DDemo: public QWidget, Ui::NoiseDemoUi
{
    Q_OBJECT
public:
    KisRandomGenerator2DDemo(QWidget* parent = 0);
    ~KisRandomGenerator2DDemo() override;

public Q_SLOTS:
    void updateNoise();

private:
    bool _noUpdate;
};

#endif
// kate: hl C++; indent-width 4; replace-tabs on;
