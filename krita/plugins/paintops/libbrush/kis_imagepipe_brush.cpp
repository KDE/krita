/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2005 Bart Coppens <kde@bartcoppens.be>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "kis_imagepipe_brush.h"
#include "kis_imagepipe_brush_p.h"

class KisImagePipeBrush::Private
{
public:

    QString name;
    QString parasiteString; // Contains instructions on how to use the brush
    mutable KisPipeBrushParasite parasite;
    qint32 numOfBrushes;
    mutable quint32 currentBrush;

    QByteArray data;
    mutable QList<KisGbrBrush*> brushes;

    enumBrushType brushType;
};

KisImagePipeBrush::KisImagePipeBrush(const QString& filename)
        : KisGbrBrush(filename)
        , m_d(new Private())
{
    m_d->brushType = INVALID;
    m_d->numOfBrushes = 0;
    m_d->currentBrush = 0;
}

KisImagePipeBrush::KisImagePipeBrush(const QString& name, int w, int h,
                                     QVector< QVector<KisPaintDevice*> > devices,
                                     QVector<KisParasite::SelectionMode > modes)
        : KisGbrBrush("")
        , m_d(new Private())
{
    Q_ASSERT(devices.count() == modes.count());
    Q_ASSERT(devices.count() > 0);
    Q_ASSERT(devices.count() < 2); // XXX Multidimensionals not supported yet, change to MaxDim!

    setName(name);
    m_d->parasite.dim = devices.count();
    // XXX Change for multidim! :
    m_d->parasite.ncells = devices.at(0).count();
    m_d->parasite.rank[0] = m_d->parasite.ncells; // ### This can masquerade some bugs, be careful here in the future
    m_d->parasite.selection[0] = modes.at(0);
    // XXX needsmovement!

    m_d->parasite.setBrushesCount();

    for (int i = 0; i < devices.at(0).count(); i++) {
        m_d->brushes.append(new KisGbrBrush(devices.at(0).at(i), 0, 0, w, h));
    }

    setImage(m_d->brushes.at(0)->image());

    m_d->brushType = PIPE_IMAGE;
}

KisImagePipeBrush::KisImagePipeBrush(const KisImagePipeBrush& rhs)
        : KisGbrBrush(rhs),
        m_d(new Private)
{
    *m_d = *(rhs.m_d);
    m_d->brushes.clear();
    m_d->parasite = rhs.m_d->parasite;
    for (int i = 0; i < rhs.m_d->brushes.count(); i++) {
        m_d->brushes.append(rhs.m_d->brushes.at(i)->clone());
    }
}


KisImagePipeBrush::~KisImagePipeBrush()
{
    qDeleteAll(m_d->brushes);
    delete m_d;
}

bool KisImagePipeBrush::load()
{
    QFile file(filename());
    file.open(QIODevice::ReadOnly);
    m_d->data = file.readAll();
    file.close();
    return init();
}

bool KisImagePipeBrush::init()
{
    // XXX: this doesn't correctly load the image pipe brushes yet.

    // XXX: This stuff is in utf-8, too.
    // The first line contains the name -- this means we look until we arrive at the first newline
    QByteArray line1;

    qint32 i = 0;

    while (m_d->data[i] != '\n' && i < m_d->data.size()) {
        line1.append(m_d->data[i]);
        i++;
    }
    setName(QString::fromUtf8(line1, line1.size()));

    i++; // Skip past the first newline

    // The second line contains the number of brushes, separated by a space from the parasite

    // XXX: This stuff is in utf-8, too.
    QByteArray line2;
    while (m_d->data[i] != '\n' && i < m_d->data.size()) {
        line2.append(m_d->data[i]);
        i++;
    }

    QString paramline = QString::fromUtf8(line2, line2.size());
    m_d->numOfBrushes = paramline.left(paramline.indexOf(' ')).toUInt();
    m_d->parasiteString = paramline.mid(paramline.indexOf(' ') + 1);
    m_d->parasite = KisPipeBrushParasite(m_d->parasiteString);
    i++; // Skip past the second newline

    qint32 numOfBrushes = 0;
    while (numOfBrushes < m_d->numOfBrushes && i < m_d->data.size()) {
        KisGbrBrush* brush = new KisGbrBrush(name() + '_' + QString().setNum(numOfBrushes),
                                             m_d->data,
                                             i);
        Q_CHECK_PTR(brush);

        m_d->brushes.append(brush);

        numOfBrushes++;
    }

    if (!m_d->brushes.isEmpty()) {
        setValid(true);
        if (m_d->brushes.at(0)->brushType() == MASK) {
            m_d->brushType = PIPE_MASK;
        } else {
            m_d->brushType = PIPE_IMAGE;
        }
        setSpacing(m_d->brushes.at(m_d->brushes.count() - 1)->spacing());
        setWidth(m_d->brushes.at(0)->width());
        setHeight(m_d->brushes.at(0)->height());
    }

    m_d->data.resize(0);

    sanitize();

    return true;
}

