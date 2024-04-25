import FWCore.ParameterSet.Config as cms
from FWCore.ParameterSet.VarParsing import VarParsing
from Configuration.Eras.Era_Run3_cff import Run3
from Configuration.Eras.Era_Run2_2018_cff import Run2_2018

import sys, os
### cmsRun CSCEventDisplay.py plotdir="/afs/cern.ch/work/t/tahuang/CSCEmulation/CMSSW_10_2_1/src/displayplots/"
options = VarParsing ('analysis')
options.register("plotdir", '', mytype=VarParsing.varType.string)
options.register("eventListFile", 'eventList.txt', VarParsing.multiplicity.singleton, VarParsing.varType.string,
                            "txt file contains the list of event and chamber to display")
options.register("unpack", False, VarParsing.multiplicity.singleton, VarParsing.varType.bool,
                         "Set to True when you want to unpack the CSC DAQ data.")
options.register("unpackGEM", False, VarParsing.multiplicity.singleton, VarParsing.varType.bool,
                         "Set to True when you want to unpack the GEM DAQ data.")
options.register("l1", False, VarParsing.multiplicity.singleton, VarParsing.varType.bool,
                         "Set to True when you want to re-emulate the CSC trigger primitives.")
options.register("l1GEM", False, VarParsing.multiplicity.singleton, VarParsing.varType.bool,
                         "Set to True when you want to re-emulate the GEM trigger primitives.")
options.register("mc", False, VarParsing.multiplicity.singleton, VarParsing.varType.bool,
                         "Set to True when running on MC.")
options.register("dqm", False, VarParsing.multiplicity.singleton, VarParsing.varType.bool,
                         "Set to True when you want to run the CSC DQM")
options.register("dqmGEM", False, VarParsing.multiplicity.singleton, VarParsing.varType.bool,
                         "Set to True when you want to run the GEM DQM")
options.register("useEmtfGEM", False, VarParsing.multiplicity.singleton, VarParsing.varType.bool,
                         "Set to True when you want to use GEM clusters from the EMTF in the DQM")
options.register("useB904ME11", False, VarParsing.multiplicity.singleton, VarParsing.varType.bool,
                         "Set to True when using B904 ME1/1 data.")
options.register("useB904ME21", False, VarParsing.multiplicity.singleton, VarParsing.varType.bool,
                         "Set to True when using B904 ME2/1 data (also works for ME3/1 and ME4/1).")
options.register("useB904ME234s2", False, VarParsing.multiplicity.singleton, VarParsing.varType.bool,
                         "Set to True when using B904 ME1/1 data (also works for MEX/2 and ME1/3).")
options.register("run3", True, VarParsing.multiplicity.singleton, VarParsing.varType.bool,
                         "Set to True when using Run-3 data.")
options.register("runCCLUTOTMB", False, VarParsing.multiplicity.singleton, VarParsing.varType.bool,
                         "Set to True when using the CCLUT OTMB algorithm.")
options.register("runCCLUTTMB", False, VarParsing.multiplicity.singleton, VarParsing.varType.bool,
                         "Set to True when using the CCLUT TMB algorithm.")
options.register("runME11ILT", False, VarParsing.multiplicity.singleton, VarParsing.varType.bool,
                         "Set to True when running the GEM-CSC integrated local trigger algorithm in ME1/1.")
options.register("runME21ILT", False, VarParsing.multiplicity.singleton, VarParsing.varType.bool,
                         "Set to True when running the GEM-CSC integrated local trigger algorithm in ME2/1.")
options.register("preTriggerAnalysis", False, VarParsing.multiplicity.singleton, VarParsing.varType.bool,
                         "Set to True if you want to print out more details about CLCTs and LCTs in the offline CSC DQM module.")
options.register("dropNonMuonCollections", True, VarParsing.multiplicity.singleton, VarParsing.varType.bool,
                 "Option to drop most non-muon collections generally considered unnecessary for GEM/CSC analysis")
