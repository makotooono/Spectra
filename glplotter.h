#ifndef GLPLOTTER_H
#define GLPLOTTER_H

#include <QGLWidget>
#include <QtGui>
#include <QList>
#include <glu.h>
#include <qglviewer.h>


typedef QVector<float> FloatArray;
typedef QVector<FloatArray> SpecGramArray;

class CHANNELINFO : public QObject
{
    Q_OBJECT
public:
    CHANNELINFO(QColor col, QString nam, QObject *parent, bool l)
        : QObject(parent),
          color(col),
          name(nam),
          enable(false)
    {
        name = nam;
        isL = l;

    }
    bool enable;
    bool isL;
    enum CHANNELTYPE{
        CHANNELINPUT,
        CHANNELFILE
    };
    CHANNELTYPE type;

    QColor color;
    QString name;
public slots:
    void setEnable(bool b) { enable = b; }

};

class GLPlotter : public QGLViewer
{
    Q_OBJECT
public:
    GLPlotter(QWidget *parent);
    QSize sizeHint() const;


    enum ZoomAlignment{
        CENTER,
        LEFT,
        RIGHT
    };

    enum PAINTMODE{
        POLYGON,
        DOT,
        LINE
    };

protected:
    virtual void draw();
    virtual void init();

signals:
    void parameterNumChanged(int x);
    void nowCursorPoint(QPointF p);

    void sendValue(float value);


public slots:
    void realtimeDataPost(QMap<CHANNELINFO *, FloatArray > waveform);
    void realtimeDataPost(QMap<CHANNELINFO *, SpecGramArray> waveform);
    void setmode(int mode);

private:
    void drawp();


    int md;
    QMap<CHANNELINFO *, FloatArray > form;
    QMap<CHANNELINFO *, SpecGramArray > form3d;
    qint64 length;
    int width, height;

};



class Plotter : public QWidget
{
    Q_OBJECT
public:
    QSize sizeHint() const;
    Plotter(QList<CHANNELINFO *> channelList, QWidget *parent);


    enum MODE{
        WAVE,
        SPECTLE,
        IMPULSELOGGER,
        SPECGRAM
    };

    enum MODE mode;



signals:
    void modeChanged(int md);
    void sendValue(float value);

public slots:
    void init(QList<CHANNELINFO *> channelList);
    void realtimeDataPost(QMap<CHANNELINFO *, FloatArray > waveform,
                          QMap<CHANNELINFO *, FloatArray > wavepower,
                          QMap<CHANNELINFO *, FloatArray > wavedfft,
                          QMap<CHANNELINFO *, SpecGramArray > wavespgram);
    void setMode(int);
    void settext(QPointF);

private:
    GLPlotter *glplotter;
    QLabel nowpt;
    QWidget legend;
    QSlider *horizontalSlider;
    QSlider *verticalSlider;
    QHBoxLayout *legendLay;
    QList<QCheckBox *> cboxList;

};

class PlotSettings
{
public:
    PlotSettings();

    void scroll(int dx, int dy);
    void adjust();
    float spanX() const { return maxX - minX; }
    float spanY() const { return maxY - minY; }

    float minX;
    float maxX;
    int numXTicks;
    float minY;
    float maxY;
    int numYTicks;

private:
    static void adjustAxis(float &min, float &max, int &numTicks);
};

#endif  GLPLOTTER_H

