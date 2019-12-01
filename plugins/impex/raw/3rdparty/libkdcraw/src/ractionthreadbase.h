/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="https://www.digikam.org">https://www.digikam.org</a>
 *
 * @date   2011-12-28
 * @brief  re-implementation of action thread using threadweaver
 *
 * @author Copyright (C) 2011-2015 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 * @author Copyright (C) 2014 by Veaceslav Munteanu
 *         <a href="mailto:veaceslav dot munteanu90 at gmail dot com">veaceslav dot munteanu90 at gmail dot com</a>
 * @author Copyright (C) 2011-2012 by A Janardhan Reddy
 *         <a href="annapareddyjanardhanreddy at gmail dot com">annapareddyjanardhanreddy at gmail dot com</a>
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

#ifndef RACTION_THREAD_BASE_H
#define RACTION_THREAD_BASE_H

// Qt includes

#include <QtCore/QThread>

// Local includes


#include "ractionjob.h"

namespace KDcrawIface
{

class  RActionThreadBase : public QThread
{
    Q_OBJECT

public:

    RActionThreadBase(QObject* const parent=0);
    ~RActionThreadBase() override;

    /** Adjust maximum number of threads used to parallelize collection of job processing.
     */
    void setMaximumNumberOfThreads(int n);

    /** Return the maximum number of threads used to parallelize collection of job processing.
     */
    int  maximumNumberOfThreads() const;

    /** Reset maximum number of threads used to parallelize collection of job processing to max core detected on computer.
     *  This method is called in contructor.
     */
    void defaultMaximumNumberOfThreads();

    /** Cancel processing of current jobs under progress.
     */
    void cancel();

protected:

    /** Main thread loop used to process jobs in todo list.
     */
    void run() override;

    /** Append a collection of jobs to process into QThreadPool.
     *  Jobs are add to pending lists and will be deleted by RActionThreadBase, not QThreadPool.
     */
    void appendJobs(const RJobCollection& jobs);

    /** Return true if list of pending jobs to process is empty.
     */
    bool isEmpty() const;

protected Q_SLOTS:

    void slotJobFinished();

private:

    class Private;
    Private* const d;
};

}  // namespace KDcrawIface

#endif // RACTION_THREAD_BASE_H
