import FWCore.ParameterSet.Config as cms

process = cms.Process("CSCPrint")

process.load("FWCore.MessageService.MessageLogger_cfi")
process.MessageLogger = cms.Service("MessageLogger",
    destinations = cms.untracked.vstring('cout', 'cerr'),
    cout = cms.untracked.PSet(
        threshold = cms.untracked.string('INFO'),
        default   = cms.untracked.PSet(limit = cms.untracked.int32(1000000))
    ),
    cerr = cms.untracked.PSet(
        threshold = cms.untracked.string('WARNING'),
        default   = cms.untracked.PSet(limit = cms.untracked.int32(1000000))
    )
)

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(10)  # set to -1 to run all events
)

process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(
        #"root://cmsxrootd.fnal.gov//store/mc/Phase2Spring24DIGIRECOMiniAOD/DYToLL_M-10To50_TuneCP5_14TeV-pythia8/GEN-SIM-DIGI-RAW-MINIAOD/PU140_Trk1GeV_140X_mcRun4_realistic_v4_ext1-v1/70000/0f1a4693-63c6-461d-b2be-7c966f3c6e37.root"
        "file:/eos/user/y/yumeng/CMSSW_14_2_0_pre1/gifDisplay/Phase2Spring24_DYToLL_M-10To50/0f1a4693-63c6-461d-b2be-7c966f3c6e37.root"
    )
)

process.CSCEventPrinter = cms.EDAnalyzer("CSCEventPrinter",
    clctTag = cms.InputTag("simCscTriggerPrimitiveDigis", "", "HLT")
)

process.p = cms.Path(process.CSCEventPrinter)
