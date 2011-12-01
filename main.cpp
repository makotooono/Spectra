#include <QtGui/QApplication>
#include <QPlastiqueStyle>
#include <QCleanlooksStyle>
#include "mainwindow.h"
#include <QDebug>
#include <bass.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());
    //a.setStyle(new QCleanlooksStyle);
    a.setApplicationName("SPECTRA");

    BASS_RECORDINFO info;
    BASS_RecordInit(-1);
    BASS_Init(-1, 44100, 0,0,0);
    qWarning() << BASS_RecordGetInfo(&info);
    qWarning() << info.inputs;
    for(int i=0; i < 4; i++)
    {
        BASS_DEVICEINFO dinfo;
        BASS_RecordGetDeviceInfo(i,&dinfo);
        qWarning() << "----------------------";
        qWarning() << i;
        qWarning() << QString(dinfo.name);
    }
    MainWindow w;
    w.show();
    return a.exec();
}
