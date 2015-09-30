/* Copyright (c) 2012-2015 Stanislaw Halik
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#include "ftnoir_filter_accela/ftnoir_filter_accela.h"
#include <algorithm>
#include <cmath>
#include <QDebug>
#include <QMutexLocker>
#include "opentrack/plugin-api.hpp"

static constexpr double rot_gains[][2] = {
    { 2.66, 105 },
    { 2.0, 47 },
    { 1.66, 25 },
    { 1.33, 12 },
    { 1, 3.5 },
    { .66, 1 },
    { .33, .4 },
    { 0, 0 },
    { -1, 0 }
};

static constexpr double trans_gains[][2] = {
    { 2, 400 },
    { 1.66, 120 },
    { 1.33, 40 },
    { 1, 10 },
    { .66, 2 },
    { .33, .6 },
    { 0, 0 },
    { -1, 0 }
};

constexpr double settings_accela::mult_rot;
constexpr double settings_accela::mult_trans;
constexpr double settings_accela::mult_rot_dz;
constexpr double settings_accela::mult_trans_dz;
constexpr double settings_accela::mult_ewma;

FTNoIR_Filter::FTNoIR_Filter() : first_run(true)
{
    rot.setMaxInput(rot_gains[0][0]);
    trans.setMaxInput(trans_gains[0][0]);
    rot.setMaxOutput(rot_gains[0][1]);
    trans.setMaxOutput(trans_gains[0][1]);
    
    for (int i = 0; rot_gains[i][0] >= 0; i++)
    {
        rot.addPoint(QPointF(rot_gains[i][0], rot_gains[i][1]));
    }
    for (int i = 0; trans_gains[i][0] >= 0; i++)
    {
        trans.addPoint(QPointF(trans_gains[i][0], trans_gains[i][1]));
    }
}

void FTNoIR_Filter::filter(const double* input, double *output)
{
    if (first_run)
    {
        for (int i = 0; i < 6; i++)
        {
            output[i] = input[i];
            last_output[i] = input[i];
            smoothed_input[i] = input[i];
        }
        first_run = false;
        t.start();
        return;
    }
    
    const double rot_t = (1+s.rot_threshold) * s.mult_rot;
    const double trans_t = (1+s.trans_threshold) * s.mult_trans;
    
    const double dt = t.elapsed() * 1e-9;
    t.start();

    const double RC = s.mult_ewma * s.ewma / 1000.; // seconds
    const double alpha = dt/(dt+RC);
    const double rot_dz = s.rot_deadzone * s.mult_rot_dz;
    const double trans_dz = s.trans_deadzone * s.mult_trans_dz;
    
    for (int i = 0; i < 6; i++)
    {
        Map& m = i >= 3 ? rot : trans;
        
        smoothed_input[i] = smoothed_input[i] * (1.-alpha) + input[i] * alpha;
        
        const double in = smoothed_input[i];
        
        const double vec = in - last_output[i];
        const double dz = i >= 3 ? rot_dz : trans_dz;
        const double vec_ = std::max(0., fabs(vec) - dz);
        const double thres = i >= 3 ? rot_t : trans_t;
        const double val = m.getValue(vec_ / thres);
        const double result = last_output[i] + (vec < 0 ? -1 : 1) * dt * val;
        const bool negp = vec < 0.;
        const bool done = negp
            ? result <= in
            : result >= in;
        const double ret = done ? in : result;
        
        last_output[i] = output[i] = ret;
    }
}
