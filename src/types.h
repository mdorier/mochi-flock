/*
 * (C) 2024 The University of Chicago
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef _PARAMS_H
#define _PARAMS_H

#include <stdlib.h>
#include <mercury.h>
#include <mercury_macros.h>
#include <mercury_proc.h>
#include <mercury_proc_string.h>
#include "flock/flock-common.h"
#include "flock/flock-group-view.h"

/* Client RPC types */

MERCURY_GEN_PROC(get_view_in_t,
        ((uint64_t)(digest)))

MERCURY_GEN_PROC(get_view_out_t,
        ((uint8_t)(no_change))\
        ((flock_group_view_t)(view))\
        ((int32_t)(ret)))

#endif
