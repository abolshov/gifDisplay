# gifDisplay



This packge is used to display CSC hits and CSC local track for CMS events from simulation or data. Later in 2022 the GEM event display is added to display GEM pads and GEM simhits in GE11 chamber. 

## Setup
```
cmsrel CMSSW_12_5_0_pre2
cd CMSSW_12_5_0_pre2/src
cmsenv
git clone https://github.com/tahuang1991/gifDisplay.git
scram b -j 9
```
## How to use CSC event display package 


 1. CSCEventDisplay.py uses "process.source" to get input files.
  By default input root files must have RAW data to be unpacked, using edmDumpEventContent one should 
  see event has "FEDRawDataCollection" type. Configure the process you need.
 
 1. CSCEventDisplay.py needs to read in the run range and event range for each display case from a txt file given by argument eventListFile
 
 1. save event and chamber id for which you want to make eventdisplay for in eventList.txt. by default it would display the neighbor GE11 chamber if it is to display ME11 chamber.  The GE21 part is not consider yet but can be added in future. 


   Format in eventListFile: runNumber eventNumber endcapID stationID ringID chamberID (note: use ringID=1 for both ME11a and ME11b). 
   and below is one example how the format in textfile looks like
```   
1 2002 1 1 1 34
1 2002 1 4 1 17
1 2038 2 1 1 6
1 2038 1 1 1 24
```
## Examples to run CSC display

>```
>./runDisplay.sh
>```

Make sure that sample path, plot directory and eventListFile are replaced with yours


## Reference
original package:
https://github.com/mhl0116/gifDisplay

# CSC Event Display — User Guide

This tool (`CSCEventDisplay.py`) is a CMSSW configuration for visualizing and validating **CSC (Cathode Strip Chamber) trigger primitives**, with optional support for **GEM** and **GEM‑CSC integrated local trigger (ILT)** studies. It can run on RAW data, on MC, or on data that already contains emulated trigger primitives, and it can produce event‑display plots and/or DQM comparison histograms.

---

## Quick Start

    cmsRun CSCEventDisplay.py \
        inputFiles=file:/path/to/input.root \
        eventListFile=eventList.txt \
        plotdir=/path/to/output/plots/ \
        unpack=True l1=True run3=True

The tool always runs the `GifDisplay` analyzer, which produces per‑event display plots for the chambers/events specified in `eventListFile`. The other steps (unpacking, emulation, DQM) are added to the schedule **only if** the corresponding options are enabled.

---

## The Event List File

The `eventListFile` (default `eventList.txt`) drives *which* events and chambers are drawn. The job stops if the file does not exist. Format:

    # run eventnumber endcap station ring chamber
    # endcap = 1: positive (+), 2: minus (-)
    # ME1a and ME1b both use ring = 1
    355872 123456789 1 1 1 12
    ...

For every listed run:event pair, the config appends a matching entry to `process.source.eventsToProcess`, so only those events are read from the input files.

---

## Options Reference

### Input / Output

| Option | Type | Default | Meaning |
|---|---|---|---|
| `inputFiles` | list | – | Standard VarParsing input file list. |
| `plotdir` | string | `''` | Directory where the event‑display plots are written (`eventDisplayDir`). |
| `eventListFile` | string | `eventList.txt` | Text file with the run/event/chamber list to display. **Required to exist.** |
| `dqmOutputFile` | string | `step_DQM.root` | Name of the DQM output ROOT file. |
| `saveEdmOutput` | bool | `False` | Keep the EDM ROOT output (`cscdisplay_edmout.root`) after unpacking/emulation. |
| `dropNonMuonCollections` | bool | `True` | Drop most non‑muon collections (calorimeter, tracker, BMTF, sim/HLT ecal, etc.) from EDM output to reduce size. Recommended for GEM/CSC studies. |

### Data source / era

| Option | Type | Default | Meaning |
|---|---|---|---|
| `mc` | bool | `False` | Running on Monte Carlo. Changes GlobalTag, enables SimHit/SimTrack input tags, and switches digi collection labels to `simMuonCSCDigis`. |
| `run3` | bool | `True` | Use Run‑3 configuration/GlobalTag. If `False`, uses `Run2_2018` era (note: this era import is commented out — see caveats). |

> **Note:** The process era is hardcoded to `Phase2C17I13M9` and the geometry to `GeometryExtended2026D110`, so this configuration is set up for a Phase‑2 style workflow. GlobalTags are chosen based on `mc` and `run3`.

### Unpacking (RAW → digis)

