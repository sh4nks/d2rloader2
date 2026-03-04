#include "d2rloader.h"
#include "settingsmanager.h"
#include <QApplication>
#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QtQml>
#include <print>
#include <qlogging.h>
#include <qstylefactory.h>

#include "book.h"
#include "booklistmodel.h"
#include "booktablemodel.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("D2RLoader"));

    D2RLoader *d2rloader = D2RLoader::getInstance();

    std::println("D2RLoader started {}", d2rloader->appVersion().toStdString());

    // Init app components
    SettingsManager *sm = SettingsManager::getInstance();
    if (!sm)
    {
        qWarning() << "Cannot init app components!";
        return EXIT_FAILURE;
    }

    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
        QQuickStyle::setStyle(QStringLiteral("org.kde.desktop"));
    }
    QApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("kde")));

    qmlRegisterUncreatableType<BookListModel>("BookListModel", 1, 0, "BookRoles", QStringLiteral("Cannot create instances of BookListModel"));

    qmlRegisterUncreatableType<BookTableModel>("BookTableModel", 1, 0, "BookRoles", QStringLiteral("Cannot create instances of BookTableModel"));

    QList<Book *> bookList;
    bookList.append(new Book(QStringLiteral("Harry Potter and the Philosopher's Stone"), QStringLiteral("J.K. Rowling"), 1997, 4.5));
    bookList.append(new Book(QStringLiteral("Fantastic Beasts and Where to Find Them"), QStringLiteral("J.K. Rowling"), 2001, 4.3));
    bookList.append(new Book(QStringLiteral("The Dark Tower"), QStringLiteral("Stephen King"), 1982, 4.0));
    bookList.append(new Book(QStringLiteral("American Gods"), QStringLiteral("Neil Gaiman"), 2001, 4.1));
    bookList.append(new Book(QStringLiteral("The Hobbit"), QStringLiteral("J.R.R. Tolkien"), 1937, 4.4));
    bookList.append(new Book(QStringLiteral("1984"), QStringLiteral("George Orwell"), 1949, 4.3));
    bookList.append(new Book(QStringLiteral("To Kill a Mockingbird"), QStringLiteral("Harper Lee"), 1960, 4.5));
    bookList.append(new Book(QStringLiteral("The Great Gatsby"), QStringLiteral("F. Scott Fitzgerald"), 1925, 3.9));
    bookList.append(new Book(QStringLiteral("Moby Dick"), QStringLiteral("Herman Melville"), 1851, 3.6));
    bookList.append(new Book(QStringLiteral("War and Peace"), QStringLiteral("Leo Tolstoy"), 1867, 4.3));
    bookList.append(new Book(QStringLiteral("Pride and Prejudice"), QStringLiteral("Jane Austen"), 1813, 4.1));
    bookList.append(new Book(QStringLiteral("The Catcher in the Rye"), QStringLiteral("J.D. Salinger"), 1951, 3.9));
    bookList.append(new Book(QStringLiteral("Ulysses"), QStringLiteral("James Joyce"), 1922, 3.7));
    bookList.append(new Book(QStringLiteral("One Hundred Years of Solitude"), QStringLiteral("Gabriel Garcia Marquez"), 1967, 4.4));

    BookListModel *bookListModel = new BookListModel(bookList, &app);
    BookTableModel *bookTableModel = new BookTableModel(bookList, &app);

    QSortFilterProxyModel *listProxy = new QSortFilterProxyModel(&app);
    listProxy->setSourceModel(bookListModel);
    listProxy->setSortRole(BookListModel::YearRole);
    listProxy->sort(0, Qt::AscendingOrder);

    QSortFilterProxyModel *tableProxy = new QSortFilterProxyModel(&app);
    tableProxy->setSourceModel(bookTableModel);
    tableProxy->setSortRole(Qt::DisplayRole);
    tableProxy->sort(BookTableModel::YearRole, Qt::AscendingOrder);

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("bookListModel"), listProxy);
    engine.rootContext()->setContextProperty(QStringLiteral("bookTableModel"), tableProxy);
    engine.rootContext()->setContextProperty(QStringLiteral("settingsManager"), sm);
    engine.rootContext()->setContextProperty(QStringLiteral("app"), d2rloader);
    qDebug() << engine.importPathList();

    // engine.load(QUrl(QStringLiteral("qrc:/qml/Main.qml")));
    engine.loadFromModule("org.someblocks.d2rloader", "Main");
    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}
