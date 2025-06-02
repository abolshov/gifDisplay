#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "DataFormats/CSCDigi/interface/CSCCLCTDigiCollection.h"
#include "FWCore/Framework/interface/MakerMacros.h"

class CSCEventPrinter : public edm::one::EDAnalyzer<> {
public:
  explicit CSCEventPrinter(const edm::ParameterSet&);
  void analyze(const edm::Event&, const edm::EventSetup&) override;

private:
  edm::EDGetTokenT<CSCCLCTDigiCollection> clctToken_;
};

CSCEventPrinter::CSCEventPrinter(const edm::ParameterSet& iConfig) {
  auto clctTag = iConfig.getParameter<edm::InputTag>("clctTag");
  clctToken_ = consumes<CSCCLCTDigiCollection>(clctTag);
}

void CSCEventPrinter::analyze(const edm::Event& iEvent, const edm::EventSetup&) {
  edm::Handle<CSCCLCTDigiCollection> clcts;
  iEvent.getByToken(clctToken_, clcts);

  if (!clcts.isValid()) {
    edm::LogWarning("CSCEventPrinter") << "CLCT collection not found!";
    return;
  }

  edm::LogVerbatim("CSCEventPrinter") << "Run: " << iEvent.id().run()
                                      << ", Event: " << iEvent.id().event();

  for (auto it = clcts->begin(); it != clcts->end(); ++it) {
    const CSCDetId& id = (*it).first;
    const auto& range = (*it).second;

    for (auto digi = range.first; digi != range.second; ++digi) {
      if (!digi->isValid()) continue;
      edm::LogVerbatim("CSCEventPrinter") << "CSCDetId: " << id << ", CLCT: " << *digi;
    }
  }
}

DEFINE_FWK_MODULE(CSCEventPrinter);
