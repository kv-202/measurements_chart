#pragma once

#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QFileDialog>
#include <QMessageBox>
#include <QPainter>
#include <QBrush>
#include <QColor>
#include <QTimer>
#include <QPolygon>
#include <QPoint>

#include "measurements.h"

class main_window : public QMainWindow {
    Q_OBJECT

    const int _x_border;

    const int _y_border;

    QString _err;

    QTimer *_timer;

    int _wait;

    scaled_measurements _measurements;

    bool _draw_mesurements;

    std::function<void(QString)> _on_opne;

    void draw_wait(QPainter &p) {

        QString wait = "Wait ";
        QRect r = p.boundingRect(QRect(0, 0, width(), height()), Qt::AlignCenter, wait);

        for (int i = 1; i < _wait; ++i) {
            wait.push_back(".");
        }

        p.drawText(r.bottomLeft(), wait);
    }

    void draw_measurements(QPainter &p) {

        int h = height();

        QPolygon pol;
        pol<<QPoint(_measurements._measurements.cbegin()->_x + _x_border, h - _y_border);

        for (std::vector<scaled_time_measurements>::const_iterator it_t = _measurements._measurements.cbegin();
             it_t != _measurements._measurements.cend();
             ++it_t) {

            if (it_t->_max_y == it_t->_min_y) {

                pol<<QPoint(it_t->_x + _x_border, h - it_t->_max_y - _y_border);
                continue;
            }

            pol<<QPoint(it_t->_x + _x_border, h - it_t->_measurements.cbegin()->_y - _y_border);
            pol<<QPoint(it_t->_x + _x_border, h - it_t->_min_y - _y_border);
            pol<<QPoint(it_t->_x + _x_border, h - it_t->_max_y - _y_border);
            pol<<QPoint(it_t->_x + _x_border, h - it_t->_measurements.crbegin()->_y - _y_border);
        }

        pol<<QPoint(_measurements._measurements.crbegin()->_x + _x_border, h - _y_border);

        p.drawPolygon(pol);
    }

    void paintEvent(QPaintEvent *) {

        QPainter p(this);

        if (_wait) {

            draw_wait(p);
            return;
        }

        if (_draw_mesurements) {
            draw_measurements(p);
        }
    }

signals:

    void clear();

    void mesures();

    void error();

public slots:

    void on_timeout() {

        if (_wait > 3) {
            _wait = 1;
        }
        else
        {
            ++_wait;
        }

        update();
        repaint();
    }

    void on_open(bool) {
        
        if (_wait) {
            return;
        }

        QString file = QFileDialog::getOpenFileName(this);
        if (!file.isEmpty() && _on_opne) {
            _on_opne(file);
        }
    }

    void on_clear() {

        if (!_timer) {
            return;
        }

        delete _timer;
        _timer = nullptr;

        _draw_mesurements = false;
        _wait = 0;

        update();
        repaint();
    }

    void on_mesures() {

        if (_timer) {

            delete _timer;
            _timer = nullptr;
        }

        _draw_mesurements = true;
        _wait = 0;

        update();
        repaint();
    }

    void on_error() {

        if (_timer) {

            delete _timer;
            _timer = nullptr;
        }

        _draw_mesurements = false;
        _wait = 0;

        update();
        repaint();

        QMessageBox::critical(this, "Error", _err);
    }

public:

    main_window():
        _x_border(10),
        _y_border(10),
        _timer(nullptr),
        _wait(0),
        _draw_mesurements(false),
        _on_opne(nullptr)
    {

        QMenu *file_menu = new QMenu("File");
        connect(file_menu->addAction("Open"), &QAction::triggered, this, &main_window::on_open);
        file_menu->addSeparator();
        connect(file_menu->addAction("Exit"), &QAction::triggered, this, &main_window::close);

        QMenuBar *main_menu = new QMenuBar();
        main_menu->addMenu(file_menu);
        setMenuBar(main_menu);

        connect(this, &main_window::clear, this, &main_window::on_clear);
        connect(this, &main_window::mesures, this, &main_window::on_mesures);
        connect(this, &main_window::error, this, &main_window::on_error);
    }

    void set_function(std::function<void(QString)> &&func) { _on_opne = std::move(func); }

    void set_error(QString &value) { _err = value; }

    void set_waitng() {

        if (_timer) {
            return;
        }

        _timer = new QTimer(this);
        connect(_timer, &QTimer::timeout, this, &main_window::on_timeout);

        _wait = 1;
        _draw_mesurements = false;

        _timer->start(500);
    }

    scaled_measurements &get_mesurements() { return _measurements; }

    int get_visible_height() { return height() - (2 * _x_border) - menuBar()->height(); }

    int get_visible_width() { return width() - (2 * _y_border); }
};
