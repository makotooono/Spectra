#include "waveanlyzer.h"
#include <float.h>

RythmView::RythmView(QWidget *parent)
    : QWidget(parent), v(0)
{
    cb.setRange(0,1023);
    cb.show();
    this->setFixedWidth(400);
    this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

void RythmView::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setBrush(Qt::magenta);
    float hw = this->width()/2;
    float hh = this->height()/2;
    p.drawEllipse(QPointF(hw, hh), v*2*hw, v*2*hh);
}

void RythmView::updateImage(float value)
{


    v = value > v ? value : v;
    this->update();
    v -= 0.03;
    if(v < 0) v = 0;
}

int find_peak(const double* a, size_t size)
{
    double max_value = DBL_MIN;
    int max_idx = 0;
    double dy = 0;
    for (size_t i = 1; i < size; ++i)
    {
        double dy_pre = dy;
        dy = a[i] - a[i-1];
        if (dy_pre > 0 && dy <= 0)
        {
            if (a[i] > max_value)
            {
                max_value = a[i];
                max_idx = i;
            }
        }
    }
    return max_idx;
}

void CALLBACK MyBpmBeatProc(
    DWORD chan,
    double beatpos,
    void *user
    )
{
    qWarning() << beatpos;
}


BOOL CALLBACK MyRecordingCalback(HRECORD handle, const void *buf, DWORD len, void *user)
{
    AudioInputAnlyzer *an = (AudioInputAnlyzer *)user;
    BASS_ChannelGetData(an->HRecord(), an->mywave.data(), an->mywave.size() * 4 | BASS_DATA_FLOAT);
    BASS_ChannelGetData(an->HRecord(), an->myfft.data(), BASS_DATA_FFT2048 | an->myfft.size());
    float nows = 0;
    for(int i = 0; i < an->mydfft.size() -1; i++)
    {
        nows += an->myfft[i];
        an->mydfft[i] = an->mydfft[i+1];
        an->in[i] = (double)an->mydfft[i];
    }
    an->mydfft[an->mydfft.size()-1] = nows - an->olds;
    qWarning() << nows - an->olds;

    an->rv.updateImage(nows);
    an->in[an->mydfft.size()-1] = (double)nows - an->olds;
    fftw_execute(an->p);
    double power[an->mydfft.size()/2];
    for(int i = 0; i < an->mydfft.size()/2; i++)
    {
        power[i] = pow(an->out[i][0],2) + pow(an->out[i][1],2);
    }
    int s = 1;

    for(int i = 0; i < an->time.size()-1; i++)
    {
        an->time[i] = an->time[i+1];
        s += an->time[i];
    }

    an->time[an->time.size()-1] = 1;


    double srate =  1024.0 / (double)s /1000.0 / 2.0 / 512.0;
    //qWarning() << an->et.elapsed();
    double tempo = find_peak(power, an->mydfft.size()/2)*srate*60;
    qWarning() << tempo;
    if(tempo > 40 && tempo < 280 )
    {
        an->sendTempo(tempo);
    }
    else
    {
        an->sendTempo(140);
    }
    an->olds = nows;
    for(int i = 0; i < an->myspgram.size() -1; i++)
    {
        an->myspgram[i] = an->myspgram[i+1];;
    }
    an->myspgram[an->myspgram.size()-1] = QVector<float>(an->myfft);
    an->myspgramr[an->myspgramr.size()-1] = QVector<float>(an->myfftr);
    an->send();
    return TRUE; // continue recording
}

WaveAnlyzer::WaveAnlyzer(QColor color, int size, QObject *parent)
    : QObject(parent), olds(0), oldsr(0)
{
    info = new CHANNELINFO(color, QString(""), this, true);
    rinfo = new CHANNELINFO(QColor(128, 255, 51, 240), QString(""), this, false);
    mywave.resize(size);
    mydfft.resize(size/8);
    myfft.resize(size/2);
    mywaver.resize(size);
    mydfftr.resize(size/8);
    myfftr.resize(size/2);
    oldfft.resize(size/2);
    oldfftr.resize(size/2);
    myspgram.resize(128);
    myspgramr.resize(128);
}



