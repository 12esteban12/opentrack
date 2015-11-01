/* Copyright (c) 2014-2015, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#include "opentrack/plugin-support.hpp"
#include <QFrame>


#ifdef BUILD_api
#   include "opentrack-compat/export.hpp"
#else
#   include "opentrack-compat/import.hpp"
#endif

struct OPENTRACK_EXPORT SelectedLibraries {
    using dylibptr = mem<dylib>;
    mem<ITracker> pTracker;
    mem<IFilter> pFilter;
    mem<IProtocol> pProtocol;
    SelectedLibraries(QFrame* frame, dylibptr t, dylibptr p, dylibptr f);
    SelectedLibraries() : pTracker(nullptr), pFilter(nullptr), pProtocol(nullptr), correct(false) {}
    bool correct;
};
