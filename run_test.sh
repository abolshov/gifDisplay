#!/bin/bash

# Define all arguments in an array
# Each argument is a separate element
ARGS=(
    "CSCEventDisplay.py"
    "mc=true"
    "l1=true"
    "l1GEM=false"
    "run3=true"
    "runCCLUTOTMB=true"
    "runME11ILT=true"
    "saveEdmOutput=false"
    "doGEMCSC=true"
    "inputFiles=file:/eos/cms/store/group/upgrade/GEMCSC_Trigger_PhaseII/Phase2Spring24_DYtoLL50_00d881de-dccf-4232-b4a7-39083949f5fd.root"
    "plotdir=/eos/user/a/abolshov/GEMCSC/test_plot"
    "eventListFile=file_lists/test_event.txt"
)

# Execute cmsRun with the arguments
# The "${ARGS[@]}" syntax expands the array correctly
cmsRun "${ARGS[@]}"