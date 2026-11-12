#include "gifDisplay/GifDisplay/interface/display.h"

#include <math.h>
#include <stdlib.h> /* srand, rand */
#include <time.h>   /* time */

#include <algorithm>
#include <boost/lexical_cast.hpp>
#include <fstream>
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <set>  // design mark: added for std::map in overlay functions
#include <string>
#include <vector>

#include "Rtypes.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TH2.h"
#include "TLegend.h"
#include "TLorentzVector.h"
#include "TMath.h"
#include "TPaletteAxis.h"
#include "TPaveText.h"
#include "TStyle.h"
#define PI 3.14159265

// yumeng
#include "TLatex.h"
#include "TMarker.h"
#include "TPolyLine.h"
#include "TLine.h"
#include "TBox.h"
#include "TColor.h"

// line colors of CSC segments (RecoOverlay and the text panel)
static const int kSegmentColors[] = {kRed, kMagenta + 1, kBlue, kOrange + 7,
                                     kCyan + 2};
static const int kNSegmentColors = 5;

// per-stub colours linking ALCT/CLCT boxes to their text lines (stub i gets
// colour i); dark so they read on the bright time-bin cells and differ from the
// segment colours
static const int kStubColors[] = {kBlack, kGreen + 3, kOrange + 3, kAzure + 3,
                                  kViolet + 3};
static const int kNStubColors = 5;
static int StubColor(int i) { return kStubColors[i % kNStubColors]; }

// one colour per time bin 0-15 on the Anode Hits and Comparator Hits panels:
// bright, but mid-tone so the white labels stay readable; later bins get the
// last colour
static const char* kTimeBinColorsHex[] = {
    "#9e9e9e", "#c8742e", "#e8483b", "#ff2d2d", "#ff8c00", "#c9a800",
    "#7cc42e", "#22b14c", "#12b5a5", "#1ea7f0", "#3a6dff", "#6a4dff",
    "#a64dff", "#e03ae6", "#ff4f9a", "#4a4a4a"};
static const int kNTimeBinColors = 16;

// strip panel: max ADC in bands of kADCBandWidth counts, light (low) to dark
// (high); the last colour is for everything above
static const char* kADCColorsHex[] = {"#fed976", "#feb24c", "#fd8d3c",
                                      "#fc4e2a", "#e31a1c", "#bd0026",
                                      "#800026", "#67001f", "#4a0014",
                                      "#2b000b"};
static const double kADCBandWidth = 50.;

vector<CorrelatedLCT> findStubsInChamber(CSCDetID id,
                                         vector<CSCIDLCTs> alllcts) {
  vector<CorrelatedLCT> lcts;
  for (auto const& [detId, primitives]: alllcts)
  {
    if (id == detId)
    {
      for (auto const& p: primitives)
      {
        lcts.push_back(p);
      }
    }
  }
  return lcts;
}