options.register("dqmOutputFile", "step_DQM.root", VarParsing.multiplicity.singleton, VarParsing.varType.string,
                 "Name of the DQM output file. Default: step_DQM.root")
options.register("saveEdmOutput", False, VarParsing.multiplicity.singleton, VarParsing.varType.bool,
                 "Set to True if you want to keep the EDM ROOT after unpacking and re-emulating.")
options.register("doGEMCSC", False, VarParsing.multiplicity.singleton, VarParsing.varType.bool,
                 "Set to True if you want to enable GEMCSC plotting.")
options.parseArguments()
process_era = Run3
if not options.run3:
          process_era = Run2_2018
from Configuration.StandardSequences.Eras import eras
process = cms.Process("CSCDisplay", process_era)

process.load('Configuration.StandardSequences.GeometryRecoDB_cff')
#process.load("Configuration/Geometry/GeometryIdeal2015Reco_cff")
#process.load("Configuration/Geometry/IdealGeometry_cff")
#process.load("Configuration/StandardSequences/Geometry_cff")
process.load("Configuration/StandardSequences/MagneticField_cff")
process.load("Configuration/StandardSequences/FrontierConditions_GlobalTag_cff")
#process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_condDBv2_cff')
#process.load("Configuration/StandardSequences/RawToDigi_Data_cff")
process.load("Configuration.StandardSequences.Reconstruction_cff")
process.load("RecoMuon.MuonSeedGenerator.standAloneMuonSeeds_cff")
#process.load("RecoMuon.GlobalMuonProducer.globalMuons_cff")
process.load('Configuration.StandardSequences.DQMSaverAtRunEnd_cff')
process.load('Configuration.StandardSequences.EndOfProcess_cff')
process.load('Configuration.EventContent.EventContent_cff')
process.load("EventFilter.CSCRawToDigi.cscUnpacker_cfi")
process.load('EventFilter.GEMRawToDigi.muonGEMDigis_cfi')
process.load('EventFilter.L1TRawToDigi.emtfStage2Digis_cfi')
process.load("L1Trigger.CSCTriggerPrimitives.cscTriggerPrimitiveDigis_cfi")
process.load("CalibMuon.CSCCalibration.CSCL1TPLookupTableEP_cff")
process.load('L1Trigger.L1TGEM.simGEMDigis_cff')
process.load("DQM.L1TMonitor.L1TdeCSCTPG_cfi")
process.load("DQM.L1TMonitor.L1TdeGEMTPG_cfi")


process.options = cms.untracked.PSet(
	SkipEvent = cms.untracked.vstring('LogicError','ProductNotFound')
)


process.maxEvents = cms.untracked.PSet(
     input = cms.untracked.int32(-1)
)


process.source = cms.Source(
      "PoolSource",
      #skipEvents=cms.untracked.uint32(options.skipEvents),
      eventsToProcess = cms.untracked.VEventRange(),
      fileNames = cms.untracked.vstring(options.inputFiles),
      inputCommands = cms.untracked.vstring(
            'keep *',
            'drop CSCDetIdCSCShowerDigiMuonDigiCollection_simCscTriggerPrimitiveDigis_*_*'
      )
)

## this line is needed to run the GEM unpacker on output from AMC13SpyReadout.py or readFile_b904_Run3.py
if options.unpackGEM:
      process.source.labelRawDataLikeMC = cms.untracked.bool(False)

## global tag (data or MC, Run-2 or Run-3)
from Configuration.AlCa.GlobalTag import GlobalTag
if options.mc:
      process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:run2_mc', '')
      if options.run3:
            #process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:phase1_2021_realistic', '')
            process.GlobalTag = GlobalTag(process.GlobalTag, '123X_mcRun3_2021_realistic_v14', '')
else:
      process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:run2_data', '')
      if options.run3:
            process.GlobalTag = GlobalTag(process.GlobalTag, '112X_dataRun3_Prompt_v5', '')
