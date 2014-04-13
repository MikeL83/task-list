#include "tasksdb.h"
#include <QDebug>
#include <QMessageBox>
#include <QStandardPaths>
#include <QtSql/QSqlError>
#include <QDir>
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QApplication>
#include <QMetaEnum>
#include <QMessageBox>
#include <QtCore/qmath.h>

TasksDB::TasksDB(QObject *parent) :
    QObject(parent)
{
    createConnection();
}

QSqlQuery TasksDB::prepare(const QString &statement) const
{
    QSqlQuery query(db);
    query.setForwardOnly(true);
    if (!query.prepare(statement)) {
        qWarning() << Q_FUNC_INFO << "failed to prepare query";
        qWarning() << query.lastQuery();
        qWarning() << query.lastError().text();
        return QSqlQuery();
    }
    return query;
}

bool TasksDB::execute(QSqlQuery &query) const
{
    if (!query.exec()) {
        qWarning() << Q_FUNC_INFO << "failed execute query";
        qWarning() << query.lastQuery();
        qWarning() << query.lastError().text();
        return false;
    }
    return true;
}

void TasksDB::createConnection() {
    db = QSqlDatabase::addDatabase("QSQLITE");
    QString databaseDir =
        QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    const QString dbFileName = QString("_tasklist.db");
    QDir dir(databaseDir);
    if (!QDir().mkpath(databaseDir)) {
        qWarning("Cannot create directory %s",
                 qPrintable(QStandardPaths::writableLocation(
                     QStandardPaths::DataLocation)));
        return;
    }
    db.setDatabaseName(dir.absoluteFilePath(dbFileName));
    if (!db.open())
        qFatal("Error while opening the database: %s",
               qPrintable(db.lastError().text()));

    createInitialData();
}

void TasksDB::createInitialData() const {
    QSqlQuery query =
        prepare(QString("CREATE TABLE IF NOT EXISTS "
                        "Users (id INTEGER PRIMARY KEY AUTOINCREMENT, "
                        "name TEXT NOT NULL, "
                        "username TEXT NOT NULL);"));
    execute(query);
}

bool TasksDB::addNewUser(const QString &name, const QString &username) const {
    QSqlQuery query = prepare(QString("SELECT username FROM Users;"));
    if (execute(query)) {
        while (query.next()) {
            if (query.value(0) != Invalid && query.value(0).toString().compare(username) == 0) {
                QMessageBox::warning(0, tr("Task List"),
                                        tr("There already exists user %1.\n"
                                           "Please choose another username").arg(username),
                                           QMessageBox::Ok | QMessageBox::Cancel,
                                           QMessageBox::Ok);
                return false;
            }
        }
    }
    query = prepare(QString("INSERT INTO Users "
                            "(name, username) "
                            "VALUES (:name, :username);"));
            query.bindValue(":name", name);
            query.bindValue(":username", username);
    if (!execute(query))
        return false;

    query = prepare(QString("CREATE TABLE IF NOT EXISTS %1"
                            "(id INTEGER PRIMARY KEY AUTOINCREMENT, "
                            "name TEXT NOT NULL, "
                            "desc TEXT NOT NULL, "
                            "deadline TEXT NOT NULL, "
                            "reminder TEXT NOT NULL, "
                            "created TEXT NOT NULL, "
                            "snoozed TEXT NOT NULL, "
                            "snoozetime TEXT NOT NULL);").arg(username));
    if (!execute(query))
        return false;

    return true;
}

void TasksDB::addNewTask(const QString &username, const QString &taskName,
                         const QString &taskDesc, const QString &taskDeadline,
                         const QString &taskReminder, const QString &taskCreated) const {
    QSqlQuery query = prepare(QString("INSERT INTO %1 "
                              "(name, desc, deadline, reminder, created, snoozed, snoozetime) "
                              "VALUES (:name, :desc, :deadline, "
                              ":reminder, :created, :snoozed, :snoozetime);").arg(username));
    query.bindValue(":name", taskName);
    query.bindValue(":desc", taskDesc);
    query.bindValue(":deadline", taskDeadline);
    query.bindValue(":reminder", taskReminder);
    query.bindValue(":created", taskCreated);
    query.bindValue(":snoozed", "");
    query.bindValue(":snoozetime", "");
    execute(query);
}

TaskList TasksDB::getUserTasks(const QString &name, const QString &username) const {
    QSqlQuery query = prepare(QString("SELECT name, username FROM Users "
                                      "WHERE name = ? AND username = ?;"));
    query.bindValue(0, name);
    query.bindValue(1, username);
    TaskList tasks;
    if (!execute(query)) {
        tasks.append(QStringList() << "invalid");
        return tasks;
    }
    try  {

        query.next();

    } catch(...) {
        tasks.append(QStringList() << "invalid");
        QMessageBox::warning(0, tr("Task List"),
                                tr("The database does not contain user (%1, %2).\n"
                                   "Please choose an existing user.").arg(name).arg(username),
                                   QMessageBox::Ok | QMessageBox::Cancel,
                                   QMessageBox::Ok);
        return tasks;
    }

    if (query.value(0) != Invalid && query.value(1) != Invalid) {
        QSqlQuery queryForTasks = prepare(QString("SELECT name, desc, deadline, "
                                                  "reminder, created FROM %1;").
                                          arg(query.value(1).toString()));
        if (!execute(queryForTasks)) {
            tasks.append(QStringList() << "invalid");
            return tasks;
        }
        while (queryForTasks.next()) {
            if (queryForTasks.value(0) != Invalid && queryForTasks.value(1) != Invalid
                && queryForTasks.value(2) != Invalid && queryForTasks.value(3) != Invalid &&
                queryForTasks.value(4) != Invalid) {
                tasks.append(QStringList() << queryForTasks.value(0).toString()
                                           << queryForTasks.value(1).toString()
                                           << queryForTasks.value(2).toString()
                                           << queryForTasks.value(3).toString()
                                           << queryForTasks.value(4).toString());
            }
        }
        return tasks;
    } else {
        tasks.append(QStringList() << "invalid");
        QMessageBox::warning(0, tr("Task List"),
                                tr("The database does not contain user (%1, %2).\n"
                                   "Please choose an existing user.").arg(name).arg(username),
                                   QMessageBox::Ok | QMessageBox::Cancel,
                                   QMessageBox::Ok);
        return tasks;
    }
}