void WireStripDisplay(
    TString address, CSCDetID id, vector<SIMHIT>& simhit,
    vector<SIMTRACK>& simtracks, vector<WIRE>& wire, vector<STRIP>& strip,
    vector<COMPARATOR>& comparator, vector<CSCIDLCTs>& allalcts,
    vector<CSCIDLCTs>& allalcts_emul, vector<CSCIDLCTs>& allclcts,
    vector<CSCIDLCTs>& allclcts_emul, vector<CSCIDLCTs>& alllcts,
    vector<CSCIDLCTs>& alllcts_emul, vector<CSCDetID>& usedChamber,
    // int type,
    edm::RunNumber_t Run, edm::EventNumber_t Event, bool addEmulation, TString compareA, TString compareB,
    const vector<RecHitPoint>& rechits, const vector<SegmentLine>& segments,
    bool plotDataStubBorders, bool overlayRecHits, bool recHitPage,
    int doDebug) {
  const int alct_bx_offset = 5;  // ALCT BX offset
  const int clct_bx_offset = 1;  // CLCT BX offset

  // gStyle->SetPalette(55);
  //  design mark: split overlay containers to avoid redraw overlap.
  //  stripOverlays and wireOverlays here are std::vector<TObject*> containers
  //  that temporarily store all drawable ROOT objects (mainly TMarker and
  //  TLatex) created for SimHits. filled inside SimHitDisplay() /
  //  SimHitWireDisplay() and then drawn in WireStripDisplay() using for ...
  //  Draw("same") iteration
  std::vector<TObject*> stripOverlays;
  std::vector<TObject*> wireOverlays;
  std::vector<TObject*> recoOverlays;  // CSC rechits and segments
  std::vector<TObject*> cfebBorders;   // CFEB boundaries on the strip panel
  std::vector<TObject*> stubBoxes;     // per-stub ALCT/CLCT boxes and tags
  std::vector<TObject*> timeBinCells;  // wire/comparator/strip cells, labels, keys
  static bool paletteDone = false;
  if (!paletteDone) {
    const int N = 11;  // allocate up to index 10
    Int_t pal[N];
    for (int i = 0; i < N; ++i) pal[i] = i;  // start with ROOT defaults
    pal[10] = TColor::GetColorTransparent(kGray, 0.2);
    gStyle->SetPalette(N, pal);
    paletteDone = true;
  }

  // yumeng: this TH2F only for looking up wire group numbers for different
  // chambers. x 4bins from bin1 to bin4:[1,2),[2,3),[3,4),[4,5)
  TH2F* NWireGroup = new TH2F("NWireGroup", "NWireGroup", 4, 1, 5, 4, 1, 5);
  TH2F* NStrip = new TH2F("NStrip", "NStrip", 4, 1, 5, 4, 1, 5);

  NWireGroup->SetBinContent(1, 1, 48);
  // NWireGroup->SetBinContent(1, 3, 48);
  NWireGroup->SetBinContent(1, 3, 32);
  NWireGroup->SetBinContent(1, 4, 48);
  NWireGroup->SetBinContent(2, 1, 112);
  NWireGroup->SetBinContent(3, 1, 96);
  NWireGroup->SetBinContent(4, 1, 96);
  // NWireGroup->SetBinContent(1, 2, 48);
  NWireGroup->SetBinContent(1, 2, 64);
  NWireGroup->SetBinContent(2, 2, 64);
  NWireGroup->SetBinContent(3, 2, 64);
  NWireGroup->SetBinContent(4, 2, 64);

  NStrip->SetBinContent(1, 1, 112);
  // NStrip->SetBinContent(1, 1, 64);
  NStrip->SetBinContent(1, 2, 80);
  NStrip->SetBinContent(1, 3, 64);
  NStrip->SetBinContent(1, 4, 112);
  // NStrip->SetBinContent(1, 4, 48);
  NStrip->SetBinContent(2, 1, 80);
  NStrip->SetBinContent(3, 1, 80);
  NStrip->SetBinContent(4, 1, 80);
  NStrip->SetBinContent(2, 2, 80);
  NStrip->SetBinContent(3, 2, 80);
  NStrip->SetBinContent(4, 2, 80);

  if (ChamberUsedForEventDisplay(id, usedChamber))
    return;  // this chamber has not been used for eventdisplay

  bool doSimHit = simhit.size() > 0;

  srand(time(NULL));
  TString name = "";
  TString legendName = "";

  SetSaveNameLegendName(name, legendName, address, id, Run, Event);

  vector<int> layer_simhit = FindChamberIndex(id, simhit);
  vector<int> layer_strip = FindChamberIndex(id, strip);
  vector<int> layer_wire = FindChamberIndex(id, wire);
  vector<int> layer_comparator = FindChamberIndex(id, comparator);

  if (id.Ring == 1 and id.Station == 1) {
    CSCDetID idr4 = id;
    idr4.Ring = 4;
    vector<int> layer_wire_r4 = FindChamberIndex(idr4, wire);
    vector<int> layer_strip_r4 = FindChamberIndex(idr4, strip);
    vector<int> layer_simhit_r4 = FindChamberIndex(idr4, simhit);
    vector<int> layer_comparator_r4 = FindChamberIndex(idr4, comparator);

    layer_wire.insert(layer_wire.end(), layer_wire_r4.begin(),
                      layer_wire_r4.end());
    layer_strip.insert(layer_strip.end(), layer_strip_r4.begin(),
                       layer_strip_r4.end());
    layer_simhit.insert(layer_simhit.end(), layer_simhit_r4.begin(),
                        layer_simhit_r4.end());
    layer_comparator.insert(layer_comparator.end(), layer_comparator_r4.begin(),
                            layer_comparator_r4.end());
  }

  const int nStrip = NStrip->GetBinContent(id.Station, id.Ring);
  const int nWireGroup = NWireGroup->GetBinContent(id.Station, id.Ring);
  const int nCFEB = nStrip / 16;
  // draw event display
  TCanvas* c1 = new TCanvas("c1", "c1", 0, 0, 600, 800);

  // the SimHit wire-group pad only exists when there are sim hits (MC);
  // for data it is dropped and the remaining pads move up by one
  const int padSimHitWire = 1;
  const int padWire = doSimHit ? 2 : 1;
  const int padStrip = padWire + 1;
  const int padComparator = padWire + 2;
  const int padText = padWire + 3;
  c1->Divide(1, padText);
  c1->SetRightMargin(0.15);
  c1->SetBottomMargin(0.25);
  c1->SetTopMargin(0.25);
  // legend
  // ignore here
  // SetEventDisplayLegend(legendName);

  // strip display
  c1->cd(padStrip)->SetGridy();
  gPad->SetBottomMargin(0.2);
  TH2F* stripDis =
      new TH2F("stripDis", "", nStrip * 2 + 2, 1, nStrip + 2, 6, 1, 7);
  TH2F* stripDis_text =
      new TH2F("stripDis_text", "", nStrip * 2 + 2, 1, nStrip + 2, 6, 0.5, 6.5);
  TH1F* cfebNotReadOut =
      new TH1F("cfebNotReadOut", "", nStrip + 1, 1, nStrip + 2);
  TPaveText* pt3 = new TPaveText(0.4, 0.90, 0.6, 0.96, "NDC");
  if (doSimHit) {
    SimHitDisplay(
        /*c1,*/ id, layer_simhit, simhit, stripDis, stripDis_text,
        stripOverlays);  // design mark: add separate strip overlay container
    SetTitle(pt3, "SimHit Strips");

    // design mark
    // stripDis->Draw("COLZtext");
    stripDis->Draw("");
    for (auto* o : stripOverlays)
      o->Draw("same");  // design mark: draw only strip overlays
  } else {
    TH1F* cfebNotInstall_me21 = new TH1F("cfebNotInstall_me21", "", 81, 1, 82);
    TH1F* cfebNotInstall_me11 = new TH1F("cfebNotInstall_me11", "", 81, 1, 82);

    TH1F* cfebNotReadOut_comparator = new TH1F(
        "cfebNotReadOut_comparator", "", nStrip * 2 + 2, 1, nStrip * 2 + 3);
    TH1F* cfebNotInstall_comparator_me21 =
        new TH1F("cfebNotInstall_comparator_me21", "", 162, 1, 163);
    TH1F* cfebNotInstall_comparator_me11 =
        new TH1F("cfebNotInstall_comparator_me11", "", 162, 1, 163);

    //           double cfeb[5] = {0, 0, 0, 0, 0};
    double cfeb[nCFEB] = {};
    StripDisplay(/*c1,*/ id, layer_strip, strip, cfeb, stripDis, stripDis_text,
                 cfebNotReadOut, cfebNotInstall_me21, cfebNotInstall_me11);
    MakeShadeForComparatorPanel(
        cfebNotReadOut, cfebNotInstall_me21, cfebNotReadOut_comparator,
        cfebNotInstall_comparator_me21, cfebNotInstall_comparator_me11);
    SetTitle(pt3, "Strip Hits and ADC Count");
    DrawADCCells(stripDis, timeBinCells);
    cfebNotReadOut->Draw("B same");
    if (id.Station == 2) {
      //              cfebNotInstall_me21->Draw("B same");
    } else if (id.Station == 1) {
      //                        cfebNotInstall_me11->Draw("B same");
    }
  }
  DrawCFEBBorders(id, nCFEB, cfebBorders);
  if (overlayRecHits) RecoOverlay(id, 1, rechits, segments, recoOverlays);
  pt3->Draw();

  TH2F* simhitWireDis =
      new TH2F("simhitWireDis", "", nWireGroup + 1, 0, nWireGroup + 1, 6, 1, 7);
  TH2F* simhitWireDis_text = new TH2F("simhitWireDis_text", "", nWireGroup, 0,
                                      nWireGroup + 1, 6, 1, 7);
  TPaveText* pt1 = new TPaveText(0.4, .90, 0.6, 0.96, "NDC");

  if (doSimHit) {
    c1->cd(padSimHitWire)->SetGridy();
    gPad->SetTopMargin(0.15);
    gPad->SetBottomMargin(0.2);
    SimHitWireDisplay(
        id, layer_simhit, simhit, simtracks, simhitWireDis, simhitWireDis_text,
        wireOverlays);  // design mark: use separate wire overlay container
    SetTitle(pt1, "SimHit Wire Groups");

    simhitWireDis->SetMarkerSize(2);
    // design mark
    // simhitWireDis->Draw("COLZ text");
    simhitWireDis->Draw("");
    simhitWireDis_text->Draw("text same");
    for (auto* o : wireOverlays)
      o->Draw("same");  // design mark: draw only wire overlays
    pt1->Draw();
  }

  // show LCTs
  c1->cd(padText);
  gPad->SetTopMargin(0.0);
  gPad->SetBottomMargin(0.0);
  vector<CorrelatedLCT> alcts = findStubsInChamber(id, allalcts);
  vector<CorrelatedLCT> clcts = findStubsInChamber(id, allclcts);
  vector<CorrelatedLCT> lcts = findStubsInChamber(id, alllcts);
  // chamber, run, event
  stringstream ss;
  ss << "Chamber " << (id.Endcap == 1 ? "+" : "-") << id.Station << "/"
     << id.Ring << "/" << id.Chamber << " ";
  ss << " run: " << Run << "  event #" << Event;

  TPaveText* tex1 = new TPaveText(0.1, 0.0, 0.9, 1.0, "NDC");
  // tex1->SetTextFont(42);
  // tex1->AddText("Type0: No matched ALCT & No matched CLCT");
  // tex1->AddText("Type1: Matched ALCT & No matched CLCT");
  // tex1->AddText("Type2: No matched ALCT & Matched CLCT");
  // tex1->AddText("Type3: Matched ALCT & Matched CLCT, but no matched LCT");
  tex1->AddText(ss.str().c_str());

  vector<CorrelatedLCT> alcts_emul;
  vector<CorrelatedLCT> clcts_emul;
  vector<CorrelatedLCT> lcts_emul;
  if (addEmulation) {
    alcts_emul = findStubsInChamber(id, allalcts_emul);
    clcts_emul = findStubsInChamber(id, allclcts_emul);
    lcts_emul = findStubsInChamber(id, alllcts_emul);
  }
  // the boxes show the Emul stubs with emulation, the Data stubs without; that
  // set is tagged A<i>/C<i> in the colour of its box, and its LCT lines name the
  // ALCT/CLCT with the same key wire group / key half-strip
  tex1->AddText(!addEmulation ? "Boxes: " + compareA + " stubs (tagged)"
                : plotDataStubBorders
                    ? "Boxes: " + compareB + " stubs (tagged), dashed: " +
                          compareA + " stubs"
                    : "Boxes: " + compareB + " stubs (tagged)");
  auto addStubLines = [&](const TString& label, const vector<CorrelatedLCT>& a,
                          const vector<CorrelatedLCT>& c,
                          const vector<CorrelatedLCT>& l, int alctBxOffset,
                          int clctBxOffset, bool tagged) {
    for (unsigned int i = 0; i < a.size(); i++) {
      stringstream line;
      if (tagged) line << "A" << i << "   ";
      line << label << " #" << i << " ALCT keyWg " << a[i].keyWG << " Quality "
           << a[i].quality << " Pattern " << a[i].pattern << " bx "
           << a[i].BX + alctBxOffset;
      TText* t = tex1->AddText(line.str().c_str());
      if (tagged) t->SetTextColor(StubColor(i));
    }
    for (unsigned int i = 0; i < c.size(); i++) {
      stringstream line;
      if (tagged) line << "C" << i << "   ";
      line << label << " #" << i << " CLCT HS " << c[i].keyStrip << " Quality "
           << c[i].quality << " Pattern " << c[i].pattern << " Run3_Pattern "
           << c[i].run3_pattern << " CC 0x" << std::hex << c[i].CC << std::dec
           << " bx " << c[i].BX + clctBxOffset;
      TText* t = tex1->AddText(line.str().c_str());
      if (tagged) t->SetTextColor(StubColor(i));
    }
    for (unsigned int i = 0; i < l.size(); i++) {
      stringstream line;
      line << label << " #" << i << " LCT keyWg " << l[i].keyWG << " HS "
           << l[i].keyStrip << " Quality " << l[i].quality << " Pattern "
           << l[i].pattern << " bx " << l[i].BX;
      if (tagged) {
        int ia = -1, ic = -1;
        for (unsigned int j = 0; j < a.size() && ia < 0; j++)
          if (a[j].keyWG == l[i].keyWG) ia = j;
        for (unsigned int j = 0; j < c.size() && ic < 0; j++)
          if (c[j].keyStrip == l[i].keyStrip) ic = j;
        if (ia >= 0 || ic >= 0)
          line << "  = " << (ia >= 0 ? "A" + std::to_string(ia) : "A?") << " + "
               << (ic >= 0 ? "C" + std::to_string(ic) : "C?");
      }
      tex1->AddText(line.str().c_str());
    }
  };
  // data stubs are listed with or without emulation (not for MC with sim hits)
  if (!doSimHit) addStubLines(compareA, alcts, clcts, lcts, 0, 0, !addEmulation);
  if (addEmulation)
    addStubLines(compareB, alcts_emul, clcts_emul, lcts_emul, alct_bx_offset,
                 clct_bx_offset, true);
  /*
          if (lcts.size() >= 1){
              stringstream ss_lct1;
              ss_lct1 <<"#1 LCT keyWg "<< lcts[0].keyWG <<" HS "<<
     lcts[0].keyStrip <<" Quality "<< lcts[0].quality <<" Pattern "<<
     lcts[0].pattern; tex1->AddText(ss_lct1.str().c_str());

          }
          if (lcts.size() >= 2){
              stringstream ss_lct2;
              ss_lct2 <<"#2 LCT keyWg "<< lcts[1].keyWG <<" HS "<<
     lcts[1].keyStrip <<" Quality "<< lcts[1].quality<<" Pattern "<<
     lcts[1].pattern; tex1->AddText(ss_lct2.str().c_str());
          }
          */
  if (overlayRecHits) AddSegmentLines(id, rechits, segments, tex1);
  tex1->Draw();

  // strip hit display
  /*          c1->cd(4)->SetGridy();
          gPad->SetBottomMargin(0.15);
          TGraph* stripHitDis;
          TGraph* comparatorHitNotReadOut;
          vector<vector<double> > sHit_cHit_layer = StripHitDisplay(id,
     layer_strip, layer_comparator, strip, comparator, cfeb); double* sHit =
     &(sHit_cHit_layer[0][0]); double* sHitLayer = &(sHit_cHit_layer[1][0]);
          double* cHit = &(sHit_cHit_layer[2][0]);
          double* cHitLayer = &(sHit_cHit_layer[3][0]);

          if (int(sHit_cHit_layer[0].size()) > 0){
          stripHitDis = new TGraph(int(sHit_cHit_layer[0].size()), sHit,
     sHitLayer); TPaveText *pt2 = new TPaveText(0.4,.95,0.6,0.99, "NDC");
          SetTitle(pt2, "Strip Hit");
          stripHitDis->Draw("ap");
          cfebNotInstall->Draw("B same");
          pt2->Draw();
          SetPlotDetail_StripHit(stripHitDis);

          }

          if (int(sHit_cHit_layer[3].size() > 0)){
          comparatorHitNotReadOut = new TGraph(int(sHit_cHit_layer[2].size()),
     cHit, cHitLayer); comparatorHitNotReadOut->Draw("p same");
          comparatorHitNotReadOut->SetMarkerStyle(24);
          comparatorHitNotReadOut->SetMarkerColor(2);
          }
*/

  // wire display
  c1->cd(padWire)->SetGridy();
  gPad->SetTopMargin(0.15);
  gPad->SetBottomMargin(0.2);
  TH2F* wireDis =
      new TH2F("wireDis", "", nWireGroup + 1, 0, nWireGroup + 1, 6, 1, 7);
  TH2F* wireDis_text =
      new TH2F("wireDis_text", "", nWireGroup, 0, nWireGroup + 1, 6, 1, 7);
  TH2F* alctDis =
      new TH2F("alctDis", "", nWireGroup + 1, 0, nWireGroup + 1, 6, 1, 7);
  TPaveText* pt2 = new TPaveText(0.4, .90, 0.6, 0.96, "NDC");

  if (addEmulation) {
    WireDisplay(id, layer_wire, wire, wireDis, wireDis_text, alctDis,
                alcts_emul);
  } else {
    WireDisplay(id, layer_wire, wire, wireDis, wireDis_text, alctDis, alcts);
  }
  SetTitle(pt2, "Anode Hits");

  // yumeng
  // gStyle->SetPaintTextFormat("bx:%0.0f");
  wireDis->GetZaxis()->SetTitle("BX");
  wireDis->GetZaxis()->SetTitleSize(0.08);
  wireDis->GetZaxis()->SetTitleOffset(0.6);
  wireDis->SetMarkerSize(2);
  DrawTimeBinCells(wireDis, timeBinCells);
  // ALCT boxes, one per stub: the drawn set tagged, the other set dashed on top
  // (so a matching data stub shows as a dashed line along the emulator box)
  std::set<int> wireLayers;
  for (int i : layer_wire) wireLayers.insert(int(wire[i].first.Layer));
  DrawStubBoxes(alctDis, 0, addEmulation ? alcts_emul : alcts, wireLayers, true,
                stubBoxes);
  if (addEmulation && plotDataStubBorders)
    DrawStubBoxes(alctDis, 0, alcts, wireLayers, false, stubBoxes);
  if (overlayRecHits) RecoOverlay(id, 0, rechits, segments, recoOverlays);
  pt2->Draw();

  // comparator display
  c1->cd(padComparator)->SetGridy();
  gPad->SetTopMargin(0.15);  // room for the CLCT tags below the panel title
  gPad->SetBottomMargin(0.2);
  TPaveText* pt4 = new TPaveText(0.4, .90, 0.6, 0.96, "NDC");
  TH2F* comparatorDis =
      new TH2F("comparatorDis", "", nStrip * 2 + 2, 0, nStrip * 2 + 2, 6, 1, 7);
  TH2F* comparatorDis_text = new TH2F("comparatorDis_text", "", nStrip * 2 + 2,
                                      0, nStrip * 2 + 2, 6, 1, 7);
  TH2F* clctDis =
      new TH2F("clctDis", "", nStrip * 2 + 2, 0, nStrip * 2 + 2, 6, 1, 7);
  if (addEmulation)
    ComparatorDisplay(id, layer_comparator, comparator, comparatorDis,
                      comparatorDis_text, clctDis, clcts_emul);
  else
    ComparatorDisplay(id, layer_comparator, comparator, comparatorDis,
                      comparatorDis_text, clctDis, clcts);
  comparatorDis->SetMarkerSize(2);
  // yumeng
  // gStyle->SetPaintTextFormat("bx:%0.0f");
  // wireDis->SetPaintTextFormat("BX%1.0f")
  wireDis->GetZaxis()->SetTitle("BX");
  wireDis->GetZaxis()->SetTitleSize(0.08);
  wireDis->GetZaxis()->SetTitleOffset(0.6);
  DrawTimeBinCells(comparatorDis, timeBinCells);
  // CLCT boxes, one per stub: the drawn set tagged, the other set dashed on top
  std::set<int> comparatorLayers;
  for (int i : layer_comparator)
    comparatorLayers.insert(int(comparator[i].first.Layer));
  DrawStubBoxes(clctDis, 1, addEmulation ? clcts_emul : clcts, comparatorLayers,
                true, stubBoxes);
  if (addEmulation && plotDataStubBorders)
    DrawStubBoxes(clctDis, 1, clcts, comparatorLayers, false, stubBoxes);
  if (overlayRecHits) RecoOverlay(id, 2, rechits, segments, recoOverlays);

  //          cfebNotInstall->Draw("B same");
  //          cfebNotReadOut_comparator->Draw("B same");
  /*
          if (id.Station == 2) {
             cfebNotInstall_comparator_me21->Draw("B same");
             } else if (id.Station == 1) {
                       cfebNotInstall_comparator_me11->Draw("B same");
                       }
*/
  SetTitle(pt4, "Comparator Hits");
  pt4->Draw();

  c1->Update();
  // c1->SaveAs(name + ".png");
  // c1->SaveAs(name + ".C");
  std::vector<TObject*> recHitPageObjects;  // page 2 (overlayRecHits = false)
  if (!recHitPage) {
    c1->SaveAs(name + ".pdf");
  } else {
    // page 1: the usual plot without rechits; page 2: rechit panels with the
    // same axes as Anode Hits / Strip Hits, and the segment lines
    c1->Print(name + ".pdf(");
    TCanvas* c2 = new TCanvas("c2", "c2", 0, 0, 600, 800);
    c2->Divide(1, 3);
    c2->cd(1)->SetGridy();
    gPad->SetTopMargin(0.15);
    gPad->SetBottomMargin(0.2);
    TH2F* recHitWireFrame = (TH2F*)wireDis->Clone("recHitWireFrame");
    recHitWireFrame->Reset();
    recHitWireFrame->GetYaxis()->SetTitleOffset(0.3);  // taller pads than page 1
    recHitWireFrame->Draw("AXIS");
    RecoOverlay(id, 0, rechits, segments, recHitPageObjects);
    TPaveText* ptWire = new TPaveText(0.4, .90, 0.6, 0.96, "NDC");
    SetTitle(ptWire, "RecHit Wire Groups");
    ptWire->Draw();
    c2->cd(2)->SetGridy();
    gPad->SetTopMargin(0.15);
    gPad->SetBottomMargin(0.2);
    TH2F* recHitStripFrame = (TH2F*)stripDis->Clone("recHitStripFrame");
    recHitStripFrame->Reset();
    recHitStripFrame->GetYaxis()->SetTitleOffset(0.3);  // taller pads than page 1
    recHitStripFrame->Draw("AXIS");
    DrawCFEBBorders(id, nCFEB, recHitPageObjects);
    RecoOverlay(id, 1, rechits, segments, recHitPageObjects);
    TPaveText* ptStrip = new TPaveText(0.4, .90, 0.6, 0.96, "NDC");
    SetTitle(ptStrip, "RecHit Strips");
    ptStrip->Draw();
    c2->cd(3);
    gPad->SetTopMargin(0.0);
    gPad->SetBottomMargin(0.0);
    TPaveText* tex2 = new TPaveText(0.1, 0.0, 0.9, 1.0, "NDC");
    tex2->AddText(ss.str().c_str());
    AddSegmentLines(id, rechits, segments, tex2);
    if (segments.empty() && rechits.empty()) tex2->AddText("no CSC rechits in this chamber");
    tex2->Draw();
    c2->Update();
    c2->Print(name + ".pdf)");
    recHitPageObjects.insert(recHitPageObjects.end(),
                             {ptWire, ptStrip, tex2, recHitWireFrame,
                              recHitStripFrame});
    delete c2;
  }

  // design mark: clean up overlay objects to prevent memory leaks
  auto destroy = [](std::vector<TObject*>& v) {
    for (auto* o : v) delete o;
    v.clear();
  };
  destroy(stripOverlays);
  destroy(wireOverlays);
  destroy(recoOverlays);
  destroy(cfebBorders);
  destroy(timeBinCells);
  destroy(stubBoxes);
  destroy(recHitPageObjects);

  delete c1;
  delete wireDis;
  delete stripDis;
  delete wireDis_text;
  delete stripDis_text;
  // yumeng
  delete simhitWireDis;
  delete simhitWireDis_text;

  delete comparatorDis;
  delete comparatorDis_text;
  delete clctDis;
  delete alctDis;
  delete cfebNotReadOut;
  delete tex1;
}

