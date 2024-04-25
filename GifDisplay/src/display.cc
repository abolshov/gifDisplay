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

vector<CorrelatedLCT> findStubsInChamber(CSCDetID id, vector<CSCIDLCTs> alllcts){
   vector<CorrelatedLCT> lcts;
   for (auto idlcts : alllcts){
       if (idlcts.first == id){
	   lcts = idlcts.second;
	   break;
       }
   }
  
 return lcts;
}


void WireStripDisplay(TString address, CSCDetID id, vector<SIMHIT> &simhit, vector<WIRE> &wire, vector<STRIP> &strip, vector<COMPARATOR> &comparator, 
			 vector<CSCIDLCTs> &allalcts, vector<CSCIDLCTs> &allalcts_emul, 
			 vector<CSCIDLCTs> &allclcts, vector<CSCIDLCTs> &allclcts_emul, 
			 vector<CSCIDLCTs> &alllcts, vector<CSCIDLCTs> &alllcts_emul, 
			 vector<CSCDetID> &usedChamber, int Run, int Event, bool addEmulation, int doDebug){

        gStyle->SetPalette(55);

	
        TH2F* NWireGroup = new TH2F("NWireGroup", "NWireGroup", 4, 1, 5, 4, 1, 5);
        TH2F* NStrip = new TH2F("NStrip", "NStrip", 4, 1, 5, 4, 1, 5);

        NWireGroup->SetBinContent(1, 1, 48);
        NWireGroup->SetBinContent(1, 2, 48);
        NWireGroup->SetBinContent(1, 3, 48);
        NWireGroup->SetBinContent(1, 4, 48);
        NWireGroup->SetBinContent(2, 1, 112);
        NWireGroup->SetBinContent(3, 1, 96);
        NWireGroup->SetBinContent(4, 1, 96);
        NWireGroup->SetBinContent(2, 2, 64);
        NWireGroup->SetBinContent(3, 2, 64);
        NWireGroup->SetBinContent(4, 2, 64);

        NStrip->SetBinContent(1, 1, 112);
        NStrip->SetBinContent(1, 2, 80);
        NStrip->SetBinContent(1, 3, 64);
        NStrip->SetBinContent(1, 4, 112);
        NStrip->SetBinContent(2, 1, 80);
        NStrip->SetBinContent(3, 1, 80);
        NStrip->SetBinContent(4, 1, 80);
        NStrip->SetBinContent(2, 2, 80);
        NStrip->SetBinContent(3, 2, 80);
        NStrip->SetBinContent(4, 2, 80);

        if (ChamberUsedForEventDisplay(id, usedChamber)) return;//this chamber has not been used for eventdisplay

        bool doSimHit = simhit.size() > 0;

           srand (time(NULL));
           TString name = "";
           TString legendName = "";

           SetSaveNameLegendName(name, legendName, address, id, Run, Event);

           vector<int> layer_simhit = FindChamberIndex(id, simhit);
           vector<int> layer_strip = FindChamberIndex(id, strip);
           vector<int> layer_wire = FindChamberIndex(id, wire);
           vector<int> layer_comparator = FindChamberIndex(id, comparator);

           if (id.Ring==1 and id.Station==1) {

              CSCDetID idr4 = id; idr4.Ring = 4;
              vector<int> layer_wire_r4 = FindChamberIndex(idr4, wire);
              vector<int> layer_strip_r4 = FindChamberIndex(idr4, strip);
              vector<int> layer_simhit_r4 = FindChamberIndex(idr4, simhit);
              vector<int> layer_comparator_r4 = FindChamberIndex(idr4, comparator);

              layer_wire.insert(layer_wire.end(),layer_wire_r4.begin(),layer_wire_r4.end());
              layer_strip.insert(layer_strip.end(),layer_strip_r4.begin(),layer_strip_r4.end());
              layer_simhit.insert(layer_simhit.end(),layer_simhit_r4.begin(),layer_simhit_r4.end());
              layer_comparator.insert(layer_comparator.end(),layer_comparator_r4.begin(),layer_comparator_r4.end());
              }
	   

           const int nStrip = NStrip->GetBinContent(id.Station, id.Ring);
           const int nWireGroup = NWireGroup->GetBinContent(id.Station, id.Ring);
           const int nCFEB = nStrip/16;
//draw event display
           TCanvas *c1 = new TCanvas("c1", "c1", 0, 0, 600, 800);

           c1->Divide(1,4);

           c1->SetRightMargin(0.15);
           c1->SetBottomMargin(0.25);
           c1->SetTopMargin(0.25);
//legend
//ignore here
          //SetEventDisplayLegend(legendName);

//strip display
           c1->cd(3)->SetGridy();
           gPad->SetBottomMargin(0.2);
           TH2F* stripDis = new TH2F("stripDis", "", nStrip*2+2, 1, nStrip+2, 6, 1, 7);
           TH2F* stripDis_text = new TH2F("stripDis_text", "", nStrip*2+2, 1, nStrip+2, 6, 0.5, 6.5);
           TH1F* cfebNotReadOut = new TH1F("cfebNotReadOut", "", nStrip+1, 1, nStrip+2);
           TPaveText *pt3 = new TPaveText(0.4,0.90,0.6,0.96,"NDC");
           if (doSimHit){
               SimHitDisplay(/*c1,*/ id, layer_simhit,simhit, stripDis, stripDis_text);
               SetTitle(pt3, "SimHits and PDGID");

               stripDis->Draw("COLZtext");
           }else{
               TH1F* cfebNotInstall_me21 = new TH1F("cfebNotInstall_me21", "", 81, 1, 82);
               TH1F* cfebNotInstall_me11 = new TH1F("cfebNotInstall_me11", "", 81, 1, 82);

               TH1F* cfebNotReadOut_comparator = new TH1F("cfebNotReadOut_comparator", "", nStrip*2+2, 1, nStrip*2+3);
               TH1F* cfebNotInstall_comparator_me21 = new TH1F("cfebNotInstall_comparator_me21", "", 162, 1, 163);
               TH1F* cfebNotInstall_comparator_me11 = new TH1F("cfebNotInstall_comparator_me11", "", 162, 1, 163);

    //           double cfeb[5] = {0, 0, 0, 0, 0};
               double cfeb[nCFEB] = {}; 
               StripDisplay(/*c1,*/ id, layer_strip, strip, cfeb, stripDis, stripDis_text, cfebNotReadOut, cfebNotInstall_me21, cfebNotInstall_me11);
               MakeShadeForComparatorPanel(cfebNotReadOut, cfebNotInstall_me21, cfebNotReadOut_comparator, cfebNotInstall_comparator_me21, cfebNotInstall_comparator_me11);
               SetTitle(pt3, "Strip Hits and ADC Count");
               stripDis->Draw("COLZ");
               stripDis_text->Draw("text same");
               cfebNotReadOut->Draw("B same");
               if (id.Station == 2) {
    //              cfebNotInstall_me21->Draw("B same");
                  } else if (id.Station == 1) {
    //                        cfebNotInstall_me11->Draw("B same");
                            }
           }
           pt3->Draw();


	  //show LCTs
	   c1->cd(4);
	  gPad->SetTopMargin(0.0);
	  gPad->SetBottomMargin(0.0);
	   vector<CorrelatedLCT> alcts = findStubsInChamber(id, allalcts);
	   vector<CorrelatedLCT> clcts = findStubsInChamber(id, allclcts);
	   vector<CorrelatedLCT> lcts = findStubsInChamber(id, alllcts);
 	  //chamber, run, event
	  stringstream ss;
	  ss <<"Chamber "<< (id.Endcap == 1 ? "+":"-") << id.Station <<"/"<< id.Ring <<"/"<< id.Chamber <<" ";
	  ss <<" run: "<<  Run <<"  event #"<< Event;
	  TPaveText * tex1 = new TPaveText(0.1, 0.0, 0.9, 1.0, "NDC");
	  //tex1->SetTextFont(42);
	  tex1->AddText(ss.str().c_str());
	  stringstream ss_alcts[10];
	  for (unsigned int ilct = 0; ilct < alcts.size(); ilct++){
	      ss_alcts[ilct] <<"Run2 #"<<ilct<<" ALCT keyWg "<< alcts[ilct].keyWG <<" Quality "<< alcts[ilct].quality <<" Pattern "<< alcts[ilct].pattern<<" bx "<<  alcts[ilct].BX;
	      tex1->AddText(ss_alcts[ilct].str().c_str());
	  }
	  stringstream ss_clcts[10];
	  for (unsigned int ilct = 0; ilct < clcts.size(); ilct++){
	      ss_clcts[ilct] <<"Run2 #"<<ilct<<" CLCT HS "<< clcts[ilct].keyStrip <<" Quality "<< clcts[ilct].quality <<" Pattern "<< clcts[ilct].pattern << " Run3_Pattern " << clcts[ilct].run3_pattern << " bx "<<  clcts[ilct].BX;
	      tex1->AddText(ss_clcts[ilct].str().c_str());
	  }
	  stringstream ss_lcts[10];
	  for (unsigned int ilct = 0; ilct < lcts.size(); ilct++){
	      ss_lcts[ilct] <<"Run2 #"<<ilct<<" LCT keyWg "<< lcts[ilct].keyWG <<" HS "<< lcts[ilct].keyStrip <<" Quality "<< lcts[ilct].quality <<" Pattern "<< lcts[ilct].pattern << " bx "<<  lcts[ilct].BX;
	      tex1->AddText(ss_lcts[ilct].str().c_str());
	  }
          vector<CorrelatedLCT> alcts_emul;
          vector<CorrelatedLCT> clcts_emul;
          vector<CorrelatedLCT> lcts_emul;
	  if (addEmulation){
             alcts_emul = findStubsInChamber(id, allalcts_emul);
	     clcts_emul = findStubsInChamber(id, allclcts_emul);
	     lcts_emul = findStubsInChamber(id, alllcts_emul);
	     stringstream ss_alcts_emul[10];
	     for (unsigned int ilct = 0; ilct < alcts_emul.size(); ilct++){
	         ss_alcts_emul[ilct] <<"Run3 #"<<ilct<<" ALCT keyWg "<< alcts_emul[ilct].keyWG <<" Quality "<< alcts_emul[ilct].quality <<" Pattern "<< alcts_emul[ilct].pattern<<" bx "<<  alcts_emul[ilct].BX;
	         tex1->AddText(ss_alcts_emul[ilct].str().c_str());
	     }
	     stringstream ss_clcts_emul[10];
	     for (unsigned int ilct = 0; ilct < clcts_emul.size(); ilct++){
	         ss_clcts_emul[ilct] <<"Run3 #"<<ilct<<" CLCT HS "<< clcts_emul[ilct].keyStrip <<" Quality "<< clcts_emul[ilct].quality <<" Pattern "<< clcts_emul[ilct].pattern << " Run3_Pattern " << clcts_emul[ilct].run3_pattern << " bx "<< clcts_emul[ilct].BX;
	         tex1->AddText(ss_clcts_emul[ilct].str().c_str());
	     }
	     stringstream ss_lcts_emul[10];
	     for (unsigned int ilct = 0; ilct < lcts_emul.size(); ilct++){
	         ss_lcts_emul[ilct] <<"Run3 #"<<ilct<<" LCT keyWg "<< lcts_emul[ilct].keyWG <<" HS "<< lcts_emul[ilct].keyStrip <<" Quality "<< lcts_emul[ilct].quality <<" Pattern "<< lcts_emul[ilct].pattern << " bx "<<  lcts_emul[ilct].BX;
	         tex1->AddText(ss_lcts_emul[ilct].str().c_str());
	     }
	  }
        /*
	  if (lcts.size() >= 1){
	      stringstream ss_lct1;
	      ss_lct1 <<"#1 LCT keyWg "<< lcts[0].keyWG <<" HS "<< lcts[0].keyStrip <<" Quality "<< lcts[0].quality <<" Pattern "<< lcts[0].pattern;
	      tex1->AddText(ss_lct1.str().c_str());

	  }
	  if (lcts.size() >= 2){
	      stringstream ss_lct2;
	      ss_lct2 <<"#2 LCT keyWg "<< lcts[1].keyWG <<" HS "<< lcts[1].keyStrip <<" Quality "<< lcts[1].quality<<" Pattern "<< lcts[1].pattern;
	      tex1->AddText(ss_lct2.str().c_str());
	  }
	  */
	  tex1->Draw();

//strip hit display
/*          c1->cd(4)->SetGridy();
          gPad->SetBottomMargin(0.15);
          TGraph* stripHitDis;
          TGraph* comparatorHitNotReadOut;
          vector<vector<double> > sHit_cHit_layer = StripHitDisplay(id, layer_strip, layer_comparator, strip, comparator, cfeb);
          double* sHit = &(sHit_cHit_layer[0][0]);
          double* sHitLayer = &(sHit_cHit_layer[1][0]);
          double* cHit = &(sHit_cHit_layer[2][0]);
          double* cHitLayer = &(sHit_cHit_layer[3][0]);

          if (int(sHit_cHit_layer[0].size()) > 0){
          stripHitDis = new TGraph(int(sHit_cHit_layer[0].size()), sHit, sHitLayer);
          TPaveText *pt2 = new TPaveText(0.4,.95,0.6,0.99, "NDC");
          SetTitle(pt2, "Strip Hit");
          stripHitDis->Draw("ap");
          cfebNotInstall->Draw("B same");
          pt2->Draw();
          SetPlotDetail_StripHit(stripHitDis);

          }

          if (int(sHit_cHit_layer[3].size() > 0)){
          comparatorHitNotReadOut = new TGraph(int(sHit_cHit_layer[2].size()), cHit, cHitLayer);
          comparatorHitNotReadOut->Draw("p same");
          comparatorHitNotReadOut->SetMarkerStyle(24);
          comparatorHitNotReadOut->SetMarkerColor(2);
          }
*/

          //wire display
          c1->cd(1)->SetGridy();
          gPad->SetTopMargin(0.15);
          gPad->SetBottomMargin(0.2);
          TH2F* wireDis = new TH2F("wireDis", "", nWireGroup+1, 0, nWireGroup+1, 6, 1, 7);
          TH2F* wireDis_text = new TH2F("wireDis_text", "", nWireGroup, 0, nWireGroup+1, 6, 1, 7);
          TH2F* alctDis = new TH2F("alctDis", "", nWireGroup+1, 0, nWireGroup+1, 6, 1, 7);
          TPaveText *pt1 = new TPaveText(0.4,.90,0.6,0.96, "NDC");

          WireDisplay(id, layer_wire, wire, wireDis, wireDis_text, alctDis, alcts);
          SetTitle(pt1, "Anode Hits");

          wireDis->SetMarkerSize(2);
          wireDis->Draw("COLZ text");
          wireDis_text->Draw("text same");
          alctDis->SetMarkerSize(2);
          alctDis->SetFillStyle(3244);
          alctDis->SetFillColorAlpha(kBlack, 0.5);
          alctDis->SetLineWidth(2);
          alctDis->Draw("BOX SAME");
          pt1->Draw();

          //comparator display
          c1->cd(2)->SetGridy();
              gPad->SetBottomMargin(0.2);
          TPaveText *pt4 = new TPaveText(0.4,.90,0.6,0.96, "NDC");
          TH2F* comparatorDis = new TH2F("comparatorDis", "", nStrip*2+2, 0, nStrip*2+2, 6, 1, 7);
          TH2F* comparatorDis_text = new TH2F("comparatorDis_text", "", nStrip*2+2, 0, nStrip*2+2, 6, 1, 7);
          TH2F* clctDis = new TH2F("clctDis", "", nStrip*2+2, 0, nStrip*2+2, 6, 1, 7);
          if (addEmulation) 
              ComparatorDisplay(id, layer_comparator, comparator, comparatorDis, comparatorDis_text, clctDis, clcts_emul);
          else 
              ComparatorDisplay(id, layer_comparator, comparator, comparatorDis, comparatorDis_text, clctDis, clcts);
          comparatorDis->SetMarkerSize(2);
          comparatorDis->Draw("COLZtext");
          clctDis->SetMarkerSize(2);
          clctDis->SetFillStyle(3244);
          clctDis->SetFillColorAlpha(kBlack, 0.5);
          clctDis->SetLineWidth(2);
          clctDis->Draw("BOX SAME");

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
          c1->SaveAs(name + ".png");
          c1->SaveAs(name + ".pdf");
          // c1->SaveAs(name + ".C");

          delete c1;
          delete wireDis;
          delete stripDis;
          delete wireDis_text;
          delete stripDis_text;
          delete comparatorDis;
          delete comparatorDis_text;
          delete clctDis;
          delete alctDis;
          delete cfebNotReadOut;
          delete  tex1;
}




