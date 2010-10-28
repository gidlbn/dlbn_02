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

#include "taskwindow.h"

#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/vcsmanager.h>
#include <coreplugin/iversioncontrol.h>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/icore.h>
#include <coreplugin/uniqueidmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <texteditor/basetexteditor.h>
#include <texteditor/itexteditor.h>

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QDebug>
#include <QtGui/QApplication>
#include <QtGui/QClipboard>
#include <QtGui/QKeyEvent>
#include <QtGui/QListView>
#include <QtGui/QPainter>
#include <QtGui/QStyledItemDelegate>
#include <QtGui/QSortFilterProxyModel>
#include <QtGui/QMenu>

namespace {
const int TASK_ICON_SIZE = 16;
const int TASK_ICON_MARGIN = 2;
}

namespace ProjectExplorer {

namespace Internal {

class TaskView : public QListView
{
public:
    TaskView(QWidget *parent = 0);
    ~TaskView();
    void resizeEvent(QResizeEvent *e);
    void keyPressEvent(QKeyEvent *e);
};

class TaskDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    TaskDelegate(QObject * parent = 0);
    ~TaskDelegate();
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

    // TaskView uses this method if the size of the taskview changes
    void emitSizeHintChanged(const QModelIndex &index);

public slots:
    void currentChanged(const QModelIndex &current, const QModelIndex &previous);

private:
    void generateGradientPixmap(int width, int height, QColor color, bool selected) const;
};

class TaskWindowContext : public Core::IContext
{
public:
    TaskWindowContext(QWidget *widget);
    virtual QList<int> context() const;
    virtual QWidget *widget();
private:
    QWidget *m_taskList;
    QList<int> m_context;
};

class TaskModel : public QAbstractItemModel
{
public:
    // Model stuff
    TaskModel();
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    QStringList categoryIds() const;
    QString categoryDisplayName(const QString &categoryId) const;
    void addCategory(const QString &categoryId, const QString &categoryName);

    QList<Task> tasks(const QString &categoryId = QString()) const;
    void addTask(const Task &task);
    void removeTask(const Task &task);
    void clearTasks(const QString &categoryId = QString());

    int sizeOfFile();
    int sizeOfLineNumber();
    void setFileNotFound(const QModelIndex &index, bool b);

    enum Roles { File = Qt::UserRole, Line, Description, FileNotFound, Type, Category, Icon, Task_t };

    QIcon taskTypeIcon(Task::TaskType t) const;

    int taskCount();
    int errorTaskCount();

private:
    QHash<QString,QString> m_categories; // category id -> display name
    QList<Task> m_tasks;   // all tasks (in order of insertion)
    QMap<QString,QList<Task> > m_tasksInCategory; // categoryId->tasks

    QHash<QString,bool> m_fileNotFound;
    int m_maxSizeOfFileName;
    const QIcon m_errorIcon;
    const QIcon m_warningIcon;
    int m_taskCount;
    int m_errorTaskCount;
    int m_sizeOfLineNumber;
};

class TaskFilterModel : public QSortFilterProxyModel
{
public:
    TaskFilterModel(TaskModel *sourceModel, QObject *parent = 0);

    TaskModel *taskModel() const;

    bool filterIncludesUnknowns() const { return m_includeUnknowns; }
    void setFilterIncludesUnknowns(bool b) { m_includeUnknowns = b; invalidateFilter(); }

    bool filterIncludesWarnings() const { return m_includeWarnings; }
    void setFilterIncludesWarnings(bool b) { m_includeWarnings = b; invalidateFilter(); }

    bool filterIncludesErrors() const { return m_includeErrors; }
    void setFilterIncludesErrors(bool b) { m_includeErrors = b; invalidateFilter(); }

    QStringList filteredCategories() const { return m_categoryIds; }
    void setFilteredCategories(const QStringList &categoryIds) { m_categoryIds = categoryIds; invalidateFilter(); }

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;

private:
    bool m_includeUnknowns;
    bool m_includeWarnings;
    bool m_includeErrors;
    QStringList m_categoryIds;
};

} // Internal
} // ProjectExplorer


using namespace ProjectExplorer;
using namespace ProjectExplorer::Internal;


////
//  TaskView
////

TaskView::TaskView(QWidget *parent)
    : QListView(parent)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

TaskView::~TaskView()
{

}

