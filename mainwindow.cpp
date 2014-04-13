#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTableView>
#include <QMenu>
#include <QAction>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QContextMenuEvent>
#include <QHeaderView>
#include <QSizePolicy>
#include <QDebug>
#include <QList>
#include <QStringList>
#include <QDateTime>
#include <QFontMetrics>
#include <QFont>
#include <QTimer>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), currentUser("")
{
    ui->setupUi(this);

    tasksDB = std::unique_ptr<TasksDB>{ new TasksDB };
    timerForRem = new QTimer(this);
    timerForRem->setInterval(1000 * 60);
    initializeModel();
    createWidgets();
    createActions();
    createMenus();
    createConnections();

    QApplication::setApplicationName(tr("Task List"));

    setWindowTitle(tr("Task List - [*]"));

    setCentralWidget(view);

    resize(840, 560);
    show();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initializeModel() {
    model = new QStandardItemModel();
    model->setHorizontalHeaderLabels((QStringList() << "Task name"
                                                    << "Task description"
                                                    << "Deadline"));
    //model->setColumnCount(3);
}

void MainWindow::clearModel() {
    model->clear();
    model->setHorizontalHeaderLabels((QStringList() << "Task name"
                                                    << "Task description"
                                                    << "Deadline"));
    model->setColumnCount(3);
    QFont font("Verdana", 16);
    QFontMetrics fm(font);
    view->setColumnWidth(0, fm.width("Task name") + 50);
    view->setColumnWidth(1, fm.width("Task description") + 50);
}

void MainWindow::createWidgets() {
    view = new QTableView(this);
    view->setModel(model);
    view->horizontalHeader()->setStretchLastSection(true);
    view->verticalHeader()->hide();
    QFont font("Verdana", 16);
    QFontMetrics fm(font);
    view->setColumnWidth(0, fm.width("Task name") + 50);
    view->setColumnWidth(1, fm.width("Task description") + 50);
    view->setStyleSheet("QHeaderView::section {background: lightblue;"
                        "font-family: Verdana;"
                        "font-size: 16px;"
                        "font-weight: bold}");
    view->horizontalHeader()->setStretchLastSection(true);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    view->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    view->horizontalHeader()->setSortIndicatorShown(true);
    view->setSelectionBehavior(QTableView::SelectRows);
    view->setSortingEnabled(true);
    //view->resizeColumnsToContents();
    //view->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    //view->horizontalHeader()->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum);
    view->setAttribute(Qt::WA_DeleteOnClose);
}

void MainWindow::createActions() {
    createUserAction = new QAction(tr("&New User"),this);
    createUserAction->setShortcut(tr("Ctrl+N"));
    createUserAction->setStatusTip(tr("New User"));

    openUserAction = new QAction(tr("&Open"),this);
    openUserAction->setShortcut(tr("Ctrl+O"));
    openUserAction->setStatusTip(tr("Open User Tasks"));

    addNewTaskAction = new QAction(tr("&Add New Task"),this);
    addNewTaskAction->setShortcut(tr("Ctrl+T"));
    addNewTaskAction->setStatusTip(tr("Add New Task"));

    addNewTaskAction->setEnabled(false);

    editTaskAction = new QAction(tr("&Edit"),this);
    editTaskAction->setShortcut(tr("Ctrl+E"));
    editTaskAction->setStatusTip(tr("Edit task"));

    editTaskAction->setEnabled(false);

    deleteTaskAction = new QAction(tr("&Delete Task"),this);
    deleteTaskAction->setShortcut(tr("Ctrl+D"));
    deleteTaskAction->setStatusTip(tr("Delete Tasks"));

    deleteTaskAction->setEnabled(false);

    sendTaskAction = new QAction(tr("&Send"),this);
    sendTaskAction->setShortcut(tr("Ctrl+S"));
    sendTaskAction->setStatusTip(tr("Send Task"));

    sendTaskAction->setEnabled(false);

    exitAction = new QAction(tr("E&xit"), this);
    exitAction->setShortcut(tr("Ctrl+Q"));
    exitAction->setStatusTip(tr("Exit"));

    importTaskAction = new QAction(tr("&Import..."),this);
    importTaskAction->setShortcut(tr("Ctrl+I"));
    importTaskAction->setStatusTip(tr("Import"));

    importTaskAction->setEnabled(false);

    exportTaskAction = new QAction(tr("&Export..."),this);
    exportTaskAction->setShortcut(tr("Ctrl+E"));
    exportTaskAction->setStatusTip(tr("Export"));

    exportTaskAction->setEnabled(false);
}

void MainWindow::createMenus() {
    fileMenu = menuBar()->addMenu(tr("&File"));
    toolsMenu = menuBar()->addMenu(tr("&Tools"));
    toolsMenu->addAction(addNewTaskAction);
    toolsMenu->addAction(createUserAction);
    toolsMenu->addAction(openUserAction);
    fileMenu->addAction(importTaskAction);
    fileMenu->addAction(exportTaskAction);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAction);
}

