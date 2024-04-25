#include "gifDisplay/GifDisplay/interface/display.h"

#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <math.h>
#include <vector>
#include <list>
#include <iterator>
#include <boost/lexical_cast.hpp>
#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include "Rtypes.h"

#include "TF1.h"
#include "TH2.h"
#include "TStyle.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TPaletteAxis.h"
#include "TMath.h"
#include "TPaveText.h"
#include "TLegend.h"
#include "TLorentzVector.h"
#define PI 3.14159265


void GEMPadDisplay(TString address, GEMDetID id, CSCDetID CSCid, vector<GEMSIMHIT> &simhit, vector<GEMPAD> &gempads, vector<GEMCLUSTER> &gemclusters, vector<GEMDetID> &usedChamber, vector<CSCIDLCTs> &alllcts, vector<CSCIDLCTs> &alllcts_emul, int Run, int Event, bool addEmulation, bool doGEMCSC, int doDebug){

    gStyle->SetPalette(55);
    const int NPad = 192;
    const int NRoll = 8;
    const int NLayer = 2;

    //if (std::find(usedChamber.begin(), usedChamber.end(), id) == usedChamber.end()) return;

    bool doSimHit = simhit.size() > 0;
    vector<GEMDetID> allids_sh;
    for (const auto& id_simhit : simhit){
        allids_sh.push_back(id_simhit.first);
    }
    vector<GEMDetID> allids_pads;
    for (const auto& id_pad : gempads){
        allids_pads.push_back(id_pad.first);
    }

    srand (time(NULL));
    TString name = "";
    TString legendName = "";

    SetSaveNameLegendNameGEM(name, legendName, address, id, Run, Event);

    TCanvas *c1 = new TCanvas("c1", "c1", 0, 0, 600, 800);

    int npad_canvas = doSimHit ? 3 : 2;
    if(doDebug>2)  cout <<"start to display GEM, total in canvas " << npad_canvas << endl;
    
    c1->Divide(1, npad_canvas);

    c1->SetRightMargin(0.15);
    c1->SetBottomMargin(0.25);
    c1->SetTopMargin(0.25);

//GEM Pad display
    int ipad = 1;
    int nybins = NRoll*(NLayer+1)-1;

    c1->cd(ipad)->SetGridy(); ipad++;
    gPad->SetTopMargin(0.1);
    gPad->SetBottomMargin(0.15);
    TH2F* padDis = new TH2F("padDis", "", NPad, 0, NPad,  nybins, 0, nybins);
    TPaveText *pt1 = new TPaveText(0.4,.92,0.6,0.96, "NDC");

    PadDigiDisplay(id, allids_pads, gempads, padDis, doDebug);
    SetTitle(pt1, "GEM Pads");

    padDis->SetMarkerSize(1.0);
    padDis->Draw("COLZ text");
    pt1->Draw();  

    if (doGEMCSC) {
        string luts_folder = "GifDisplay/GEMCSC/";
        TH2F* extrapLCT = new TH2F("extrapLCT", "", NPad, 0, NPad,  nybins, 0, nybins);
        TH2F* unextrapLCT = new TH2F("unextrapLCT", "", NPad, 0, NPad,  nybins, 0, nybins);
        TPaveText *LCTLegend = new TPaveText(0.75,.7,0.9,0.96, "NDC");

        vector<CorrelatedLCT> lcts;
        if (addEmulation) lcts = findStubsInChamber(CSCid, alllcts_emul);
        else lcts = findStubsInChamber(CSCid, alllcts);

        //if (lcts.size() == 1 && lcts.at(0).quality >= 5) {
        //    cout << "-------" << "station  "<< CSCid.Station <<" endcap "<< CSCid.Endcap <<" chamber "<< CSCid.Chamber << "-----" << endl;
        //}

        LCTPropagationDisplay(id, lcts, extrapLCT, unextrapLCT, luts_folder);

       	TText *t1 = LCTLegend->AddText("Unextrapolated LCT");
        t1->SetTextColor(kBlue); 
        t1->Draw();
        TText *t2 = LCTLegend->AddText("Extrapolated LCT");
        t2->SetTextColor(kRed);
        t2->Draw();

        extrapLCT->Draw("BOX SAME");
        unextrapLCT->Draw("BOX SAME");
        LCTLegend->Draw(); 
    }

    TH2F* simhitDis = new TH2F("padDis", "", NPad, 0, NPad*2, nybins, 0, nybins);
    TPaveText *pt2 = new TPaveText(0.4,.92,0.6,0.96, "NDC");
    if (doSimHit){
        if (doDebug>2) cout <<"dipslay GEM simHits at ipad canvas "<< ipad << endl;
        c1->cd(ipad)->SetGridy(); ipad++;
        gPad->SetTopMargin(0.1);
        gPad->SetBottomMargin(0.15);

        GEMSimHitDisplay(id, allids_sh, simhit, simhitDis, doDebug);
        SetTitle(pt2, "SimHits and PDGID");
        simhitDis->SetMarkerSize(1.0);
        simhitDis->Draw("COLZtext");
        pt2->Draw();
    }


    //show GEMClusters
    if (doDebug>2) cout <<"dipslay GEM Cluster at ipad canvas "<< ipad << " superchamber "<< id << endl;
    c1->cd(ipad)->SetGridy(); ipad++;
    gPad->SetTopMargin(0.0);
    gPad->SetBottomMargin(0.0);
    //chamber, run, event
    stringstream ss;
    ss <<"GEM SuperChamber "<< (id.Endcap == 1 ? "+":"-") << id.Station <<"/"<< id.Ring <<"/"<< id.Chamber <<" ";
    ss <<" run: "<<  Run <<"  event #"<< Event <<" nominalBX=8(<-0)";
    TPaveText * tex1 = new TPaveText(0.1, 0.0, 0.9, 1.0, "NDC");
    //tex1->SetTextFont(42);
    tex1->AddText(ss.str().c_str());
    stringstream ss_clusters[100];
    int iclst = 0;
    for (int l=1; l <=NLayer; l++){
        for (int ieta=1; ieta <= NRoll; ieta++){
          
            GEMDetID thisid;
            thisid.Endcap  = id.Endcap;
            thisid.Station = id.Station;
            thisid.Ring    = id.Ring;
            thisid.Chamber = id.Chamber;
            thisid.Layer = l;
            thisid.Roll = ieta;
            vector<GEMPadCluster> clts = findClustersInChamber(thisid, gemclusters);
            for (unsigned int i = 0; i < clts.size(); i++){
                ss_clusters[iclst] <<"Cluster #"<< iclst <<" Layer "<< l <<" Roll "<< ieta <<" first Pad "<< clts[i].firstPad << " size " << clts[i].size <<" BX "<< clts[i].BX+8;
                tex1->AddText(ss_clusters[iclst].str().c_str());
                iclst++;
            }
        }
    }
    tex1->Draw();
 

    c1->Update();
    c1->SaveAs(name + ".png");
    c1->SaveAs(name + ".pdf");
    //c1->SaveAs(name + ".C");

    delete c1;
    delete padDis;
    delete simhitDis;
    delete tex1;
}





