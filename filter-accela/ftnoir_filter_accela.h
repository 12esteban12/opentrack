/* Copyright (c) 2012-2015 Stanislaw Halik
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#pragma once
#include "ui_ftnoir_accela_filtercontrols.h"
#include "opentrack/plugin-api.hpp"
#include "spline-widget/functionconfig.h"
#include <atomic>
#include <QMutex>
#include <QTimer>

#include "opentrack-compat/options.hpp"
using namespace options;
#include "opentrack-compat/timer.hpp"

struct settings_accela : opts
{
    value<int> rot_threshold, trans_threshold, ewma, rot_deadzone, trans_deadzone;
    value<slider_value> rot_nonlinearity;
    static constexpr double mult_rot = 4. / 100.;
    static constexpr double mult_trans = 4. / 100.;
    static constexpr double mult_rot_dz = 2. / 100.;
    static constexpr double mult_trans_dz = 2. / 100.;
    static constexpr double mult_ewma = 1.25;
    static constexpr double max_rot_nl = 1.33;
    settings_accela() :
        opts("Accela"),
        rot_threshold(b, "rotation-threshold", 45),
        trans_threshold(b, "translation-threshold", 50),
        ewma(b, "ewma", 2),
        rot_deadzone(b, "rotation-deadzone", 0),
        trans_deadzone(b, "translation-deadzone", 0),
        rot_nonlinearity(b, "rotation-nonlinearity", slider_value(1, 1, 2))
    {}
};

class FTNoIR_Filter : public IFilter
{
public:
    FTNoIR_Filter();
    void filter(const double* input, double *output) override;
    void center() override { first_run = true; }
    Map rot, trans;
private:
    settings_accela s;
    bool first_run;
    double last_output[6];
    double smoothed_input[6];
    Timer t;
};

class FilterControls: public IFilterDialog
{
    Q_OBJECT
public:
    FilterControls();
    void register_filter(IFilter* filter);
    void unregister_filter();
private:
    Ui::AccelaUICFilterControls ui;
    void save();
    FTNoIR_Filter* accela_filter;
    settings_accela s;
private slots:
    void doOK();
    void doCancel();
    void update_ewma_display(int value);
    void update_rot_display(int value);
    void update_trans_display(int value);
    void update_rot_dz_display(int value);
    void update_trans_dz_display(int value);
    void update_rot_nl_slider(const slider_value& sl);
};

class FTNoIR_FilterDll : public Metadata
{
public:
    QString name() { return QString("Accela"); }
    QIcon icon() { return QIcon(":/images/filter-16.png"); }
};
