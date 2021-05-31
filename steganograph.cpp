#include "steganograph.h"
#include "ui_steganograph.h"
#include <QFileDialog>
#include <QMessageBox>
#include <stdio.h>
#include <iostream>
#include <QTextStream>
#include <QtEndian>
#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <QtMath>
#include <opencv>

QTextStream cout(stdout);
QTextStream cin(stdin);

Steganograph::Steganograph(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Steganograph)
{
    ui->setupUi(this);
    //задание фонового цвета
    QColor QColor1(246, 232, 230);
    QPalette Palette1 = ui->tabWidget->palette();
    Palette1.setColor(backgroundRole(), QColor1);
    Palette1.setColor(foregroundRole(), Qt::black);
    ui->tabWidget->setPalette(Palette1);
    //вывод ключа в строку при открытии программы
    ui->InputKeyArea->setText(StringKey);
    ui->CounterSymb->setText("Введено символов: 0");
    ui->tabWidget->setTabText(0, "Встраивание");
    ui->tabWidget->setTabText(1, "Изъятие");
    ui->tabWidget->setTabText(2, "Редактировать ключ");

}

Steganograph::~Steganograph()
{
    delete ui;
}

QBitArray Steganograph::toBitArray(const QByteArray dataByte)
{
    //размер (в битах) переданного массива в функцию
     unsigned int sizeArr = dataByte.size()*8;
    unsigned int index=0;
    //создание нового битного массива
    QBitArray dataBit(sizeArr);
    //побитовое чтение байтового массива
    for (unsigned char Byte : dataByte){
        for (unsigned int Bit = 0; Bit < 8; Bit++){
            if (Byte & 0x01)
                dataBit.setBit(index, true);
            Byte = Byte >> 1;
            index++;
        }
    }
    //возвращение полученного массива бит
    return dataBit;
}

QByteArray Steganograph::toByteArray(const QBitArray dataBit)
{
    unsigned int index = 0;
    //подсчет длины битовой последовательности
    unsigned int countBit = dataBit.count();
    //подсчет длины байтовой последовательности
    unsigned int countByte = (countBit + 7) / 8;
    //создание нового массива байт
    QByteArray byteArr(countByte, 0);
    //побитовый перевод в байты
    for (unsigned int n = 0; n < countBit;){
        unsigned char byte = 0;
        for (unsigned int bit = 0; bit < 8; bit++){
            byte = byte >> 1;
            if (n < countBit)
                if (dataBit[n++])
                    byte |= 0x80;
        }
    byteArr[index++] = byte;
    }
    //возвращение массива
    return byteArr;
}

void Steganograph::createNewImage()
{
    //задание полного имени файла
    QString newPath("/Users/igor/Desktop/Steganography/Images/result.bmp");
    //копирование параметров открытого изображения в новое изображение
    newImg = Img;
    int index = 0;
    int ChangedBit=0;
    //вектор, хранящий цветовые компоненты пикселей в байтах
    QVector<unsigned char> colors(imageSizeBit);
    for (int y = 0; y < height; y++){
        for (int x = 0; x < width; x++){
            //построчное считывание пикселей
            QRgb rgb = Img.pixel(x, y);
            colors[index++] = qRed(rgb);
            colors[index++] = qGreen(rgb);
            colors[index++] = qBlue(rgb);
        }
    }
    //QMessageBox::information(this, tr("Message!"), (QString::number(textSize)));

    index = 0;
    //переменная, хранящая число символов в сообщении в байтах
    QByteArray countSymbByte;
    for(int i = 0; i < 16; i++)
    {
        //перенос значения из переменной textSize
        countSymbByte.append((char)((textSize & (0xFF << (i*8))) >> (i*8)));
    }
    QBitArray countSymbBit = toBitArray(countSymbByte);
    //встраивание два байта с информацией о длине сообщения в младшие биты массива color
    for (int i=0; i<16 && textSize!=0; i++){
        if (!(countSymbBit[i])){
            //если изменяем младший бит, то увеличиваем счётчик на 1
            if (colors[index] & 0x0001)
                ChangedBit++;
            colors[index++] &= 0x00FE;
        }else{
            if (!(colors[index] & 0x0001))
                ChangedBit++;
            colors[index++] |= 0x0001;
        }
    }
    //встраивание сообщения в массив color
    for (unsigned int i = 0; i<imageSizeBit && i<textSize*8; i++){
        if (!(MessageBit[i])){
            if (colors[index] & 0x0001)
                ChangedBit++;
            colors[index++] &= 0x00FE;
        }else{
            if (!(colors[index] & 0x0001))
                ChangedBit++;
            colors[index++] |= 0x0001;
        }
    }
    index=0;
    //перенос данных из массива color в цвета нового изображения
    for ( int y = 0 ; y < height; y++)
        for ( int x = 0 ; x < width; x++)
            {
            int r = colors[index++];
            int g = colors[index++];
            int b = colors[index++];
            //задание цвета нового пикселя
            QRgb rgb = qRgb(r, g, b);
            //привязка пикселя к конкретному месту в изображении
            newImg.setPixelColor(x, y, rgb);
     }
    //сохранение полученного изображения
    newImg.save(newPath);
    //расчёт и вывод PSNR
    //QMessageBox::information(this, tr("Message!"), "ChangeBit: " + QString::number(ChangedBit));
    //QMessageBox::information(this, tr("Message!"), "Width: " + QString::number(width));
    //QMessageBox::information(this, tr("Message!"), "Height: " + QString::number(height));


    PSNR=10*log10(width*height*255*3/(ChangedBit*ChangedBit)*255);
    QMessageBox::information(this, tr("Message!"), "PSNR: " + QString::number(PSNR));
}