void TaskView::resizeEvent(QResizeEvent *e)
{
    Q_UNUSED(e)
    static_cast<TaskDelegate *>(itemDelegate())->emitSizeHintChanged(selectionModel()->currentIndex());
}

void TaskView::keyPressEvent(QKeyEvent *e)
{
    if (!e->modifiers() && e->key() == Qt::Key_Return) {
        emit activated(currentIndex());
        e->accept();
        return;
    }
    QListView::keyPressEvent(e);
}

/////
// TaskModel
/////

TaskModel::TaskModel() :
    m_maxSizeOfFileName(0),
    m_errorIcon(QLatin1String(":/projectexplorer/images/compile_error.png")),
    m_warningIcon(QLatin1String(":/projectexplorer/images/compile_warning.png")),
    m_taskCount(0),
    m_errorTaskCount(0),
    m_sizeOfLineNumber(0)
{

}

int TaskModel::taskCount()
{
    return m_taskCount;
}

int TaskModel::errorTaskCount()
{
    return m_errorTaskCount;
}

QIcon TaskModel::taskTypeIcon(Task::TaskType t) const
{
    switch (t) {
    case Task::Warning:
        return m_warningIcon;
    case Task::Error:
        return m_errorIcon;
    case Task::Unknown:
        break;
    }
    return QIcon();
}

void TaskModel::addCategory(const QString &categoryId, const QString &categoryName)
{
    Q_ASSERT(!categoryId.isEmpty());
    m_categories.insert(categoryId, categoryName);
}

QList<Task> TaskModel::tasks(const QString &categoryId) const
{
    if (categoryId.isEmpty()) {
        return m_tasks;
    } else {
        return m_tasksInCategory.value(categoryId);
    }
}

void TaskModel::addTask(const Task &task)
{
    Q_ASSERT(m_categories.keys().contains(task.category));

    if (m_tasksInCategory.contains(task.category)) {
        m_tasksInCategory[task.category].append(task);
    } else {
        QList<Task> temp;
        temp.append(task);
        m_tasksInCategory.insert(task.category, temp);
    }

    beginInsertRows(QModelIndex(), m_tasks.size(), m_tasks.size());
    m_tasks.append(task);
    endInsertRows();

    QFont font;
    QFontMetrics fm(font);
    QString filename = task.file;
    const int pos = filename.lastIndexOf(QLatin1Char('/'));
    if (pos != -1)
        filename = task.file.mid(pos +1);

    m_maxSizeOfFileName = qMax(m_maxSizeOfFileName, fm.width(filename));
    ++m_taskCount;
    if (task.type == Task::Error)
        ++m_errorTaskCount;
}

void TaskModel::removeTask(const Task &task)
{
    if (m_tasks.contains(task)) {
        int index = m_tasks.indexOf(task);
        beginRemoveRows(QModelIndex(), index, index);
        m_tasks.removeAt(index);
        --m_taskCount;
        if (task.type == Task::Error)
            --m_errorTaskCount;
        endRemoveRows();
    }
}

void TaskModel::clearTasks(const QString &categoryId)
{
    if (categoryId.isEmpty()) {
        if (m_tasks.size() == 0)
            return;
        beginRemoveRows(QModelIndex(), 0, m_tasks.size() -1);
        m_tasks.clear();
        m_tasksInCategory.clear();
        m_taskCount = 0;
        m_errorTaskCount = 0;
        endRemoveRows();
        m_maxSizeOfFileName = 0;
    } else {
        int index = 0;
        int start = 0;
        int subErrorTaskCount = 0;
        while (index < m_tasks.size()) {
            while (index < m_tasks.size() && m_tasks.at(index).category != categoryId) {
                ++start;
                ++index;
            }
            if (index == m_tasks.size())
                break;
            while (index < m_tasks.size() && m_tasks.at(index).category == categoryId) {
                if (m_tasks.at(index).type == Task::Error)
                    ++subErrorTaskCount;
                ++index;
            }
            // Index is now on the first non category
            beginRemoveRows(QModelIndex(), start, index - 1);

            for (int i = start; i < index; ++i) {
                m_tasksInCategory[categoryId].removeOne(m_tasks.at(i));
            }

            m_tasks.erase(m_tasks.begin() + start, m_tasks.begin() + index);

            m_taskCount -= index - start;
            m_errorTaskCount -= subErrorTaskCount;

            endRemoveRows();
            index = start;
        }
        // what to do with m_maxSizeOfFileName ?
    }
}


