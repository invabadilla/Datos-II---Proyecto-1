#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "Compiler.h"
#include <string>
#include <iostream>
#include <thread>

/**
 *Linea actual donde se ubica el compilador en tiempo real
 */
int i_line = 0;

/**
 * Constructor de la interface para el GUI
 * @param parent Heredacion de QMainWindow
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

/**
 * Destructor de la interface para el GUI
 */
MainWindow::~MainWindow()
{
    delete ui;
}

/**
 * Metodo para la ejecucion del RUN
 */
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

/**
 * Metodo para la actualizacion del GUI con la informacion del
 * STD_OUT, el LOG y la RAM
 */
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
    QStringList logParts = log.split("\n");
    for(int i=0; i < logParts.length(); i++){
        if(logParts.at(i).split("ERROR").length() == 2 || logParts.at(i).split("FATAL").length() == 2 ){
            this->ui->textEdit->setEnabled(true);
            Compiler *compiler = new Compiler();
            compiler->compile("y!@#$%^&*(");
            compiler->s_existing.clear();
            compiler->std_out = "<< \n";
            compiler->log = "<< \n";
            compiler->ram = "<< \n";
            i_line = 0;
            break;
        }
    }
}

/**
 * Metodo para la limpieza de la interfaz
 */
void MainWindow::Clear() {
    Compiler *compiler = new Compiler();
    compiler->compile("y!@#$%^&*(");
    this->UpdateGUI();
    compiler->s_existing.clear();
    compiler->std_out = "<< \n";
    compiler->log = "<< \n";
    compiler->ram = "<< \n";

}

/**
 * Metodo para el boton STOP
 */
void MainWindow::on_pushButton_2_clicked()
{
    i_line = 0;
    this->ui->textEdit->setEnabled(true);
    this->Clear();
}
