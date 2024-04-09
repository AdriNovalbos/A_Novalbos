#include "fpimage.h"
#include "ui_fpimage.h"

#include <QFileDialog>
#include <QPainter>
#include <math.h>

//--------------------------------------------------
//-- Filtro de eventos para capturar mouse clicks --
//--------------------------------------------------
bool FPImage::eventFilter(QObject *Ob, QEvent *Ev)
{
    // Comprobamos que el evento capturado es un  mouseclick
    if(Ev->type()==QEvent::MouseButtonPress) {
        // Comprobamos que el click ocurrió sobre nuestro QLabel
        if(Ob==ui->Ecran) {
            // Hacemos un cast del evento para poder acceder a sus propiedades
            const QMouseEvent *me=static_cast<const QMouseEvent *>(Ev);
            // Nos interesan las coordenadas del click
            int yc=me->y(), xc=me->x();
            // Si estamos fuera de la imagen, nos vamos
            if(yc>=H||xc>=W) return true;
            // Hacemos algo con las coordenadas y el píxel
            statusBar()->showMessage(QString::number(xc)+":"+
                                      QString::number(yc)+" "+
                                      QString::number(pixR[(yc*S+3*xc)])+":"+
                                      QString::number(pixG[(yc*S+3*xc)])+":"+
                                      QString::number(pixB[(yc*S+3*xc)]));


            memset(histoLocalR,0,256*sizeof(int));//pone todo a 0 en histoR[i]
            memset(histoLocalG,0,256*sizeof(int));
            memset(histoLocalB,0,256*sizeof(int));


            for(int y=yc-10;y<=yc+10;y++) { //el padding hace que se salte los ceros de relleno del final de cada fila e y son las filas
                for(int x=xc-10;x<=xc+10;x++) {


                    histoLocalR[pixR[y*S+3*x]]++;
                    histoLocalG[pixG[y*S+3*x]]++;
                    histoLocalB[pixB[y*S+3*x]]++;



                }
            }
            //dibujar
            QPainter p1(&Dib1);
            QPainter p2(&Dib2);
            QPainter p3(&Dib3);
            p1.setPen(QColor(255,129,6));
            p2.setPen(QColor(69,255,34));
            p3.setPen(QColor(20,100,255));
            int maxR = 0;
            int maxG = 0;
            int maxB = 0;

            for (int n=0;n<256;n++){
                if(histoLocalR[n]>maxR) maxR= histoLocalR[n];
                if(histoLocalG[n]>maxG) maxG= histoLocalG[n];
                if(histoLocalB[n]>maxB) maxB= histoLocalB[n];
            }
            Dib1.fill((Qt::white));
            Dib3.fill((Qt::white));
            Dib2.fill((Qt::white));

            for (int l=0;l<256;l++){
                //ui->ERes->appendPlainText("R:"+QString::number(histoR[l])+"  G:"+QString::number(histoG[l])+"  B:"+QString::number(histoB[l]));

                p1.drawLine(l,100,l,100- histoLocalR[l]*100/maxR);


                p2.drawLine(l,100,l,100-histoLocalG[l]*100/maxG);


                p3.drawLine(l,100,l,100-histoLocalB[l]*100/maxB);

            }
            ui->EcranHistoR->setPixmap(Dib1);
            ui->EcranHistoG->setPixmap(Dib2);
            ui->EcranHistoB->setPixmap(Dib3);






            //21x21
            // Devolvemos un "true" que significa que hemos gestionado el evento
            return true;
        } else return false;  // No era para nosotros, lo dejamos en paz
    } else return false;
}

