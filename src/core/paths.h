#pragma once

#include <QObject>
#include <QUrl>
#include <qqmlintegration.h>

/**
 * Turns the file URLs of the QML file dialogs into local paths. QML cannot
 * do that itself, and cutting the scheme off leaves "/C:/..." on Windows.
 */
class Paths : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    using QObject::QObject;

    Q_INVOKABLE QString toLocalFile(const QUrl &url) const
    {
        return url.toLocalFile();
    }
};
