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

#include <config.h>

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#include <math.h>

#include <netinet/in.h>
#include <limits.h>
#include <stdlib.h>

#include <qimage.h>
#include <qpoint.h>
#include <q3valuevector.h>
#include <qfile.h>
#include <qregexp.h>
#include <qstringlist.h>
#include <qtextstream.h>
//Added by qt3to4:
#include <Q3CString>

#include <kdebug.h>
#include <klocale.h>
#include <kapplication.h>
#include <krandom.h>

#include "kis_global.h"
#include "kis_paint_device.h"
#include "kis_imagepipe_brush.h"
#include "kis_brush.h"
#include "kis_alpha_mask.h"
#include "kis_layer.h"
#include "kis_meta_registry.h"
#include "kis_colorspace_factory_registry.h"


KisPipeBrushParasite::KisPipeBrushParasite(const QString& source)
{
    needsMovement = false;
    QRegExp basicSplitter(" ", true);
    QRegExp parasiteSplitter(":", true);
    QStringList parasites = QStringList::split(basicSplitter, source);
    for (uint i = 0; i < parasites.count(); i++) {
        QStringList splitted = QStringList::split(parasiteSplitter, parasites.at(i));
        if (splitted.count() != 2) {
            kWarning(41001) << "Wrong count for this parasite key/value:" << parasites.at(i) << endl;
            continue;
        }
        QString index = splitted.at(0);
        if (index == "dim") {
            dim = (splitted.at(1)).toInt();
            if (dim < 1 || dim > MaxDim) {
                dim = 1;
            }
        } else if (index.startsWith("sel")) {
            int selIndex = index.mid(strlen("sel")).toInt();
            if (selIndex >= 0 && selIndex < dim) {
                QString selectionMode = splitted.at(1);
                if (selectionMode == "incremental")
                    selection[selIndex] = Incremental;
                else if (selectionMode == "angular") {
                    selection[selIndex] = Angular;
                    needsMovement = true;
                } else if (selectionMode == "random")
                    selection[selIndex] = Random;
                else if (selectionMode == "pressure")
                    selection[selIndex] = Pressure;
                else if (selectionMode == "xtilt")
                    selection[selIndex] = TiltX;
                else if (selectionMode == "ytilt")
                    selection[selIndex] = TiltY;
                else
                    selection[selIndex] = Constant;
            } else {
                kWarning(41001)<< "Sel: wrong index: " << selIndex << "(dim = " << dim << ")" << endl;
            }
        } else if (index.startsWith("rank")) {
            int rankIndex = index.mid(strlen("rank")).toInt();
            if (rankIndex < 0 || rankIndex > dim) {
                kWarning(41001) << "Rankindex out of range: " << rankIndex << endl;
                continue;
            }
            rank[rankIndex] = (splitted.at(1)).toInt();
        } else if (index == "ncells") {
            ncells = (splitted.at(1)).toInt();
            if (ncells < 1 ) {
                kWarning(41001) << "ncells out of range: " << ncells << endl;
                ncells = 1;
            }
        }
    }

    for (int i = 0; i < dim; i++) {
        index[i] = 0;
    }

    setBrushesCount();
}

void KisPipeBrushParasite::setBrushesCount() {
    // I assume ncells is correct. If it isn't, complain to the parasite header.
    brushesCount[0] = ncells / rank[0];
    for (int i = 1; i < dim; i++) {
        brushesCount[i] = brushesCount[i-1] / rank[i];
    }
}

bool KisPipeBrushParasite::saveToDevice(QIODevice* dev) const {
    // write out something like
    // <count> ncells:<count> dim:<dim> rank0:<rank0> sel0:<sel0> <...>

    QTextStream stream(dev);
    /// FIXME things like step, placement and so are not added (nor loaded, as a matter of fact)
    stream << ncells << " ncells:" << ncells << " dim:" << dim;

    for (int i = 0; i < dim; i++) {
        stream << " rank" << i << ":" << rank[i] << " sel" << i << ":";
        switch (selection[i]) {
            case Constant: stream << "constant"; break;
            case Incremental: stream << "incremental"; break;
            case Angular: stream << "angular"; break;
            case Velocity: stream << "velocity"; break;
            case Random: stream << "random"; break;
            case Pressure: stream << "pressure"; break;
            case TiltX: stream << "xtilt"; break;
            case TiltY: stream << "ytilt"; break;
        }
    }

    return true;
}

KisImagePipeBrush::KisImagePipeBrush(const QString& filename) : super(filename)
{
    m_brushType = INVALID;
    m_numOfBrushes = 0;
    m_currentBrush = 0;
}