void Steganograph::extractTextBit()
{
    int index = 0;
    QVector<unsigned char> colors(imageSizeBit);
    //переменная, хранящая длину встроенного сообщения
    QVector<unsigned char> sizeTextByte(16);
    //считывание цветовых компонент пикселей, содержащих информацию о длине сообщения
    for (int y=0; y < height && index < 16; y++)
    {
        for (int x=0; x < width && index < 16; x++)
        {

                QRgb rgb = Img.pixel(x, y);
                sizeTextByte[index++] = qRed(rgb);
                if (index < 16){
                    sizeTextByte[index++] = qGreen(rgb);
                    sizeTextByte[index++] = qBlue(rgb);
                }
        }
    }
    //массив бит, хранящий число символов скрытого собщения
    QBitArray sizeTextBit(16);
    //Изъятие числа спрятанных символов
    for (int n = 0; n < 16; n++){
        if (sizeTextByte[n] & 0x0001)
            sizeTextBit[n]=true;
        else
            sizeTextBit[n]=false;
    }
    //перевод из битного в байтовую форму
    QByteArray countText = toByteArray(sizeTextBit);
    //число символов
    textSize  = qFromLittleEndian<quint16>((uchar*)countText.data());
    //считывание первых двух байт RGB компонент, хранящих два бита скрытой информации
    index = 0;
    QRgb rgb = Img.pixel(5, 0);
    colors[index++] = qGreen(rgb);
    colors[index++] = qBlue(rgb);
    bool firstCircle = true;
    //считывание остальной части пикселей, хранящих сообщение
    for (int y=0; (y < height) && (index < textSize*8-2); y++)
    {
        for (int x = 0; (x < width) && (index < textSize*8-2); x++)
        {
                if (y==0 && firstCircle==true){ x=6; firstCircle=false; }
            QRgb rgb = Img.pixel(x, y);
            colors[index++] = qRed(rgb);
            colors[index++] = qGreen(rgb);
            colors[index++] = qBlue(rgb);
        }
    }
    //массив бит, хранящий передаваемое сообщение
    QBitArray BitArray(textSize*8);
    for (int i = 0; i < textSize*8; i++){
        if ((colors[i] & 0x0001) == 0x0001){
            BitArray[i]=true;
        }
        else{
            BitArray[i]=false;
        }
    }
   MessageBit = BitArray;
   //вызов функции, расшифровыващей сообщение
   decryptMessage();
   //перевод из бит в байты
   message = toByteArray(MessageBit);
   //стирание лишних вспомогательных символов
   int sizeMessage=message.size()-1;
   while (message.endsWith('!')){
       message=message.left(sizeMessage);
       sizeMessage--;
   }
   //вывод сообщения, скрытого в изображении
   ui->PrintTextArea->setText(message);
}

