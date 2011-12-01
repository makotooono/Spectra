#include "audiomanager.h"
#include <QDebug>


MusicControler::MusicControler(QWidget *parent)
    : QToolBar(parent)
{
    setupUI();
}

void MusicControler::setupUI()
{
    playAction = new QAction(style()->standardIcon(QStyle::SP_MediaPlay), tr("Play"), this);
    playAction->setShortcut(tr("Crl+P"));
    stopAction = new QAction(style()->standardIcon(QStyle::SP_MediaStop), tr("Stop"), this);
    stopAction->setShortcut(tr("Ctrl+S"));
    pauseAction = new QAction(style()->standardIcon(QStyle::SP_MediaPause), tr("Pause"), this);
    connect(playAction, SIGNAL(triggered()), this, SIGNAL(play()));
    connect(playAction,SIGNAL(triggered()), this, SLOT(playButtonClicked()));
    connect(stopAction, SIGNAL(triggered()), this, SIGNAL(stop()));
    connect(pauseAction, SIGNAL(triggered()), this, SIGNAL(pause()));
    connect(pauseAction, SIGNAL(triggered()), this, SLOT(playButtonClicked()));
    seeker = new QSlider(Qt::Horizontal);
    seeker->setRange(0,100);
    seeker->setValue(0);
    connect(seeker, SIGNAL(sliderMoved(int)), this, SIGNAL(seeked(int)));

    this->addAction(playAction);
    this->addAction(stopAction);
    this->addWidget(seeker);
}

void MusicControler::playButtonClicked()
{
    if(this->actions().first() == playAction)
    {
        this->removeAction(playAction);
        this->insertAction(stopAction, pauseAction);
    }
    else
    {
        this->removeAction(pauseAction);
        this->insertAction(stopAction, playAction);
    }
}

AudioManager::AudioManager(QWidget *parent)
    : QWidget(parent)
{
    inputSource = new AudioInputAnlyzer(2, Qt::yellow, 4096, parent);
    fileSource = new AudioFileAnlyzer(QColor(51, 128, 255, 240), 4096, parent);
    cinfoList.append(inputSource->getCInfo());
    qRegisterMetaType<FloatArray>("FloatArray");
    qRegisterMetaType<SpecGramArray>("SpecGramArray");
    connect(inputSource, SIGNAL(sendData(CHANNELINFO*,FloatArray,FloatArray,FloatArray,SpecGramArray)), this, SLOT(retriveData(CHANNELINFO*,FloatArray,FloatArray,FloatArray,SpecGramArray)));
    connect(fileSource, SIGNAL(sendData(CHANNELINFO*,FloatArray,FloatArray,FloatArray,SpecGramArray)), this, SLOT(retriveData(CHANNELINFO*,FloatArray,FloatArray,FloatArray,SpecGramArray)));
    setupAction();
    setupUI();
    this->setAcceptDrops(true);
    connect(&t, SIGNAL(timeout()), this, SLOT(process()));
    connect(fileSource, SIGNAL(lengthChanged(int)), this, SIGNAL(lengthChanged(int)));
    connect(fileSource, SIGNAL(posChanged(int)), this, SIGNAL(posChanged(int)));
    //connect(inputSource, SIGNAL(sendt(int)), fileSource, SLOT(setTempo(int)));
    t.start(20);
}

AudioManager::~AudioManager()
{
}

void AudioManager::dragEnterEvent(QDragEnterEvent *event)
{

    event->acceptProposedAction();
}

void AudioManager::dropEvent(QDropEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    if(urls.isEmpty())
        return;

    foreach(QUrl ur, urls)
    {
        addSource(ur.toLocalFile());
    }
}

void AudioManager::addSource(QString name)
{

    audiofileList.append(name);
    QStringList tes = name.split(tr("/"));
    QString t = tes.last();
    QString title = t.split(".").first();
    if(title.length() > 25)
        title.replace(25, title.length()-25, "...");



    QTableWidgetItem *titleItem = new QTableWidgetItem(title);
    titleItem->setFlags(titleItem->flags() ^ Qt::ItemIsEditable);

    //![14]

    int currentRow = musicTable->rowCount();
    musicTable->insertRow(currentRow);
    qWarning() << titleItem->text();
    musicTable->setItem(currentRow, 0, titleItem);

    //![15]
    if (audiofileList.count() == 1) {
        musicTable->selectRow(0);
    }

}

void AudioManager::play()
{
    fileSource->play();
}

void AudioManager::stop()
{
    fileSource->stop();
}

void AudioManager::pause()
{
    fileSource->pause();
}

void AudioManager::seek(int value)
{
    fileSource->seek(value);
}



void AudioManager::retriveData(CHANNELINFO *info, FloatArray wave, FloatArray fft, FloatArray dfft, SpecGramArray spgram)
{
    wavemap.insert(info, wave);
    fftmap.insert(info, fft);
    dfftmap.insert(info, dfft);
    spgrammap.insert(info, spgram);
}

void AudioManager::tableClicked(int row, int col)
{
    qWarning() << audiofileList.at(row);
    fileSource->setSource(audiofileList.at(row));
    cinfoList.clear();
    cinfoList.append(inputSource->getCInfo());
    cinfoList.append(fileSource->getCInfo());
    emit audioListChanged(cinfoList);

}

void AudioManager::process()

{
    emit sendData(wavemap, fftmap, dfftmap, spgrammap);
}

void AudioManager::setupAction()
{
}

void AudioManager::setupUI()
{

    musicTable = new QTableWidget(0, 1);
    musicTable->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    musicTable->setFixedWidth(200);
    //musicTable->setHorizontalHeaderLabels(headers);
    musicTable->setSelectionMode(QAbstractItemView::SingleSelection);
    musicTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    connect(musicTable, SIGNAL(cellClicked(int,int)),this, SLOT(tableClicked(int,int)));
    QVBoxLayout *mainLay = new QVBoxLayout(this);
    mainLay->addWidget(musicTable);
    this->setLayout(mainLay);
}
