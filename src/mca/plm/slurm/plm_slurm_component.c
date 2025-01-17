/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil -*- */
/*
 * Copyright (c) 2004-2008 The Trustees of Indiana University and Indiana
 *                         University Research and Technology
 *                         Corporation.  All rights reserved.
 * Copyright (c) 2004-2005 The University of Tennessee and The University
 *                         of Tennessee Research Foundation.  All rights
 *                         reserved.
 * Copyright (c) 2004-2005 High Performance Computing Center Stuttgart,
 *                         University of Stuttgart.  All rights reserved.
 * Copyright (c) 2004-2005 The Regents of the University of California.
 *                         All rights reserved.
 * Copyright (c) 2015      Los Alamos National Security, LLC. All rights
 *                         reserved.
 * Copyright (c) 2017-2019 Intel, Inc.  All rights reserved.
 * Copyright (c) 2019      Research Organization for Information Science
 *                         and Technology (RIST).  All rights reserved.
 * Copyright (c) 2020      Cisco Systems, Inc.  All rights reserved
 * Copyright (c) 2021-2022 Nanook Consulting.  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 *
 * These symbols are in a file by themselves to provide nice linker
 * semantics.  Since linkers generally pull in symbols by object
 * files, keeping these symbols as the only symbols in this file
 * prevents utility programs such as "ompi_info" from having to import
 * entire components just to query their version and parameters.
 */

#include "prte_config.h"
#include "constants.h"

#include "src/mca/base/prte_mca_base_var.h"
#include "src/runtime/prte_globals.h"
#include "src/util/name_fns.h"
#include "src/util/pmix_environ.h"
#include "src/util/show_help.h"

#include "plm_slurm.h"
#include "src/mca/plm/base/plm_private.h"
#include "src/mca/plm/plm.h"

/*
 * Public string showing the plm ompi_slurm component version number
 */
const char *prte_plm_slurm_component_version_string
    = "PRTE slurm plm MCA component version " PRTE_VERSION;

/*
 * Local functions
 */
static int plm_slurm_register(void);
static int plm_slurm_open(void);
static int plm_slurm_close(void);
static int prte_plm_slurm_component_query(prte_mca_base_module_t **module, int *priority);

/*
 * Instantiate the public struct with all of our public information
 * and pointers to our public functions in it
 */

prte_plm_slurm_component_t prte_plm_slurm_component = {

    {
        /* First, the mca_component_t struct containing meta
           information about the component itself */

        .base_version = {
            PRTE_PLM_BASE_VERSION_2_0_0,

            /* Component name and version */
            .mca_component_name = "slurm",
            PRTE_MCA_BASE_MAKE_VERSION(component, PRTE_MAJOR_VERSION, PRTE_MINOR_VERSION,
                                        PMIX_RELEASE_VERSION),

            /* Component open and close functions */
            .mca_open_component = plm_slurm_open,
            .mca_close_component = plm_slurm_close,
            .mca_query_component = prte_plm_slurm_component_query,
            .mca_register_component_params = plm_slurm_register,
        },
        .base_data = {
            /* The component is checkpoint ready */
            PRTE_MCA_BASE_METADATA_PARAM_CHECKPOINT
        },
    }

    /* Other prte_plm_slurm_component_t items -- left uninitialized
       here; will be initialized in plm_slurm_open() */
};

static int plm_slurm_register(void)
{
    prte_mca_base_component_t *comp = &prte_plm_slurm_component.super.base_version;

    prte_plm_slurm_component.custom_args = NULL;
    (void) prte_mca_base_component_var_register(comp, "args", "Custom arguments to srun",
                                                PRTE_MCA_BASE_VAR_TYPE_STRING, NULL, 0,
                                                PRTE_MCA_BASE_VAR_FLAG_NONE, PRTE_INFO_LVL_9,
                                                PRTE_MCA_BASE_VAR_SCOPE_READONLY,
                                                &prte_plm_slurm_component.custom_args);

    prte_plm_slurm_component.slurm_warning_msg = true;
    (void) prte_mca_base_component_var_register(comp, "warning", "Turn off warning message",
                                                PRTE_MCA_BASE_VAR_TYPE_BOOL, NULL, 0,
                                                PRTE_MCA_BASE_VAR_FLAG_NONE, PRTE_INFO_LVL_9,
                                                PRTE_MCA_BASE_VAR_SCOPE_READONLY,
                                                &prte_plm_slurm_component.slurm_warning_msg);

    return PRTE_SUCCESS;
}

static int plm_slurm_open(void)
{
    return PRTE_SUCCESS;
}

static int prte_plm_slurm_component_query(prte_mca_base_module_t **module, int *priority)
{
    /* Are we running under a SLURM job? */

    if (NULL != getenv("SLURM_JOBID")) {
        *priority = 75;

        PRTE_OUTPUT_VERBOSE((1, prte_plm_base_framework.framework_output,
                             "%s plm:slurm: available for selection",
                             PRTE_NAME_PRINT(PRTE_PROC_MY_NAME)));

        *module = (prte_mca_base_module_t *) &prte_plm_slurm_module;
        return PRTE_SUCCESS;
    }

    /* Sadly, no */
    *module = NULL;
    return PRTE_ERROR;
}

static int plm_slurm_close(void)
{
    return PRTE_SUCCESS;
}