bool ChamberUsedForEventDisplay(CSCDetID id, vector<CSCDetID> usedChamber) {
  bool flag = false;

  for (int i = 0; i < int(usedChamber.size()); i++) {
    CSCDetID tempID = usedChamber[i];

    // yumeng debug
    // if (id.Endcap == tempID.Endcap && id.Station == tempID.Endcap && id.Ring
    // == tempID.Ring &&
    if (id.Endcap == tempID.Endcap && id.Station == tempID.Station &&
        id.Ring == tempID.Ring && id.Chamber == tempID.Chamber) {
      flag = true;
    }
  }

  return flag;
}

void SetSaveNameLegendName(TString& name, TString& legendName, TString address,
                           CSCDetID id, edm::RunNumber_t Run, edm::EventNumber_t Event) {
  // int Run = 1;
  // int Event = 1;
  TString space = "                                   ";
  if (id.Endcap == 1) {
    legendName = "ME+" + NumberToString(id.Station) + "/" +
                 NumberToString(id.Ring) + "/" + NumberToString(id.Chamber) +
                 space + space + "Run #" + NumberToString(Run) + "  " +
                 "Event #" + NumberToString(Event);

    name = address + "/" + NumberToString(Run) + "_" + NumberToString(Event) +
           "_MEPlus" + NumberToString(id.Station) + "_" +
           NumberToString(id.Ring) + "_" + NumberToString(id.Chamber);
  }

  if (id.Endcap == 2) {
    legendName = "ME-" + NumberToString(id.Station) + "/" +
                 NumberToString(id.Ring) + "/" + NumberToString(id.Chamber) +
                 space + space + "Run #" + NumberToString(Run) + "  " +
                 "Event #" + NumberToString(Event);

    name = address + "/" + NumberToString(Run) + "_" + NumberToString(Event) +
           "_MEMinus" + NumberToString(id.Station) + "_" +
           NumberToString(id.Ring) + "_" + NumberToString(id.Chamber);
  }
}

void SaveUsedChamber(CSCDetID id, vector<int> layer_strip,
                     vector<int> layer_wire, vector<int> layer_comparator,
                     vector<CSCDetID>& usedChamber) {
  CSCDetID tempID;

  if (int(layer_strip.size()) > 0 && int(layer_wire.size()) > 0 &&
      int(layer_comparator.size()) > 0) {
    tempID.Endcap = id.Endcap;
    tempID.Station = id.Station;
    tempID.Ring = id.Ring;
    tempID.Chamber = id.Chamber;

    usedChamber.push_back(tempID);
  }
}

void SimHitDisplay(/*TCanvas* c1,*/ CSCDetID id, vector<int>& layer_simhit,
                   vector<SIMHIT>& simhit, TH2F* stripDis, TH2F* stripDis_text,
                   std::vector<TObject*>& simhitLabels) {
  //         c1->cd(4)->SetGridy();

  /*           TH2F* stripDis = new TH2F("stripDis", "", 162, 1, 82, 6, 1, 7);
           TH2F* stripDis_text = new TH2F("stripDis_text", "", 162, 1, 82, 6, 1,
     7); TH1F* cfebNotReadOut = new TH1F("cfebNotReadOut", "", 81, 1, 82);
*/
  for (int i = 0; i < int(layer_simhit.size());
       i++) {  // in each interesting layer has strip hits

    int tempStation = simhit[layer_simhit[i]].first.Station;
    int tempRing = simhit[layer_simhit[i]].first.Ring;
    int tempLayer = simhit[layer_simhit[i]].first.Layer;
    vector<SimHit> tempSimHit = simhit[layer_simhit[i]].second;
    // std::cout <<"display CSC simhits in "<< simhit[layer_simhit[i]].first <<"
    // size of simhits "<< tempSimHit.size() << std::endl;

    // CountCFEB(cfeb, tempsimHit);
    // int option1 = 1;
    int option2 = 2;

    bool doStagger = false;
    if (!(tempStation == 1 && (tempRing == 1 || tempRing == 4)))
      doStagger = true;

    // yumeng
    // MakeOneLayerSimHitDisplay(tempLayer, tempSimHit, stripDis, option2,
    // doStagger);
    MakeOneLayerSimHitDisplay(tempLayer, tempSimHit, stripDis, option2,
                              doStagger, simhitLabels);

    if (tempLayer == id.Layer) {  // chamber level??

      // yumeng
      // MakeOneLayerSimHitDisplay(tempLayer, tempSimHit, stripDis_text,
      // option2, doStagger);
      MakeOneLayerSimHitDisplay(tempLayer, tempSimHit, stripDis_text, option2,
                                doStagger, simhitLabels);
    }
  }

  // yumeng
  // SetHistContour(stripDis, 11, 22);
  // SetHistContour(stripDis, 1, 10);
  // stripDis->GetZaxis()->SetRangeUser(11, 22);
  // stripDis->GetZaxis()->SetRangeUser(1, 10);

  stripDis_text->SetMarkerSize(1.5);

  stripDis->GetZaxis()->SetLabelSize(0.1);
  stripDis->SetStats(0);
  stripDis->GetXaxis()->SetTitle("Strip Number, SimHit");
  stripDis->GetYaxis()->SetTitle("Layer");
  stripDis->GetXaxis()->SetLabelSize(0.1);
  stripDis->GetYaxis()->SetLabelSize(0.1);
  stripDis->GetXaxis()->SetTitleSize(0.11);
  stripDis->GetYaxis()->SetTitleSize(0.11);
  stripDis->GetXaxis()->SetTitleOffset(0.81);
  stripDis->GetYaxis()->SetTitleOffset(0.2);
  stripDis->GetXaxis()->SetNdivisions(1010);
  stripDis->GetYaxis()->SetNdivisions(110);
  stripDis->SetTitle("");

  for (auto* label : simhitLabels) {
    label->Draw("same");
  }
}

