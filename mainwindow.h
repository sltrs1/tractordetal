#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <future>
#include <thread>
#include <mutex>
#include <atomic>
#include <fstream>
#include <iostream>
#include <limits>
#include <condition_variable>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    int getFirstNum() const;
    int getSecondNum() const;
    int getTimerVal() const;

private:
    Ui::MainWindow *ui;
    int firstNum = 0;
    int secondNum = 0;
    int timerVal = 0;

    std::mutex fileMtx;
    std::mutex interMtx;
    double interRes = 0.0;

    std::condition_variable pauseCv;
    std::mutex pauseMtx;
    bool paused = false;

    std::ofstream logFile;
    std::atomic<int> finishedThreads = 0;

    void runMultiplicationTask(int first, int second, int duration);
    void runControlTask(int duration);

    void logToFile(const QString& message);

private slots:
    void showSettingsDialog();
    void pushButtonRunClicked();
};
#endif // MAINWINDOW_H
