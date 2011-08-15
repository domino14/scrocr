#ifndef BOARDSCANNER_H
#define BOARDSCANNER_H

#include <QObject>
#include <QtGui>

class BoardScanner : public QObject
{
    Q_OBJECT
public:
    explicit BoardScanner(QObject *parent = 0);
signals:

public slots:

private:
    QGraphicsScene scene;
};

#endif // BOARDSCANNER_H