// yumeng
void SimHitWireDisplay(CSCDetID id, vector<int>& layer_simhit,
                       vector<SIMHIT>& simhit, vector<SIMTRACK>& simtracks,
                       TH2F* wireDis, TH2F* wireDis_text,
                       std::vector<TObject*>& simhitLabels) {
  for (int i = 0; i < int(layer_simhit.size());
       i++) {  // in each interesting layer has simhit wire hits

    int tempStation = simhit[layer_simhit[i]].first.Station;
    int tempRing = simhit[layer_simhit[i]].first.Ring;
    int tempLayer = simhit[layer_simhit[i]].first.Layer;
    vector<SimHit> tempSimHit = simhit[layer_simhit[i]].second;

    MakeOneLayerSimHitWireDisplay(tempLayer, tempSimHit, simtracks, wireDis,
                                  simhitLabels);

    if (tempLayer == id.Layer) {  // chamber level??
      MakeOneLayerSimHitWireDisplay(tempLayer, tempSimHit, simtracks,
                                    wireDis_text, simhitLabels);
    }
  }

  // SetHistContour(wireDis, 1, 10);
  // wireDis->GetZaxis()->SetRangeUser(1, 10);

  wireDis_text->SetMarkerSize(1.5);

  wireDis->GetZaxis()->SetLabelSize(0.1);
  wireDis->SetStats(0);
  wireDis->GetXaxis()->SetTitle("Wire Group Number, SimHit");
  wireDis->GetYaxis()->SetTitle("Layer");
  wireDis->GetXaxis()->SetLabelSize(0.1);
  wireDis->GetYaxis()->SetLabelSize(0.1);
  wireDis->GetXaxis()->SetTitleSize(0.11);
  wireDis->GetYaxis()->SetTitleSize(0.11);
  wireDis->GetXaxis()->SetTitleOffset(0.81);
  wireDis->GetYaxis()->SetTitleOffset(0.2);
  wireDis->GetXaxis()->SetNdivisions(2012);
  wireDis->GetYaxis()->SetNdivisions(110);
  wireDis->SetTitle("");

  for (auto* label : simhitLabels) {
    label->Draw("same");
  }
}

// yumeng
void MakeOneLayerSimHitWireDisplay(int layer, vector<SimHit>& s,
                                   vector<SIMTRACK>& simtracks,
                                   TH2F* wireDisplay,
                                   std::vector<TObject*>& simhitLabels) {
  // design mark: small symmetric VERTICAL offsets to fan-out coincident hits in
  // the same bin
  static const double OFF_Y[] = {-0.12, 0.0, +0.12, -0.18, +0.18};
  // design mark: computes how many offsets exist (here, 5)
  const int NOFF = (int)(sizeof(OFF_Y) / sizeof(double));

  // design mark: count how many we've already placed per (layer, binX)
  std::map<std::pair<int, int>, int> placed;

  // design mark: Z-range to 1→3(bins have values 1 (non-muon) or 2 (muon)), so
  // the color palette will map only this small range, Giving a subtle
  // background palette(light gray or faint color behind the TMarkers) rather
  // than the full rainbow. so the overlaid TMarkers (colored dots/squares for
  // each particle) visually stand out instead of being drowned in strong color
  // gradients. wireDisplay->GetZaxis()->SetRangeUser(1, 3);

  // design mark: finds which histogram bin corresponds to the wire group number
  // for this SimHit.
  for (const auto& h : s) {
    int bx = wireDisplay->GetXaxis()->FindBin(h.WireGroup);

    std::cout << "\n-------- [ Info of this SIMHIT ] --------" << std::endl;
    std::cout << "Layer " << layer << ", WireGroup " << h.WireGroup
              << ", PDG ID = " << h.PdgId << ", TrackID = " << h.TrackID
              << ", OriginalTrackID = " << h.OriginalTrackID
              << ", ProcessType = " << h.ProcessType << std::endl;

    // Optional background: Assign a simple background intensity to each hit's
    // bin:
    //   - 1 for non-muons, 2 for muons (so muon bins appear brighter).
    //  If multiple hits fall in the same (wireGroup, layer) bin, leep the
    //  higher value so a muon overwrites any non-muon in that spot.
    // This ensures the heatmap indicates both occupancy and particle type
    // (Empty bins (0) → blank, 1->at least one non-muon, 2->at least one muon
    // (or mixed muon+non-muon))
    // float bg = (h.PdgId == 13) ? 2.f : 1.f;
    // wireDisplay->SetBinContent(bx, layer, std::max(bg,
    // (float)wireDisplay->GetBinContent(bx, layer)));

    // design mark: per-bin collision index → pick a small vertical offset
    int k = placed[{layer, bx}]++;
    double dy = OFF_Y[k % NOFF];
    double xcenter = wireDisplay->GetXaxis()->GetBinCenter(bx);

    // design mark: marker style by PDG
    // if pdg id of simhit is 13 (muon hit) => draw circle marker, else star
    int mstyle = (h.PdgId == 13) ? 24 : 29;  // . muon 20, * photon

    // design mark: color by TrackID (stable, small palette), 0~8 shift to 1~9
    int color = 1 + (std::abs(h.TrackID) % 9);  // 1..9
    // Make muon black for priority
    if (h.PdgId == 13) color = kBlack;

    // design mark: place the marker with vertical fan-out inside the bin
    TMarker* m = new TMarker(xcenter, layer + 0.35 + dy, mstyle);
    m->SetMarkerColor(color);
    m->SetMarkerSize(0.8);
    simhitLabels.push_back(m);

    // design mark: small track label
    if (h.PdgId == 13) {
      TLatex* t = new TLatex(xcenter + 0.10, layer + 0.42 + dy,
                             Form("MuTrk:%d", h.TrackID));
      t->SetTextSize(0.045);
      t->SetTextColor(color);
      int alphaBlack = TColor::GetColorTransparent(kBlack, 0.5);
      t->SetTextColor(alphaBlack);
      simhitLabels.push_back(t);
    }
  }
}

// yumeng
void MakeOneLayerSimHitDisplay(int layer, vector<SimHit>& s, TH2F* stripDisplay,
                               int option, bool doStagger,
                               std::vector<TObject*>& simhitLabels) {
  // design mark: use VERTICAL offsets instead of horizontal
  static const double OFF_Y[] = {-0.10, -0.06, +0.06, +0.10, -0.14, +0.14};
  const int NOFF = (int)(sizeof(OFF_Y) / sizeof(double));

  // design mark
  std::map<std::pair<int, int>, int> placed;

  // design mark
  // stripDisplay->GetZaxis()->SetRangeUser(1, 3);

  if (option == 1) {
    for (int i = 0; i < int(s.size()); i++) {
      int x1 = stripDisplay->GetXaxis()->FindBin(s[i].Stripf);
      int x2 = 2 * (s[i].Strip - 1) + 2;
      int x3 = 2 * (s[i].Strip - 1) + 3;

      float value = (s[i].PdgId == 13) ? (s[i].TrackID % 10 + 1) : 10.;
      static int grayIdx = TColor::GetColorTransparent(kGray + 1, 0.2);
      int color = (s[i].PdgId == 13) ? (s[i].TrackID % 10 + 1) : grayIdx;
      TLatex* text =
          new TLatex(x2 + 0.3, layer + 0.1, Form("Trk:%d", s[i].TrackID));
      if (s[i].PdgId == 13) {
        TMarker* marker = new TMarker(x2, layer, 20);  // 20 = full circle
        marker->SetMarkerColor(kBlack);
        marker->SetMarkerStyle(20);
        marker->SetMarkerSize(1.2);
        simhitLabels.push_back(marker);  // Will be drawn in the same loop
      }
      if (doStagger && (layer == 1 || layer == 3 || layer == 5)) {
        stripDisplay->SetBinContent(x2, layer, value);
        stripDisplay->SetBinContent(x3, layer, value);

        text->SetTextSize(0.06);
        text->SetTextColor(color);
        simhitLabels.push_back(text);
      } else {
        stripDisplay->SetBinContent(x1, layer, value);
        stripDisplay->SetBinContent(x2, layer, value);

        text->SetTextSize(0.06);
        text->SetTextColor(color);
        simhitLabels.push_back(text);
      }
    }
  } else if (option == 2) {
    for (const auto& h : s) {
      // Compute the "physics" bin X (respecting staggering)
      int x1 = 2 * (h.Strip - 1) + 1;
      int x2 = 2 * (h.Strip - 1) + 2;
      int bx =
          (doStagger && (layer == 1 || layer == 3 || layer == 5)) ? x2 : x1;

      // Print info for each hit
      std::cout << "\n-------- [ Info of this SIMHIT ] --------" << std::endl;
      std::cout << "Layer " << layer << ", Strip " << h.Strip
                << ", PDG ID = " << h.PdgId << ", TrackID = " << h.TrackID
                << ", OriginalTrackID = " << h.OriginalTrackID
                << ", ProcessType = " << h.ProcessType << std::endl;

      // design mark
      // float bg = (h.PdgId == 13) ? 2.f : 1.f;
      // stripDisplay->SetBinContent(bx, layer, std::max(bg,
      // (float)stripDisplay->GetBinContent(bx, layer)));

      // design mark:
      int k = placed[{layer, bx}]++;
      double dy = OFF_Y[k % NOFF];

      // design mark
      int mstyle = (h.PdgId == 13) ? 24 : 29;

      // design mark
      int color = 1 + (std::abs(h.TrackID) % 9);  // 1..9
      // Make muon black for priority
      if (h.PdgId == 13) color = kBlack;

      // design mark:
      double xcenter = stripDisplay->GetXaxis()->GetBinCenter(bx);

      TMarker* m = new TMarker(xcenter, layer + 0.35 + dy, mstyle);
      m->SetMarkerColor(color);
      m->SetMarkerSize(0.8);
      simhitLabels.push_back(m);

      // design mark
      if (h.PdgId == 13) {
        TLatex* t = new TLatex(xcenter + 0.12, layer + 0.42 + dy,
                               Form("MuTrk:%d", h.TrackID));
        t->SetTextSize(0.045);
        t->SetTextColor(color);
        int alphaBlack = TColor::GetColorTransparent(kBlack, 0.5);
        t->SetTextColor(alphaBlack);
        simhitLabels.push_back(t);
      }
    }
  }
}

