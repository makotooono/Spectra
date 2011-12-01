#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H
#include <waveanlyzer.h>
#include <QList>
#include <QTimer>
#include <QDragEnterEvent>
#include <QDropEvent>


class MusicControler : public QToolBar
{
    Q_OBJECT
public:
    MusicControler(QWidget *parent = NULL);


signals:
    void play();
    void stop();
    void pause();
    void seeked(int value);

public slots:
    void seekerLengthChanged(int max) { seeker->setMaximum(max); }
    void posChanged(int pos) { seeker->setValue(pos); }

private slots:
    void playButtonClicked();

private:
    void setupUI();


    QAction *playAction;
    QAction *stopAction;
    QAction *pauseAction;
    QSlider *seeker;
};

class AudioManager : public QWidget
{
    Q_OBJECT
public:
    AudioManager(QWidget *parent = NULL);
    ~AudioManager();
    void run();
    QList<CHANNELINFO *> infoList() { return cinfoList; }
protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

signals:
    void sendData(QMap<CHANNELINFO *, FloatArray > form,
                  QMap<CHANNELINFO *, FloatArray > power,
                  QMap<CHANNELINFO *, FloatArray > dfft,
                  QMap<CHANNELINFO *, SpecGramArray > spgram);
    void audioListChanged(QList<CHANNELINFO *> cinfo);
    void lengthChanged(int length);
    void posChanged(int pos);

private slots:
    void play();
    void stop();
    void pause();
    void seek(int value);
    void process();
    void tableClicked(int row, int col);
    void retriveData(CHANNELINFO *info, FloatArray wave, FloatArray fft, FloatArray dfft, SpecGramArray spgram);
private:
    void setupUI();
    void setupAction();
    void addSource(QString name);
    AudioInputAnlyzer *inputSource;
    AudioFileAnlyzer *fileSource;
    QAction *repeatAction;
    QAction *playAction;
    QAction *stopAction;
    QTimer t;
    QList<QString> audiofileList;
    QList<CHANNELINFO *> cinfoList;
    QMap<CHANNELINFO *, FloatArray > wavemap, fftmap, dfftmap;
    QMap<CHANNELINFO *, SpecGramArray> spgrammap;
    QTableWidget *musicTable;

};

#endif // AUDIOMANAGER_H