void SetSaveNameLegendNameGEM(TString& name, TString& legendName, TString address, GEMDetID id, int Run, int Event){

    TString  space = "                                   "; 
    if (id.Endcap == 1){
        legendName = "GE+" + NumberToString(id.Station) + "/" + NumberToString(id.Ring) + "/" + NumberToString(id.Chamber) + space+space + "Run #" + NumberToString(Run) + "  " + "Event #" + NumberToString(Event);
        name = address +"/"+ NumberToString(Run) + "_" + NumberToString(Event)+"_GEPlus" + NumberToString(id.Station) + "_" + NumberToString(id.Ring) + "_" + NumberToString(id.Chamber);
        }

    if (id.Endcap == 2){
        legendName = "GE-" + NumberToString(id.Station) + "/" + NumberToString(id.Ring) + "/" + NumberToString(id.Chamber)  + space+space + "Run #" + NumberToString(Run) + "  " + "Event #" + NumberToString(Event);
        name = address +"/" +NumberToString(Run) + "_" + NumberToString(Event)+"_GEMinus" + NumberToString(id.Station) + "_" + NumberToString(id.Ring) + "_" + NumberToString(id.Chamber);

        }


}



void GEMSimHitDisplay(GEMDetID id, vector<GEMDetID>& allIds, vector<GEMSIMHIT>& gemsimHits, TH2F* stripDis, int doDebug){ 

    stripDis->GetXaxis()->SetNdivisions(1010);
    stripDis->GetYaxis()->SetNdivisions(-23);
    stringstream label[24];
    for (int i = 1; i < 3; i++){//in each interesting layer has strip hits
      for (int ieta = 1; ieta <= 8; ieta++){

        GEMDetID tempId;
        tempId.Station = id.Station;
        tempId.Ring    = id.Ring;
        tempId.Layer   = i;
        tempId.Roll    = ieta;
        tempId.Endcap  = id.Endcap;
        tempId.Chamber = id.Chamber;
        int ybin = (8-tempId.Roll)*3+2-tempId.Layer;
        label[ybin] <<"L"<< i <<"Eta"<<ieta;
        stripDis->GetYaxis()->ChangeLabel(ybin+1,-1,-1,-1,-1,-1, label[ybin].str().c_str());
        if (i==1)  
            stripDis->GetYaxis()->ChangeLabel(ybin+2,-1,-1,-1,-1,-1, "--");

        if (doDebug > 2) cout <<"GEMSimhit display id "<< tempId <<" label "<< label[ybin].str()<<" ybin "<< ybin <<endl;
        auto it = find(allIds.begin(), allIds.end(), tempId);
        int index = (it == allIds.end()) ? -1 : distance(allIds.begin (), it);
        if (doDebug > 2)
            cout <<"\t total ids  "<< allIds.size() <<" index in SIMHIT vector " << index << endl;
        if (index < 0) continue;

        vector<SimHit> tempHits = gemsimHits[index].second;
        if (doDebug > 2) cout <<"\t GEM simhits found in chamber, size "<< tempHits.size() << endl;
        MakeOneLayerGEMSimHitDisplay(tempId, tempHits, stripDis);
      }
    }

    SetHistContour(stripDis, 11, 13);

    //stripDis_text->SetMarkerSize(1.5);

    stripDis->GetZaxis()->SetLabelSize(0.1);
    stripDis->GetZaxis()->SetRangeUser(11, 13);
    stripDis->SetStats(0);
    stripDis->GetXaxis()->SetTitle("Strip Number, SimHit");
    stripDis->GetXaxis()->SetLabelSize(0.06);
    stripDis->GetYaxis()->SetLabelSize(0.03);
    stripDis->GetXaxis()->SetTitleSize(0.06);
    stripDis->GetYaxis()->SetTitleSize(0.0);
    stripDis->GetXaxis()->SetTitleOffset(0.81);
    stripDis->GetYaxis()->SetTitleOffset(0.2);
    stripDis->GetYaxis()->CenterLabels();
    stripDis->SetTitle("");
}

