#include "WheelSignalEmitter.h"


bool WheelSignalEmitter::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::Wheel) {
         emit wheelScrolled(static_cast<QWheelEvent*>(event));
        // return true to consume; false to allow normal scroll
    }
    return QObject::eventFilter(obj, event);
}