//-------------------------------------------------
//-- Constructor: Conexiones e inicializaciones ---
//-------------------------------------------------
FPImage::FPImage(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::FPImage)
{
    ui->setupUi(this);

    // CONEXIONES de nuestros objetos (botones, etc...) a nuestros slots
    connect(ui->BLoad,SIGNAL(clicked()),this,SLOT(Load()));
    connect(ui->BDoIt,SIGNAL(clicked()),this,SLOT(DoIt()));


    // "Instalamos" un "filtro de eventos" en nuestro QLabel Ecran
    // para capturar clicks de ratón sobre la imagen
    ui->Ecran->installEventFilter(this);


    // INICIALIZACIONES
    W=H=0;      // Empezamos sin imagen cargada ancho y alto de la imagen
    Path="..";  // Carpeta inicial
    old_value=0;
    pixOri = NULL;
    brillo = 0;
    contraste = 45;
    rango=0;




    // Inicializamos a negro los lienzos (QPixmap) y los asociamos a las "pantallas" gráficas (QLabel)
    //   Le damos tamaño
    Dib1=QPixmap(256,100);
    //   Lo pintamos de negro
    Dib1.fill(Qt::white);
    //   Lo asignamos a un QLabel
    ui->EcranHistoR->setPixmap(Dib1);

    //   Idem
    Dib2=QPixmap(256,100);
    Dib2.fill(Qt::white);
    ui->EcranHistoG->setPixmap(Dib2);

    // De ídem
    Dib3=QPixmap(256,100);
    Dib3.fill(Qt::white);

/*
    // Ejemplo de cómo dibujar usando funciones de alto nivel (QPainter)
    //   Declaramos un pintor (QPainter) y lo asociamos a un lienzo (QPixmap)
    QPainter p(&Dib3);
    //   Escogemos un lápiz (QPen; también hay pinceles, QBrush, para los rellenos)
    p.setPen(QPen(Qt::yellow));
    //   Trazamos un par de líneas, por ejemplo
    p.drawLine(0,0,255,99);
    p.drawLine(0,99,255,0);
*/

    ui->EcranHistoB->setPixmap(Dib3);
}

//-------------------------------------------------
//------ Destructor: Limpieza antes de salir ------
//-------------------------------------------------
FPImage::~FPImage()
{
    delete ui;
}

//-------------------------------------------------
//----------- Carga una imagen de disco -----------
//-------------------------------------------------
void FPImage::Load(void)
{
    // Permite al usuario escoger un fichero de imagen
    QString file=QFileDialog::getOpenFileName(this,tr("Abrir imagen"),Path,tr("Image Files (*.png *.jpg *.bmp)"));
    // Si no escogió nada, nos vamos
    if(file.isEmpty()) return;

    // Creamos un QFileInfo para manipular cómodamente el nombre del fichero a efectos informativos
    // Por ejemplo deshacernos del path para que el nombre no ocupe demasiado
    QFileInfo finfo(file);
    // Memorizamos la carpeta usando la variable global Path, para la próxima vez
    Path=finfo.path();
    // Ponemos el nombre del fichero en el recuadro de texto
    ui->EFile->setText(finfo.fileName());
    // Decoración: Añadimos el nombre del fichero al título de la ventana
    setWindowTitle("FPImage v0.1b - "+finfo.fileName());

    // Cargamos la imagen a nuestra variable "Image" usando la función apropiada de la clase QImage
    Image.load(file);
    // Convertimos a RGB (eliminamos el canal A)
    Image=Image.convertToFormat(QImage::Format_RGB888);

    // Almacenamos las dimensiones de la imagen
    W=Image.width();
    H=Image.height();

    // Ponemos nuestros punteros apuntando a cada canal del primer píxel
    pixB=(pixG=(pixR=Image.bits())+1)+1;

    // Ojo! La imagen puede llevar "relleno" ("zero padding"):

    // Longitud en bytes de cada línea incluyendo el padding
    S=Image.bytesPerLine();
    // Padding (número de zeros añadidos al final de cada línea)
    Padding=S-3*W;//quitamos el relleno que se puede haber añadido al moverse por los buses
    if(pixOri) delete []pixOri;
    pixOri=new uchar[S*H];
    memcpy(pixOri,pixR,S*H);

    // Mostramos algo de texto informativo
    ui->ERes->appendPlainText("Loaded "+finfo.fileName());
    ui->ERes->appendPlainText("Size "+QString::number(W)+"x"+QString::number(H));
    ui->ERes->appendPlainText("Padded length "+QString::number(S));
    ui->ERes->appendPlainText("Pad "+QString::number(Padding));
    ui->ERes->appendPlainText("");

    // Ponemos algo en la barra de estado durante 2 segundos
    statusBar()->showMessage("Loaded.",2000);

    // Ajustamos el tamaño de la "pantalla" al de la imagen
    ui->Ecran->setFixedWidth(W);
    ui->Ecran->setFixedHeight(H);

    // Volcamos la imagen a pantalla
    ShowIt();

    Histograma();
}