void MainWindow::createConnections() {
    connect(addNewTaskAction, SIGNAL(triggered()), this, SLOT(addNewTask()));
    connect(view, SIGNAL(clicked(QModelIndex)), this, SLOT(editTask(QModelIndex)));
    connect(deleteTaskAction, SIGNAL(triggered()), this, SLOT(deleteTask()));
    connect(openUserAction, SIGNAL(triggered()), this, SLOT(openUser()));
    connect(createUserAction, SIGNAL(triggered()), this, SLOT(createUser()));
    connect(importTaskAction, SIGNAL(triggered()), this, SLOT(importTask()));
    connect(exportTaskAction, SIGNAL(triggered()), this, SLOT(exportTask()));
    connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));
    connect(sendTaskAction, SIGNAL(triggered()), this, SLOT(sendTask()));
    connect(timerForRem, SIGNAL(timeout()), this, SLOT(checkReminders()));
}

void MainWindow::importTask() {
    if (!currentUser.isEmpty()) {
        auto tasks = tasksDB->loadFromFile(currentUser);
        if (!tasks.isEmpty()) {
            int k = model->rowCount();
            for (const auto& item : tasks) {
                QStandardItem *nameItem = new QStandardItem(item.at(0));
                nameItem->setEditable(false);
                nameItem->setData(item.at(4));
                QStandardItem *descItem = new QStandardItem(item.at(1));
                descItem->setEditable(false);
                QStandardItem *deadlineItem = new QStandardItem(item.at(2));
                deadlineItem->setEditable(false);
                QFont font("Verdana", 10);
                QFont font2("Verdana", 10, QFont::Bold);
                nameItem->setFont(font);
                descItem->setFont(font);
                deadlineItem->setFont(font2);
                if (k % 2) {
                    nameItem->setBackground(QBrush(QColor(135,206,250)));
                    descItem->setBackground(QBrush(QColor(135,206,250)));
                    deadlineItem->setBackground(QBrush(QColor(135,206,250)));
                }
                model->appendRow(QList<QStandardItem *>() << nameItem
                                                          << descItem
                                                          << deadlineItem);
                k++;
            }
        }
    }

}

void MainWindow::exportTask() {
    if (!currentUser.isEmpty() && model->rowCount() > 0)
        tasksDB->saveToFile(currentUser);
}

void MainWindow::contextMenuEvent(QContextMenuEvent *event) {
    QMenu menu(this);
    menu.addAction(addNewTaskAction);
    menu.addAction(deleteTaskAction);
    menu.addAction(sendTaskAction);
    menu.exec(event->globalPos());
}

void MainWindow::createUser() {
    userDialog = std::unique_ptr<UserInputDialog>{ new UserInputDialog("Create User") };
    connect(userDialog.get(), SIGNAL(accepted(const QString&, const QString&)),
            this, SLOT(addUser(const QString&, const QString&)));
}

void MainWindow::addUser(const QString& name, const QString& username) {
    if (tasksDB->addNewUser(name, username)) {
        clearModel();
        currentUser = username;
        setWindowTitle(tr("%1 - %2[*]").arg(QApplication::applicationName())
                       .arg(currentUser));
        view->setVisible(true);
        addNewTaskAction->setEnabled(true);
        editTaskAction->setEnabled(true);
        deleteTaskAction->setEnabled(true);
        sendTaskAction->setEnabled(true);
        importTaskAction->setEnabled(true);
        exportTaskAction->setEnabled(true);
    }
    userDialog->close();
    timerForRem->start();
}

void MainWindow::openUser() {
    userDialog = std::unique_ptr<UserInputDialog>{ new UserInputDialog("Open user tasks") };
    connect(userDialog.get(), SIGNAL(accepted(const QString&, const QString&)),
            this, SLOT(openUserTasks(const QString&, const QString&)));
}

