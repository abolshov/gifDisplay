import FWCore.ParameterSet.Config as cms

process = cms.Process("TEST")

# --- basic setup
process.load("FWCore.MessageService.MessageLogger_cfi")
process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(5))
process.MessageLogger.cerr.FwkReport.reportEvery = 1

inputFile = "file:/eos/user/y/yumeng/CMSSW_13_0_14/mu_pt100_phase2fall22_el9/step2_local500.root"
#inputFile = "file:/eos/cms/store/relval/CMSSW_14_0_0/RelValSingleMuPt1000/GEN-SIM-DIGI-RAW/140X_mcRun3_2023_realistic_v3_STD_2023_noPU-v1/2580000/70e46844-1ef6-4259-b8bf-2e505011bef9.root"
process.source = cms.Source("PoolSource",
    #fileNames = cms.untracked.vstring('file:step2bis_phase2.root'),
    fileNames = cms.untracked.vstring(inputFile),
)

# --- analyzer (class name matches DEFINE_FWK_MODULE)
process.cscIndexCheck = cms.EDAnalyzer(
    'CSCIndexCheck',
    wiresTag = cms.InputTag('simMuonCSCDigis','MuonCSCWireDigi','L1'),
    stripsTag = cms.InputTag('simMuonCSCDigis','MuonCSCStripDigi','L1'),
    compsTag  = cms.InputTag('simMuonCSCDigis','MuonCSCComparatorDigi','L1'),
    clctTag   = cms.InputTag('simCscTriggerPrimitiveDigis','','L1'),
    sampleN   = cms.untracked.uint32(10)   #how many values to print
)

process.p = cms.Path(process.cscIndexCheck)

