#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QPalette>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("Диспетчер задач для Windows");

    // Установка изображения на кнопку
    ui->pushButton_searchButton->setIcon(QIcon(":/icons/images/search.png"));
    ui->pushButton_terminateProcess->setEnabled(false);

    //Настройка таблицы
    ui->tableWidget_processes->setColumnCount(6);
    ui->tableWidget_processes->setHorizontalHeaderLabels({"Имя процесса","PID","Parent PID","Количество потоков процесса","Приоритет процесса","Путь процесса"});
    ui->tableWidget_processes->resizeColumnsToContents();
    ui->tableWidget_processes->resizeRowsToContents();
    ui->tableWidget_processes->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    //////////////////////////////////////////////////

    updateProcessTable();
    //обновление таблицы каждые n мс
     updateTableTimer= new QTimer(this);
    //updateTableTimer->setInterval(2000);
    connect(updateTableTimer,&QTimer::timeout,this,&MainWindow::updateProcessTable);
    updateTableTimer->start(2000);
    //////////////////////////////////////////////////////////

    updateCPULabelTimer=new QTimer(this);
    connect(updateCPULabelTimer,&QTimer::timeout,this,&MainWindow::updateCPULoadLabel);
    updateCPULabelTimer->start(500);


    //Горячие клавиши
    QShortcut *executeNewTask=new  QShortcut(this);
    executeNewTask->setKey(Qt::CTRL + Qt::Key_N);
    connect(executeNewTask,&QShortcut::activated,this,&MainWindow::on_pushButton_startNewTask_clicked);

    QShortcut *killProcess=new  QShortcut(this);
    killProcess->setKey(Qt::CTRL + Qt::Key_D);
    connect(killProcess,&QShortcut::activated,this,&MainWindow::on_pushButton_terminateProcess_clicked);

    QShortcut *findProcess=new  QShortcut(this);
    findProcess->setKey(Qt::CTRL + Qt::Key_F);
    connect(findProcess,&QShortcut::activated,this,&MainWindow::on_pushButton_searchButton_clicked);

   // ui->lcdNumber->setAutoFillBackground(true);
}

void MainWindow::updateProcessTable(){
    int activatedRow =-1;
    //qDebug() << "Индекс текущей выделенной строки: " << activatedRow;
    activatedRow =  ui->tableWidget_processes->currentRow();

    //qDebug() << "Индекс текущей выделенной строки: " << activatedRow;
    // Очищаем таблицу перед заполнением
    ui->tableWidget_processes->clearContents();
    ui->tableWidget_processes->setRowCount(0);

    int processCount = 0;
    int threadCount = 0;

    //
    HANDLE Snapshot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);//Включает все процессы в системе в snapshot.

    if(Snapshot!=INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 _processEntry;
        _processEntry.dwSize=sizeof(PROCESSENTRY32);//Размер структуры в байтах.Если не инициализировать dwSize, process32First завершается сбоем.

        if(Process32First(Snapshot,&_processEntry)){

            do{
                int row = ui->tableWidget_processes->rowCount();
                ui->tableWidget_processes->insertRow(row);


                QTableWidgetItem* processNameItem = new QTableWidgetItem(QString::fromWCharArray(_processEntry.szExeFile));
                ui->tableWidget_processes->setItem(row,0,processNameItem);

                QTableWidgetItem* processIdItem = new QTableWidgetItem(QString::number(_processEntry.th32ProcessID));
                ui->tableWidget_processes->setItem(row,1,processIdItem);

                QTableWidgetItem* processPidItem = new QTableWidgetItem(QString::number(_processEntry.th32ParentProcessID));
                ui->tableWidget_processes->setItem(row,2,processPidItem);

                QTableWidgetItem* processThreadItem = new QTableWidgetItem(QString::number(_processEntry.cntThreads));
                ui->tableWidget_processes->setItem(row,3,processThreadItem);

                QTableWidgetItem* processPriorityItem = new QTableWidgetItem(determinePriority(_processEntry.pcPriClassBase));
                ui->tableWidget_processes->setItem(row,4,processPriorityItem);

                HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION  | READ_CONTROL | PROCESS_ALL_ACCESS, FALSE, _processEntry.th32ProcessID);
                if(hProcess){
                    WCHAR filename[MAX_PATH];
                    if(GetModuleFileNameEx(hProcess, NULL, filename, MAX_PATH)){
                        QTableWidgetItem* processPathItem = new QTableWidgetItem(QString::fromWCharArray(filename));
                        ui->tableWidget_processes->setItem(row,5,processPathItem);
                    }
                    else{
                        QMessageBox::critical(this, "Error", QString("QueryFullProcessImageName failed with error %1").arg(GetLastError()));
                    }


                    CloseHandle(hProcess);
                }
                else{
                    QTableWidgetItem* processPathItem = new QTableWidgetItem(QString("Нет доступа"));
                    ui->tableWidget_processes->setItem(row,5,processPathItem);
                }

                processCount++;
                threadCount += _processEntry.cntThreads;

            }while(Process32Next(Snapshot,&_processEntry));
        }

        CloseHandle(Snapshot);
    }
    else{
        QMessageBox::critical(this,"Error of Snapshot","Error of Snapshot");
        return;
    }

    ui->tableWidget_processes->selectRow(activatedRow);
    ui->statusbar->showMessage(QString("Процессы: %1, Потоки: %2").arg(processCount).arg(threadCount));
}


