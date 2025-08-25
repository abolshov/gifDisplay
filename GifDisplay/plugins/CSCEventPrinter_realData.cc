// CSCEventPrinterData.cc
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "DataFormats/MuonDetId/interface/CSCDetId.h"
#include "DataFormats/Common/interface/RangeMap.h"
#include "DataFormats/Common/interface/OwnVector.h"
#include "DataFormats/CSCRecHit/interface/CSCSegment.h"
#include "DataFormats/CSCRecHit/interface/CSCRecHit2D.h"

#include <fstream>
#include <set>
#include <tuple>

class CSCEventPrinterData : public edm::one::EDAnalyzer<> {
public:
  explicit CSCEventPrinterData(const edm::ParameterSet&);
  ~CSCEventPrinterData() override;
  void analyze(const edm::Event&, const edm::EventSetup&) override;

private:
  edm::EDGetTokenT<edm::RangeMap<CSCDetId, edm::OwnVector<CSCSegment>>> segTok_;
  edm::EDGetTokenT<edm::RangeMap<CSCDetId, edm::OwnVector<CSCRecHit2D>>> rechitTok_;
  bool useRechitFallback_{false};

  std::ofstream out_;
  std::set<std::tuple<uint32_t,uint32_t,int,int,int,int>> seen_;
};

CSCEventPrinterData::CSCEventPrinterData(const edm::ParameterSet& cfg) {
  auto segTag = cfg.getParameter<edm::InputTag>("segmentTag");   // e.g. "cscSegments"
  segTok_ = consumes<edm::RangeMap<CSCDetId, edm::OwnVector<CSCSegment>>>(segTag);

  // Optional fallback to rechits if segments missing in an event
  if (cfg.existsAs<edm::InputTag>("rechit2DTag", false)) {
    auto rhTag = cfg.getParameter<edm::InputTag>("rechit2DTag"); // e.g. "csc2DRecHits"
    if (!rhTag.label().empty()) {
      rechitTok_ = consumes<edm::RangeMap<CSCDetId, edm::OwnVector<CSCRecHit2D>>>(rhTag);
      useRechitFallback_ = true;
    }
  }

  out_.open("eventList_temp.txt");
  out_ << "## run event endcap station ring chamber\n";
}

CSCEventPrinterData::~CSCEventPrinterData() { out_.close(); }

void CSCEventPrinterData::analyze(const edm::Event& ev, const edm::EventSetup&) {
  const uint32_t run   = ev.id().run();
  const uint32_t event = ev.id().event();

  bool wrote = false;

  // Prefer segments
  edm::Handle<edm::RangeMap<CSCDetId, edm::OwnVector<CSCSegment>>> segs;
  ev.getByToken(segTok_, segs);
  if (segs.isValid()) {
    for (auto it = segs->id_begin(); it != segs->id_end(); ++it) {
      CSCDetId id(*it);
      auto range = segs->get(*it);
      if (range.first == range.second) continue; // no segments in this chamber
      auto key = std::make_tuple(run,event,id.endcap(),id.station(),id.ring(),id.chamber());
      if (seen_.insert(key).second) {
        out_ << run << " " << event << " "
             << id.endcap() << " " << id.station() << " "
             << id.ring()   << " " << id.chamber() << "\n";
        wrote = true;
      }
    }
  } else {
    edm::LogWarning("CSCEventPrinterData") << "cscSegments not found!";
  }

  // Fallback to rechits if requested and nothing was written
  if (!wrote && useRechitFallback_) {
    edm::Handle<edm::RangeMap<CSCDetId, edm::OwnVector<CSCRecHit2D>>> hits;
    ev.getByToken(rechitTok_, hits);
    if (hits.isValid()) {
      for (auto it = hits->id_begin(); it != hits->id_end(); ++it) {
        CSCDetId id(*it);
        auto range = hits->get(*it);
        if (range.first == range.second) continue;
        auto key = std::make_tuple(run,event,id.endcap(),id.station(),id.ring(),id.chamber());
        if (seen_.insert(key).second) {
          out_ << run << " " << event << " "
               << id.endcap() << " " << id.station() << " "
               << id.ring()   << " " << id.chamber() << "\n";
          wrote = true;
        }
      }
    } else {
      edm::LogWarning("CSCEventPrinterData") << "csc2DRecHits not found!";
    }
  }

  if (!wrote) {
    edm::LogVerbatim("CSCEventPrinterData") << "Run " << run << " Event " << event
                                            << ": no CSC segments/rechits.";
  }
}

DEFINE_FWK_MODULE(CSCEventPrinterData);
