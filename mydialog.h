#ifndef MYDIALOG_H
#define MYDIALOG_H

#include <QDialog>

namespace Ui {
class MyDialog;
}

class MyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MyDialog(QWidget *parent = nullptr);
    ~MyDialog();

    int getFirstNum() const;
    int getSecondNum() const;
    int getTimerVal() const;

private:
    Ui::MyDialog *ui;

private slots:
    void onThemeToggled(bool checked);
    void onConfirmClicked();

private:  // methods
    void applyTheme(bool dark);
};

#endif // MYDIALOG_H