void PadDigiDisplay(GEMDetID id, vector<GEMDetID>& allIds, vector<GEMPAD>& gempads, TH2F*padDis, int doDebug){ 
//Roll structure, eta increasing from top to bottom
// top: roll=1, bottom roll=8
//
    padDis->GetXaxis()->SetNdivisions(1010);
    padDis->GetYaxis()->SetNdivisions(-23);
    stringstream label[24];
    for (int i = 1; i < 3; i++){//in each interesting layer has strip hits
      for (int ieta = 1; ieta <= 8; ieta++){

        GEMDetID tempId;
        tempId.Station = id.Station;
        tempId.Ring    = id.Ring;
        tempId.Layer   = i;
        tempId.Roll    = ieta;
        tempId.Endcap  = id.Endcap;
        tempId.Chamber = id.Chamber;
        int ybin = (8-tempId.Roll)*3+2-tempId.Layer;
        label[ybin] <<"L"<< i <<"Eta"<<ieta;
        padDis->GetYaxis()->ChangeLabel(ybin+1,-1,-1,-1,-1,-1, label[ybin].str().c_str());
        if (i==1)  
            padDis->GetYaxis()->ChangeLabel(ybin+2,-1,-1,-1,-1,-1, "--");

        if (doDebug > 2) cout <<"GEMPad display id "<< tempId << " label "<< label[ybin].str() <<" ybin "<< ybin <<endl;
        auto it = find(allIds.begin(), allIds.end(), tempId);
        int index = (it == allIds.end()) ? -1 : distance(allIds.begin (), it);
        if (doDebug > 2)
           cout <<"\t GEPads display, total ids "<< allIds.size() <<" index in GEMPads vector " << index << endl;
        if (index < 0) continue;

        vector<GEMPad> tempPads = gempads[index].second;
        if (doDebug > 2) cout <<"\t GEMpads found in the detector, size " << tempPads.size() << endl;
        MakeOneLayerGEMPadDisplay(tempId, tempPads, padDis);

      }
    }

    SetHistContour(padDis, 0, 16);

    padDis->GetZaxis()->SetLabelSize(0.1);
    padDis->GetZaxis()->SetRangeUser(0,16);
    padDis->SetStats(0);
    padDis->GetXaxis()->SetTitle("GEM Pad Number");
    padDis->GetYaxis()->SetTitle("");
    padDis->GetXaxis()->SetLabelSize(0.06);
    padDis->GetYaxis()->SetLabelSize(0.03);
    padDis->GetXaxis()->SetTitleSize(0.06);
    padDis->GetYaxis()->SetTitleSize(0.0);
    padDis->GetXaxis()->SetTitleOffset(0.81);
    padDis->GetYaxis()->SetTitleOffset(0.2);
    padDis->GetYaxis()->CenterLabels();
    padDis->SetTitle("");
}



