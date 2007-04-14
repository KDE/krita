/*
 *  Copyright (c) 2004,2006-2007 Cyrille Berger <cberger@cberger.net>
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
#include "kis_filter.h"

#include <QString>

#include <kconfig.h>

#include "kis_types.h"
#include "kis_filter_configuration.h"

const KoID KisFilter::ConfigDefault = KoID("Default", i18n("Default"));
const KoID KisFilter::ConfigDesigner = KoID("Designer", i18n("Designer"));
const KoID KisFilter::ConfigLastUsed = KoID("Last Used", i18n("Last used"));

KisFilter::KisFilter(const KoID& id, const QString & category, const QString & entry)
    : KisProgressSubject(0, id.id().toLatin1())
    , m_id(id)
    , m_progressDisplay(0)
    , m_category(category)
    , m_entry(entry)
{
}

KisFilterConfiguration * KisFilter::designerConfiguration(const KisPaintDeviceSP)
{
    return new KisFilterConfiguration(m_id.id(), 0);
}

KisFilterConfigWidget * KisFilter::createConfigurationWidget(QWidget *, const KisPaintDeviceSP)
{
    return 0;
}

KisFilterConfiguration * KisFilter::defaultConfiguration(const KisPaintDeviceSP pd)
{
  if(existInBookmark(KisFilter::ConfigDefault.id()))
  {
    return loadFromBookmark(KisFilter::ConfigDefault.id());
  }
  else if(existInBookmark(KisFilter::ConfigLastUsed.id()))
  {
    return loadFromBookmark(KisFilter::ConfigLastUsed.id());
  }
  else {
    return designerConfiguration(pd);
  }
}

QHash<QString, KisFilterConfiguration*> KisFilter::bookmarkedConfigurations( const KisPaintDeviceSP )
{
  QHash<QString, KisFilterConfiguration*> bookmarkedConfig;
  bookmarkedConfig.insert(i18n("designer"), designerConfiguration(0));
  return bookmarkedConfig;
}


void KisFilter::saveToBookmark(const QString& configname, KisFilterConfiguration* config)
{
    if ( config == 0 ) return;

    KConfigGroup cfg = KGlobal::config()->group(configEntryGroup());
    cfg.writeEntry(configname,config->toString());
}

KisFilterConfiguration* KisFilter::loadFromBookmark(const QString& configname)
{
    if(not existInBookmark(configname)) return 0;
    KConfigGroup cfg = KGlobal::config()->group(configEntryGroup());
    KisFilterConfiguration* config = new KisFilterConfiguration(id(), 1);
    config->fromXML(cfg.readEntry<QString>(configname, ""));
    return config;
}

bool KisFilter::existInBookmark(const QString& configname)
{
    KSharedConfig::Ptr cfg = KGlobal::config();
    QMap< QString, QString > m = cfg->entryMap(configEntryGroup());
    return (m.find(configname) != m.end());
}


void KisFilter::setProgressDisplay(KisProgressDisplayInterface * progressDisplay)
{
    m_progressDisplay = progressDisplay;
}


void KisFilter::enableProgress() {
    m_progressEnabled = true;
    m_cancelRequested = false;
}

void KisFilter::disableProgress() {
    m_progressEnabled = false;
    m_cancelRequested = false;
}

void KisFilter::setProgressTotalSteps(qint32 totalSteps)
{
    if (m_progressEnabled) {

        m_progressTotalSteps = totalSteps;
        m_lastProgressPerCent = 0;
        m_progressSteps = 0;
        emit notifyProgress(0);
    }
}

void KisFilter::setProgress(qint32 progress)
{
    if (m_progressEnabled) {
        qint32 progressPerCent = (progress * 100) / m_progressTotalSteps;
        m_progressSteps = progress;

        if (progressPerCent != m_lastProgressPerCent) {

            m_lastProgressPerCent = progressPerCent;
            emit notifyProgress(progressPerCent);
        }
    }
}

void KisFilter::incProgress()
{
    setProgress(++m_progressSteps);

}

void KisFilter::setProgressStage(const QString& stage, qint32 progress)
{
    if (m_progressEnabled) {

        qint32 progressPerCent = (progress * 100) / m_progressTotalSteps;

        m_lastProgressPerCent = progressPerCent;
        emit notifyProgressStage(stage, progressPerCent);
    }
}

void KisFilter::setProgressDone()
{
    if (m_progressEnabled) {
        emit notifyProgressDone();
    }
}


bool KisFilter::autoUpdate() {
    return m_autoUpdate;
}

void KisFilter::setAutoUpdate(bool set) {
    m_autoUpdate = set;
}

QRect KisFilter::enlargeRect(QRect rect, KisFilterConfiguration* c) const {
    int margin = overlapMarginNeeded(c);
    rect.setLeft(rect.left() - margin);
    rect.setTop(rect.top() - margin);
    rect.setRight(rect.right() + margin);
    rect.setBottom(rect.bottom() + margin);
    return rect;
}

#include "kis_filter.moc"