bool ChamberUsedForEventDisplay(CSCDetID id, vector<CSCDetID> usedChamber){

        bool flag = false;


        for (int i = 0; i < int(usedChamber.size()); i++){

            CSCDetID tempID = usedChamber[i];

            if (id.Endcap == tempID.Endcap &&
                id.Station == tempID.Endcap &&
                id.Ring == tempID.Ring &&
                id.Chamber == tempID.Chamber){

                flag = true;

                }

            }

        return flag;
}



void SetSaveNameLegendName(TString& name, TString& legendName, TString address, CSCDetID id, int Run, int Event){

           //int Run = 1;
           //int Event = 1;
 TString  space = "                                   "; 
           if (id.Endcap == 1){
              legendName = "ME+" + NumberToString(id.Station) + "/" + NumberToString(id.Ring) + "/" + NumberToString(id.Chamber) + space+space + "Run #" + NumberToString(Run) + "  " + "Event #" + NumberToString(Event);

              name = address +"/"+ NumberToString(Run) + "_" + NumberToString(Event)+"_MEPlus" + NumberToString(id.Station) + "_" + NumberToString(id.Ring) + "_" + NumberToString(id.Chamber);
              }

           if (id.Endcap == 2){
              legendName = "ME-" + NumberToString(id.Station) + "/" + NumberToString(id.Ring) + "/" + NumberToString(id.Chamber)  + space+space + "Run #" + NumberToString(Run) + "  " + "Event #" + NumberToString(Event);

              name = address +"/"+ NumberToString(Run) + "_" + NumberToString(Event)+"_MEMinus" + NumberToString(id.Station) + "_" + NumberToString(id.Ring) + "_" + NumberToString(id.Chamber);

              }


}


