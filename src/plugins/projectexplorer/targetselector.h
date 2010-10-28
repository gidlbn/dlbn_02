#ifndef TARGETSELECTOR_H
#define TARGETSELECTOR_H

#include <QtGui/QWidget>
#include <QtGui/QPixmap>

namespace ProjectExplorer {
namespace Internal {

class TargetSelector : public QWidget
{
Q_OBJECT
public:
    struct Target {
        QString name;
        int currentSubIndex;
    };

    explicit TargetSelector(QWidget *parent = 0);

    QSize minimumSizeHint() const;

    int targetWidth() const;
    QString runButtonString() const { return tr("Run"); }
    QString buildButtonString() const { return tr("Build"); }

    Target targetAt(int index) const;
    int targetCount() const { return m_targets.size(); }
    int currentIndex() const { return m_currentTargetIndex; }
    int currentSubIndex() const { return m_targets.at(m_currentTargetIndex).currentSubIndex; }

    bool isAddButtonEnabled() const;
    bool isRemoveButtonEnabled() const;

public slots:
    void addTarget(const QString &name);
    void insertTarget(int index, const QString &name);
    void removeTarget(int index);
    void setCurrentIndex(int index);
    void setCurrentSubIndex(int subindex);
    void setAddButtonEnabled(bool enabled);
    void setRemoveButtonEnabled(bool enabled);

signals:
    void addButtonClicked();
    void removeButtonClicked();
    // This signal is emited whenever the target pointed to by the indices
    // has changed.
    void currentChanged(int targetIndex, int subIndex);

protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);

private:
    const QImage m_unselected;
    const QImage m_runselected;
    const QImage m_buildselected;
    const QPixmap m_targetaddbutton;
    const QPixmap m_targetaddbuttondisabled;
    const QPixmap m_targetremovebutton;
    const QPixmap m_targetremovebuttondisabled;

    QList<Target> m_targets;

    int m_currentTargetIndex;
    bool m_addButtonEnabled;
    bool m_removeButtonEnabled;
};

} // namespace Internal
} // namespace ProjectExplorer

#endif // TARGETSELECTOR_H