void StripDisplay(/*TCanvas* c1,*/ CSCDetID id, vector<int>& layer_strip,
                  vector<STRIP>& strip, double cfeb[], TH2F* stripDis,
                  TH2F* stripDis_text, TH1F* cfebNotReadOut,
                  TH1F* cfebNotInstall_me21, TH1F* cfebNotInstall_me11) {
  //         c1->cd(4)->SetGridy();

  /*           TH2F* stripDis = new TH2F("stripDis", "", 162, 1, 82, 6, 1, 7);
           TH2F* stripDis_text = new TH2F("stripDis_text", "", 162, 1, 82, 6, 1,
     7); TH1F* cfebNotReadOut = new TH1F("cfebNotReadOut", "", 81, 1, 82);
*/
  for (int i = 0; i < int(layer_strip.size());
       i++) {  // in each interesting layer has strip hits

    int tempStation = strip[layer_strip[i]].first.Station;
    int tempRing = strip[layer_strip[i]].first.Ring;
    int tempLayer = strip[layer_strip[i]].first.Layer;
    vector<Strips> tempStrip = strip[layer_strip[i]].second;

    CountCFEB(cfeb, tempStrip);
    int option1 = 1;
    int option2 = 2;

    bool doStagger = false;
    if (!(tempStation == 1 && (tempRing == 1 || tempRing == 4)))
      doStagger = true;

    MakeOneLayerStripDisplay(tempLayer, tempStrip, stripDis, option1,
                             doStagger);

    if (tempLayer == id.Layer) {
      MakeOneLayerStripDisplay(tempLayer, tempStrip, stripDis_text, option2,
                               doStagger);
    }
  }

  //          BlockUnreadCFEB(cfeb, cfebNotReadOut);
  BlockUnreadCFEB(cfeb, cfebNotReadOut, (cfebNotReadOut->GetNbinsX()) / 16);
  BlockNotInstalledCFEB(cfebNotInstall_me21, cfebNotInstall_me11);

  SetHistContour(stripDis, 1, 500);

  cfebNotReadOut->SetFillStyle(3001);
  cfebNotReadOut->SetFillColor(15);
  cfebNotInstall_me21->SetFillStyle(3001);
  cfebNotInstall_me11->SetFillStyle(3001);

  //          TColor *color = gROOT->GetColor("#2F2828");
  cfebNotInstall_me21->SetFillColor(12);
  cfebNotInstall_me11->SetFillColor(12);

  stripDis_text->SetMarkerSize(2);

  stripDis->GetZaxis()->SetLabelSize(0.1);
  stripDis->GetZaxis()->SetRangeUser(1, 500);
  stripDis->SetStats(0);
  stripDis->GetXaxis()->SetTitle("Strip Number");
  stripDis->GetYaxis()->SetTitle("Layer");
  stripDis->GetXaxis()->SetLabelSize(0.1);
  stripDis->GetYaxis()->SetLabelSize(0.1);
  stripDis->GetXaxis()->SetTitleSize(0.11);
  stripDis->GetYaxis()->SetTitleSize(0.11);
  stripDis->GetXaxis()->SetTitleOffset(0.81);
  stripDis->GetYaxis()->SetTitleOffset(0.2);
  stripDis->GetXaxis()->SetNdivisions(1010);
  stripDis->GetYaxis()->SetNdivisions(110);
  stripDis->SetTitle("");
}

void MakeOneLayerStripDisplay(int layer, vector<Strips>& s, TH2F* stripDisplay,
                              int option, bool doStagger) {
  if (option == 1) {
    for (int i = 0; i < int(s.size()); i++) {
      int x1 = 2 * (s[i].Strip - 1) + 1;
      int x2 = 2 * (s[i].Strip - 1) + 2;
      int x3 = 2 * (s[i].Strip - 1) + 3;

      if (doStagger && (layer == 1 || layer == 3 || layer == 5)) {
        stripDisplay->SetBinContent(x2, layer, s[i].MaxADC);
        stripDisplay->SetBinContent(x3, layer, s[i].MaxADC);

      } else {  // if(layer == 2 || layer == 4 || layer ==6){

        stripDisplay->SetBinContent(x1, layer, s[i].MaxADC);
        stripDisplay->SetBinContent(x2, layer, s[i].MaxADC);
      }
    }

  } else if (option == 2) {
    for (int i = 0; i < int(s.size()); i++) {
      int x1 = 2 * (s[i].Strip - 1) + 1;
      int x2 = 2 * (s[i].Strip - 1) + 2;

      if (doStagger && (layer == 1 || layer == 3 || layer == 5)) {
        stripDisplay->SetBinContent(x2, layer, s[i].MaxADC);

      } else {  // if(layer == 2 || layer == 4 || layer == 6){

        stripDisplay->SetBinContent(x1, layer, s[i].MaxADC);
      }
    }
  }
}

void MakeOneLayerALCT(int layer, TH2F* alctDis, vector<CorrelatedLCT>& alcts) {
  for (CorrelatedLCT track : alcts) {
    int position = track.keyWG + 1;
    if (layer == 1) {
      alctDis->SetBinContent(position + 1, layer, 15);
      alctDis->SetBinContent(position + 2, layer, 15);
      alctDis->SetBinContent(position - 1, layer, 15);
      alctDis->SetBinContent(position - 2, layer, 15);
    } else if (layer == 2) {
      alctDis->SetBinContent(position + 1, layer, 15);
      alctDis->SetBinContent(position - 1, layer, 15);
    } else if (layer == 5) {
      alctDis->SetBinContent(position + 1, layer, 15);
      alctDis->SetBinContent(position - 1, layer, 15);
    } else if (layer == 6) {
      alctDis->SetBinContent(position + 1, layer, 15);
      alctDis->SetBinContent(position + 2, layer, 15);
      alctDis->SetBinContent(position - 1, layer, 15);
      alctDis->SetBinContent(position - 2, layer, 15);
    }
    alctDis->SetBinContent(position, layer, 15);
  }
}

void MakeOneLayerWireDisplay(int layer, vector<Wire>& w, TH2F* wireDisplay) {
  for (int i = 0; i < int(w.size()); i++) {
    double time = w[i].TimeBin;

    if (w[i].TimeBin == 0) {
      time += 0.1;
    }

    wireDisplay->SetBinContent(w[i].WireGroup, layer, time);
  }
}

void MakeOneLayerCLCT(int layer, TH2F* clctDis, vector<CorrelatedLCT>& clcts) {
  for (CorrelatedLCT track : clcts) {
    int position = track.keyStrip + 1;
    if (track.run3_pattern == 0) {
      if (layer == 1) {
        position += 3;
      } else if (layer == 2) {
        position += 2;
      } else if (layer == 4) {
        position -= 2;
      } else if (layer == 5) {
        position -= 4;
      } else if (layer == 6) {
        position -= 5;
      }
      clctDis->SetBinContent(position, layer, 15);
      clctDis->SetBinContent(position + 1, layer, 15);
      clctDis->SetBinContent(position + 2, layer, 15);
    } else if (track.run3_pattern == 1) {
      if (layer == 1) {
        position -= 3;
      } else if (layer == 2) {
        position -= 2;
      } else if (layer == 4) {
        position += 2;
      } else if (layer == 5) {
        position += 4;
      } else if (layer == 6) {
        position += 5;
      }
      clctDis->SetBinContent(position, layer, 15);
      clctDis->SetBinContent(position - 1, layer, 15);
      clctDis->SetBinContent(position - 2, layer, 15);
    } else if (track.run3_pattern == 2) {
      if (layer == 1) {
        position += 2;
      } else if (layer == 2) {
        position += 1;
      } else if (layer == 5) {
        position -= 1;
      } else if (layer == 6) {
        position -= 2;
      }
      clctDis->SetBinContent(position, layer, 15);
      clctDis->SetBinContent(position - 1, layer, 15);
      clctDis->SetBinContent(position + 1, layer, 15);
    } else if (track.run3_pattern == 3) {
      if (layer == 1) {
        position -= 2;
      } else if (layer == 2) {
        position -= 1;
      } else if (layer == 5) {
        position += 1;
      } else if (layer == 6) {
        position += 2;
      }
      clctDis->SetBinContent(position, layer, 15);
      clctDis->SetBinContent(position - 1, layer, 15);
      clctDis->SetBinContent(position + 1, layer, 15);
    } else {
      clctDis->SetBinContent(position, layer, 15);
      clctDis->SetBinContent(position - 1, layer, 15);
      clctDis->SetBinContent(position + 1, layer, 15);
    }
  }
}

void MakeOneLayerComparatorDisplay(int layer, vector<Comparator>& c,
                                   TH2F* comparatorDisplay, bool doStagger) {
  for (int i = 0; i < int(c.size()); i++) {
    double time = c[i].TimeBin;
    if (time == 0) {
      time += 0.1;
    }

    int comparator = 2 * (c[i].Strip - 1) + c[i].ComparatorNumber % 2 + 1;

    if (doStagger && (layer == 1 || layer == 3 || layer == 5)) {
      comparatorDisplay->SetBinContent(comparator + 1, layer, time);

    } else {  // if(layer == 2 || layer == 4 || layer ==6){

      comparatorDisplay->SetBinContent(comparator, layer, time);
    }
  }
}