#process.GlobalTag.globaltag = '74X_dataRun2_Prompt_v0'
#process.GlobalTag.globaltag = '92X_dataRun2_Prompt_v11'
#process.GlobalTag.globaltag = '102X_dataRun2_Prompt_v1'

## running on unpacked data, or after running the unpacker
if not options.mc or options.unpack:
      process.cscTriggerPrimitiveDigis.CSCComparatorDigiProducer = "muonCSCDigis:MuonCSCComparatorDigi"
      process.cscTriggerPrimitiveDigis.CSCWireDigiProducer = "muonCSCDigis:MuonCSCWireDigi"

## unpacker
useB904Data = options.useB904ME11 or options.useB904ME21 or options.useB904ME234s2
if useB904Data:
      ## CSC
      process.muonCSCDigis.DisableMappingCheck = True
      process.muonCSCDigis.B904Setup = True
      process.muonCSCDigis.InputObjects = "rawDataCollectorCSC"
      if options.unpackGEM:
            process.muonCSCDigis.useGEMs = True
      ## GEM
      process.muonGEMDigis.InputLabel = "rawDataCollectorGEM"

## l1 emulator
#from L1Trigger.CSCTriggerPrimitives.params.showerParams_cosmic import showerPSet_cosmic
l1csc = process.cscTriggerPrimitiveDigis
if options.l1:
      l1csc.commonParam.runCCLUT_OTMB = cms.bool(options.runCCLUTOTMB)
      l1csc.commonParam.runCCLUT_TMB = cms.bool(options.runCCLUTTMB)
      l1csc.commonParam.runME11ILT = options.runME11ILT
      l1csc.commonParam.runME21ILT = options.runME21ILT
      l1csc.tmbPhase2.tmbReadoutEarliest2 = cms.bool(False)
      l1csc.tmbPhase2GE11.tmbReadoutEarliest2 = cms.bool(False)
      ## running on unpacked data, or after running the unpacker
      if (not options.mc or options.unpack):
            l1csc.CSCComparatorDigiProducer = "muonCSCDigis:MuonCSCComparatorDigi"
            l1csc.CSCWireDigiProducer = "muonCSCDigis:MuonCSCWireDigi"
            ## GEM-CSC trigger enabled
            if options.runME11ILT or options.runME21ILT:
                  l1csc.GEMPadDigiClusterProducer = "muonCSCDigis:MuonGEMPadDigiCluster"

if options.l1GEM:
      process.simMuonGEMPadDigis.InputCollection = 'muonGEMDigis'

#####Run2 emulator 
process.simCscTriggerPrimitiveDigisRun2 = process.cscTriggerPrimitiveDigis.clone()
process.simCscTriggerPrimitiveDigisRun2.commonParam.runME11Up = cms.bool(False)
process.simCscTriggerPrimitiveDigisRun2.commonParam.runME21Up = cms.bool(False)
process.simCscTriggerPrimitiveDigisRun2.commonParam.runME31Up = cms.bool(False)
process.simCscTriggerPrimitiveDigisRun2.commonParam.runME41Up = cms.bool(False)
process.simCscTriggerPrimitiveDigisRun2.commonParam.runCCLUT = cms.bool(False)
process.simCscTriggerPrimitiveDigisRun2.commonParam.runCCLUT_OTMB = cms.bool(False)
process.simCscTriggerPrimitiveDigisRun2.commonParam.runCCLUT_TMB = cms.bool(False)
process.simCscTriggerPrimitiveDigisRun2.commonParam.runME11ILT = False
process.simCscTriggerPrimitiveDigisRun2.commonParam.run3 = False
process.simCscTriggerPrimitiveDigisRun2.commonParam.runPhase2 = False
process.simCscTriggerPrimitiveDigisRun2.clctPhase1.verbosity = cms.int32(0)
process.simCscTriggerPrimitiveDigisRun2.alctPhase1.verbosity = cms.int32(0)
process.simCscTriggerPrimitiveDigisRun2.tmbPhase1.verbosity = cms.int32(0)
process.simCscTriggerPrimitiveDigisRun2.tmbPhase1.tmbReadoutEarliest2 = cms.bool(False)
#print("Run2 emulation, common parameter ", process.simCscTriggerPrimitiveDigisRun2.commonParam)
#print("Run2 emulation, CLCT parameter ", process.simCscTriggerPrimitiveDigisRun2.clctPhase1)
process.simCscTriggerPrimitiveDigisRun2.commonParam.GEMPadDigiClusterProducer = cms.InputTag("simMuonGEMPadDigiClusters")