QModelIndex TaskModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid())
        return QModelIndex();
    return createIndex(row, column, 0);
}

QModelIndex TaskModel::parent(const QModelIndex &child) const
{
    Q_UNUSED(child)
    return QModelIndex();
}

int TaskModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_tasks.count();
}

int TaskModel::columnCount(const QModelIndex &parent) const
{
        return parent.isValid() ? 0 : 1;
}

QVariant TaskModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_tasks.size() || index.column() != 0)
        return QVariant();

    if (role == TaskModel::File) {
        return m_tasks.at(index.row()).file;
    } else if (role == TaskModel::Line) {
        if (m_tasks.at(index.row()).line <= 0)
            return QVariant();
        else
            return m_tasks.at(index.row()).line;
    } else if (role == TaskModel::Description) {
        return m_tasks.at(index.row()).description;
    } else if (role == TaskModel::FileNotFound) {
        return m_fileNotFound.value(m_tasks.at(index.row()).file);
    } else if (role == TaskModel::Type) {
        return (int)m_tasks.at(index.row()).type;
    } else if (role == TaskModel::Category) {
        return m_tasks.at(index.row()).category;
    } else if (role == TaskModel::Icon) {
        return taskTypeIcon(m_tasks.at(index.row()).type);
    } else if (role == TaskModel::Task_t) {
        return QVariant::fromValue(m_tasks.at(index.row()));
    }
    return QVariant();
}

QStringList TaskModel::categoryIds() const
{
    return m_categories.keys();
}

QString TaskModel::categoryDisplayName(const QString &categoryId) const
{
    return m_categories.value(categoryId);
}

int TaskModel::sizeOfFile()
{
    return m_maxSizeOfFileName;
}

int TaskModel::sizeOfLineNumber()
{
    if (m_sizeOfLineNumber == 0) {
        QFont font;
        QFontMetrics fm(font);
        m_sizeOfLineNumber = fm.width("8888");
    }
    return m_sizeOfLineNumber;
}

void TaskModel::setFileNotFound(const QModelIndex &idx, bool b)
{
    if (idx.isValid() && idx.row() < m_tasks.size()) {
        m_fileNotFound.insert(m_tasks[idx.row()].file, b);
        emit dataChanged(idx, idx);
    }
}

/////
// TaskFilterModel
/////

TaskFilterModel::TaskFilterModel(TaskModel *sourceModel, QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setSourceModel(sourceModel);
    setDynamicSortFilter(true);
    m_includeUnknowns = m_includeWarnings = m_includeErrors = true;
}

TaskModel *TaskFilterModel::taskModel() const
{
    return static_cast<TaskModel*>(sourceModel());
}

bool TaskFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    bool accept = true;

    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    Task::TaskType type = Task::TaskType(index.data(TaskModel::Type).toInt());
    switch (type) {
    case Task::Unknown:
        accept = m_includeUnknowns;
        break;
    case Task::Warning:
        accept = m_includeWarnings;
        break;
    case Task::Error:
        accept = m_includeErrors;
        break;
    }

    const QString &categoryId = index.data(TaskModel::Category).toString();
    if (m_categoryIds.contains(categoryId))
        accept = false;

    return accept;
}

/////
// TaskWindow
/////

static QToolButton *createFilterButton(QIcon icon, const QString &toolTip,
                                       QObject *receiver, const char *slot)
{
    QToolButton *button = new QToolButton;
    button->setIcon(icon);
    button->setToolTip(toolTip);
    button->setCheckable(true);
    button->setChecked(true);
    button->setAutoRaise(true);
    button->setEnabled(true);
    QObject::connect(button, SIGNAL(toggled(bool)), receiver, slot);
    return button;
}

