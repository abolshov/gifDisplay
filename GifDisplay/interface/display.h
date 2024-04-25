#ifndef gifDisplay_GifDisplay_display_h 
#define gifDisplay_GifDisplay_display_h 


#include "gifDisplay/GifDisplay/interface/structForDisplay.h"
#include <iostream>
#include <fstream>
#include <list>
#include <vector>
#include <iomanip>
#include <ctime>
#include <string>
#include <cmath>

#include "TF1.h"
#include "TH2.h"
#include "TString.h"
#include "TPaveText.h"
#include "TGraph.h"
#define PI 3.14159265
using namespace std;

typedef pair<GEMDetID, vector<SimHit> > GEMSIMHIT;
typedef pair<GEMDetID, vector<GEMPad> > GEMPAD;
typedef pair<GEMDetID, vector<GEMPadCluster> > GEMCLUSTER;
typedef pair<CSCDetID, vector<SimHit> > SIMHIT;
typedef pair<CSCDetID, vector<Wire> > WIRE;
typedef pair<CSCDetID, vector<Strips> > STRIP;
typedef pair<CSCDetID, vector<Comparator> > COMPARATOR;
typedef pair<CSCDetID, vector<CorrelatedLCT> > CSCIDLCTs;

void WireStripDisplay(TString address, CSCDetID id, vector<SIMHIT> &simhits, vector<WIRE> &wire, vector<STRIP> &strip, vector<COMPARATOR> &comparator, 
			vector<CSCIDLCTs> &idalcts,vector<CSCIDLCTs> &idalcts_emul, 
			vector<CSCIDLCTs> &idclcts,vector<CSCIDLCTs> &idclcts_emul, 
			vector<CSCIDLCTs> &idlcts,vector<CSCIDLCTs> &idlcts_emul, 
			vector<CSCDetID> &usedChamber, int Run, int Event, bool addEmulation, int doDebug=0);
bool ChamberUsedForEventDisplay(CSCDetID id, vector<CSCDetID> usedChamber);
bool GEMChamberUsedForEventDisplay(CSCDetID id, vector<CSCDetID> usedChamber);
void SetSaveNameLegendName(TString& name, TString& legendName, TString address, CSCDetID id, int Run, int Event);
void SaveUsedChamber(CSCDetID id, vector<int> layer_strip, vector<int> layer_wire, vector<int> layer_comparator, vector<CSCDetID> &usedChamber);
void StripDisplay(CSCDetID id, vector<int>& layer_strip, vector<STRIP>& strip, double cfeb[], TH2F* stripDis, TH2F* stripDis_text, TH1F* cfebNotReadOut, TH1F* cfebNotInstall_me21, TH1F* cfebNotInstall_me11);
void SimHitDisplay(CSCDetID id, vector<int>& layer_simhit, vector<SIMHIT>& simhit, TH2F* stripDis, TH2F* stripDis_text);
void MakeOneLayerStripDisplay(int layer, vector<Strips> &s, TH2F* stripDisplay, int option, bool doStagger);
void MakeOneLayerSimHitDisplay(int layer, vector<SimHit> &s, TH2F* stripDisplay, int option, bool doStagger);
void MakeOneLayerWireDisplay(int layer, vector<Wire> &w, TH2F* wireDisplay);
void MakeOneLayerComparatorDisplay(int layer, vector<Comparator> &c, TH2F* comparatorDisplay, bool doStagger);
void MakeOneLayerALCT(int layer, TH2F* alctDis, vector<CorrelatedLCT>& alcts);
void MakeOneLayerCLCT(int layer, TH2F* clctDis, vector<CorrelatedLCT>& clcts);
vector<vector<double> > StripHitDisplay(CSCDetID id, vector<int>& layer_strip, vector<int>& layer_comparator, vector<STRIP> strip, vector<COMPARATOR> &comparator, double cfeb[]);
vector<double> MakeComparatorHitNotReadout(vector<Comparator> c, double cfeb[]);
void SetTitle(TPaveText* pt, TString name);
template <typename T>
string NumberToString( T Number );
void CountCFEB(double cfeb[], vector<Strips> s);
void MakeShadeForComparatorPanel(TH1F* cfebNotReadOut,  TH1F* cfebNotInstall, TH1F* cfebNotReadOut_comparator,  TH1F* cfebNotInstall_comparator_me21, TH1F* cfebNotInstall_comparator_me11);
void BlockUnreadCFEB(double cfeb[], TH1F* cfebNotReadOut, int nCFEB);
void BlockNotInstalledCFEB(TH1F* cfebNotInstall_me21, TH1F* cfebNotInstall_me11);
void SetHistContour(TH2F* hist, double Min, double Max);
vector<double> MakeStripHit(vector<Strips> sp);
void ShiftStripHits(vector<double>& stripHits, double layer);
void MakeHit1(Strips &s, vector<double>& stripHits);//deal with strip singlet
void MakeHit2(Strips &s, Strips &sSide, vector<double>& stripHits);// deal with first last fired strip in one layer
void MakeHit3(Strips &s, Strips &sSideL, Strips  &sSideR, vector<double>& stripHits);
bool IsGoodPeak(Strips &s, Strips &sSideL, Strips& sSideR);
double FindRatio(Strips &s, Strips& sSideL, Strips& sSideR);
bool CheckRight(Strips &s, Strips &sR);
bool CheckLeft(Strips &s, Strips &sL);
template <class T>
vector<int> FindChamberIndex(CSCDetID id, vector<T> &vec);
void WireDisplay(CSCDetID id, vector<int>& layer_wire, vector<WIRE>& wire, TH2F* wireDis, TH2F* wireDis_text, TH2F* alctDis, vector<CorrelatedLCT>& alcts);
//void SetEventDisplayLegend(TString legendName);
void ComparatorDisplay(CSCDetID id, vector<int>& layer_comparator, vector<COMPARATOR>& comparator, TH2F* comparatorDis, TH2F* comparatorDis_text, TH2F* clctDis, vector<CorrelatedLCT>& clcts);
void SetPlotDetail_StripHit(TGraph* stripHitDis);