//-------------------------------------------------
//------------- Jugamos con la imagen -------------
//-------------------------------------------------
void FPImage::DoIt(void)
{
    // Nos aseguramos de que hay una imagen cargada
    if(!H) return;

    // Ejemplo de procesamiento A BAJO NIVEL
    //   Recorremos toda la imagen manipulando los píxeles uno a uno
    //   Atención a los límites y a los saltos de 3 en 3 (3 canales)
    for(int y=0,i=0;y<H;y++,i+=Padding) { //el padding hace que se salte los ceros de relleno del final de cada fila e y son las filas
        for(int x=0;x<W;x++,i+=3) { //x lo compara con el ancho max(columnas) para ir cogiendo cada pixel
            if(pixR[i]<pixG[i]+30){

                if(((x/50)%2==0 && (y/50)%2==0) ||((x/50)%2!=0 && (y/50)%2!=0) ){
                    pixR[i]=pixG[i]=pixB[i]=255;

                }else{
                    pixR[i]=pixG[i]=pixB[i]=0;
                }





            }
        }
    }

    // Sacamos algo de texto informativo
    ui->ERes->appendPlainText("Did it");

    // Ponemos algo en la barra de estado durante 2 segundos
    statusBar()->showMessage("Did it.",2000);

    // Volcamos la imagen a pantalla
    // OJO: Si os olvidáis de esto, la imagen en pantalla no refleja los cambios y
    // pensaréis que no habéis hecho nada, pero Image e Ima (que son la misma) sí
    // que han cambiado aunqu eno lo veáis
    ShowIt();
}

//-------------------------------------------------
//-------------- Mostramos la imagen --------------
//-------------------------------------------------
inline void FPImage::ShowIt(void)
{
    // Creamos un lienzo (QPixmap) a partir de la QImage
    // y lo asignamos a la QLabel central
    ui->Ecran->setPixmap(QPixmap::fromImage(Image));
}

void FPImage::on_Brillo_valueChanged(int value)
{
    brillo = value;
    Brillo_Contraste();

}


void FPImage::on_Contraste_valueChanged(int value)//es hacer que los valores oscuros sean mas oscuros y los claros mas claros
{

    contraste = tan(value*M_PI/180);
    Brillo_Contraste();




}

void FPImage::Brillo_Contraste(){
    memcpy(pixR,pixOri,S*H);

    int b=127-contraste*127;
    uchar tabla[256];
    for(int i= 0; i<256; i++){
        int a = contraste*i+b+brillo;

        if(a>255)tabla[i]=255;
        else if(a<0)tabla[i]=0;
        else{

            tabla[i] = a;

        }
    }



    for(int y=0,i=0;y<H;y++,i+=Padding) { //el padding hace que se salte los ceros de relleno del final de cada fila e y son las filas
        for(int x=0;x<W;x++,i+=3) {


            //int opR =contraste*pixR[i]+b+brillo;

            //int opG =contraste*pixG[i]+b+brillo;

            //int opB =contraste*pixB[i]+b+brillo;



            pixR[i]= tabla[pixR[i]];

            pixG[i]=tabla[pixG[i]];

            pixB[i] = tabla[pixB[i]];
        }
    }
    ShowIt();
    Histograma();
}



