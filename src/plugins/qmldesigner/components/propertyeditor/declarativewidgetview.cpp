#include "declarativewidgetview.h"

#include <qdeclarative.h>
#include <QDeclarativeItem>
#include <QDeclarativeEngine>
#include <QDeclarativeContext>
#include <QPointer>

namespace QmlDesigner {

class DeclarativeWidgetViewPrivate
{
public:
    DeclarativeWidgetViewPrivate(DeclarativeWidgetView *view)
        : q(view), root(0), component(0) {}
    ~DeclarativeWidgetViewPrivate() { delete root; }
    void execute();

    DeclarativeWidgetView *q;

    QPointer<QWidget> root;
    QUrl source;
    QDeclarativeEngine engine;
    QDeclarativeComponent *component;
};

void DeclarativeWidgetViewPrivate::execute()
{
    if (root) {
        delete root;
        root = 0;
    }
    if (component) {
        delete component;
        component = 0;
    }
    if (!source.isEmpty()) {
        component = new QDeclarativeComponent(&engine, source, q);
        if (!component->isLoading()) {
            q->continueExecute();
        } else {
            QObject::connect(component, SIGNAL(statusChanged(QDeclarativeComponent::Status)), q, SLOT(continueExecute()));
        }
    }
}

DeclarativeWidgetView::DeclarativeWidgetView(QWidget *parent) :
    QWidget(parent), d(new DeclarativeWidgetViewPrivate(this))
{
}

DeclarativeWidgetView::~DeclarativeWidgetView()
{
    delete d;
}

QUrl DeclarativeWidgetView::source() const
{
    return d->source;
}

void DeclarativeWidgetView::setSource(const QUrl& url)
{
    d->source = url;
    d->execute();
}

QDeclarativeEngine* DeclarativeWidgetView::engine()
{
   return &d->engine;
}

QWidget *DeclarativeWidgetView::rootWidget() const
{
    return d->root;
}

QDeclarativeContext* DeclarativeWidgetView::rootContext()
{
   return d->engine.rootContext();
}

DeclarativeWidgetView::Status DeclarativeWidgetView::status() const
{
    if (!d->component)
        return DeclarativeWidgetView::Null;

    return DeclarativeWidgetView::Status(d->component->status());
}


void DeclarativeWidgetView::continueExecute()
{

    disconnect(d->component, SIGNAL(statusChanged(QDeclarativeComponent::Status)), this, SLOT(continueExecute()));

    if (d->component->isError()) {
        QList<QDeclarativeError> errorList = d->component->errors();
        foreach (const QDeclarativeError &error, errorList) {
            qWarning() << error;
        }
        emit statusChanged(status());
        return;
    }

    QObject *obj = d->component->create();

    if(d->component->isError()) {
        QList<QDeclarativeError> errorList = d->component->errors();
        foreach (const QDeclarativeError &error, errorList) {
            qWarning() << error;
        }
        emit statusChanged(status());
        return;
    }

    setRootWidget(qobject_cast<QWidget *>(obj));
    emit statusChanged(status());
}

void DeclarativeWidgetView::setRootWidget(QWidget *widget)
{
    if (d->root == widget)
        return;

    window()->setAttribute(Qt::WA_OpaquePaintEvent, false);
    window()->setAttribute(Qt::WA_NoSystemBackground, false);
    widget->setParent(this);
    if (isVisible()) {
        widget->setVisible(true);
    }
    resize(widget->size());
    d->root = widget;

    if (d->root) {
        QSize initialSize = d->root->size();
        if (initialSize != size()) {
            resize(initialSize);
        }
    }
}

} //QmlDesigner