void MainWindow::openUserTasks(const QString &name, const QString &username) {
    auto tasks = tasksDB->getUserTasks(name, username);
    if (!tasks.isEmpty() && tasks.at(0).at(0) == "invalid") {
        return;
    }
    clearModel();
    qDebug() << tasks;
    currentUser = username;
    setWindowTitle(tr("%1 - %2[*]").arg(QApplication::applicationName())
                   .arg(currentUser));
    //view->setVisible(true);
    addNewTaskAction->setEnabled(true);
    editTaskAction->setEnabled(true);
    deleteTaskAction->setEnabled(true);
    sendTaskAction->setEnabled(true);
    importTaskAction->setEnabled(true);
    exportTaskAction->setEnabled(true);
    if (!tasks.isEmpty()) {
        int k = 0;
        for (const auto& item : tasks) {
            QStandardItem *nameItem = new QStandardItem(item.at(0));
            nameItem->setEditable(false);
            nameItem->setData(item.at(4));
            QStandardItem *descItem = new QStandardItem(item.at(1));
            descItem->setEditable(false);
            QStandardItem *deadlineItem = new QStandardItem(item.at(2));
            deadlineItem->setEditable(false);
            QFont font("Verdana", 10);
            QFont font2("Verdana", 10, QFont::Bold);
            nameItem->setFont(font);
            descItem->setFont(font);
            deadlineItem->setFont(font2);
            if (k % 2) {
                nameItem->setBackground(QBrush(QColor(135,206,250)));
                descItem->setBackground(QBrush(QColor(135,206,250)));
                deadlineItem->setBackground(QBrush(QColor(135,206,250)));
            }
            model->appendRow(QList<QStandardItem *>() << nameItem
                                                      << descItem
                                                      << deadlineItem);
            k++;
        }
    }
    userDialog->close();

    checkReminders();

    timerForRem->start();
}

void MainWindow::addNewTask() {
    taskDialog = std::unique_ptr<TaskInputDialog>{ new TaskInputDialog };
    connect(taskDialog.get(), SIGNAL(accepted(const QString&, const QString&,
                                              const QString&, const QString&)),
            this, SLOT(insertNewTask(const QString&, const QString&,
                                     const QString&, const QString&)));
}

void MainWindow::insertNewTask(const QString &taskName, const QString &taskDesc,
                               const QString &deadline, const QString &remainder) {
    QString created = QDateTime::currentDateTime().toString("d MMMM yyyy hh:mm:ss.z");
    tasksDB->addNewTask(currentUser, taskName, taskDesc, deadline, remainder, created);
    QStandardItem *nameItem = new QStandardItem(taskName);
    nameItem->setData(created);
    nameItem->setEditable(false);
    QStandardItem *descItem = new QStandardItem(taskDesc);
    descItem->setEditable(false);
    QStandardItem *deadlineItem = new QStandardItem(deadline);
    deadlineItem->setEditable(false);
    QFont font("Verdana", 10);
    QFont font2("Verdana", 10, QFont::Bold);
    nameItem->setFont(font);
    descItem->setFont(font);
    deadlineItem->setFont(font2);
    if (model->rowCount() % 2) {
        nameItem->setBackground(QBrush(QColor(135,206,250)));
        descItem->setBackground(QBrush(QColor(135,206,250)));
        deadlineItem->setBackground(QBrush(QColor(135,206,250)));
    }
    model->appendRow(QList<QStandardItem *>() << nameItem
                                              << descItem
                                              << deadlineItem);
    taskDialog->close();
}

void MainWindow::editTask(const QModelIndex& index) {

    if (index.isValid()) {
        currentIndex = index;
    } else {
        currentIndex = view->currentIndex();
    }
    auto item = model->item(view->currentIndex().row(),0);
    auto items = tasksDB->getTask(currentUser, item->data().toString());
    taskDialog = std::unique_ptr<TaskInputDialog>{ new TaskInputDialog };
    taskDialog->setFields(items.at(0),items.at(1),items.at(2),items.at(3));
    connect(taskDialog.get(), SIGNAL(accepted(const QString&, const QString&,
                                              const QString&, const QString&)),
            this, SLOT(editNewTask(const QString&, const QString&,
                                   const QString&, const QString&)));
}

void MainWindow::editNewTask(const QString &taskName, const QString &taskDesc,
                             const QString &taskDeadline, const QString &taskRemainder) {
    QString created = QDateTime::currentDateTime().toString("d MMMM yyyy hh:mm:ss.z");

    tasksDB->updateTask(currentUser,model->item(currentIndex.row(),0)->data().toString(),
                        taskName, taskDesc, taskDeadline, taskRemainder, created);
    QStandardItem *nameItem = new QStandardItem(taskName);
    nameItem->setData(created);
    nameItem->setEditable(false);
    QStandardItem *descItem = new QStandardItem(taskDesc);
    descItem->setEditable(false);
    QStandardItem *deadlineItem = new QStandardItem(taskDeadline);
    deadlineItem->setEditable(false);
    QFont font("Verdana", 10);
    QFont font2("Verdana", 10, QFont::Bold);
    nameItem->setFont(font);
    descItem->setFont(font);
    deadlineItem->setFont(font2);
    if (currentIndex.row() % 2) {
        nameItem->setBackground(QBrush(QColor(135,206,250)));
        descItem->setBackground(QBrush(QColor(135,206,250)));
        deadlineItem->setBackground(QBrush(QColor(135,206,250)));
    }
    model->setItem(currentIndex.row(),0,nameItem);
    model->setItem(currentIndex.row(),1,descItem);
    model->setItem(currentIndex.row(),2,deadlineItem);
    taskDialog->close();
    checkReminders();
}

