#ifndef STEGANOGRAPH_H
#define STEGANOGRAPH_H

#include <QWidget>
#include <QByteArray>
#include <QBitArray>

namespace Ui {
class Steganograph;
}

class Steganograph : public QWidget
{
    Q_OBJECT

public:
    explicit Steganograph(QWidget *parent = 0);
    ~Steganograph();

private slots:
    void on_OpenFile_clicked();
    void on_InputTextButton_clicked();
    void on_SaveFile_clicked();
    void on_OpenFile_2_clicked();
    void on_PrintText_clicked();
    void on_ChangeKey_clicked();
    void on_InputKeyArea_textChanged();
    void on_InputTextArea_textChanged();
private:
    QBitArray BitArray;
    QImage Img;
    QImage newImg;
    QString filename;
    int width;
    int height;
    unsigned int imageSizeBit;
    unsigned int PSNR;
    bool BoolLoadImage=false;
    bool BoolInputText=false;
    QString message;
    quint16 textSize=0;
    QBitArray MessageBit;
    QBitArray toBitArray(const QByteArray);
    QByteArray toByteArray(const QBitArray);
    void createNewImage();
    void extractTextBit();
    void cryptMessage();
    void decryptMessage();
    void on_label_change_name(const int &num);
    QString StringKey="Sk36236kjhHjhkehGKhg48oKH4gjkjb3";
    QBitArray BlockA, BlockB, ExchangeBlock;
    Ui::Steganograph *ui;
};

#endif // STEGANOGRAPH_H
