/*
 *  SPDX-FileCopyrightText: 2006 Bart Coppens <kde@bartcoppens.be>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_CUSTOM_PATTERN_H_
#define KIS_CUSTOM_PATTERN_H_

#include <QObject>
#include <QShowEvent>

#include <KoResourceServer.h>

#include "ui_wdgcustompattern.h"

#include <KoPattern.h>
#include <KoResource.h>

class KisViewManager;

class KisWdgCustomPattern : public QWidget, public Ui::KisWdgCustomPattern
{
    Q_OBJECT

public:
    KisWdgCustomPattern(QWidget *parent, const char *name) : QWidget(parent) {
        setObjectName(name); setupUi(this);
    }
};

class KisCustomPattern : public KisWdgCustomPattern
{
    Q_OBJECT
public:
    KisCustomPattern(QWidget *parent, const char* name, const QString& caption, KisViewManager* view);
    ~KisCustomPattern() override;

Q_SIGNALS:
    void activatedResource(KoResourceSP);
    void addPattern(KoPatternSP);

private Q_SLOTS:
    void slotAddPredefined();
    void slotUsePattern();
    void slotUpdateCurrentPattern();

private:
    void createPattern();
    KisViewManager* m_view {0};
    KoPatternSP m_pattern;
    KoResourceServer<KoPattern>* m_rServer {0};
};


#endif // KIS_CUSTOM_PATTERN_H_
