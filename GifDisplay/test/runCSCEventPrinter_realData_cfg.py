import FWCore.ParameterSet.Config as cms

process = cms.Process("CSCPrintData")

# MessageLogger: mirror your MC config, add our analyzer category
process.load("FWCore.MessageService.MessageLogger_cfi")
process.MessageLogger = cms.Service("MessageLogger",
    destinations = cms.untracked.vstring('cout', 'cerr'),
    categories   = cms.untracked.vstring('CSCEventPrinterData'),
    cout = cms.untracked.PSet(
        threshold = cms.untracked.string('INFO'),
        default   = cms.untracked.PSet(limit = cms.untracked.int32(0)),
        CSCEventPrinterData = cms.untracked.PSet(limit = cms.untracked.int32(1000000))
    ),
    cerr = cms.untracked.PSet(
        threshold = cms.untracked.string('WARNING'),
        default   = cms.untracked.PSet(limit = cms.untracked.int32(1000000))
    )
)

process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(-1))

process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(
        "file:/eos/user/y/yumeng/CMSSW_14_2_0_pre1/gifDisplay/realDataDevin_forYumeng/pickevents.root"
    )
)

# Use segments first, then fall back to 2D rechits (all from your edmDump lines)
process.pr = cms.EDAnalyzer("CSCEventPrinterData",
    segmentTag  = cms.InputTag("cscSegments",  "", "RECO"),
    rechit2DTag = cms.InputTag("csc2DRecHits","", "RECO")
)

process.p = cms.Path(process.pr)
