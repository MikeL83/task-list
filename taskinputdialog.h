#ifndef TASKINPUTDIALOG_H
#define TASKINPUTDIALOG_H

#include <QDialog>

class QLabel;
class QLineEdit;
class QDialogButtonBox;
class QPushButton;
class QWidget;
class QCloseEvent;
class QComboBox;
class QDateEdit;
class QTimeEdit;
class QCloseEvent;

class TaskInputDialog : public QDialog
{
    Q_OBJECT
  public:
    explicit TaskInputDialog(QWidget *parent = 0);
    void setFields(const QString &, const QString &, const QString &,
                   const QString &);

  protected:
    void closeEvent(QCloseEvent *event);

  signals:
    void accepted(const QString &, const QString &, const QString &,
                  const QString &);

  private slots:
    void acceptInput();

  private:
    void createWidgets();
    void createLayout();
    void createConnections();

    QLabel *taskNameLabel;
    QLineEdit *taskNameEdit;
    QLabel *taskDescLabel;
    QLineEdit *taskDescEdit;
    QLabel *taskDeadlineLabel;
    QDateEdit *taskDeadlineDateEdit;
    QTimeEdit *taskDeadlineTimeEdit;
    QLabel *taskRemainderLabel;
    QComboBox *remainderBox;
    QDialogButtonBox *buttonBox;
};

#endif // TASKINPUTDIALOG_H
