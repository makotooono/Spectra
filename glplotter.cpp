#include "glplotter.h"
#include <QDebug>
#include <cmath>
#include <QtGui>
#include <omp.h>
#include <glu.h>

const int MAX_SHORT = 32767;



void NURBS_Err(GLenum error_code)
{

    qWarning() << QString((char *)gluErrorString(error_code));
}
int g, j, k, i;
int accm;

GLPlotter::GLPlotter(QWidget *parent)
{


    this->setMouseTracking(false);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);


    length = 0;
    width = height = 0;

}

void GLPlotter::setmode(int mode)
{
    md = mode;
}


void GLPlotter::init()
{
    // Smooth out lines
    glEnable( GL_LINE_SMOOTH );
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    // Enable Blending
    glEnable(GL_BLEND);
    // Specifies pixel arithmetic
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    setBackgroundColor(Qt::darkGray);
    restoreStateFromFile();
}




void GLPlotter::realtimeDataPost(QMap<CHANNELINFO *, FloatArray > waveform)
{
    form = waveform;
    update();
}

void GLPlotter::realtimeDataPost(QMap<CHANNELINFO *, SpecGramArray> waveform)
{
    form3d = waveform;
    draw();
}


void GLPlotter::draw()
{
    drawp();
}

QSize GLPlotter::sizeHint() const
{
    return QSize(650, 600);
}



void GLPlotter::drawp()
{


    QMapIterator<CHANNELINFO *, FloatArray > i(form);

    while (i.hasNext())
    {
        i.next();

        CHANNELINFO *cinfo = i.key();
        FloatArray vdata = i.value();
        if(cinfo->enable && !vdata.isEmpty())
        {
            glColor4f(cinfo->color.redF(), cinfo->color.greenF(), cinfo->color.blueF(), cinfo->color.alphaF());

            const float *data = vdata.constData();


            for(int j = 0; j < vdata.size()-1; j++)
            {
                glBegin(GL_LINES);


                glVertex2d(GLdouble((j-vdata.size()/2)/(double)vdata.size()*2), GLdouble(data[j]));
                glVertex2d(GLdouble((j+1-vdata.size()/2)/(double)vdata.size()*2), GLdouble(data[j+1]));

                glEnd();

            }

        }
    }

}




Plotter::Plotter(QList<CHANNELINFO *> channelList, QWidget *parent)
    : QWidget(parent)
{


    legendLay = new QHBoxLayout(this);
    foreach(CHANNELINFO *info, channelList)
    {
        QCheckBox *box = new QCheckBox(info->name,this);
        connect(box, SIGNAL(clicked(bool)), info, SLOT(setEnable(bool)));
        info->enable = true;
        box->setChecked(true);
        cboxList.append(box);
        legendLay->addWidget(box);
    }
    legend.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    legend.setFixedHeight(50);
    legend.setLayout(legendLay);


    nowpt.setText("");
    glplotter = new GLPlotter(this);
    glplotter->setCursor(Qt::OpenHandCursor);
    QDial *modedial = new QDial;
    QPainter vp, hp;
    vp.setViewport(0, 0, 400, 30);
    horizontalSlider = new QSlider(Qt::Horizontal);
    horizontalSlider->setMaximum(100000);
    horizontalSlider->setMinimum(6000);
    horizontalSlider->setValue(100000);
    verticalSlider = new QSlider(Qt::Vertical);
    verticalSlider->setMaximum(100000);
    verticalSlider->setMinimum(6000);
    verticalSlider->setValue(100000);
    modedial->setMaximum(3);
    modedial->setValue(0);
    modedial->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    modedial->setFixedWidth(50);
    modedial->setFixedHeight(50);
    connect(modedial, SIGNAL(valueChanged(int)), glplotter, SLOT(setmode(int)));
    connect(modedial, SIGNAL(valueChanged(int)), this, SLOT(setMode(int)));
    connect(this, SIGNAL(modeChanged(int)), glplotter, SLOT(setmode(int)));
    connect(horizontalSlider, SIGNAL(valueChanged(int)), glplotter, SLOT(setZoomXParams(int)));
    connect(verticalSlider, SIGNAL(valueChanged(int)), glplotter, SLOT(setZoomYParams(int)));
    connect(glplotter, SIGNAL(ZoomXChanged(int)), horizontalSlider, SLOT(setValue(int)));
    connect(glplotter, SIGNAL(ZoomYChanged(int)), verticalSlider, SLOT(setValue(int)));

    connect(glplotter, SIGNAL(nowCursorPoint(QPointF)), this, SLOT(settext(QPointF)));
    setMode(Plotter::WAVE);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    QGridLayout *plotterLayout = new QGridLayout;
    //plotterLayout->addWidget(verticalSlider, 0, 0, 1, 1);
    plotterLayout->addWidget(glplotter, 0, 1, 1, 1);
    //plotterLayout->addWidget(&vp, 1, 1, 1, 1);
    //plotterLayout->addWidget(horizontalSlider, 1, 1, 1, 1);
    //dialLayout->addWidget(&nowpt);
    mainLayout->addLayout(plotterLayout);
    //mainLayout->addLayout(dialLayout);
    this->setLayout(mainLayout);

}

void Plotter::init(QList<CHANNELINFO *> channelList)
{
    foreach(QCheckBox *box, cboxList)
    {
        legendLay->removeWidget(box);
        cboxList.removeOne(box);
        delete box;
    }
    foreach(CHANNELINFO *info, channelList)
    {
        QCheckBox *box = new QCheckBox(info->name,this);
        connect(box, SIGNAL(clicked(bool)), info, SLOT(setEnable(bool)));
        box->setChecked(true);
        cboxList.append(box);
        legendLay->addWidget(box);
    }
}

void Plotter::setMode(int md)
{
    this->mode = (Plotter::MODE)md;
    emit modeChanged(md);
}


void Plotter::settext(QPointF p)
{
    nowpt.setText(QString::number(p.x())+QString(",")+QString::number(p.y()));

}

QSize Plotter::sizeHint() const
{
    return QSize(400, 400);
}

void Plotter::realtimeDataPost(QMap<CHANNELINFO *, FloatArray >waveform,
                               QMap<CHANNELINFO *, FloatArray >wavepower,
                               QMap<CHANNELINFO *, FloatArray >wavedfft,
                               QMap<CHANNELINFO *, SpecGramArray >wavespgram)
{
    QMapIterator<CHANNELINFO *, FloatArray > it(waveform);
    switch(this->mode)
    {
        case Plotter::WAVE:
            glplotter->realtimeDataPost(waveform);
            break;
        case Plotter::SPECTLE:
            glplotter->realtimeDataPost(wavepower);
            break;
        case Plotter::IMPULSELOGGER:
            glplotter->realtimeDataPost(wavedfft);
            break;
        case Plotter::SPECGRAM:
            glplotter->realtimeDataPost(wavespgram);
            break;
    }




}

