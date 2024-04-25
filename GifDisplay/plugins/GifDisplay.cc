//# -*- C++ -*-
// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/ConsumesCollector.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "Geometry/Records/interface/GlobalTrackingGeometryRecord.h"
#include "Geometry/CommonDetUnit/interface/GlobalTrackingGeometry.h"
#include "Geometry/CommonDetUnit/interface/GeomDet.h"



#include <Geometry/CSCGeometry/interface/CSCGeometry.h> 
#include <Geometry/Records/interface/MuonGeometryRecord.h> 
#include <RecoMuon/Records/interface/MuonRecoGeometryRecord.h> 
#include "DataFormats/CSCRecHit/interface/CSCRecHit2D.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "CondFormats/AlignmentRecord/interface/GlobalPositionRcd.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include <DataFormats/GeometrySurface/interface/LocalError.h> 
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

#include <DataFormats/Provenance/interface/EventID.h> 

#include "SimDataFormats/TrackingHit/interface/PSimHitContainer.h"
#include "DataFormats/CSCDigi/interface/CSCWireDigi.h"
#include "DataFormats/CSCDigi/interface/CSCWireDigiCollection.h"
#include "DataFormats/CSCDigi/interface/CSCStripDigi.h"
#include "DataFormats/CSCDigi/interface/CSCStripDigiCollection.h"
#include "DataFormats/CSCDigi/interface/CSCComparatorDigi.h"
#include "DataFormats/CSCDigi/interface/CSCComparatorDigiCollection.h"
#include "DataFormats/CSCDigi/interface/CSCALCTDigi.h"
#include "DataFormats/CSCDigi/interface/CSCALCTDigiCollection.h"
#include "DataFormats/CSCDigi/interface/CSCCLCTDigi.h"
#include "DataFormats/CSCDigi/interface/CSCCLCTDigiCollection.h"
#include "DataFormats/CSCDigi/interface/CSCCorrelatedLCTDigi.h"
#include "DataFormats/CSCDigi/interface/CSCCorrelatedLCTDigiCollection.h"

#include "DataFormats/GEMDigi/interface/GEMPadDigi.h"
#include "DataFormats/GEMDigi/interface/GEMPadDigiCollection.h"
#include "DataFormats/GEMDigi/interface/GEMPadDigiCluster.h"
#include "DataFormats/GEMDigi/interface/GEMPadDigiClusterCollection.h"
//add ME0 in future!
 
#include "DataFormats/MuonDetId/interface/CSCDetId.h"
#include "DataFormats/MuonDetId/interface/GEMDetId.h"
#include "DataFormats/MuonDetId/interface/ME0DetId.h"
#include "DataFormats/CSCRecHit/interface/CSCRecHit2D.h"
#include "DataFormats/CSCRecHit/interface/CSCSegmentCollection.h"
#include "DataFormats/MuonDetId/interface/RPCDetId.h"

#include "DataFormats/L1GlobalMuonTrigger/interface/L1MuGMTReadoutRecord.h"
#include "DataFormats/L1GlobalMuonTrigger/interface/L1MuGMTReadoutCollection.h"

#include "Geometry/CSCGeometry/interface/CSCGeometry.h"
#include "Geometry/CSCGeometry/interface/CSCChamber.h"
#include "Geometry/CSCGeometry/interface/CSCLayer.h"
#include "Geometry/CSCGeometry/interface/CSCLayerGeometry.h"
#include "Geometry/GEMGeometry/interface/GEMGeometry.h"
#include "Geometry/GEMGeometry/interface/GEMChamber.h"
#include "Geometry/Records/interface/MuonGeometryRecord.h"

#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "Geometry/CommonDetUnit/interface/GlobalTrackingGeometry.h"
#include "Geometry/Records/interface/GlobalTrackingGeometryRecord.h"

#include "DataFormats/CSCRecHit/interface/CSCRecHit2DCollection.h"
#include "SimDataFormats/TrackingHit/interface/PSimHitContainer.h"

#include "DataFormats/GeometryVector/interface/GlobalPoint.h"
#include "DataFormats/GeometryVector/interface/GlobalVector.h"
#include "DataFormats/GeometryVector/interface/LocalPoint.h"
#include "DataFormats/GeometryVector/interface/LocalVector.h"
#include "DataFormats/CLHEP/interface/AlgebraicObjects.h"
#include "DataFormats/MuonDetId/interface/CSCIndexer.h"

#include "CondFormats/CSCObjects/interface/CSCDBGains.h"
#include "CondFormats/DataRecord/interface/CSCDBGainsRcd.h"
#include "CondFormats/CSCObjects/interface/CSCDBNoiseMatrix.h"
#include "CondFormats/DataRecord/interface/CSCDBNoiseMatrixRcd.h"
#include "CondFormats/CSCObjects/interface/CSCDBCrosstalk.h"
#include "CondFormats/DataRecord/interface/CSCDBCrosstalkRcd.h"
#include "CondFormats/CSCObjects/interface/CSCDBPedestals.h"
#include "CondFormats/DataRecord/interface/CSCDBPedestalsRcd.h"
#include "CondFormats/Alignment/interface/Alignments.h"