TaskWindow::TaskWindow()
{
    Core::ICore *core = Core::ICore::instance();

    m_model = new TaskModel;
    m_filter = new TaskFilterModel(m_model);
    m_listview = new TaskView;

    m_listview->setModel(m_filter);
    m_listview->setFrameStyle(QFrame::NoFrame);
    m_listview->setWindowTitle(tr("Build Issues"));
    m_listview->setSelectionMode(QAbstractItemView::SingleSelection);
    TaskDelegate *tld = new TaskDelegate(this);
    m_listview->setItemDelegate(tld);
    m_listview->setWindowIcon(QIcon(":/qt4projectmanager/images/window.png"));
    m_listview->setContextMenuPolicy(Qt::ActionsContextMenu);
    m_listview->setAttribute(Qt::WA_MacShowFocusRect, false);

    m_taskWindowContext = new TaskWindowContext(m_listview);
    core->addContextObject(m_taskWindowContext);

    m_copyAction = new QAction(QIcon(Core::Constants::ICON_COPY), tr("&Copy"), this);
    Core::Command *command = core->actionManager()->
            registerAction(m_copyAction, Core::Constants::COPY, m_taskWindowContext->context());
    m_listview->addAction(command->action());
    connect(m_copyAction, SIGNAL(triggered()), SLOT(copy()));

    // Annotate using VCS: Make visible in all contexts
    m_vcsAnnotateAction = new QAction(tr("&Annotate"), this);
    m_vcsAnnotateAction->setToolTip("Annotate using version control system");
    QList<int> annotateContext = m_taskWindowContext->context();
    annotateContext << Core::ICore::instance()->uniqueIDManager()->uniqueIdentifier(QLatin1String(Core::Constants::C_GLOBAL));
    command = core->actionManager()->
            registerAction(m_vcsAnnotateAction, QLatin1String("ProjectExplorer.Task.VCS_Annotate"), annotateContext);
    m_listview->addAction(command->action());
    connect(m_vcsAnnotateAction, SIGNAL(triggered()), SLOT(vcsAnnotate()));

    connect(m_listview->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            tld, SLOT(currentChanged(QModelIndex,QModelIndex)));

    connect(m_listview, SIGNAL(activated(QModelIndex)),
            this, SLOT(showTaskInFile(QModelIndex)));
    connect(m_listview, SIGNAL(clicked(QModelIndex)),
            this, SLOT(showTaskInFile(QModelIndex)));

    m_filterWarningsButton = createFilterButton(taskTypeIcon(Task::Warning),
                                                tr("Show Warnings"),
                                                this, SLOT(setShowWarnings(bool)));

    m_categoriesMenu = new QMenu;
    connect(m_categoriesMenu, SIGNAL(aboutToShow()), this, SLOT(updateCategoriesMenu()));
    connect(m_categoriesMenu, SIGNAL(triggered(QAction*)), this, SLOT(filterCategoryTriggered(QAction*)));

    m_categoriesButton = new QToolButton;
    m_categoriesButton->setIcon(QIcon(":/projectexplorer/images/filtericon.png"));
    m_categoriesButton->setToolTip(tr("Filter by categories"));
    m_categoriesButton->setAutoRaise(true);
    m_categoriesButton->setPopupMode(QToolButton::InstantPopup);
    m_categoriesButton->setMenu(m_categoriesMenu);

    qRegisterMetaType<ProjectExplorer::Task>("ProjectExplorer::Task");
    qRegisterMetaType<QList<ProjectExplorer::Task> >("QList<ProjectExplorer::Task>");

    updateActions();
}

TaskWindow::~TaskWindow()
{
    Core::ICore::instance()->removeContextObject(m_taskWindowContext);
    delete m_filterWarningsButton;
    delete m_listview;
    delete m_filter;
    delete m_model;
}

QList<QWidget*> TaskWindow::toolBarWidgets() const
{
    return QList<QWidget*>() << m_filterWarningsButton << m_categoriesButton;
}

QWidget *TaskWindow::outputWidget(QWidget *)
{
    return m_listview;
}

void TaskWindow::clearTasks(const QString &categoryId)
{
    m_model->clearTasks(categoryId);

    updateActions();
    emit tasksChanged();
    emit tasksCleared();
    navigateStateChanged();
}

void TaskWindow::visibilityChanged(bool /* b */)
{
}

void TaskWindow::addCategory(const QString &categoryId, const QString &displayName)
{
    Q_ASSERT(!categoryId.isEmpty());
    m_model->addCategory(categoryId, displayName);
}

void TaskWindow::addTask(const Task &task)
{
    m_model->addTask(task);

    updateActions();
    emit tasksChanged();
    navigateStateChanged();
}

void TaskWindow::removeTask(const Task &task)
{
    m_model->removeTask(task);

    updateActions();
    emit tasksChanged();
    navigateStateChanged();
}

