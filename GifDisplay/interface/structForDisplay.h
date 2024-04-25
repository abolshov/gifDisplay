#ifndef gifDisplay_GifDisplay_structForDisplay_h 
#define gifDisplay_GifDisplay_structForDisplay_h 

#include <iostream>
#include <fstream>
#include <list>
#include <vector>
#include <iomanip>
#include <ctime>
#include <string>
#include <cmath>

using namespace std;

inline bool doubleEqual(double a, double b) {
        return abs(a - b) < 1E-6;
}
//https://github.com/cms-sw/cmssw/blob/master/DataFormats/MuonDetId/interface/CSCDetId.h
struct CSCDetID{

        double Endcap;
        double Station;
        double Ring;
        double Chamber;
        double Layer;

        bool operator == (const CSCDetID& id)  {
                return doubleEqual(id.Endcap, this->Endcap)
                        && doubleEqual(id.Station, this->Station)

                        && doubleEqual(id.Chamber, this->Chamber)
                        && doubleEqual(id.Layer, this->Layer);
        }

        CSCDetID(){
                this->Endcap = 0.0;
                this->Station = 0.0;
                this->Ring=0.0;
                this->Chamber = 0.0;
                this->Layer = 0.0;
        }
        friend ostream& operator << (ostream& os, const CSCDetID& id)
        {
                os << "E:" << id.Endcap <<" S:"<< id.Station <<" R:"<<id.Ring <<" Chamber:"<<id.Chamber <<" layer:"<<id.Layer;
                return os;
        }

};

//GEMDetID for GE11 and GE21, use station=0 and ring=0 for ME0
//GEMDetId from CMSSW: GEMDetId(int region, int ring, int station, int layer, int chamber, int ieta)
//region, -1/+1, ring=1, station=0,1,2, layer=1,2. 0 is superchamber, etapartition=0-16
//for ME0  ME0DetId(int region, int layer, int chamber, int roll); with ch0-18,region=-1/+1 layer=0-6, and roll0-10
//ME0: https://github.com/cms-sw/cmssw/blob/master/DataFormats/MuonDetId/interface/ME0DetId.h
struct GEMDetID{

        double Endcap;
        double Station;
        double Ring;
        double Chamber;
        double Layer;
        double Roll;

        bool operator == (const GEMDetID& id)  {
                return doubleEqual(id.Endcap, this->Endcap)
                        && doubleEqual(id.Station, this->Station)
                        && doubleEqual(id.Chamber, this->Chamber)
                        && doubleEqual(id.Layer, this->Layer)
                        && doubleEqual(id.Roll, this->Roll);
        }

        GEMDetID(){
                this->Endcap = 0.0;
                this->Station = 0.0;
                this->Ring=0.0;
                this->Chamber = 0.0;
                this->Layer = 0.0;
                this->Roll = 0.0;
        }
        friend ostream& operator << (ostream& os, const GEMDetID& id)
        {
                os << "E:" << id.Endcap <<" S:"<< id.Station <<" R:"<<id.Ring <<" Chamber:"<<id.Chamber <<" layer:"<<id.Layer<<" etaPartition:"<<id.Roll;
                return os;
        }

};

//https://github.com/cms-sw/cmssw/blob/master/SimDataFormats/TrackingHit/interface/PSimHit.h
struct  SimHit{ //GEM and CSC?

  int WireGroup;
  float Stripf;
  int Strip;
  int PdgId;

  bool operator == ( const SimHit& sh) const{

       return sh.WireGroup == this->WireGroup && sh.Strip == this->Strip && sh.PdgId == this->PdgId;

  }


  SimHit(){

        this->WireGroup = -99;
        this->Strip = -99;
        this->Stripf = -99;
        this->PdgId = 0;

        }

};

struct  Wire{

  int WireGroup;
  int TimeBin;
  int NumberTimeBin;

  bool operator == ( const Wire& WG) const{

       return WG.WireGroup == this->WireGroup;

  }

  bool operator < ( const Wire& WG) const{

       return this->WireGroup < WG.WireGroup;}

  Wire(){

        this->WireGroup = -99;
        this->TimeBin = -99;
        this->NumberTimeBin = -99;

        }

};


struct Strips{

 int Strip;
 int ADCTotal;
 int MaxADC;
 int ADCMaxTime;
 vector<double> ADC;
 vector<double> TimeBin;

 bool operator == ( const Strips& st) const {

       return st.Strip == this->Strip;

  }

  bool operator < ( const Strips& st) const {

       return  this->Strip < st.Strip;
  }
  Strips(){

        this->Strip = -99;
        this->ADCTotal = -99;
        this->MaxADC = -99;
        this->ADCMaxTime = -99;

   }

};


struct Comparator{

 int Strip;
 int TimeBin;
 int ComparatorNumber;

 bool operator < ( const Comparator& com) const{

  return this->ComparatorNumber < com.ComparatorNumber;

  }

};

//https://github.com/cms-sw/cmssw/blob/master/DataFormats/GEMDigi/interface/GEMPadDigi.h
struct GEMPad{

 int Pad;
 int TimeBin;

 bool operator < ( const GEMPad& gempad) const{

  return this->Pad < gempad.Pad;

  }

};


//https://github.com/cms-sw/cmssw/blob/master/DataFormats/GEMDigi/interface/GEMPadDigiCluster.h
struct GEMPadCluster{
  int firstPad;
  int size;
  int nRolls;
  int BX;
  GEMPadCluster(){
      this->firstPad = -1;
      this->size = -1;
      this->nRolls = -1;
      this->BX = -1;
  }
};



struct CorrelatedLCT{
  int keyStrip;
  int keyWG;
  int quality;
  int pattern;
  int BX;
  int run3_pattern;
  int eighthStrip;
  int slope;
  int bend;  
  CorrelatedLCT(){
      this->keyStrip = -1;
      this->keyWG = -1;
      this->quality = -1;
      this->pattern = -1;
      this->BX = -1;
      this->run3_pattern = -1;  
      this->eighthStrip = -1;
      this->slope = -1;
      this->bend = -1;
  }
};

#endif