KisImagePipeBrush::KisImagePipeBrush(const QString& name, int w, int h,
                                     Q3ValueVector< Q3ValueVector<KisPaintDevice*> > devices,
                                     Q3ValueVector<KisPipeBrushParasite::SelectionMode> modes)
    : super("")
{
    Q_ASSERT(devices.count() == modes.count());
    Q_ASSERT(devices.count() > 0);
    Q_ASSERT(devices.count() < 2); // XXX Multidimensionals not supported yet, change to MaxDim!

    setName(name);

    m_parasite.dim = devices.count();
    // XXX Change for multidim! :
    m_parasite.ncells = devices.at(0).count();
    m_parasite.rank[0] = m_parasite.ncells;
    m_parasite.selection[0] = modes.at(0);
    // XXX needsmovement!

    m_parasite.setBrushesCount();

    for (uint i = 0; i < devices.at(0).count(); i++) {
        m_brushes.append(new KisBrush(devices.at(0).at(i), 0, 0, w, h));
    }

    setImage(m_brushes.at(0)->img());

    m_brushType = PIPE_IMAGE;
}

KisImagePipeBrush::~KisImagePipeBrush()
{
    m_brushes.setAutoDelete(true);
    m_brushes.clear();
}

bool KisImagePipeBrush::load()
{
    QFile file(filename());
    file.open(QIODevice::ReadOnly);
    m_data = file.readAll();
    file.close();
    return init();
}

bool KisImagePipeBrush::init()
{
    // XXX: this doesn't correctly load the image pipe brushes yet.

    // XXX: This stuff is in utf-8, too.
    // The first line contains the name -- this means we look until we arrive at the first newline
    Q3ValueVector<char> line1;

    quint32 i = 0;

    while (m_data[i] != '\n' && i < m_data.size()) {
        line1.append(m_data[i]);
        i++;
    }
    setName(i18n(QString::fromUtf8(&line1[0], i).ascii()));

    i++; // Skip past the first newline

    // The second line contains the number of brushes, separated by a space from the parasite

    // XXX: This stuff is in utf-8, too.
     Q3ValueVector<char> line2;
     while (m_data[i] != '\n' && i < m_data.size()) {
        line2.append(m_data[i]);
         i++;
     }

    QString paramline = QString::fromUtf8((&line2[0]), line2.size());
    quint32 m_numOfBrushes = paramline.left(paramline.find(' ')).toUInt();
    m_parasite = paramline.mid(paramline.find(' ') + 1);
    i++; // Skip past the second newline

     quint32 numOfBrushes = 0;
      while (numOfBrushes < m_numOfBrushes && i < m_data.size()){
        KisBrush * brush = new KisBrush(name() + "_" + numOfBrushes,
                        m_data,
                        i);
        Q_CHECK_PTR(brush);

        m_brushes.append(brush);

         numOfBrushes++;
     }

    if (!m_brushes.isEmpty()) {
        setValid(true);
        if (m_brushes.at( 0 )->brushType() == MASK) {
            m_brushType = PIPE_MASK;
        }
        else {
            m_brushType = PIPE_IMAGE;
        }
        setSpacing(m_brushes.at(m_brushes.count() - 1)->spacing());
        setWidth(m_brushes.at(0)->width());
        setHeight(m_brushes.at(0)->height());
    }

    m_data.resize(0);
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
    Q3CString utf8Name = name().utf8(); // Names in v2 brushes are in UTF-8
    char const* name = utf8Name.data();
    int len = qstrlen(name);

    if (parasite().dim != 1) {
        kWarning(41001) << "Save to file for pipe brushes with dim != not yet supported!" << endl;
        return false;
    }

    // Save this pipe brush: first the header, and then all individual brushes consecutively
    // (this needs some care for when we have > 1 dimension), FIXME

    // Gimp Pipe Brush header format: Name\n<number of brushes> <parasite>\n

    // The name\n
    if (dev->write(name, len) == -1)
        return false;

    if (dev->putch('\n') == -1)
        return false;

    // Write the parasite (also writes number of brushes)
    if (!m_parasite.saveToDevice(dev))
        return false;

    if (dev->putch('\n') == -1)
        return false;

    // <gbr brushes>
    for (uint i = 0; i < m_brushes.count(); i++)
        if (!m_brushes.at(i)->saveToDevice(dev))
            return false;

    return true;
}

QImage KisImagePipeBrush::img()
{
    if (m_brushes.isEmpty()) {
        return QImage();
    }
    else {
        return m_brushes.at(0)->img();
    }
}

KisAlphaMaskSP KisImagePipeBrush::mask(const KisPaintInformation& info, double subPixelX, double subPixelY) const
{
    if (m_brushes.isEmpty()) return KisAlphaMaskSP(0);
    selectNextBrush(info);
    return m_brushes.at(m_currentBrush)->mask(info, subPixelX, subPixelY);
}