void SaveUsedChamber(CSCDetID id, vector<int> layer_strip, vector<int> layer_wire, vector<int> layer_comparator, vector<CSCDetID> &usedChamber){

           CSCDetID tempID;

           if (int(layer_strip.size()) > 0 && int(layer_wire.size()) > 0 && int(layer_comparator.size()) > 0){

              tempID.Endcap = id.Endcap;
              tempID.Station = id.Station;
              tempID.Ring = id.Ring;
              tempID.Chamber = id.Chamber;

              usedChamber.push_back(tempID);

              }


}




void SimHitDisplay(/*TCanvas* c1,*/ CSCDetID id, vector<int>& layer_simhit, vector<SIMHIT>& simhit, TH2F* stripDis, TH2F* stripDis_text){ 

//         c1->cd(4)->SetGridy();         

/*           TH2F* stripDis = new TH2F("stripDis", "", 162, 1, 82, 6, 1, 7);
           TH2F* stripDis_text = new TH2F("stripDis_text", "", 162, 1, 82, 6, 1, 7);
           TH1F* cfebNotReadOut = new TH1F("cfebNotReadOut", "", 81, 1, 82);
*/
           for (int i = 0; i < int(layer_simhit.size()); i++){//in each interesting layer has strip hits

               int tempStation = simhit[layer_simhit[i]].first.Station;
               int tempRing    = simhit[layer_simhit[i]].first.Ring;
               int tempLayer   = simhit[layer_simhit[i]].first.Layer;
               vector<SimHit> tempSimHit = simhit[layer_simhit[i]].second;
               //std::cout <<"display CSC simhits in "<< simhit[layer_simhit[i]].first <<" size of simhits "<< tempSimHit.size() << std::endl;


               //CountCFEB(cfeb, tempsimHit);
               //int option1 = 1;
               int option2 = 2;

               bool doStagger = false;
               if (!(tempStation == 1 &&(tempRing==1 || tempRing==4))) doStagger = true;

               MakeOneLayerSimHitDisplay(tempLayer, tempSimHit, stripDis, option2, doStagger);

               if (tempLayer == id.Layer){//chamber level??

                  MakeOneLayerSimHitDisplay(tempLayer, tempSimHit, stripDis_text, option2, doStagger);

                  }
               }

          SetHistContour(stripDis, 11, 22);

          stripDis_text->SetMarkerSize(1.5);

          stripDis->GetZaxis()->SetLabelSize(0.1);
          stripDis->GetZaxis()->SetRangeUser(11, 22);
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
}


