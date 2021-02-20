/*
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "kis_random_generator_demo.h"
#include "../kis_random_generator.h"

#include <ctime>
#include <cstdlib>
#include <QApplication>
#include <KLocalizedString>
#include <QCommandLineParser>

//BEGIN Noise
Noise::Noise(int wx, int wy) : _wx(wx), _wy(wy),
                               _image(_wx, _wy, QImage::Format_RGB32),
                               _hist(256, 256, QImage::Format_RGB32)
{
}

Noise::~Noise()
{
}

void Noise::update(quint64 seed, int shift, int cutoff, bool chR, bool chG, bool chB)
{
    KisRandomGenerator rand(seed);
    int h[256][256] = { { 0 } };
    int m = 0;

    _min = ~quint64(0);
    _max = 0;
    _sum = 0;

    _image.fill(0);
    for (int x = 0; x < _wx; ++x) {
        for (int y = 0; y < _wy; ++y) {
            quint64 c = rand.randomAt(x, y);
            unsigned char r, g, b, k;
            r = (c >> shift) & 0xFF;
            g = (c >> (shift > 48 ? shift - 8 : shift + 8)) & 0xFF;
            b = (c >> (shift > 40 ? (shift > 48 ? shift - 16 : shift - 8) : shift + 16)) & 0xFF;
            k = (c >> (shift > 32 ? (shift > 40 ? shift - 20 : shift - 8) : shift + 24)) & 0xFF;
            h[r][g]++;
            m = qMax(m, h[r][g]);
            if (!chR) r = 0;
            if (!chG) g = 0;
            if (!chB) b = 0;
            _image.setPixel(x, y, k < cutoff ? qRgb(r, g, b) : qRgb(0, 0, 0));

            _min = qMin(_min, c);
            _max = qMax(_max, c);
            _sum += (c >> 24);
        }
    }

    _hist.fill(0);
    for (int i = 0; i < 256; ++i) {
        for (int j = 0; j < 256; ++j) {
            int v = (h[i][j] * 255) / m;
            _hist.setPixel(i, j, (v < 64 ? qRgb(0, v + 255 - (v<<2), v) : v > 253 ? qRgb(255, 0, 0) : qRgb(0, 0, v)));
        }
    }
}

QImage Noise::image() const
{
    return _image;
}

QImage Noise::histogram() const
{
    return _hist;
}

quint64 Noise::min() const
{
    return _min;
}

quint64 Noise::max() const
{
    return _max;
}

quint64 Noise::mean() const
{
    return qreal(1<<24) * qreal(_sum) / qreal(_wx * _wy);
}
//END Noise

const int WIDTH = 1024;
const int HEIGHT = 1024;
Noise noise(WIDTH, HEIGHT);

KisRandomGeneratorDemo::KisRandomGeneratorDemo(QWidget* parent) : QWidget(parent)
{
    _noUpdate = true;
    setupUi(this);

    srand(time(0));
    seed1->setValue(rand() & 0xFFFF);
    seed2->setValue(rand() & 0xFFFF);
    seed3->setValue(rand() & 0xFFFF);
    seed4->setValue(rand() & 0xFFFF);
    _noUpdate = false;

    updateNoise();
}

KisRandomGeneratorDemo::~KisRandomGeneratorDemo()
{
}

void KisRandomGeneratorDemo::updateNoise()
{
    if (_noUpdate)
        return;

    union {
        quint64 u64;
        quint16 part[4];
    } seed;

    seed.part[0] = seed1->value();
    seed.part[1] = seed2->value();
    seed.part[2] = seed3->value();
    seed.part[3] = seed4->value();

    noise.update(seed.u64, shift->value(), cutoff->value(),
                 chR->isChecked(), chG->isChecked(), chB->isChecked());

    hist->setImage(noise.histogram());
    view->setImage(noise.image());

    const QString f("%1");
    stMin->setText(f.arg(noise.min(),  16, 16, QChar('0')));
    stMax->setText(f.arg(noise.max(),  16, 16, QChar('0')));
    stMean->setText(f.arg(noise.mean(),  16, 16, QChar('0')));
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    KisRandomGeneratorDemo *demo = new KisRandomGeneratorDemo;
    demo->show();
    return app.exec();
}

// kate: hl C++; indent-width 4; replace-tabs on;
