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

#include <QComboBox>
#include <klocale.h>
#include <QButtonGroup>

#include "kis_factory.h"
#include "KoColorSpaceRegistry.h"
#include "kis_types.h"
#include "KoColorProfile.h"
#include "KoColorSpace.h"
#include "kis_dlg_apply_profile.h"
#include "kis_config.h"
#include "KoID.h"
#include <kis_meta_registry.h>
#include "kis_cmb_idlist.h"
#include "squeezedcombobox.h"

// XXX: Hardcode RGBA name. This should be a constant, somewhere.
KisDlgApplyProfile::KisDlgApplyProfile(QWidget *parent, const char *name)
    : super(parent )
{
    setButtons( Ok |Cancel );
    setObjectName(name);
    setWindowTitle(i18n("Apply Image Profile to Clipboard Data"));
    m_page = new WdgApplyProfile(this);

    setMainWidget(m_page);
    resize(m_page->sizeHint());

    // XXX: This is BAD! (bsar)
    fillCmbProfiles(KoID("RGBA", ""));

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


KoColorProfile *  KisDlgApplyProfile::profile() const
{
    QString profileName;

    profileName = m_page->cmbProfile->currentText();

    return KisMetaRegistry::instance()->csRegistry()->profileByName(profileName);
}

int KisDlgApplyProfile::renderIntent() const
{
    return m_intentButtonGroup->checkedId();
}


// XXX: Copy & paste from kis_custom_image_widget -- refactor to separate class
void KisDlgApplyProfile::fillCmbProfiles(const KoID & s)
{
    m_page->cmbProfile->clear();

    if (!KisMetaRegistry::instance()->csRegistry()->exists(s)) {
        return;
    }

    KoColorSpaceFactory * csf = KisMetaRegistry::instance()->csRegistry()->get(s);
    if (csf == 0) return;

    QList<KoColorProfile *> profileList = KisMetaRegistry::instance()->csRegistry()->profilesFor( csf );

    foreach (KoColorProfile *profile, profileList) {
            m_page->cmbProfile->addSqueezedItem(profile->productName());
    }
    m_page->cmbProfile->setCurrent(csf->defaultProfile());
}

#include "kis_dlg_apply_profile.moc"

