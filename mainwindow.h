#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui>
#include <glplotter.h>
#include <audiomanager.h>

class ToolBar : public QToolBar
{
    Q_OBJECT
public:
    ToolBar(QString title);

    void createTools(QList<CHANNELINFO *> cinfo);

public slots:
    void channelInfoChanged(QList<CHANNELINFO *> cinfo);
private slots:
    void toolsClicked(QAbstractButton *ab);

private:
    QMap<QString, CHANNELINFO *> cmap;
    QList<CHANNELINFO *> cinfoList;
    QButtonGroup *buttonGroup;
    QToolButton *LfileButton;
    QToolButton *RfileButton;
    QList<QToolButton *> inputButtonList;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    void setupUI();
    AudioManager *m;
    MusicControler *mc;
    ToolBar *bar;
    QButtonGroup *viewGroup;
    QToolBar *viewBar;


    Plotter *plotter, *plotter2, *plotter3;
};

#endif // MAINWINDOW_H
