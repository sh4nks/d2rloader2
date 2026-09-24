#pragma once

#include <KAboutData>
#include <QObject>
#include <qqmlintegration.h>

class About : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(KAboutData aboutData READ aboutData CONSTANT)
    Q_PROPERTY(QString qtVersion READ qtVersion CONSTANT)

public:
    using QObject::QObject;

    KAboutData aboutData() const
    {
        return KAboutData::applicationData();
    }

    QString qtVersion() const
    {
        return QString::fromLatin1(qVersion());
    }
};
