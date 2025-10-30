import FWCore.ParameterSet.Config as cms
from Configuration.AlCa.GlobalTag import GlobalTag

process = cms.Process("CSCinfo")

# --- geometry loads (D110) ---
process.load('Configuration.Geometry.GeometryExtended2026D110Reco_cff')
process.load('Configuration.Geometry.GeometryExtended2026D110_cff')

# --- Standard sequences ---
process.load("Configuration.StandardSequences.MagneticField_cff")
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")
process.GlobalTag = GlobalTag(process.GlobalTag, "auto:phase2_realistic", "")

# --- CSC geometry + numbering (needed to query wire groups) ---
process.load("Geometry.MuonNumbering.muonNumberingInitialization_cfi")
process.load("Geometry.CSCGeometryBuilder.cscGeometry_cfi")
process.CSCGeometryESModule.applyAlignment = False  # keep geometry ideal

# If GlobalPositionRcd error again, keep this ON:
#process.load("Alignment.CommonAlignmentProducer.GlobalPosition_Fake_cff")

# --- Boilerplate source / events ---
process.source = cms.Source("EmptySource")
process.maxEvents = cms.untracked.PSet(input=cms.untracked.int32(1))

# --- Built-in analyzer that prints CSC geometry info ---
#process.printCSCs = cms.EDAnalyzer("CSCGeometryAnalyzer")
#process.p = cms.Path(process.printCSCs)

# --- Custom analyzer to print only wire group numbers ---
process.printWireGroups = cms.EDAnalyzer("CSCGeometryESModuleAnalyzer")

process.add_(cms.Service("InitRootHandlers", EnableIMT = cms.untracked.bool(False)))

process.analyzer = cms.EDAnalyzer("CSCWireGroupPrinter")

process.CSCWireGroupPrinter = cms.EDAnalyzer("CSCWireGroupPrinter")
process.p = cms.Path(process.CSCWireGroupPrinter)
