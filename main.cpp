
#include <QApplication>

#include "main_window.h"
#include "main_view_model.h"
#include "measurements.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    main_window wnd;
    wnd.show();

    measurements m;
    main_view_model view_model(wnd, m);

    return a.exec();
}