## Run-3 patterns with CCLUT, with ILT, no deadtime zone
process.simCscTriggerPrimitiveDigisRun3CCLUTv0 = process.cscTriggerPrimitiveDigis.clone()
process.simCscTriggerPrimitiveDigisRun3CCLUTv0.commonParam.runME11Up = cms.bool(True)
process.simCscTriggerPrimitiveDigisRun3CCLUTv0.commonParam.runME21Up = cms.bool(True)
process.simCscTriggerPrimitiveDigisRun3CCLUTv0.commonParam.runME31Up = cms.bool(True)
process.simCscTriggerPrimitiveDigisRun3CCLUTv0.commonParam.runME41Up = cms.bool(True)
process.simCscTriggerPrimitiveDigisRun3CCLUTv0.commonParam.runCCLUT = cms.bool(True)
process.simCscTriggerPrimitiveDigisRun3CCLUTv0.commonParam.runCCLUT_OTMB = cms.bool(True)
process.simCscTriggerPrimitiveDigisRun3CCLUTv0.commonParam.runCCLUT_TMB = cms.bool(False)
process.simCscTriggerPrimitiveDigisRun3CCLUTv0.commonParam.runME11ILT = True
process.simCscTriggerPrimitiveDigisRun3CCLUTv0.commonParam.run3 = True
process.simCscTriggerPrimitiveDigisRun3CCLUTv0.commonParam.runPhase2 = True
process.simCscTriggerPrimitiveDigisRun3CCLUTv0.commonParam.GEMPadDigiClusterProducer = cms.InputTag("simMuonGEMPadDigiClusters")
process.simCscTriggerPrimitiveDigisRun3CCLUTv0.clctPhase2.useDeadTimeZoning = False
#print("Run3 CCLUT nodeadtime, common parameter ", process.simCscTriggerPrimitiveDigisRun3CCLUTv0.commonParam)
#print("CLCT parameter ", process.simCscTriggerPrimitiveDigisRun3CCLUTv0.clctPhase2)
#print("CLCT parameter ", process.simCscTriggerPrimitiveDigisRun3CCLUTv0.clctPhase2GEM)

## DQM monitor
if options.dqm:
      process.l1tdeCSCTPG.useB904ME11 = options.useB904ME11
      process.l1tdeCSCTPG.useB904ME21 = options.useB904ME21
      process.l1tdeCSCTPG.useB904ME234s2 = options.useB904ME234s2
      process.l1tdeCSCTPG.emulALCT = "cscTriggerPrimitiveDigis"
      process.l1tdeCSCTPG.emulCLCT = "cscTriggerPrimitiveDigis"
      process.l1tdeCSCTPG.emulLCT = "cscTriggerPrimitiveDigis:MPCSORTED"
      process.l1tdeCSCTPG.preTriggerAnalysis = options.preTriggerAnalysis

if options.dqmGEM:
      ## GEM pad clusters from the EMTF
      if options.useEmtfGEM:
            process.l1tdeGEMTPG.data = "emtfStage2Digis"
      ## GEM pad clusters from the CSC TPG
      else:
            process.l1tdeGEMTPG.data = "muonCSCDigis"
      ## GEM pad clusters from the GEM TPG
      process.l1tdeGEMTPG.emul = "simMuonGEMPadDigiClusters"


# Output
process.output = cms.OutputModule(
    "PoolOutputModule",
      outputCommands = cms.untracked.vstring(
            ['keep *',
             'drop *_rawDataCollector_*_*',
      ]),
      fileName = cms.untracked.string("cscdisplay_edmout.root"),
)