void MakeOneLayerGEMPadDisplay(GEMDetID tempId, vector<GEMPad> &pads, TH2F* padDisplay){

    for (int i = 0; i < int(pads.size()); i++){

        double time = pads[i].TimeBin+8;
        if (time==0){time+=0.1;}

        int pad = pads[i].Pad;
        int x1 = padDisplay->GetXaxis()->FindBin(pad);
        int ybin = (8-tempId.Roll)*3+2-tempId.Layer+1;
        padDisplay->SetBinContent(x1 + 1, ybin, time);

    }

}

void MakeOneLayerGEMSimHitDisplay(GEMDetID tempId, vector<SimHit> &shs, TH2F* stripDisplay){
    for (int i = 0; i < int(shs.size()); i++){

        double pdgid = shs[i].PdgId;
        int strip = shs[i].Strip;
        int x1 = stripDisplay->GetXaxis()->FindBin(strip);
        int ybin = (8-tempId.Roll)*3+2-tempId.Layer+1;
        //cout<<"Do one GEM layer display strip " << strip <<" ybin "<< ybin <<" pdgId " << pdgid <<endl;
        stripDisplay->SetBinContent(x1 + 1, ybin, pdgid);

    }
}

vector<GEMPadCluster> findClustersInChamber(GEMDetID id, vector<GEMCLUSTER>& allclusters){
   vector<GEMPadCluster> clusters;
   for (auto idclusters : allclusters){
      // cout <<"findCluster in "<< id <<" from allclusters "<< idclusters.first <<" num of cluster "<< idclusters.second.size() << endl;
       if (idclusters.first == id){
	   clusters = idclusters.second;
	   break;
       }
   }
  
 return clusters;
}


int slopePropagation(int slope, int layer, bool evenChamber, bool ME1a, bool ME1b, string luts_folder) {
    ifstream file; 
    string filename;
    string delimiter = " ";
    string line;
    int propagated_slope = -1; 
 
    if (layer == 1) {
        if (evenChamber) {
            if (ME1a) {
                filename = luts_folder + "SlopeCorrection/FacingChambers/ExtrapolationBySlope_ME11a_even_GEMlayer1.txt";
            }
            else if (ME1b) {
                filename = luts_folder + "SlopeCorrection/FacingChambers/ExtrapolationBySlope_ME11b_even_GEMlayer1.txt";
            }
        }
        else { 
            if (ME1a) {
                filename = luts_folder + "SlopeCorrection/FacingChambers/ExtrapolationBySlope_ME11a_odd_GEMlayer1.txt";
            }
            else if (ME1b) {
                filename = luts_folder + "SlopeCorrection/FacingChambers/ExtrapolationBySlope_ME11b_odd_GEMlayer1.txt";
            }
        }
    }
    else if (layer == 2) {
        if (evenChamber) {
            if (ME1a) {
                filename = luts_folder + "SlopeCorrection/FacingChambers/ExtrapolationBySlope_ME11a_even_GEMlayer2.txt";
            }
            else if (ME1b) {
                filename = luts_folder + "SlopeCorrection/FacingChambers/ExtrapolationBySlope_ME11b_even_GEMlayer2.txt";
            }
        }
        else {
            if (ME1a) {
                filename = luts_folder + "SlopeCorrection/FacingChambers/ExtrapolationBySlope_ME11a_odd_GEMlayer2.txt";
            }
            else if (ME1b) {
                filename = luts_folder + "SlopeCorrection/FacingChambers/ExtrapolationBySlope_ME11b_odd_GEMlayer2.txt";
            }
        }
    }

    file.open(filename);
    if (file.is_open()) {
        while (getline(file, line)) {
            if (line.at(0) == '#') continue;
            if (slope == atoi(line.substr(0, line.find(delimiter)).c_str())) {
                propagated_slope = atoi(line.substr(line.find(delimiter), -1).c_str());  
            }
        }
    }
    return propagated_slope;
}