#include "DataFormats/MuonReco/interface/MuonFwd.h"
#include "DataFormats/MuonReco/interface/Muon.h"

#include <TGClient.h>
#include <TRootEmbeddedCanvas.h>
#include "TCanvas.h"
#include "TPad.h"
#include "TStyle.h"

#include "TF1.h"
#include "TH1F.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TH1I.h"
#include "TProfile.h"
#include "TProfile2D.h"
#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TGraph.h"
#include <vector>
#include <cmath>
#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>

//#include "Gif.h"
#include "gifDisplay/GifDisplay/interface/display.h"
//##include "../interface/display.h"
// class declaration
//

// If the analyzer does not use TFileService, please remove
// the template argument to the base class so the class inherits
// from  edm::one::EDAnalyzer<> and also remove the line from
// constructor "usesResource("TFileService");"
// This will improve performance in multithreaded jobs.

class GifDisplay : public edm::one::EDAnalyzer<edm::one::SharedResources>  {
   public:
      explicit GifDisplay(const edm::ParameterSet&);
      ~GifDisplay();

      static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);


   private:
      virtual void beginJob() override;
      virtual void analyze(const edm::Event&, const edm::EventSetup&) override;
      virtual void endJob() override;

      void fillALCT(edm::Handle<CSCALCTDigiCollection> digicollection, vector<CSCIDLCTs>& alct_container);
      void fillCLCT(edm::Handle<CSCCLCTDigiCollection> digicollection, vector<CSCIDLCTs>& clct_container);
      void fillCorrLCT(edm::Handle<CSCCorrelatedLCTDigiCollection> digicollection, vector<CSCIDLCTs>& lct_container);
      void fillGEMCluster(GEMPadDigiClusterCollection gemclusters, vector<GEMCLUSTER>& gemcluster_container);
// ----------member data ---------------------------
//edm::InputTag stripDigiTag;
//edm::InputTag wireDigiTag;
//edm::InputTag comparatorDigiTag;
  edm::EDGetTokenT<edm::PSimHitContainer> simHitTagSrc;
  edm::EDGetTokenT<edm::PSimHitContainer> gemsimHitTagSrc;

  edm::EDGetTokenT<CSCWireDigiCollection> wireDigiTagSrc;
  edm::EDGetTokenT<CSCStripDigiCollection> stripDigiTagSrc;
  edm::EDGetTokenT<CSCComparatorDigiCollection> compDigiTagSrc;
  edm::EDGetTokenT<CSCALCTDigiCollection> alctDigiTagSrc;
  edm::EDGetTokenT<CSCCLCTDigiCollection> clctDigiTagSrc;
  edm::EDGetTokenT<CSCCorrelatedLCTDigiCollection> corrlctDigiTagSrc;

  edm::EDGetTokenT<GEMPadDigiCollection> gemPadTagSrc;
  edm::EDGetTokenT<GEMPadDigiClusterCollection> gemPadClusterTagSrc;

  edm::EDGetTokenT<CSCALCTDigiCollection> alctDigiTagSrc_emul;
  edm::EDGetTokenT<CSCCLCTDigiCollection> clctDigiTagSrc_emul;
  edm::EDGetTokenT<CSCCorrelatedLCTDigiCollection> corrlctDigiTagSrc_emul;

  edm::ESGetToken<CSCGeometry, MuonGeometryRecord> cscToken_;
  edm::ESGetToken<GEMGeometry, MuonGeometryRecord> gemToken_;

//edm::InputTag compDigiTag;
//edm::InputTag cscRecHitTag;
//edm::InputTag cscSegTag;
//edm::InputTag saMuonTag;
//edm::InputTag l1aTag;
//edm::InputTag alctDigiTag;
//edm::InputTag clctDigiTag;
//edm::InputTag corrlctDigiTag;
//edm::InputTag hltTag;

//std::string theRootFileName,pdf;
std::string eventDisplayDir;
std::vector<double> eventList;
std::vector<int> runList;
std::vector<int> endcapList;
std::vector<int> stationList;
std::vector<int> ringList;
std::vector<int> chamberList;
std::string eventlistFile;


//std::string chamberType;

int doDebug;
bool addEmulation;
bool addSimHits;
bool doGEMDisplay;
bool doGEMCSC;
};

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
GifDisplay::GifDisplay(const edm::ParameterSet& iConfig)

