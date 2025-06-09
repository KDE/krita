/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <unordered_map>

#include <QDebug>
#include <QElapsedTimer>


namespace TestUtil {

template <typename T>
struct TagWrapper
{
    static constexpr bool shouldPrintTag = true;
    TagWrapper(T t) : tag(t) {}

    T tag;
};

template <typename T>
QDebug operator<<(QDebug dbg, const TagWrapper<T> &t)
{
    dbg.nospace() << t.tag;
    return dbg.space();
}

namespace detail {
    struct notag {};
}

template <>
struct TagWrapper<detail::notag>
{
    TagWrapper(detail::notag) {}
    static constexpr bool shouldPrintTag = false;
};

template <typename TagType = detail::notag>
class MeasureAvgPortion
{
public:
    MeasureAvgPortion(int period, TagType tag = {}) : m_period(period), m_val(0), m_total(0), m_cycles(0), m_tagWrapper(tag) { }

    ~MeasureAvgPortion() { printValues(true); }

    void addVal(int x)
    {
        m_val += x;
        m_valMax = std::max(m_valMax, qint64(x));
        m_valMin = std::min(m_valMin, qint64(x));
    }

    void addTotal(int x)
    {
        m_total += x;
        m_totalMax = std::max(m_totalMax, qint64(x));
        m_totalMin = std::min(m_totalMin, qint64(x));
        m_cycles++;
        printValues();
    }

private:
    void printValues(bool force = false)
    {
        if (m_cycles > m_period || force) {
            if constexpr (TagWrapper<TagType>::shouldPrintTag) {
                // auto surface = reinterpret_cast<QPlatformSurface*>(m_tagWrapper.tag);
                // auto window = dynamic_cast<QPlatformWindow*>(surface);
                // if (window) {
                //     qDebug() << "=== stat for tag" << m_tag << window << "===";
                // } else {
                //     qDebug() << "=== stat for tag" << m_tag << "===";
                // }
                qDebug() << "=== stat for tag" << m_tagWrapper << "===";
            } else {
                qDebug() << "=== stat ===";
            }

            qDebug() << "Val / Total:" << qreal(m_val) / qreal(m_total);
            qDebug() << "Avg. Val:   " << qreal(m_val) / m_cycles
                     << "min:" << m_valMin << "max:" << m_valMax;
            qDebug() << "Avg. Total: " << qreal(m_total) / m_cycles
                     << "min:" << m_totalMin << "max:" << m_totalMax;
            qDebug().nospace() << "  (val: " << m_val << ", total: " << m_total
                               << ", cycles:" << m_cycles << ")";

            m_val = 0;
            m_total = 0;
            m_cycles = 0;
            m_totalMin = std::numeric_limits<qint64>::max();
            m_totalMax = std::numeric_limits<qint64>::min();
            m_valMin = std::numeric_limits<qint64>::max();
            m_valMax = std::numeric_limits<qint64>::min();
        }
    }

private:
    int m_period;
    qint64 m_val;
    qint64 m_valMax = std::numeric_limits<qint64>::min();
    qint64 m_valMin = std::numeric_limits<qint64>::max();
    qint64 m_total;
    qint64 m_totalMax = std::numeric_limits<qint64>::min();
    qint64 m_totalMin = std::numeric_limits<qint64>::max();
    qint64 m_cycles;
    TagWrapper<TagType> m_tagWrapper;
};

template <typename TagType>
struct PerObjectMetric
{
    struct DelayMeasure {
        MeasureAvgPortion<TagType> portion;
        QElapsedTimer timer;
    };

    auto getIterator(TagType tag) {
        auto it = hash.find(tag);
        if (it != hash.end()) {
            return it;
        } else {
            bool unused;
            std::tie(it, unused) = hash.emplace(tag, DelayMeasure{ { 60, tag }, {} });
            return it;
        }
    }

    void startFrame(TagType tag) {
        auto it = getIterator(tag);
        DelayMeasure &delay = it->second;

        if (delay.timer.isValid()) {
            delay.portion.addTotal(delay.timer.restart());
        } else {
            delay.timer.start();
        }
    }

    void endFrame(TagType tag) {
        auto it = getIterator(tag);
        DelayMeasure &delay = it->second;

        Q_ASSERT(delay.timer.isValid());
        delay.portion.addVal(delay.timer.elapsed());
    }

    using MapType = std::unordered_map<TagType, DelayMeasure>;

    MapType hash;
};

}

/// Usage:
//
// QDebug operator<<(QDebug dbg, const TestUtil::TagWrapper<QPlatformSurface *> &t)
// {
//     if (qApp) {
//         auto surface = reinterpret_cast<QPlatformSurface*>(t.tag);
//         auto window = dynamic_cast<QPlatformWindow*>(surface);
//         dbg.nospace() << window;
//     } else {
//         dbg.nospace() << "<deleted>";
//     }
//
//     return dbg.space();
// }
//
// static TestUtil::PerObjectMetric<QPlatformSurface*> SwapCounter;