void Steganograph::cryptMessage()
{
    QByteArray ByteKey; //байтовое представление ключа
    QBitArray BitKey; //битовое представление ключа
    int NumOfBlocks=textSize/8; //хранит число секций текста по 64 бита
    ByteKey = StringKey.toLatin1(); //байтовое представление ключа  toUtf8()
    BitKey=toBitArray(ByteKey); //битовое представление ключа
    BlockA.resize(32);
    BlockB.resize(32);
    ExchangeBlock.resize(32);
    for (int NumBlocks=0; NumBlocks<NumOfBlocks; NumBlocks++){
        //формирование блоков А и Б
        for (int i=0; i<32; i++){
            BlockA[i]=MessageBit[NumBlocks*64+i];
            BlockB[i]=MessageBit[NumBlocks*64+32+i];
        }
        int ItterKey=0;
        //переменная ItterOfCrypt отвечает за число циклов шифрования
        for (int ItterOfCrypt=0; ItterOfCrypt<8; ItterOfCrypt++){
            //Копируем блок А во временную переменную
            for (int i=0; i<32; i++)
                ExchangeBlock[i]=BlockA[i];
            for (int a=0; a<8; a++){ //каждый проход цикла работает с одним блоком по 4 бита
                //Сложение блока А с ключом
                for(int b=0; b<4; b++)
                    BlockA[a*4+b]=BlockA[a*4+b] ^ BitKey[ItterKey*4+b];
                ItterKey++; if (ItterKey==64) ItterKey=0;
                //Сложение полученного блока А с блоком Б, и копирование значения из начального блока А в блок Б
                for(int b=0; b<4; b++){
                    BlockA[a*4+b]=BlockA[a*4+b] ^ BlockB[a*4+b];
                    BlockB[a*4+b]=ExchangeBlock[a*4+b];
                }
            }
        }
        //Возвращение полученной 64-битной последовательности
        for (int i=0; i<32; i++){
            MessageBit[NumBlocks*64+i]=BlockA[i];
            MessageBit[NumBlocks*64+32+i]=BlockB[i];
        }
    }
}

void Steganograph::decryptMessage()
{
    QByteArray ByteKey; //байтовое представление ключа
    QBitArray BitKey; //битовое представление ключа
    int NumOfBlocks=textSize/8; //хранит число секций текста по 64 бита
    BlockA.resize(32);
    BlockB.resize(32);
    ExchangeBlock.resize(32);
    ByteKey = StringKey.toLatin1(); //перевод ключа в байтовое представление
    BitKey=toBitArray(ByteKey); //перевод ключа в битовое представление
    int ItterKey; //хранит актуальный номер блока ключа из 4-х бит
    for (int NumBlocks=0; NumBlocks<NumOfBlocks; NumBlocks++){
        //формирование блоков А и Б
        for (int i=0; i<32; i++){
            BlockA[i]=MessageBit[NumBlocks*64+i];
            BlockB[i]=MessageBit[NumBlocks*64+32+i];
        }
        ItterKey=1;
        for (int ItterOfCrypt=0; ItterOfCrypt<8; ItterOfCrypt++){
            //формирования блока замены
            for (int i=0; i<32; i++)
                ExchangeBlock[i]=BlockB[i];
            //каждый проход цикла работает с одним блоком по 4 бита
            for (int a=7; a>=0; a--){
                //Сложение блока Б с ключом
                for(int b=0; b<4; b++){
                    BlockB[a*4+b]=BlockB[a*4+b] ^ BitKey[256-ItterKey*4+b];
                }
                //инкрементирование счётчика блоков ключа
                ItterKey++; if (ItterKey==65) ItterKey=1;
                //Сложение полученного блока А с блоком Б, и копирование значения из начального блока А в блок Б
                for(int b=0; b<4; b++){
                    BlockB[a*4+b]=BlockB[a*4+b] ^ BlockA[a*4+b];
                    BlockA[a*4+b]=ExchangeBlock[a*4+b];
                }
            }
        }
        //Возвращение полученной 64-битной последовательности
        for (int i=0; i<32; i++){
            MessageBit[NumBlocks*64+i]=BlockA[i];
            MessageBit[NumBlocks*64+32+i]=BlockB[i];
        }
    }
}
void Steganograph::on_OpenFile_clicked()
{
    //переменная, хранящая полное имя выбранного файла
    filename = QFileDialog::getOpenFileName(
                this,
                tr("Open File"),
                "/Users/igor/Desktop/Steganography/Images",
                "All Files (*.*)");
    //загрузка изображения в переменную Img
    Img.load(filename);
    //переменная, хранящая ширину изображения
    width=Img.width();
    //переменная, хранящая высоту изображения
    height=Img.height();
    //вычисление объёма всего файла
    imageSizeBit=width*height*8*3-16;
    //переменная, содержащая факт нажатия кнопки
    BoolLoadImage=true;
}