{
   
//now do what ever initialization is needed
//theRootFileName= iConfig.getUntrackedParameter<std::string>("rootFileName");
/*std::cout<<theRootFileName.c_str()<<"\n";
pdf=theRootFileName+".pdf";
fout=new TFile(theRootFileName.c_str(),"RECREATE");
fout->cd();
*/
//cscRecHitTag  = iConfig.getParameter<edm::InputTag>("cscRecHitTag");
//stripDigiTag  = iConfig.getParameter<edm::InputTag>("stripDigiTag");
//wireDigiTag  = iConfig.getParameter<edm::InputTag>("wireDigiTag");
//comparatorDigiTag = iConfig.getParameter<edm::InputTag>("comparatorDigiTag");
  simHitTagSrc=consumes<edm::PSimHitContainer>(iConfig.getUntrackedParameter<edm::InputTag>("simHitTagSrc")),
  gemsimHitTagSrc=consumes<edm::PSimHitContainer>(iConfig.getUntrackedParameter<edm::InputTag>("gemsimHitTagSrc")),
  wireDigiTagSrc=consumes<CSCWireDigiCollection>(iConfig.getUntrackedParameter<edm::InputTag>("wireDigiTagSrc")),
  stripDigiTagSrc=consumes<CSCStripDigiCollection>(iConfig.getUntrackedParameter<edm::InputTag>("stripDigiTagSrc")),
  compDigiTagSrc=consumes<CSCComparatorDigiCollection>(iConfig.getUntrackedParameter<edm::InputTag>("compDigiTagSrc")),
//alctDigiTag = iConfig.getParameter<edm::InputTag>("alctDigiTag");
  alctDigiTagSrc = consumes<CSCALCTDigiCollection>(iConfig.getUntrackedParameter<edm::InputTag>("alctDigiTagSrc"));
  clctDigiTagSrc = consumes<CSCCLCTDigiCollection>(iConfig.getUntrackedParameter<edm::InputTag>("clctDigiTagSrc"));
  corrlctDigiTagSrc = consumes<CSCCorrelatedLCTDigiCollection>(iConfig.getUntrackedParameter<edm::InputTag>("corrlctDigiTagSrc"));
  gemPadTagSrc = consumes<GEMPadDigiCollection>(iConfig.getUntrackedParameter<edm::InputTag>("gemPadTagSrc"));
  gemPadClusterTagSrc = consumes<GEMPadDigiClusterCollection>(iConfig.getUntrackedParameter<edm::InputTag>("gemPadClusterTagSrc"));

  alctDigiTagSrc_emul = consumes<CSCALCTDigiCollection>(iConfig.getUntrackedParameter<edm::InputTag>("alctDigiTagSrc_Emul"));
  clctDigiTagSrc_emul = consumes<CSCCLCTDigiCollection>(iConfig.getUntrackedParameter<edm::InputTag>("clctDigiTagSrc_Emul"));
  corrlctDigiTagSrc_emul = consumes<CSCCorrelatedLCTDigiCollection>(iConfig.getUntrackedParameter<edm::InputTag>("corrlctDigiTagSrc_Emul"));


  eventDisplayDir = iConfig.getUntrackedParameter<std::string>("eventDisplayDir","/home/mhl/public_html/2017/20171025_cscSeg/eventdisplay/");
  eventlistFile = iConfig.getUntrackedParameter<std::string>("eventList","eventList.txt");
  std::cout <<"Eventlist file "<< eventlistFile <<" outfolder "<< eventDisplayDir << (addEmulation ? " addEmulation":" NOEmulation") << std::endl;
 //eventlistFile = "eventList.txt";
  //chamberType = iConfig.getUntrackedParameter<std::string>("chamberType", "11");
  doDebug = iConfig.getUntrackedParameter<int>("debug", 0);
  addEmulation = iConfig.getUntrackedParameter<bool>("addEmulation", false);
  addSimHits = iConfig.getUntrackedParameter<bool>("addSimHits", false);
  doGEMDisplay = iConfig.getUntrackedParameter<bool>("doGEMDisplay", false);
  doGEMCSC = iConfig.getUntrackedParameter<bool>("doGEMCSC", false);  

  cscToken_ = consumesCollector().esConsumes<CSCGeometry, MuonGeometryRecord>();
  gemToken_ = consumesCollector().esConsumes<GEMGeometry, MuonGeometryRecord>();

  //vector<CSCDetID> chamberList;
  //chamberList.clear();
  endcapList.clear();
  stationList.clear();
  ringList.clear();
  chamberList.clear();
  eventList.clear();
  runList.clear();
  //==========================Get event list for display================
  ifstream file(eventlistFile);
  while (file){
      string line;
      if (not getline(file, line)) break;
      istringstream is(line);
      //while (is) {

      int run;
      is >> run;
      runList.push_back(run);

      double event;
      is >> event;
      eventList.push_back(event);

      int endcap; is >> endcap;
      endcapList.push_back(endcap);

      int station; is >> station;
      stationList.push_back(station);

      int ring; is >> ring;
      ringList.push_back(ring);

      int chamber; is >> chamber;
      chamberList.push_back(chamber);
      if (doDebug > 1)
            cout <<"Eventlist: run "<< run <<" event "<< event <<" endcap "<< endcap <<" station "<< station << " ring "<< ring <<" chamber "<< chamber << endl;

   }
}