QStringList TasksDB::getTask(const QString &username, const QString &created) const {
    QSqlQuery query = prepare(QString("SELECT name, desc, deadline, reminder FROM %1 "
                                      "WHERE created = ?;").arg(username));
    query.bindValue(0, created);
    QStringList task;
    if (!execute(query)) {
        return task;
    } else {
        query.next();
        if (query.value(0) != Invalid && query.value(1) != Invalid &&
                query.value(2) != Invalid && query.value(3) != Invalid) {
            task << query.value(0).toString() << query.value(1).toString()
                 << query.value(2).toString() << query.value(3).toString();
        }
        return task;
    }
}

void TasksDB::updateTask(const QString &username, const QString& old_created,
                         const QString &taskname, const QString &taskdesc,
                         const QString &taskdeadline, const QString &reminder,
                         const QString &new_created) const {
    QSqlQuery query = prepare(QString("UPDATE %1 SET name = ?, desc = ?, deadline = ?, "
                                      "reminder = ?, created = ? WHERE created = ?;").arg(username));
    query.bindValue(0, taskname);
    query.bindValue(1, taskdesc);
    query.bindValue(2, taskdeadline);
    query.bindValue(3, reminder);
    query.bindValue(4, new_created);
    query.bindValue(5, old_created);
    execute(query);
}

void TasksDB::deleteTask(const QString &username, const QString& created) const {
    QSqlQuery query;
    query = prepare(QString("DELETE FROM %1 "
                    "WHERE created = ?;").arg(username));
    query.bindValue(0, created);
    execute(query);
}

void TasksDB::importTask(const QString &fromUsername,const QString &toUsername,
                         const QString &created) const {
    QSqlQuery query = prepare(QString("SELECT name, desc, deadline, reminder, created, snoozed, snoozetime FROM %1 "
                                      "WHERE created = ?;").arg(fromUsername));
    query.bindValue(0, created);
    if (!execute(query)) {
        return;
    } else {
        query.next();
        if (query.value(0) != Invalid && query.value(1) != Invalid &&
                query.value(2) != Invalid && query.value(3) != Invalid &&
                query.value(4) != Invalid && query.value(5) != Invalid) {
            QSqlQuery queryToUser = prepare(QString("INSERT INTO %1 "
                                            "(name, desc, deadline, reminder, created, snoozed, snoozetime) "
                                            "VALUES (:name, :desc, :deadline, :reminder, "
                                                    ":created, :snoozed, :snoozetime);").arg(toUsername));
            queryToUser.bindValue(":name", query.value(0).toString());
            queryToUser.bindValue(":desc", query.value(1).toString());
            queryToUser.bindValue(":deadline", query.value(2).toString());
            queryToUser.bindValue(":reminder", query.value(3).toString());
            queryToUser.bindValue(":created", query.value(4).toString());
            queryToUser.bindValue(":snoozed", query.value(5).toString());
            queryToUser.bindValue(":snoozed", query.value(6).toString());
            execute(queryToUser);
        }
    }
}

void TasksDB::saveToFile(const QString& username) const {
    QString fileName = QFileDialog::getSaveFileName(
        0, tr("%1 - Save User Tasks").arg(QApplication::applicationName()),
                "/home", tr("Text files (*.txt)"));
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
            qFatal("Cannot open file %s for writing: %s", qPrintable(fileName),
                   qPrintable(file.errorString()));
            return;
        }
        QSqlQuery query = prepare(QString("SELECT name, desc, deadline, "
                                          "reminder, created FROM %1;").
                                          arg(username));
        if (!execute(query)) {
            file.close();
            return;
        }
        QTextStream out(&file);
        out.setCodec("UTF-8");
        out << MagicNumber() << "\n";
        while (query.next()) {
            if (query.value(0) != Invalid && query.value(1) != Invalid
                && query.value(2) != Invalid && query.value(3) != Invalid &&
                query.value(4) != Invalid) {
                if (QDateTime::currentDateTime() > QDateTime::fromString(
                            query.value(2).toString(),"d.M.yyyy hh.mm").addDays(-1)) {
                    continue;
                } else {
                    out << query.value(0).toString() << "\n";
                    out << query.value(1).toString() << "\n";
                    out << query.value(2).toString() << "\n";
                    out << query.value(3).toString() << "\n";
                    out << "\n";
                }
            }
        }
        file.close();
    }
}