## for most studies, you don't need these collections.
## adjust as necessary
if options.dropNonMuonCollections:
      outputCom = process.output.outputCommands
      outputCom.append('drop *_rawDataCollector_*_*')
      outputCom.append('drop *_sim*al*_*_*')
      outputCom.append('drop *_hlt*al*_*_*')
      outputCom.append('drop *_g4SimHits_*al*_*')
      outputCom.append('drop *_simSi*_*_*')
      outputCom.append('drop *_hltSi*_*_*')
      outputCom.append('drop *_simBmtfDigis_*_*')
      outputCom.append('drop *_*_*BMTF*_*')
      outputCom.append('drop *_hltGtStage2ObjectMap_*_*')
      outputCom.append('drop *_simGtStage2Digis_*_*')
      outputCom.append('drop *_hltTriggerSummary*_*_*')

## DQM output
process.DQMoutput = cms.OutputModule("DQMRootOutputModule",
    dataset = cms.untracked.PSet(
        dataTier = cms.untracked.string('DQMIO'),
        filterName = cms.untracked.string('')
    ),
    fileName = cms.untracked.string('file:{}'.format(options.dqmOutputFile)),
    outputCommands = process.DQMEventContent.outputCommands,
    splitLevel = cms.untracked.int32(0)
)

## helper for files on dCache/EOS (LPC)
#def useInputDir(process, inputDir, onEOS = True):
#    theInputFiles = []
#    for d in range(len(inputDir)):
#        my_dir = inputDir[d]
#        if not os.path.isdir(my_dir):
#            print "ERROR: This is not a valid directory: ", my_dir
#            if d==len(inputDir)-1:
#                print "ERROR: No input files were selected"
#                exit()
#            continue
#        print "Proceed to next directory"
#        ls = os.listdir(my_dir)
#        if onEOS:
#            theInputFiles.extend(['file:' + my_dir[:] + x for x in ls if x.endswith('root')])
#        else:
#            ## this works only if you pass the location on pnfs - FIXME for files staring with store/user/...                                                            
#            theInputFiles.extend([my_dir[16:] + x for x in ls if x.endswith('root')])
#
#    process.source.fileNames = cms.untracked.vstring(*theInputFiles)
#    return process

#inputdirs = ["/eos/cms/store/data/Run2018D/SingleMuon/RAW/v1/000/323/524/00000/"]
#useInputDir(process, inputdirs)

print("===============================================")
print("input files: ")
print(process.source.fileNames)
print("===============================================")
print("List of event for display", options.eventListFile)
if os.path.isfile(options.eventListFile):
    os.system("cat "+options.eventListFile)
else:
    print("file not found , exit()")
    exit()

##eventList.txt example:
##run eventnumber endcap station ring chamber
## endcap =1: positive, =2:minus
## ME1a and ME1b use ring=1
eventlist_display = options.eventListFile
fevents = open(eventlist_display, 'r')

totEvents = 0
evlist = []
for line in fevents:
   if line.startswith("===="): break
   if not line[0].isdigit(): continue
   runnumber = int(line.split()[0])
   eventnumber = int(line.split()[1])
   run_event_str = "%d:%d"%(runnumber, eventnumber)
   if run_event_str not in evlist:
       evlist.append(run_event_str)
   else: continue
   process.source.eventsToProcess.append('%d:%d-%d:%d'%(runnumber, eventnumber, runnumber, eventnumber))
   totEvents = totEvents+1
   #print("run ",runnumber, "eventnumber ",eventnumber)
print("Events to run ", process.source.eventsToProcess)
#process.maxEvents.input = totEvents
print("process.maxEvents ", process.maxEvents.input)
fevents.close()