KisPaintDeviceSP KisImagePipeBrush::image(KisColorSpace * colorSpace, const KisPaintInformation& info, double subPixelX, double subPixelY) const
{
    if (m_brushes.isEmpty()) return KisPaintDeviceSP(0);
    selectNextBrush(info);
    return m_brushes.at(m_currentBrush)->image(colorSpace, info, subPixelX, subPixelY);
}

void KisImagePipeBrush::setParasiteString(const QString& parasite)
{
    m_parasiteString = parasite;
    m_parasite = KisPipeBrushParasite(parasite);
}


enumBrushType KisImagePipeBrush::brushType() const
{
    if (m_brushType == PIPE_IMAGE && useColorAsMask()) {
        return PIPE_MASK;
    }
    else {
        return m_brushType;
    }
}

bool KisImagePipeBrush::useColorAsMask() const
{
    if (m_brushes.count() > 0) {
        return m_brushes.at(0)->useColorAsMask();
    }
    else {
        return false;
    }
}

void KisImagePipeBrush::setUseColorAsMask(bool useColorAsMask)
{
    for (uint i = 0; i < m_brushes.count(); i++) {
        m_brushes.at(i)->setUseColorAsMask(useColorAsMask);
    }
}

bool KisImagePipeBrush::hasColor() const
{
    if (m_brushes.count() > 0) {
        return m_brushes.at(0)->hasColor();
    }
    else {
        return false;
    }
}

KisBoundary KisImagePipeBrush::boundary() {
    Q_ASSERT(!m_brushes.isEmpty());
    return m_brushes.at(0)->boundary();
}

void KisImagePipeBrush::selectNextBrush(const KisPaintInformation& info) const {
    m_currentBrush = 0;
    double angle;
    for (int i = 0; i < m_parasite.dim; i++) {
        int index = m_parasite.index[i];
        switch (m_parasite.selection[i]) {
            case KisPipeBrushParasite::Constant: break;
            case KisPipeBrushParasite::Incremental:
                index = (index + 1) % m_parasite.rank[i]; break;
            case KisPipeBrushParasite::Random:
                index = int(float(m_parasite.rank[i])*KRandom::random() / RAND_MAX); break;
            case KisPipeBrushParasite::Pressure:
                index = static_cast<int>(info.pressure * (m_parasite.rank[i] - 1) + 0.5); break;
            case KisPipeBrushParasite::Angular:
                // + M_PI_2 to be compatible with the gimp
                angle = atan2(info.movement.y(), info.movement.x()) + M_PI_2;
                // We need to be in the [0..2*Pi[ interval so that we can more nicely select it
                if (angle < 0)
                    angle += 2.0 * M_PI;
                else if (angle > 2.0 * M_PI)
                    angle -= 2.0 * M_PI;
                index = static_cast<int>(angle / (2.0 * M_PI) * m_parasite.rank[i]);
                break;
            default:
                kWarning(41001) << "This parasite selectionMode has not been implemented. Reselecting"
                        << " to Incremental" << endl;
                m_parasite.selection[i] = KisPipeBrushParasite::Incremental;
                index = 0;
        }
        m_parasite.index[i] = index;
        m_currentBrush += m_parasite.brushesCount[i] * index;
    }
}

bool KisImagePipeBrush::canPaintFor(const KisPaintInformation& info) {
    if (info.movement.isNull() && m_parasite.needsMovement)
        return false;
    return true;
}

void KisImagePipeBrush::makeMaskImage() {
    for (uint i = 0; i < m_brushes.count(); i++)
        m_brushes.at(i)->makeMaskImage();

    setBrushType(PIPE_MASK);
    setUseColorAsMask(false);
}

KisImagePipeBrush* KisImagePipeBrush::clone() const {
    // The obvious way of cloning each brush in this one doesn't work for some reason...

    // XXX Multidimensionals not supported yet, change together with the constructor...
    Q3ValueVector< Q3ValueVector<KisPaintDevice*> > devices;
    Q3ValueVector<KisPipeBrushParasite::SelectionMode> modes;

    devices.push_back(Q3ValueVector<KisPaintDevice*>());
    modes.push_back(m_parasite.selection[0]);

    for (uint i = 0; i < m_brushes.count(); i++) {
        KisPaintDevice* pd = new KisPaintDevice(
                KisMetaRegistry::instance()->csRegistry()->getColorSpace(KisID("RGBA",""),""), "clone pd" );
        pd->convertFromQImage(m_brushes.at(i)->img(), "");
        devices.at(0).append(pd);
    }

    KisImagePipeBrush* c = new KisImagePipeBrush(name(), width(), height(), devices, modes);
    // XXX clean up devices

    return c;
}

#include "kis_imagepipe_brush.moc"

