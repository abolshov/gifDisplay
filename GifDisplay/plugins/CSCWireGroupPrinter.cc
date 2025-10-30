#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "Geometry/CSCGeometry/interface/CSCGeometry.h"
#include "Geometry/Records/interface/MuonGeometryRecord.h"
#include <map>
#include <iostream>
#include <iomanip>

class CSCWireGroupPrinter : public edm::one::EDAnalyzer<> {
public:
  explicit CSCWireGroupPrinter(const edm::ParameterSet&) {}
  void analyze(const edm::Event&, const edm::EventSetup&) override;

private:
  edm::ESGetToken<CSCGeometry, MuonGeometryRecord> geomToken_{esConsumes()};
};

void CSCWireGroupPrinter::analyze(const edm::Event&, const edm::EventSetup& iSetup) {
  const auto& geom = iSetup.getData(geomToken_);

  // key: (station, ring) → pair(nWireGroups, nStrips)
  std::map<std::pair<int,int>, std::pair<int,int>> chamberInfo;

  for (auto const* chamber : geom.chambers()) {
    const auto& id = chamber->id();
    int station = id.station();
    int ring = id.ring();

    // Take layer(1) as representative (all layers identical geometry)
    const auto* layerGeom = chamber->layer(1)->geometry();
    int nWG = layerGeom->numberOfWireGroups();
    int nStrips = layerGeom->numberOfStrips();

    chamberInfo[{station, ring}] = {nWG, nStrips};
  }

  std::cout << "\n=== CSC Geometry Summary (from CMSSW geometry) ===\n";
  std::cout << std::left << std::setw(8) << "Chamber"
            << std::setw(20) << "Wire Groups"
            << std::setw(20) << "Strips" << "\n";
  std::cout << "----------------------------------------------------\n";

  for (const auto& kv : chamberInfo) {
    std::cout << "ME" << kv.first.first << "/" << kv.first.second
              << std::setw(10) << "  "
              << std::setw(10) << kv.second.first
              << std::setw(10) << "  "
              << kv.second.second << "\n";
  }

  std::cout << "----------------------------------------------------\n";
}

DEFINE_FWK_MODULE(CSCWireGroupPrinter);