#process.source.duplicateCheckMode = cms.untracked.string('noDuplicateCheck')
##scram b -j8 USER_CXXFLAGS="-DEDM_ML_DEBUG"
#process.MessageLogger = cms.Service("MessageLogger",
#    destinations = cms.untracked.vstring("displaydebug"),
#    displaydebug = cms.untracked.PSet(
#        extension = cms.untracked.string(".txt"),
#        threshold = cms.untracked.string("DEBUG"),
#        lineLength = cms.untracked.int32(132),
#        noLineBreaks = cms.untracked.bool(True)
#    ),
#    # debugModules = cms.untracked.vstring("*")
#    debugModules = cms.untracked.vstring("cscTriggerPrimitiveDigis")
#)



# CSC Trigger Primitives reader

processName = process._Process__name
process.GifDisplay = cms.EDAnalyzer('GifDisplay',
#rootFileName = cms.untracked.string("output_me11_test27_oct30.root"),
rootFileName = cms.untracked.string("output.root"),

#cscRecHitTag = cms.InputTag("csc2DRecHits",""),
stripDigiTagSrc        = cms.untracked.InputTag("muonCSCDigis","MuonCSCStripDigi"),
wireDigiTagSrc         = cms.untracked.InputTag("muonCSCDigis","MuonCSCWireDigi"),
compDigiTagSrc         = cms.untracked.InputTag("muonCSCDigis", "MuonCSCComparatorDigi"),
alctDigiTagSrc         = cms.untracked.InputTag('muonCSCDigis', 'MuonCSCALCTDigi'),
clctDigiTagSrc         = cms.untracked.InputTag('muonCSCDigis', 'MuonCSCCLCTDigi'),
corrlctDigiTagSrc      = cms.untracked.InputTag('muonCSCDigis', 'MuonCSCCorrelatedLCTDigi'),
alctDigiTagSrc_Emul    = cms.untracked.InputTag('cscTriggerPrimitiveDigis'),
clctDigiTagSrc_Emul    = cms.untracked.InputTag('cscTriggerPrimitiveDigis'),
corrlctDigiTagSrc_Emul = cms.untracked.InputTag('cscTriggerPrimitiveDigis'),

simHitTagSrc           = cms.untracked.InputTag(""),
gemsimHitTagSrc        = cms.untracked.InputTag(""),
gemPadTagSrc           = cms.untracked.InputTag(""),
gemPadClusterTagSrc    = cms.untracked.InputTag(""),

addEmulation = cms.untracked.bool(options.l1 or options.l1GEM),
addSimHits = cms.untracked.bool(True),
doGEMDisplay = cms.untracked.bool(options.runME11ILT or options.runME21ILT or options.l1GEM),
doGEMCSC = cms.untracked.bool(options.doGEMCSC),  

debug = cms.untracked.int32(1),
eventList = cms.untracked.string(eventlist_display),
#directory for eventdisplay
eventDisplayDir = cms.untracked.string(options.plotdir)
)

