#ifndef FPIMAGE_H
#define FPIMAGE_H



#include <QMainWindow>
#include <QMessageBox>

namespace Ui {
class FPImage;
}

class FPImage : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit FPImage(QWidget *parent = 0);
    ~FPImage();
    
private:
    Ui::FPImage *ui;

    QString Path;               // Para recordar la carpeta al cargar imágenes
    QImage Image;               // Imagen Qt
    uchar *pixR, *pixG, *pixB;  // Punteros a los tres canales R, G y B
    int W, H;                   // Tamaño de la imagen actual
    int S, Padding;                // Step en imágenes no continuas
    int old_value;
    float contraste;
    int brillo;
    int rango;
    uchar *pixOri;
    int histoR[256];
    int histoG[256];
    int histoB[256];
    int histoLocalR[256];
    int histoLocalB[256];
    int histoLocalG[256];

    QPixmap Dib1, Dib2, Dib3;   // Tres lienzos en los que dibujar

    void ShowIt(void);          // Muestra la imagen actual

    bool eventFilter(QObject *Ob, QEvent *Ev);  // Un "filtro de eventos"
    void Brillo_Contraste();

private slots:
    void Load(void);    // Slot para el botón de carga de imagen

    void DoIt(void);    // Slot para el botón de hacer algo con la imagen

    void on_Brillo_valueChanged(int value);

    void on_Contraste_valueChanged(int value);

    void on_negro_valueChanged(int value);

    void on_ProporcionChroma_toggled(bool checked);

    void Histograma();



    void on_STRETCH_clicked();
    void on_eq_clicked();
    void on_pushButton_clicked();
    void on_valueHisto_valueChanged(int value);
};

#endif // FPIMAGE_H