void WaveAnlyzer::resize(int size)
{
    mywave.resize(size);
    mydfft.resize(size/2);
    myfft.resize(size/4);
    mywaver.resize(size);
    mydfftr.resize(size/2);
    myfftr.resize(size/4);
}

AudioFileAnlyzer::AudioFileAnlyzer(QColor color, int size, QObject *parent)
    : WaveAnlyzer(color, size, parent), mymode(NONE)
{
    info->type = CHANNELINFO::CHANNELFILE;
    rinfo->type = CHANNELINFO::CHANNELFILE;
    ilist.append(info);
    ilist.append(rinfo);
    t.setInterval(10);
    connect(&t, SIGNAL(timeout()), this, SLOT(process()));
    rvb.show();
    rvs.show();

}


void AudioFileAnlyzer::play()
{

    if(mymode != NONE && mymode != Playing)
    {
        BASS_ChannelPlay(stream, false);
        qWarning() << BASS_ErrorGetCode();
        mymode = Playing;
        t.start();
    }
}

void AudioFileAnlyzer::stop()
{
    if(mymode != NONE)
    {
        BASS_ChannelPause(stream);
        BASS_ChannelSetPosition(stream, 0, BASS_POS_BYTE);
        mymode = Stoping;
        t.stop();
    }
}


void AudioFileAnlyzer::pause()
{
    if(mymode != NONE)
    {
        BASS_ChannelPause(stream);
        mymode = Pausing;
        t.stop();
    }
}

void AudioFileAnlyzer::process()
{
    AudioFileAnlyzer *af = this;
    int channel = (int)(this->cinfo.chans);
    FloatArray wavebuf(af->mywave.size()*channel);
    FloatArray fftbuf(af->myfft.size()*channel);
    BASS_ChannelGetData(af->HStream(), wavebuf.data(), wavebuf.size()*4| BASS_DATA_FLOAT);
    BASS_ChannelGetData(af->HStream(), fftbuf.data(), BASS_DATA_FFT2048 | BASS_DATA_FFT_INDIVIDUAL | fftbuf.size());
    QVector<float> ddfft(oldfft.size());
    QVector<float> ddfftr(oldfftr.size());

    if(channel == 2)
    {
        for(int i=0; i < wavebuf.size(); i+=2)
        {
            af->mywave[i/2] = wavebuf[i];
            af->mywaver[i/2] = wavebuf[i+1];
        }
        for(int i=0; i < fftbuf.size(); i+=2)
        {
            af->myfft[i/2] = fftbuf[i];

            af->myfftr[i/2] = fftbuf[i+1];
            ddfft[i/2] = af->myfft[i/2] - af->oldfft[i/2];
            ddfftr[i/2] = af->myfft[i/2] - af->oldfftr[i/2];
        }
        rvb.updateImage(af->myfft[rvb.point()]);
        rvs.updateImage(af->myfft[rvs.point()]);



        float nows = 0, nowsr = 0;

        for(int i = 0; i < af->mydfft.size() -1; i++)
        {
            nows += af->myfft[i];
            af->mydfft[i] = af->mydfft[i+1];
            nowsr += af->myfftr[i];
            af->mydfftr[i] = af->mydfftr[i+1];
        }
        oldfft = af->myfft;
        oldfftr = af->myfftr;
        af->mydfft[af->mydfft.size()-1] = nows;
        af->mydfftr[af->mydfftr.size()-1] = nowsr;
        af->olds = nows;
        af->oldsr = nowsr;
        for(int i = 0; i < af->myspgram.size() -1; i++)
        {
            af->myspgram[i] = af->myspgram[i+1];
            af->myspgramr[i] = af->myspgramr[i+1];
        }
        af->myspgram[af->myspgram.size()-1] = QVector<float>(myfft);
        af->myspgramr[af->myspgramr.size()-1] = QVector<float>(myfftr);
        af->send();
        af->sendr();
    }
    else
    {
        for(int i = 0; i < wavebuf.size(); i++)
        {
            af->mywave[i] = wavebuf[i];
        }
        for(int i = 0; i < fftbuf.size(); i++)
        {
            af->myfft[i] = fftbuf[i];
        }
        float nows = 0;

        for(int i = 0; i < af->mydfft.size() -1; i++)
        {
            nows += af->myfft[i];
            af->mydfft[i] = af->mydfft[i+1];
        }
        oldfft = af->myfft;
        af->mydfft[af->mydfft.size()-1] = nows - af->olds;
        af->olds = nows;
        for(int i = 0; i < af->myspgram.size() -1; i++)
        {
            af->myspgram[i] = af->myspgram[i+1];
        }
        af->myspgram[af->myspgram.size()-1] = QVector<float>(af->myfft);
        af->send();

    }
    emit posChanged((int)BASS_ChannelGetPosition(stream, BASS_POS_BYTE));

}