void FPImage::on_negro_valueChanged(int value)
{
    memcpy(pixR,pixOri,S*H);


    if(ui->Norma2->isChecked()){

    rango = value*value;


    for(int y=0,i=0;y<H-1;y++,i+=Padding) { //el padding hace que se salte los ceros de relleno del final de cada fila e y son las filas
        for(int x=0;x<W;x++,i+=3) {

            if((pixR[i+3]-pixR[i])*(pixR[i+3]-pixR[i])+(pixG[i+3]-pixG[i])*(pixG[i+3]-pixG[i])+(pixB[i+3]-pixB[i])*(pixB[i+3]-pixB[i])>rango){
                pixR[i]=0;
                pixG[i]=0;
                pixB[i]=0;
            }else if((pixR[(y+1)*S+3*x]-pixR[i])*(pixR[(y+1)*S+3*x]-pixR[i])+(pixG[(y+1)*S+3*x]-pixG[i])*(pixG[(y+1)*S+3*x]-pixG[i])+(pixB[(y+1)*S+3*x]-pixB[i])*(pixB[(y+1)*S+3*x]-pixB[i])>rango){
                pixR[i]=0;
                pixG[i]=0;
                pixB[i]=0;
            }else{
                pixR[i]=255;
                pixG[i]=255;
                pixB[i]=255;
            }
        }
    }
    }    if(ui->Norma1->isChecked()){

        rango = value;


        for(int y=0,i=0;y<H-1;y++,i+=Padding) { //el padding hace que se salte los ceros de relleno del final de cada fila e y son las filas
            for(int x=0;x<W;x++,i+=3) {

                if(abs(pixR[i+3]-pixR[i])+abs(pixG[i+3]-pixG[i])+abs(pixB[i+3]-pixB[i])>rango){
                    pixR[i]=0;
                    pixG[i]=0;
                    pixB[i]=0;
                }else if(abs(pixR[(y+1)*S+3*x]-pixR[i])+abs(pixG[(y+1)*S+3*x]-pixG[i])+abs(pixB[(y+1)*S+3*x]-pixB[i])>rango){
                    pixR[i]=0;
                    pixG[i]=0;
                    pixB[i]=0;
                }else{
                    pixR[i]=255;
                    pixG[i]=255;
                    pixB[i]=255;

                }
            }
        }
    }


    if(ui->Normainf->isChecked()){

        rango = value;


        for(int y=0,i=0;y<H-1;y++,i+=Padding) { //el padding hace que se salte los ceros de relleno del final de cada fila e y son las filas
            for(int x=0;x<W;x++,i+=3) {

                if(std::max({(pixR[i+3]-pixR[i]),(pixG[i+3]-pixG[i]),(pixB[i+3]-pixB[i])})>rango){
                    pixR[i]=0;
                    pixG[i]=0;
                    pixB[i]=0;
                }else if(std::max({(pixR[(y+1)*S+3*x]-pixR[i]),(pixG[(y+1)*S+3*x]-pixG[i]),(pixB[(y+1)*S+3*x]-pixB[i])})>rango){
                    pixR[i]=0;
                    pixG[i]=0;
                    pixB[i]=0;
                }else{
                    pixR[i]=255;
                    pixG[i]=255;
                    pixB[i]=255;

                }
            }
        }
    }

    if(ui->NormaChroma->isChecked()){
            rango = value*value;
        double rang = rango/1000000.0;

        for(int y=0,i=0;y<H-1;y++,i+=Padding) { //el padding hace que se salte los ceros de relleno del final de cada fila e y son las filas
            for(int j=0;j<W;j++,i+=3) {

                double X = pixR[i]*0.412+pixG[i]*0.357+pixB[i]*0.180;
                double Y = pixR[i]*0.212+pixG[i]*0.715+pixB[i]*0.072;
                double Z = pixR[i]*0.019+pixG[i]*0.119+pixB[i]*0.950;

                double X1 = pixR[i+3]*0.412+pixG[i+3]*0.357+pixB[i+3]*0.180;
                double Y1 = pixR[i+3]*0.212+pixG[i+3]*0.715+pixB[i+3]*0.072;
                double Z1 = pixR[i+3]*0.019+pixG[i+3]*0.119+pixB[i+3]*0.950;

                double X2 = pixR[(y+1)*S+3*j]*0.412+pixG[(y+1)*S+3*j]*0.357+pixB[(y+1)*S+3*j]*0.180;
                double Y2 = pixR[(y+1)*S+3*j]*0.212+pixG[(y+1)*S+3*j]*0.715+pixB[(y+1)*S+3*j]*0.072;
                double Z2 = pixR[(y+1)*S+3*j]*0.019+pixG[(y+1)*S+3*j]*0.119+pixB[(y+1)*S+3*j]*0.950;

                double x0= X / (X+Y+Z);
                double y0= Y / (X+Y+Z);

                double x1= X1 / (X1+Y1+Z1);
                double y1= Y1 / (X1+Y1+Z1);

                double x2= X2 / (X2+Y2+Z2);
                double y2= Y2 / (X2+Y2+Z2);

                if((x1-x0)*(x1-x0)+(y1-y0)*(y1-y0)>rang){
                    pixR[i]=0;
                    pixG[i]=0;
                    pixB[i]=0;
                }else if((x2-x0)*(x2-x0)+(y2-y0)*(y2-y0)>rang){
                    pixR[i]=0;
                    pixG[i]=0;
                    pixB[i]=0;
                }else{
                    pixR[i]=255;
                    pixG[i]=255;
                    pixB[i]=255;

                }
            }
        }

    }
    if(ui->ProporcionChroma->isChecked()){
        rango = value;



        double rang = rango/1000.0;

        for(int y=0,i=0;y<H-1;y++,i+=Padding) { //el padding hace que se salte los ceros de relleno del final de cada fila e y son las filas
            for(int j=0;j<W;j++,i+=3) {

                double escalarL = (pixR[i]*pixR[i+3]+pixG[i]*pixG[i+3]+pixB[i]*pixB[i+3]);
                 double escalarB = (pixR[i]*pixR[(y+1)*S+3*j]+pixG[i]*pixG[(y+1)*S+3*j]+pixB[i]*pixB[(y+1)*S+3*j]);

                double M1 =sqrt(pixR[i]*pixR[i]+pixG[i]*pixG[i]+pixB[i]*pixB[i]);
                double M2 =sqrt(pixR[i+3]*pixR[i+3]+pixG[i+3]*pixG[i+3]+pixB[i+3]*pixB[i+3]);
                double M3 =sqrt(pixR[(y+1)*S+3*j]*pixR[(y+1)*S+3*j]+pixG[(y+1)*S+3*j]*pixG[(y+1)*S+3*j]+pixB[(y+1)*S+3*j]*pixB[(y+1)*S+3*j]);

                double sol1 = escalarL/(M1*M2);
                double sol2 = escalarB/(M1*M3);
                if(sol1<rang||sol2<rang){
                    pixR[i]=0;
                    pixG[i]=0;
                    pixB[i]=0;

                }else{
                    pixR[i]=255;
                    pixG[i]=255;
                    pixB[i]=255;

                }
            }
        }
    }

    ShowIt();
    Histograma();

}










