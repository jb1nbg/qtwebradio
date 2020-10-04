#ifndef FAVORITEUSER_H
#define FAVORITEUSER_H

#include <QDir>
#include <QPushButton>

class FavoriteUser
{
public:
    QDir favoriteStreamDir;
    QString userName;
    QPushButton* favoriteToggleButton;
};

#endif // FAVORITEUSER_H
