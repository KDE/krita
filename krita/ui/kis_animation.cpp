/*
 *  Copyright (c) 2013 Somsubhra Bairi <somsubhra.bairi@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or(at you option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_animation.h"
#include "krita_export.h"
#include "kis_config.h"

#include <KisApplication.h>
#include <KisPart.h>

#include <QFile>
#include <QDomDocument>
#include <QByteArray>
#include <QString>

KisAnimation::KisAnimation(QObject *parent)
    : QObject(parent)
{
    KisConfig cfg;
    m_frameBreakingEnabled = cfg.defAutoFrameBreakEnabled();
    m_onionSkinningEnabled = cfg.defOnionSkinningEnabled();
    m_loopingEnabled = cfg.defLoopingEnabled();
    m_fps = cfg.defFps();
    m_localPlaybackRange = cfg.defLocalPlaybackRange();
}

void KisAnimation::setName(const QString &name)
{
    m_name = name;
}

QString KisAnimation::name() const
{
    return m_name;
}

void KisAnimation::setAuthor(const QString &author)
{
    m_author = author;
}

QString KisAnimation::author() const
{
    return m_author;
}

void KisAnimation::setDescription(const QString &description)
{
    m_description = description;
}

QString KisAnimation::description() const
{
    return m_description;
}

void KisAnimation::setFps(int fps)
{
    m_fps = fps;
    KisConfig cfg;
    cfg.defFps(fps);
}

int KisAnimation::fps() const
{
    return m_fps;
}

void KisAnimation::setLocalPlaybackRange(int range)
{
    m_localPlaybackRange = range;
    KisConfig cfg;
    cfg.defLocalPlaybackRange(range);
}

int KisAnimation::localPlaybackRange() const
{
    return m_localPlaybackRange;
}

void KisAnimation::setTime(int time)
{
    m_time = time;
}

int KisAnimation::time() const
{
    return m_time;
}

void KisAnimation::setColorSpace(const KoColorSpace *colorSpace)
{
    this->m_colorSpace = colorSpace;
}

const KoColorSpace *KisAnimation::colorSpace()
{
    return m_colorSpace;
}

void KisAnimation::setWidth(qint32 w)
{
    this->m_width = w;
}

qint32 KisAnimation::width() const
{
    return this->m_width;
}

void KisAnimation::setHeight(qint32 h)
{
    this->m_height = h;
}

qint32 KisAnimation::height() const
{
    return this->m_height;
}

void KisAnimation::setResolution(double res)
{
    this->m_resolution = res;
}

double KisAnimation::resolution() const
{
    return this->m_resolution;
}

void KisAnimation::setBgColor(KoColor bgColor)
{
    this->m_bgColor = bgColor;
}

KoColor KisAnimation::bgColor() const
{
    return this->m_bgColor;
}

void KisAnimation::setLocation(QString location)
{
    m_location = location;
}

QString KisAnimation::location()
{
    return m_location;
}

void KisAnimation::enableFrameBreaking(bool enable)
{
    m_frameBreakingEnabled = enable;
    KisConfig cfg;
    cfg.defAutoFrameBreakEnabled(enable);
}

bool KisAnimation::frameBreakingEnabled()
{
    return m_frameBreakingEnabled;
}

void KisAnimation::enableOnionSkinning(bool enable)
{
    m_onionSkinningEnabled = enable;
    KisConfig cfg;
    cfg.defOnionSkinningEnabled(enable);
}

bool KisAnimation::onionSkinningEnabled()
{
    return m_onionSkinningEnabled;
}

void KisAnimation::enableLooping(bool enable)
{
    m_loopingEnabled = enable;
    KisConfig cfg;
    cfg.defLoopingEnabled(enable);
}

bool KisAnimation::loopingEnabled()
{
    return m_loopingEnabled;
}

void KisAnimation::setPrevOnionSkinOpacityValues(QList<int> *values)
{
    m_prevOnionSkinOpacityValues = values;
}

QList<int>* KisAnimation::prevOnionSkinOpacityValues()
{
    return m_prevOnionSkinOpacityValues;
}

void KisAnimation::setNextOnionSkinOpacityValues(QList<int> *values)
{
    m_nextOnionSkinOpacityValues = values;
}

QList<int>* KisAnimation::nextOnionSkinOpacityValues()
{
    return m_nextOnionSkinOpacityValues;
}

void KisAnimation::setPrevOnionSkinColor(QColor color)
{
    m_prevOnionSkinColor = color;
}

QColor KisAnimation::prevOnionSkinColor()
{
    return m_prevOnionSkinColor;
}

void KisAnimation::setNextOnionSkinColor(QColor color)
{
    m_nextOnionSkinColor = color;
}

QColor KisAnimation::nextOnionSkinColor()
{
    return m_nextOnionSkinColor;
}
