#pragma once

#include <thread>
#include "main_window.h"
#include "measurements.h"

class main_view_model {

    main_window &_wnd;

    measurements &_mesurements;

    std::thread _t;

    void open(QString file_path) {

        _wnd.set_waitng();

        if (_t.joinable()) {
            _t.join();
        }

        _t = (std::thread([this, file_path](){

            std::string err;
            if (!_mesurements.load(file_path.toStdString().c_str(), err)) {

                QString e(err.c_str());
                _wnd.set_error(e);
                _wnd.error();
                return;
            }

            _mesurements.get_scaled(_wnd.get_visible_width(), _wnd.get_visible_height(), _wnd.get_mesurements());
            _wnd.mesures();
        }));
    }

public:

    main_view_model(main_window &wnd, measurements &m):
        _wnd(wnd),
        _mesurements(m)
    {       
        wnd.set_function([this](QString file_path) { open(file_path); } );
        wnd.resize(1020, 520);
    }

    ~main_view_model() {

        if (_t.joinable()) {
            _t.detach();
        }
    }
};