vector<CorrelatedLCT> findStubsInChamber(CSCDetID id, vector<CSCIDLCTs> alllcts);


//GEM display part
void GEMPadDisplay(TString address, GEMDetID id, CSCDetID CSCid, vector<GEMSIMHIT> &simhits, vector<GEMPAD> & gempads, vector<GEMCLUSTER> &gemclusters,\
vector<GEMDetID> &usedChamber, vector<CSCIDLCTs> &alllcts, vector<CSCIDLCTs> &alllcts_emul, int Run, int Event, bool addEmulation, bool doGEMCSC, int doDebug= 0);
void SetSaveNameLegendNameGEM(TString& name, TString& legendName, TString address, GEMDetID id, int Run, int Event);
void GEMSimHitDisplay(GEMDetID id, vector<GEMDetID>& allIds, vector<GEMSIMHIT>& gemsimHits, TH2F* stripDis, int doDebug=0);
void MakeOneLayerGEMSimHitDisplay(GEMDetID tempId, vector<SimHit> &shs, TH2F* stripDisplay);
void PadDigiDisplay(GEMDetID id, vector<GEMDetID>& allIds, vector<GEMPAD>& gempads, TH2F*padDis,int doDebug=0);
void MakeOneLayerGEMPadDisplay(GEMDetID tempId, vector<GEMPad> &pads, TH2F* padDisplay);
vector<GEMPadCluster> findClustersInChamber(GEMDetID id, vector<GEMCLUSTER>& allcluster);
pair<int, int> EighthStripToPad(int eighthStrip, bool evenChamber, bool ME1a, bool ME1b, string luts_folder);
pair<int, int> HalfStripToPad(int halfStrip, bool evenChamber, bool ME1a, bool ME1b, string luts_folder);
int slopePropagation(int slope, int layer, bool evenChamber, bool ME1a, bool ME1b, string luts_folder);
int WireToRoll(int wireGroup, int layer, bool evenChamber, string luts_folder);
void LCTPropagationDisplay(GEMDetID id, vector<CorrelatedLCT> &lcts, TH2F* extrapLCT, TH2F* unextrapLCT, string luts_folder);

#endif