GifDisplay::~GifDisplay()
{
 
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//

// ------------ method called for each event  ------------
void
GifDisplay::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
   using namespace edm;
   using namespace std;

   edm::ESHandle<CSCGeometry> cscGeom;
   cscGeom = &iSetup.getData(cscToken_);
   edm::ESHandle<GEMGeometry> gemGeom;
   gemGeom = &iSetup.getData(gemToken_);
   //iSetup.get<MuonGeometryRecord>().get(cscGeom);
   //edm::EventId evId=iEvent.id();
   //unsigned int time=iEvent.time().unixTime();

   double Run,Event;
   Run = iEvent.id().run();
   Event = iEvent.id().event();

   vector<SIMHIT>   simhit_container;
   vector<GEMSIMHIT>   gemsimhit_container;
   vector<GEMPAD>  gempad_container;
   vector<GEMCLUSTER> gemcluster_container;
   vector<WIRE>   wire_container;
   vector<STRIP>  strip_container;
   vector<COMPARATOR> com_container;
   //vector<CSCALCTDigi> alct_container;
   //vector<CSCCLCTDigi> clct_container;
   vector<CSCIDLCTs> alct_container;
   vector<CSCIDLCTs> alct_emul_container;
   vector<CSCIDLCTs> clct_container;
   vector<CSCIDLCTs> clct_emul_container;
   vector<CSCIDLCTs> lct_container;
   vector<CSCIDLCTs> lct_emul_container;

   if (doDebug > 0) 
      cout <<"this run "<< Run <<" this Event "<< Event << endl;
   if (find (runList.begin(), runList.end(), Run) == runList.end()) return;
   if (find (eventList.begin(), eventList.end(), Event) == eventList.end()) return;

   if (doDebug > 2) cout <<"start to process this event "<< Event << std::endl;
   
   //========================== PSimHits ========================
   if(addSimHits){
      edm::Handle<edm::PSimHitContainer> simHits;

      try{
         iEvent.getByToken(simHitTagSrc, simHits);
         bool simMuOnly_ = false;
         if (doDebug > 1) cout <<"Event "<<Event <<" collecting simHits "<< endl;
         for (const auto& sh : *simHits.product()){
            int pdgid = sh.particleType();
            if (simMuOnly_ && std::abs(pdgid) != 13) continue;
            const CSCDetId& id(sh.detUnitId()); //layer id 
            CSCDetID tmpID;
            SimHit tmpSH;

            tmpID.Endcap = id.endcap();
            tmpID.Ring = id.ring();//==4 ? 1 : id.ring();
            tmpID.Station = id.station();
            tmpID.Chamber = id.chamber();
            tmpID.Layer = id.layer();

            const LocalPoint& lp = sh.entryPoint();
            const auto& layerG(cscGeom->layer(sh.detUnitId())->geometry());
            int nearestWire(layerG->nearestWire(lp));
            tmpSH.WireGroup = layerG->wireGroup(nearestWire); 
            tmpSH.Strip = id.ring()==4 ? layerG->nearestStrip(lp)+64 : layerG->nearestStrip(lp);
            tmpSH.PdgId = abs(pdgid);
            tmpSH.Stripf = layerG->strip(lp);
            //cout <<"chamber "<< id <<" CSC simhit pdgid "<< pdgid <<" wg "<< tmpSH.WireGroup <<" strip "<< tmpSH.Strip << endl;
            bool found = false;
            for (auto& idsh : simhit_container){
               if (idsh.first == tmpID) {
                     idsh.second.emplace_back(tmpSH);
                     found = true;
                     //cout <<"found exsit id "<< tmpID <<" size of simhits "<< idsh.second.size() << std::endl;
                     break;
               }
            }
            if (not found){
               //cout <<"add new id to  simhit_container "<< id << endl;
               SIMHIT id_simhit_tmp;
               id_simhit_tmp.first  = tmpID;
               id_simhit_tmp.second.push_back(tmpSH);
               simhit_container.push_back(id_simhit_tmp); 
            }

         }

         //GEM SimHits
         edm::Handle<edm::PSimHitContainer> gemsimHits;
         iEvent.getByToken(gemsimHitTagSrc, gemsimHits);
         if (doDebug > 1) cout <<"Event "<<Event <<" collecting gem simHits "<< endl;
         for (const auto& sh : *gemsimHits.product()){
            int pdgid = sh.particleType();
            if (simMuOnly_ && std::abs(pdgid) != 13) continue;
            const GEMDetId& id(sh.detUnitId()); //layer id 
            GEMDetID tmpID;
            SimHit tmpSH; //SimHit for GEM/CSC

            tmpID.Endcap = id.region(); //-1 or +1
            tmpID.Ring = id.ring();//==4 ? 1 : id.ring();
            tmpID.Station = id.station();
            tmpID.Chamber = id.chamber();
            tmpID.Layer = id.layer();
            tmpID.Roll = id.roll();

            const LocalPoint& lp = sh.entryPoint();
            const auto& etaPart(gemGeom->etaPartition(GEMDetId(sh.detUnitId())));
            tmpSH.Stripf = etaPart->strip(lp); 
            tmpSH.Strip  = int(etaPart->strip(lp)+0.5); 
            tmpSH.PdgId = abs(pdgid);
            bool found = false;
            for (auto& idsh : gemsimhit_container){
               if (idsh.first == tmpID) {
                     idsh.second.emplace_back(tmpSH);
                     found = true;
                     //cout <<"found exsit id "<< tmpID <<" size of simhits "<< idsh.second.size() << std::endl;
                     break;
               }
            }
            if (not found){
               //cout <<"add new id to  simhit_container "<< id << endl;
               GEMSIMHIT id_simhit_tmp;
               id_simhit_tmp.first  = tmpID;
               id_simhit_tmp.second.push_back(tmpSH);
               gemsimhit_container.push_back(id_simhit_tmp); 
            }

         }


      } catch (cms::Exception){
         std::cout <<" failed to find CSC/GEM simhits, ignore "<< std::endl;
      }
   }

   if (doDebug > 2 and addSimHits){
      //for (auto id_sh : simhit_container)
      //   cout <<"CSC simtcontainer id "<< id_sh.first <<" size "<< id_sh.second.size() << endl;
      for (auto id_sh : gemsimhit_container)
         cout <<"GEM simtcontainer id "<< id_sh.first <<" size "<< id_sh.second.size() << endl;
   }

   //========================== WIRES ========================
   edm::Handle<CSCWireDigiCollection> wires;
   iEvent.getByToken(wireDigiTagSrc, wires);

   if (doDebug > 1) cout <<"Event "<<Event <<" collecting wire digi "<< endl;
   for (CSCWireDigiCollection::DigiRangeIterator wi=wires->begin(); wi!=wires->end(); wi++) {
      CSCDetId id = (CSCDetId)(*wi).first;
      std::vector<CSCWireDigi>::const_iterator wireIt = (*wi).second.first;
      std::vector<CSCWireDigi>::const_iterator lastWire = (*wi).second.second;

      CSCDetID tmpID;
      WIRE tmpWIRE;


      tmpID.Endcap = id.endcap();
      tmpID.Ring = id.ring();//==4 ? 1 : id.ring();
      tmpID.Station = id.station();
      tmpID.Chamber = id.chamber();
      tmpID.Layer = id.layer();


      for( ; wireIt != lastWire; ++wireIt){

         Wire tmpWG;

         tmpWG.WireGroup = wireIt->getWireGroup();
         tmpWG.TimeBin = wireIt->getTimeBin();
         tmpWG.NumberTimeBin = int((wireIt->getTimeBinsOn()).size());

         tmpWIRE.second.push_back(tmpWG);
         if (doDebug > 2) cout << id << endl;
         if (doDebug > 2) cout << "wg: " << tmpWG.WireGroup << ", time: " << tmpWG.TimeBin << endl;

         }// all wires
   
      tmpWIRE.first = tmpID;
      wire_container.push_back(tmpWIRE); 
   }	//all layers for wires		



   //========================== STRIPS ========================

   //edm::Handle<CSCStripDigiCollection> strips;
   //iEvent.getByLabel(stripDigiTag,strips);
   //edm::EDGetTokenT<CSCStripDigiCollection> s_token = consumes<CSCStripDigiCollection>(stripDigiTag);
   edm::Handle<CSCStripDigiCollection> strips;
   //iEvent.getByLabel(wireDigiTag,wires);
   iEvent.getByToken(stripDigiTagSrc, strips);

   if (doDebug > 1) cout <<"Event "<<Event <<" collecting strip digi "<< endl;
   for (CSCStripDigiCollection::DigiRangeIterator si=strips->begin(); si!=strips->end(); si++) {
      CSCDetId id = (CSCDetId)(*si).first;
      std::vector<CSCStripDigi>::const_iterator stripIt = (*si).second.first;
      std::vector<CSCStripDigi>::const_iterator lastStrip = (*si).second.second;

      CSCDetID tmpID;
      STRIP tmpSTRIP;

      tmpID.Endcap = id.endcap();
      tmpID.Ring = id.ring();//==4 ? 1 : id.ring();//if me11a, set its ring=1
      tmpID.Station = id.station();
      tmpID.Chamber = id.chamber();
      tmpID.Layer = id.layer();


      for( ; stripIt != lastStrip; ++stripIt) {

         Strips tmpSP;

         tmpSP.Strip = stripIt->getStrip();
         if (id.ring() == 4) tmpSP.Strip+=64;//for now ME11A only shift 2cfeb, BECAUSE GIF ME11A ONLY HAS dcfeb 1,5,7

         std::vector<int> myADCVals = stripIt->getADCCounts();
         int adcTotal, maxADC, adcMaxTime;
         double ped = 0.5*(myADCVals[0]+myADCVals[1]);   

         adcTotal = 0; maxADC = 0; adcMaxTime = 0;

         //cout << tmpID.Layer << ", strip: " << tmpSP.Strip << endl;     
         //cout << myADCVals[0] << " " << myADCVals[1] << endl;

         for (int i = 2; i < int(myADCVals.size()); i++){

            double tmpADC = myADCVals[i]-ped;
            adcTotal+=tmpADC;

            //cout << myADCVals[i] - ped << " " ;
            if (tmpADC > maxADC && tmpADC > 13.3){

               maxADC = tmpADC;
               adcMaxTime = i;
   
            }

         }//loop over all 6 time bins for each strip, from 2 to 7
         //cout << endl;

         if (doDebug > 2) cout << id << endl;
         if (doDebug > 2&&maxADC>0) cout << "strip: " << tmpSP.Strip << ", adc: " << maxADC << endl;

         tmpSP.ADCTotal = adcTotal;
         tmpSP.MaxADC = maxADC;
         tmpSP.ADCMaxTime = adcMaxTime;

         tmpSTRIP.second.push_back(tmpSP);
      }   

      tmpSTRIP.first = tmpID;
      strip_container.push_back(tmpSTRIP);

   }//strip collection

   //==========COMPARATOR=========================


   //edm::Handle<CSCComparatorDigiCollection> comparators;
   //iEvent.getByLabel(comparatorDigiTag,comparators);
   //edm::EDGetTokenT<CSCComparatorDigiCollection> c_token = consumes<CSCComparatorDigiCollection>(comparatorDigiTag);
   edm::Handle<CSCComparatorDigiCollection> comparators;
   //iEvent.getByLabel(wireDigiTag,wires);
   iEvent.getByToken(compDigiTagSrc, comparators);


   if (doDebug > 1) cout <<"Event "<<Event <<" collecting comparator digi "<< endl;
   for (CSCComparatorDigiCollection::DigiRangeIterator com=comparators->begin(); com!=comparators->end(); com++) {

      CSCDetId id = (CSCDetId)(*com).first;

      std::vector<CSCComparatorDigi>::const_iterator comparatorIt = (*com).second.first;
      std::vector<CSCComparatorDigi>::const_iterator lastComparator = (*com).second.second;
      
      CSCDetID tmpID;
      
      COMPARATOR tmpCOMPARATOR;

      tmpID.Endcap = id.endcap();
      tmpID.Ring = id.ring();// ? 1 : id.ring();
      tmpID.Station = id.station();
      tmpID.Chamber = id.chamber();
      tmpID.Layer = id.layer();

      for( ; comparatorIt != lastComparator; ++comparatorIt) {

         Comparator tmpCOM;   

         tmpCOM.Strip = comparatorIt->getStrip();
         if (id.ring() == 4) tmpCOM.Strip+=64; //gif for now, same reason as strip
         tmpCOM.TimeBin = comparatorIt->getTimeBin();
         tmpCOM.ComparatorNumber = comparatorIt->getComparator();
         if (id.ring() == 4) tmpCOM.ComparatorNumber+=128;
         if (doDebug > 2) cout << id << endl;
         if (doDebug > 2) cout << "strip: " << tmpCOM.Strip << ", comparator: " << tmpCOM.ComparatorNumber << endl;
         tmpCOMPARATOR.second.push_back(tmpCOM);

      }


      tmpCOMPARATOR.first = tmpID;
      com_container.push_back(tmpCOMPARATOR);

   }//comparator collection


   if (doDebug > 1) cout <<"Event "<<Event <<" collecting  GEMPadDigi "<< endl;
   edm::Handle<GEMPadDigiCollection> gempads_;
   iEvent.getByToken(gemPadTagSrc, gempads_);
   GEMPadDigiCollection  gempads = *gempads_.product();

   for (auto it = gempads.begin(); it != gempads.end(); ++it) {
      const GEMDetId& id = (*it).first;
      GEMDetID tmpID;
      

      tmpID.Endcap = id.region();
      tmpID.Ring = id.ring();// ? 1 : id.ring();
      tmpID.Station = id.station();
      tmpID.Chamber = id.chamber();
      tmpID.Layer = id.layer();
      tmpID.Roll = id.roll();

      GEMPAD tmpGEMPAD;
      tmpGEMPAD.first = tmpID;
      const auto& padsvec = (*it).second;
      for (auto pad = padsvec.first; pad != padsvec.second; ++pad) {
         GEMPad tmpPad;
         tmpPad.Pad = pad->pad();
         tmpPad.TimeBin = pad->bx();
         if (doDebug > 1) cout << id << endl;
         if (doDebug > 1) cout << " gempad: " << tmpPad.Pad << ", bx: " << tmpPad.TimeBin << endl;
         tmpGEMPAD.second.push_back(tmpPad);

      }
      gempad_container.push_back(tmpGEMPAD);
   }

   if (doDebug > 1) cout <<"Event "<<Event <<" collecting  ALCT/CLCT/LCT "<< endl;
   //ALCT
   edm::Handle<CSCALCTDigiCollection> alcts;
   iEvent.getByToken(alctDigiTagSrc, alcts);
   fillALCT(alcts, alct_container);

   // CLCT
   edm::Handle<CSCCLCTDigiCollection> clcts;
   iEvent.getByToken(clctDigiTagSrc, clcts);
   fillCLCT(clcts, clct_container);


   // LCT
   edm::Handle<CSCCorrelatedLCTDigiCollection> lcts;
   iEvent.getByToken(corrlctDigiTagSrc, lcts);
   fillCorrLCT(lcts, lct_container);

   //GEMCluster 
   edm::Handle<GEMPadDigiClusterCollection> gemclusters;
   iEvent.getByToken(gemPadClusterTagSrc, gemclusters);
   if (doDebug > 1) cout <<"Event "<<Event <<" collecting  GEMClusters "<< endl;
   fillGEMCluster(*gemclusters.product(), gemcluster_container);


   //ALCT
   edm::Handle<CSCALCTDigiCollection> alcts_emul;
   // CLCT
   edm::Handle<CSCCLCTDigiCollection> clcts_emul;
   // LCT
   edm::Handle<CSCCorrelatedLCTDigiCollection> lcts_emul;
   if (addEmulation){
      if (doDebug > 1) cout <<"Event "<<Event <<" collecting Emulated ALCT/CLCT/LCT "<< endl;
      try{
      iEvent.getByToken(alctDigiTagSrc_emul, alcts_emul);
      fillALCT(alcts_emul, alct_emul_container);

      iEvent.getByToken(clctDigiTagSrc_emul, clcts_emul);
      fillCLCT(clcts_emul, clct_emul_container);

      iEvent.getByToken(corrlctDigiTagSrc_emul, lcts_emul);
      fillCorrLCT(lcts_emul, lct_emul_container);
      } catch (cms::Exception){
      std::cout <<" failed to emulated trigger primitives, ignore "<< std::endl;
      addEmulation = false;
      }
   }

   //make event display
   vector<CSCDetID> usedChamber;
   vector<GEMDetID> usedGEMChamber;//neighbor GEM chamber

   //if (Event < 500){//find(eventList.begin(), eventList.end(), Event) != eventList.end()){
   //if (find(eventList.begin(), eventList.end(), Event) != eventList.end()){

   for (int i = 0; i < int(eventList.size()); i++) {

      if (i >= 1000) continue;

      double eventL = eventList[i];
      int endcapL = endcapList[i];
      int stationL = stationList[i];
      int ringL = ringList[i];
      int chamberL = chamberList[i];

      if (Event != eventL) continue;

      CSCDetID tmpId;
      tmpId.Endcap = endcapL;
      tmpId.Station =  stationL;
      tmpId.Ring = ringL;
      tmpId.Chamber = chamberL;

      if (doDebug > 0) 
         cout <<"Start to display event "<< eventL <<" "<< tmpId <<endl;

      WireStripDisplay(eventDisplayDir, tmpId, simhit_container, wire_container, strip_container, com_container, alct_container, alct_emul_container, clct_container, clct_emul_container, lct_container, lct_emul_container, usedChamber, Run, Event, addEmulation);
      //if (stationL == 1 and (ringL== 1 or ringL==2)) 
      if (doGEMDisplay and stationL == 1 and (ringL== 1)){// only GE11 now
        if (doDebug) cout <<"doGEMdisplay, station  "<< stationL <<" endcap "<< endcapL <<" chamber "<< chamberL <<endl; 
        GEMDetID gemId;
        gemId.Endcap = endcapL==1 ? 1 : -1;
        gemId.Station =  stationL;
        gemId.Ring = ringL;
        gemId.Chamber = chamberL;
        gemId.Layer = 0; //for superchamber
        gemId.Roll = -1; //for superchamber
        GEMPadDisplay(eventDisplayDir, gemId, tmpId, gemsimhit_container, gempad_container, gemcluster_container, usedGEMChamber, lct_container, lct_emul_container, Run, Event, addEmulation, doGEMCSC, doDebug);
      }
   }
      


#ifdef THIS_IS_AN_EVENT_EXAMPLE
   Handle<ExampleData> pIn;
   iEvent.getByLabel("example",pIn);
#endif
   
#ifdef THIS_IS_AN_EVENTSETUP_EXAMPLE
   ESHandle<SetupData> pSetup;
   iSetup.get<SetupRecord>().get(pSetup);
#endif
}// end of main analyze()