//QString MainWindow::determinePriority(long valueOfPriority){
//    if(valueOfPriority <= 4){
//        return QString("Низкий");
//    }
//    else if(valueOfPriority <= 8){
//        return QString("Обычный");
//    }
//    else if(valueOfPriority <= 13){
//        return QString("Высокий");
//    }
//    else if(valueOfPriority <= 24){
//        return QString("Реального времени");
//    }
//    else{
//        return QString("Обычный");
//    }
//}

QString MainWindow::determinePriority(int valueOfPriority) {
    if (valueOfPriority <= 4) {
        return QString("Низкий");
    } else if (valueOfPriority > 4 && valueOfPriority <= 7) {
        return QString("Ниже среднего");
    } else if (valueOfPriority == 8) {
        return QString("Обычный");
    } else if (valueOfPriority <= 12) {
        return QString("Выше среднего");
    } else if (valueOfPriority == 13) {
        return QString("Высокий");
    } else if (valueOfPriority <= 24) {
        return QString("Реального времени");
    } else {
        return QString("Обычный");
    }
}

MainWindow::~MainWindow()
{
    delete ui;

}



void MainWindow::on_pushButton_startNewTask_clicked()
{
    QString programPath=QFileDialog::getOpenFileName(this,"Обзор","","Программы (*.exe;*.bat;*.cmd)");

    if(programPath.isNull()){
        return;
    }

//    BOOL CreateProcessA(
//        [in, optional]      LPCSTR                lpApplicationName,
//        [in, out, optional] LPSTR                 lpCommandLine,
//        [in, optional]      LPSECURITY_ATTRIBUTES lpProcessAttributes,
//        [in, optional]      LPSECURITY_ATTRIBUTES lpThreadAttributes,
//        [in]                BOOL                  bInheritHandles,
//        [in]                DWORD                 dwCreationFlags,
//        [in, optional]      LPVOID                lpEnvironment,
//        [in, optional]      LPCSTR                lpCurrentDirectory,
//        [in]                LPSTARTUPINFOA        lpStartupInfo,
//        [out]               LPPROCESS_INFORMATION lpProcessInformation
//        );

    STARTUPINFO startupInfo = {sizeof(STARTUPINFO)};
    PROCESS_INFORMATION processInfo;

    if (CreateProcess(NULL, (LPWSTR)programPath.utf16(), NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInfo)) {
        CloseHandle(processInfo.hProcess);
        CloseHandle(processInfo.hThread);
        QMessageBox::information(this,"Новый процесс успешно запущен",QString("Процесс %1 запущен").arg(programPath));
    }
    else{
        QMessageBox::critical(this, "Error", QString("Create process failed with error %1").arg(GetLastError()));
    }
}