TaskList TasksDB::loadFromFile(const QString& username) const {
    TaskList tasks;
    QString fileName = QFileDialog::getOpenFileName(
        0, tr("Open Tasks"), "/home", tr("Text files (*.txt)"));
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (!file.open(QFile::ReadOnly)) {
            qFatal("Cannot open file %s for reading: %s", qPrintable(fileName),
                   qPrintable(file.errorString()));
        }
        QTextStream in(&file);
        in.setCodec("UTF-8");
        quint32 magic;
        magic = in.readLine().toUInt();
        int lineno = 1;
        if (magic != MagicNumber()) {
            QMessageBox::warning(0,"Task List - File error",
                                   tr("File %1 is not recognized by this application.\n"
                                      "Line number %1.")
                                   .arg(fileName).arg(lineno));
            return tasks;
        }
        QStringList list;
        QString input;
        int k = 0;
        while (!in.atEnd()) {
            input = in.readLine();
            lineno += 1;
            if (input.isEmpty()) {
                QMessageBox::warning(0,"Task List - Input error (unnamed task)",
                                       tr("Consider naming a task.\n"
                                          "Notice that program assumes that tasks are separated\n"
                                          "from each other with line.\n"
                                          "Line number %1 in file %2.")
                                       .arg(lineno).arg(fileName));
            }
            list.append(input);
            list.append(in.readLine());
            lineno += 1;
            input = in.readLine();
            lineno += 1;
            if (input.isEmpty()) {
                QMessageBox::warning(0,"Task List - Input error (no deadline specified)",
                                       tr("You need to provide deadline for the task.\n"
                                          "Remember to use correct datetime format d.M.yyyy hh.mm.\n"
                                          "Line number %1 in file %2.")
                                          .arg(lineno).arg(fileName));
                tasks.clear();
                return tasks;
            }
            QDateTime date = QDateTime::fromString(input,"d.M.yyyy hh.mm");
            if (!date.isValid()) {
                QMessageBox::warning(0,"Task List - Input error (deadline datetime format)",
                                       tr("Datetime format is not correct for the task.\n"
                                          "Use the correct datetime format d.M.yyyy hh.mm.\n"
                                          "Notice that program assumes that tasks are separated\n"
                                          "from each other with (empty)line.\n"
                                          "Line number %2 in file %3.")
                                          .arg(lineno).arg(fileName));
                tasks.clear();
                return tasks;
            }
            list.append(input);
            input = in.readLine();
            lineno += 1;
            QStringList reminders {"1 day", "2 hrs","no reminder"
                                   "1 hr", "30 mins", "10 mins"};
            if (!reminders.contains(input)) {
                QMessageBox::warning(0,"Task List - Input error (wrong reminder)",
                                       tr("Given reminder is not valid.\n"
                                          "Correct reminders are:\n"
                                          "1 day, 2 hrs, 1 hr, 30 mins, 10 mins and no reminder.\n"
                                          "Line number %1 in file %2.")
                                          .arg(lineno).arg(fileName));
                tasks.clear();
                return tasks;
            }
            list.append(input);
            input = in.readLine();
            lineno += 1;
            list.append(QDateTime::currentDateTime().addMSecs(k).toString("d MMMM yyyy hh:mm:ss.z"));
            tasks.append(list);
            list.clear();
            k++;
        }
        qDebug() << tasks;
        if (!tasks.isEmpty()) {
            for (const auto& item : tasks) {
                QSqlQuery query = prepare(QString("INSERT INTO %1 "
                                          "(name, desc, deadline, reminder, created, snoozed, snoozetime) "
                                          "VALUES (:name, :desc, :deadline, :reminder, "
                                                  ":created, :snoozed, :snoozetime);")
                                          .arg(username));
                query.bindValue(":name", item.at(0));
                query.bindValue(":desc", item.at(1));
                query.bindValue(":deadline", item.at(2));
                query.bindValue(":reminder", item.at(3));
                query.bindValue(":created", item.at(4));
                query.bindValue(":snoozed", "");
                query.bindValue(":snoozetime", "");
                execute(query);
            }
        }
        return tasks;
    } else {
        return tasks;
    }
}