| Option | Type | Default | Meaning |
|---|---|---|---|
| `unpack` | bool | `False` | Run the CSC unpacker (`muonCSCDigis`) to convert RAW CSC DAQ data into digis. |
| `unpackGEM` | bool | `False` | Also unpack GEM DAQ data (`muonGEMDigis`). Sets `labelRawDataLikeMC=False` on the source. |

### Emulation (re‑running trigger primitives)

| Option | Type | Default | Meaning |
|---|---|---|---|
| `l1` | bool | `False` | Re‑emulate CSC trigger primitives (runs `cscTriggerPrimitiveDigis` + the Run‑3 CCLUT clone). |
| `l1GEM` | bool | `False` | Re‑emulate GEM trigger primitives; points `simMuonGEMPadDigis` input at `muonGEMDigis`. |

### CSC emulator algorithm variants (only relevant when `l1=True`)

| Option | Type | Default | Meaning |
|---|---|---|---|
| `runCCLUTOTMB` | bool | `False` | Enable the CCLUT OTMB algorithm. |
| `runCCLUTTMB` | bool | `False` | Enable the CCLUT TMB algorithm. |
| `runME11ILT` | bool | `False` | Run GEM‑CSC integrated local trigger in ME1/1 (requires GEM pad clusters). |
| `runME21ILT` | bool | `False` | Run GEM‑CSC integrated local trigger in ME2/1. |

### B904 test‑stand data

| Option | Type | Default | Meaning |
|---|---|---|---|
| `useB904ME11` | bool | `False` | Input is B904 ME1/1 data. |
| `useB904ME21` | bool | `False` | Input is B904 ME2/1 data (also ME3/1, ME4/1). |
| `useB904ME234s2` | bool | `False` | Input is B904 MEX/2 or ME1/3 data. |

If any B904 flag is set, the unpacker is configured for the B904 setup (`DisableMappingCheck`, `B904Setup=True`, custom `InputObjects` / `InputLabel`).

### DQM (data‑vs‑emulator comparison)

| Option | Type | Default | Meaning |
|---|---|---|---|
| `dqm` | bool | `False` | Run the CSC DQM module (`l1tdeCSCTPG`) comparing data vs emulated ALCT/CLCT/LCT. |
| `dqmGEM` | bool | `False` | Also run the GEM DQM module (`l1tdeGEMTPG`). |
| `useEmtfGEM` | bool | `False` | In GEM DQM, take GEM pad clusters from the EMTF (`emtfStage2Digis`) rather than from the CSC TPG (`muonCSCDigis`). |
| `preTriggerAnalysis` | bool | `False` | Print more CLCT/LCT details in the offline CSC DQM module. |

### Display extras

| Option | Type | Default | Meaning |
|---|---|---|---|
| `doGEMCSC` | bool | `False` | Enable GEM‑CSC plotting in the display; also turns on the GEM display. |

---

## Schedule Logic (What Actually Runs)

The final schedule is built conditionally:

1. **`p1` (unpack sequence)** — added only if `unpack=True`. Runs `muonCSCDigis` (+`muonGEMDigis`, +`emtfStage2Digis` if `unpackGEM`/`useEmtfGEM`).
2. **`p2` (l1 sequence)** — added only if `l1=True`. Runs `simMuonGEMPadDigis → simMuonGEMPadDigiClusters → cscTriggerPrimitiveDigis → simCscTriggerPrimitiveDigisRun3CCLUTv0`.
3. **`display`** — always added. Runs the `GifDisplay` analyzer.
4. **`p3` + `p4` (DQM + DQMoutput)** — added only if `dqm=True`.
5. **`p5` (EDM output)** — added only if `saveEdmOutput=True`.
6. **`p6` (endOfProcess)** — always added.

The `GifDisplay` analyzer configures itself from options:

- `addEmulation = l1 OR l1GEM`
- `addSimHits = mc`
- `doGEMCSC = doGEMCSC`
- `doGEMDisplay = runME11ILT OR runME21ILT OR l1GEM OR doGEMCSC`
- In MC mode, digi/simhit input tags switch to the `simMuon*`/`g4SimHits` collections and the emulated primitives are read from `simCscTriggerPrimitiveDigisRun3CCLUTv0`.

---

## Typical Workflows

### 1. RAW data → emulate primitives → display

You have RAW CSC DAQ data and want to produce emulated trigger primitives on the fly:

    cmsRun CSCEventDisplay.py \
        inputFiles=file:raw.root \
        unpack=True l1=True run3=True \
        plotdir=./plots/ eventListFile=eventList.txt

- `unpack=True` decodes RAW → digis.
- `l1=True` runs the emulator (the config automatically wires the emulator inputs to `muonCSCDigis:MuonCSC{Comparator,Wire}Digi`).
- Display shows **Data vs Emul** (top/bottom labels `compareA="Data"`, `compareB="Emul"`).