pair<int, int> EighthStripToPad(int eighthStrip, bool evenChamber, bool ME1a, bool ME1b, string luts_folder) {
    ifstream file;
    string filename;
    string delimiter = " ";
    string line;
    pair<int, int> pad = {-1, -1}; 
    int es = 0;
    int pad_prev = 0; 
        
    if (evenChamber) {
        if (ME1a) {
            filename = luts_folder + "CoordinateConversion/GEMCSCLUT_pad_es_ME1a_even.txt";
        }
        else if (ME1b) {
            filename = luts_folder + "CoordinateConversion/GEMCSCLUT_pad_es_ME1b_even.txt";
        }
    }
    else {
        if (ME1a) {
             filename = luts_folder + "CoordinateConversion/GEMCSCLUT_pad_es_ME1a_odd.txt";
        }
        else if (ME1b) {
            filename = luts_folder + "CoordinateConversion/GEMCSCLUT_pad_es_ME1b_odd.txt";
        }
    }

    file.open(filename);
    if (file.is_open()) {
        while (getline(file, line)) {
            if (line.at(0) == '#') continue;
            if (eighthStrip == atoi(line.substr(line.find(delimiter), -1).c_str())) {
                pad.first = atoi(line.substr(0, line.find(delimiter)).c_str());
                break;
            }
            else if (es != 0 && (eighthStrip - es) * (eighthStrip - atoi(line.substr(line.find(delimiter), -1).c_str())) < 0) {
                pad.first = pad_prev;
                pad.second = atoi(line.substr(0, line.find(delimiter)).c_str());
                break;
            } 
            es = atoi(line.substr(line.find(delimiter), -1).c_str());
            pad_prev = atoi(line.substr(0, line.find(delimiter)).c_str());
        }
    }
    if (pad.second == -1) pad.second = pad.first;
    return pad;               
}


pair<int, int> HalfStripToPad(int halfStrip, bool evenChamber, bool ME1a, bool ME1b, string luts_folder) {
    ifstream file;
    string filename;
    string delimiter = " ";
    string line;
    pair<int, int> pad = {-1, -1};

    if (evenChamber) {
        if (ME1a) {
            filename = luts_folder + "CoordinateConversion/GEMCSCLUT_pad_hs_ME1a_even.txt";
        }
        else if (ME1b) {
            filename = luts_folder + "CoordinateConversion/GEMCSCLUT_pad_hs_ME1b_even.txt";
        }
    }
    else {
        if (ME1a) {
             filename = luts_folder + "CoordinateConversion/GEMCSCLUT_pad_hs_ME1a_odd.txt";
        }
        else if (ME1b) {
            filename = luts_folder + "CoordinateConversion/GEMCSCLUT_pad_hs_ME1b_odd.txt";
        }
    }

    file.open(filename);
    if (file.is_open()) {
        while (getline(file, line)) {
            if (line.at(0) == '#') continue;
            if (halfStrip == atoi(line.substr(line.find(delimiter), -1).c_str())) {
                if (pad.first == -1) {
                    pad.first = atoi(line.substr(0, line.find(delimiter)).c_str()); 
                }
                else {
                    pad.second = atoi(line.substr(0, line.find(delimiter)).c_str());
                }
            }
        }
    }
    if (pad.second == -1) pad.second = pad.first; 
    return pad;
}