void TaskWindow::showTaskInFile(const QModelIndex &index)
{
    if (!index.isValid())
        return;
    QString file = index.data(TaskModel::File).toString();
    int line = index.data(TaskModel::Line).toInt();
    if (file.isEmpty() || line == -1)
        return;

    QFileInfo fi(file);
    if (fi.exists()) {
        TextEditor::BaseTextEditor::openEditorAt(fi.canonicalFilePath(), line);
        Core::EditorManager::instance()->ensureEditorManagerVisible();
    }
    else
        m_model->setFileNotFound(index, true);
    m_listview->selectionModel()->setCurrentIndex(index, QItemSelectionModel::Select);
    m_listview->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
}

// Right-click VCS annotate: Find version control and point it to line
void TaskWindow::vcsAnnotate()
{
    const QModelIndex index = m_listview->selectionModel()->currentIndex();
    if (!index.isValid())
        return;
    const QString file = index.data(TaskModel::File).toString();
    const int line = index.data(TaskModel::Line).toInt();
    const QFileInfo fi(file);
    if (fi.exists())
        if (Core::IVersionControl *vc = Core::ICore::instance()->vcsManager()->findVersionControlForDirectory(fi.absolutePath()))
            if (vc->supportsOperation(Core::IVersionControl::AnnotateOperation))
                vc->vcsAnnotate(fi.absoluteFilePath(), line);
}

void TaskWindow::copy()
{
    const QModelIndex index = m_listview->selectionModel()->currentIndex();
    if (!index.isValid())
        return;
    const QString file = index.data(TaskModel::File).toString();
    const QString line = index.data(TaskModel::Line).toString();
    const QString description = index.data(TaskModel::Description).toString();
    QString type;
    switch (index.data(TaskModel::Type).toInt()) {
    case Task::Error:
        type = "error: ";
        break;
    case Task::Warning:
        type = "warning: ";
        break;
    }

    QApplication::clipboard()->setText(file + ':' + line + ": " + type + description);
}

void TaskWindow::setShowWarnings(bool show)
{
    m_filter->setFilterIncludesWarnings(show);
    m_filter->setFilterIncludesUnknowns(show); // "Unknowns" are often associated with warnings
}

void TaskWindow::updateCategoriesMenu()
{
    m_categoriesMenu->clear();

    const QStringList filteredCategories = m_filter->filteredCategories();

    foreach (const QString &categoryId, m_model->categoryIds()) {
        const QString categoryName = m_model->categoryDisplayName(categoryId);

        QAction *action = new QAction(m_categoriesMenu);
        action->setCheckable(true);
        action->setText(categoryName);
        action->setData(categoryId);
        action->setChecked(!filteredCategories.contains(categoryId));

        m_categoriesMenu->addAction(action);
    }
}

void TaskWindow::filterCategoryTriggered(QAction *action)
{
    QString categoryId = action->data().toString();
    Q_ASSERT(!categoryId.isEmpty());

    QStringList categories = m_filter->filteredCategories();
    Q_ASSERT(m_filter->filteredCategories().contains(categoryId) == action->isChecked());

    if (action->isChecked()) {
        categories.removeOne(categoryId);
    } else {
        categories.append(categoryId);
    }

    m_filter->setFilteredCategories(categories);
}

int TaskWindow::taskCount() const
{
    return m_model->taskCount();
}

int TaskWindow::errorTaskCount() const
{
    return m_model->errorTaskCount();
}

int TaskWindow::priorityInStatusBar() const
{
    return 90;
}

void TaskWindow::clearContents()
{
    clearTasks();
}

bool TaskWindow::hasFocus()
{
    return m_listview->hasFocus();
}

bool TaskWindow::canFocus()
{
    return m_filter->rowCount();
}

void TaskWindow::setFocus()
{
    if (m_filter->rowCount()) {
        m_listview->setFocus();
        if (m_listview->currentIndex() == QModelIndex()) {
            m_listview->setCurrentIndex(m_filter->index(0,0, QModelIndex()));
        }
    }
}

bool TaskWindow::canNext()
{
    return m_filter->rowCount();
}

bool TaskWindow::canPrevious()
{
    return m_filter->rowCount();
}

