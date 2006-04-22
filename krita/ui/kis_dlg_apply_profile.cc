/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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

#include <qcombobox.h>
#include <klocale.h>
#include <QButtonGroup>

#include "kis_factory.h"
#include "kis_colorspace_factory_registry.h"
#include "kis_types.h"
#include "kis_profile.h"
#include "kis_colorspace.h"
#include "kis_dlg_apply_profile.h"
#include "kis_config.h"
#include "kis_id.h"
#include <kis_meta_registry.h>
#include "kis_cmb_idlist.h"
#include "squeezedcombobox.h"

// XXX: Hardcode RGBA name. This should be a constant, somewhere.
KisDlgApplyProfile::KisDlgApplyProfile(QWidget *parent, const char *name)
    : super(parent, "", Ok | Cancel)
{
    setObjectName(name);
    setWindowTitle(i18n("Apply Image Profile to Clipboard Data"));
    m_page = new WdgApplyProfile(this);

    setMainWidget(m_page);
    resize(m_page->sizeHint());

    // XXX: This is BAD! (bsar)
    fillCmbProfiles(KisID("RGBA", ""));

    m_intentButtonGroup = new QButtonGroup(this);

    m_intentButtonGroup->addButton(m_page->radioPerceptual, INTENT_PERCEPTUAL);
    m_intentButtonGroup->addButton(m_page->radioRelativeColorimetric, INTENT_RELATIVE_COLORIMETRIC);
    m_intentButtonGroup->addButton(m_page->radioSaturation, INTENT_SATURATION);
    m_intentButtonGroup->addButton(m_page->radioAbsoluteColorimetric, INTENT_ABSOLUTE_COLORIMETRIC);

    KisConfig cfg;
    QAbstractButton *currentIntentButton = m_intentButtonGroup->button(cfg.renderIntent());
    Q_ASSERT(currentIntentButton);

    if (currentIntentButton) {
        currentIntentButton->setChecked(true);
    }
}

KisDlgApplyProfile::~KisDlgApplyProfile()
{
    delete m_page;
}


KisProfile *  KisDlgApplyProfile::profile() const
{
    QString profileName;

    profileName = m_page->cmbProfile->currentText();

    return KisMetaRegistry::instance()->csRegistry()->getProfileByName(profileName);
}

int KisDlgApplyProfile::renderIntent() const
{
    return m_intentButtonGroup->checkedId();
}


// XXX: Copy & paste from kis_custom_image_widget -- refactor to separate class
void KisDlgApplyProfile::fillCmbProfiles(const KisID & s)
{
    m_page->cmbProfile->clear();

    if (!KisMetaRegistry::instance()->csRegistry()->exists(s)) {
        return;
    }

    KisColorSpaceFactory * csf = KisMetaRegistry::instance()->csRegistry()->get(s);
    if (csf == 0) return;

    Q3ValueVector<KisProfile *>  profileList = KisMetaRegistry::instance()->csRegistry()->profilesFor( csf );
        Q3ValueVector<KisProfile *> ::iterator it;
        for ( it = profileList.begin(); it != profileList.end(); ++it ) {
            m_page->cmbProfile->addSqueezedItem((*it)->productName());
    }
    m_page->cmbProfile->setCurrentIndexFromText(csf->defaultProfile());
}

#include "kis_dlg_apply_profile.moc"

