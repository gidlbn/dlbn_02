/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
**
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** Commercial Usage
**
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at http://qt.nokia.com/contact.
**
**************************************************************************/

#include "futureprogress.h"
#include "progressbar.h"

#include <QtGui/QColor>
#include <QtGui/QVBoxLayout>
#include <QtGui/QMenu>
#include <QtGui/QProgressBar>
#include <QtGui/QHBoxLayout>
#include <QtGui/QPainter>
#include <QtGui/QMouseEvent>
#include <QtCore/QTimer>
#include <QtCore/QCoreApplication>
#include <QtCore/QPropertyAnimation>
#include <QtCore/QSequentialAnimationGroup>
#include <utils/stylehelper.h>

using namespace Core;
using namespace Core::Internal;

const int notificationTimeout = 8000;
const int shortNotificationTimeout = 1000;

namespace Core {
namespace Internal {

class FadeWidgetHack : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(float opacity READ opacity WRITE setOpacity)
public:
    FadeWidgetHack(QWidget *parent):QWidget(parent), m_opacity(0){
        setAttribute(Qt::WA_TransparentForMouseEvents);
    }
    void paintEvent(QPaintEvent *);

    void setOpacity(float o) { m_opacity = o; update(); }
    float opacity() const { return m_opacity; }

private:
    float m_opacity;
};

} // namespace Internal
} // namespace Core

void FadeWidgetHack::paintEvent(QPaintEvent *)
{
    if (m_opacity == 0)
        return;

    QPainter p(this);
    p.setOpacity(m_opacity);
    if (m_opacity > 0)
        Utils::StyleHelper::verticalGradient(&p, rect(), rect());
}

/*!
    \mainclass
    \class Core::FutureProgress
    \brief The FutureProgress class is used to adapt the appearance of
    progress indicators that were created through the ProgressManager class.

    Use the instance of this class that was returned by
    ProgressManager::addTask() to define a widget that
    should be shown below the progress bar, or to change the
    progress title.
    Also use it to react on the event that the user clicks on
    the progress indicator (which can be used to e.g. open a more detailed
    view, or the results of the task).
*/

/*!
    \fn void FutureProgress::clicked()
    Connect to this signal to get informed when the user clicks on the
    progress indicator.
*/

/*!
    \fn void FutureProgress::finished()
    Another way to get informed when the task has finished.
*/

/*!
    \fn QWidget FutureProgress::widget() const
    Returns the custom widget that is shown below the progress indicator.
*/

/*!
    \fn FutureProgress::FutureProgress(QWidget *parent)
    \internal
*/
FutureProgress::FutureProgress(QWidget *parent)
        : QWidget(parent),
        m_progress(new ProgressBar),
        m_widget(0),
        m_widgetLayout(new QHBoxLayout)
{
    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);
    layout->addWidget(m_progress);
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addLayout(m_widgetLayout);
    m_widgetLayout->setContentsMargins(7, 0, 7, 2);
    m_widgetLayout->setSpacing(0);

    connect(&m_watcher, SIGNAL(started()), this, SLOT(setStarted()));
    connect(&m_watcher, SIGNAL(finished()), this, SLOT(setFinished()));
    connect(&m_watcher, SIGNAL(progressRangeChanged(int,int)), this, SLOT(setProgressRange(int,int)));
    connect(&m_watcher, SIGNAL(progressValueChanged(int)), this, SLOT(setProgressValue(int)));
    connect(&m_watcher, SIGNAL(progressTextChanged(const QString&)),
            this, SLOT(setProgressText(const QString&)));
    connect(m_progress, SIGNAL(clicked()), this, SLOT(cancel()));

    m_keep = false;
    m_waitingForUserInteraction = false;
    m_faderWidget = new FadeWidgetHack(this);
}

/*!
    \fn FutureProgress::~FutureProgress()
    \internal
*/
FutureProgress::~FutureProgress()
{
    if (m_widget)
        delete m_widget;
}

/*!
    \fn void FutureProgress::setWidget(QWidget *widget)
    Sets the \a widget to show below the progress bar.
    This will be destroyed when the progress indicator is destroyed.
    Default is to show no widget below the progress indicator.
*/
void FutureProgress::setWidget(QWidget *widget)
{
    if (m_widget)
        delete m_widget;
    QSizePolicy sp = widget->sizePolicy();
    sp.setHorizontalPolicy(QSizePolicy::Ignored);
    widget->setSizePolicy(sp);
    m_widget = widget;
    if (m_widget)
        m_widgetLayout->addWidget(m_widget);
}