TaskList TasksDB::getReminders(const QString& username) const {
    TaskList dueTasks;
    if (username.isEmpty())
        return dueTasks;
    QDateTime currentTime = QDateTime::currentDateTime();
    QSqlQuery query = prepare(QString("SELECT name, desc, deadline, "
                                      "reminder, created FROM %1;").
                                      arg(username));
    if (!execute(query)) {
        return dueTasks;
    }
    QMetaObject metaObj = this->staticMetaObject;
    QMetaEnum metaEnum = metaObj.enumerator(metaObj.indexOfEnumerator("Reminders"));
    while (query.next()) {
        if (query.value(0) != Invalid && query.value(1) != Invalid
            && query.value(2) != Invalid && query.value(3) != Invalid &&
            query.value(4) != Invalid) {

            if (query.value(3).toString().compare("no reminder") == 0) {
                continue;
            } else if ( QDateTime::fromString(query.value(2).toString(),"d.M.yyyy hh.mm")
                                          < currentTime ||
                        QDateTime::fromString(query.value(2).toString(),"d.M.yyyy hh.mm").addDays(-1)
                                        > currentTime ) {
                //dismissReminder(username, query.value(4).toString());
                continue;
            } else {
                switch (metaEnum.keysToValue(
                            "DUE" + query.value(3).toString().
                            replace(QRegExp(" "), "").toUpper().toLatin1())) {
                case DUE1DAY:
                    if (QDateTime::fromString(query.value(2).toString(),"d.M.yyyy hh.mm")
                            .addDays(-1).toString("d.M.yyyy hh.mm")
                            .compare(currentTime.toString("d.M.yyyy hh.mm")) == 0) {
                        dueTasks.append(QStringList() << query.value(0).toString()
                                                      << query.value(2).toString()
                                                      << "1 day"
                                                      << query.value(4).toString());
                    }
                    break;
                case DUE2HRS:
                    if (QDateTime::fromString(query.value(2).toString(),"d.M.yyyy hh.mm")
                            .addSecs(-60 * 60 * 2).toString("d.M.yyyy hh.mm")
                            .compare(currentTime.toString("d.M.yyyy hh.mm")) == 0) {
                        dueTasks.append(QStringList() << query.value(0).toString()
                                                      << query.value(2).toString()
                                                      << "2 hours"
                                                      << query.value(4).toString());
                    }
                    break;
                case DUE1HR:
                    if (QDateTime::fromString(query.value(2).toString(),"d.M.yyyy hh.mm")
                            .addSecs(-60 * 60).toString("d.M.yyyy hh.mm")
                            .compare(currentTime.toString("d.M.yyyy hh.mm")) == 0) {
                        dueTasks.append(QStringList() << query.value(0).toString()
                                                      << query.value(2).toString()
                                                      << "1 hour"
                                                      << query.value(4).toString());
                    }
                    break;
                case DUE30MINS:
                    if (QDateTime::fromString(query.value(2).toString(),"d.M.yyyy hh.mm")
                            .addSecs(-60 * 30).toString("d.M.yyyy hh.mm")
                            .compare(currentTime.toString("d.M.yyyy hh.mm")) == 0) {
                        dueTasks.append(QStringList() << query.value(0).toString()
                                                      << query.value(2).toString()
                                                      << "30 mins"
                                                      << query.value(4).toString());
                    }
                    break;
                case DUE10MINS:
                    if (QDateTime::fromString(query.value(2).toString(),"d.M.yyyy hh.mm")
                            .addSecs(-60 * 10).toString("d.M.yyyy hh.mm")
                            .compare(currentTime.toString("d.M.yyyy hh.mm")) == 0) {
                        dueTasks.append(QStringList() << query.value(0).toString()
                                                      << query.value(2).toString()
                                                      << "10 mins"
                                                      << query.value(4).toString());
                    }
                    break;
                  default:
                    break;
                }
            }
        }
    }
    return dueTasks;
}

void TasksDB::dismissReminder(const QString &username, const QString &created) const {
    QSqlQuery query = prepare(QString("UPDATE %1 SET reminder = ?, snoozed = ? WHERE created = ?;").arg(username));
    query.bindValue(0, "no reminder");
    query.bindValue(1, "");
    query.bindValue(2, created);
    execute(query);
}

void TasksDB::setSnoozeForTask(const QString &username, const QString &created,
                               const QString& text, const QString& time) const
{
    QSqlQuery query = prepare(QString("UPDATE %1 SET snoozed = ?, snoozetime = ? WHERE created = ?;").arg(username));
    query.bindValue(0, text);
    query.bindValue(1, time);
    query.bindValue(2, created);
    execute(query);
}

