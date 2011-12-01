#include "mainwindow.h"

ToolBar::ToolBar(QString title)
    : QToolBar(title)
{
    buttonGroup = new QButtonGroup(this);
    buttonGroup->setExclusive(true);
    connect(buttonGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(toolsClicked(QAbstractButton*)));
    setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
}


void ToolBar::createTools(QList<CHANNELINFO *> cinfo)
{

    cinfoList = cinfo;
    LfileButton = new QToolButton;
    LfileButton->setIcon(QIcon(":/images/Lchanel.png"));
    LfileButton->setCheckable(true);
    LfileButton->setStatusTip("オーディオファイルの左チャネルを表示します");
    RfileButton = new QToolButton;
    RfileButton->setIcon(QIcon(":/images/Rchanel.png"));
    RfileButton->setCheckable(true);
    RfileButton->setStatusTip("オーディオファイルの右チャネルを表示します");
    foreach(CHANNELINFO *ci, cinfo)
    {
        cmap.insert(ci->name, ci);
        if(ci->type == CHANNELINFO::CHANNELINPUT)
        {
            QToolButton *b = new QToolButton;
            b->setIcon(QIcon(":/images/Microphone.png"));
            b->setCheckable(true);
            b->setText(ci->name);
            b->setStatusTip("インプットソースとして" + ci->name + "を表示します");
            buttonGroup->addButton(b);
            addWidget(b);
        }
        else
        {
            if(ci->isL) LfileButton->setText(ci->name);
            else RfileButton->setText(ci->name);
        }

    }
    buttonGroup->addButton(LfileButton);
    buttonGroup->addButton(RfileButton);

    buttonGroup->buttons().first()->setChecked(true);
    addWidget(LfileButton);
    addWidget(RfileButton);


}

void ToolBar::toolsClicked(QAbstractButton *ab)
{
    ab->text();
    if(!ab->text().isEmpty())
        cmap.value(ab->text())->enable = true;
    foreach(QAbstractButton *a, buttonGroup->buttons())
    {
        if(a != ab && !a->text().isEmpty())
            cmap.value(a->text())->enable = false;
    }

}

void ToolBar::channelInfoChanged(QList<CHANNELINFO *> cinfo)
{
    cmap.clear();
    cinfoList = cinfo;
    foreach(CHANNELINFO *ci, cinfo)
    {
        cmap.insert(ci->name, ci);
        if(ci->type == CHANNELINFO::CHANNELFILE)
        {
            if(ci->isL) LfileButton->setText(ci->name);
            else RfileButton->setText(ci->name);
        }
    }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();
    setWindowTitle("Sound Viewer");
}

MainWindow::~MainWindow()
{
    delete m;


}

