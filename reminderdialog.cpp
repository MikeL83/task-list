#include "reminderdialog.h"
#include <tuple>
#include <QApplication>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QCloseEvent>
#include <QMessageBox>
#include <QFile>
#include <QSpacerItem>
#include <QString>
#include <QFont>
#include <QSizePolicy>
#include <QDateTime>
#include <QStringList>
#include <QTimer>

ReminderDialog::ReminderDialog(const QString &taskName,
                               const QString &startTime, const QString &dueIn,
                               int numberOfRem, const QString &username,
                               const QString &created, QWidget *parent)
    : QDialog(parent)
{
    // store user input temporarily into tuple
    inputs = std::make_tuple(taskName, startTime, dueIn, numberOfRem, username,
                             created);
    timer = new QTimer(this);
    timer->setInterval(1000 * 60);
    createWidgets();
    createLayout();
    createConnections();

    setWindowTitle(QString("Reminder %1 - ").arg(std::get<3>(inputs)) +
                   QApplication::applicationName());
    resize(400, 200);
    show();
}

void ReminderDialog::closeEvent(QCloseEvent *event)
{
    event->accept();
}

void ReminderDialog::createWidgets()
{
    dismissButton = new QPushButton(tr("Dismiss"), this);
    snoozeButton = new QPushButton(tr("Snooze"), this);
    iconLabel = new QLabel(this);
    QPixmap icon(":/images/Information-icon.png");
    setWindowIcon(icon);
    iconLabel->setPixmap(icon);
    taskNameLabel = new QLabel(
        tr("&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"
           "<b><font color = '#006699'>Task "
           "name:&nbsp;&nbsp;&nbsp;</font></b>" +
           std::get<0>(inputs).toLatin1()),
        this);
    taskNameLabel->setTextFormat(Qt::RichText);
    startTimeLabel = new QLabel(
        tr("&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"
           "<b><font color = '#006699'>Start "
           "time:&nbsp;&nbsp;&nbsp;</font></b>" +
           std::get<1>(inputs).toLatin1()),
        this);
    startTimeLabel->setTextFormat(Qt::RichText);
    dueTimeLabel = new QLabel(
        tr("&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"
           "<b><font color = '#006699'>Due in:&nbsp;&nbsp;&nbsp;</font></b>" +
           std::get<2>(inputs).toLatin1()),
        this);
    dueTimeLabel->setTextFormat(Qt::RichText);
    snoozeBox = new QComboBox(this);
    populateCombobox();
}

void ReminderDialog::createLayout()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QHBoxLayout *layoutForIcon = new QHBoxLayout;
    layoutForIcon->addWidget(iconLabel);
    layoutForIcon->addStretch(1);
    mainLayout->addLayout(layoutForIcon);
    mainLayout->addWidget(taskNameLabel);
    mainLayout->addWidget(startTimeLabel);
    mainLayout->addWidget(dueTimeLabel);
    QHBoxLayout *layoutForDismiss = new QHBoxLayout;
    layoutForDismiss->addStretch(1);
    layoutForDismiss->addWidget(dismissButton);
    mainLayout->addLayout(layoutForDismiss);
    QHBoxLayout *layoutForSnooze = new QHBoxLayout;
    layoutForSnooze->addWidget(snoozeBox);
    snoozeBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    layoutForSnooze->addWidget(snoozeButton);
    mainLayout->addLayout(layoutForSnooze);
    setLayout(mainLayout);
}

void ReminderDialog::createConnections()
{
    connect(dismissButton, SIGNAL(clicked()), this, SLOT(dismissDialog()));
    connect(snoozeButton, SIGNAL(clicked()), this, SLOT(snoozeTask()));
    connect(timer, SIGNAL(timeout()), this, SLOT(populateCombobox()));

    timer->start();
}

void ReminderDialog::dismissDialog()
{

    emit dismiss(std::get<4>(inputs), std::get<5>(inputs));
    timer->stop();
    close();
}

void ReminderDialog::snoozeTask()
{
    emit snooze(std::get<4>(inputs), std::get<5>(inputs),
                snoozeBox->currentText());
    timer->stop();
    close();
}

void ReminderDialog::populateCombobox()
{
    /**
      * Combobox will be populated with appropriate items based on
      * due time of a task. Also this method has been set trigger
      * every 1 min (in case the user does not close the window,
      * items will be interactively changed).
      *
    **/

    QDateTime currentTime = QDateTime::currentDateTime();
    QDateTime dueTime =
        QDateTime::fromString(std::get<1>(inputs), "d.M.yyyy hh.mm");
    if (currentTime > dueTime.addSecs(-5 * 60)) {
        snoozeBox->clear();
        return;
    } else if (currentTime < dueTime.addSecs(-3600 * 4)) {
        snoozeBox->clear();
        snoozeBox->addItems(QStringList() << "5 mins before start"
                                          << "10 mins before start"
                                          << "5 mins"
                                          << "10 mins"
                                          << "15 mins"
                                          << "30 mins"
                                          << "1 hour"
                                          << "2 hours"
                                          << "4 hours");
        return;
    } else if (currentTime < dueTime.addSecs(-3600 * 2) &&
               currentTime > dueTime.addSecs(-3600 * 4)) {
        snoozeBox->clear();
        snoozeBox->addItems(QStringList() << "5 mins before start"
                                          << "10 mins before start"
                                          << "5 mins"
                                          << "10 mins"
                                          << "15 mins"
                                          << "30 mins"
                                          << "1 hour"
                                          << "2 hours");
        return;
    } else if (currentTime < dueTime.addSecs(-3600) &&
               currentTime > dueTime.addSecs(-3600 * 2)) {
        snoozeBox->clear();
        snoozeBox->addItems(QStringList() << "5 mins before start"
                                          << "10 mins before start"
                                          << "5 mins"
                                          << "10 mins"
                                          << "15 mins"
                                          << "30 mins"
                                          << "1 hour");
        return;
    } else if (currentTime < dueTime.addSecs(-60 * 30) &&
               currentTime > dueTime.addSecs(-3600)) {
        snoozeBox->clear();
        snoozeBox->addItems(QStringList() << "5 mins before start"
                                          << "10 mins before start"
                                          << "5 mins"
                                          << "10 mins"
                                          << "15 mins"
                                          << "30 mins");
        return;
    } else if (currentTime < dueTime.addSecs(-60 * 15) &&
               currentTime > dueTime.addSecs(-60 * 30)) {
        snoozeBox->clear();
        snoozeBox->addItems(QStringList() << "5 mins before start"
                                          << "10 mins before start"
                                          << "5 mins"
                                          << "10 mins"
                                          << "15 mins");
        return;
    } else if (currentTime < dueTime.addSecs(-60 * 10) &&
               currentTime > dueTime.addSecs(-60 * 15)) {
        snoozeBox->clear();
        snoozeBox->addItems(QStringList() << "5 mins before start"
                                          << "10 mins before start"
                                          << "5 mins"
                                          << "10 mins");
        return;
    } else if (currentTime < dueTime.addSecs(-60 * 5) &&
               currentTime > dueTime.addSecs(-60 * 10)) {
        snoozeBox->clear();
        snoozeBox->addItems(QStringList() << "5 mins before start"
                                          << "5 mins");

        return;
    }
}