TaskList TasksDB::checkSnoozedTasks(const QString &username) const
{
    TaskList snoozedTasks;
    if (username.isEmpty())
        return snoozedTasks;
    QDateTime currentTime = QDateTime::currentDateTime();
    QSqlQuery query = prepare(QString("SELECT name, desc, deadline, "
                                      "reminder, created, snoozed, snoozetime FROM %1;").
                                      arg(username));
    if (!execute(query)) {
        return snoozedTasks;
    }
    QMetaObject metaObj = this->staticMetaObject;
    QMetaEnum metaEnum = metaObj.enumerator(metaObj.indexOfEnumerator("Snoozed"));
    while (query.next()) {
        if (query.value(0) != Invalid && query.value(1) != Invalid
            && query.value(2) != Invalid && query.value(3) != Invalid &&
            query.value(4) != Invalid && query.value(5) != Invalid) {

            if (query.value(5).toString().compare("") == 0) {
                continue;
            } else {
                switch (metaEnum.keysToValue(
                            "S_" + query.value(5).toString().
                            replace(QRegExp(" "), "").toUpper().toLatin1())) {
                    case S_5MINSBEFORESTART:
                        if (QDateTime::fromString(query.value(2).toString(),"d.M.yyyy hh.mm")
                                .addSecs(-5 * 60).toString("d.M.yyyy hh.mm")
                                .compare(currentTime.toString("d.M.yyyy hh.mm")) == 0) {
                            snoozedTasks.append(QStringList() << query.value(0).toString()
                                                              << query.value(2).toString()
                                                              << "5 mins"
                                                              << query.value(4).toString()
                                                              << query.value(5).toString());
                        }
                        break;
                    case S_10MINSBEFORESTART:
                        if (QDateTime::fromString(query.value(2).toString(),"d.M.yyyy hh.mm")
                                .addSecs(-10 * 60).toString("d.M.yyyy hh.mm")
                                .compare(currentTime.toString("d.M.yyyy hh.mm")) == 0) {
                            snoozedTasks.append(QStringList() << query.value(0).toString()
                                                              << query.value(2).toString()
                                                              << "10 mins"
                                                              << query.value(4).toString()
                                                              << query.value(5).toString());
                        }
                        break;
                    case S_5MINS:
                        if (QDateTime::fromString(query.value(6).toString(),"d.M.yyyy hh.mm")
                                .addSecs(5 * 60).toString("d.M.yyyy hh.mm")
                                .compare(currentTime.toString("d.M.yyyy hh.mm")) == 0) {
                            QDateTime deadline = QDateTime::fromString(query.value(2).toString(),"d.M.yyyy hh.mm");
                            qint64 secs = deadline.secsTo(currentTime);
                            int hours = qFloor(secs / 3600);
                            int mins = qFloor(secs / 60) - qFloor(secs / 3600) * 60;
                            snoozedTasks.append(QStringList() << query.value(0).toString()
                                                              << query.value(2).toString()
                                                              << QString("%1 hours %2 mins").arg(hours).arg(mins)
                                                              << query.value(4).toString()
                                                              << query.value(5).toString());
                        }
                        break;
                    case S_10MINS:
                        if (QDateTime::fromString(query.value(6).toString(),"d.M.yyyy hh.mm")
                                .addSecs(10 * 60).toString("d.M.yyyy hh.mm")
                                .compare(currentTime.toString("d.M.yyyy hh.mm")) == 0) {
                            QDateTime deadline = QDateTime::fromString(query.value(2).toString(),"d.M.yyyy hh.mm");
                            qint64 secs = deadline.secsTo(currentTime);
                            int hours = qFloor(secs / 3600);
                            int mins = qFloor(secs / 60) - qFloor(secs / 3600) * 60;
                            snoozedTasks.append(QStringList() << query.value(0).toString()
                                                              << query.value(2).toString()
                                                              << QString("%1 hours %2 mins").arg(hours).arg(mins)
                                                              << query.value(4).toString()
                                                              << query.value(5).toString());
                        }
                        break;
                    case S_15MINS:
                        if (QDateTime::fromString(query.value(6).toString(),"d.M.yyyy hh.mm")
                                .addSecs(15 * 60).toString("d.M.yyyy hh.mm")
                                .compare(currentTime.toString("d.M.yyyy hh.mm")) == 0) {
                            QDateTime deadline = QDateTime::fromString(query.value(2).toString(),"d.M.yyyy hh.mm");
                            qint64 secs = deadline.secsTo(currentTime);
                            int hours = qFloor(secs / 3600);
                            int mins = qFloor(secs / 60) - qFloor(secs / 3600) * 60;
                            snoozedTasks.append(QStringList() << query.value(0).toString()
                                                              << query.value(2).toString()
                                                              << QString("%1 hours %2 mins").arg(hours).arg(mins)
                                                              << query.value(4).toString()
                                                              << query.value(5).toString());
                        }
                        break;
                    case S_30MINS:
                        if (QDateTime::fromString(query.value(6).toString(),"d.M.yyyy hh.mm")
                                .addSecs(30 * 60).toString("d.M.yyyy hh.mm")
                                .compare(currentTime.toString("d.M.yyyy hh.mm")) == 0) {
                            QDateTime deadline = QDateTime::fromString(query.value(2).toString(),"d.M.yyyy hh.mm");
                            qint64 secs = deadline.secsTo(currentTime);
                            int hours = qFloor(secs / 3600);
                            int mins = qFloor(secs / 60) - qFloor(secs / 3600) * 60;
                            snoozedTasks.append(QStringList() << query.value(0).toString()
                                                              << query.value(2).toString()
                                                              << QString("%1 hours %2 mins").arg(hours).arg(mins)
                                                              << query.value(4).toString()
                                                              << query.value(5).toString());
                        }
                        break;
                    case S_1HOUR:
                        if (QDateTime::fromString(query.value(6).toString(),"d.M.yyyy hh.mm")
                                .addSecs(3600).toString("d.M.yyyy hh.mm")
                                .compare(currentTime.toString("d.M.yyyy hh.mm")) == 0) {
                            QDateTime deadline = QDateTime::fromString(query.value(2).toString(),"d.M.yyyy hh.mm");
                            qint64 secs = deadline.secsTo(currentTime);
                            int hours = qFloor(secs / 3600);
                            int mins = qFloor(secs / 60) - qFloor(secs / 3600) * 60;
                            snoozedTasks.append(QStringList() << query.value(0).toString()
                                                              << query.value(2).toString()
                                                              << QString("%1 hours %2 mins").arg(hours).arg(mins)
                                                              << query.value(4).toString()
                                                              << query.value(5).toString());
                        }
                        break;
                    case S_2HOURS:
                        if (QDateTime::fromString(query.value(6).toString(),"d.M.yyyy hh.mm")
                                .addSecs(3600 * 2).toString("d.M.yyyy hh.mm")
                                .compare(currentTime.toString("d.M.yyyy hh.mm")) == 0) {
                            QDateTime deadline = QDateTime::fromString(query.value(2).toString(),"d.M.yyyy hh.mm");
                            qint64 secs = deadline.secsTo(currentTime);
                            int hours = qFloor(secs / 3600);
                            int mins = qFloor(secs / 60) - qFloor(secs / 3600) * 60;
                            snoozedTasks.append(QStringList() << query.value(0).toString()
                                                              << query.value(2).toString()
                                                              << QString("%1 hours %2 mins").arg(hours).arg(mins)
                                                              << query.value(4).toString()
                                                              << query.value(5).toString());
                        }
                    break;
                    case S_4HOURS:
                        if (QDateTime::fromString(query.value(6).toString(),"d.M.yyyy hh.mm")
                                .addSecs(3600 * 4).toString("d.M.yyyy hh.mm")
                                .compare(currentTime.toString("d.M.yyyy hh.mm")) == 0) {
                            QDateTime deadline = QDateTime::fromString(query.value(2).toString(),"d.M.yyyy hh.mm");
                            qint64 secs = deadline.secsTo(currentTime);
                            int hours = qFloor(secs / 3600);
                            int mins = qFloor(secs / 60) - qFloor(secs / 3600) * 60;
                            snoozedTasks.append(QStringList() << query.value(0).toString()
                                                              << query.value(2).toString()
                                                              << QString("%1 hours %2 mins").arg(hours).arg(mins)
                                                              << query.value(4).toString()
                                                              << query.value(5).toString());
                        }
                        break;
                    default:
                        break;
                }
            }
        }
    }
    return snoozedTasks;

}

