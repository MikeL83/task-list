#ifndef TASKSDB_H
#define TASKSDB_H

#include <QObject>
#include <QtSql/QSqlDatabase>
#include <QVariant>
#include <QtSql/QSqlQuery>
#include <QList>

class QStandardItem;

constexpr quint32 MagicNumber() {return 0x21091983;}

using TaskList = QList<QStringList>;

class TasksDB : public QObject
{
    Q_OBJECT
    Q_ENUMS(Reminders)
    Q_ENUMS(Snoozed)
public:
    explicit TasksDB(QObject *parent = 0);

    enum Reminders {
        DUE1DAY,
        DUE2HRS,
        DUE1HR,
        DUE30MINS,
        DUE10MINS
    };

    enum Snoozed {
        S_5MINSBEFORESTART,
        S_10MINSBEFORESTART,
        S_5MINS,
        S_10MINS,
        S_15MINS,
        S_30MINS,
        S_1HOUR,
        S_2HOURS,
        S_4HOURS
    };



    bool addNewUser(const QString&, const QString&) const;
    TaskList getUserTasks(const QString&, const QString&) const;
    void addNewTask(const QString&, const QString&,
                    const QString&, const QString&,
                    const QString&, const QString&) const;
    void updateTask(const QString&, const QString&,
                    const QString&, const QString&,
                    const QString&, const QString&,
                    const QString&) const;
    void deleteTask(const QString&, const QString&) const;
    QStringList getTask(const QString&, const QString&) const;
    void importTask(const QString&, const QString&,
                    const QString&) const;
    TaskList loadFromFile(const QString&) const;
    void saveToFile(const QString&) const;
    TaskList getReminders(const QString&) const;
    void dismissReminder(const QString&, const QString&) const;
    void setSnoozeForTask(const QString&, const QString&,
                          const QString&, const QString&) const;
    TaskList checkSnoozedTasks(const QString&) const;
    TaskList checkOverDues(const QString&) const;
    TaskList checkPendingTasks(const QString&) const;
    //void snoozeTask(const QString&, const QString&, const QString&) const;

private:
    QSqlQuery prepare(const QString& statement) const;
    bool execute(QSqlQuery &query) const;
    void createConnection();
    void createInitialData() const;
    const QVariant Invalid;
    QSqlDatabase db;

};

#endif // TASKSDB_H
