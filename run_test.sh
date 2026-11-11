#!/bin/bash

# Define all arguments in an array
# Each argument is a separate element
ARGS=(
    "CSCEventDisplay.py"
    "mc=false"
    "l1=true"
    "l1GEM=false"
    "run3=true"
    "unpack=true"
    "unpackGEM=false"
    "runCCLUTOTMB=true"
    "runME11ILT=true"
    "saveEdmOutput=false"
    "doGEMCSC=false"
    # "inputFiles=file:/eos/cms/store/group/upgrade/GEMCSC_Trigger_PhaseII/Phase2Spring24_DYtoLL50_00d881de-dccf-4232-b4a7-39083949f5fd.root"
    "inputFiles=file:/eos/cms/store/data/Run2026D/Muon0/RAW/v1/000/403/775/00001/d2fef35b-ee86-4a98-87db-6d2786bd8e40.root"
    # "inputFiles=file:/eos/cms/store/data/Run2026D/Muon0/RAW-RECO/ZMu-PromptReco-v1/000/403/567/00000/0766bd68-6506-422c-80af-d7ad017dbe07.root"
    # "inputFiles=file:/eos/user/p/pflanaga/cmssw_clean/CMSSW_16_1_0/src/L1Trigger/CSCTriggerPrimitives/test/lcts2.root"
    "plotdir=/eos/user/a/abolshov/GEMCSC/patrick_plot"
    "eventListFile=file_lists/test_event.txt"
)
# ARGS=(
#     "CSCEventDisplay.py"
#     "mc=true"
#     "l1=true"
#     "l1GEM=false"
#     "run3=true"
#     "unpack=false"
#     "unpackGEM=false"
#     "runCCLUTOTMB=true"
#     "runME11ILT=true"
#     "saveEdmOutput=false"
#     "doGEMCSC=true"
#     "inputFiles=file:/eos/user/a/abolshov/GEMCSC/SingleMuonGun/pt99to101/step2_DIGIRAW_job0.root"
#     "plotdir=/eos/user/a/abolshov/GEMCSC/json_injection_validation_no_emulation/"
#     "eventListFile=file_lists/json_events.txt"
# )

# Execute cmsRun with the arguments
# The "${ARGS[@]}" syntax expands the array correctly
cmsRun "${ARGS[@]}"