TaskList TasksDB::checkOverDues(const QString &username) const
{
    TaskList overDueTasks;
    if (username.isEmpty())
        return overDueTasks;
    QDateTime currentTime = QDateTime::currentDateTime();
    QSqlQuery query = prepare(QString("SELECT name, desc, deadline, "
                                      "reminder, created, snoozed FROM %1;").
                                      arg(username));
    if (!execute(query)) {
        return overDueTasks;
    }
    while (query.next()) {
        if (query.value(0) != Invalid && query.value(1) != Invalid
            && query.value(2) != Invalid && query.value(3) != Invalid &&
            query.value(4) != Invalid && query.value(5) != Invalid) {
            if (QDateTime::fromString(query.value(2).toString(),"d.M.yyyy hh.mm") <
                currentTime && (query.value(3).toString().compare("no reminder") != 0 ||
                    !query.value(5).toString().isEmpty())) {
                QDateTime deadline = QDateTime::fromString(query.value(2).toString(),"d.M.yyyy hh.mm");
                qint64 secs = deadline.secsTo(currentTime);
                int hours = qFloor(secs / 3600);
                int mins = qFloor(secs / 60) - qFloor(secs / 3600) * 60;
                overDueTasks.append(QStringList() << query.value(0).toString()
                                                  << query.value(2).toString()
                                                  << QString("Overdue: %1 hours %2 mins").arg(hours).arg(mins)
                                                  << query.value(4).toString()
                                                  << query.value(5).toString());
                dismissReminder(username, query.value(4).toString());
            }
        }
    }
    return overDueTasks;

}