void AudioFileAnlyzer::setTempo(int tempo)
{

    if(stream != NULL)
    {
        BASS_ChannelSetAttribute(stream, BASS_ATTRIB_TEMPO, (tempo-140)/5.0);
    }
}

void AudioFileAnlyzer::seek(int value)
{

    if(stream != NULL)
    {
        BASS_ChannelSetPosition(stream, value, BASS_POS_BYTE);
    }

}

void AudioFileAnlyzer::setSource(QString path)
{
    qWarning() << "a";
    MODE beforeMode = mymode;
    stop();
    BASS_StreamFree(stream);
    stream = BASS_StreamCreateFile(
                false,
                path.toLocal8Bit().data(),
                0,
                0,
                BASS_SAMPLE_LOOP | BASS_STREAM_DECODE
                );
    if(!(stream = BASS_FX_TempoCreate(stream, BASS_SAMPLE_LOOP)))
    {
        qWarning() << BASS_ErrorGetCode();
    }
    BASS_ChannelGetInfo(stream, &cinfo);

    info->name = Path2Title(path) + " L";
    rinfo->name = Path2Title(path) + " R";
    if(beforeMode == Playing)
        play();
    else if(beforeMode == NONE)
        mymode = Stoping;
    emit lengthChanged((int)BASS_ChannelGetLength(stream, BASS_POS_BYTE));
}

FloatArray WaveAnlyzer::EmphasisFilter(FloatArray in)
{
    FloatArray out(in.size());
    out[0] = in[0];
    for(int i = 1; i < in.size(); i++)
    {
        out[i] = in[i] - 0.97 * in[i-1];

        out[i] = in[i] - 1.2 * in[i-1];
    }
    return out;

}


AudioInputAnlyzer::AudioInputAnlyzer(int device, QColor color, int size, QObject *parent)
    : WaveAnlyzer(color, size, parent), p(NULL), out(NULL), onoff(false)
{


    time.resize(1024);
    N = 2048;
    before = 0;
    in = new double[N];
    size_t mem_size = sizeof(fftw_complex)*N;
    out = (fftw_complex*)fftw_malloc( mem_size );
    p = fftw_plan_dft_r2c_1d(N, in, out, FFTW_ESTIMATE);
    info->type = CHANNELINFO::CHANNELINPUT;
    if(!BASS_RecordSetDevice(device))
    {
        qWarning() << "device can't use";
        return;
    }
    BASS_DEVICEINFO dinfo;
    BASS_RecordGetDeviceInfo(device,&dinfo);
    info->name = QString(dinfo.name);
    ilist.append(info);
    rv.show();




    BASS_RecordSetInput(device, BASS_INPUT_ON, 1.0);
    record = BASS_RecordStart(44100, 1, MAKELONG(0,10), &MyRecordingCalback, this);
    et.start();

}

AudioInputAnlyzer::~AudioInputAnlyzer()
{
    BASS_RecordFree();
}