int WireToRoll(int wireGroup, int layer, bool evenChamber, string luts_folder) {
    ifstream file;
    string filename_max;
    string filename_min; 
    string delimiter = " ";
    string line;
    vector<int> WG_max;
    vector<int> WG_min;
    int roll = -1; 
 
    if (evenChamber) {
        if (layer == 1) {
            filename_max = luts_folder + "CoordinateConversion/GEMCSCLUT_roll_l1_max_wg_ME11_even.txt";
            filename_min = luts_folder + "CoordinateConversion/GEMCSCLUT_roll_l1_min_wg_ME11_even.txt";
        }
        else if (layer == 2) {
            filename_max = luts_folder + "CoordinateConversion/GEMCSCLUT_roll_l2_max_wg_ME11_even.txt";
            filename_min = luts_folder + "CoordinateConversion/GEMCSCLUT_roll_l2_min_wg_ME11_even.txt";
        }
    }
    else {
        if (layer == 1) {
            filename_max = luts_folder + "CoordinateConversion/GEMCSCLUT_roll_l1_max_wg_ME11_odd.txt";
            filename_min = luts_folder + "CoordinateConversion/GEMCSCLUT_roll_l1_min_wg_ME11_odd.txt";
        }
        else if (layer == 2) {
            filename_max = luts_folder + "CoordinateConversion/GEMCSCLUT_roll_l2_max_wg_ME11_odd.txt";
            filename_min = luts_folder + "CoordinateConversion/GEMCSCLUT_roll_l2_min_wg_ME11_odd.txt";
        }
    }
    
    file.open(filename_max);
    if (file.is_open()) {
        while (getline(file, line)) {
            if (line.at(0) == '#') continue;
            WG_max.push_back(atoi(line.substr(line.find(delimiter), -1).c_str()));
        }   
    }
    file.close();
    file.open(filename_min);
    if (file.is_open()) {
        while (getline(file, line)) {
            if (line.at(0) == '#') continue;
            WG_min.push_back(atoi(line.substr(line.find(delimiter), -1).c_str()));
        }
    } 

 
    for (int i = 0; i < 8; i++) {
        if (wireGroup <= WG_max.at(i) && wireGroup >= WG_min.at(i)) {
            roll = i + 1;
            break; 
        }
    }
    return roll;   
}

void LCTPropagationDisplay(GEMDetID id, vector<CorrelatedLCT> &lcts, TH2F* extrapLCT, TH2F* unextrapLCT, string luts_folder) {
    extrapLCT->GetXaxis()->SetNdivisions(1010);
    extrapLCT->GetYaxis()->SetNdivisions(-23);
    unextrapLCT->GetXaxis()->SetNdivisions(1010);
    unextrapLCT->GetYaxis()->SetNdivisions(-23);
    for (CorrelatedLCT lct : lcts){
        int eighthStrip = lct.eighthStrip;
        int wireGroup = lct.keyWG;
        int slope = lct.slope;
        int bend = lct.bend;
        bool evenChamber = ((int)id.Chamber % 2 == 0);
        bool ME1a = (eighthStrip > 512) ? 1 : 0;
        bool ME1b = (eighthStrip < 512) ? 1 : 0;
    
        for (int layer = 1; layer <= 2; layer++) {
            int slope_propagation = slopePropagation(slope, layer, evenChamber, ME1a, ME1b, luts_folder); // Extrapolated slope
            int eighthStrip_extrap = eighthStrip - slope_propagation*((bend*2) - 1); // Extrapolated eighthstrip position

            pair<int, int> x_unextrap = EighthStripToPad(eighthStrip, evenChamber, ME1a, ME1b, luts_folder);
            //pair<int, int> x_extrap = EighthStripToPad(eighthStrip_extrap, evenChamber, ME1a, ME1b, luts_folder);
 
            int roll = WireToRoll(wireGroup, layer, evenChamber, luts_folder); // Eta partition
            int ybin = (8-roll)*3+2-layer+1;

            int match_window;
            (evenChamber) ? match_window = 20 : match_window = 40; 
            // Extrapolated matching window
            pair<int, int> window_high = EighthStripToPad(eighthStrip_extrap + match_window, evenChamber, ME1a, ME1b, luts_folder); 
            pair<int, int> window_low = EighthStripToPad(eighthStrip_extrap - match_window, evenChamber, ME1a, ME1b, luts_folder);

            int x_min, x_max; 
            if (evenChamber) {
                x_min = min(window_low.first, window_low.second);
                x_max =  max(window_high.first, window_high.second);
            }
            else {
                x_min = min(window_high.first, window_high.second);
                x_max = max(window_low.first, window_low.second);
            }
            for (int x = x_min; x <= x_max; x++) {
                extrapLCT->SetBinContent(x + 1, ybin - 3, 3);
                extrapLCT->SetBinContent(x + 1, ybin, 3);
                extrapLCT->SetBinContent(x + 1, ybin + 3, 3);
            }
            unextrapLCT->SetBinContent(x_unextrap.first + 1, ybin, 15);
            unextrapLCT->SetBinContent(x_unextrap.second + 1, ybin, 15);
        }
    }
    extrapLCT->SetMarkerSize(2);
    extrapLCT->SetFillStyle(3144);
    extrapLCT->SetFillColorAlpha(kRed, 0.5);
    extrapLCT->SetLineWidth(2);
    unextrapLCT->SetMarkerSize(2);
    unextrapLCT->SetFillStyle(3144);
    unextrapLCT->SetFillColorAlpha(kBlue, 0.5);
    unextrapLCT->SetLineWidth(2);
}

