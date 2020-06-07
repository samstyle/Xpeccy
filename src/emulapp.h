#pragma once
#include <QApplication>
#include "emulwin.h"

class EmulatorApp : public QApplication
{
public:
    EmulatorApp(int &argc, char **argv, int interval);
    bool event(QEvent *event) override;
    
    MainWin *mwin;
};