bool KisImagePipeBrush::save()
{
    QFile file(filename());
    file.open(QIODevice::WriteOnly | QIODevice::Truncate);
    bool ok = saveToDevice(&file);
    file.close();
    return ok;
}

bool KisImagePipeBrush::saveToDevice(QIODevice* dev) const
{
    QByteArray utf8Name = name().toUtf8(); // Names in v2 brushes are in UTF-8
    char const* name = utf8Name.data();
    int len = qstrlen(name);

    if (m_d->parasite.dim != 1) {
        warnImage << "Save to file for pipe brushes with dim != not yet supported!";
        return false;
    }

    // Save this pipe brush: first the header, and then all individual brushes consecutively
    // XXX: this needs some care for when we have > 1 dimension)

    // Gimp Pipe Brush header format: Name\n<number of brushes> <parasite>\n

    // The name\n
    if (dev->write(name, len) == -1)
        return false;

    if (!dev->putChar('\n'))
        return false;

    // Write the parasite (also writes number of brushes)
    if (!m_d->parasite.saveToDevice(dev))
        return false;

    if (!dev->putChar('\n'))
        return false;

    // <gbr brushes>
    for (int i = 0; i < m_d->brushes.count(); i++)
        if (!m_d->brushes.at(i)->saveToDevice(dev))
            return false;

    return true;
}

QImage KisImagePipeBrush::image() const
{
    if (m_d->brushes.isEmpty()) {
        return QImage();
    } else {
        return m_d->brushes.at(0)->image();
    }
}


void KisImagePipeBrush::generateMaskAndApplyMaskOrCreateDab(KisFixedPaintDeviceSP dst, KisBrush::ColoringInformation* coloringInformation, double scaleX, double scaleY, double angle, const KisPaintInformation& info, double subPixelX , double subPixelY) const
{
    Q_UNUSED(scaleX);
    Q_UNUSED(scaleY);
    Q_UNUSED(angle);
    if (m_d->brushes.isEmpty()) return;
    selectNextBrush(info);
    m_d->brushes.at(m_d->currentBrush)->generateMaskAndApplyMaskOrCreateDab(dst, coloringInformation, 1.0, 1.0, 0.0, info, subPixelX, subPixelY);
}

KisFixedPaintDeviceSP KisImagePipeBrush::image(const KoColorSpace * colorSpace, double scale, double angle, const KisPaintInformation& info, double subPixelX, double subPixelY) const
{
    Q_UNUSED(scale);
    Q_UNUSED(angle);
    if (m_d->brushes.isEmpty()) return KisFixedPaintDeviceSP(0);
    selectNextBrush(info);
    return m_d->brushes.at(m_d->currentBrush)->paintDevice(colorSpace, 1.0, 0.0, info, subPixelX, subPixelY);
}

void KisImagePipeBrush::setParasiteString(const QString& parasite)
{
    m_d->parasiteString = parasite;
    m_d->parasite = KisPipeBrushParasite(parasite);
}


