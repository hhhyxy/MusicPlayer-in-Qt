#include "music.h"

Music::Music(int id, QString songName, QString author, QString album, int songDuration, QString songUrl)
    :m_id(id),
    m_songName(songName),
    m_author(author),
    m_album(album),
    m_songDuration(songDuration),
    m_songUrl(songUrl)
{


}

int Music::getSongDuration() const
{
    return m_songDuration;
}

void Music::setSongUrl(const QString &newSongUrl)
{
    m_songUrl = newSongUrl;
}

QString Music::getSongUrl() const
{
    return m_songUrl;
}

QString Music::getAuthor() const
{
    return m_author;
}

QString Music::getAlbum() const
{
    return m_album;
}

QString Music::getSongName() const
{
    return m_songName;
}


int Music::getId() const
{
    return m_id;
}