void MainWindow::deleteTask() {
    auto item = model->item(view->currentIndex().row(),0);
    tasksDB->deleteTask(currentUser, item->data().toString());
    model->takeRow(view->currentIndex().row());
}

void MainWindow::sendTask() {
    userDialog = std::unique_ptr<UserInputDialog>{ new UserInputDialog("Send task to user") };
    connect(userDialog.get(), SIGNAL(accepted(const QString&, const QString&)),
            this, SLOT(sendTaskToOtherUser(const QString&, const QString&)));
}

void MainWindow::sendTaskToOtherUser(const QString &name, const QString &username) {
    Q_UNUSED(name)
    auto item = model->item(view->currentIndex().row(),0);
    tasksDB->importTask(currentUser, username, item->data().toString());
    userDialog->close();
}

void MainWindow::checkReminders() {
    QList<QStringList> dueTasks = tasksDB->getReminders(currentUser);
    int k = 1;
    if (!dueTasks.isEmpty()) {
        for (const auto& item : dueTasks) {
            reminderDialog = std::unique_ptr<ReminderDialog>{
                    new ReminderDialog(item.at(0),item.at(1),item.at(2),k,currentUser,item.at(3)) };
            connect(reminderDialog.get(), SIGNAL(dismiss(const QString&, const QString&)),
                    this, SLOT(dismissReminder(const QString&, const QString&)));
            connect(reminderDialog.get(), SIGNAL(snooze(const QString&, const QString&, const QString&)),
                    this, SLOT(snoozeReminder(const QString&, const QString&, const QString&)));
            k++;
        }
    }
    auto count = model->rowCount();
    QDateTime currentTime = QDateTime::currentDateTime();
    for (auto i = 0; i < count; i++) {
        QStandardItem *item = model->item(i,2);
        if (QDateTime::fromString(item->text(),"d.M.yyyy hh.mm") <
                                  currentTime) {
            item->setBackground(QBrush(QColor(255,0,0)));
        }
    }

    QList<QStringList> snoozedTasks = tasksDB->checkSnoozedTasks(currentUser);
    if (!snoozedTasks.isEmpty()) {
        for (const auto& item : snoozedTasks) {
            reminderDialog = std::unique_ptr<ReminderDialog>{
                    new ReminderDialog(item.at(0),item.at(1),item.at(2),k,currentUser,item.at(3)) };
            connect(reminderDialog.get(), SIGNAL(dismiss(const QString&, const QString&)),
                    this, SLOT(dismissReminder(const QString&, const QString&)));
            connect(reminderDialog.get(), SIGNAL(snooze(const QString&, const QString&, const QString&)),
                    this, SLOT(snoozeReminder(const QString&, const QString&, const QString&)));
            k++;
        }
    }

    QList<QStringList> overDueTasks = tasksDB->checkOverDues(currentUser);
    if (!overDueTasks.isEmpty()) {
        for (const auto& item : overDueTasks) {
            reminderDialog = std::unique_ptr<ReminderDialog>{
                    new ReminderDialog(item.at(0),item.at(1),item.at(2),k,currentUser,item.at(3)) };
            connect(reminderDialog.get(), SIGNAL(dismiss(const QString&, const QString&)),
                    this, SLOT(dismissReminder(const QString&, const QString&)));
            connect(reminderDialog.get(), SIGNAL(snooze(const QString&, const QString&, const QString&)),
                    this, SLOT(snoozeReminder(const QString&, const QString&, const QString&)));
            k++;
        }
    }

    QList<QStringList> pendingTasks = tasksDB->checkPendingTasks(currentUser);
    if (!pendingTasks.isEmpty()) {
        for (const auto& item : pendingTasks) {
            reminderDialog = std::unique_ptr<ReminderDialog>{
                    new ReminderDialog(item.at(0),item.at(1),item.at(2),k,currentUser,item.at(3)) };
            connect(reminderDialog.get(), SIGNAL(dismiss(const QString&, const QString&)),
                    this, SLOT(dismissReminder(const QString&, const QString&)));
            connect(reminderDialog.get(), SIGNAL(snooze(const QString&, const QString&, const QString&)),
                    this, SLOT(snoozeReminder(const QString&, const QString&, const QString&)));
            k++;
        }
    }
}

void MainWindow::dismissReminder(const QString& username, const QString& created) {
    tasksDB->dismissReminder(username,created);
}

void MainWindow::snoozeReminder(const QString &username, const QString &created, const QString &snoozeText) {
    qDebug() << snoozeText;
    tasksDB->dismissReminder(username,created);
    QString snoozeCreated = QDateTime::currentDateTime().toString("d.M.yyyy hh:mm");
    tasksDB->setSnoozeForTask(username, created, snoozeText, snoozeCreated);
}

