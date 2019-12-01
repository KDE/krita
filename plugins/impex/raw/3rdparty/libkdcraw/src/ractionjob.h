/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="https://www.digikam.org">https://www.digikam.org</a>
 *
 * @date   2014-15-11
 * @brief  QRunnable job extended with QObject features
 *
 * @author Copyright (C) 2011-2015 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 * @author Copyright (C) 2014 by Veaceslav Munteanu
 *         <a href="mailto:veaceslav dot munteanu90 at gmail dot com">veaceslav dot munteanu90 at gmail dot com</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef RACTIONJOB_H
#define RACTIONJOB_H

// Qt includes

#include <QObject>
#include <QRunnable>

// Local includes



namespace KDcrawIface
{

class  RActionJob : public QObject,
                                    public QRunnable
{
    Q_OBJECT

public:

    /** Constructor which delegate deletion of QRunnable instance to RActionThreadBase, not QThreadPool.
     */
    RActionJob();

    /** Re-implement destructor in you implementation. Don't forget to cancel job.
     */
    ~RActionJob() override;

Q_SIGNALS:

    /** Use this signal in your implementation to inform RActionThreadBase manager that job is started
     */
    void signalStarted();

    /** Use this signal in your implementation to inform RActionThreadBase manager the job progress
     */
    void signalProgress(int);

    /** Use this signal in your implementation to inform RActionThreadBase manager the job is done.
     */
    void signalDone();

public Q_SLOTS:

    /** Call this method to cancel job.
     */
    void cancel();

protected:

    /** You can use this boolean in your implementation to know if job must be canceled.
     */
    bool m_cancel;
};

/** Define a map of job/priority to process by RActionThreadBase manager. 
 *  Priority value can be used to control the run queue's order of execution.
 *  Zero priority want mean to process job with higher priority.
 */
typedef QMap<RActionJob*, int> RJobCollection;

} // namespace KDcrawIface

#endif // RACTIONJOB_H