void MakeOneLayerSimHitDisplay(int layer, vector<SimHit> &s, TH2F* stripDisplay, int option, bool doStagger){

      if (option == 1){

        for (int i = 0; i < int(s.size()); i++){

           int x1 = stripDisplay->GetXaxis()->FindBin(s[i].Stripf);
           int x2 = x1+1;
           int x3 = x1+2;

           if(doStagger && (layer == 1 || layer == 3 || layer == 5)){

              stripDisplay->SetBinContent(x2, layer, s[i].PdgId);
              stripDisplay->SetBinContent(x3, layer, s[i].PdgId);

             }else {// if(layer == 2 || layer == 4 || layer ==6){

                      stripDisplay->SetBinContent(x1, layer, s[i].PdgId);
                      stripDisplay->SetBinContent(x2, layer, s[i].PdgId);

                      }

            }

        }else if(option == 2){

                          for (int i = 0; i < int(s.size()); i++){

                            int x1 = 2*(s[i].Strip-1) + 1;
                            int x2 = 2*(s[i].Strip-1) + 2;

                            if (doStagger && (layer == 1 || layer == 3 || layer ==5) ){

                               stripDisplay->SetBinContent(x2, layer, s[i].PdgId);

                               }else {//if(layer == 2 || layer == 4 || layer == 6){

                                        stripDisplay->SetBinContent(x1, layer, s[i].PdgId);
                                                                                                                                                       
                                        }
                              }

                          }


}