void FPImage::on_ProporcionChroma_toggled(bool checked)
{
    if(checked){

        ui->negro->setMaximum(1000);
         ui->negro->setMinimum(980);

    }
    else{

        ui->negro->setMaximum(50);
        ui->negro->setMinimum(0);
    }


}

void FPImage::Histograma(){
     //calcular
    memset(histoR,0,256*sizeof(int));//pone todo a 0 en histoR[i]
    memset(histoG,0,256*sizeof(int));
    memset(histoB,0,256*sizeof(int));

    for(int y=0,i=0;y<H;y++,i+=Padding) { //el padding hace que se salte los ceros de relleno del final de cada fila e y son las filas
        for(int x=0;x<W;x++,i+=3) {
            histoR[pixR[i]]++;
            histoG[pixG[i]]++;
            histoB[pixB[i]]++;
        }
    }


    //dibujar
    QPainter p1(&Dib1);
    QPainter p2(&Dib2);
    QPainter p3(&Dib3);
    p1.setPen(QColor(255,129,6));
    p2.setPen(QColor(69,255,34));
    p3.setPen(QColor(20,100,255));
    int maxR = 0;
    int maxG = 0;
    int maxB = 0;

    for (int n=0;n<256;n++){
        if(histoR[n]>maxR) maxR= histoR[n];
        if(histoG[n]>maxG) maxG= histoG[n];
        if(histoB[n]>maxB) maxB= histoB[n];
    }
    Dib1.fill((Qt::white));
    Dib3.fill((Qt::white));
    Dib2.fill((Qt::white));

    for (int l=0;l<256;l++){
        //ui->ERes->appendPlainText("R:"+QString::number(histoR[l])+"  G:"+QString::number(histoG[l])+"  B:"+QString::number(histoB[l]));

         p1.drawLine(l,100,l,100- histoR[l]*100/maxR);


         p2.drawLine(l,100,l,100-histoG[l]*100/maxG);


        p3.drawLine(l,100,l,100-histoB[l]*100/maxB);

    }
    ui->EcranHistoR->setPixmap(Dib1);
    ui->EcranHistoG->setPixmap(Dib2);
    ui->EcranHistoB->setPixmap(Dib3);
     ui->ERes->appendPlainText("R:"+QString::number(maxR)+"  G:"+QString::number(maxG)+"  B:"+QString::number(maxB));

}


