#include "userinputdialog.h"
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

UserInputDialog::UserInputDialog(const QString &title, QWidget *parent)
    : QDialog(parent)
{
    createWidgets();
    createLayout();
    createConnections();

    setWindowTitle(tr(qPrintable(title)));
    show();
}

void UserInputDialog::closeEvent(QCloseEvent *event)
{
    event->accept();
}

void UserInputDialog::createWidgets()
{
    buttonBox =
        new QDialogButtonBox(QDialogButtonBox::Close | QDialogButtonBox::Ok);
    nameLabel = new QLabel(tr("Name:"));
    nameLineEdit = new QLineEdit;
    nameLineEdit->setValidator(
        new QRegExpValidator(QRegExp("[A-Za-z\\s]+"), this));
    nameLineEdit->setMaxLength(100);
    nameLabel->setBuddy(nameLineEdit);

    usernameLabel = new QLabel(tr("Username:"));
    usernameLineEdit = new QLineEdit;
    usernameLineEdit->setValidator(
        new QRegExpValidator(QRegExp("[A-Za-z\\d_]+"), this));
    usernameLineEdit->setMaxLength(100);
    usernameLabel->setBuddy(usernameLineEdit);
}

void UserInputDialog::createLayout()
{
    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->addWidget(nameLabel);
    vLayout->addWidget(nameLineEdit);
    vLayout->addWidget(usernameLabel);
    vLayout->addWidget(usernameLineEdit);
    vLayout->addStretch(3);
    QHBoxLayout *layoutForButtons = new QHBoxLayout;
    layoutForButtons->addStretch(2);
    layoutForButtons->addWidget(buttonBox);
    vLayout->addLayout(layoutForButtons);
    setLayout(vLayout);
}

void UserInputDialog::createConnections()
{
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(acceptInput()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(close()));
}

void UserInputDialog::acceptInput()
{
    emit accepted(nameLineEdit->text(), usernameLineEdit->text());
}
