#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_LOAD_clicked();


    void on_imagen1_linkActivated(const QString &link);

    void on_imagen2_linkActivated(const QString &link);

    void on_INFO_blockCountChanged(int newBlockCount);


    void on_Luminancia_valueChanged(int value);



    void on_LRed_valueChanged(int value);

    void on_LGreen_valueChanged(int value);

    void on_LBlue_valueChanged(int value);

    void on_Espectro_linkActivated(const QString &link);

private:
    Ui::MainWindow *ui;
    QString Path;
    int SAMPLES;
    double WaveCurvas[91];
    double Xcie[91];
    double Ycie[91];
    double Zcie[91];
    double Wfichero[2048];
    double Esp[2048];
    double v[90];
    double L1[90],v1[90],L2[90],v2[90];
    double interpolar(double L1, double v1, double L2, double v2, double Lambda);
    QPixmap Dib,Dib2,Dib3;
    double X ;
    double Y ;
    double Z;
    double R;
    double G;
    double B;
    double LambdaR;
    double LambdaG;
    double LambdaB;
    int UB;
    int UG;
    int UR;


    void Reescalar();
    void Calcular();
    void Ver_espectro();
    //double interpolacion;




};
#endif // MAINWINDOW_H
