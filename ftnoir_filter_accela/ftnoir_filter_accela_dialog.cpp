/* Copyright (c) 2012-2015 Stanislaw Halik
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#include "ftnoir_filter_accela/ftnoir_filter_accela.h"
#include <cmath>
#include <QDebug>
#include <algorithm>
#include <QDoubleSpinBox>
#include "opentrack/plugin-api.hpp"
#include "qfunctionconfigurator/qfunctionconfigurator.h"
#include <QDialog>

FilterControls::FilterControls() :
    accela_filter(nullptr)
{
    ui.setupUi( this );

    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));
    
    connect(ui.rotation_slider, SIGNAL(valueChanged(int)), this, SLOT(update_rot_display(int)));
    connect(ui.translation_slider, SIGNAL(valueChanged(int)), this, SLOT(update_trans_display(int)));
    connect(ui.ewma_slider, SIGNAL(valueChanged(int)), this, SLOT(update_ewma_display(int)));
    connect(ui.rot_dz_slider, SIGNAL(valueChanged(int)), this, SLOT(update_rot_dz_display(int)));
    connect(ui.trans_dz_slider, SIGNAL(valueChanged(int)), this, SLOT(update_trans_dz_display(int)));

    tie_setting(s.rot_threshold, ui.rotation_slider);
    tie_setting(s.trans_threshold, ui.translation_slider);
    tie_setting(s.ewma, ui.ewma_slider);
    tie_setting(s.rot_deadzone, ui.rot_dz_slider);
    tie_setting(s.trans_deadzone, ui.trans_dz_slider);
    
    update_rot_display(ui.rotation_slider->value());
    update_trans_display(ui.translation_slider->value());
    update_ewma_display(ui.ewma_slider->value());
    update_rot_dz_display(ui.rot_dz_slider->value());
    update_trans_dz_display(ui.trans_dz_slider->value());
}

void FilterControls::register_filter(IFilter* filter)
{
    accela_filter = static_cast<FTNoIR_Filter*>(filter);
//#define LEAKING_DEBUG
#ifdef LEAKING_DEBUG
    auto d = new QDialog();
    auto r = new QFunctionConfigurator(d);
    r->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    r->setConfig(&accela_filter->rot, "");
    r->setFixedSize(800, 300);
    d->show();
#endif
}

void FilterControls::unregister_filter()
{
    accela_filter = nullptr;
}

void FilterControls::doOK() {
    save();
    this->close();
}

void FilterControls::doCancel() {
    discard();
    close();
}

void FilterControls::discard()
{
    s.b->reload();
}

void FilterControls::save() {
    s.b->save();
}

void FilterControls::update_rot_display(int value)
{
    ui.rot_gain->setText(QString::number((value + 1) * s.mult_rot) + "°");
}

void FilterControls::update_trans_display(int value)
{
    ui.trans_gain->setText(QString::number((value + 1) * s.mult_trans) + "mm");
}

void FilterControls::update_ewma_display(int value)
{
    ui.ewma_label->setText(QString::number(value * s.mult_ewma) + "ms");
}

void FilterControls::update_rot_dz_display(int value)
{
    ui.rot_dz->setText(QString::number(value * s.mult_rot_dz) + "°");
}

void FilterControls::update_trans_dz_display(int value)
{
    ui.trans_dz->setText(QString::number(value * s.mult_trans_dz) + "mm");
}

