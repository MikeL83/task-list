#ifndef REMINDERDIALOG_H
#define REMINDERDIALOG_H

#include <QDialog>
#include <tuple>
class QLabel;
class QComboBox;
class QPushButton;
class QCloseEvent;
class QTimer;

class ReminderDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ReminderDialog(const QString&, const QString&,
                            const QString&, int,const QString&,
                            const QString&, QWidget *parent = 0);

protected:
    void closeEvent(QCloseEvent *event);

signals:
    void dismiss(const QString&, const QString&);
    void snooze(const QString&, const QString&,
                const QString&);
    //void accepted(const QString&, const QString&,
    //              const QString&, const QString&);

private slots:
    void dismissDialog();
    void snoozeTask();
    void populateCombobox();

private:
    void createWidgets();
    void createLayout();
    void createConnections();

    QLabel *iconLabel;
    QLabel *taskNameLabel;
    QLabel *startTimeLabel;
    QLabel *dueTimeLabel;
    QComboBox *snoozeBox;
    QPushButton *dismissButton;
    QPushButton *snoozeButton;
    std::tuple<QString,QString,QString,int,QString,QString> inputs;
    QTimer *timer;

};

#endif // REMINDERDIALOG_H
