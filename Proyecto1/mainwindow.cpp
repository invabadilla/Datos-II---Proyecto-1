#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "Compiler.h"
#include <string>
#include <iostream>



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
    QString Qline = this->ui->textEdit->toPlainText();
    std::string line = Qline.toStdString();
    Compiler *compiler = new Compiler();
    compiler->compile(line);
}