void GifDisplay::fillALCT(edm::Handle<CSCALCTDigiCollection> digicoll, vector<CSCIDLCTs>& alct_container){

 for (CSCALCTDigiCollection::DigiRangeIterator j=digicoll->begin(); j!=digicoll->end(); j++) {
    const CSCALCTDigiCollection::Range& range =(*j).second;
    const CSCDetId& idLCT = (*j).first;

    CSCIDLCTs tmpidlcts;
    CSCDetID tmpID;
   

    tmpID.Endcap = idLCT.endcap();
    tmpID.Ring = idLCT.ring();// ? 1 : id.ring();
    tmpID.Station = idLCT.station();
    tmpID.Chamber = idLCT.chamber();
    tmpID.Layer = idLCT.layer();

    tmpidlcts.first = tmpID ;

    for (CSCALCTDigiCollection::const_iterator digiIt = range.first; digiIt!=range.second; ++digiIt){
      if (!digiIt->isValid()) continue;
      CorrelatedLCT tmplct;
      tmplct.keyWG = digiIt->getKeyWG();
      tmplct.quality =  digiIt->getQuality();
      tmplct.pattern = digiIt->getCollisionB();
      tmplct.BX = digiIt->getBX();
      tmpidlcts.second.push_back(tmplct);

	   if (doDebug > 2) cout <<"fillALCT "<< idLCT << endl;
	   if (doDebug > 2) {
       cout << "ALCT strip: " << tmplct.keyStrip;
       cout << " keyWG: " << tmplct.keyWG;
       cout << " pattern: " << tmplct.pattern;
       cout << " quality: " << tmplct.quality << endl;

	   }


   }
   alct_container.push_back(tmpidlcts);

  }
}


