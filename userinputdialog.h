#ifndef USERINPUTDIALOG_H
#define USERINPUTDIALOG_H

#include <QDialog>

class QLabel;
class QLineEdit;
class QDialogButtonBox;
class QPushButton;
class QWidget;
class QCloseEvent;

class UserInputDialog : public QDialog
{
    Q_OBJECT
public:
    explicit UserInputDialog(const QString&,
                             QWidget *parent = 0);
protected:
    void closeEvent(QCloseEvent *event);

signals:
    void accepted(const QString&, const QString&);

private slots:
  void acceptInput();

private:
    void createWidgets();
    void createLayout();
    void createConnections();

    QLabel *nameLabel;
    QLineEdit *nameLineEdit;
    QLabel *usernameLabel;
    QLineEdit *usernameLineEdit;
    QDialogButtonBox *buttonBox;
    QPushButton *okButton;
    QPushButton *cancelButton;
};

#endif // USERINPUTDIALOG_H
