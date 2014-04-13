#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>
#include <QModelIndex>
#include "userinputdialog.h"
#include "tasksdb.h"
#include "taskinputdialog.h"
#include "reminderdialog.h"

namespace Ui {
class MainWindow;
}

class QMenu;
class QAction;
class QTableView;
class QStandardItemModel;
class QContextMenuEvent;
class QTimer;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void contextMenuEvent(QContextMenuEvent*);

signals:

private slots:
    void importTask();
    void exportTask();
    void createUser();
    void openUser();
    void openUserTasks(const QString&, const QString&);
    void addUser(const QString&, const QString&);
    void addNewTask();
    void insertNewTask(const QString&, const QString&,
                       const QString&, const QString&);
    void editTask(const QModelIndex& index = QModelIndex());
    void editNewTask(const QString&, const QString&,
                     const QString&, const QString&);
    void deleteTask();
    //void snoozeTask();
    void sendTask();
    void sendTaskToOtherUser(const QString&, const QString&);
    void checkReminders();
    void dismissReminder(const QString&, const QString&);
    void snoozeReminder(const QString&, const QString&,
                         const QString&);

private:
    Ui::MainWindow *ui;

    void createWidgets();
    void initializeModel();
    void createActions();
    void createMenus();
    void createConnections();
    void clearModel();

    //QLineEdit *locationEdit;
    //QToolBar *toolBar;
    QMenu *fileMenu;
    QMenu *toolsMenu;

    QAction *exitAction;
    QAction *createUserAction;
    QAction *openUserAction;
    QAction *addNewTaskAction;
    QAction *editTaskAction;
    QAction *deleteTaskAction;
    QAction *sendTaskAction;
    QAction *importTaskAction;
    QAction *exportTaskAction;

    QTableView *view;
    QStandardItemModel *model;
    QString currentUser;
    std::unique_ptr<TaskInputDialog> taskDialog;
    std::unique_ptr<UserInputDialog> userDialog;
    std::unique_ptr<TasksDB> tasksDB;
    std::unique_ptr<ReminderDialog> reminderDialog;
    QModelIndex currentIndex;
    QTimer *timerForRem;
};

#endif // MAINWINDOW_H