void MainWindow::on_pushButton_terminateProcess_clicked()
{
    int selectedRow = ui->tableWidget_processes->currentRow();

//    if(!ui->pushButton_terminateProcess->isEnabled()){
//         QMessageBox::warning(this,"Ошибка завершения процесса","Необходимо выбрать процесс");
//         return;
//    }

    QTableWidgetItem* pidItem = ui->tableWidget_processes->item(selectedRow, 1);
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE,FALSE,pidItem->text().toInt());
    if(hProcess){
        //hProcess - дескриптор процесса, 0 - Код завершения, который будет использоваться процессом и потоками, завершенными в результате этого вызова
        if(TerminateProcess(hProcess,0)!=0){
            QMessageBox::information(this,"Процесс успешно завершен",QString("Прооцесс с PID %1 успешно завершен").arg(pidItem->text()));
            CloseHandle(hProcess);
            ui->pushButton_terminateProcess->setEnabled(false);
            ui->tableWidget_processes->clearSelection();
            updateProcessTable();

        }
        else{
            QMessageBox::warning(this,"Ошибка завершения процесса",QString("Не удалось завершить прооцесс с PID %1").arg(pidItem->text()));
        }

    }
    else{
        QMessageBox::critical(this, "Error", QString("OpenProcess failed with error %1").arg(GetLastError()));
    }

}






void MainWindow::on_tableWidget_processes_itemClicked(QTableWidgetItem *item)
{
   // updateTableTimer->stop();
    ui->pushButton_terminateProcess->setEnabled(true);
}


void MainWindow::on_pushButton_clearSearchField_clicked()
{

     //ui->tableWidget_processes->clearSelection();
     //updateTableTimer->start(2000);
     ui->pushButton_terminateProcess->setEnabled(false);
     ui->lineEdit_searchField->clear();
     updateProcessTable();
    // ui->tableWidget_processes->clearSelection();
}





void MainWindow::on_pushButton_searchButton_clicked()
{

     if(ui->lineEdit_searchField->text().isEmpty()){
        QMessageBox::warning(this,"Поле для поиска пустое","Заполните поле для поиска процесса");
        return;
     }


     bool isFind = false;
     QString value = ui->lineEdit_searchField->text();

     // Создаем snapshot всех процессов в системе
     HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

     if (hSnapshot == INVALID_HANDLE_VALUE) {
         QMessageBox::critical(this,"Error of Snapshot","Error of Snapshot");
         return;
     }

     // Инициализируем структуру PROCESSENTRY32
     PROCESSENTRY32 _processEntry;
     _processEntry.dwSize = sizeof(PROCESSENTRY32);

     // Получаем первый процесс
     if (!Process32First(hSnapshot, &_processEntry)) {
        CloseHandle(hSnapshot);
        return;
     }

     bool isNumeric;
     // Пробуем преобразовать введенное значение в число (PID)
     int pid = value.toInt(&isNumeric);
     updateTableTimer->stop();


     int processCount = 0;
     int threadCount = 0;

     ui->tableWidget_processes->clearContents();
     ui->tableWidget_processes->setRowCount(0);

     // Проходим по всем процессам и ищем по PID или имени
     do {

        QString processName = QString::fromWCharArray(_processEntry.szExeFile);
        // Сравниваем имена процессов и/или PID
        if ((processName.compare(value, Qt::CaseInsensitive) == 0) || (isNumeric && _processEntry.th32ProcessID == pid)) {
            isFind=true;


            int row = ui->tableWidget_processes->rowCount();
            ui->tableWidget_processes->insertRow(row);



            QTableWidgetItem* processNameItem = new QTableWidgetItem(QString::fromWCharArray(_processEntry.szExeFile));
            ui->tableWidget_processes->setItem(row,0,processNameItem);

            QTableWidgetItem* processIdItem = new QTableWidgetItem(QString::number(_processEntry.th32ProcessID));
            ui->tableWidget_processes->setItem(row,1,processIdItem);

            QTableWidgetItem* processPidItem = new QTableWidgetItem(QString::number(_processEntry.th32ParentProcessID));
            ui->tableWidget_processes->setItem(row,2,processPidItem);

            QTableWidgetItem* processThreadItem = new QTableWidgetItem(QString::number(_processEntry.cntThreads));
            ui->tableWidget_processes->setItem(row,3,processThreadItem);

            QTableWidgetItem* processPriorityItem = new QTableWidgetItem(determinePriority(_processEntry.pcPriClassBase));
            //QTableWidgetItem* processPriorityItem = new QTableWidgetItem(QString::number(_processEntry.pcPriClassBase));
            ui->tableWidget_processes->setItem(row,4,processPriorityItem);

            HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | READ_CONTROL | PROCESS_ALL_ACCESS, FALSE, _processEntry.th32ProcessID);
            if(hProcess){
                WCHAR filename[MAX_PATH];
                if(GetModuleFileNameEx(hProcess, NULL, filename, MAX_PATH)){
                    QTableWidgetItem* processPathItem = new QTableWidgetItem(QString::fromWCharArray(filename));
                    ui->tableWidget_processes->setItem(row,5,processPathItem);
                }
                else{
                    QMessageBox::critical(this, "Error", QString("QueryFullProcessImageName failed with error %1").arg(GetLastError()));
                }


                CloseHandle(hProcess);
            }
            else{
                //QMessageBox::information(this, "Error", QString("OpenProcess failed with error %1").arg(GetLastError()));
                //continue;
                QTableWidgetItem* processPathItem = new QTableWidgetItem(QString("Нет доступа"));
                ui->tableWidget_processes->setItem(row,5,processPathItem);
            }
            processCount++;
            threadCount += _processEntry.cntThreads;
        }

     } while (Process32Next(hSnapshot, &_processEntry));

     // Закрываем snapshot
     CloseHandle(hSnapshot);

     ui->statusbar->showMessage(QString("Процессы: %1, Потоки: %2").arg(processCount).arg(threadCount));
     if(isFind == false){
        updateProcessTable();
        QMessageBox::information(this,"Процесс не найден","Такого процесса в системе нет");
        ui->lineEdit_searchField->clear();
     }
}


