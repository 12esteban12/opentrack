#include "ftnoir_filter_ewma2.h"
#include <cmath>
#include <QDebug>
#include "opentrack/plugin-api.hpp"
#include "ui_ftnoir_ewma_filtercontrols.h"

FilterControls::FilterControls() : pFilter(NULL)
{
    ui.setupUi( this );

    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));

    tie_setting(s.kMaxSmoothing, ui.maxSmooth);
    tie_setting(s.kMinSmoothing, ui.minSmooth);
    tie_setting(s.kSmoothingScaleCurve, ui.powCurve);
}

void FilterControls::register_filter(IFilter* flt)
{
    pFilter = (FTNoIR_Filter*) flt;
}

void FilterControls::unregister_filter()
{
    pFilter = NULL;
}

void FilterControls::doOK() {
    save();
    close();
}

void FilterControls::doCancel()
{
    close();
}

void FilterControls::save() {
    s.b->save();
    if (pFilter)
        pFilter->receiveSettings();
}
