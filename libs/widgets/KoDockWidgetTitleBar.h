/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2007 Marijn Kruisselbrink <mkruisselbrink@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KODOCKWIDGETTITLEBAR_H_
#define KODOCKWIDGETTITLEBAR_H_

#include "kritawidgets_export.h"
#include <QDockWidget>

/**
 * @short A custom title bar for dock widgets.
 *
 * Allow customization such as hidden text.
 * 
 * @see KoDockWidgetTitleBarButton
 */
class KRITAWIDGETS_EXPORT KoDockWidgetTitleBar : public QWidget
{
    Q_OBJECT
public:
    explicit KoDockWidgetTitleBar(QDockWidget *dockWidget);
    ~KoDockWidgetTitleBar() override;

    QSize minimumSizeHint() const override; ///< reimplemented from QWidget
    QSize sizeHint() const override;  ///< reimplemented from QWidget

    enum TextVisibilityMode {TextCanBeInvisible, FullTextAlwaysVisible};
    /// Define whether the minimal width should ensure that the full text is visible.
    /// textVisibilityMode is FullTextAlwaysVisible by default
    void setTextVisibilityMode(TextVisibilityMode textVisibilityMode);

    void updateIcons();

public Q_SLOTS:
    void setLocked(bool locked);

protected:
    void paintEvent(QPaintEvent* event) override; ///< reimplemented from QWidget
    void resizeEvent(QResizeEvent* event) override; ///< reimplemented from QWidget
private:
    Q_PRIVATE_SLOT(d, void toggleFloating())
    Q_PRIVATE_SLOT(d, void topLevelChanged(bool topLevel))
    Q_PRIVATE_SLOT(d, void featuresChanged(QDockWidget::DockWidgetFeatures))

    class Private;
    Private * const d;
};

#endif // KODOCKWIDGETTITLEBAR_H_
