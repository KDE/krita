/*
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

#include <KoImageResource.h>
#include <kdebug.h>
#include <qlineedit.h>
#include <qimage.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <qvalidator.h>

#include <kglobal.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <kcolordialog.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "kis_view.h"
#include "kis_palette.h"
#include "kis_palette_view.h"
#include "kis_custom_palette.h"
#include "kis_resource_mediator.h"
#include "kis_resourceserver.h"

KisCustomPalette::KisCustomPalette(QWidget *parent, const char* name, const QString& caption, KisView* view)
    : KisWdgCustomPalette(parent, name), m_view(view)
{
    Q_ASSERT(m_view);
    m_mediator = 0;
    m_server = 0;
    m_editMode = false;
    setWindowTitle(caption);

    m_palette = new KisPalette();
    m_ownPalette = true;
    this->view->setPalette(m_palette);

    connect(addColor, SIGNAL(pressed()), this, SLOT(slotAddNew()));
    connect(removeColor, SIGNAL(pressed()), this, SLOT(slotRemoveCurrent()));
    connect(addPalette, SIGNAL(pressed()), this, SLOT(slotAddPredefined()));
}

KisCustomPalette::~KisCustomPalette() {
    if (m_ownPalette)
        delete m_palette;
}

void KisCustomPalette::setPalette(KisPalette* p) {
    if (m_ownPalette)
        delete m_palette;
    m_ownPalette = false;
    m_palette = p;
    view->setPalette(m_palette);
}

void KisCustomPalette::setEditMode(bool b) {
    m_editMode = b;

    if (m_editMode) {
        addPalette->setText(i18n("Save changes"));
    } else {
        addPalette->setText(i18n("Add to Predefined Palettes"));
    }
}

void KisCustomPalette::slotAddNew() {
    // Let the user select a new color
    // FIXME also let him add the current paint color to the palette
    // or even better, let the color picker have an option 'Add to palette'!

    QColor color;
    int result = KColorDialog::getColor(color);
    if (result != KColorDialog::Accepted)
        return;

    bool ok;
    QRegExpValidator validator(QRegExp(".*"), this);
    QString name = KInputDialog::getText(i18n("Add Color to Palette"),
                                         i18n("Color name (optional):"),
                                         QString::null, &ok,
                                         0, &validator);
    if (!ok)
        return;

    KisPaletteEntry entry;
    entry.color = color;
    entry.name = name;

    m_palette->add(entry);

    // Just reload the palette completely for the view updating
    view->setPalette(m_palette);
}

void KisCustomPalette::slotRemoveCurrent() {
    m_palette->remove(view->currentEntry());
    // Just reload the palette completely for the view updating
    view->setPalette(m_palette);
}

void KisCustomPalette::slotAddPredefined() {
    m_palette->setName(palettename->text());

    if (!m_editMode) {
        // Save in the directory that is likely to be: ~/.kde/share/apps/krita/palettes
        // a unique file with this palettename
        QString dir = KGlobal::dirs()->saveLocation("data", "krita/palettes");
        QString extension;

        extension = ".gpl";
        KTempFile file(dir, extension);
        file.close(); // If we don't, and palette->save first, it might get truncated!

        // Save it to that file
        m_palette->setFilename(file.name());
    } else {
        // The filename is already set
    }

    if (!m_palette->save()) {
        KMessageBox::error(0, i18n("Cannot write to palette file %1. Maybe it is read-only.")
                                   .arg(m_palette->filename()), i18n("Palette"));
        return;
    }

    // Add it to the palette server, so that it automatically gets to the mediators, and
    // so to the other choosers can pick it up, if they want to
    // This probably leaks!
    if (m_server)
        m_server->addResource(new KisPalette(*m_palette));
}


#include "kis_custom_palette.moc"