//void MainWindow::on_pushButton_searchButton_clicked()
//{
//     bool isFind = false;
//     QString value = ui->lineEdit_searchField->text();

//     // Создаем snapshot всех процессов в системе
//     HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

//     if (hSnapshot == INVALID_HANDLE_VALUE) {
//        QMessageBox::critical(this,"Error of Snapshot","Error of Snapshot");
//        return;
//     }

//     bool isNumeric;
//     // Пробуем преобразовать введенное значение в число (PID)
//     int pid = value.toInt(&isNumeric);
//     updateTableTimer->stop();


//     int processCount = 0;
//     int threadCount = 0;

//     ui->tableWidget_processes->clearContents();
//     ui->tableWidget_processes->setRowCount(0);
//     // Инициализируем структуру PROCESSENTRY32
//     PROCESSENTRY32 _processEntry;
//     _processEntry.dwSize = sizeof(PROCESSENTRY32);

//     // Получаем первый процесс
//     if (Process32First(hSnapshot, &_processEntry)) {
//        // Проходим по всем процессам и ищем по PID или имени
//        do {

//            QString processName = QString::fromWCharArray(_processEntry.szExeFile);
//            // Сравниваем имена процессов и/или PID
//            if ((processName.compare(value, Qt::CaseInsensitive) == 0) || (isNumeric && _processEntry.th32ProcessID == pid)) {
//                isFind=true;


//                int row = ui->tableWidget_processes->rowCount();
//                ui->tableWidget_processes->insertRow(row);



//                QTableWidgetItem* processNameItem = new QTableWidgetItem(QString::fromWCharArray(_processEntry.szExeFile));
//                ui->tableWidget_processes->setItem(row,0,processNameItem);

//                QTableWidgetItem* processIdItem = new QTableWidgetItem(QString::number(_processEntry.th32ProcessID));
//                ui->tableWidget_processes->setItem(row,1,processIdItem);

//                QTableWidgetItem* processPidItem = new QTableWidgetItem(QString::number(_processEntry.th32ParentProcessID));
//                ui->tableWidget_processes->setItem(row,2,processPidItem);

//                QTableWidgetItem* processThreadItem = new QTableWidgetItem(QString::number(_processEntry.cntThreads));
//                ui->tableWidget_processes->setItem(row,3,processThreadItem);

//                QTableWidgetItem* processPriorityItem = new QTableWidgetItem(determinePriority(_processEntry.pcPriClassBase));
//                //QTableWidgetItem* processPriorityItem = new QTableWidgetItem(QString::number(_processEntry.pcPriClassBase));
//                ui->tableWidget_processes->setItem(row,4,processPriorityItem);

