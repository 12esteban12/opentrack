/* Copyright (c) 2011-2014, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#include <QList>
#include <QPointF>
#include <QString>
#include <QSettings>
#include <QMutex>
#include <vector>
#include <limits>
#include "opentrack-compat/qcopyable-mutex.hpp"

class Map {
private:
    static constexpr int value_count = 10000;

    struct State {
        QList<QPointF> input;
        std::vector<float> data;        
    };

    int precision() const;
    void reload();
    float getValueInternal(int x);

    MyMutex _mutex;
    QPointF last_input_value;
    volatile bool activep;
    double max_x;
    double max_y;

    State cur, saved;
    bool lazy_reload;
public:
    double maxInput() const { return max_x; }
    double maxOutput() const { return max_y; }
    Map();
    Map(double maxx, double maxy)
    {
        setMaxInput(maxx);
        setMaxOutput(maxy);
        lazy_reload = true;
    }

    float getValue(float x);
    bool getLastPoint(QPointF& point);
    void removePoint(int i);
    void removeAllPoints() {
        QMutexLocker foo(&_mutex);
        cur.input.clear();
        lazy_reload = true;
    }

    void addPoint(QPointF pt);
    void movePoint(int idx, QPointF pt);
    const QList<QPointF> getPoints();
    void setMaxInput(double MaxInput) {
        max_x = MaxInput;
    }
    void setMaxOutput(double MaxOutput) {
        max_y = MaxOutput;
    }

    void saveSettings(QSettings& settings, const QString& title);
    void loadSettings(QSettings& settings, const QString& title);
    void invalidate_unsaved_settings();

    void setTrackingActive(bool blnActive);
};