void StripDisplay(/*TCanvas* c1,*/ CSCDetID id, vector<int>& layer_strip, vector<STRIP>& strip, double cfeb[], TH2F* stripDis, TH2F* stripDis_text, TH1F* cfebNotReadOut, TH1F* cfebNotInstall_me21, TH1F* cfebNotInstall_me11){ 

//         c1->cd(4)->SetGridy();         

/*           TH2F* stripDis = new TH2F("stripDis", "", 162, 1, 82, 6, 1, 7);
           TH2F* stripDis_text = new TH2F("stripDis_text", "", 162, 1, 82, 6, 1, 7);
           TH1F* cfebNotReadOut = new TH1F("cfebNotReadOut", "", 81, 1, 82);
*/
           for (int i = 0; i < int(layer_strip.size()); i++){//in each interesting layer has strip hits

               int tempStation = strip[layer_strip[i]].first.Station;
               int tempRing = strip[layer_strip[i]].first.Ring;
               int tempLayer = strip[layer_strip[i]].first.Layer;
               vector<Strips> tempStrip = strip[layer_strip[i]].second;


               CountCFEB(cfeb, tempStrip);
               int option1 = 1;
               int option2 = 2;

               bool doStagger = false;
               if (!(tempStation == 1 &&(tempRing==1 || tempRing==4))) doStagger = true;

               MakeOneLayerStripDisplay(tempLayer, tempStrip, stripDis, option1, doStagger);

               if (tempLayer == id.Layer){

                  MakeOneLayerStripDisplay(tempLayer, tempStrip, stripDis_text, option2, doStagger);

                  }
               }

//          BlockUnreadCFEB(cfeb, cfebNotReadOut);
          BlockUnreadCFEB(cfeb, cfebNotReadOut, (cfebNotReadOut->GetNbinsX())/16);
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


void MakeOneLayerStripDisplay(int layer, vector<Strips> &s, TH2F* stripDisplay, int option, bool doStagger){

      if (option == 1){

        for (int i = 0; i < int(s.size()); i++){

           int x1 = 2*(s[i].Strip-1) + 1;
           int x2 = 2*(s[i].Strip-1) + 2;
           int x3 = 2*(s[i].Strip-1) + 3;

           if(doStagger && (layer == 1 || layer == 3 || layer == 5)){

              stripDisplay->SetBinContent(x2, layer, s[i].MaxADC);
              stripDisplay->SetBinContent(x3, layer, s[i].MaxADC);

             }else {// if(layer == 2 || layer == 4 || layer ==6){

                      stripDisplay->SetBinContent(x1, layer, s[i].MaxADC);
                      stripDisplay->SetBinContent(x2, layer, s[i].MaxADC);

                      }

            }

        }else if(option == 2){

                          for (int i = 0; i < int(s.size()); i++){

                            int x1 = 2*(s[i].Strip-1) + 1;
                            int x2 = 2*(s[i].Strip-1) + 2;

                            if (doStagger && (layer == 1 || layer == 3 || layer ==5) ){

                               stripDisplay->SetBinContent(x2, layer, s[i].MaxADC);

                               }else {//if(layer == 2 || layer == 4 || layer == 6){

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
            }
            else if (layer == 2) {
                alctDis->SetBinContent(position + 1, layer, 15);
                alctDis->SetBinContent(position - 1, layer, 15);
            }
            else if (layer == 5) {
                alctDis->SetBinContent(position + 1, layer, 15);
                alctDis->SetBinContent(position - 1, layer, 15);
            }
            else if (layer == 6) {
                alctDis->SetBinContent(position + 1, layer, 15);
                alctDis->SetBinContent(position + 2, layer, 15);
                alctDis->SetBinContent(position - 1, layer, 15);
                alctDis->SetBinContent(position - 2, layer, 15);
            }
            alctDis->SetBinContent(position, layer, 15);
       } 
}

void MakeOneLayerWireDisplay(int layer, vector<Wire> &w, TH2F* wireDisplay){

        for (int i = 0; i < int(w.size()); i++){

            double time = w[i].TimeBin;

            if (w[i].TimeBin == 0){time+=0.1;}

            wireDisplay->SetBinContent(w[i].WireGroup, layer, time);

        }
}

void MakeOneLayerCLCT(int layer, TH2F* clctDis, vector<CorrelatedLCT>& clcts) {
        for (CorrelatedLCT track : clcts) {
            int position = track.keyStrip + 1;
            if (track.run3_pattern == 0) {
                if (layer == 1) {
                    position += 3;
                }
                else if (layer == 2) {
                    position += 2;
                }
                else if (layer == 4) {
                    position -= 2;
                }
                else if (layer == 5) {
                    position -= 4;
                }
                else if (layer == 6) {
                    position -= 5;
                }
                clctDis->SetBinContent(position, layer, 15);
                clctDis->SetBinContent(position + 1, layer, 15);
                clctDis->SetBinContent(position + 2, layer, 15);
            }
            else if (track.run3_pattern == 1) {
                if (layer == 1) {
                    position -= 3;
                }
                else if (layer == 2) {
                    position -= 2;
                }
                else if (layer == 4) {
                    position += 2;
                }
                else if (layer == 5) {
                    position += 4;
                }
                else if (layer == 6) {
                    position += 5;
                }
                clctDis->SetBinContent(position, layer, 15);
                clctDis->SetBinContent(position - 1, layer, 15);
                clctDis->SetBinContent(position - 2, layer, 15);
            }
            else if (track.run3_pattern == 2) {
                if (layer == 1) {
                    position += 2;
                }
                else if (layer == 2) {
                    position += 1;
                }
                else if (layer == 5) {
                    position -= 1;
                }
                else if (layer == 6) {
                    position -= 2;
                }
                clctDis->SetBinContent(position, layer, 15);
                clctDis->SetBinContent(position - 1, layer, 15);
                clctDis->SetBinContent(position + 1, layer, 15);
            }
            else if (track.run3_pattern == 3) {
                if (layer == 1) {
                    position -= 2;
                }
                else if (layer == 2) {
                    position -= 1;
                }
                else if (layer == 5) {
                    position += 1;
                }
                else if (layer == 6) {
                    position += 2;
                }
                clctDis->SetBinContent(position, layer, 15);
                clctDis->SetBinContent(position - 1, layer, 15);
                clctDis->SetBinContent(position + 1, layer, 15);
            }
            else {
                clctDis->SetBinContent(position, layer, 15);
                clctDis->SetBinContent(position - 1, layer, 15);
                clctDis->SetBinContent(position + 1, layer, 15);
            }
        }
                
} 

void MakeOneLayerComparatorDisplay(int layer, vector<Comparator> &c, TH2F* comparatorDisplay, bool doStagger){

        for (int i = 0; i < int(c.size()); i++){

            double time = c[i].TimeBin;
            if (time==0){time+=0.1;}

            int comparator = 2*(c[i].Strip-1)+c[i].ComparatorNumber%2 + 1;

            if (doStagger && (layer == 1 || layer == 3 || layer ==5)){

               comparatorDisplay->SetBinContent(comparator + 1, layer, time);

               }else { //if(layer == 2 || layer == 4 || layer ==6){

                        comparatorDisplay->SetBinContent(comparator, layer, time);

                        }
            }
}


vector<vector<double> > StripHitDisplay(CSCDetID id, vector<int>& layer_strip, vector<int>& layer_comparator, vector<STRIP> strip, vector<COMPARATOR> &comparator, double cfeb[]){

        vector<vector<double> > sHit_cHit_layer;

        vector<double> stripHitsContainer;
        vector<double> stripHitsLayerContainer;
        vector<double> comparatorHitsContainer;
        vector<double> comparatorHitsLayerContainer;

        for (int i = 0; i < int(layer_strip.size()); i++){

            double tempLayer = strip[layer_strip[i]].first.Layer;
            vector<Strips> tempStrip = strip[layer_strip[i]].second;

            vector<double> stripHits = MakeStripHit(tempStrip);
            vector<double> stripHitsLayer (int(stripHits.size()), tempLayer+0.5);//make a vector containing strip hits' layer, since using tgraph, make it 1.5, 2.5...
            ShiftStripHits(stripHits, tempLayer);

/*      for (i=0; i<stripHits.size(); i++){

}
*/
            stripHitsContainer.insert(stripHitsContainer.begin(), stripHits.begin(), stripHits.end());
            stripHitsLayerContainer.insert(stripHitsLayerContainer.begin(), stripHitsLayer.begin(), stripHitsLayer.end());

            }


        for (int i = 0; i < int(layer_comparator.size()); i++){

            double tempLayer  = comparator[layer_comparator[i]].first.Layer;
            vector<Comparator> tempComparator = comparator[layer_comparator[i]].second;

            vector<double> comparatorHits = MakeComparatorHitNotReadout(tempComparator, cfeb);
            vector<double> comparatorHitsLayer (int(comparatorHits.size()), tempLayer+0.5);
            ShiftStripHits(comparatorHits, tempLayer);

            comparatorHitsContainer.insert(comparatorHitsContainer.end(), comparatorHits.begin(), comparatorHits.end());
            comparatorHitsLayerContainer.insert(comparatorHitsLayerContainer.end(), comparatorHitsLayer.begin(), comparatorHitsLayer.end());

            }
        sHit_cHit_layer.push_back(stripHitsContainer);
        sHit_cHit_layer.push_back(stripHitsLayerContainer);
        sHit_cHit_layer.push_back(comparatorHitsContainer);
        sHit_cHit_layer.push_back(comparatorHitsLayerContainer);

        return sHit_cHit_layer;
//      stripHitDis = new TGraph(int(stripHitsContainer.size()), sHit, sHitLayer);//stripHitsContainer, stripHitsLayerContainer);
//      comparatorHitNotReadOut = new TGraph(int(comparatorHitsContainer.size()), comparatorHitsContainer, comparatorHitsLayerContainer);


//      stripHitDis->GetXaxis()->SetRangeUser(1, 82);
//      stripHitDis->GetYaxis()->SetRangeUser(1, 7);
}


vector<double> MakeComparatorHitNotReadout(vector<Comparator> c, double cfeb[]){

        vector<double> cHit;

        for (int i = 0; i < int(c.size()); i++){

            if (c[i].Strip < 80 && cfeb[int(c[i].Strip/16)] == 0){

               cHit.push_back(c[i].Strip+0.5);

               }else if(c[i].Strip == 80 && cfeb[4] == 0){

                       cHit.push_back(c[i].Strip+0.5);

                       }

            }

        return cHit;

}




void SetTitle(TPaveText* pt, TString name){


   pt->SetFillColor(0);
   pt->SetTextSize(0.06);
   pt->SetBorderSize(0);
   pt->SetTextAlign(21);
   pt->AddText(name);
}

template <typename T>
string NumberToString( T Number )
{
        stringstream ss;
        ss << Number;
        return ss.str();
}

void CountCFEB(double cfeb[], vector<Strips> s){

        for (int i = 0; i < int(s.size()); i++){

            int icfeb = (s[i].Strip-1)/16;
            cfeb[icfeb]++;
/*            if (s[i].Strip <= 16){

                cfeb[0]++;

                }else if(s[i].Strip >= 17 && s[i].Strip <= 32){

                        cfeb[1]++;

                        }else if(s[i].Strip >= 33 && s[i].Strip <= 48){

                                   cfeb[2]++;

                                   }else if(s[i].Strip >= 49 && s[i].Strip <= 64){

                                             cfeb[3]++;

                                             }else if(s[i].Strip >= 65 && s[i].Strip <=80){

                                                     cfeb[4]++;

                                                     } else if (s[i].Strip >= 81 && s[i].Strip <=96){

                                                               cfeb[5]++;
                                                               }
*/
            }

}

void MakeShadeForComparatorPanel(TH1F* cfebNotReadOut,  TH1F* cfebNotInstall, TH1F* cfebNotReadOut_comparator,  TH1F* cfebNotInstall_comparator_me21, TH1F* cfebNotInstall_comparator_me11) {

     for (int i = 1; i < cfebNotReadOut->GetSize()-1; i++) {

         if (cfebNotReadOut->GetBinContent(i) > 0) {

            cfebNotReadOut_comparator->SetBinContent(((i-1)/16*2)*16 + (i%16) + 1, 7);
            cfebNotReadOut_comparator->SetBinContent(((i-1)/16*2)*16 + (i%16) + 17, 7);

            }

         }
     for (int i = 129; i < cfebNotInstall_comparator_me21->GetSize()-1; i++) {

            cfebNotInstall_comparator_me21->SetBinContent(i, 7);

         }
     for (int i = 97; i < 128; i++) {

            cfebNotInstall_comparator_me11->SetBinContent(i, 7);

         }

      cfebNotInstall_comparator_me21->GetYaxis()->SetRangeUser(1,7);
      cfebNotInstall_comparator_me11->GetYaxis()->SetRangeUser(1,7);

      cfebNotReadOut_comparator->GetYaxis()->SetRangeUser(1,7);
    
      cfebNotReadOut_comparator->SetFillStyle(3001);
      cfebNotReadOut_comparator->SetFillColor(15);
      cfebNotInstall_comparator_me21->SetFillStyle(3001);
      cfebNotInstall_comparator_me21->SetFillColor(12);
      cfebNotInstall_comparator_me11->SetFillStyle(3001);
      cfebNotInstall_comparator_me11->SetFillColor(12);

}

//void BlockUnreadCFEB(double cfeb[], TH1F* cfebNotReadOut){
void BlockUnreadCFEB(double cfeb[], TH1F* cfebNotReadOut, int nCFEB){ 
//      gStyle->SetPalette(52, 0);

           for (int i = 0; i < nCFEB; i++){//in each interesting layer has strip hits

               if (cfeb[i] == 0){

//                for (int j = 0; j < 6; j++){//each layer

                      for (int k = 0; k < 16; k++){//each 16 strips for one cfeb

//                      if ()           

                          int x1 = (k+i*16)+1;
//                        int x2 = 2*(k+i*16)+2;
//                        int x3 = 2*(k+i*16)+3;

                          cfebNotReadOut->SetBinContent(x1, 7);

                          }

//                    } 
                  
                  if (nCFEB == 7 && i == 6) cfebNotReadOut->SetBinContent(113, 7);
                  if (nCFEB == 5 && i == 4) cfebNotReadOut->SetBinContent(81, 7);

                  }


               }

cfebNotReadOut->GetYaxis()->SetRangeUser(1,7);

}

void BlockNotInstalledCFEB(TH1F* cfebNotInstall_me21, TH1F* cfebNotInstall_me11) {

     for (int i = 65; i < cfebNotInstall_me21->GetSize()-1; i++) {

         cfebNotInstall_me21->SetBinContent(i, 7);

         }
cfebNotInstall_me21->GetYaxis()->SetRangeUser(1,7);

     for (int i = 49; i < 65; i++) {

         cfebNotInstall_me11->SetBinContent(i, 7);

         }
cfebNotInstall_me11->GetYaxis()->SetRangeUser(1,7);

}


void SetHistContour(TH2F* hist, double Min, double Max){

        gStyle->SetNumberContours(Max -Min);
          //here the actually interesting code starts
  const Double_t min = Min;
  const Double_t max = Max;

  const Int_t nLevels = Max - Min;
  Double_t levels[nLevels];


  for(int i = 1; i < nLevels; i++) {
    levels[i] = min + (max - min) / (nLevels - 1) * (i);
  }
  levels[0] = 0;

hist->SetContour((sizeof(levels)/sizeof(Double_t)), levels);
}


vector<double> MakeStripHit(vector<Strips> sp){

        vector<double> stripHits;

        sort(sp.begin(), sp.end());

        if (int(sp.size()) == 1){//one fired strip per layer

           MakeHit1(sp[0], stripHits);

           }else if(int(sp.size() > 1)){//more than one fired strip per layer

                   for (vector<Strips>::iterator it = sp.begin(); it != sp.end(); it++){

                        if ((*it).ADCMaxTime <= 2 || (*it).ADCMaxTime ==7){

                           (*it).MaxADC = 0;

                           }

                        if (it == sp.begin()){//first strip in layer

                           MakeHit2(*it, *(it+1), stripHits);

                           }else if(it == sp.end()-1){//last strip in layer

                                   MakeHit2(*it, *(it-1), stripHits);

                                   }else{

                                        MakeHit3(*it, *(it-1), *(it+1), stripHits);

                                        }

                        }

                   }


        return stripHits;
}


void ShiftStripHits(vector<double>& stripHits, double layer){

        if (layer == 1 || layer == 3 || layer ==5){

           for (int i = 0; i < int(stripHits.size()); i++){

               stripHits[i]+=0.5;

               }

           }

}


void MakeHit1(Strips &s, vector<double>& stripHits){//deal with strip singlet

        double sHitPos = -99;

        if (s.MaxADC >= 25 && s.ADCMaxTime > 2 && s.ADCMaxTime < 7){

           sHitPos = s.Strip + 0.5;

           }

        if (sHitPos > 0){

           stripHits.push_back(sHitPos);

           }

}


void MakeHit2(Strips &s, Strips &sSide, vector<double>& stripHits){// deal with first last fired strip in one layer

        Strips tempStrip;
        tempStrip.ADCTotal = 0;
        tempStrip.MaxADC = 0;

        if (s.Strip < sSide.Strip){//first fired strip in one layer

           if (CheckRight(s, sSide)){//have adjacent fired strip on the right

              if (IsGoodPeak(s, tempStrip, sSide)){//good peak

                   stripHits.push_back(s.Strip + 0.5 + FindRatio(s, tempStrip, sSide));

                   }

              }else{//singlet

                   MakeHit1(s, stripHits);

                   }

           }else if(s.Strip > sSide.Strip){//last fired strip in one layer

                   if (CheckLeft(s, sSide)){//have adjacent fired strip on the left

                      if (IsGoodPeak(s, sSide, tempStrip)){// good peak

                         stripHits.push_back(s.Strip + 0.5 + FindRatio(s, sSide, tempStrip));

                         }

                      }else{//singlet

                           MakeHit1(s, stripHits);

                           }

                   }

}


void MakeHit3(Strips &s, Strips &sSideL, Strips  &sSideR, vector<double>& stripHits){

        bool stripOnTheLeft = CheckLeft(s, sSideL);
        bool stripOnTheRight = CheckRight(s, sSideR);

        if ( !stripOnTheLeft && !stripOnTheRight){//singlet

           MakeHit1(s, stripHits);

           }else if( stripOnTheLeft && !stripOnTheRight ){// left, c, no right

                   MakeHit2(s, sSideL, stripHits);

                   }else if(!stripOnTheLeft && stripOnTheRight){//no left, c, right

                           MakeHit2(s, sSideR, stripHits);

                           }else if(stripOnTheLeft && stripOnTheRight){//left , c, right

                                   if (IsGoodPeak(s, sSideL, sSideR)){

                                      stripHits.push_back(s.Strip + 0.5 + FindRatio(s, sSideL, sSideR));

                                      }

                                   }



}

bool IsGoodPeak(Strips &s, Strips &sSideL, Strips& sSideR){

        bool goodPeak = false;

        if ( (s.MaxADC + sSideL.MaxADC + sSideR.MaxADC) > 25 &&//toal charge of strip cluster > 25
             (s.ADCMaxTime > 2 && s.ADCMaxTime < 7) &&//peak time >2 < 7
             (s.MaxADC >= sSideL.MaxADC && s.MaxADC >= sSideR.MaxADC) ){//is a peak..

           goodPeak = true;

           }


        return goodPeak;
}

double FindRatio(Strips &s, Strips& sSideL, Strips& sSideR){

        double ratio = -99;

        ratio = 0.5*(sSideR.ADCTotal - sSideL.ADCTotal)/(s.ADCTotal - min(sSideL.ADCTotal, sSideR.ADCTotal));

        return ratio;

}

bool CheckRight(Strips &s, Strips &sR){

//      double chargeOfRightStrip = -99;
        bool nextToThisStrip = false;

        if (sR.Strip - s.Strip == 1){

//         chargeOfRightStrip = s2.ADCTotal;
           nextToThisStrip = true;

           }

//      return chargeOfRightStrip;
        return nextToThisStrip;
}

bool CheckLeft(Strips &s, Strips &sL){

//        double chargeOfLeftStrip = -99;
        bool nextToThisStrip = false;

        if (s.Strip - sL.Strip == 1){

//           chargeOfLeftStrip = s2.ADCTotal;
             nextToThisStrip = true;

           }

//        return chargeOfLeftStrip;
        return nextToThisStrip;
}


template <class T>
vector<int> FindChamberIndex(CSCDetID id, vector<T> &vec){

        vector<int> chamber;
        for (int i = 0; i < int(vec.size()); i++){

            CSCDetID tempID = vec[i].first;
            
            if (id.Endcap == tempID.Endcap &&
                id.Station == tempID.Station &&
                id.Ring == tempID.Ring &&
                id.Chamber == tempID.Chamber){

                chamber.push_back(i);

                }

            }

        return chamber;
}


void WireDisplay(CSCDetID id, vector<int>& layer_wire, vector<WIRE>& wire, TH2F* wireDis, TH2F* wireDis_text, TH2F* alctDis, vector<CorrelatedLCT>& alcts){


          if (id.Ring == 4) id.Ring = 1; //me11a wire digi are saved in me11b

          for (int i = 0; i < int(layer_wire.size()); i++){//in each interesting layer has wire hits

              int tempLayer = wire[layer_wire[i]].first.Layer;
              vector<Wire> tempWire = wire[layer_wire[i]].second;

              MakeOneLayerWireDisplay(tempLayer, tempWire, wireDis);

              if (tempLayer == id.Layer){

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
          wireDis->SetTitle("");//"Anode Hit Timing");
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


void ComparatorDisplay(CSCDetID id, vector<int>& layer_comparator, vector<COMPARATOR>& comparator, TH2F* comparatorDis, TH2F* comparatorDis_text, TH2F* clctDis, vector<CorrelatedLCT>& clcts){
 //cout <<"ComparatorDisplay "<<  id.Endcap << "," << id.Station << "," << id.Ring << "," << id.Chamber << endl;
 //cout << "layer_comparator: " << layer_comparator.size() << endl;

          for (int i = 0; i < int(layer_comparator.size()); i++){//in each interesting layer has wire hits

              int tempStation = comparator[layer_comparator[i]].first.Station;
              int tempRing = comparator[layer_comparator[i]].first.Ring;
              int tempLayer = comparator[layer_comparator[i]].first.Layer;
              vector<Comparator> tempComparator = comparator[layer_comparator[i]].second;
              //cout <<" looking for id  "<< comparator[layer_comparator[i]].first <<" comparator size "<< tempComparator.size()  << endl;
              if (tempComparator.size() == 0) cout <<"Error!! No Comparator is found in "<< comparator[layer_comparator[i]].first << endl;
              //for (auto c : tempComparator) cout << c.ComparatorNumber<<","<<c.Strip<<","<<c.TimeBin <<";  ";
              //cout << endl;

              bool doStagger = false;
              if (!(tempStation==1 && (tempRing==1||tempRing==4))) doStagger = true;

              MakeOneLayerComparatorDisplay(tempLayer, tempComparator, comparatorDis, doStagger);

              if (tempLayer == id.Layer){

                 MakeOneLayerComparatorDisplay(tempLayer, tempComparator, comparatorDis_text, doStagger);

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
          comparatorDis->SetTitle("");//"Comparator Hit Timing");
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


void SetPlotDetail_StripHit(TGraph* stripHitDis){

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
          stripHitDis->SetTitle();//"Strip Hit");


}

