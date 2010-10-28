#include "targetsettingswidget.h"
#include "ui_targetsettingswidget.h"

static int WIDTH = 900;

using namespace ProjectExplorer::Internal;

TargetSettingsWidget::TargetSettingsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TargetSettingsWidget),
    m_targetSelector(new TargetSelector(this))
{
    ui->setupUi(this);
    ui->separator->setStyleSheet(QLatin1String("* { "
        "background-image: url(:/projectexplorer/images/targetseparatorbackground.png);"
        "}"));
    m_targetSelector->raise();
    connect(m_targetSelector, SIGNAL(addButtonClicked()),
            this, SIGNAL(addButtonClicked()));
    connect(m_targetSelector, SIGNAL(removeButtonClicked()),
            this, SIGNAL(removeButtonClicked()));
    connect(m_targetSelector, SIGNAL(currentChanged(int,int)),
            this, SIGNAL(currentChanged(int,int)));

    m_shadow = new QWidget(this);

    // Create shadow below targetselector
    m_targetSelector->raise();
    QPalette shadowPal = palette();
    QLinearGradient grad(0, 0, 0, 2);
    grad.setColorAt(0, QColor(0, 0, 0, 60));
    grad.setColorAt(1, Qt::transparent);
    shadowPal.setBrush(QPalette::All, QPalette::Window, grad);
    m_shadow->setPalette(shadowPal);
    m_shadow->setAutoFillBackground(true);

    updateTargetSelector();
}

TargetSettingsWidget::~TargetSettingsWidget()
{
    delete ui;
}

void TargetSettingsWidget::addTarget(const QString &name)
{
    m_targetSelector->addTarget(name);
    updateTargetSelector();
}

void TargetSettingsWidget::insertTarget(int index, const QString &name)
{
    m_targetSelector->insertTarget(index, name);
    updateTargetSelector();
}

void TargetSettingsWidget::removeTarget(int index)
{
    m_targetSelector->removeTarget(index);
    updateTargetSelector();
}

void TargetSettingsWidget::setCurrentIndex(int index)
{
    m_targetSelector->setCurrentIndex(index);
}

void TargetSettingsWidget::setCurrentSubIndex(int index)
{
    m_targetSelector->setCurrentSubIndex(index);
}

void TargetSettingsWidget::setAddButtonEnabled(bool enabled)
{
    m_targetSelector->setAddButtonEnabled(enabled);
}

void TargetSettingsWidget::setRemoveButtonEnabled(bool enabled)
{
    m_targetSelector->setRemoveButtonEnabled(enabled);
}

QString TargetSettingsWidget::targetNameAt(int index) const
{
    return m_targetSelector->targetAt(index).name;
}

void TargetSettingsWidget::setCentralWidget(QWidget *widget)
{
    ui->scrollArea->setWidget(widget);
}

int TargetSettingsWidget::targetCount() const
{
    return m_targetSelector->targetCount();
}

int TargetSettingsWidget::currentIndex() const
{
    return m_targetSelector->currentIndex();
}

int TargetSettingsWidget::currentSubIndex() const
{
    return m_targetSelector->currentSubIndex();
}

bool TargetSettingsWidget::isAddButtonEnabled() const
{
    return m_targetSelector->isAddButtonEnabled();
}

bool TargetSettingsWidget::isRemoveButtonEnabled() const
{
    return m_targetSelector->isRemoveButtonEnabled();
}

void TargetSettingsWidget::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    m_shadow->setGeometry(0, m_targetSelector->height() + 3, width(), 2);
}

void TargetSettingsWidget::updateTargetSelector()
{
    m_targetSelector->setGeometry((WIDTH-m_targetSelector->minimumSizeHint().width())/2, 13,
        m_targetSelector->minimumSizeHint().width(),
        m_targetSelector->minimumSizeHint().height());
}

void TargetSettingsWidget::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
