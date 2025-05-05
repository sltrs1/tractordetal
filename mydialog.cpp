#include "mydialog.h"
#include "ui_mydialog.h"

#include <QIntValidator>
#include <QLocale>
#include <QSettings>
#include <QMessageBox>

MyDialog::MyDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MyDialog)
{
    ui->setupUi(this);

    QIntValidator *validatorNums = new QIntValidator(-9, 9, this);
    ui->lineEditFirstNum->setValidator(validatorNums);
    ui->lineEditSecondNum->setValidator(validatorNums);

    QIntValidator *validatorTime = new QIntValidator(0, 30, this);
    ui->lineEditTimer->setValidator(validatorTime);

    // Явное указание на ini-файл в рабочей директории
    QSettings settings("settings.ini", QSettings::IniFormat);

    ui->lineEditFirstNum->setText(settings.value("firstNum", "0").toString());
    ui->lineEditSecondNum->setText(settings.value("secondNum", "0").toString());
    ui->lineEditTimer->setText(settings.value("timer", "0").toString());

    bool darkTheme = settings.value("darkTheme", false).toBool();
    ui->checkBoxTheme->setChecked(darkTheme);  // установим состояние чекбокса
    applyTheme(darkTheme);                     // применим стиль

    connect(ui->checkBoxTheme, &QCheckBox::toggled, this, &MyDialog::onThemeToggled);
    connect(ui->pushButtonDialogConfirm, &QPushButton::clicked, this, &MyDialog::onConfirmClicked);
}

MyDialog::~MyDialog()
{
    delete ui;
}

void MyDialog::onThemeToggled(bool checked)
{
    applyTheme(checked);
}

void MyDialog::onConfirmClicked()
{
    bool ok1, ok2, ok3;
    int first = ui->lineEditFirstNum->text().toInt(&ok1);
    int second = ui->lineEditSecondNum->text().toInt(&ok2);
    int timer = ui->lineEditTimer->text().toInt(&ok3);

    // Двойная проверка на всякий случай
    if (!ok1 || first < -9 || first > 9)
    {
        QMessageBox::warning(this, "Ошибка", "Первое число должно быть от -9 до 9.");
        return;
    }

    if (!ok2 || second < -9 || second > 9)
    {
        QMessageBox::warning(this, "Ошибка", "Второе число должно быть от -9 до 9.");
        return;
    }

    if (!ok3 || timer < 0 || timer > 30)
    {
        QMessageBox::warning(this, "Ошибка", "Значение таймера должно быть от 0 до 30 секунд.");
        return;
    }

    // Сохраняем настройки в тот же ini-файл
    QSettings settings("settings.ini", QSettings::IniFormat);

    settings.setValue("firstNum", ui->lineEditFirstNum->text());
    settings.setValue("secondNum", ui->lineEditSecondNum->text());
    settings.setValue("timer", ui->lineEditTimer->text());
    settings.setValue("darkTheme", ui->checkBoxTheme->isChecked());

    accept(); // если хочешь закрыть окно при подтверждении
}

void MyDialog::applyTheme(bool dark)
{
    QString darkStyle = "QWidget { background-color: #2e2e2e; color: white; }";
    QString lightStyle = "";

    this->setStyleSheet(dark ? darkStyle : lightStyle);

    if (parentWidget()) {
        parentWidget()->setStyleSheet(dark ? darkStyle : lightStyle);
    }
}

int MyDialog::getFirstNum() const {
    return ui->lineEditFirstNum->text().toInt();
}

int MyDialog::getSecondNum() const {
    return ui->lineEditSecondNum->text().toInt();
}

int MyDialog::getTimerVal() const {
    return ui->lineEditTimer->text().toInt();
}
