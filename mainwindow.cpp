#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "mydialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->pushButtonSettings, &QPushButton::clicked, this, &MainWindow::showSettingsDialog);
    connect(ui->pushButtonRun, &QPushButton::clicked, this, &MainWindow::pushButtonRunClicked);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::showSettingsDialog()
{
    MyDialog dialog(this);

    if (dialog.exec() == QDialog::Accepted)
    {
        firstNum = dialog.getFirstNum();
        secondNum = dialog.getSecondNum();
        timerVal = dialog.getTimerVal();
    }

}

int MainWindow::getFirstNum() const
{
    return this->firstNum;
};
int MainWindow::getSecondNum() const
{
    return this->secondNum;
};
int MainWindow::getTimerVal() const
{
    return this->timerVal;
};

void MainWindow::logToFile(const QString& message)
{
    std::lock_guard<std::mutex> lock(fileMtx);
    if (logFile.is_open())
    {
        logFile << message.toUtf8().constData() << std::endl;
    }
};


void MainWindow::pushButtonRunClicked()
{
    int first = this->getFirstNum();
    int second = this->getSecondNum();
    int duration = this->getTimerVal();

    ui->textEditLog->clear();
    ui->pushButtonRun->setEnabled(false); // Выключить кнопку

    {
        std::lock_guard<std::mutex> lock(fileMtx);
        logFile.open("output.txt", std::ios::out); // перезаписать при каждом запуске
    }
    finishedThreads = 0;


    // Запуск потоков
    std::thread multiplicationTask(&MainWindow::runMultiplicationTask, this, first, second, duration);
    std::thread controlTask(&MainWindow::runControlTask, this, duration);
    multiplicationTask.detach();
    controlTask.detach();
}

void MainWindow::runMultiplicationTask(int first, int second, int duration)
{
    auto start = std::chrono::steady_clock::now();
    auto finish = start + std::chrono::seconds(duration);
    constexpr double softCap = (std::numeric_limits<double>::max() /1e3);

    {
        std::lock_guard<std::mutex> lock(interMtx);
        interRes = (double)first*second;
    }

    while (finish > std::chrono::steady_clock::now())
    {
        {
            // Пауза, если того требует второй поток
            std::unique_lock<std::mutex> lock(pauseMtx);
            pauseCv.wait(lock, [this]() { return !paused; });
        }

        {
            std::lock_guard<std::mutex> lock(interMtx);
            interRes *= second;
            // Защита от переполнения, double поддерживает максимум порядка e308
            if(interRes > softCap)
            {
                interRes = softCap;
            }
        }

        logToFile( QString::number(interRes) );
        // Задержка для того, чтобы файл лога не был бесконечным
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    }

    if (++finishedThreads == 2)
    {
        std::lock_guard<std::mutex> lock(fileMtx);
        if (logFile.is_open())
        {
            //logFile << "All tasks finished." << std::endl;
            logFile.close();
        }
    }

    QMetaObject::invokeMethod(this, [this]()
    {
        ui->pushButtonRun->setEnabled(true);
    });

}

void MainWindow::runControlTask(int duration)
{
    auto start = std::chrono::steady_clock::now();
    auto finish = start + std::chrono::seconds(duration);
    int interval = duration*1000/60; // milliseconds
    int cnt = 0;

    while (true)
    {
        auto now = std::chrono::steady_clock::now();
        if (now + std::chrono::milliseconds(interval) > finish)
            break;

        std::this_thread::sleep_for(std::chrono::milliseconds(interval));

        {
            std::lock_guard<std::mutex> lock(pauseMtx);
            paused = true;
        }

        double valueToLog;
        {
            std::lock_guard<std::mutex> lock(interMtx);
            valueToLog = interRes;
        }

        QMetaObject::invokeMethod(this, [this, valueToLog]()
        {
            ui->textEditLog->append(QString::number(valueToLog));
            QString str = QString::number(valueToLog) + " - intermediate result";
            logToFile(str);
        });

        {
            std::lock_guard<std::mutex> lock(pauseMtx);
            paused = false;
        }
        pauseCv.notify_one();
    }

    if (++finishedThreads == 2)
    {
        std::lock_guard<std::mutex> lock(fileMtx);
        if (logFile.is_open())
        {
            //logFile << "All tasks finished." << std::endl;
            logFile.close();
        }
    }
}

