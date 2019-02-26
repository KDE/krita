/*
 * Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA. */

#ifndef ANIMATIONRENDERERIMAGE_H
#define ANIMATIONRENDERERIMAGE_H

#include <QVariant>

#include <KisActionPlugin.h>
class KisAnimationRenderingOptions;
class KisDocument;

class AnimaterionRenderer : public KisActionPlugin
{
    Q_OBJECT
public:
    AnimaterionRenderer(QObject *parent, const QVariantList &);
    ~AnimaterionRenderer() override;

private Q_SLOTS:

    /**
     * @brief slotRenderAnimation
     *
     * Triggered from the renderanimation action. This calls a dialog
     * to set the animation settings and then takes that to call the appropriate exporter,
     * this can be a frame exporter, or it is a KisVideoExport object as defined in
     *  plugins/extensions/impex.
     */
    void slotRenderAnimation();
    /**
     * @brief slotRenderSequenceAgain
     *
     * triggered from the renderanimationagain action. This does not call a dialog, but
     * instead uses the settings to set the animation settings and then takes that to
     * call the appropriate exporter, this can be a frame exporter, or it is a
     * KisVideoExport object as defined in plugins/extensions/impex.
     */
    void slotRenderSequenceAgain();

private:
    void renderAnimationImpl(KisDocument *doc, KisAnimationRenderingOptions encoderOptions);

};

#endif // ANIMATIONRENDERERIMAGE_H
