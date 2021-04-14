#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "Compiler.h"
#include <string>
#include <iostream>
#include <thread>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked()
{
    if (this->ui->textEdit->toPlainText() != NULL){
        this->ui->textEdit->setEnabled(false);
        QString Qtext = this->ui->textEdit->toPlainText();
        QStringList Qlines = Qtext.split("\n", QString::SkipEmptyParts);
        static int i_line = 0;
        QString line = Qlines.at(i_line);
        i_line++;
        if (i_line >= Qlines.count()){
            i_line = 0;
            this->ui->textEdit->setEnabled(true);
        }
        Compiler *compiler = new Compiler();
        compiler->compile(line);
        //this->UpdateGUI();
    }
}
/**void MainWindow::UpdateGUI() {
    Compiler *compiler = new Compiler();
    std::string info = compiler->updateGUI();
    QString Qinfo = QString::fromStdString(info);
    QStringList Qlistinfo = Qinfo.split("-", QString::SkipEmptyParts);
    QString std_out = Qlistinfo.at(0);
    QString log = Qlistinfo.at(1);
    QString ram = Qlistinfo.at(2);
}**/