if options.mc:
    process.GifDisplay.simHitTagSrc           = cms.untracked.InputTag("g4SimHits", "MuonCSCHits")
    process.GifDisplay.gemsimHitTagSrc        = cms.untracked.InputTag("g4SimHits", "MuonGEMHits")
    process.GifDisplay.stripDigiTagSrc        = cms.untracked.InputTag("simMuonCSCDigis", "MuonCSCStripDigi")
    process.GifDisplay.wireDigiTagSrc         = cms.untracked.InputTag("simMuonCSCDigis", "MuonCSCWireDigi")
    process.GifDisplay.compDigiTagSrc         = cms.untracked.InputTag("simMuonCSCDigis", "MuonCSCComparatorDigi")
    #process.GifDisplay.alctDigiTagSrc         = cms.untracked.InputTag('simCscTriggerPrimitiveDigis', '')
    #process.GifDisplay.clctDigiTagSrc         = cms.untracked.InputTag('simCscTriggerPrimitiveDigis', '')
    #process.GifDisplay.corrlctDigiTagSrc      = cms.untracked.InputTag('simCscTriggerPrimitiveDigis', '')
    process.GifDisplay.alctDigiTagSrc         = cms.untracked.InputTag('simCscTriggerPrimitiveDigisRun2', '', processName)
    process.GifDisplay.clctDigiTagSrc         = cms.untracked.InputTag('simCscTriggerPrimitiveDigisRun2', '', processName)
    process.GifDisplay.corrlctDigiTagSrc      = cms.untracked.InputTag('simCscTriggerPrimitiveDigisRun2', '', processName)
    #process.GifDisplay.alctDigiTagSrc         = cms.untracked.InputTag('simCscTriggerPrimitiveDigisRun3CCLUTv0', '', processName)
    #process.GifDisplay.clctDigiTagSrc         = cms.untracked.InputTag('simCscTriggerPrimitiveDigisRun3CCLUTv0', '', processName)
    #process.GifDisplay.corrlctDigiTagSrc      = cms.untracked.InputTag('simCscTriggerPrimitiveDigisRun3CCLUTv0', '', processName)
    process.GifDisplay.alctDigiTagSrc_Emul    = cms.untracked.InputTag('cscTriggerPrimitiveDigis', "",processName)
    process.GifDisplay.clctDigiTagSrc_Emul    = cms.untracked.InputTag('cscTriggerPrimitiveDigis', "",processName)
    process.GifDisplay.corrlctDigiTagSrc_Emul = cms.untracked.InputTag('cscTriggerPrimitiveDigis', "",processName)
if options.runME11ILT or options.runME21ILT or options.l1GEM:
    #process.GifDisplay.gemPadTagSrc          = cms.untracked.InputTag('MuonGEMPadDigis')
    process.GifDisplay.gemPadTagSrc          = cms.untracked.InputTag('simMuonGEMPadDigis')
    process.GifDisplay.gemPadClusterTagSrc   = cms.untracked.InputTag('simMuonGEMPadDigiClusters')

#process.p = cms.Path(process.muonCSCDigis * process.cscTriggerPrimitiveDigis * process.GifDisplay)
process.display = cms.Path(process.GifDisplay)
## schedule and path definition
#process.muonCSCDigis.selectedChambers = options.selectedChambers
process.unpacksequence = cms.Sequence(process.muonCSCDigis)

if options.unpackGEM:
      ## unpack GEM strip digis
      process.unpacksequence += process.muonGEMDigis
      ## unpack GEM pad clusters from the EMTF
      if options.useEmtfGEM:
            process.unpacksequence += process.emtfStage2Digis
process.p1 = cms.Path(process.unpacksequence)

#process.l1sequence = cms.Sequence(l1csc)
#if options.l1GEM:
#      ## not sure if append would work for the GEM-CSC trigger
#      ## maybe the modules need to come first
#      process.l1sequence += process.simMuonGEMPadDigis
#      process.l1sequence += process.simMuonGEMPadDigiClusters
#redefine the l1 sequence 
process.l1sequence = cms.Sequence(
        process.simMuonGEMPadDigis *
        process.simMuonGEMPadDigiClusters *
        l1csc *
        process.simCscTriggerPrimitiveDigisRun2
        #process.simCscTriggerPrimitiveDigisRun3CCLUTv0
        )

process.p2 = cms.Path(process.l1sequence)

process.dqmsequence = cms.Sequence(process.l1tdeCSCTPG)
if options.dqmGEM:
      process.dqmsequence += process.l1tdeGEMTPG
process.p3 = cms.Path(process.dqmsequence)

process.p4 = cms.EndPath(process.DQMoutput)
process.p5 = cms.EndPath(process.output)
process.p6 = cms.EndPath(process.endOfProcess)

process.schedule = cms.Schedule()
## add the unpacker
if options.unpack:
      process.schedule.extend([process.p1])

## add the emulator
if options.l1:
      process.schedule.extend([process.p2])

process.schedule.extend([process.display])


## output step
## add DQM step 1
if options.dqm:
      process.schedule.extend([process.p3, process.p4])
if options.saveEdmOutput:
      process.schedule.extend([process.p5])

process.schedule.extend([process.p6])
print("All scheduled processes: ", process.schedule)
