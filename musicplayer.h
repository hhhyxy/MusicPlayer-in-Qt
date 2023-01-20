#ifndef MUSICPLAYER_H
#define MUSICPLAYER_H

#include <QWidget>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QListWidgetItem>
#include <QSystemTrayIcon>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QVariant>
#include <QByteArray>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QPixmap>
#include <QSize>

QT_BEGIN_NAMESPACE
namespace Ui { class MusicPlayer; }
QT_END_NAMESPACE

class MusicPlayer : public QWidget
{
    Q_OBJECT

public:
    MusicPlayer(QWidget *parent = nullptr);
    ~MusicPlayer();

protected:
    void mouseMoveEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseDoubleClickEvent(QMouseEvent *e);

private slots:
    void iconActived(QSystemTrayIcon::ActivationReason);

    void on_pushButton_openFile_clicked();

    void on_pushButton_lastSong_clicked();

    void on_pushButton_switch_clicked();

    void on_pushButton_nextSong_clicked();

    void on_pushButton_mode_clicked();

    void on_pushButton_volumn_clicked();

    void on_listWidget_itemDoubleClicked(QListWidgetItem *item);

    void updateDuration(qint64 duration);// 歌曲信息改变时，更新时长

    void updatePosition(qint64 position);// 更新进度条

//    void on_horizontalSlider_sliderMoved(int position);

    void sliderClicked();

//    void volumeSliderClicked();

    void on_pushButtonClose_clicked();

    void on_pushButtonMin_clicked();

    void on_pushButtonMax_clicked();

    void on_horizontalSlider_Volume_valueChanged(int value);


private:
    Ui::MusicPlayer *ui;

    QMediaPlaylist *playList; //播放列表
    QMediaPlayer  *mediaPlayer; //播放器
    bool isCanPlay; // 是否能播放
    int volume; // 音量

    QPoint m_mousePoint;    // 鼠标坐标
    QPoint movePoint;   // 窗口移动距离
    bool mousePress;    // 鼠标左键是否按下

    void setTrayIcon();
    void initPlayer();
    void connectSignalsAndSlots();


};
#endif // MUSICPLAYER_H
