// CSCNumberingPrinter.cc
// Prints min indices and a short sample of CSC wire/strip/halfstrip numbering from DIGI/L1 collections.
//
// BuildFile.xml (same dir):
// <library file="CSCNumberingPrinter.cc" name="GifDisplayCSCNumberingPrinter">
//   <use name="FWCore/Framework"/>
//   <use name="FWCore/ParameterSet"/>
//   <use name="DataFormats/CSCDigi"/>
// </library>

#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DataFormats/CSCDigi/interface/CSCWireDigi.h"
#include "DataFormats/CSCDigi/interface/CSCStripDigi.h"
#include "DataFormats/CSCDigi/interface/CSCComparatorDigi.h"
#include "DataFormats/CSCDigi/interface/CSCCLCTDigi.h"

#include "DataFormats/CSCDigi/interface/CSCWireDigiCollection.h"
#include "DataFormats/CSCDigi/interface/CSCStripDigiCollection.h"
#include "DataFormats/CSCDigi/interface/CSCComparatorDigiCollection.h"
#include "DataFormats/CSCDigi/interface/CSCCLCTDigiCollection.h"

#include <limits>
#include <vector>
#include <iostream>
#include <algorithm>
#include <iomanip>

class CSCIndexCheck : public edm::one::EDAnalyzer<> {
public:
  explicit CSCIndexCheck(const edm::ParameterSet& p)
  : tokW_(consumes<CSCWireDigiCollection>(p.getParameter<edm::InputTag>("wiresTag"))),
    tokS_(consumes<CSCStripDigiCollection>(p.getParameter<edm::InputTag>("stripsTag"))),
    tokC_(consumes<CSCComparatorDigiCollection>(p.getParameter<edm::InputTag>("compsTag"))),
    tokCLCT_(consumes<CSCCLCTDigiCollection>(p.getParameter<edm::InputTag>("clctTag"))),
    sampleN_(p.getUntrackedParameter<unsigned>("sampleN", 10)) {}

  void analyze(const edm::Event& iEvent, const edm::EventSetup&) override {
    edm::Handle<CSCWireDigiCollection> wiresH;
    edm::Handle<CSCStripDigiCollection> stripsH;
    edm::Handle<CSCComparatorDigiCollection> compsH;
    edm::Handle<CSCCLCTDigiCollection> clctsH;

    // Try to get each product; if missing, we keep going and report
    bool hasW = iEvent.getByToken(tokW_, wiresH);
    bool hasS = iEvent.getByToken(tokS_, stripsH);
    bool hasC = iEvent.getByToken(tokC_, compsH);
    bool hasL1= iEvent.getByToken(tokCLCT_, clctsH);

    // Accumulators
    int minWG = std::numeric_limits<int>::max();
    int minStrip = std::numeric_limits<int>::max();
    int minCompStrip = std::numeric_limits<int>::max();
    int minKeyHS = std::numeric_limits<int>::max();

    std::vector<int> sampleWG, sampleStrip, sampleCompStrip, sampleKeyHS;
    sampleWG.reserve(sampleN_);
    sampleStrip.reserve(sampleN_);
    sampleCompStrip.reserve(sampleN_);
    sampleKeyHS.reserve(sampleN_);

    // ---- wires
    if (hasW && wiresH.isValid()) {
      for (auto it = wiresH->begin(); it != wiresH->end(); ++it) {
        const auto& range = (*it).second;
        for (auto p = range.first; p != range.second; ++p) {
          const int wg = p->getWireGroup();
          if (wg < minWG) minWG = wg;
          if (sampleWG.size() < sampleN_) sampleWG.push_back(wg);
        }
      }
    }

    // ---- strips
    if (hasS && stripsH.isValid()) {
      for (auto it = stripsH->begin(); it != stripsH->end(); ++it) {
        const auto& range = (*it).second;
        for (auto p = range.first; p != range.second; ++p) {
          const int st = p->getStrip();
          if (st < minStrip) minStrip = st;
          if (sampleStrip.size() < sampleN_) sampleStrip.push_back(st);
        }
      }
    }

    // ---- comparators (store strip index as saved in digi)
    if (hasC && compsH.isValid()) {
      for (auto it = compsH->begin(); it != compsH->end(); ++it) {
        const auto& range = (*it).second;
        for (auto p = range.first; p != range.second; ++p) {
          const int cs = p->getStrip(); // comparator-digi stores strip index; comparator bit is another thing
          if (cs < minCompStrip) minCompStrip = cs;
          if (sampleCompStrip.size() < sampleN_) sampleCompStrip.push_back(cs);
        }
      }
    }

    // ---- CLCTs (key half-strip index)
    if (hasL1 && clctsH.isValid()) {
      for (auto it = clctsH->begin(); it != clctsH->end(); ++it) {
        const auto& range = (*it).second;
        for (auto p = range.first; p != range.second; ++p) {
          if (!p->isValid()) continue;
          const int khs = p->getKeyStrip(); // this is the key half-strip index
          if (khs < minKeyHS) minKeyHS = khs;
          if (sampleKeyHS.size() < sampleN_) sampleKeyHS.push_back(khs);
        }
      }
    }

    auto safe = [](int v){ return v==std::numeric_limits<int>::max() ? -1 : v; };

    std::cout << "[CSCIndexCheck] Run " << iEvent.id().run()
              << " Lumi " << iEvent.luminosityBlock()
              << " Event " << iEvent.id().event() << "\n"
              << "  products: wires=" << (hasW && wiresH.isValid())
              << " strips=" << (hasS && stripsH.isValid())
              << " comps="  << (hasC && compsH.isValid())
              << " clcts="  << (hasL1 && clctsH.isValid()) << "\n"
              << "  min wireGroup        = " << safe(minWG)        << "\n"
              << "  min strip            = " << safe(minStrip)     << "\n"
              << "  min comp.stripIndex  = " << safe(minCompStrip) << "\n"
              << "  min CLCT keyHalfStrip= " << safe(minKeyHS)     << "\n";

    auto printSample = [&](const char* tag, const std::vector<int>& v){
      std::cout << "  sample " << std::setw(18) << std::left << tag << ": ";
      if (v.empty()) { std::cout << "(none)\n"; return; }
      for (size_t i=0; i<v.size(); ++i) {
        if (i) std::cout << ", ";
        std::cout << v[i];
      }
      std::cout << "\n";
    };

    printSample("wireGroup",   sampleWG);
    printSample("strip",       sampleStrip);
    printSample("comp.strip",  sampleCompStrip);
    printSample("keyHalfStrip",sampleKeyHS);
    std::cout << std::flush;
  }

private:
  edm::EDGetTokenT<CSCWireDigiCollection>        tokW_;
  edm::EDGetTokenT<CSCStripDigiCollection>       tokS_;
  edm::EDGetTokenT<CSCComparatorDigiCollection>  tokC_;
  edm::EDGetTokenT<CSCCLCTDigiCollection>        tokCLCT_;
  unsigned sampleN_;
};

DEFINE_FWK_MODULE(CSCIndexCheck);