enumBrushType KisImagePipeBrush::brushType() const
{
    if (m_d->brushType == PIPE_IMAGE && useColorAsMask()) {
        return PIPE_MASK;
    } else {
        return m_d->brushType;
    }
}

bool KisImagePipeBrush::useColorAsMask() const
{
    if (m_d->brushes.count() > 0) {
        return m_d->brushes.at(0)->useColorAsMask();
    } else {
        return false;
    }
}

void KisImagePipeBrush::setUseColorAsMask(bool useColorAsMask)
{
    for (int i = 0; i < m_d->brushes.count(); i++) {
        m_d->brushes.at(i)->setUseColorAsMask(useColorAsMask);
    }
}

bool KisImagePipeBrush::hasColor() const
{
    if (m_d->brushes.count() > 0) {
        return m_d->brushes.at(0)->hasColor();
    } else {
        return false;
    }
}

const KisBoundary* KisImagePipeBrush::boundary() const
{
    Q_ASSERT(!m_d->brushes.isEmpty());
    return m_d->brushes.at(0)->boundary();
}

void KisImagePipeBrush::selectNextBrush(const KisPaintInformation& info) const
{
    m_d->currentBrush = 0;
    double angle;
    for (int i = 0; i < m_d->parasite.dim; i++) {
        int index = m_d->parasite.index[i];
        switch (m_d->parasite.selection[i]) {
        case KisParasite::Constant: break;
        case KisParasite::Incremental:
            index = (index + 1) % m_d->parasite.rank[i]; break;
        case KisParasite::Random:
            index = int(float(m_d->parasite.rank[i]) * KRandom::random() / RAND_MAX); break;
        case KisParasite::Pressure:
            index = static_cast<int>(info.pressure() * (m_d->parasite.rank[i] - 1) + 0.5); break;
        case KisParasite::Angular:
            // + m_d->PI_2 to be compatible with the gimp
            angle = info.angle() + M_PI_2;
            // We need to be in the [0..2*Pi[ interval so that we can more nicely select it
            if (angle < 0)
                angle += 2.0 * M_PI;
            else if (angle > 2.0 * M_PI)
                angle -= 2.0 * M_PI;
            index = static_cast<int>(angle / (2.0 * M_PI) * m_d->parasite.rank[i]);
            break;
        default:
            warnImage << "This parasite KisParasite::SelectionMode has not been implemented. Reselecting"
            << " to Constant";
            m_d->parasite.selection[i] = KisParasite::Constant; // Not incremental, since that assumes rank > 0
            index = 0;
        }
        m_d->parasite.index[i] = index;
        m_d->currentBrush += m_d->parasite.brushesCount[i] * index;
    }
}

bool KisImagePipeBrush::canPaintFor(const KisPaintInformation& info)
{
    if (info.movement().isMuchSmallerThan(1) // FIXME the 1 here is completely arbitrary.
            // What is the correct order of magnitude?
            && m_d->parasite.needsMovement)
        return false;
    return true;
}

void KisImagePipeBrush::makeMaskImage()
{
    for (int i = 0; i < m_d->brushes.count(); i++)
        m_d->brushes.at(i)->makeMaskImage();

    setBrushType(PIPE_MASK);
    setUseColorAsMask(false);
}

KisImagePipeBrush* KisImagePipeBrush::clone() const
{
    return new KisImagePipeBrush(*this);
}

QString KisImagePipeBrush::defaultFileExtension() const
{
    return QString(".gih");
}

void KisImagePipeBrush::sanitize()
{
    for (int i = 0; i < m_d->parasite.dim; i++) {
        // In the 2 listed cases, we'd divide by 0!
        if (m_d->parasite.selection[i] == KisParasite::Incremental
                || m_d->parasite.selection[i] == KisParasite::Angular) {
            if (m_d->parasite.rank[i] == 0) {
                warnImage << "Brush" << name() << " has a wrong rank for its selection mode!";
                m_d->parasite.selection[i] = KisParasite::Constant;
            }
        }
    }
}