void GifDisplay::fillCLCT(edm::Handle<CSCCLCTDigiCollection> digicoll, vector<CSCIDLCTs>& clct_container){

 for (CSCCLCTDigiCollection::DigiRangeIterator j=digicoll->begin(); j!=digicoll->end(); j++) {
    const CSCCLCTDigiCollection::Range& range =(*j).second;
    const CSCDetId& idLCT = (*j).first;

    CSCIDLCTs tmpidlcts;
    CSCDetID tmpID;
   

    tmpID.Endcap = idLCT.endcap();
    tmpID.Ring = idLCT.ring();// ? 1 : id.ring();
    tmpID.Station = idLCT.station();
    tmpID.Chamber = idLCT.chamber();
    tmpID.Layer = idLCT.layer();

    tmpidlcts.first = tmpID ;

    for (CSCCLCTDigiCollection::const_iterator digiIt = range.first; digiIt!=range.second; ++digiIt){
      if (!digiIt->isValid()) continue;
      CorrelatedLCT tmplct;
      tmplct.keyStrip = digiIt->getKeyStrip() ;
      tmplct.quality =  digiIt->getQuality();
      tmplct.pattern = digiIt->getPattern();
      tmplct.BX = digiIt->getBX();
      tmplct.run3_pattern = digiIt->getRun3Pattern();
      tmpidlcts.second.push_back(tmplct);

	   if (doDebug > 2) cout <<"fillCLCT "<< idLCT << endl;
	   if (doDebug > 2) {
       cout << "CLCT strip: " << tmplct.keyStrip;
       cout << " keyWG: " << tmplct.keyWG;
       cout << " pattern: " << tmplct.pattern;
       cout << " quality: " << tmplct.quality << endl;

	   }


    }
   clct_container.push_back(tmpidlcts);

  }
}


