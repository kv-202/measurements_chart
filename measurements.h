# pragma once

#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <memory>
#include <limits>
#include <stdio.h>

struct measurement {

    double _time;

    double _value;

    measurement(double time, double value):
        _time(time),
        _value(value)
    {}
};

struct scaled_measurement {

    int _y;

    int _measurement_position;

    scaled_measurement(int y, int measurement_position):
        _y(y),
        _measurement_position(measurement_position)
    {}
};

struct scaled_time_measurements {

    int _x;

    int _min_y;

    int _max_y;

    std::vector<scaled_measurement> _measurements;

    scaled_time_measurements(
        int x,
        std::vector<measurement>::const_iterator begin,
        std::vector<measurement>::const_iterator start,
        std::vector<measurement>::const_iterator end,
        double _min_y_value,
        double y_scale):
        _x(x),
        _min_y(0),
        _max_y(0)
    {
        _measurements.reserve(std::distance(start, end));

        double min_y = std::numeric_limits<double>::max();
        double max_y = std::numeric_limits<double>::min();

        for(; start != end; ++start) {

            if (start->_value < min_y) {
                min_y = start->_value;
            }

            if (start->_value > max_y) {
                max_y = start->_value;
            }

            _measurements.emplace_back(((start->_value - _min_y_value) * y_scale), std::distance(begin, start));
        }

        _min_y = (min_y - _min_y_value) * y_scale;
        _max_y = (max_y - _min_y_value) * y_scale;
    }
};

struct scaled_measurements {

    int _x_range;

    int _y_range;

    std::vector<scaled_time_measurements> _measurements;
};

class measurements {

    static bool get_file_size(FILE *f, long int &size, std::string &err) {

        if (fseek(f, 0, SEEK_END)) {

            err = "Cannot set file pointer to end of file.";
            return false;
        }

        size = ftell(f);
        if (size < 0) {

            err = "Cannot get file pointer position.";
            return false;
        }

        if (fseek(f, 0, SEEK_SET)) {

            err = "Cannot set file pointer to begin of file.";
            return false;
        }

        return true;
    }

    inline bool read_line(char *line, int &line_number, std::string &err) {

        line_number++;

        if (line[0] == '#') {
            return true;
        }

        double time = 0;
        double value = 0;
        int count = sscanf(line, "%lf%lf", &time, &value);
        if (count <= 0) {

            std::stringstream ss;
            ss<<"Line "<<line_number<<": Cannot parse time.";
            err = ss.str();
            return false;
        }

        if (count == 1) {

            std::stringstream ss;
            ss<<"Line "<<line_number<<": Cannot parse time.";
            err = ss.str();
            return false;
        }

        if (_min_time > time) {
            _min_time = time;
        }

        if (_max_time < time) {
            _max_time = time;
        }

        if (_min_value > value) {
            _min_value = value;
        }

        if (_max_value < value) {
            _max_value = value;
        }

        _measures.emplace_back(time, value);
        return true;
    }

    bool read(char *buff, char *buff_end, int &line_number, std::vector<char> &line_buff, std::string &err) {

        if (line_buff.size()) {

            for (; buff < buff_end; ++buff) {

                if (*buff == '\n') {

                    line_buff.push_back('\0');
                    if (!read_line(line_buff.data(), line_number, err)) {
                        return false;
                    }

                    ++buff;
                    line_buff.resize(0);
                    break;
                }

                line_buff.push_back(*buff);
            }
        }

        if (line_buff.size()) {
            return true;
        }

        for (char *readed_buff = buff; readed_buff < buff_end; ++readed_buff) {

            if (*readed_buff == '\n') {

                *readed_buff = '\0';
                if (!read_line(buff, line_number, err)) {
                    return false;
                }

                buff = readed_buff + 1;
            }
        }

        if (buff < buff_end) {
            for (; buff < buff_end; ++buff) {
                line_buff.push_back(*buff);
            }
        }

        return true;
    }

public:

    double _min_time;

    double _max_time;

    double _min_value;

    double _max_value;

    std::vector<measurement> _measures;

    bool load(const char *file_path, std::string &err) {

        std::unique_ptr<FILE, void (*)(FILE*)> f(fopen(file_path, "r"), [](FILE* f) { fclose(f); });
        if (!f) {

            err = "Cannot open file.";
            return false;
        }

        long int file_size = 0;
        if (!get_file_size(f.get(), file_size, err)) {
            return false;
        }

        _measures.clear();

        if (file_size > 1000000) {
            _measures.reserve(file_size / 40);
        }

        _min_time = std::numeric_limits<double>::max();
        _max_time = std::numeric_limits<double>::min();

        _min_value = std::numeric_limits<double>::max();
        _max_value = std::numeric_limits<double>::min();

        int line_number = 0;

        std::vector<char> line_buff;
        std::vector<char> buff(1000);
        for(;;) {

            std::size_t ret = fread(buff.data(), 1, buff.size() - 1, f.get());
            buff[ret] = '\0';

            if (!read(buff.data(), buff.data() + ret, line_number, line_buff, err)) {
                return false;
            }

            if (ret != (buff.size() - 1)) {

                if (feof(f.get())) {
                    return true;
                }

                err = "Cannot read file.";
                return false;
            }
        }
        return true;
    }

    void get_scaled(int x_size, int y_size, scaled_measurements &value) {

        double time_range = _max_time - _min_time;
        double value_range = _max_value - _min_value;

        double time_scale = time_range / x_size;
        double value_scale = y_size / value_range;

        int x = 0;
        double new_point_threshold = _min_time + time_scale;

        value._measurements.clear();

        std::vector<measurement>::const_iterator start = _measures.cbegin();
        std::vector<measurement>::const_iterator end = start;
        while(end != _measures.cend()) {

            if (end->_time > new_point_threshold) {

                value._measurements.emplace_back(x, _measures.cbegin(), start, end, _min_value, value_scale);

                x = static_cast<int>(floor(((end->_time - _min_time) / time_range) * x_size)) + 1;
                new_point_threshold = _min_time + (time_scale * x);

                start = end;
            }
            ++end;
        }

        value._measurements.emplace_back(x, _measures.cbegin(), start, end, _min_value, value_scale);
        value._x_range = x_size;
        value._y_range = y_size;
    }

    measurements():
        _min_time(0),
        _max_time(0),
        _min_value(0),
        _max_value(0)
    {}
};
