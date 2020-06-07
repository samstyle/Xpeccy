#include <QApplication>
#include <QFileOpenEvent>
#include "xgui/xgui.h"
#include "emulapp.h"
#include "filer.h"
#include "emulwin.h"
            
EmulatorApp::EmulatorApp(int &argc, char **argv, int interval) : QApplication(argc, argv, interval) {}

bool EmulatorApp::event(QEvent *event)
{
    if (event->type() == QEvent::FileOpen) {
        QFileOpenEvent *openEvent = static_cast<QFileOpenEvent *>(event);
        // in openEvent->file() filename 
        if (this->mwin) {
            load_file(this->mwin->comp, 
                      openEvent->file().toUtf8().data(), 
                      FG_ALL, 
                      0
                      );
        } else {
            shitHappens("FileOpen event received before Application inited");
        }
    }

    return QApplication::event(event);
}
