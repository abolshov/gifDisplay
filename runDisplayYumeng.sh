#voms-proxy-init --voms cms
#root -l root://cmsxrootd.fnal.gov/" "root -l root://cms-xrd-global.cern.ch/"


#cmsRun CSCEventDisplay.py mc=true l1=true run3=true runCCLUTOTMB=true runME11ILT=true saveEdmOutput=false inputFiles=file:/eos/user/t/tahuang/RelValSamples/CMSSW_12_4_0_pre3/1000GeV/27a95851-6358-485b-b15b-619f3404d795.root  plotdir="/eos/user/t/tahuang/CSCEmulation/displayTest2022/" eventListFile="eventList.txt"

#20250508 success
#cmsRun CSCEventDisplay.py mc=true l1=true run3=true runCCLUTOTMB=true runME11ILT=true saveEdmOutput=false inputFiles=file:/eos/user/s/sgottlie/RelValSamples/CMSSW_12_4_0_pre3/1000GeV/27a95851-6358-485b-b15b-619f3404d795.root  plotdir="./plots" eventListFile="eventList_yumeng.txt"
#cmsRun CSCEventDisplay.py mc=true l1=true run3=true runCCLUTOTMB=true runME11ILT=true saveEdmOutput=false doGEMCSC=true inputFiles=file:/eos/user/s/sgottlie/RelValSamples/CMSSW_12_4_0_pre3/1000GeV/27a95851-6358-485b-b15b-619f3404d795.root  plotdir="./plots/temp" eventListFile="eventList_yumeng.txt"
#cmsRun CSCEventDisplay.py mc=true l1=true run3=true runCCLUTOTMB=true runME11ILT=true saveEdmOutput=false doGEMCSC=true inputFiles=file:/eos/user/y/yumeng/CMSSW_13_0_3/gifDisplay/27a95851-6358-485b-b15b-619f3404d795.root  plotdir="./plots/temp" eventListFile="eventList_sw1303eg.txt"

#cmsRun CSCEventDisplay.py mc=true l1=true run3=true runCCLUTOTMB=true runME11ILT=true saveEdmOutput=false doGEMCSC=true inputFiles="root://cmsxrootd.fnal.gov//store/mc/Phase2Spring24DIGIRECOMiniAOD/DYToLL_M-50_TuneCP5_14TeV-pythia8/GEN-SIM-DIGI-RAW-MINIAOD/PU200_Trk1GeV_140X_mcRun4_realistic_v4-v1/2810000/0027a144-58e8-4e80-a3a2-7ea8f5de45dd.root"  plotdir="./plots/temp" eventListFile="eventList_yumeng.txt"
cmsRun CSCEventDisplay.py mc=true l1=true run3=true runCCLUTOTMB=true runME11ILT=true saveEdmOutput=false doGEMCSC=true inputFiles=file:/eos/cms/store/group/upgrade/GEMCSC_Trigger_PhaseII/Phase2Spring24_DYtoLL50_00d881de-dccf-4232-b4a7-39083949f5fd.root plotdir="./plots/temp" eventListFile="badeventList_Ph2Spring24DY50_type0.txt"

#file1
#cmsRun CSCEventDisplay.py mc=true l1=true run3=true runCCLUTOTMB=true runME11ILT=true saveEdmOutput=false doGEMCSC=true inputFiles="root://cmsxrootd.fnal.gov//store/mc/Phase2Spring24DIGIRECOMiniAOD/DYToLL_M-10To50_TuneCP5_14TeV-pythia8/GEN-SIM-DIGI-RAW-MINIAOD/PU140_Trk1GeV_140X_mcRun4_realistic_v4_ext1-v1/70000/0f1a4693-63c6-461d-b2be-7c966f3c6e37.root" plotdir="./plots/temp" eventListFile="eventList_file1.txt"
#cmsRun CSCEventDisplay.py mc=true l1=true run3=true runCCLUTOTMB=true runME11ILT=true saveEdmOutput=false doGEMCSC=true inputFiles=file:/eos/user/y/yumeng/CMSSW_14_2_0_pre1/gifDisplay/Phase2Spring24_DYToLL_M-10To50/0f1a4693-63c6-461d-b2be-7c966f3c6e37.root plotdir="./plots/temp" eventListFile="eventList_file1eg.txt"

#cmsRun CSCEventDisplay.py mc=true l1=true run3=true runCCLUTOTMB=true runME11ILT=true saveEdmOutput=false doGEMCSC=true inputFiles="root://cmsxrootd.fnal.gov//store/mc/Phase2Fall22DRMiniAOD/SingleMuon_Pt-0To200_Eta-1p4To3p1-gun/GEN-SIM-DIGI-RAW-MINIAOD/PU200_125X_mcRun4_realistic_v2-v1/30000/004ac631-8090-4eac-b87f-24497a31651d.root"  plotdir="./plots/temp" eventListFile="eventList.txt"