void MainWindow::setupUI()
{

    bar = new ToolBar("select");
    bar->addSeparator();
    addToolBar(bar);
    addToolBarBreak();
    viewBar = addToolBar("view");
    viewBar->addSeparator();
    QToolButton *waveform = new QToolButton;
    waveform->setText("waveform");
    waveform->setIcon(QIcon(":/images/audio_wave.png"));
    waveform->setCheckable(true);
    waveform->setStatusTip("波形を表示します");
    QToolButton *fftform = new QToolButton;
    fftform->setText("fftform");
    fftform->setIcon(QIcon(":/images/audio_wave.png"));
    fftform->setCheckable(true);
    fftform->setStatusTip("スペクトルを表示します");
    QToolButton *impulsform = new QToolButton;
    impulsform->setText("impulsform");
    impulsform->setIcon(QIcon(":/images/audio_wave.png"));
    impulsform->setCheckable(true);
    impulsform->setStatusTip("発音タイミングを表示します");
    QToolButton *specgram = new QToolButton;
    specgram->setText("specgram");
    specgram->setIcon(QIcon(":/images/audio_wave.png"));
    specgram->setCheckable(true);
    specgram->setStatusTip("スペクトログラムを表示します");
    viewGroup = new QButtonGroup;
    viewGroup->addButton(waveform, 0);
    viewGroup->addButton(fftform, 1);
    viewGroup->addButton(impulsform, 2);
    viewGroup->addButton(specgram, 3);
    waveform->setChecked(true);
    viewGroup->setExclusive(true);
    viewBar->addWidget(waveform);
    viewBar->addWidget(fftform);
    viewBar->addWidget(impulsform);
    viewBar->addWidget(specgram);
    viewBar->addSeparator();
    m = new AudioManager(this);
    mc = new MusicControler(this);
    mc->addSeparator();
    addToolBar(mc);
    bar->createTools(m->infoList());



    connect(m, SIGNAL(audioListChanged(QList<CHANNELINFO*>)), bar, SLOT(channelInfoChanged(QList<CHANNELINFO*>)));
    connect(mc, SIGNAL(play()), m, SLOT(play()));
    connect(mc, SIGNAL(stop()), m, SLOT(stop()));
    connect(mc, SIGNAL(pause()), m, SLOT(pause()));
    connect(mc, SIGNAL(seeked(int)), m, SLOT(seek(int)));
    connect(m, SIGNAL(lengthChanged(int)), mc, SLOT(seekerLengthChanged(int)));
    connect(m, SIGNAL(posChanged(int)), mc, SLOT(posChanged(int)));
    plotter = new Plotter(m->infoList(),this);
    plotter->setMode(0);
    connect(viewGroup, SIGNAL(buttonClicked(int)), plotter, SLOT(setMode(int)));
//    plotter2 = new Plotter(m->infoList(),this);
//    plotter2->setMode(1);
//    plotter3 = new Plotter(m->infoList(),this);
//    plotter3->setMode(0);
    qRegisterMetaType<FloatArray>("FloatArray");
    connect(m, SIGNAL(audioListChanged(QList<CHANNELINFO*>)), plotter, SLOT(init(QList<CHANNELINFO*>)));
    connect(m, SIGNAL(sendData(QMap<CHANNELINFO*,FloatArray>,QMap<CHANNELINFO*,FloatArray>,QMap<CHANNELINFO*,FloatArray>,QMap<CHANNELINFO*,SpecGramArray>)),
            plotter, SLOT(realtimeDataPost(QMap<CHANNELINFO*,FloatArray>,QMap<CHANNELINFO*,FloatArray>,QMap<CHANNELINFO*,FloatArray>,QMap<CHANNELINFO*,SpecGramArray>)));
//    connect(m, SIGNAL(audioListChanged(QList<CHANNELINFO*>)), plotter2, SLOT(init(QList<CHANNELINFO*>)));
//    connect(m, SIGNAL(sendData(QMap<CHANNELINFO*,FloatArray>,QMap<CHANNELINFO*,FloatArray>,QMap<CHANNELINFO*,FloatArray>,QMap<CHANNELINFO*,SpecGramArray>)),
//            plotter2, SLOT(realtimeDataPost(QMap<CHANNELINFO*,FloatArray>,QMap<CHANNELINFO*,FloatArray>,QMap<CHANNELINFO*,FloatArray>,QMap<CHANNELINFO*,SpecGramArray>)));
//    connect(m, SIGNAL(audioListChanged(QList<CHANNELINFO*>)), plotter3, SLOT(init(QList<CHANNELINFO*>)));
//    connect(m, SIGNAL(sendData(QMap<CHANNELINFO*,FloatArray>,QMap<CHANNELINFO*,FloatArray>,QMap<CHANNELINFO*,FloatArray>,QMap<CHANNELINFO*,SpecGramArray>)),
//            plotter3, SLOT(realtimeDataPost(QMap<CHANNELINFO*,FloatArray>,QMap<CHANNELINFO*,FloatArray>,QMap<CHANNELINFO*,FloatArray>,QMap<CHANNELINFO*,SpecGramArray>)));

    QHBoxLayout *hplotters = new QHBoxLayout(this);
    hplotters->addWidget(plotter);
    //hplotters->addWidget(plotter2);
    QVBoxLayout *vplotters = new QVBoxLayout(this);
    vplotters->addLayout(hplotters);
    //vplotters->addWidget(plotter3);
    QHBoxLayout *mainLay = new QHBoxLayout(this);
    mainLay->addLayout(vplotters);
    mainLay->addWidget(m);
    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->addLayout(mainLay);
    //lay->addWidget(mc);

    QWidget *w = new QWidget(this);
    w->setLayout(lay);
    this->setCentralWidget(w);
    setUnifiedTitleAndToolBarOnMac(true);
    statusBar();
}
