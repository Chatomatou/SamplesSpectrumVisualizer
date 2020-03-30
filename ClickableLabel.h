#ifndef CLICKABLELABEL_H
#define CLICKABLELABEL_H

#include <QMouseEvent>
#include <QLabel>


class ClickableLabel : public QLabel
{
    Q_OBJECT
public:
    explicit ClickableLabel(QWidget* parent = Q_NULLPTR,  Qt::WindowFlags flags = Qt::WindowFlags());
    ~ClickableLabel();
signals:
    void clicked();
protected:
    void mousePressEvent(QMouseEvent* event);
};

#endif // CLICKABLELABEL_H