/*!
    \fn void FutureProgress::setTitle(const QString &title)
    Changes the \a title of the progress indicator.
*/
void FutureProgress::setTitle(const QString &title)
{
    m_progress->setTitle(title);
}

/*!
    \fn QString FutureProgress::title() const
    Returns the title of the progress indicator.
*/
QString FutureProgress::title() const
{
    return m_progress->title();
}

void FutureProgress::cancel()
{
    m_watcher.future().cancel();
}

void FutureProgress::updateToolTip(const QString &text)
{
    setToolTip("<b>" + title() + "</b><br>" + text);
}

void FutureProgress::setStarted()
{
    m_progress->reset();
    m_progress->setError(false);
    m_progress->setRange(m_watcher.progressMinimum(), m_watcher.progressMaximum());
    m_progress->setValue(m_watcher.progressValue());
}


bool FutureProgress::eventFilter(QObject *, QEvent *e)
{
    if (m_waitingForUserInteraction
        && (e->type() == QEvent::MouseMove || e->type() == QEvent::KeyPress)) {
        qApp->removeEventFilter(this);
        QTimer::singleShot(notificationTimeout, this, SLOT(fadeAway()));
    }
    return false;
}

void FutureProgress::setFinished()
{
    updateToolTip(m_watcher.future().progressText());

    // Special case for concurrent jobs that don't use QFutureInterface to report progress
    if (m_watcher.progressMinimum() == 0 && m_watcher.progressMaximum() == 0) {
        m_progress->setRange(0, 1);
        m_progress->setValue(1);
    }

    if (m_watcher.future().isCanceled()) {
        m_progress->setError(true);
    } else {
        m_progress->setError(false);
    }
    emit finished();
    if (m_keep) {
        m_waitingForUserInteraction = true;
        qApp->installEventFilter(this);
    } else if (!m_progress->hasError()) {
        QTimer::singleShot(shortNotificationTimeout, this, SLOT(fadeAway()));
    }
}

void FutureProgress::setProgressRange(int min, int max)
{
    m_progress->setRange(min, max);
}

void FutureProgress::setProgressValue(int val)
{
    m_progress->setValue(val);
}

void FutureProgress::setProgressText(const QString &text)
{
    updateToolTip(text);
}

/*!
    \fn void FutureProgress::setFuture(const QFuture<void> &future)
    \internal
*/
void FutureProgress::setFuture(const QFuture<void> &future)
{
    m_watcher.setFuture(future);
}

/*!
    \fn QFuture<void> FutureProgress::future() const
    Returns a QFuture object that represents this running task.
*/
QFuture<void> FutureProgress::future() const
{
    return m_watcher.future();
}

/*!
    \fn void FutureProgress::mousePressEvent(QMouseEvent *event)
    \internal
*/
void FutureProgress::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        emit clicked();
    QWidget::mousePressEvent(event);
}

void FutureProgress::resizeEvent(QResizeEvent *)
{
    m_faderWidget->setGeometry(rect());
}

/*!
    \fn bool FutureProgress::hasError() const
    Returns the error state of this progress indicator.
*/
bool FutureProgress::hasError() const
{
    return m_progress->hasError();
}

void FutureProgress::fadeAway()
{
    m_faderWidget->raise();
    QSequentialAnimationGroup *group = new QSequentialAnimationGroup;
    QPropertyAnimation *animation = new QPropertyAnimation(m_faderWidget, "opacity");
    animation->setDuration(600);
    animation->setEndValue(1.0);
    group->addAnimation(animation);
    animation = new QPropertyAnimation(this, "maximumHeight");
    animation->setDuration(120);
    animation->setEasingCurve(QEasingCurve::InCurve);
    animation->setStartValue(sizeHint().height());
    animation->setEndValue(0.0);
    group->addAnimation(animation);
    group->start(QAbstractAnimation::DeleteWhenStopped);

    connect(group, SIGNAL(finished()), this, SIGNAL(removeMe()));
}

#include "futureprogress.moc"
