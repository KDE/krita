/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_GUIDES_DECORATION_H
#define __KIS_GUIDES_DECORATION_H

#include <QScopedPointer>
#include "kis_canvas_decoration.h"

class KisGuidesConfig;

static const QString GUIDES_DECORATION_ID = "guides-decoration";

class KRITAUI_EXPORT KisGuidesDecoration : public KisCanvasDecoration
{
    Q_OBJECT
public:
    KisGuidesDecoration(QPointer<KisView> view);
    ~KisGuidesDecoration() override;


    void setGuidesConfig(const KisGuidesConfig &value);
    const KisGuidesConfig& guidesConfig() const;

protected:
    void drawDecoration(QPainter& gc, const QRectF& updateArea, const KisCoordinatesConverter *converter, KisCanvas2 *canvas) override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_GUIDES_DECORATION_H */