void Steganograph::on_InputTextButton_clicked()
{
    //считывание введённого текста из окна
    message=ui->InputTextArea->toPlainText();
    //переменная, хранящая размер сообщения (в байтах)
    textSize = message.size();
    //проверка длины текста на кратность 8
    int sizesize=8-textSize%8;
    if (textSize%8!=0){
        for (int i=0; i<sizesize; i++){
            message+='!';
            textSize++;
        }
    }
    //проверка на порядок нажатия кнопок
    if (BoolLoadImage==false){
        QMessageBox::information(this, tr("Message!"), "Сначала загрузите изображение");
    }else{
        //проверка на факт введения сообщения
        if (textSize==0)
            QMessageBox::information(this, tr("Message!"), "Введите сообщение!");
        //проверка на превышение доступного объёма для встраивания сообщения
        else if ((textSize*8) > (imageSizeBit-16))
            QMessageBox::information(this, tr("Message!"), "Слишком длинное сообщение");
        //если все проверки пройдены
        else{
            //массив байт, хранящий сообщение в двоичном виде
            QByteArray dataByte = message.toLatin1();
            //перевод из массива байт в массив бит
            MessageBit = toBitArray(dataByte);
            //вызов функции, шифрующей текст
            cryptMessage();
            //переменная, содержащая факт нажатия кнопки
            BoolInputText=true;
        }
    }
}

void Steganograph::on_SaveFile_clicked()
{
    //проверки на правильную последовательность нажатия кнопок
    if (BoolLoadImage==false){
        QMessageBox::information(this, tr("Message!"), "Сначала откройте изображение");
    }else if (BoolInputText==false){
        QMessageBox::information(this, tr("Message!"), "Сначала введите сообщение");
    }else{
        //вызов функции создания и сохранения нового изображения
        createNewImage();
        //обнуление переменных
        BitArray.fill(0);
        Img=QImage();
        newImg=QImage();
        filename.fill(0);
        width=0;
        height=0;
        imageSizeBit=0;
        BoolLoadImage=false;
        BoolInputText=false;
        message.fill(0);
        textSize=0;
        MessageBit.fill(0);
    }
}

void Steganograph::on_OpenFile_2_clicked()
{
    //переменная, хранящая полное имя выбранного файла
    filename = QFileDialog::getOpenFileName(
                this,
                tr("Open File"),
                "/Users/igor/Desktop/Steganography/Images", //потом - оставить только кавычки, всё работает и без пути
                "All Files (*.*)");
    //загрузка изображения в переменную Img
    Img.load(filename);
    //переменная, хранящая ширину изображения
    width=Img.width();
    //переменная, зранящая высоту изображения
    height=Img.height();
    //вычисление объёма всего файла
    imageSizeBit=width*height*8*3;
    //переменная, содержащая факт нажатия кнопки
    BoolLoadImage=true;
}

void Steganograph::on_PrintText_clicked()
{
    //проверка на загруженное изображение
    if (BoolLoadImage==false)
        QMessageBox::information(this, tr("Message!"), "Сначала откройте изображение");
    else{
        //вызов функции, извекающей текст из изображения
        extractTextBit();
        //обнуление переменных
        BitArray.fill(0);
        Img=QImage();
        newImg=QImage();
        filename.fill(0);
        width=0;
        height=0;
        imageSizeBit=0;
        BoolLoadImage=false;
        BoolInputText=false;
        message.fill(0);
        textSize=0;
        MessageBit.fill(0);
    }
}

void Steganograph::on_ChangeKey_clicked()
{
    //Проверка на длину введённого ключа
    if (ui->InputKeyArea->text().size()!=32)
        QMessageBox::information(this, tr("Message!"), "Измените длину ключа.");
    else{
        StringKey=ui->InputKeyArea->text();
    }
}

void Steganograph::on_InputKeyArea_textChanged()
{
    QString EditKey=ui->InputKeyArea->text();
    int EditKeySize=EditKey.size();
    //передача числа введённых символов
    on_label_change_name(EditKeySize);
}

void Steganograph::on_label_change_name(const int &num)
{
    //динамическая проверка на длину ключа
    if (num<32)
        ui->Status->setText("Не хватает " + (QString::number(32-num)) + " символов");
    else if (num==32)
        ui->Status->setText("Длина ключа подходит.");
    else
        ui->Status->setText("Удалите " + (QString::number(num-32)) + " символов");
}

void Steganograph::on_InputTextArea_textChanged()
{
    //отображение числа введённых символов
    ui->CounterSymb->setText("Введено символов: " + QString::number(ui->InputTextArea->toPlainText().size()));
}