### 2. Data that already contains emulated primitives → display only

If the input already has emulated trigger primitives, **do not** re‑run the emulator:

    cmsRun CSCEventDisplay.py \
        inputFiles=file:withEmul.root \
        unpack=False l1=False run3=True \
        plotdir=./plots/ eventListFile=eventList.txt

- No `p1`/`p2` steps; only the display runs, reading the existing primitives from the file.

### 3. Monte Carlo

    cmsRun CSCEventDisplay.py \
        inputFiles=file:mc.root \
        mc=True l1=True run3=True \
        plotdir=./plots/ eventListFile=eventList.txt

- `mc=True` switches to `simMuonCSCDigis`, enables SimHits/SimTracks in the display, and reads emulated primitives from the Run‑3 CCLUT clone.

### 4. GEM‑CSC integrated local trigger (ILT)

    cmsRun CSCEventDisplay.py \
        inputFiles=file:raw.root \
        unpack=True unpackGEM=True l1=True l1GEM=True \
        runME11ILT=True run3=True doGEMCSC=True \
        plotdir=./plots/ eventListFile=eventList.txt

- Unpacks both CSC and GEM, re‑emulates both, runs the ME1/1 ILT algorithm, and enables GEM‑CSC displays.

### 5. Data‑vs‑emulator DQM validation

    cmsRun CSCEventDisplay.py \
        inputFiles=file:raw.root \
        unpack=True l1=True dqm=True dqmGEM=True \
        dqmOutputFile=step_DQM.root run3=True \
        plotdir=./plots/ eventListFile=eventList.txt

- Produces DQM comparison histograms in `step_DQM.root` in addition to the display.

### 6. B904 test‑stand data

    cmsRun CSCEventDisplay.py \
        inputFiles=file:b904.root \
        unpack=True l1=True dqm=True useB904ME11=True \
        plotdir=./plots/ eventListFile=eventList.txt

- Configures the unpacker and DQM module for the B904 ME1/1 setup.

---

## Caveats / Notes

- The emulator step always includes the `simCscTriggerPrimitiveDigisRun3CCLUTv0` clone, whose parameters (CCLUT, ILT, shower zoning) are hardcoded in the config; the `l1csc` (`cscTriggerPrimitiveDigis`) module's ILT/CCLUT behavior is controlled by the `runCCLUT*`/`runME*ILT` options.
- When `l1=True` and running on data/after unpacking, the emulator's comparator/wire digi inputs are automatically pointed to `muonCSCDigis`. When `runME11ILT`/`runME21ILT` is on, the GEM pad cluster producer is pointed to `muonCSCDigis:MuonGEMPadDigiCluster`.
- `saveEdmOutput=True` writes `cscdisplay_edmout.root`. With `dropNonMuonCollections=True` (default), the file is trimmed to muon‑relevant collections only.
- The input source drops `CSCDetIdCSCShowerDigiMuonDigiCollection_simCscTriggerPrimitiveDigis_*_*` on read, and the EDM output always drops `rawDataCollector`.
- The commented‑out `useInputDir(...)` helper can populate `process.source.fileNames` from an EOS/dCache directory (only files starting with `lcts` and ending in `.root`). Uncomment and set `inputdirs` if you prefer directory‑based input over the `inputFiles` option.
- `maxEvents` is set to `-1` (all events), but effectively only the events listed in `eventListFile` are read because of `eventsToProcess`.

---

## Output Files Summary

| File | Produced when | Content |
|---|---|---|
| Event‑display plots (in `plotdir`) | always | Per‑event chamber displays from `GifDisplay`. |
| `output.root` | always | `GifDisplay` internal ROOT output (histograms/trees). |
| `cscdisplay_edmout.root` | `saveEdmOutput=True` | EDM event data after unpack/emulation (trimmed if `dropNonMuonCollections`). |
| `step_DQM.root` (or `dqmOutputFile`) | `dqm=True` | DQM comparison histograms (data vs emulator). |

---

## Option Decision Cheat‑Sheet

    Input is RAW?                 → unpack=True
    Need emulated CSC primitives? → l1=True   (skip if file already has them)
    Need emulated GEM primitives? → l1GEM=True (+ unpackGEM=True on RAW)
    Running on simulation?        → mc=True
    GEM-CSC ILT study?            → runME11ILT / runME21ILT (+ GEM unpack/emul)
    Want data-vs-emu validation?  → dqm=True (+ dqmGEM=True for GEM)
    B904 test stand?              → useB904ME11 / useB904ME21 / useB904ME234s2
    Keep EDM file?                → saveEdmOutput=True
    Run-3 vs Phase-2/Run-2?       → run3 (default True)