vector<vector<double> > StripHitDisplay(CSCDetID id, vector<int>& layer_strip,
                                        vector<int>& layer_comparator,
                                        vector<STRIP> strip,
                                        vector<COMPARATOR>& comparator,
                                        double cfeb[]) {
  vector<vector<double> > sHit_cHit_layer;

  vector<double> stripHitsContainer;
  vector<double> stripHitsLayerContainer;
  vector<double> comparatorHitsContainer;
  vector<double> comparatorHitsLayerContainer;

  for (int i = 0; i < int(layer_strip.size()); i++) {
    double tempLayer = strip[layer_strip[i]].first.Layer;
    vector<Strips> tempStrip = strip[layer_strip[i]].second;

    vector<double> stripHits = MakeStripHit(tempStrip);
    vector<double> stripHitsLayer(
        int(stripHits.size()),
        tempLayer + 0.5);  // make a vector containing strip hits' layer, since
                           // using tgraph, make it 1.5, 2.5...
    ShiftStripHits(stripHits, tempLayer);

    /*      for (i=0; i<stripHits.size(); i++){

}
*/
    stripHitsContainer.insert(stripHitsContainer.begin(), stripHits.begin(),
                              stripHits.end());
    stripHitsLayerContainer.insert(stripHitsLayerContainer.begin(),
                                   stripHitsLayer.begin(),
                                   stripHitsLayer.end());
  }

  for (int i = 0; i < int(layer_comparator.size()); i++) {
    double tempLayer = comparator[layer_comparator[i]].first.Layer;
    vector<Comparator> tempComparator = comparator[layer_comparator[i]].second;

    vector<double> comparatorHits =
        MakeComparatorHitNotReadout(tempComparator, cfeb);
    vector<double> comparatorHitsLayer(int(comparatorHits.size()),
                                       tempLayer + 0.5);
    ShiftStripHits(comparatorHits, tempLayer);

    comparatorHitsContainer.insert(comparatorHitsContainer.end(),
                                   comparatorHits.begin(),
                                   comparatorHits.end());
    comparatorHitsLayerContainer.insert(comparatorHitsLayerContainer.end(),
                                        comparatorHitsLayer.begin(),
                                        comparatorHitsLayer.end());
  }
  sHit_cHit_layer.push_back(stripHitsContainer);
  sHit_cHit_layer.push_back(stripHitsLayerContainer);
  sHit_cHit_layer.push_back(comparatorHitsContainer);
  sHit_cHit_layer.push_back(comparatorHitsLayerContainer);

  return sHit_cHit_layer;
  //      stripHitDis = new TGraph(int(stripHitsContainer.size()), sHit,
  //      sHitLayer);//stripHitsContainer, stripHitsLayerContainer);
  //      comparatorHitNotReadOut = new
  //      TGraph(int(comparatorHitsContainer.size()), comparatorHitsContainer,
  //      comparatorHitsLayerContainer);

  //      stripHitDis->GetXaxis()->SetRangeUser(1, 82);
  //      stripHitDis->GetYaxis()->SetRangeUser(1, 7);
}

vector<double> MakeComparatorHitNotReadout(vector<Comparator> c,
                                           double cfeb[]) {
  vector<double> cHit;

  for (int i = 0; i < int(c.size()); i++) {
    if (c[i].Strip < 80 && cfeb[int(c[i].Strip / 16)] == 0) {
      cHit.push_back(c[i].Strip + 0.5);

    } else if (c[i].Strip == 80 && cfeb[4] == 0) {
      cHit.push_back(c[i].Strip + 0.5);
    }
  }

  return cHit;
}

void SetTitle(TPaveText* pt, TString name) {
  pt->SetFillColor(0);
  pt->SetTextSize(0.06);
  pt->SetBorderSize(0);
  pt->SetTextAlign(21);
  pt->AddText(name);
}

template <typename T>
string NumberToString(T Number) {
  stringstream ss;
  ss << Number;
  return ss.str();
}

void CountCFEB(double cfeb[], vector<Strips> s) {
  for (int i = 0; i < int(s.size()); i++) {
    int icfeb = (s[i].Strip - 1) / 16;
    cfeb[icfeb]++;
    /*            if (s[i].Strip <= 16){

                cfeb[0]++;

                }else if(s[i].Strip >= 17 && s[i].Strip <= 32){

                        cfeb[1]++;

                        }else if(s[i].Strip >= 33 && s[i].Strip <= 48){

                                   cfeb[2]++;

                                   }else if(s[i].Strip >= 49 && s[i].Strip <=
       64){

                                             cfeb[3]++;

                                             }else if(s[i].Strip >= 65 &&
       s[i].Strip <=80){

                                                     cfeb[4]++;

                                                     } else if (s[i].Strip >= 81
       && s[i].Strip <=96){

                                                               cfeb[5]++;
                                                               }
*/
  }
}

void MakeShadeForComparatorPanel(TH1F* cfebNotReadOut, TH1F* cfebNotInstall,
                                 TH1F* cfebNotReadOut_comparator,
                                 TH1F* cfebNotInstall_comparator_me21,
                                 TH1F* cfebNotInstall_comparator_me11) {
  for (int i = 1; i < cfebNotReadOut->GetSize() - 1; i++) {
    if (cfebNotReadOut->GetBinContent(i) > 0) {
      cfebNotReadOut_comparator->SetBinContent(
          ((i - 1) / 16 * 2) * 16 + (i % 16) + 1, 7);
      cfebNotReadOut_comparator->SetBinContent(
          ((i - 1) / 16 * 2) * 16 + (i % 16) + 17, 7);
    }
  }
  for (int i = 129; i < cfebNotInstall_comparator_me21->GetSize() - 1; i++) {
    cfebNotInstall_comparator_me21->SetBinContent(i, 7);
  }
  for (int i = 97; i < 128; i++) {
    cfebNotInstall_comparator_me11->SetBinContent(i, 7);
  }

  cfebNotInstall_comparator_me21->GetYaxis()->SetRangeUser(1, 7);
  cfebNotInstall_comparator_me11->GetYaxis()->SetRangeUser(1, 7);

  cfebNotReadOut_comparator->GetYaxis()->SetRangeUser(1, 7);

  cfebNotReadOut_comparator->SetFillStyle(3001);
  cfebNotReadOut_comparator->SetFillColor(15);
  cfebNotInstall_comparator_me21->SetFillStyle(3001);
  cfebNotInstall_comparator_me21->SetFillColor(12);
  cfebNotInstall_comparator_me11->SetFillStyle(3001);
  cfebNotInstall_comparator_me11->SetFillColor(12);
}

// void BlockUnreadCFEB(double cfeb[], TH1F* cfebNotReadOut){
void BlockUnreadCFEB(double cfeb[], TH1F* cfebNotReadOut, int nCFEB) {
  //      gStyle->SetPalette(52, 0);

  for (int i = 0; i < nCFEB; i++) {  // in each interesting layer has strip hits

    if (cfeb[i] == 0) {
      //                for (int j = 0; j < 6; j++){//each layer

      for (int k = 0; k < 16; k++) {  // each 16 strips for one cfeb

        //                      if ()

        int x1 = (k + i * 16) + 1;
        //                        int x2 = 2*(k+i*16)+2;
        //                        int x3 = 2*(k+i*16)+3;

        cfebNotReadOut->SetBinContent(x1, 7);
      }

      //                    }

      if (nCFEB == 7 && i == 6) cfebNotReadOut->SetBinContent(113, 7);
      if (nCFEB == 5 && i == 4) cfebNotReadOut->SetBinContent(81, 7);
    }
  }

  cfebNotReadOut->GetYaxis()->SetRangeUser(1, 7);
}

void BlockNotInstalledCFEB(TH1F* cfebNotInstall_me21,
                           TH1F* cfebNotInstall_me11) {
  for (int i = 65; i < cfebNotInstall_me21->GetSize() - 1; i++) {
    cfebNotInstall_me21->SetBinContent(i, 7);
  }
  cfebNotInstall_me21->GetYaxis()->SetRangeUser(1, 7);

  for (int i = 49; i < 65; i++) {
    cfebNotInstall_me11->SetBinContent(i, 7);
  }
  cfebNotInstall_me11->GetYaxis()->SetRangeUser(1, 7);
}

void SetHistContour(TH2F* hist, double Min, double Max) {
  gStyle->SetNumberContours(Max - Min);
  // here the actually interesting code starts
  const Double_t min = Min;
  const Double_t max = Max;

  const Int_t nLevels = Max - Min;
  Double_t levels[nLevels];

  for (int i = 1; i < nLevels; i++) {
    levels[i] = min + (max - min) / (nLevels - 1) * (i);
  }
  levels[0] = 0;

  hist->SetContour((sizeof(levels) / sizeof(Double_t)), levels);
}

vector<double> MakeStripHit(vector<Strips> sp) {
  vector<double> stripHits;

  sort(sp.begin(), sp.end());

  if (int(sp.size()) == 1) {  // one fired strip per layer

    MakeHit1(sp[0], stripHits);

  } else if (int(sp.size() > 1)) {  // more than one fired strip per layer

    for (vector<Strips>::iterator it = sp.begin(); it != sp.end(); it++) {
      if ((*it).ADCMaxTime <= 2 || (*it).ADCMaxTime == 7) {
        (*it).MaxADC = 0;
      }

      if (it == sp.begin()) {  // first strip in layer

        MakeHit2(*it, *(it + 1), stripHits);

      } else if (it == sp.end() - 1) {  // last strip in layer

        MakeHit2(*it, *(it - 1), stripHits);

      } else {
        MakeHit3(*it, *(it - 1), *(it + 1), stripHits);
      }
    }
  }

  return stripHits;
}

void ShiftStripHits(vector<double>& stripHits, double layer) {
  if (layer == 1 || layer == 3 || layer == 5) {
    for (int i = 0; i < int(stripHits.size()); i++) {
      stripHits[i] += 0.5;
    }
  }
}

void MakeHit1(Strips& s, vector<double>& stripHits) {  // deal with strip
                                                       // singlet

  double sHitPos = -99;

  if (s.MaxADC >= 25 && s.ADCMaxTime > 2 && s.ADCMaxTime < 7) {
    sHitPos = s.Strip + 0.5;
  }

  if (sHitPos > 0) {
    stripHits.push_back(sHitPos);
  }
}

void MakeHit2(Strips& s, Strips& sSide,
              vector<double>&
                  stripHits) {  // deal with first last fired strip in one layer

  Strips tempStrip;
  tempStrip.ADCTotal = 0;
  tempStrip.MaxADC = 0;

  if (s.Strip < sSide.Strip) {  // first fired strip in one layer

    if (CheckRight(s, sSide)) {  // have adjacent fired strip on the right

      if (IsGoodPeak(s, tempStrip, sSide)) {  // good peak

        stripHits.push_back(s.Strip + 0.5 + FindRatio(s, tempStrip, sSide));
      }

    } else {  // singlet

      MakeHit1(s, stripHits);
    }

  } else if (s.Strip > sSide.Strip) {  // last fired strip in one layer

    if (CheckLeft(s, sSide)) {  // have adjacent fired strip on the left

      if (IsGoodPeak(s, sSide, tempStrip)) {  // good peak

        stripHits.push_back(s.Strip + 0.5 + FindRatio(s, sSide, tempStrip));
      }

    } else {  // singlet

      MakeHit1(s, stripHits);
    }
  }
}

void MakeHit3(Strips& s, Strips& sSideL, Strips& sSideR,
              vector<double>& stripHits) {
  bool stripOnTheLeft = CheckLeft(s, sSideL);
  bool stripOnTheRight = CheckRight(s, sSideR);

  if (!stripOnTheLeft && !stripOnTheRight) {  // singlet

    MakeHit1(s, stripHits);

  } else if (stripOnTheLeft && !stripOnTheRight) {  // left, c, no right

    MakeHit2(s, sSideL, stripHits);

  } else if (!stripOnTheLeft && stripOnTheRight) {  // no left, c, right

    MakeHit2(s, sSideR, stripHits);

  } else if (stripOnTheLeft && stripOnTheRight) {  // left , c, right

    if (IsGoodPeak(s, sSideL, sSideR)) {
      stripHits.push_back(s.Strip + 0.5 + FindRatio(s, sSideL, sSideR));
    }
  }
}

