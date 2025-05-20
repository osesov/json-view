#pragma once

#include <QObject>
#include <QWheelEvent>

class WheelSignalEmitter : public QObject
{
    Q_OBJECT

signals:
    void wheelScrolled(QWheelEvent* event);

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;
};
