/*
    Copyright (C) 2017 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "selectionratiosource.h"
#include "abstractdatasource_p.h"
#include "logging_p.h"

#include <QDebug>
#include <QHash>
#include <QItemSelectionModel>
#include <QSettings>
#include <QStringList>
#include <QTime>

#include <memory>

using namespace UserFeedback;

namespace UserFeedback {
class SelectionRatioSourcePrivate : public AbstractDataSourcePrivate
{
public:
    SelectionRatioSourcePrivate();

    void selectionChanged();
    QString selectedValue() const;

    QItemSelectionModel *model;
    std::unique_ptr<QObject> monitor;
    QString description;
    QString previousValue;
    QTime lastChangeTime;
    QHash<QString, int> ratioSet; // data we are currently tracking
    QHash<QString, int> baseRatioSet; // data loaded from storage
    int role;
};

class SelectionMonitor : public QObject
{
    Q_OBJECT
public:
    explicit SelectionMonitor(SelectionRatioSourcePrivate *d) : m_receiver(d) {}
public Q_SLOTS:
    void selectionChanged()
    {
        m_receiver->selectionChanged();
    }
private:
    SelectionRatioSourcePrivate *m_receiver;
};

}

SelectionRatioSourcePrivate::SelectionRatioSourcePrivate()
    : model(nullptr)
    , role(Qt::DisplayRole)
{
}

void SelectionRatioSourcePrivate::selectionChanged()
{
    if (!previousValue.isEmpty() && lastChangeTime.elapsed() > 1000) {
        ratioSet[previousValue] += lastChangeTime.elapsed() / 1000;
    }

    lastChangeTime.start();
    previousValue = selectedValue();
}

QString SelectionRatioSourcePrivate::selectedValue() const
{
    if (!model->hasSelection())
        return QString();
    const auto idxs = model->selectedIndexes();
    Q_ASSERT(!idxs.isEmpty());
    const auto idx = idxs.at(0);
    return idx.data(role).toString();
}

SelectionRatioSource::SelectionRatioSource(QItemSelectionModel* selectionModel, const QString& sampleName)
    : AbstractDataSource(sampleName, new SelectionRatioSourcePrivate)
{
    Q_D(SelectionRatioSource);

    d->model = selectionModel;
    Q_ASSERT(selectionModel);

    d->monitor.reset(new SelectionMonitor(d));
    QObject::connect(selectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)), d->monitor.get(), SLOT(selectionChanged()));
    d->lastChangeTime.start();
    d->selectionChanged();
}

void SelectionRatioSource::setRole(int role)
{
    Q_D(SelectionRatioSource);
    d->role = role;
}

QString SelectionRatioSource::description() const
{
    Q_D(const SelectionRatioSource);
    return d->description;
}

void SelectionRatioSource::setDescription(const QString& desc)
{
    Q_D(SelectionRatioSource);
    d->description = desc;
}

QVariant SelectionRatioSource::data()
{
    Q_D(SelectionRatioSource);
    d->selectionChanged();

    QVariantMap m;
    int total = 0;
    for (auto it = d->ratioSet.constBegin(); it != d->ratioSet.constEnd(); ++it)
        total += it.value() + d->baseRatioSet.value(it.key());

    for (auto it = d->ratioSet.constBegin(); it != d->ratioSet.constEnd(); ++it) {
        double currentValue = it.value() + d->baseRatioSet.value(it.key());
        QVariantMap v;
        v.insert(QStringLiteral("property"), currentValue / (double)(total));
        m.insert(it.key(), v);
    }
    return m;
}

void SelectionRatioSource::load(QSettings *settings)
{
    Q_D(SelectionRatioSource);
    foreach (const auto &value, settings->childKeys()) {
        const auto amount = std::max(settings->value(value, 0).toInt(), 0);
        d->baseRatioSet.insert(value, amount);
        if (!d->ratioSet.contains(value))
            d->ratioSet.insert(value, 0);
    }
}

void SelectionRatioSource::store(QSettings *settings)
{
    Q_D(SelectionRatioSource);
    d->selectionChanged();

    // note that a second process can have updated the data meanwhile!
    for (auto it = d->ratioSet.begin(); it != d->ratioSet.end(); ++it) {
        if (it.value() == 0)
            continue;
        const auto oldValue = std::max(settings->value(it.key(), 0).toInt(), 0);
        const auto newValue = oldValue + it.value();
        settings->setValue(it.key(), newValue);
        *it = 0;
        d->baseRatioSet.insert(it.key(), newValue);
    }
}

void SelectionRatioSource::reset(QSettings* settings)
{
    Q_D(SelectionRatioSource);
    d->baseRatioSet.clear();
    d->ratioSet.clear();
    settings->remove(QString());
}

#include "selectionratiosource.moc"