void FPImage::on_STRETCH_clicked(){





    Histograma();

    int MB =256;
    int MG =256;
    int MR =256;

    while (histoR[--MR]<W*H*0.0001);
    while (histoG[--MG]<W*H*0.0001);
    while (histoB[--MB]<W*H*0.0001);

    int mr = 0;
    int mg = 0;
    int mb = 0;
    while(histoR[mr]<W*H*0.0001) mr++;
    while(histoG[mg]<W*H*0.0001) mg++;
    while(histoB[mb]<W*H*0.0001) mb++;

    uchar tablaR[256];
    uchar tablaG[256];
    uchar tablaB[256];

    for(int i= 0; i<256; i++){
        int a = 255*(i-mr)/(MR-mr);
        int b = 255*(i-mg)/(MG-mg);
        int c = 255*(i-mb)/(MB-mb);

        if(a>255)tablaR[i]=255;
        else if(a<0)tablaR[i]=0;
        else{

            tablaR[i] = a;

        }
        if(b>255)tablaG[i]=255;
        else if(b<0)tablaG[i]=0;
        else{

            tablaG[i] = b;

        }
        if(c>255)tablaB[i]=255;
        else if(c<0)tablaB[i]=0;
        else{

            tablaB[i] = c;

        }
    }

    for(int y=0,i=0;y<H;y++,i+=Padding) {
        for(int x=0;x<W;x++,i+=3) {

            pixR[i]= tablaR[pixR[i]];

            pixG[i]= tablaG[pixG[i]];

            pixB[i]= tablaB[pixB[i]];

        }
    }
    //ui->ERes->appendPlainText("R:"+QString::number(pixR[2])+"  G:"+QString::number(pixG[2])+"  B:"+QString::number(pixB[2]));

    ui->ERes->appendPlainText("R:"+QString::number(mr)+"  G:"+QString::number(mg)+"  B:"+QString::number(mb));
    ui->ERes->appendPlainText("R:"+QString::number(MR)+"  G:"+QString::number(MG)+"  B:"+QString::number(MB));

    ShowIt();
    Histograma();



//CalculR PIXR G Y B, LLAMAR A SHOWIT E HISTOGRAMA


}


void FPImage::on_eq_clicked()
{


    Histograma();



    int HacR[256];
    int HacG[256];
    int HacB[256];
    uchar LUTR[256];
    uchar LUTG[256];
    uchar LUTB[256];
    memset(HacR,0,256*sizeof(int));
    memset(HacG,0,256*sizeof(int));
    memset(HacB,0,256*sizeof(int));





    for(int i=0; i<256; i++){


        HacR[i]= histoR[i]+HacR[i-1];
        HacG[i]= histoG[i]+HacG[i-1];
        HacB[i]= histoB[i]+HacB[i-1];

        LUTR[i]= 255* HacR[i]/(W*H);
        LUTG[i]= 255* HacG[i]/(W*H);
        LUTB[i]= 255* HacB[i]/(W*H);

    }


    for(int y=0,i=0;y<H;y++,i+=Padding) {
        for(int x=0;x<W;x++,i+=3) {

            pixR[i]= LUTR[pixR[i]];

            pixG[i]= LUTG[pixG[i]];

            pixB[i]= LUTB[pixB[i]];

        }
    }

    Histograma();
    ShowIt();




}


void FPImage::on_pushButton_clicked()
{
    memcpy(pixR,pixOri,S*H);

    int histoLocal2R[256];
    int histoLocal2B[256];
    int histoLocal2G[256];

    int rangoHisto= ui->valueHisto->value()*10;

    for(int a=10;a<H-10;a++) {
        for(int b=10;b<W-10;b++) {

            memset(histoLocal2R,0,256*sizeof(int));//pone todo a 0 en histoR[i]
            memset(histoLocal2G,0,256*sizeof(int));
            memset(histoLocal2B,0,256*sizeof(int));

             for(int y=b+10;y<=b-10;y++) { //el padding hace que se salte los ceros de relleno del final de cada fila e y son las filas
                for(int x=a+10;x<=a-10;x++) {


                    histoLocal2R[pixOri[y*S+3*x]]++;
                    histoLocal2G[pixOri[y*S+3*x+1]]++;
                    histoLocal2B[pixOri[y*S+3*x+2]]++;


                }
            }
             float distanciaR=0;
             float distanciaG=0;
             float distanciaB=0;
             float res=0;


             for(int z=0;z<256;z++){
                distanciaR=abs(histoLocalR[z]-histoLocal2R[z]);
                distanciaG=abs(histoLocalG[z]-histoLocal2G[z]);
                distanciaB=abs(histoLocalB[z]-histoLocal2B[z]);
                res= (distanciaR+distanciaG+distanciaB);

                if(res>rangoHisto){

                    pixR[a*S+3*b]=0;
                    pixG[a*S+3*b]=0;
                    pixB[a*S+3*b]=0;



                }
             }

        }
    }

    ShowIt();


}


void FPImage::on_valueHisto_valueChanged(int value)
{





}

