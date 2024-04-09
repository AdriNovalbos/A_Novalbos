#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFileDialog>
#include <QPainter>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //connect(ui->LOAD,SIGNAL(clicked()),this,SLOT(Load()));
    Path="..";  // Carpeta inicial
    SAMPLES=90;
    X = 0;
    Y = 0;
    Z = 0;
    R = 0;
    G = 0;
    B = 0;
    LambdaR=450;
    LambdaG=550;
    LambdaB=650;
    UR=0;
    UG=0;
    UB=0;





    Dib=QPixmap(100,100);
    Dib2 = QPixmap(100,100);
    Dib3 = QPixmap(512,100);


    QFile file("C:/Users/rrunn/Documents/curvas.txt");
    if(file.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream parser(&file);
        for(int i=0;i<SAMPLES;i++){
            parser>>WaveCurvas[i]>>Xcie[i]>>Ycie[i]>>Zcie[i];

        }
        file.close();
    } else {
        QMessageBox msgBox;
        msgBox.setText("Couldn't load CMF coeficients.");
        msgBox.exec();
    }
    //ui->INFO->appendPlainText("Valores " + QString::number(Wave[44]) + " " + QString::number(Xcie[44])+ " " + QString::number(Ycie[44]) + " "+ QString::number(Zcie[44]));


}

MainWindow::~MainWindow()
{
    delete ui;
}



void MainWindow::on_LOAD_clicked()
{
    QString fileName=QFileDialog::getOpenFileName(this,tr("Abrir imagen"),Path,tr("Image Files (*.txt)"));
    QFile file(fileName);
    if(file.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream parser(&file);
        for(int u=0;u<2048;u++){
            parser>>Wfichero[u]>>Esp[u];
        }
        file.close();
    } else {
        QMessageBox msgBox;
        msgBox.setText("Couldn't load CMF coeficients.");
        msgBox.exec();
    }
    int j=0; while(Wfichero[j]<380)j++;
    int i = 0;
    X = 0;
    Y = 0;
    Z = 0;




    do{
    if(Wfichero[j]>WaveCurvas[i+1])i++;//aumento el valor de i si j lo supera

    double XcieInt=interpolar(WaveCurvas[i],Xcie[i],WaveCurvas[i+1],Xcie[i+1],Wfichero[j]);//debe ser un array para ir interpolando valor a valor

    double YcieInt = interpolar(WaveCurvas[i],Ycie[i],WaveCurvas[i+1],Ycie[i+1],Wfichero[j]);

    double ZcieInt = interpolar(WaveCurvas[i],Zcie[i],WaveCurvas[i+1],Zcie[i+1],Wfichero[j]);

    X += XcieInt*Esp[j];
    Y += YcieInt*Esp[j];
    Z += ZcieInt*Esp[j];

    }while(Wfichero[++j]<825);
    Reescalar();
    Ver_espectro();




}


void MainWindow::on_imagen1_linkActivated(const QString &link)
{

}


void MainWindow::on_imagen2_linkActivated(const QString &link)
{


}


void MainWindow::on_INFO_blockCountChanged(int newBlockCount)

{

}

double MainWindow::interpolar(double L1, double v1, double L2, double v2, double Lambda){
    return  v1+(Lambda-L1)*(v2-v1)/(L2-L1);
}

void MainWindow::on_Luminancia_valueChanged(int value)
{
    Reescalar();
    Calcular();


}

void MainWindow::Reescalar(){

    double R = 0;
    double G = 0;
    double B = 0;

    int Ytarget = ui->Luminancia->value();
    double XR = X*Ytarget/Y;
    double ZR = Z*Ytarget/Y;

    double YR = Ytarget;



    R = (3.240479*XR)+(-1.537150*YR)+(-0.498535*ZR);
    G = (-0.969256*XR)+(1.875992*YR)+(0.041556*ZR);
    B = (0.055648*XR)+(-0.204043*YR)+(1.057311*ZR);
    if(R<255 && G<255&&B<255){
    Dib.fill(QColor(R,G,B));
    ui->coloReal->setPixmap(Dib);

    // ui->RGB1->appendPlainText("Valores " + QString::number(X[1]) + " " + QString::number(Y[1]));
    ui->RGB1->setText(QString::number(R)+ " " + QString::number(G)+ " "+ QString::number(B));
    }
}






void MainWindow::on_LRed_valueChanged(int value)
{
    LambdaR = value;
    Calcular();
    Ver_espectro();




}


void MainWindow::on_LGreen_valueChanged(int value)
{
    LambdaG = value;
    Calcular();
    Ver_espectro();

}


void MainWindow::on_LBlue_valueChanged(int value)
{
    LambdaB = value;
    Calcular();
    Ver_espectro();


}

void MainWindow:: Calcular(){
    int u = 0;

    while(LambdaR>Wfichero[u]) u++;
    ui->INFO->appendPlainText( QString::number(LambdaR));
    ui->INFO->appendPlainText( QString::number(Wfichero[u]));



    R = Esp[u];
    UR=u;
    ui->INFO->appendPlainText( QString::number(u));
    u=0;

    while(LambdaG>Wfichero[u]) u++;

    G = Esp[u];
    UG=u;

    u=0;

    while(LambdaB>Wfichero[u]) u++;

    B = Esp[u];

    UB=u;

    double Y2 = 0.21*R + 0.756*G + 0.072175*B;
    int Ytarget = ui->Luminancia->value();

    double R2 = R*Ytarget/Y2;
    double G2 = G*Ytarget/Y2;
    double B2 = B*Ytarget/Y2;

     ui->RGB2->setText(QString::number(R2)+ " " + QString::number(G2)+ " "+ QString::number(B2));
    ui->INFO->appendPlainText("Valores " + QString::number(R) + " " + QString::number(G)+ " " + QString::number(B) + " "+ QString::number(Ytarget));
    Dib2.fill(QColor(R2,G2,B2));
    ui->imagen2->setPixmap(Dib2);

}

void MainWindow::on_Espectro_linkActivated(const QString &link)//valor del esp entre el max por 100
{

}
void MainWindow::Ver_espectro(){
    QPainter p(&Dib3);//contrato pintor
    p.setPen(QColor(255,255,255));
    double ValorAct=0;
    int BRed = ((UR)/4);
    int BGreen = (UG)/4;
    int BBlue = (UB)/4;

    for(int i=0;i<2048;i++){ //numero de valores en el fichero
        if(ValorAct<Esp[i]) ValorAct = Esp[i];
    }
     Dib3.fill((Qt::black));
    for(int j=0;j<512;j++){
         if(BRed == j){
            p.setPen(QColor(255,0,0));
            p.drawLine(j,100,j,100-(Esp[j*4]*100)/ValorAct);
         }else if(BGreen == j){
             p.setPen(QColor(0,255,0));
             p.drawLine(j,100,j,100-(Esp[j*4]*100)/ValorAct);

         }else if(BBlue == j){
             p.setPen(QColor(0,0,255));
             p.drawLine(j,100,j,100-(Esp[j*4]*100)/ValorAct);

         }

         else{
        p.setPen(QColor(255,255,255));
        p.drawLine(j,100,j,100-(Esp[j*4]*100)/ValorAct);
         }



        //siguiente clase: llamar función con cambio de sliders/ líneas de diferente color para RGB.
    }

    ui->Espectro->setPixmap(Dib3);
}

//podemos usar XCIEINT ycie y zcie ese valor lo pasamos a rgb y lo representamos para cada punto entre 0 y 512