void TaskWindow::goToNext()
{
    if (!m_filter->rowCount())
        return;
    QModelIndex currentIndex = m_listview->currentIndex();
    if (currentIndex.isValid()) {
        int row = currentIndex.row() + 1;
        if (row == m_filter->rowCount())
            row = 0;
        currentIndex = m_filter->index(row, 0);
    } else {
        currentIndex = m_filter->index(0, 0);
    }
    m_listview->setCurrentIndex(currentIndex);
    showTaskInFile(currentIndex);
}

void TaskWindow::goToPrev()
{
    if (!m_filter->rowCount())
        return;
    QModelIndex currentIndex = m_listview->currentIndex();
    if (currentIndex.isValid()) {
        int row = currentIndex.row() -1;
        if (row < 0)
            row = m_filter->rowCount() - 1;
        currentIndex = m_filter->index(row, 0);
    } else {
        currentIndex = m_filter->index(m_filter->rowCount()-1, 0);
    }
    m_listview->setCurrentIndex(currentIndex);
    showTaskInFile(currentIndex);
}

bool TaskWindow::canNavigate()
{
    return true;
}

void TaskWindow::updateActions()
{
    m_copyAction->setEnabled(m_model->tasks().count() > 0);
}

QIcon TaskWindow::taskTypeIcon(Task::TaskType t) const
{
    return m_model->taskTypeIcon(t);
}

/////
// Delegate
/////

TaskDelegate::TaskDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

TaskDelegate::~TaskDelegate()
{
}

QSize TaskDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItemV4 opt = option;
    initStyleOption(&opt, index);

    QFontMetrics fm(option.font);
    int fontHeight = fm.height();
    int fontLeading = fm.leading();

    QSize s;
    s.setWidth(option.rect.width());
    const QAbstractItemView * view = qobject_cast<const QAbstractItemView *>(opt.widget);
    TaskModel *model = static_cast<TaskFilterModel *>(view->model())->taskModel();
    int width = opt.rect.width() - model->sizeOfFile() - model->sizeOfLineNumber() - 12 - 22;
    if (view->selectionModel()->currentIndex() == index) {
        QString description = index.data(TaskModel::Description).toString();
        // Layout the description
        int leading = fontLeading;
        int height = 0;
        description.replace('\n', QChar::LineSeparator);
        QTextLayout tl(description);
        tl.beginLayout();
        while (true) {
            QTextLine line = tl.createLine();
            if (!line.isValid())
                break;
            line.setLineWidth(width);
            height += leading;
            line.setPosition(QPoint(0, height));
            height += static_cast<int>(line.height());
        }
        tl.endLayout();

        s.setHeight(height + leading + fontHeight + 3);
    } else {
        s.setHeight(fontHeight + 3);
    }
    if (s.height() < TASK_ICON_SIZE + 2 * TASK_ICON_MARGIN)
        s.setHeight(TASK_ICON_SIZE + 2 * TASK_ICON_MARGIN);
    return s;
}

void TaskDelegate::emitSizeHintChanged(const QModelIndex &index)
{
    emit sizeHintChanged(index);
}

void TaskDelegate::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    emit sizeHintChanged(current);
    emit sizeHintChanged(previous);
}

void TaskDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItemV4 opt = option;
    initStyleOption(&opt, index);
    painter->save();

    QFontMetrics fm(opt.font);
    QColor backgroundColor;
    QColor textColor;

    const QAbstractItemView * view = qobject_cast<const QAbstractItemView *>(opt.widget);
    bool selected = view->selectionModel()->currentIndex() == index;

    if (selected) {
        painter->setBrush(opt.palette.highlight().color());
        backgroundColor = opt.palette.highlight().color();
    } else {
        painter->setBrush(opt.palette.background().color());
        backgroundColor = opt.palette.background().color();
    }
    painter->setPen(Qt::NoPen);
    painter->drawRect(opt.rect);

    // Set Text Color
    if (selected)
        textColor = opt.palette.highlightedText().color();
    else
        textColor = opt.palette.text().color();

    painter->setPen(textColor);

    TaskModel *model = static_cast<TaskFilterModel *>(view->model())->taskModel();
    QIcon icon = index.data(TaskModel::Icon).value<QIcon>();
    painter->drawPixmap(TASK_ICON_MARGIN, opt.rect.top() + TASK_ICON_MARGIN, icon.pixmap(TASK_ICON_SIZE, TASK_ICON_SIZE));

    int width = opt.rect.width() - model->sizeOfFile() - model->sizeOfLineNumber() - 12 - 22;
    if (!selected) {
        // in small mode we lay out differently
        QString bottom = index.data(TaskModel::Description).toString().split('\n').first();
        painter->drawText(22, 2 + opt.rect.top() + fm.ascent(), bottom);
        if (fm.width(bottom) > width) {
            // draw a gradient to mask the text
            int gwidth = opt.rect.right() + 1 - width;
            QLinearGradient lg(QPoint(width, 0), QPoint(width+gwidth, 0));
            QColor c = backgroundColor;
            c.setAlpha(0);
            lg.setColorAt(0, c);
            lg.setColorAt(20.0/gwidth, backgroundColor);
            painter->fillRect(width, 2 + opt.rect.top(), gwidth, fm.height() + 1, lg);
        }
    } else {
        // Description
        QString description = index.data(TaskModel::Description).toString();
        // Layout the description
        int leading = fm.leading();
        int height = 0;
        description.replace('\n', QChar::LineSeparator);
        QTextLayout tl(description);
        tl.setAdditionalFormats(index.data(TaskModel::Task_t).value<ProjectExplorer::Task>().formats);
        tl.beginLayout();
        while (true) {
            QTextLine line = tl.createLine();
            if (!line.isValid())
                break;
            line.setLineWidth(width);
            height += leading;
            line.setPosition(QPoint(0, height));
            height += static_cast<int>(line.height());
        }
        tl.endLayout();
        tl.draw(painter, QPoint(22, 2 + opt.rect.top()));
        //painter->drawText(22, 2 + opt.rect.top() + fm.ascent(), description);

        QColor mix;
        mix.setRgb( static_cast<int>(0.7 * textColor.red()   + 0.3 * backgroundColor.red()),
                static_cast<int>(0.7 * textColor.green() + 0.3 * backgroundColor.green()),
                static_cast<int>(0.7 * textColor.blue()  + 0.3 * backgroundColor.blue()));
        painter->setPen(mix);

        const QString directory = QDir::toNativeSeparators(index.data(TaskModel::File).toString());
        int secondBaseLine = 2 + fm.ascent() + opt.rect.top() + height + leading; //opt.rect.top() + fm.ascent() + fm.height() + 6;
        if (index.data(TaskModel::FileNotFound).toBool()) {
            QString fileNotFound = tr("File not found: %1").arg(directory);
            painter->setPen(Qt::red);
            painter->drawText(22, secondBaseLine, fileNotFound);
        } else {
            painter->drawText(22, secondBaseLine, directory);
        }
    }

    painter->setPen(textColor);
    // Assemble string for the right side
    // just filename + linenumer
    QString file = index.data(TaskModel::File).toString();
    const int pos = file.lastIndexOf(QLatin1Char('/'));
    if (pos != -1)
        file = file.mid(pos +1);
    painter->drawText(width + 22 + 4, 2 + opt.rect.top() + fm.ascent(), file);

    QString topRight = index.data(TaskModel::Line).toString();
    painter->drawText(opt.rect.right() - fm.width(topRight) - 6 , 2 + opt.rect.top() + fm.ascent(), topRight);
    // Separator lines
    painter->setPen(QColor::fromRgb(150,150,150));
    painter->drawLine(0, opt.rect.bottom(), opt.rect.right(), opt.rect.bottom());
    painter->restore();
}

TaskWindowContext::TaskWindowContext(QWidget *widget)
    : Core::IContext(widget), m_taskList(widget)
{
    Core::UniqueIDManager *uidm = Core::UniqueIDManager::instance();
    m_context << uidm->uniqueIdentifier(Core::Constants::C_PROBLEM_PANE);
}

QList<int> TaskWindowContext::context() const
{
    return m_context;
}

QWidget *TaskWindowContext::widget()
{
    return m_taskList;
}


//
// functions
//
bool ProjectExplorer::operator==(const Task &t1, const Task &t2)
{
    return t1.type == t2.type
            && t1.line == t2.line
            && t1.description == t2.description
            && t1.file == t2.file
            && t1.category == t2.category;
}

uint ProjectExplorer::qHash(const Task &task)
{
    return qHash(task.file) + task.line;
}

#include "taskwindow.moc"
