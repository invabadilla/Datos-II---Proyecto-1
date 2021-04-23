#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "Compiler.h"
#include <string>
#include <iostream>
#include <thread>
int i_line = 0;

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
        QString line = Qlines.at(i_line);
        i_line++;
        Compiler *compiler = new Compiler();
        compiler->compile(line);
        this->UpdateGUI();
        if (i_line >= Qlines.count()){
            i_line = 0;
            this->ui->textEdit->setEnabled(true);
            this->Clear();

        }

    }
}
void MainWindow::UpdateGUI() {
    Compiler *compiler = new Compiler();
    std::string info = compiler->updateGUI();
    QString Qinfo = QString::fromStdString(info);
    QStringList Qlistinfo = Qinfo.split("-", QString::SkipEmptyParts);
    QString std_out = Qlistinfo.at(0);
    QString log = Qlistinfo.at(1);
    QString ram = Qlistinfo.at(2);
    this->ui->textEdit_2->setText(std_out);
    this->ui->textEdit_3->setText(log);
    this->ui->textEdit_4->setText(ram);
}

void MainWindow::Clear() {
    Compiler *compiler = new Compiler();
    compiler->s_existing.clear();
    compiler->std_out = "<< \n";
    compiler->log = "<< \n";
    compiler->ram = "<< \n";
    compiler->compile("cancel");
}


void MainWindow::on_pushButton_2_clicked()
{
    i_line = 0;
    this->ui->textEdit->setEnabled(true);
    this->Clear();
}
