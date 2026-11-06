#pragma once

#include <QFileInfo>
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

    /**
     * The folder a file dialog should open in to show \a path: the path
     * itself when it is a directory, otherwise the directory holding it.
     * Empty when nothing of \a path exists, so the dialog keeps its default.
     *
     * The trailing slash matters: FolderDialog.selectedFolder goes to the
     * parent of a URL that looks like a file name.
     */
    Q_INVOKABLE QUrl folderUrl(const QString &path) const
    {
        const QFileInfo info(path);
        // protonPath holds a runtime name such as "UMU-Latest", which must
        // not be resolved against the working directory.
        if (path.isEmpty() || info.isRelative()) {
            return {};
        }

        if (info.isDir()) {
            return directoryUrl(info.absoluteFilePath());
        }

        const QFileInfo parent(info.absolutePath());
        if (parent.isDir()) {
            return directoryUrl(parent.absoluteFilePath());
        }

        return {};
    }

private:
    static QUrl directoryUrl(const QString &directory)
    {
        QUrl url = QUrl::fromLocalFile(directory);
        if (!url.path().endsWith(QLatin1Char('/'))) {
            url.setPath(url.path() + QLatin1Char('/'));
        }
        return url;
    }
};
