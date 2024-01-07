#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAction>
#include <QSettings>
#include <QFileDialog>
#include <QTableWidgetItem>
#include <QTimer>
#include <QMessageBox>
#include <QShortcut>

//все для процессов ОС
#include <Windows.h>
#include <Psapi.h>
#include <TlHelp32.h>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    double GetCPULoad();

    void updateCPULoadLabel();
    void updateProcessTable();

    QString determinePriority(int valueOfPriority);
    ~MainWindow();
private slots:
    //void on_pushButton_searchButton_clicked();
    void on_pushButton_startNewTask_clicked();
    void on_pushButton_terminateProcess_clicked();
    void on_tableWidget_processes_itemClicked(QTableWidgetItem *item);
    void on_pushButton_clearSearchField_clicked();
    void on_pushButton_searchButton_clicked();
    void on_tableWidget_processes_itemSelectionChanged();

private:
    Ui::MainWindow *ui;
    QTimer* updateTableTimer;
    QTimer* updateCPULabelTimer;
    QTableWidgetItem *currentItemToRemove;

};
#endif // MAINWINDOW_H