TaskList TasksDB::checkPendingTasks(const QString& username) const
{
    TaskList pendingTasks;
    if (username.isEmpty())
        return pendingTasks;
    QDateTime currentTime = QDateTime::currentDateTime();
    QSqlQuery query = prepare(QString("SELECT name, desc, deadline, "
                                      "reminder, created, snoozed, snoozetime FROM %1;").
                                      arg(username));
    if (!execute(query)) {
        return pendingTasks;
    }
    QMetaObject metaObj = this->staticMetaObject;
    QMetaEnum metaEnum = metaObj.enumerator(metaObj.indexOfEnumerator("Reminders"));
    while (query.next()) {
        if (query.value(0) != Invalid && query.value(1) != Invalid
            && query.value(2) != Invalid && query.value(3) != Invalid &&
            query.value(4) != Invalid && query.value(5) != Invalid) {
            if (query.value(3).toString().compare("no reminder") != 0) {
                switch (metaEnum.keysToValue(
                            "DUE" + query.value(3).toString().
                            replace(QRegExp(" "), "").toUpper().toLatin1())) {
                case DUE1DAY:
                    if (QDateTime::fromString(query.value(2).toString(),"d.M.yyyy hh.mm") >
                                        currentTime &&
                    QDateTime::fromString(query.value(2).toString(),"d.M.yyyy hh.mm")
                            .addSecs(-86340) < currentTime) {
                        QDateTime deadline = QDateTime::fromString(query.value(2).toString(),"d.M.yyyy hh.mm");
                        qint64 secs = currentTime.secsTo(deadline);
                        int hours = qFloor(secs / 3600);
                        int mins = qFloor(secs / 60) - qFloor(secs / 3600) * 60;
                        pendingTasks.append(QStringList() << query.value(0).toString()
                                                          << query.value(2).toString()
                                                          << QString("%1 hours %2 mins").arg(hours).arg(mins)
                                                          << query.value(4).toString()
                                                          << query.value(5).toString());
                    }
                    break;
                case DUE2HRS:
                    if (QDateTime::fromString(query.value(2).toString(),"d.M.yyyy hh.mm") >
                                        currentTime &&
                    QDateTime::fromString(query.value(2).toString(),"d.M.yyyy hh.mm")
                            .addSecs(-2 * 3570) < currentTime) {
                        QDateTime deadline = QDateTime::fromString(query.value(2).toString(),"d.M.yyyy hh.mm");
                        qint64 secs = currentTime.secsTo(deadline);
                        int hours = qFloor(secs / 3600);
                        int mins = qFloor(secs / 60) - qFloor(secs / 3600) * 60;
                        pendingTasks.append(QStringList() << query.value(0).toString()
                                                          << query.value(2).toString()
                                                          << QString("%1 hours %2 mins").arg(hours).arg(mins)
                                                          << query.value(4).toString()
                                                          << query.value(5).toString());
                    }
                    break;
                case DUE1HR:
                    if (QDateTime::fromString(query.value(2).toString(),"d.M.yyyy hh.mm") >
                                        currentTime &&
                    QDateTime::fromString(query.value(2).toString(),"d.M.yyyy hh.mm")
                            .addSecs(-3540) < currentTime) {
                        QDateTime deadline = QDateTime::fromString(query.value(2).toString(),"d.M.yyyy hh.mm");
                        qint64 secs = currentTime.secsTo(deadline);
                        int hours = qFloor(secs / 3600);
                        int mins = qFloor(secs / 60) - qFloor(secs / 3600) * 60;
                        pendingTasks.append(QStringList() << query.value(0).toString()
                                                          << query.value(2).toString()
                                                          << QString("%1 hours %2 mins").arg(hours).arg(mins)
                                                          << query.value(4).toString()
                                                          << query.value(5).toString());
                    }
                    break;
                case DUE30MINS:
                    if (QDateTime::fromString(query.value(2).toString(),"d.M.yyyy hh.mm") >
                                        currentTime &&
                    QDateTime::fromString(query.value(2).toString(),"d.M.yyyy hh.mm")
                            .addSecs(-29 * 60) < currentTime) {
                        QDateTime deadline = QDateTime::fromString(query.value(2).toString(),"d.M.yyyy hh.mm");
                        qint64 secs = currentTime.secsTo(deadline);
                        int hours = qFloor(secs / 3600);
                        int mins = qFloor(secs / 60) - qFloor(secs / 3600) * 60;
                        pendingTasks.append(QStringList() << query.value(0).toString()
                                                          << query.value(2).toString()
                                                          << QString("%1 hours %2 mins").arg(hours).arg(mins)
                                                          << query.value(4).toString()
                                                          << query.value(5).toString());
                    }
                    break;
                case DUE10MINS:
                    if (QDateTime::fromString(query.value(2).toString(),"d.M.yyyy hh.mm") >
                                        currentTime &&
                    QDateTime::fromString(query.value(2).toString(),"d.M.yyyy hh.mm")
                            .addSecs(-9 * 60) < currentTime) {
                        QDateTime deadline = QDateTime::fromString(query.value(2).toString(),"d.M.yyyy hh.mm");
                        qint64 secs = currentTime.secsTo(deadline);
                        int hours = qFloor(secs / 3600);
                        int mins = qFloor(secs / 60) - qFloor(secs / 3600) * 60;
                        pendingTasks.append(QStringList() << query.value(0).toString()
                                                          << query.value(2).toString()
                                                          << QString("%1 hours %2 mins").arg(hours).arg(mins)
                                                          << query.value(4).toString()
                                                          << query.value(5).toString());
                    }
                    break;
                  default:
                    break;
                }
            } else if (query.value(3).toString().compare("no reminder") == 0 &&
                       !query.value(5).toString().isEmpty()) {
                switch (metaEnum.keysToValue(
                            "S_" + query.value(5).toString().
                            replace(QRegExp(" "), "").toUpper().toLatin1())) {
                case S_5MINSBEFORESTART:
                    if (QDateTime::fromString(query.value(2).toString(),"d.M.yyyy hh.mm") >
                                        currentTime &&
                    QDateTime::fromString(query.value(2).toString(),"d.M.yyyy hh.mm")
                            .addSecs(-4 * 60) < currentTime) {
                        QDateTime deadline = QDateTime::fromString(query.value(2).toString(),"d.M.yyyy hh.mm");
                        qint64 secs = currentTime.secsTo(deadline);
                        int hours = qFloor(secs / 3600);
                        int mins = qFloor(secs / 60) - qFloor(secs / 3600) * 60;
                        pendingTasks.append(QStringList() << query.value(0).toString()
                                                          << query.value(2).toString()
                                                          << QString("%1 hours %2 mins").arg(hours).arg(mins)
                                                          << query.value(4).toString()
                                                          << query.value(5).toString());
                    }
                    break;
                case S_10MINSBEFORESTART:
                    if (QDateTime::fromString(query.value(2).toString(),"d.M.yyyy hh.mm") >
                                        currentTime &&
                    QDateTime::fromString(query.value(2).toString(),"d.M.yyyy hh.mm")
                            .addSecs(-9 * 60) < currentTime) {
                        QDateTime deadline = QDateTime::fromString(query.value(2).toString(),"d.M.yyyy hh.mm");
                        qint64 secs = currentTime.secsTo(deadline);
                        int hours = qFloor(secs / 3600);
                        int mins = qFloor(secs / 60) - qFloor(secs / 3600) * 60;
                        pendingTasks.append(QStringList() << query.value(0).toString()
                                                          << query.value(2).toString()
                                                          << QString("%1 hours %2 mins").arg(hours).arg(mins)
                                                          << query.value(4).toString()
                                                          << query.value(5).toString());
                    }
                    break;

                case S_5MINS:
                    if (QDateTime::fromString(query.value(2).toString(),"d.M.yyyy hh.mm") >
                                        currentTime &&
                    QDateTime::fromString(query.value(6).toString(),"d.M.yyyy hh.mm")
                            .addSecs(6 * 60) < currentTime) {
                        QDateTime deadline = QDateTime::fromString(query.value(6).toString(),"d.M.yyyy hh.mm");
                        qint64 secs = currentTime.secsTo(deadline);
                        int hours = qFloor(secs / 3600);
                        int mins = qFloor(secs / 60) - qFloor(secs / 3600) * 60;
                        pendingTasks.append(QStringList() << query.value(0).toString()
                                                          << query.value(2).toString()
                                                          << QString("%1 hours %2 mins").arg(hours).arg(mins)
                                                          << query.value(4).toString()
                                                          << query.value(5).toString());
                    }
                    break;
                case S_10MINS:
                    if (QDateTime::fromString(query.value(2).toString(),"d.M.yyyy hh.mm") >
                                        currentTime &&
                    QDateTime::fromString(query.value(6).toString(),"d.M.yyyy hh.mm")
                            .addSecs(11 * 60) < currentTime) {
                        QDateTime deadline = QDateTime::fromString(query.value(2).toString(),"d.M.yyyy hh.mm");
                        qint64 secs = currentTime.secsTo(deadline);
                        int hours = qFloor(secs / 3600);
                        int mins = qFloor(secs / 60) - qFloor(secs / 3600) * 60;
                        pendingTasks.append(QStringList() << query.value(0).toString()
                                                          << query.value(2).toString()
                                                          << QString("%1 hours %2 mins").arg(hours).arg(mins)
                                                          << query.value(4).toString()
                                                          << query.value(5).toString());
                    }
                    break;
                case S_15MINS:
                    if (QDateTime::fromString(query.value(2).toString(),"d.M.yyyy hh.mm") >
                                        currentTime &&
                    QDateTime::fromString(query.value(6).toString(),"d.M.yyyy hh.mm")
                            .addSecs(16 * 60) < currentTime) {
                        QDateTime deadline = QDateTime::fromString(query.value(2).toString(),"d.M.yyyy hh.mm");
                        qint64 secs = currentTime.secsTo(deadline);
                        int hours = qFloor(secs / 3600);
                        int mins = qFloor(secs / 60) - qFloor(secs / 3600) * 60;
                        pendingTasks.append(QStringList() << query.value(0).toString()
                                                          << query.value(2).toString()
                                                          << QString("%1 hours %2 mins").arg(hours).arg(mins)
                                                          << query.value(4).toString()
                                                          << query.value(5).toString());
                    }
                    break;
                case S_30MINS:
                    if (QDateTime::fromString(query.value(2).toString(),"d.M.yyyy hh.mm") >
                                        currentTime &&
                    QDateTime::fromString(query.value(6).toString(),"d.M.yyyy hh.mm")
                            .addSecs(31 * 60) < currentTime) {
                        QDateTime deadline = QDateTime::fromString(query.value(2).toString(),"d.M.yyyy hh.mm");
                        qint64 secs = currentTime.secsTo(deadline);
                        int hours = qFloor(secs / 3600);
                        int mins = qFloor(secs / 60) - qFloor(secs / 3600) * 60;
                        pendingTasks.append(QStringList() << query.value(0).toString()
                                                          << query.value(2).toString()
                                                          << QString("%1 hours %2 mins").arg(hours).arg(mins)
                                                          << query.value(4).toString()
                                                          << query.value(5).toString());
                    }
                    break;
                case S_1HOUR:
                    if (QDateTime::fromString(query.value(2).toString(),"d.M.yyyy hh.mm") >
                                        currentTime &&
                    QDateTime::fromString(query.value(6).toString(),"d.M.yyyy hh.mm")
                            .addSecs(3660) < currentTime) {
                        QDateTime deadline = QDateTime::fromString(query.value(2).toString(),"d.M.yyyy hh.mm");
                        qint64 secs = currentTime.secsTo(deadline);
                        int hours = qFloor(secs / 3600);
                        int mins = qFloor(secs / 60) - qFloor(secs / 3600) * 60;
                        pendingTasks.append(QStringList() << query.value(0).toString()
                                                          << query.value(2).toString()
                                                          << QString("%1 hours %2 mins").arg(hours).arg(mins)
                                                          << query.value(4).toString()
                                                          << query.value(5).toString());
                    }
                    break;
                case S_2HOURS:
                    if (QDateTime::fromString(query.value(2).toString(),"d.M.yyyy hh.mm") >
                                        currentTime &&
                    QDateTime::fromString(query.value(6).toString(),"d.M.yyyy hh.mm")
                            .addSecs(2 * 3600 + 60) < currentTime) {
                        QDateTime deadline = QDateTime::fromString(query.value(2).toString(),"d.M.yyyy hh.mm");
                        qint64 secs = currentTime.secsTo(deadline);
                        int hours = qFloor(secs / 3600);
                        int mins = qFloor(secs / 60) - qFloor(secs / 3600) * 60;
                        pendingTasks.append(QStringList() << query.value(0).toString()
                                                          << query.value(2).toString()
                                                          << QString("%1 hours %2 mins").arg(hours).arg(mins)
                                                          << query.value(4).toString()
                                                          << query.value(5).toString());
                    }
                    break;
                case S_4HOURS:
                    if (QDateTime::fromString(query.value(2).toString(),"d.M.yyyy hh.mm") >
                                        currentTime &&
                    QDateTime::fromString(query.value(6).toString(),"d.M.yyyy hh.mm")
                            .addSecs(4 * 3600 + 60) < currentTime) {
                        QDateTime deadline = QDateTime::fromString(query.value(2).toString(),"d.M.yyyy hh.mm");
                        qint64 secs = currentTime.secsTo(deadline);
                        int hours = qFloor(secs / 3600);
                        int mins = qFloor(secs / 60) - qFloor(secs / 3600) * 60;
                        pendingTasks.append(QStringList() << query.value(0).toString()
                                                          << query.value(2).toString()
                                                          << QString("%1 hours %2 mins").arg(hours).arg(mins)
                                                          << query.value(4).toString()
                                                          << query.value(5).toString());
                    }
                    break;
                }
            }
        }
    }
    return pendingTasks;

}




