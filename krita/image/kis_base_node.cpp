/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_base_node.h"
#include <klocale.h>

#include <KoProperties.h>
#include <KoColorSpace.h>
#include <KoCompositeOp.h>
#include "kis_paint_device.h"

class KisBaseNode::Private
{
public:

    QString compositeOp;
    KoProperties properties;
    KisBaseNodeSP linkedTo;
    bool systemLocked;
};

KisBaseNode::KisBaseNode()
    : m_d(new Private())
{
    setVisible(true);
    setUserLocked(false);
    setSystemLocked(false);
    m_d->linkedTo = 0;
    m_d->compositeOp = COMPOSITE_OVER;
}


KisBaseNode::KisBaseNode(const KisBaseNode & rhs)
    : QObject()
    , KisShared(rhs)
    ,  m_d(new Private())
{
    QMapIterator<QString, QVariant> iter = rhs.m_d->properties.propertyIterator();
    while (iter.hasNext()) {
        iter.next();
        m_d->properties.setProperty(iter.key(), iter.value());
    }
    m_d->linkedTo = rhs.m_d->linkedTo;
    m_d->compositeOp = rhs.m_d->compositeOp;
}

KisBaseNode::~KisBaseNode()
{
    delete m_d;
}

KisPaintDeviceSP KisBaseNode::paintDevice() const
{
    return 0;
}

KisPaintDeviceSP KisBaseNode::original() const
{
    return 0;
}

KisPaintDeviceSP KisBaseNode::projection() const
{
    return 0;
}

quint8 KisBaseNode::opacity() const
{
    return nodeProperties().intProperty("opacity", OPACITY_OPAQUE);
}

void KisBaseNode::setOpacity(quint8 val)
{
    if (opacity() != val) {
        nodeProperties().setProperty("opacity", val);
    }
}

quint8 KisBaseNode::percentOpacity() const
{
    return int(float(opacity() * 100) / 255 + 0.5);
}

void KisBaseNode::setPercentOpacity(quint8 val)
{
    setOpacity(int(float(val * 255) / 100 + 0.5));
}

const QString& KisBaseNode::compositeOpId() const
{
    return m_d->compositeOp;
}

/**
 * FIXME: Rename this function to setCompositeOpId()
 */
void KisBaseNode::setCompositeOp(const QString& compositeOp)
{
    m_d->compositeOp = compositeOp;
}

KoDocumentSectionModel::PropertyList KisBaseNode::sectionModelProperties() const
{
    KoDocumentSectionModel::PropertyList l;
    l << KoDocumentSectionModel::Property(i18n("Visible"), KIcon("visible"), KIcon("novisible"), visible());
    l << KoDocumentSectionModel::Property(i18n("Locked"), KIcon("locked"), KIcon("unlocked"), userLocked());
    return l;
}

void KisBaseNode::setSectionModelProperties(const KoDocumentSectionModel::PropertyList &properties)
{
    setVisible(properties.at(0).state.toBool());
    setUserLocked(properties.at(1).state.toBool());
}

KoProperties & KisBaseNode::nodeProperties() const
{
    return m_d->properties;
}

void KisBaseNode::mergeNodeProperties(const KoProperties & properties)
{
    QMapIterator<QString, QVariant> iter = properties.propertyIterator();
    while (iter.hasNext()) {
        iter.next();
        m_d->properties.setProperty(iter.key(), iter.value());
    }
}

bool KisBaseNode::check(const KoProperties & properties) const
{
    QMapIterator<QString, QVariant> iter = properties.propertyIterator();
    while (iter.hasNext()) {
        iter.next();
        if (m_d->properties.contains(iter.key())) {
            if (m_d->properties.value(iter.key()) != iter.value())
                return false;
        }
    }
    return true;
}


QImage KisBaseNode::createThumbnail(qint32 w, qint32 h)
{
    try {
        QImage img(w, h, QImage::Format_ARGB32);
        img.fill(0);
        return img;
    } catch (std::bad_alloc) {
        return QImage();
    }

}

bool KisBaseNode::visible() const
{
    return m_d->properties.boolProperty("visible", true);
}

void KisBaseNode::setVisible(bool visible)
{
    m_d->properties.setProperty("visible", visible);
    emit(visibilityChanged(visible));
}

bool KisBaseNode::userLocked() const
{
    return m_d->properties.boolProperty("locked", false);
}

void KisBaseNode::setUserLocked(bool locked)
{
    m_d->properties.setProperty("locked", locked);
    emit(userLockingChanged(locked));
}

bool KisBaseNode::systemLocked() const
{
    return m_d->systemLocked;
}

void KisBaseNode::setSystemLocked(bool locked)
{
    m_d->systemLocked = locked;
    emit(systemLockingChanged(locked));
}

bool KisBaseNode::isEditable() const
{
    return (visible() && !userLocked() && !systemLocked());
}

#include "kis_base_node.moc"
