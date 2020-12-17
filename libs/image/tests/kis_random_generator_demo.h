/*
 * SPDX-FileCopyrightText: 2009 Matthew Woehlke <mw_triad@users.sourceforge.net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_RANDOM_GENERATOR_DEMO_H
#define KIS_RANDOM_GENERATOR_DEMO_H

#include "ui_kis_random_generator_demo.h"

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

class KisRandomGeneratorDemo: public QWidget, Ui::NoiseDemoUi
{
    Q_OBJECT
public:
    KisRandomGeneratorDemo(QWidget* parent = 0);
    ~KisRandomGeneratorDemo() override;

public Q_SLOTS:
    void updateNoise();

private:
    bool _noUpdate;
};

#endif
// kate: hl C++; indent-width 4; replace-tabs on;
