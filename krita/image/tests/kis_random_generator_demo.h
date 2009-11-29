/*
 * Copyright 2009 Matthew Woehlke <mw_triad@users.sourceforge.net>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef KIS_RANDOM_GENERATOR_DEMO_H
#define KIS_RANDOM_GENERATOR_DEMO_H

#include "ui_kis_random_generator_demo.h"

class Noise {
public:
    Noise(int wx, int wy);
    virtual ~Noise();

    void update(quint64 seed, int shift, int cutoff, bool chR, bool chG, bool chB);
    QImage image();
    QImage histogram();
    quint64 min();
    quint64 max();
    quint64 mean();

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
    virtual ~KisRandomGeneratorDemo();

public Q_SLOTS:
    void updateNoise();

private:
    bool _noUpdate;
};

#endif
// kate: hl C++; indent-width 4; replace-tabs on;