void GifDisplay::fillCorrLCT(edm::Handle<CSCCorrelatedLCTDigiCollection> digicoll, vector<CSCIDLCTs>& lct_container){

 for (CSCCorrelatedLCTDigiCollection::DigiRangeIterator j=digicoll->begin(); j!=digicoll->end(); j++) {
    const CSCCorrelatedLCTDigiCollection::Range& range =(*j).second;
    const CSCDetId& idLCT = (*j).first;

    CSCIDLCTs tmpidlcts;
    CSCDetID tmpID;
   

    tmpID.Endcap = idLCT.endcap();
    tmpID.Ring = idLCT.ring();// ? 1 : id.ring();
    tmpID.Station = idLCT.station();
    tmpID.Chamber = idLCT.chamber();
    tmpID.Layer = idLCT.layer();

    tmpidlcts.first = tmpID ;

    for (CSCCorrelatedLCTDigiCollection::const_iterator digiIt = range.first; digiIt!=range.second; ++digiIt){
      if (!digiIt->isValid()) continue;
      CorrelatedLCT tmplct;
      tmplct.keyWG = digiIt->getKeyWG();
      tmplct.keyStrip = digiIt->getStrip() ;
      tmplct.quality =  digiIt->getQuality();
      tmplct.pattern = digiIt->getPattern();
      tmplct.BX = digiIt->getBX();
      tmplct.eighthStrip = (tmplct.keyStrip*4 + digiIt->getQuartStripBit()*2 + digiIt->getEighthStripBit());
      tmplct.slope = digiIt->getSlope();
      tmplct.bend = digiIt->getBend();
      tmpidlcts.second.push_back(tmplct);

	   if (doDebug > 2) cout <<"fillLCT "<< idLCT << endl;
	   if (doDebug > 2) {
          cout << "CorrelatedLCT strip: " << digiIt->getStrip();
          cout << " keyWG: " << digiIt->getKeyWG();
          cout << " pattern: " << digiIt->getPattern();
          cout << " quality: " << digiIt->getQuality() << endl;

	   }


   }
   lct_container.push_back(tmpidlcts);

  }
}