bool IsGoodPeak(Strips& s, Strips& sSideL, Strips& sSideR) {
  bool goodPeak = false;

  if ((s.MaxADC + sSideL.MaxADC + sSideR.MaxADC) >
          25 &&  // toal charge of strip cluster > 25
      (s.ADCMaxTime > 2 && s.ADCMaxTime < 7) &&  // peak time >2 < 7
      (s.MaxADC >= sSideL.MaxADC &&
       s.MaxADC >= sSideR.MaxADC)) {  // is a peak..

    goodPeak = true;
  }

  return goodPeak;
}

double FindRatio(Strips& s, Strips& sSideL, Strips& sSideR) {
  double ratio = -99;

  ratio = 0.5 * (sSideR.ADCTotal - sSideL.ADCTotal) /
          (s.ADCTotal - min(sSideL.ADCTotal, sSideR.ADCTotal));

  return ratio;
}

bool CheckRight(Strips& s, Strips& sR) {
  //      double chargeOfRightStrip = -99;
  bool nextToThisStrip = false;

  if (sR.Strip - s.Strip == 1) {
    //         chargeOfRightStrip = s2.ADCTotal;
    nextToThisStrip = true;
  }

  //      return chargeOfRightStrip;
  return nextToThisStrip;
}

bool CheckLeft(Strips& s, Strips& sL) {
  //        double chargeOfLeftStrip = -99;
  bool nextToThisStrip = false;

  if (s.Strip - sL.Strip == 1) {
    //           chargeOfLeftStrip = s2.ADCTotal;
    nextToThisStrip = true;
  }

  //        return chargeOfLeftStrip;
  return nextToThisStrip;
}

template <class T>
vector<int> FindChamberIndex(CSCDetID id, vector<T>& vec) {
  vector<int> chamber;
  for (int i = 0; i < int(vec.size()); i++) {
    CSCDetID tempID = vec[i].first;

    if (id.Endcap == tempID.Endcap && id.Station == tempID.Station &&
        id.Ring == tempID.Ring && id.Chamber == tempID.Chamber) {
      chamber.push_back(i);
    }
  }

  return chamber;
}

void WireDisplay(CSCDetID id, vector<int>& layer_wire, vector<WIRE>& wire,
                 TH2F* wireDis, TH2F* wireDis_text, TH2F* alctDis,
                 vector<CorrelatedLCT>& alcts) {
  if (id.Ring == 4) id.Ring = 1;  // me11a wire digi are saved in me11b

  for (int i = 0; i < int(layer_wire.size());
       i++) {  // in each interesting layer has wire hits

    int tempLayer = wire[layer_wire[i]].first.Layer;
    vector<Wire> tempWire = wire[layer_wire[i]].second;

    MakeOneLayerWireDisplay(tempLayer, tempWire, wireDis);

    if (tempLayer == id.Layer) {
      MakeOneLayerWireDisplay(tempLayer, tempWire, wireDis_text);
    }
    MakeOneLayerALCT(tempLayer, alctDis, alcts);
  }

  SetHistContour(wireDis, 0, 16);

  wireDis_text->SetMarkerSize(2);

  wireDis->GetZaxis()->SetLabelSize(0.1);
  wireDis->GetZaxis()->SetRangeUser(0, 16);
  wireDis->SetStats(0);
  wireDis->GetXaxis()->SetTitle("Wire Group Number");
  wireDis->GetYaxis()->SetTitle("Layer");
  wireDis->GetXaxis()->SetLabelSize(0.1);
  wireDis->GetYaxis()->SetLabelSize(0.1);
  wireDis->GetXaxis()->SetTitleSize(0.11);
  wireDis->GetYaxis()->SetTitleSize(0.11);
  wireDis->GetXaxis()->SetTitleOffset(0.81);
  wireDis->GetYaxis()->SetTitleOffset(0.2);
  wireDis->SetTitle("");  //"Anode Hit Timing");
  wireDis->GetXaxis()->SetNdivisions(2012);
  wireDis->GetYaxis()->SetNdivisions(110);
}

/*void SetEventDisplayLegend(TString legendName){

          TLegend* leg = new TLegend(0.05, 0.95, .2, .99, "","brNDC");
          leg->AddEntry((TObject*)0, legendName, "lpf");
          leg->SetFillColor(0);
          leg->SetBorderSize(0);
          leg->SetTextSize(0.1);
          leg->Draw();


}*/

void ComparatorDisplay(CSCDetID id, vector<int>& layer_comparator,
                       vector<COMPARATOR>& comparator, TH2F* comparatorDis,
                       TH2F* comparatorDis_text, TH2F* clctDis,
                       vector<CorrelatedLCT>& clcts) {
  // cout <<"ComparatorDisplay "<<  id.Endcap << "," << id.Station << "," <<
  // id.Ring << "," << id.Chamber << endl; cout << "layer_comparator: " <<
  // layer_comparator.size() << endl;

  for (int i = 0; i < int(layer_comparator.size());
       i++) {  // in each interesting layer has wire hits

    int tempStation = comparator[layer_comparator[i]].first.Station;
    int tempRing = comparator[layer_comparator[i]].first.Ring;
    int tempLayer = comparator[layer_comparator[i]].first.Layer;
    vector<Comparator> tempComparator = comparator[layer_comparator[i]].second;
    // cout <<" looking for id  "<< comparator[layer_comparator[i]].first <<"
    // comparator size "<< tempComparator.size()  << endl;
    if (tempComparator.size() == 0)
      cout << "Error!! No Comparator is found in "
           << comparator[layer_comparator[i]].first << endl;
    // for (auto c : tempComparator) cout <<
    // c.ComparatorNumber<<","<<c.Strip<<","<<c.TimeBin <<";  "; cout << endl;

    bool doStagger = false;
    if (!(tempStation == 1 && (tempRing == 1 || tempRing == 4)))
      doStagger = true;

    MakeOneLayerComparatorDisplay(tempLayer, tempComparator, comparatorDis,
                                  doStagger);

    if (tempLayer == id.Layer) {
      MakeOneLayerComparatorDisplay(tempLayer, tempComparator,
                                    comparatorDis_text, doStagger);
    }
    MakeOneLayerCLCT(tempLayer, clctDis, clcts);
  }

  SetHistContour(comparatorDis, 0, 16);
  SetHistContour(clctDis, 0, 16);

  comparatorDis_text->SetMarkerSize(2);

  comparatorDis->GetZaxis()->SetLabelSize(0.1);
  comparatorDis->GetZaxis()->SetRangeUser(0, 16);
  comparatorDis->SetStats(0);
  comparatorDis->GetXaxis()->SetTitle("Comparator Halfstrip");
  comparatorDis->GetYaxis()->SetTitle("Layer");
  comparatorDis->GetXaxis()->SetLabelSize(0.1);
  comparatorDis->GetYaxis()->SetLabelSize(0.1);
  comparatorDis->GetXaxis()->SetTitleSize(0.11);
  comparatorDis->GetYaxis()->SetTitleSize(0.11);
  comparatorDis->GetXaxis()->SetTitleOffset(0.81);
  comparatorDis->GetYaxis()->SetTitleOffset(0.2);
  comparatorDis->SetTitle("");  //"Comparator Hit Timing");
  comparatorDis->GetXaxis()->SetNdivisions(2016);
  comparatorDis->GetYaxis()->SetNdivisions(110);

  clctDis->GetZaxis()->SetLabelSize(0.1);
  clctDis->GetZaxis()->SetRangeUser(0, 16);
  clctDis->SetStats(0);
  clctDis->GetXaxis()->SetLabelSize(0.1);
  clctDis->GetYaxis()->SetLabelSize(0.1);
  clctDis->GetXaxis()->SetTitleSize(0.11);
  clctDis->GetYaxis()->SetTitleSize(0.11);
  clctDis->GetXaxis()->SetTitleOffset(0.81);
  clctDis->GetYaxis()->SetTitleOffset(0.2);
  clctDis->GetXaxis()->SetNdivisions(2016);
  clctDis->GetYaxis()->SetNdivisions(110);
}

void SetPlotDetail_StripHit(TGraph* stripHitDis) {
  stripHitDis->GetXaxis()->SetLimits(1, 82);
  stripHitDis->GetYaxis()->SetRangeUser(1, 7);
  stripHitDis->SetMarkerStyle(20);
  stripHitDis->GetXaxis()->SetTitle("Local X[Strip Width]");
  stripHitDis->GetYaxis()->SetTitle("Layer");
  stripHitDis->GetXaxis()->SetLabelSize(0.1);
  stripHitDis->GetYaxis()->SetLabelSize(0.1);
  stripHitDis->GetXaxis()->SetTitleSize(0.11);
  stripHitDis->GetYaxis()->SetTitleSize(0.11);
  stripHitDis->GetXaxis()->SetTitleOffset(0.81);
  stripHitDis->GetYaxis()->SetTitleOffset(0.2);
  stripHitDis->GetXaxis()->SetNdivisions(1010);
  stripHitDis->GetYaxis()->SetNdivisions(110);
  stripHitDis->SetTitle();  //"Strip Hit");
}

void RecoOverlay(CSCDetID id, int axis, const vector<RecHitPoint>& rechits,
                 const vector<SegmentLine>& segments,
                 std::vector<TObject*>& overlays) {
  // same stagger convention as the strip/comparator digis
  const bool doStagger = !(id.Station == 1 && (id.Ring == 1 || id.Ring == 4));
  // panel x of a point; strip s spans [s-1, s) in RecHitPoint/SegmentLine and
  // [s, s+1) on the strip panel; wire group w spans [w-1, w) on the wire panel
  auto xOf = [&](int layer, double strip, int wireGroup) {
    const double stagger = (doStagger && layer % 2 == 1) ? 1. : 0.;
    if (axis == 0) return wireGroup - 0.5;
    if (axis == 1) return strip + 1. + 0.5 * stagger;
    return 2. * strip + stagger;
  };

  for (unsigned int i = 0; i < segments.size(); i++) {
    const SegmentLine& seg = segments[i];
    std::vector<double> x, y;
    for (unsigned int j = 0; j < seg.layer.size(); j++) {
      x.push_back(xOf(seg.layer[j], seg.strip[j], seg.wireGroup[j]));
      y.push_back(seg.layer[j] + 0.5);
    }
    if (x.size() < 2) continue;
    TPolyLine* line = new TPolyLine(x.size(), x.data(), y.data());
    line->SetLineColor(kSegmentColors[i % kNSegmentColors]);
    line->SetLineWidth(2);
    line->Draw();
    overlays.push_back(line);
  }
  for (const RecHitPoint& hit : rechits) {
    TMarker* marker =
        new TMarker(xOf(hit.layer, hit.strip, hit.wireGroup), hit.layer + 0.5,
                    hit.onSegment ? 20 : 24);
    marker->SetMarkerColor(kBlack);
    marker->SetMarkerSize(0.8);
    marker->Draw();
    overlays.push_back(marker);
  }
}

