#include "taskinputdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QString>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QWidget>
#include <QRegExpValidator>
#include <QCloseEvent>
#include <QDateEdit>
#include <QComboBox>
#include <QTimeEdit>
#include <QCloseEvent>

TaskInputDialog::TaskInputDialog(QWidget *parent) : QDialog(parent)
{
    createWidgets();
    createLayout();
    createConnections();

    setWindowTitle(tr("Add new task"));
    show();
}

void TaskInputDialog::closeEvent(QCloseEvent *event)
{
    event->accept();
}

void TaskInputDialog::createWidgets()
{
    buttonBox =
        new QDialogButtonBox(QDialogButtonBox::Close | QDialogButtonBox::Ok);
    taskNameLabel = new QLabel(tr("Task name:"));
    taskNameEdit = new QLineEdit;
    taskNameEdit->setMaxLength(100);
    taskNameLabel->setBuddy(taskNameEdit);
    taskNameEdit->setFocus();

    taskDescLabel = new QLabel(tr("Task description:"));
    taskDescEdit = new QLineEdit;
    taskDescEdit->setMaxLength(100);
    taskDescLabel->setBuddy(taskDescEdit);

    taskDeadlineLabel = new QLabel(tr("Task deadline:"));
    taskDeadlineDateEdit = new QDateEdit(this);
    taskDeadlineLabel->setBuddy(taskDeadlineDateEdit);
    taskDeadlineTimeEdit = new QTimeEdit(this);
    taskRemainderLabel = new QLabel(tr("Choose a reminder for the task:"));
    remainderBox = new QComboBox(this);
    remainderBox->insertItem(0, "1 day");
    remainderBox->insertItem(1, "2 hrs");
    remainderBox->insertItem(2, "1 hr");
    remainderBox->insertItem(3, "30 mins");
    remainderBox->insertItem(4, "10 mins");
    remainderBox->insertItem(5, "no reminder");
}

void TaskInputDialog::createLayout()
{
    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->addWidget(taskNameLabel);
    vLayout->addWidget(taskNameEdit);
    vLayout->addWidget(taskDescLabel);
    vLayout->addWidget(taskDescEdit);
    vLayout->addWidget(taskDeadlineLabel);
    vLayout->addWidget(taskDeadlineDateEdit);
    vLayout->addWidget(taskDeadlineTimeEdit);
    vLayout->addWidget(taskRemainderLabel);
    QHBoxLayout *layoutCombobox = new QHBoxLayout;
    layoutCombobox->addStretch(2);
    layoutCombobox->addWidget(remainderBox);
    vLayout->addLayout(layoutCombobox);
    vLayout->addStretch(3);
    QHBoxLayout *layoutForButtons = new QHBoxLayout;
    layoutForButtons->addStretch(2);
    layoutForButtons->addWidget(buttonBox);
    vLayout->addLayout(layoutForButtons);
    setLayout(vLayout);
}

void TaskInputDialog::createConnections()
{
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(acceptInput()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(close()));
}

void TaskInputDialog::acceptInput()
{
    QString deadline =
        taskDeadlineDateEdit->text() + " " + taskDeadlineTimeEdit->text();
    emit accepted(taskNameEdit->text(), taskDescEdit->text(), deadline,
                  remainderBox->currentText());
}

void TaskInputDialog::setFields(const QString &taskName,
                                const QString &taskDesc,
                                const QString &deadline,
                                const QString &remainder)
{
    taskNameEdit->setText(taskName);
    taskDescEdit->setText(taskDesc);
    QString date = deadline;
    date.truncate(date.indexOf(" "));
    QString time = deadline;
    time = time.mid(time.indexOf(" ") + 1);
    taskDeadlineDateEdit->setDate(QDate::fromString(date, "d.M.yyyy"));
    taskDeadlineTimeEdit->setTime(QTime::fromString(time, "hh.mm"));
    remainderBox->setCurrentIndex(remainderBox->findText(remainder));
    taskNameEdit->setFocus();
}
