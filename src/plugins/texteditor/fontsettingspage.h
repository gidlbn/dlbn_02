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

#ifndef FONTSETTINGSPAGE_H
#define FONTSETTINGSPAGE_H

#include "texteditor_global.h"

#include "fontsettings.h"

#include "texteditoroptionspage.h"

#include <QtCore/QList>
#include <QtCore/QString>

QT_BEGIN_NAMESPACE
class QWidget;
class QColor;
QT_END_NAMESPACE

namespace TextEditor {

namespace Internal {
class FontSettingsPagePrivate;
} // namespace Internal

// GUI description of a format consisting of id (settings key)
// and displayName to be displayed
class TEXTEDITOR_EXPORT FormatDescription
{
public:
    FormatDescription(const QString &id, const QString &displayName,
                      const QColor &foreground = Qt::black);

    QString id() const
    { return m_id; }

    QString displayName() const
    { return m_displayName; }

    QColor foreground() const;
    QColor background() const;

    const Format &format() const { return m_format; }
    Format &format() { return m_format; }

private:
    QString m_id;               // Name of the category
    QString m_displayName;      // Displayed name of the category
    Format m_format;            // Default format
};

typedef QList<FormatDescription> FormatDescriptions;

class TEXTEDITOR_EXPORT FontSettingsPage : public TextEditorOptionsPage
{
    Q_OBJECT

public:
    FontSettingsPage(const FormatDescriptions &fd,
                     const QString &id,
                     QObject *parent = 0);

    ~FontSettingsPage();

    QString id() const;
    QString displayName() const;

    QWidget *createPage(QWidget *parent);
    void apply();
    void finish();
    virtual bool matches(const QString &) const;

    void saveSettings();

    const FontSettings &fontSettings() const;

signals:
    void changed(const TextEditor::FontSettings&);

private slots:
    void delayedChange();
    void fontFamilySelected(const QString &family);
    void fontSizeSelected(const QString &sizeString);
    void fontZoomChanged();
    void colorSchemeSelected(int index);
    void copyColorScheme();
    void copyColorScheme(const QString &name);
    void confirmDeleteColorScheme();
    void deleteColorScheme();

private:
    void maybeSaveColorScheme();
    void updatePointSizes();
    QList<int> pointSizesForSelectedFont() const;
    void refreshColorSchemeList();

    Internal::FontSettingsPagePrivate *d_ptr;
};

} // namespace TextEditor

#endif // FONTSETTINGSPAGE_H