// colour key in the right margin of the current pad: one box per colour,
// first colour at the bottom; empty labels are not drawn
void DrawColorKey(const std::vector<int>& colors,
                  const std::vector<TString>& labels, const char* title,
                  std::vector<TObject*>& objects) {
  const double xmin = gPad->GetUxmin(), xmax = gPad->GetUxmax();
  const double ymin = gPad->GetUymin(), ymax = gPad->GetUymax();
  const double x1 = xmax + 0.015 * (xmax - xmin);
  const double x2 = xmax + 0.04 * (xmax - xmin);
  const double dy = (ymax - ymin) / colors.size();
  for (unsigned int i = 0; i < colors.size(); i++) {
    TBox* key = new TBox(x1, ymin + i * dy, x2, ymin + (i + 1) * dy);
    key->SetFillColor(colors[i]);
    key->SetLineColor(colors[i]);
    key->Draw();
    objects.push_back(key);
    if (i < labels.size() && labels[i] != "") {
      TLatex* keyLabel = new TLatex(x2 + 0.005 * (xmax - xmin),
                                    ymin + (i + 0.5) * dy, labels[i]);
      keyLabel->SetTextAlign(12);
      keyLabel->SetTextSize(0.06);
      keyLabel->Draw();
      objects.push_back(keyLabel);
    }
  }
  TLatex* keyTitle = new TLatex(x1, ymax + 0.025 * (ymax - ymin), title);
  keyTitle->SetTextAlign(11);
  keyTitle->SetTextSize(0.06);
  keyTitle->Draw();
  objects.push_back(keyTitle);
}

// filled box for histogram bin (ix, iy)
static TBox* CellBox(TH2F* hist, int ix, int iy, int color) {
  TBox* cell = new TBox(hist->GetXaxis()->GetBinLowEdge(ix),
                        hist->GetYaxis()->GetBinLowEdge(iy),
                        hist->GetXaxis()->GetBinUpEdge(ix),
                        hist->GetYaxis()->GetBinUpEdge(iy));
  cell->SetFillColor(color);
  cell->SetLineColor(color);
  cell->Draw();
  return cell;
}

void DrawTimeBinCells(TH2F* hist, std::vector<TObject*>& objects) {
  // cells: content = time bin (time bin 0 is stored as 0.1)
  hist->Draw("AXIS");
  gPad->Update();
  for (int ix = 1; ix <= hist->GetNbinsX(); ix++) {
    for (int iy = 1; iy <= hist->GetNbinsY(); iy++) {
      const double value = hist->GetBinContent(ix, iy);
      if (value <= 0) continue;
      const int timeBin = int(value);
      const int color = TColor::GetColor(
          kTimeBinColorsHex[std::min(std::max(timeBin, 0), kNTimeBinColors - 1)]);
      objects.push_back(CellBox(hist, ix, iy, color));
      TLatex* label = new TLatex(hist->GetXaxis()->GetBinCenter(ix),
                                 hist->GetYaxis()->GetBinCenter(iy),
                                 Form("%d", timeBin));
      label->SetTextAlign(22);
      label->SetTextColor(kWhite);
      label->SetTextFont(62);
      label->SetTextSize(0.06);
      label->Draw();
      objects.push_back(label);
    }
  }

  std::vector<int> colors;
  for (const char* hex : kTimeBinColorsHex) colors.push_back(TColor::GetColor(hex));
  std::vector<TString> labels(kNTimeBinColors, "");
  for (int t = 0; t < kNTimeBinColors; t += 3) labels[t] = Form("%d", t);
  labels[kNTimeBinColors - 1] = Form("%d+", kNTimeBinColors - 1);
  DrawColorKey(colors, labels, "time bin", objects);
  gPad->RedrawAxis();
}

void DrawADCCells(TH2F* hist, std::vector<TObject*>& objects) {
  // cells: content = max ADC above pedestal of the strip (0 = below threshold)
  hist->Draw("AXIS");
  gPad->Update();
  std::vector<int> colors;
  for (const char* hex : kADCColorsHex) colors.push_back(TColor::GetColor(hex));
  const int nBands = colors.size();
  for (int ix = 1; ix <= hist->GetNbinsX(); ix++) {
    for (int iy = 1; iy <= hist->GetNbinsY(); iy++) {
      const double adc = hist->GetBinContent(ix, iy);
      if (adc <= 0) continue;
      const int band = std::min(int(adc / kADCBandWidth), nBands - 1);
      objects.push_back(CellBox(hist, ix, iy, colors[band]));
    }
  }

  std::vector<TString> labels(nBands, "");
  for (int b = 0; b < nBands - 1; b += 2) labels[b] = Form("%d", int(b * kADCBandWidth));
  labels[nBands - 1] = Form("%d+", int((nBands - 1) * kADCBandWidth));
  DrawColorKey(colors, labels, "max ADC", objects);
  gPad->RedrawAxis();
}

void DrawStubBoxes(TH2F* frame, int kind, vector<CorrelatedLCT>& stubs,
                   const std::set<int>& layers, bool tagged,
                   std::vector<TObject*>& objects) {
  // tags already drawn; a stub close to one of them is added to that label
  std::vector<std::pair<double, TLatex*>> tags;
  const double xRange = frame->GetXaxis()->GetXmax() - frame->GetXaxis()->GetXmin();
  for (unsigned int i = 0; i < stubs.size(); i++) {
    // cells of this stub only, with the usual envelope shapes
    vector<CorrelatedLCT> one(1, stubs[i]);
    TH2F* cells = (TH2F*)frame->Clone("stubCells");
    cells->Reset();
    for (int layer : layers) {
      if (kind == 0)
        MakeOneLayerALCT(layer, cells, one);
      else
        MakeOneLayerCLCT(layer, cells, one);
    }
    const int color = tagged ? StubColor(i) : kGray + 2;
    for (int iy = 1; iy <= cells->GetNbinsY(); iy++) {
      int first = 0, last = 0;
      for (int ix = 1; ix <= cells->GetNbinsX(); ix++) {
        if (cells->GetBinContent(ix, iy) <= 0) continue;
        if (first == 0) first = ix;
        last = ix;
      }
      if (first == 0) continue;
      const double x1 = cells->GetXaxis()->GetBinLowEdge(first);
      const double x2 = cells->GetXaxis()->GetBinUpEdge(last);
      const double y1 = cells->GetYaxis()->GetBinLowEdge(iy);
      const double y2 = cells->GetYaxis()->GetBinUpEdge(iy);
      if (tagged) {
        TBox* hatch = new TBox(x1, y1, x2, y2);
        hatch->SetFillStyle(3244);
        hatch->SetFillColorAlpha(kBlack, 0.5);
        hatch->Draw();
        objects.push_back(hatch);
      }
      if (tagged) {
        TBox* outline = new TBox(x1, y1, x2, y2);
        outline->SetFillStyle(0);
        outline->SetLineColor(color);
        outline->SetLineWidth(2);
        outline->Draw();
        objects.push_back(outline);
      } else {
        // two-tone dashed outline (black dashes over a white line, visible on
        // dark cells and the white background), drawn slightly inside the box
        // so the coloured border of a matching tagged stub stays visible
        const double dx = std::min(0.004 * xRange, 0.25 * (x2 - x1));
        const double dy = 0.12 * (y2 - y1);
        for (int pass = 0; pass < 2; pass++) {
          TBox* outline = new TBox(x1 + dx, y1 + dy, x2 - dx, y2 - dy);
          outline->SetFillStyle(0);
          outline->SetLineColor(pass == 0 ? kWhite : kBlack);
          outline->SetLineStyle(pass == 0 ? 1 : 2);
          outline->SetLineWidth(2);
          outline->Draw();
          objects.push_back(outline);
        }
      }
    }
    delete cells;

    if (tagged) {
      // tag above the panel at the key wire group / key half-strip, each name
      // in its stub colour; stubs close together share one label ("C1,C2")
      const double x = (kind == 0 ? stubs[i].keyWG : stubs[i].keyStrip) + 0.5;
      const TString name =
          Form("#color[%d]{%s%d}", StubColor(i), kind == 0 ? "A" : "C", i);
      TLatex* near = nullptr;
      for (auto& t : tags)
        if (std::abs(t.first - x) < 0.03 * xRange) near = t.second;
      if (near) {
        near->SetTitle(TString(near->GetTitle()) + "," + name);
      } else {
        TLatex* tag = new TLatex(x, 7.05, name);
        tag->SetTextAlign(21);
        tag->SetTextSize(0.06);
        tag->Draw();
        objects.push_back(tag);
        tags.emplace_back(x, tag);
      }
    }
  }
}

void DrawCFEBBorders(CSCDetID id, int nCFEB, std::vector<TObject*>& objects) {
  // 16 strips per CFEB, strip s drawn at [s, s+1), so CFEB k starts at
  // x = 16*k + 1; in ME1/1 the ME1b/ME1a border (strip 65) is solid
  for (int k = 1; k < nCFEB; k++) {
    const double x = 16 * k + 1;
    TLine* border = new TLine(x, 1, x, 7);
    const bool me1aBorder = id.Station == 1 && id.Ring == 1 && k == 4;
    border->SetLineStyle(me1aBorder ? 1 : 2);
    border->SetLineWidth(me1aBorder ? 3 : 2);
    border->SetLineColor(kBlack);
    border->Draw();
    objects.push_back(border);
  }
}

void AddSegmentLines(CSCDetID id, const vector<RecHitPoint>& rechits,
                     const vector<SegmentLine>& segments, TPaveText* text) {
  // CSC segments, key positions on layer 3 in LCT units (0-based half-strip and
  // wire group); like the CLCT key half-strip, it includes the +1 half-strip
  // stagger of layer 3 outside ME1/1
  const int keyLayerStagger =
      (id.Station == 1 && (id.Ring == 1 || id.Ring == 4)) ? 0 : 1;
  for (unsigned int iseg = 0; iseg < segments.size(); iseg++) {
    const SegmentLine& seg = segments[iseg];
    stringstream ss_seg;
    ss_seg << "RecoSeg #" << iseg << " nHits " << seg.nHits << " chi2/ndof "
           << std::fixed << std::setprecision(1) << seg.chi2 << "/" << seg.ndof;
    for (unsigned int j = 0; j < seg.layer.size(); j++) {
      if (seg.layer[j] != 3) continue;
      ss_seg << " key HS " << int(2 * seg.strip[j]) + keyLayerStagger
             << " key WG "
             << seg.wireGroup[j] - 1;
    }
    text->AddText(ss_seg.str().c_str())
        ->SetTextColor(kSegmentColors[iseg % kNSegmentColors]);
  }
  if (!rechits.empty())
    text->AddText("RecHits: filled = on a RecoSeg, open = not on a segment");
}