void GifDisplay::fillGEMCluster(GEMPadDigiClusterCollection gemclusters, vector<GEMCLUSTER>& gemcluster_container){
   for (auto it = gemclusters.begin(); it != gemclusters.end(); ++it) {
     const GEMDetId id = (*it).first;
     const auto clvec = (*it).second;
     GEMDetID tmpID;
     

     tmpID.Endcap = id.region();
     tmpID.Ring = id.ring();// ? 1 : id.ring();
     tmpID.Station = id.station();
     tmpID.Chamber = id.chamber();
     tmpID.Layer = id.layer();
     tmpID.Roll = id.roll();

     GEMCLUSTER tmpGEMCluster;
     tmpGEMCluster.first = tmpID;
     for (auto cluster = clvec.first; cluster != clvec.second; ++cluster) {
        if ((cluster->pads()).size() == 0) continue;
        GEMPadCluster tmpcluster;
        tmpcluster.firstPad = (cluster->pads()).at(0);
        tmpcluster.size = (cluster->pads()).size();
        tmpcluster.BX = cluster->bx(); 
        tmpGEMCluster.second.push_back(tmpcluster);
         if (doDebug > 1) cout <<"fillGEMClusters "<< id << endl;
         if (doDebug > 1) {
            cout <<" firstPad "<< tmpcluster.firstPad <<" size " << tmpcluster.size <<" BX "<< tmpcluster.BX << endl;
         }
       
     }
     gemcluster_container.push_back(tmpGEMCluster);
   }
}

// ------------ method called once each job just before starting event loop  ------------
void 
GifDisplay::beginJob()
{
}

// ------------ method called once each job just after ending the event loop  ------------
void 
GifDisplay::endJob() 
{

}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
GifDisplay::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(GifDisplay);
