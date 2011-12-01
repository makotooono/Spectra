#ifndef WAVEANLYZER_H
#define WAVEANLYZER_H

#include <QFile>
#include <QMap>
#include <bass.h>
#include <bass_fx.h>
#include <QVector>
#include <glplotter.h>
#include <QElapsedTimer>
#include <fftw3.h>




class RythmView : public QWidget
{
    Q_OBJECT
public:
    RythmView(QWidget *parent = NULL);

    int point() { return cb.value(); }
protected:
    void paintEvent(QPaintEvent *);
public slots:
    void updateImage(float value);
private:
    QSpinBox cb;
    float v;
};
//qRegisterMetaType<FloatArray>("FloatArray");

class WaveAnlyzer : public QObject
{
    Q_OBJECT
public:
    WaveAnlyzer(QColor color, int size = 4096, QObject *parent = NULL);
    void send() { emit sendData(info, mywave, myfft, mydfft, myspgram); }
    void sendr() { emit sendData(rinfo, mywaver, myfftr, mydfftr, myspgramr); }
    QList<CHANNELINFO*> getCInfo() { return ilist; }
    static QString Path2Title(QString path)
    {
        QStringList tes = path.split(tr("/"));
        QString t = tes.last();
        QString title = t.split(".").first();
        return title;
    }
    FloatArray myfft, mydfft, mywave;
    FloatArray myfftr, mydfftr, mywaver;
    SpecGramArray myspgram, myspgramr;
    FloatArray oldfft, oldfftr;
    float olds, oldsr;
public slots:
    void resize(int size);

signals:
    void sendData(CHANNELINFO *inf, FloatArray wave, FloatArray fft, FloatArray dfft, SpecGramArray spgram);

protected:
    QList<CHANNELINFO *> ilist;
    FloatArray EmphasisFilter(FloatArray in);

    CHANNELINFO *info;
    CHANNELINFO *rinfo; //right channel info



};

class AudioFileAnlyzer : public WaveAnlyzer
{
    Q_OBJECT
public:
    AudioFileAnlyzer(QColor color, int size = 4096, QObject *parent = NULL);
    HSTREAM HStream() { return stream; }
    void setSource(QString path);


    enum MODE
    {
        NONE,
        Playing,
        Stoping,
        Pausing
    };
    BASS_CHANNELINFO cinfo;


public slots:
    void play();
    void stop();
    void pause();
    void seek(int value);
    void setTempo(int tempo);
private slots:
    void process();
signals:
    void lengthChanged(int length);
    void posChanged(int pos);
private:
    RythmView rvb, rvs;
    QString title;
    HSTREAM stream;
    enum MODE mymode;
    QTimer t;
};

class AudioInputAnlyzer : public WaveAnlyzer
{
    Q_OBJECT
public:
    AudioInputAnlyzer(int device, QColor color, int size = 4096, QObject *parent = NULL);
    ~AudioInputAnlyzer();
    HRECORD HRecord() {return record; }
    double *in;
    void sendTempo(int tempo) { emit sendt(tempo); }
    fftw_plan p;
    fftw_complex *out;
    QElapsedTimer et;
    qint64 before;
    QVector<qint64> time;
    RythmView rv;
    std::vector<unsigned char> message;
    bool onoff;

signals:
    void sendt(int tempo);

private:
    int N;

    HRECORD record;

};

#endif // WAVEANLYZER_H