//                HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | READ_CONTROL | PROCESS_ALL_ACCESS, FALSE, _processEntry.th32ProcessID);
//                if(hProcess){
//                    WCHAR filename[MAX_PATH];
//                    if(GetModuleFileNameEx(hProcess, NULL, filename, MAX_PATH)){
//                        QTableWidgetItem* processPathItem = new QTableWidgetItem(QString::fromWCharArray(filename));
//                        ui->tableWidget_processes->setItem(row,5,processPathItem);
//                    }
//                    else{
//                        QMessageBox::critical(this, "Error", QString("QueryFullProcessImageName failed with error %1").arg(GetLastError()));
//                    }


//                    CloseHandle(hProcess);
//                }
//                else{
//                    //QMessageBox::information(this, "Error", QString("OpenProcess failed with error %1").arg(GetLastError()));
//                    //continue;
//                    QTableWidgetItem* processPathItem = new QTableWidgetItem(QString("Нет доступа"));
//                    ui->tableWidget_processes->setItem(row,5,processPathItem);
//                }
//                processCount++;
//                threadCount += _processEntry.cntThreads;
//            }

//        } while (Process32Next(hSnapshot, &_processEntry));
//     }

//     // Закрываем snapshot
//     CloseHandle(hSnapshot);

//     ui->statusbar->showMessage(QString("Процессы: %1, Потоки: %2").arg(processCount).arg(threadCount));
//     if(isFind == false){
//        updateProcessTable();
//        QMessageBox::information(this,"Процесс не найден","Такого процесса в системе нет");
//        ui->lineEdit_searchField->clear();
//     }
//}


void MainWindow::on_tableWidget_processes_itemSelectionChanged()
{
     ui->pushButton_terminateProcess->setEnabled(true);
}

double MainWindow::GetCPULoad(){
     FILETIME idleTime, kernelTime, userTime;
     if (GetSystemTimes(&idleTime, &kernelTime, &userTime))
     {
        static double previousTotalTime = 0;
        static double previousIdleTime = 0;

        ULONGLONG currentTime = (static_cast<ULONGLONG>(userTime.dwLowDateTime) +
                                 static_cast<ULONGLONG>(userTime.dwHighDateTime) +
                                 static_cast<ULONGLONG>(kernelTime.dwLowDateTime) +
                                 static_cast<ULONGLONG>(kernelTime.dwHighDateTime));

        ULONGLONG currentIdleTime = static_cast<ULONGLONG>(idleTime.dwLowDateTime) +
                                    static_cast<ULONGLONG>(idleTime.dwHighDateTime);

        ULONGLONG totalElapsedTime = currentTime - previousTotalTime;
        ULONGLONG idleElapsedTime = currentIdleTime - previousIdleTime;

        double cpuLoad = 100.0 - ((static_cast<double>(idleElapsedTime) / totalElapsedTime) * 100.0);

        previousTotalTime = currentTime;
        previousIdleTime = currentIdleTime;

        return 4*cpuLoad;
     }
     return 0.0; // В случае ошибки
}

//void MainWindow::updateCPULoadLabel()
//{
//        int cpuLoad =(int)GetCPULoad();
//        if (cpuLoad > 100)
//        {
//            cpuLoad=100;
//        }
//        else if(cpuLoad<0)
//        {
//            cpuLoad=0;
//        }


//     ui->labelCPUload->setText(QString("CPU usage : %1 %").arg(QString::number(cpuLoad)));

//}

void MainWindow::updateCPULoadLabel()
{
     int cpuLoad = (int)GetCPULoad();

     // Ограничиваем значения cpuLoad от 0 до 100
     cpuLoad = qBound(0, cpuLoad, 100);

     QString text = QString("CPU usage: %1 %").arg(QString::number(cpuLoad));
     ui->labelCPUload->setText(text);

     // Определяем цвет в зависимости от значения процента CPU
     QColor textColor;
     if (cpuLoad <= 50) {
        textColor = QColor(75, 126, 53);
     } else if (cpuLoad <= 80) {
        textColor = QColor(204, 163, 0);
     } else {
        textColor = QColor(131, 0, 0);
     }

     // Устанавливаем цвет текста для label
     QString styleSheet = QString("QLabel { color: %1; }").arg(textColor.name());
     ui->labelCPUload->setStyleSheet(styleSheet);
}

