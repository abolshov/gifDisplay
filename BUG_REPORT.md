# gifDisplay — Code Audit / Bug Report

**Date:** 2026-11-05
**Release:** CMSSW_16_1_0 (API checks against `/cvmfs/cms.cern.ch/el9_amd64_gcc13/cms/cmssw/CMSSW_16_1_0/src/`)
**Repository:** `CMSSW_16_1_0/src/gifDisplay` (branch `main`, HEAD `2b21d8b`)
**State audited:** the **working tree, including uncommitted changes** (`git diff`: `CSCEventDisplay.py`, `display.h`, `structForDisplay.h`, `GifDisplay.cc`, `display.cc`, `displaygem.cc`, `run_test.sh`, `file_lists/test_event.txt`).
**Method:** I read the code only. Nothing was built, run or modified. Each claim was checked against the code and, where relevant, the CMSSW release sources. Findings that depend on runtime behaviour I could not observe are marked **Unverified**.

## Scope

| File | Lines | Read |
|---|---:|---|
| `CSCEventDisplay.py` | 515 | fully |
| `GifDisplay/plugins/GifDisplay.cc` | 1103 | fully |
| `GifDisplay/interface/display.h` | 159 | fully |
| `GifDisplay/interface/structForDisplay.h` | 301 | fully |
| `GifDisplay/src/display.cc` | 1615 | fully |
| `GifDisplay/src/displaygem.cc` | 675 | fully |
| `run_test.sh`, `GifDisplay/BuildFile.xml`, `GifDisplay/plugins/BuildFile.xml`, `GifDisplay/python/*.py`, `README.md`, `CLAUDE.md`, LUTs under `GifDisplay/GEMCSC/` | — | skimmed / checked as needed |
| `CSCEventDisplay_realdata.py`, `dataextract*`, `runDisplay*.sh` | — | skimmed (appendix only) |

## Executive summary

| Section | High | Medium | Low | Total |
|---|---:|---:|---:|---:|
| Configuration (`C-`) | 1 | 6 | 4 | 11 |
| Plugin (`P-`) | 0 | 4 | 5 | 9 |
| Implementation (`I-`) | 3 | 8 | 12 | 23 |
| **Total bugs** | **4** | **18** | **21** | **43** |
| Strange / unexplained actions (`S-`) | — | — | — | 16 (questions for the original authors) |

Severity scale: **High** = wrong physics content, a crash, or a silently wrong plot. **Medium** = wrong only in some configurations, or misleading output. **Low** = cleanup, robustness or performance.

**The 5 most important issues**

1. **I-1 (High):** In every staggered chamber (all except ME1/1), the CLCT pattern box is drawn **one half-strip to the left** of the comparator hits it was built from. The display shifts layers 1/3/5 by +1 half-strip, but the CLCT key half-strip is given in the key-layer frame and is not shifted.
2. **I-2 (High):** Legacy-format CLCTs (`run3_pattern` = 0 by default) are drawn as Run-3 **pattern 0**, the steepest left-bending envelope. This affects every non-OTMB chamber in Run-3 (ME1/2, ME1/3, MEX/2), every `run3=false` job, and the Run-2 emulator.
3. **C-1 (High):** `l1GEM=true` without `l1=true` does nothing useful. The GEM pad emulators sit in path `p2`, which is scheduled only when `l1` is set. GifDisplay then looks for emulator products that were never made. On data this shows **no GEM clusters at all**, and `addEmulation` is switched off for the rest of the job.
4. **I-9 (High):** The GEM-CSC LUT folder is the relative path `"GifDisplay/GEMCSC/"`. With `doGEMCSC=true`, any job not started from the repository root fails to open the LUTs without warning. `WireToRoll` then throws `std::out_of_range` (`WG_max.at(i)`) and **the job aborts**.
5. **I-5 (Medium):** GE1/1 pads are drawn **one pad to the right** of their true position (`SetBinContent(FindBin(pad)+1)`). In the same panel the extrapolated-LCT windows are drawn at the correct bin, so pad-vs-LCT matching looks wrong by one pad.

Other notable issues: SimHit wire groups are offset by +1 WG from the wire digis (I-4). Stub text arrays can overflow with more than 10 stubs (I-7). Data stubs are never printed unless emulation is on (I-6). The global palette changes after the first GEM plot (I-8). Several emulator-clone parameters are set at the wrong PSet level and are silently ignored (C-3). There is no consistent era/geometry/GT for Run-2 or Phase-2 (C-4).

---

## 1. Configuration bugs (`CSCEventDisplay.py`)

### C-1 — `l1GEM=true` without `l1=true`: GEM emulation never runs, so data shows no GEM clusters and emulation is disabled — **High**
- **Where:** `CSCEventDisplay.py:395` (`addEmulation = l1 or l1GEM`), `:443-446` (GEM tags → `simMuonGEMPadDigis`/`simMuonGEMPadDigiClusters` when `l1GEM`), `:450-451` (OTMB-cluster fallback only when `not l1GEM`), `:481-484` (GEM pad producers inserted into `l1sequence`/`p2`), `:501-502` (`p2` scheduled **only** `if options.l1`).
- **What goes wrong:** With `l1GEM=true l1=false`:
  - `simMuonGEMPadDigis` and `simMuonGEMPadDigiClusters` never run.
  - GifDisplay reads tags that are never produced. The `isValid()` checks hide this, so GEM pads and clusters are skipped silently.
  - On data, the OTMB-cluster fallback (`:450`) is disabled because `l1GEM` is true, so **no GEM clusters are shown at all**.
  - `addEmulation=True`, but `cscTriggerPrimitiveDigis` is not run. The first event throws inside the `try` (`GifDisplay.cc:829-842`), and emulation is switched off for the rest of the job.
- The option help text "re-emulate the GEM trigger primitives" (`:20-21`) and README line 114 promise behaviour that does not happen.
- **Fix:** Schedule a GEM-emulation path when `l1GEM` is set, independent of `l1`. For example, make `p2gem = Path(simMuonGEMPadDigis*simMuonGEMPadDigiClusters)` and add it when `options.mc and options.l1 or options.l1GEM`. Set `addEmulation = options.l1`, since the CSC emulator is the only "Emul" source the display uses. Alternatively, make the config fail with a clear message when `l1GEM and not l1`.

### C-2 — With `l1GEM=true l1=true` on data, the GEM clusters shown are not the ones fed to the CSC emulator — **Medium**
- **Where:** `:175-176` (`l1csc.GEMPadDigiClusterProducer = "muonCSCDigis:MuonGEMPadDigiCluster"` whenever `runME11ILT/runME21ILT` on data), `:178-179` (`simMuonGEMPadDigis.InputCollection = 'muonGEMDigis'`), `:443-446` (display shows `simMuonGEMPadDigiClusters`).
- **Consequence:** The GE1/1 plot and its cluster list show the **re-emulated** GEM clusters (built from unpacked GEM strips). The CSC emulator's ILT uses the **OTMB-readout** clusters. A user who sees an emulated LCT next to GEM clusters cannot tell that the matching was done with different clusters. The name `l1GEM` suggests the re-emulated GEM TPs are what feed the emulator.
- **Fix:** Decide the intent. If `l1GEM` should feed the emulator, set `l1csc.GEMPadDigiClusterProducer = "simMuonGEMPadDigiClusters"` when `l1GEM`. Otherwise label the plot clearly, or display both cluster sources.

### C-3 — Emulator-clone parameters set at the wrong PSet level are silently ignored — **Medium**
- **Where:**
  - `:199`, `:228`: `...commonParam.GEMPadDigiClusterProducer = cms.InputTag("simMuonGEMPadDigiClusters")`
  - `:214`: `...commonParam.clctStateMachineZone = 4`
  - `:216`: `...commonParam.clctPretriggerTriggerZone = 224`
  - `:187`, `:207`: `...commonParam.runCCLUT = ...`
- **Release check:**
  - `CSCTriggerPrimitivesProducer.cc:117` reads `GEMPadDigiClusterProducer` from the **top-level** PSet (`cscTriggerPrimitiveDigis_cfi.py`: `GEMPadDigiClusterProducer = cms.InputTag("")`, set to `simMuonGEMPadDigiClusters` by `run3_GEM`).
  - `CSCUpgradeCathodeLCTProcessor.cc:18,21` read `clctStateMachineZone`/`clctPretriggerTriggerZone` from `clctParams()` (`clctPhase2`), not `commonParam`.
  - `commonParam` has no `runCCLUT` (`auxiliaryParams.py`; only `runCCLUT_TMB`/`runCCLUT_OTMB`).
- **Consequence:** These lines add new, unread parameters. The visible effect is that `simCscTriggerPrimitiveDigisRun3CCLUTv0` does **not** use `simMuonGEMPadDigiClusters` as line 228 intends. It inherits whatever `l1csc` had when it was cloned: `muonCSCDigis:MuonGEMPadDigiCluster` on data with ILT, otherwise the era default. The `clctPhase2.*` duplicates at `:215`, `:217` do take effect, so only the `commonParam` copies are dead.
- **Fix:** Use `process.simCscTriggerPrimitiveDigisRun3CCLUTv0.GEMPadDigiClusterProducer = ...` (top level). Delete the `commonParam.clct*Zone` and `commonParam.runCCLUT` lines. Add a `fillDescriptions`-style check, or `print` the clone, to catch this kind of mistake.

### C-4 — Era, geometry and GT are inconsistent for `run3=false`; Phase-2 support was lost; the GT is assigned several times — **Medium**
- **Where:** `:3-5`, `:57-62` (era `Run3`, or `Run2_2018` if `run3=false`; `Phase2C17I13M9` imported but unused), `:70-74` (`run3=false` loads **Phase-2** `GeometryExtendedRun4D110Reco_cff`), `:118-141` (GT).
- **Consequences:**
  - `run3=false` gives a **Run-2 era with Phase-2 (Run4 D110) geometry** and a Run-3 data GT. That combination is not meaningful for either Run-2 data or Phase-2 MC.
  - MC with `run3=false` gets the **data** GT `112X_dataRun3_Prompt_v5` (`:122`).
  - Data GT assignments at `:132`, `:134`, `:135` are dead. They are overwritten by `:141` (`150X_dataRun3_Prompt_v1`), which is also applied to `run3=false` data.
  - The Phase-2 workflow (`runDisplayYumeng.sh` and README examples use `Phase2Spring24` DY samples with `run3=true`) now runs with the Run-3 era, DB geometry and `auto:phase1_2022_realistic`. That is the wrong geometry for those samples (see P-6 for a possible crash).
- **Fix:** Replace `run3` with an explicit `era` option (`run2`, `run3`, `phase2`) that sets era, geometry and GT together. Remove the dead GT lines.

### C-5 — The `Run3CCLUTv0` emulator clone always runs with `l1=true`, even when unused; ILT is forced on; very high verbosity — **Medium**
- **Where:** `:202-228` (clone definition: `runME11ILT = True` at `:210`, `verbosity = 10` at `:218-220`, `tmbPhase1.verbosity = 5` at `:221`), `:474-478` (always part of `l1sequence`), `:385-387` (for data, "Emul" = `cscTriggerPrimitiveDigis`, so the clone output is never read).
- **Consequences:**
  - On data the clone costs CPU and floods the log at verbosity 10 for no output.
  - Because ILT is forced on, with `runME11ILT=false` on data it consumes the era-default `simMuonGEMPadDigiClusters`. That collection is not produced, so it logs a "Collection of GEM clusters ... not found" warning every event (`CSCTriggerPrimitivesProducer.cc:291-296`).
  - It also needs the ME11 ILT LUT record even when the user switched ILT off.
- **Fix:** Add the clone to `l1sequence` only for MC (the only case where it is displayed). Respect `options.runME11ILT`. Set verbosity to 0 unless debugging.

### C-6 — The default `plotdir=''` writes plots to the filesystem root — **Medium**
- **Where:** `:10` (default `''`), `:407`. Name construction: `display.cc:517`, `:528` and `displaygem.cc:200`, `:210` (`name = address + "/" + Run + "_" + ...`).
- **Consequence:** With no `plotdir`, files go to `/403775_..._MEMinus1_1_22.pdf`. ROOT cannot write there, so the job "succeeds" with no plots. A non-existent `plotdir` also fails silently, because nothing creates or checks it.
- **Fix:** Default `plotdir` to `'.'`, or require it. Create the directory with `os.makedirs(..., exist_ok=True)` in the config, and join paths properly in C++.

### C-7 — MC with `l1=true` compares two emulators, not "Data" vs "Emul" (known, open) — **Medium**
- **Where:** `:431-436`: "Data" = `cscTriggerPrimitiveDigis` (this job), "Emul" = `simCscTriggerPrimitiveDigisRun3CCLUTv0` (this job). The labels `compareA="Data"`, `compareB="Emul"` (`:403-404`) are misleading.
- **Consequence:** For MC with `l1=true`, the TPs stored in the file (`simCscTriggerPrimitiveDigis`) are never shown. The "Emul" clone also uses non-standard parameters (see S-9).
- **Fix:** Already documented in `CLAUDE.md` as a user decision. At least set `compareA`/`compareB` to the real module names for MC.

### C-8 — `dqmGEM` and `useEmtfGEM` do nothing in most configurations — **Low**
- **Where:** `:244-252`, `:486-489` (`p3` built always), `:509-510` (`p3`/`p4` scheduled only `if options.dqm`), `:459-464` (`emtfStage2Digis` only with `unpackGEM`).
- **Consequence:** `dqmGEM=true` without `dqm=true` does nothing. `useEmtfGEM=true` without `unpackGEM` points the GEM DQM at a product that is never produced. `l1tdeGEMTPG.emul = simMuonGEMPadDigiClusters` is produced only with `l1 and (mc or l1GEM)`.
- **Fix:** Schedule `p3` if `dqm or dqmGEM`, and check that the inputs of the requested DQM are produced.

### C-9 — MC with `unpack=true`: the emulator runs on unpacked digis while the display shows `simMuonCSCDigis` — **Low**
- **Where:** `:144-146`, `:171-173` (emulator inputs → `muonCSCDigis`) vs `:414-416` (display → `simMuonCSCDigis`).
- **Consequence:** The hits drawn are not the emulator's inputs. Packing and unpacking can drop or alter digis (zero suppression, readout windows), so the stubs can look inconsistent with the hits.
- **Fix:** For `mc and unpack`, point the display tags at `muonCSCDigis` too.

### C-10 — Event-list handling — **Low**
- **Where:** `:323-327`, `:333-353`.
- **Consequences:**
  - A missing list file prints "file not found" and calls `exit()`, so the job exits with status 0 and looks like success.
  - An empty or comment-only list leaves `eventsToProcess` empty, which PoolSource treats as "no filter", so all events are processed (and emulated with `l1=true`).
  - The Python parser (stops at `====`, skips non-digit lines) and the C++ parser (`GifDisplay.cc:292-326`, parses every line) disagree; see P-5.
- **Fix:** Use `sys.exit(1)` on errors, refuse to run with an empty list, and use the same parsing rules in both places. Better still, pass the parsed list to the plugin as a `VPSet`/`vstring` instead of a file name.

### C-11 — Cleanup — **Low**
- Unneeded loads: `Reconstruction_cff`, `standAloneMuonSeeds_cff`, `DQMSaverAtRunEnd_cff` (`:79-82`). They slow the start-up.
- `from Configuration.StandardSequences.Eras import eras` (`:61`) is unused.
- `processName = process._Process__name` (`:373`) uses a private attribute; use `process.name_()`.
- `rootFileName` (`:376`) is not read by the plugin.
- `simCscTriggerPrimitiveDigisRun2` (`:182-199`) is defined but never scheduled.
- `debug = 1` is hard-coded (`:402`); it should be an option.
- `output.outputCommands` gets `drop *_rawDataCollector_*_*` twice (`:260`, `:269`).
- The plugin's `fillDescriptions` uses `setUnknown()` (`GifDisplay.cc:1097-1098`), so typos in parameter names, such as those in C-3, are never reported.

---

## 2. Plugin bugs (`GifDisplay/plugins/GifDisplay.cc`)

### P-1 — No validity checks on the CSC digi and data-stub handles — **Medium**
- **Where:** wires `:586-592`, strips `:631-638`, comparators `:706-714`, data ALCT/CLCT/LCT `:794-806` (dereferenced in `fillALCT` `:921`, `fillCLCT` `:961`, `fillCorrLCT` `:1004`). None of these calls are inside a `try`.
- **Consequence:** If any of these products is missing, the job aborts with `ProductNotFound`. Examples: data RAW run without `unpack=true`; files without TPs; a wrong process name. GEM handles are already guarded with `isValid()` (`:759`, `:813`), so behaviour is inconsistent.
- **Fix:** Check `isValid()` and skip that panel with a clear message. Optionally make "required" products configurable.

### P-2 — `addEmulation` is permanently switched off after the first failure (known, open) — **Medium**
- **Where:** `:825-843`; `addEmulation` is a member (`:197`) and is set `false` at `:841`.
- **Consequence:** One event without emulator products (for example the first event before a path ran, or a config error) disables emulation overlays for every later event. The only trace is `" failed to emulated trigger primitives, ignore "`.
- **Fix:** Use a local per-event flag, and check `isValid()` for each handle instead of `try/catch`.

### P-3 — Constructor prints `addEmulation` before reading it (known, open) — **Low**
- **Where:** `:266-268` prints `(addEmulation ? " addEmulation" : " NOEmulation")`; the value is assigned only at `:275`.
- **Consequence:** It reads an uninitialized `bool` (undefined behaviour), so the printout is random.
- **Fix:** Move the print after `:275`.

### P-4 — Run and event are matched independently; the event-list cap is silent — **Medium**
- **Where:** Filter `:376-378` (run in `runList` **and** event in `eventList`, checked separately). Display loop `:853-862` compares only `Event != eventL`; `runList[i]` is never used. `:854` `if (i >= 1000) continue;`.
- **Consequences:**
  - With a list covering several runs, event *N* of run A is drawn with the chamber lines of (run B, event *N*). An event (run B, *N*) is accepted whenever run B and event *N* each appear anywhere in the list. MC lists (all run 1) are unaffected.
  - Lines after the 1000th are ignored without warning.
  - Event numbers are stored as `double` (`:186`, `:303`). This is fine up to 2^53, but it is a type mismatch with `edm::EventNumber_t`.
- **Fix:** Store `edm::EventID` (or `std::pair<RunNumber_t, EventNumber_t>`) per line and match both. Drop the cap, or warn when it is hit.

### P-5 — Event-list parsing: no open check, and header/comment lines become entries — **Low**
- **Where:** `:292-326`.
- **Consequences:**
  - If the file cannot be opened, nothing is drawn and nothing is reported.
  - Every file in `file_lists/` starts with `## run eventnumber ...`. For that line `is >> run` fails (`run=0`), and all later extractions are skipped, so `event`, `endcap`, `station`, `ring` and `chamber` are **uninitialized** values. They are still pushed into the lists (undefined behaviour). Blank lines behave the same way.
- **Fix:** Skip lines starting with `#` or empty lines, check `if (!(is >> run >> event >> ...)) continue;`, and throw a `cms::Exception` if the file cannot be opened.

### P-6 — Geometry lookups are not null-checked (crash on geometry/sample mismatch) — **Medium (Unverified trigger)**
- **Where:** `:409` `cscGeom->layer(sh.detUnitId())->geometry()`, `:479-481` `gemGeom->etaPartition(...)->strip(lp)`.
- **Consequence:** `CSCGeometry::layer()` and `GEMGeometry::etaPartition()` return `nullptr` for a DetId that is not in the loaded geometry. Dereferencing that segfaults. The surrounding `try` catches only `cms::Exception`, which does not help.
- **Plausible trigger:** Phase-2 MC (GE2/1 or ME0 sim hits in `MuonGEMHits`) run with the Run-3 geometry that C-4 now forces on `run3=true`. **Unverified**; to confirm, run on a `Phase2Spring24` file with `mc=true run3=true`.
- **Fix:** Check the pointers and `continue`. Also skip GEM station ≠ 1 if only GE1/1 is displayed.

### P-7 — Misleading stub fields — **Low**
- **Where and consequences:**
  - `:943` `tmplct.pattern = digiIt->getCollisionB()`. The ALCT "Pattern" printed in the plot (`display.cc:272`, `:307-308`) is actually the collision-B bit (0/1). The accelerator bit (`getAccelerator()`) is dropped.
  - `:985` `CC = getCompCode()` returns −1 for legacy CLCTs (`CSCCLCTDigi.h:171`), so the plot prints `CC 0xffffffff`.
  - `:1029-1031` re-implements the eighth-strip calculation by hand. It is correct; `getStrip(8)` gives the same value.
- **Fix:** Store and print accelerator/collisionB explicitly. Print CC only when `isRun3()`. Use `getStrip(8)`.

### P-8 — Unconditional printouts and debug level not forwarded — **Low**
- **Where:**
  - Uncommitted `:873-880` prints container sizes for every displayed chamber, regardless of `doDebug`.
  - `display.cc:692-696` and `:800-804` print a block for **every SimHit**.
  - `:883-888` does not pass `doDebug` to `WireStripDisplay`, which therefore always runs with debug 0.
- Note: the `CLAUDE.md` caveat that "`clct_emul_container.size()` prints the data container size" is outdated. Line 874 now prints the emulator container.
- **Fix:** Guard with `doDebug` and forward `doDebug`.

### P-9 — Performance and framework hygiene — **Low**
- `fillGEMCluster` takes the collection **by value** (`:145`, `:1048`). `gempads = *gempads_.product()` copies the whole collection (`:760`). `findStubsInChamber` takes a `vector` by value (`display.cc:35-36`).
- Per-hit linear search over `simhit_container` (`:441-449`, `:485-493`) is O(N²).
- The class is `one::EDAnalyzer<SharedResources>` but never calls `usesResource(...)` (`:127`). ROOT global state (`gStyle`, `gPad`, `gDirectory`) is used without declaring a shared resource, which is unsafe in multi-threaded jobs.
- The comma-operator chain in the constructor (`:230-244`) works but is fragile.
- Many unused includes and BuildFile dependencies (`L1GlobalMuonTrigger`, `TrackReco`, `RPC`, `MagneticField`, ...).

---

## 3. Implementation bugs (`GifDisplay/interface/*`, `GifDisplay/src/*`)

### I-1 — The CLCT box is one half-strip left of its comparators in all staggered chambers — **High**
- **Where:**
  - `display.cc:1059-1066`: a comparator is drawn at bin `2*(strip-1)+comp+1`, **+1 on layers 1, 3, 5** when `doStagger` (all chambers except ME1/1, `:1553-1555`).
  - `display.cc:986`: CLCT `position = keyStrip + 1`, with **no stagger correction**.
- **Release check:**
  - `CSCCathodeLCTProcessor.cc:524`: internal half-strip = `2*strip + comp + stagger[layer]`, with stagger = 1 on layers 1, 3, 5 (`:84-89`, `:266`).
  - `:1015`: the reported key = `halfstrip_withstagger - stagger[KEY_CLCT_LAYER-1]`. The key layer is 3, which is staggered, so the reported key is the internal centre − 1.
  - The display's comparator frame is "internal + 1", but the CLCT is drawn at "reported + 1" = "internal centre". The box is therefore displaced by −1 half-strip on **every layer**.
- **Consequence:** In ME1/2, ME1/3, ME2/x, ME3/x and ME4/x, every CLCT box (data and emulator) is one half-strip left of the hits that made it. This makes data/emulator comparisons and pattern-shape checks misleading. ME1/1 is correct (no stagger).
- **Confirm:** Display a straight CLCT (pattern 4 / legacy 10) in an ME2/1 event; the layer-3 hit should sit in the box centre.
- **Fix:** In `MakeOneLayerCLCT`, add `+1` to `position` when the chamber is staggered (pass `doStagger`).

### I-2 — Legacy (Run-2-format) CLCTs are drawn with the Run-3 pattern-0 shape — **High**
- **Where:** `display.cc:984-1047`, which dispatches on `track.run3_pattern`. `GifDisplay.cc:986` stores `getRun3Pattern()`.
- **Release check:** `CSCCLCTDigi.h:40` (`run3_pattern = 0` default) and `CSCCLCTDigi.cc:88` (`clear()` sets 0). Legacy CLCTs never set it.
- **Consequence:** Every legacy CLCT is drawn as Run-3 pattern 0, the steepest left-bending envelope, whatever its real pattern (2–10). That covers:
  - In Run-3 (`runCCLUT_TMB=False`, `runCCLUT_OTMB=True`), all chambers read by legacy TMBs: ME1/2, ME1/3, ME2/2, ME3/2, ME4/2. This applies to both data and emulator.
  - Every `run3=false` job.
- **Fix:** Store `isRun3()` (or use `CSCCLCTDigi` directly; see I-23). For legacy CLCTs, draw from `CSCPatternBank::clct_pattern_legacy_[pattern]`. Better, draw both kinds from the pattern bank instead of hard-coded offsets.

### I-3 — Stubs from all BXs are drawn and printed, with no BX selection (known, open) — **Medium**
- **Where:** `MakeOneLayerALCT` / `MakeOneLayerCLCT` loop over every stub (`display.cc:948-970`, `:984-1047`). `findStubsInChamber` does not filter on BX (`:35-49`). The same applies to GEM pads/clusters and the LCT propagation (`displaygem.cc:613`).
- **Consequence:** Out-of-time stubs and pile-up stubs are overlaid as if they belonged to the event. The data and emulator boxes cannot be compared BX by BX. This also feeds I-7.
- **Fix:** Add a BX window parameter (e.g., central BX ± n) and print the BX next to each drawn box.

### I-4 — The SimHit wire-group panel is shifted by +1 WG relative to the wire-digi panel — **Medium**
- **Where:**
  - `display.cc:690`: `bx = wireDisplay->GetXaxis()->FindBin(h.WireGroup)`.
  - `display.cc:980`: `wireDisplay->SetBinContent(w[i].WireGroup, ...)`.
  - Both histograms are `(nWG+1, 0, nWG+1)` (`:221`, `:386`), so bin *b* covers `[b-1, b)`.
- **Consequence:** A SimHit in WG *w* is drawn in bin *w*+1 (`FindBin(w)`). A wire digi in WG *w* is drawn in bin *w*. On every MC plot the SimHit wire markers sit one wire group to the right of the anode hits and the ALCT box.
- **Fix:** Use `SetBinContent`/`GetBinCenter(h.WireGroup)` consistently, or `FindBin(w - 0.5)`.

### I-5 — GE1/1 pads are one pad off relative to the LCT extrapolation boxes — **Medium**
- **Where:**
  - `displaygem.cc:342-344`: `x1 = FindBin(pad); SetBinContent(x1 + 1, ...)`. `padDis` is `(192, 0, 192)`, so pad *p* (0-based, `GEMPadDigi.h:37`) goes to bin *p*+2 = `[p+1, p+2)`.
  - `displaygem.cc:659-664`: LCT windows and unextrapolated pads use `SetBinContent(x + 1, ...)` = `[x, x+1)`.
  - SimHits use the same +1 bin (`:353-357`), i.e. 2 strips.
- **Consequence:** In the same panel, GEM pads appear one pad to the right of the CSC-LCT pad prediction. A matched pair looks off by one; a pad on the window edge looks inside or outside incorrectly.
- **Fix:** Use `SetBinContent(pad + 1, ...)` (or `Fill(pad + 0.5, ...)`) everywhere; the same for SimHits.

### I-6 — "Data" stubs are printed only when emulation is on and there are no SimHits — **Medium**
- **Where:** `display.cc:268` `if (!doSimHit && addEmulation) { ... compareA stubs ... }`.
- **Consequence:**
  - Data with `l1=false`: the text pad shows **no** ALCT/CLCT/LCT information, although data stubs are read and drawn as boxes.
  - All MC jobs: the "Data" (file or first-emulator) stubs are never printed. With `mc=true l1=false` (the setup documented as verified in `CLAUDE.md`), the text pad contains only the chamber line.
- **Fix:** Always print the `compareA` stubs, and print `compareB` only when `addEmulation`.

### I-7 — Fixed-size `stringstream` arrays overflow with more than 10 stubs (or 100 GEM clusters) — **Medium**
- **Where:** `display.cc:266` `ss_alcts[10]`, `:276` `ss_clcts[10]`, `:287` `ss_lcts[10]`, `:303`, `:312`, `:323` (`*_emul[10]`), each indexed by stub number. `displaygem.cc:156` `ss_clusters[100]`, indexed by `iclst`.
- **Consequence:** Because all BXs are kept (I-3), a chamber can exceed 10 stubs of one type. For example, two ALCTs per BX over a 7-BX readout window gives up to 14. The array is then written past its end on the stack (undefined behaviour: corrupted output or a crash).
- **Fix:** Use a local `std::ostringstream` per line (`tex1->AddText(os.str().c_str())`); no arrays are needed.

### I-8 — Global palette side effects: colours differ between the first plot and later plots — **Medium**
- **Where:**
  - `display.cc:73-81`: a `static bool paletteDone` sets an 11-colour palette (`pal[i]=i`, i.e. kWhite, kBlack, kRed, ..., plus transparent grey) **once per job**.
  - `displaygem.cc:35`: `gStyle->SetPalette(55)` on every GEM plot.
  - `SetHistContour` changes `gStyle->SetNumberContours` globally (`display.cc:1280`).
- **Consequences:**
  - After the first GEM plot, every later CSC plot uses palette 55, because `paletteDone` prevents the custom palette being restored. The same quantity (ADC, time bin) is shown with different colour schemes in plots from one job.
  - The custom palette was designed for the SimHit values 1–10 (`MakeOneLayerSimHitDisplay` option 1, now unused) but is applied to the ADC, wire-time and comparator-time panels. Its first colour is white, so the lowest contour bins are probably rendered white, i.e. invisible. **Unverified** (depends on ROOT colour-index mapping); compare the first and second CSC PDF of a job that also makes GEM plots.
- **Fix:** Set the palette explicitly for each pad or histogram (e.g. a `TExec` per pad, or set it at the start of each display function). Do not use static or global state.

### I-9 — The GEM-CSC LUT path is relative to the working directory; a missing LUT crashes the job — **High** (when `doGEMCSC=true` and cwd ≠ repository root)
- **Where:**
  - `displaygem.cc:90`: `string luts_folder = "GifDisplay/GEMCSC/";`.
  - LUT readers `:430-438`, `:470-489`, `:520-532`, `:581-596`; none reports an open failure.
  - `:597-602`: `WG_max.at(i)` / `WG_min.at(i)` on possibly empty vectors.
- **Consequence:** Started from any other directory (batch jobs, or `cmsRun gifDisplay/CSCEventDisplay.py` from `src/`), every LUT open fails silently. The first LCT then reaches `WireToRoll`, which throws `std::out_of_range`, and the job aborts. If only the pad LUTs are missing, the pads are −1 and the boxes are drawn in the underflow bin (invisible) with no warning.
- **Fix:** Use `edm::FileInPath("gifDisplay/GifDisplay/GEMCSC/...")` (or a configurable `lutPath`). Load all LUTs once in the constructor or `beginJob`, and throw a clear `cms::Exception` if a file is missing.

### I-10 — The ADC / time "text" overlays are never filled — **Medium**
- **Where:** `display.cc:583`, `:636`, `:874`, `:1492`, `:1560`, all `if (tempLayer == id.Layer)`. The requested chamber ID is built in `GifDisplay.cc:864-868` without `Layer`, so `id.Layer == 0` and the condition is never true.
- **Consequence:**
  - `stripDis_text`, `wireDis_text` and `comparatorDis_text` (and the SimHit text histograms) are always empty.
  - The strip panel is drawn with `"COLZ"` only (`:209`) plus the empty `stripDis_text` (`:210`), so **no ADC values are printed** on data plots, although clearly intended. Wire and comparator times still appear because `wireDis`/`comparatorDis` themselves are drawn with `"text"`.
  - Latent: if this were fixed, `wireDis_text` and `simhitWireDis_text` have `nWG` bins over `[0, nWG+1)` (`:222`, `:388`). Their bin width is ≠ 1, so the text would be misaligned and two WGs would share a bin.
- **Fix:** Remove the `id.Layer` condition (fill the text histogram for every layer). Give the text histograms the same binning as the main ones, or draw the main histogram with `"COLZ TEXT"`.

### I-11 — ME1a/ME1b boundary bug at eighth-strip 512 — **Low**
- **Where:** `displaygem.cc:619-620`: `ME1a = eighthStrip > 512`, `ME1b = eighthStrip < 512`.
- **Consequence:** For ES = 512 (HS 128, the first ME1a half-strip), both flags are false, the filename stays empty, and the LCT is drawn nowhere. CMSSW uses `keyStrip > MAX_HALF_STRIP_ME1B` (127), i.e. ES ≥ 512 (`CSCGEMMatcher.cc:247`).
- **Fix:** Use `ME1a = eighthStrip >= 512; ME1b = !ME1a;`.

### I-12 — LUT reading robustness and performance — **Low**
- **Where:** `displaygem.cc:376-604`.
- Each LCT opens up to 7 files per GEM layer (slope, 3× `EighthStripToPad`, 2× roll files). There is no caching.
- `line.at(0)` throws on a blank line. The current files have no blank lines, but this is fragile.
- `EighthStripToPad` uses `es != 0` as a "first line" sentinel (`:477`), which is wrong if a LUT entry is 0.
- Parsing uses `substr`/`find`/`atoi` with no error checks.
- `HalfStripToPad` is unused.
- **Fix:** Load each LUT once into `std::vector<int>` (or use CMSSW's `CSCL1TPLookupTableME11ILT` via the EventSetup, which the emulator already uses).

### I-13 — `SetHistContour` level mapping merges time bins 0 and 1 — **Low**
- **Where:** `display.cc:1279-1294`. For (0, 16): `levels = {0, 1.067, 2.13, …, 16}`.
- **Consequence:** Time bin 0 (stored as 0.1) and time bin 1 fall in the same contour (same colour). Integer bins map to 16 levels non-uniformly. For (1, 500) there are 499 levels, which with a discrete palette gives coarse colour steps.
- **Fix:** Use `levels[i] = Min + i` for integer quantities, or `SetContour(N)` with `SetMinimum`/`SetMaximum`.

### I-14 — Memory leaks and ROOT name collisions — **Low**
- **Where (CSC):**
  - `display.cc:85-86`: `NWireGroup`/`NStrip` `TH2F` are created on every call and never deleted (they also leak on the early return at `:114-115`).
  - `:191-199`: `cfebNotInstall_*` and `cfebNotReadOut_comparator` are never deleted.
  - TPaveTexts `pt1`–`pt4` are never deleted.
- **Where (GEM):** `displaygem.cc:80`, `:91-95`, `:124` (`pt1`, `extrapLCT`, `unextrapLCT`, `LCTLegend`, `pt2`) are never deleted. `:123` creates `simhitDis` with the name **"padDis"**, the same as `:79`, which produces ROOT "Replacing existing TH1" warnings.
- **Consequence:** Memory grows with the number of plots, and "Replacing existing TH1: …" warnings appear for every chamber after the first.
- **Fix:** Use `std::unique_ptr`/stack objects or `SetDirectory(nullptr)`, use unique names, and use a static lookup table instead of histograms for NStrip/NWG.

### I-15 — VLAs and no validation of the requested chamber ID — **Low**
- **Where:** `display.cc:148-150`, `:202` (`double cfeb[nCFEB] = {}`), `:1286` (`Double_t levels[nLevels]`); `CountCFEB` `:1169-1172`.
- **Consequence:** VLAs are a non-standard GCC extension. For a station/ring not in the table (e.g. ring 3 in stations 2–4, typos), `nStrip = 0` gives a zero-length VLA, and `CountCFEB` writes out of bounds (undefined behaviour). Histograms with zero range follow. `SetSaveNameLegendName` leaves `name` empty for an endcap other than 1/2, so the plot is saved as `".pdf"`.
- **Fix:** Validate the event-list ID (e.g. build a `CSCDetId` and check it against the geometry), and use `std::vector<double>`.

### I-16 — Duplicate-chamber bookkeeping is dead code — **Low**
- **Where:** `SaveUsedChamber` (`display.cc:534-548`) is never called, so `ChamberUsedForEventDisplay` (`:114`, `:488-504`) is always false. `usedChamber`/`usedGEMChamber` are never filled. `GEMChamberUsedForEventDisplay` is declared (`display.h:48`) but never defined. The comment at `:115` is inverted.
- **Consequence:** Duplicate lines in the event list redraw and overwrite the same files.
- **Fix:** Remove it, or implement it with a `std::set<CSCDetId>` per event.

### I-17 — ALCT/CLCT boxes appear only on layers that have hits — **Low**
- **Where:** `display.cc:1495` (`MakeOneLayerALCT` inside the wire-layer loop), `:1564` (`MakeOneLayerCLCT` inside the comparator-layer loop).
- **Consequence:**
  - A stub is drawn only on layers that have at least one wire or comparator hit **anywhere** in the chamber. If the chamber has no hits, no box is drawn at all, even if the stub exists.
  - For ME1/1, a layer with both ME1b (ring 1) and ME1a (ring 4) comparator entries draws the CLCT twice (harmless).
- **Fix:** Draw stubs in a separate loop over layers 1–6.

### I-18 — In MC, the strip panel shows SimHits instead of strip digis — **Low** (design question)
- **Where:** `display.cc:179-217` (`if (doSimHit) SimHitDisplay(...) else StripDisplay(...)`).
- **Consequence:** MC plots never show strip ADCs or CFEB read-out shading, and the "SimHit Strips" panel replaces the "Strip Hits and ADC Count" panel. There is a separate SimHit wire panel, but no separate SimHit strip panel.
- **Fix:** Use a 6th pad for SimHit strips, or overlay SimHits on the strip-digi panel.

### I-19 — Only the first time bin is shown, and later digis overwrite earlier ones — **Low**
- **Where:** `GifDisplay.cc:610`, `:738` (`getTimeBin()` = first bin ON, `CSCComparatorDigi.cc:51-60`); `display.cc:980`, `:1062`/`:1066` (`SetBinContent` overwrites).
- **Consequence:** Multiple comparator or wire digis on the same half-strip/WG in different BXs are reduced to whichever comes last in the collection.
- **Fix:** Show the earliest one, or mark multiplicity.

### I-20 — The GEM pad histogram is always empty for data (known, open) — **Low**
- **Where:** `displaygem.cc:79-87` plus the config. Data has no `GEMPadDigi`; OTMB clusters appear only as text lines (`:156-177`).
- **Fix:** Draw `GEMPadDigiCluster` pads (`cluster.pads()`) in the pad histogram when pads are absent.

### I-21 — GE2/1 is never displayed although `runME21ILT` turns the GEM display on — **Low**
- **Where:** `GifDisplay.cc:890` (`stationL == 1 and ringL == 1` only); `CSCEventDisplay.py:400`. The ME21 LUTs ship in `GEMCSC/` but are unused; `NRoll=8`/`NPad=192` are hard-coded for GE1/1 (`displaygem.cc:36-38`).
- **Fix:** Document that only GE1/1 is supported, or generalise.

### I-22 — Dead or misleading code — **Low**
- `StripHitDisplay`, `MakeComparatorHitNotReadout` (hard-codes 80 strips, `:1143-1148`), `MakeStripHit`/`MakeHit1-3`/`FindRatio` (possible division by zero, `:1432-1433`), `SetPlotDetail_StripHit` and `HalfStripToPad` are only referenced from commented-out code.
- `MakeShadeForComparatorPanel` / `BlockNotInstalledCFEB` fill histograms that are never drawn (`:205-207`, `:212-216`, `:446-454`).
- `MakeOneLayerSimHitDisplay` option 1 is unused.
- `WireDisplay` `if (id.Ring == 4) id.Ring = 1;` (`:1482`) has no effect (local copy, not used afterwards).
- `srand(time(NULL))` (`display.cc:119`, `displaygem.cc:53`) is called without any `rand()`.
- `legendName` is computed and never used.
- `ComparatorNumber += 128` for ME1a (`GifDisplay.cc:740`) has no effect because the only use is `% 2` (`display.cc:1059`).
- Headers use `using namespace std;` and `#define PI` (`display.h:23-24`, `structForDisplay.h:13`).

### I-23 — Design: custom structs instead of CMSSW classes — **Medium**
- **Where:** `GifDisplay/interface/structForDisplay.h` (whole file) and the `pair<ID, vector<...>>` typedefs (`display.h:26-34`); the copy-in code in `GifDisplay.cc:398-1079`; the lookups `FindChamberIndex` (`display.cc:1464-1477`), `findStubsInChamber` (`:35-49`), `findClustersInChamber` (`displaygem.cc:361-374`).
- **What:** IDs and primitives are copied into hand-made structs: `CSCDetID`/`GEMDetID` (all fields `double`, compared with `doubleEqual`), `Wire`, `Strips`, `Comparator`, `GEMPad`, `GEMPadCluster`, a single `CorrelatedLCT` used for ALCT, CLCT and LCT alike, `SimHit` (shared by CSC and GEM), and `SIMTRACK`. Chamber lookup is a linear search over `vector<pair<ID, vector<>>>`. The package already links `DataFormats/MuonDetId`, `CSCDigi` and `GEMDigi`, and nothing outside it uses the header.
- **Bugs it caused or invites (concrete):**
  - `CSCDetID::operator==` ignored `Ring`. Stubs from ME1/2/N and ME1/3/N were drawn on ME1/1/N (fixed in the working tree).
  - GE-minus naming checked `Endcap == 2`, the CSC convention, while GEM IDs use region −1 (fixed). `GEMPadDisplay` still mixes conventions: `gemId.Endcap = endcapL == 1 ? 1 : -1` (`GifDisplay.cc:895`).
  - `GEMDetID::operator==` ignores `Ring` (`structForDisplay.h:60-66`). This is harmless today but is the same latent bug.
  - Loss of information: `isRun3()` is not stored, which causes I-2. ALCT `pattern` is the collision-B bit (P-7). `GEMPadCluster::nRolls` is declared but never filled, so clusters spanning two rolls (`nPartitions()`) are not handled. Legacy `CC = -1` is printed as hex.
  - Manual re-implementations of CMSSW logic: ME1a strip +64 and comparator +128 (`GifDisplay.cc:656-657`, `:736-740`); half-strip `2*(strip-1)+comp` (`display.cc:1059`, equal to `CSCComparatorDigi::getHalfStrip()`); eighth strip (`GifDisplay.cc:1029-1031`, equal to `getStrip(8)`).
  - Every lookup is O(N) over all chambers, plus copies by value (P-9).
- **Suggested fix (incremental):**
  1. `CSCDetID` → `CSCDetId`, `GEMDetID` → `GEMDetId` (use `chamberId()`/`superChamberId()` for comparisons).
  2. Pass the digi collections (or `MuonDigiCollection::get(detId)` ranges) to the display functions instead of the vectors.
  3. Use `CSCALCTDigi`/`CSCCLCTDigi`/`CSCCorrelatedLCTDigi`, `GEMPadDigi`, `GEMPadDigiCluster`, `CSCComparatorDigi` (`getHalfStrip()`, `getTimeBinsOn()`).
  4. Keep small helper structs only for derived quantities (strip ADC sums/max, SimHit WG/strip from geometry).
  5. Watch ME1/1: ME1a is ring 4 for strips/comparators, but wires and stubs are stored under ME1b (ring 1) with stub HS 128–223 for ME1a.

---

## 4. Strange / unexplained actions (questions for the original authors)

Each item lists: what the code does, where it is, why it is suspicious or unclear, and the question to ask.

### S-1 — BX offsets on emulated ALCT/CLCT text only
- **What and where:** `alct_bx_offset = 5`, `clct_bx_offset = 1` (`display.cc:61-62`). They are added to the **printed** BX of emulated ALCTs (`:309`) and CLCTs (`:320`), but not to emulated LCTs (`:330`), not to data stubs (`:273`, `:284`), and not to the drawn boxes.
- **Why suspicious:** These are presumably the data-vs-emulator BX convention differences. The LCT is left uncorrected, so the printed ALCT/CLCT/LCT BXs of one emulated stub disagree. In MC with `l1=true`, both sides are emulators, but only "Emul" gets the shift.
- **Question:** Where do 5 and 1 come from (TMB/ALCT readout-window settings? `alctL1aWindowWidth`, `tmbL1aWindowSize`)? Should they apply to LCTs? Should they depend on the stub source rather than the column?

### S-2 — GEM "nominalBX = 8"
- **What and where:** `displaygem.cc:152` prints `nominalBX=8(<-0)`. Cluster BX is printed as `BX + 8` (`:172`). Pad colour is `TimeBin + 8` (`:336`), and `if (time == 0)` (`:337`) only catches bx = −8.
- **Why suspicious:** This maps emulator GEM BX 0 to the CSC central BX 8. The same +8 is applied to OTMB-readout clusters from `muonCSCDigis`, whose BX convention may differ (**Unverified**). Pads with bx < −8 get negative content and disappear.
- **Question:** Is GEM BX 0 always "nominal" for every source (emulator, OTMB unpacker, EMTF)? Should the shift depend on the source?

### S-3 — Per-layer CLCT position shifts in `MakeOneLayerCLCT`
- **What and where:** Hard-coded offsets per `run3_pattern` and layer (`display.cc:984-1047`).
- **Check:** For Run-3 patterns 0–4 the shifts match `CSCPatternBank::clct_pattern_run3_` (`CSCPatternBank.cc:141-181`) exactly, assuming pattern row *i* = layer *i*+1.
- **Why suspicious:** The `else` branch (pattern 4 and anything else) is a ±1 box. Legacy patterns are not handled (I-2). The stagger correction is missing (I-1).
- **Question:** Was this checked against the firmware pattern orientation (row ↔ layer, and left/right per endcap)? Why not draw directly from `CSCPatternBank`?

### S-4 — ALCT box envelope
- **What and where:** `MakeOneLayerALCT` draws ±2 WG on layers 1 and 6, ±1 on layers 2 and 5, and 0 on layers 3 and 4 (`display.cc:948-970`).
- **Why suspicious:** The ALCT collision pattern (`CSCPatternBank.cc:15-16`) is asymmetric: layer 1 −2..0, layer 2 −1..0, layer 3 0, layer 4 0..+1, layers 5–6 0..+2, mirrored for ME3/ME4 by `alct_keywire_offset_`. The drawn envelope misses +1 on layer 4 and +2 on layer 5, and adds cells that are not in the pattern. The accelerator pattern (straight) is drawn the same way.
- **Question:** Is the symmetric envelope intentional (covering both mirror images), or should it follow the real pattern per station?

### S-5 — Comparator half-strip formula and stagger direction
- **What and where:** `2*(Strip-1) + ComparatorNumber%2 + 1` (`display.cc:1059`). This equals `CSCComparatorDigi::getHalfStrip() + 1` (`CSCComparatorDigi.cc:76`). It adds +1 on layers 1, 3, 5 in non-ME1/1 chambers (`:1061-1062`). Strips (`:921-929`) and SimHits (`:796-797`) get +½ strip on the same layers.
- **Why suspicious:** The stagger direction agrees with the emulator's internal frame, but the stub drawing does not follow it (I-1). The `%2` exists only to undo the ME1a `+128` hack (S-6).
- **Question:** Was the stagger direction verified against data (e.g. the firmware "odd layers −1" convention mentioned in `CSCCathodeLCTProcessor.cc:255-263`)?

### S-6 — ME1a offsets
- **What and where:** Strip +64 (`GifDisplay.cc:656-657`, comment "BECAUSE GIF ME11A ONLY HAS dcfeb 1,5,7"), comparator strip +64 and comparator number +128 (`:736-740`), SimHit strip +64 (`:412-413`).
- **Why suspicious:** +64 is right for unganged ME1a (ME1b has 64 strips) and matches the emulator's ME1/1 frame (HS 128–223 for ME1a). The comment refers to a GIF test-beam setup and is misleading. The +128 has no effect.
- **Question:** Is the comment outdated? Can the +128 be removed?

### S-7 — Strip ADC processing
- **What and where:** The pedestal is the mean of the first two samples. The peak search uses samples 2–7 with a hard threshold `13.3` ADC (`GifDisplay.cc:662-681`). `adcTotal` is an `int` that accumulates `double`s (truncation). The colour range is 1–500 (`display.cc:884`, `:898`). The unused `MakeHit*` code has further thresholds (`MaxADC >= 25`, peak time 3–6).
- **Question:** Where does 13.3 come from (≈ 3σ noise?) Should the threshold depend on the chamber or on the CSC noise conditions (`CSCDBNoiseMatrix` is included but not used)?

### S-8 — The "time = 0 → 0.1" trick
- **What and where:** `display.cc:976-978`, `:1055-1057`; `displaygem.cc:337-339`.
- **Why suspicious:** It exists so that time bin 0 is distinguishable from an empty bin, but with the contour levels of I-13 it gets the same colour as time bin 1.
- **Question:** Would an explicit "empty" value (e.g. −1 with `SetMinimum(-0.5)`) be cleaner?

### S-9 — `Run3CCLUTv0` emulator-clone parameter choices
- **What and where:** `CSCEventDisplay.py:202-228`:
  - `clctPhase2.clctNplanesHitPattern = 3` (default 4)
  - `clctStateMachineZone = 4` (default 5)
  - `clctPretriggerTriggerZone = 224` (= default)
  - local-shower zone/threshold 20/10 (defaults 25/12)
  - `sortClctBx = False` (default True)
  - `runME11ILT = True` forced
  - verbosity 10 / 5
  - comment "?no deadtime zone", while the line that would disable `useDeadTimeZoning` is commented out (`:229`), so dead-time zoning stays **on** (`clctParams.py:44`).
- **Why suspicious:** This "Emul" is a custom emulator variant. Its output is shown as "Emul" in MC plots without any indication.
- **Question:** What study were these values for? Are they still wanted? The name says "no deadtime zone", but the setting does the opposite.

### S-10 — `tmbReadoutEarliest2 = False` on the main emulator
- **What and where:** `CSCEventDisplay.py:168-169`, set only on `tmbPhase2` and `tmbPhase2GE11` (default True, `tmbParams.py:32`).
- **Why suspicious:** The emulator reads out more LCTs than the default and than the firmware may, so data-vs-emulator LCT counts can differ by construction.
- **Question:** Is this intentional for the display? Should `tmbPhase1` (legacy TMB chambers) be treated the same?

### S-11 — Dropping the CSC shower collection from the input
- **What and where:** `CSCEventDisplay.py:107-110` drops `CSCDetIdCSCShowerDigiMuonDigiCollection_simCscTriggerPrimitiveDigis_*_*`.
- **Why suspicious:** Probably a workaround for a schema/dictionary change in old files, but it also drops showers from new files.
- **Question:** Which file or release needed this? Is it still required in 16_1_0?

### S-12 — GEM-CSC extrapolation constants
- **What and where:** Match window 20/40 eighth-strips for even/odd chambers (`displaygem.cc:640-641`). This equals 4 × `maxDeltaHsEven/Odd` (5/10, `tmbParams.py:94-95`), but is duplicated by hand. The window is drawn on three rolls, `ybin-3`, `ybin`, `ybin+3` (`:659-661`), while CMSSW uses `maxDeltaWG = 7`. The sign `(2*bend-1)` agrees with `CSCGEMMatcher.cc:260-263`. The even/odd `x_min`/`x_max` swap (`:651-657`) assumes the LUT direction. Not applied: the shipped `AlignmentCorrection/*.txt` LUTs and the COSI slope mitigation (`BendingAngle/*`). The LCT slope is used, not the CLCT slope.
- **Question:** Should the window and corrections be read from the emulator configuration or EventSetup LUTs so that they stay in sync with what the emulator actually did? Why ±1 roll instead of the WG window?

### S-13 — Axis conventions differ between panels
- **What and where:** Wires: WG *w* (1-based) is drawn in `[w-1, w)`, i.e. a 0-based axis matching ALCT `keyWG` (`display.cc:386`, `:980`, `:950`). Strips: strip *s* is drawn in `[s, s+1)` (1-based axis, `:173`, `:917`). Comparators: 0-based half-strip axis (`:421`, `:1059`). `stripDis_text` uses y-range `[0.5, 6.5)` while `stripDis` uses `[1, 7)` (`:173-175`).
- **Question:** Which convention should the axis labels show? Today a reader of the wire panel sees WG *w* between ticks *w*−1 and *w*.

### S-14 — Hard-coded NStrip/NWireGroup tables
- **What and where:** `display.cc:85-112`.
- **Check:** The values are correct for the real chambers: ME1/1 112 strips (64+48 unganged) / 48 WG; ME1/2 80/64; ME1/3 64/32; ME2/1 80/112; ME3/1 and ME4/1 80/96; MEX/2 80/64. Commented-out alternatives (48 WG for ME1/2, 64 or 48 strips for ME1/1 or ring 4) remain.
- **Question:** Why not take the numbers from `CSCGeometry` (`numberOfStrips()`, `numberOfWireGroups()`)?

### S-15 — Commented-out alternative paths that change behaviour
- **Where:**
  - CFEB "not installed" shading (`display.cc:212-216`, `:446-454`)
  - Strip-hit TGraph panel (`:352-379`)
  - Alternative stub tags (`CSCEventDisplay.py:417-430`) and `doGEMDisplay` variants (`:398-400`)
  - `simMuOnly_ = false` hard-coded (`GifDisplay.cc:388`)
  - `// if (Event < 500)` (`:849-851`)
- **Question:** Which of these are still wanted? They make the behaviour depend on uncommented edits.

### S-16 — SimHit decoration choices and GEM SimHit strip rounding
- **What and where:**
  - SimHit colour = `1 + |TrackID| % 9` (`display.cc:720`, `:819`). Unrelated tracks can share a colour.
  - Vertical fan-out offsets (`:673`, `:748`), marker y-offset `+0.35`.
  - Text colour set and then overwritten with semi-transparent black (`:735-737`, `:836-838`).
  - GEM SimHit strip = `int(strip(lp) + 0.5)` (`GifDisplay.cc:481`). `GEMEtaPartition::strip()` returns a continuous position from 0, so rounding moves hits in the upper half of a strip to the next strip; `floor` would give the strip index.
- **Question:** Is the rounding intended? Should GEM SimHits be converted to pads with `padOfStrip`/`pad(lp)` so they can be compared with pads?

---

## 5. Appendix — other files (brief)

- **`CSCEventDisplay_realdata.py`**
  - It does not define `simTrackTagSrc`. The current plugin reads it with `getUntrackedParameter` and no default (`GifDisplay.cc:234-235`), so **construction throws**: this config cannot run with the current plugin.
  - It still sets `labelRawDataLikeMC=False` for any `unpackGEM` (`:107-108`), which is the segfault fixed in the main config.
  - It uses GT `140X_dataRun3_Prompt_v2` and `GeometryRecoDB_cff`.
  - `gemPadTagSrc`/`gemPadClusterTagSrc` are `""`, and `compareA`/`compareB`/`doGEMCSC` are missing (defaults apply).
  - Suggest deleting it or regenerating it from the main config.
- **`GifDisplay/python/`**
  - `GifDisplay_cfi.py` uses obsolete or tracked parameter names (`alctDigiTag`, `corrlctDigiTag`, `cscRecHitTag`, `chamberType`) and lacks the required untracked tags, so the plugin would throw.
  - `ConfFile_cfg.py` defines `process.Display` but puts `process.demo` in the path.
  - `__init__.py` hard-codes an `slc6_amd64_gcc530` path.
  - All three are stale.
- **`dataextract_run_csc_extraction.sh`** calls `extract_csc_info.py`, `simple_csc_extract.C` and `extractCSCInfo.C`, but the files are named `dataextract_*`, so every option fails. `dataextract_csc_info.py` always returns `None` and writes nothing. The `.C` macros only list branches, and use guessed branch names (e.g. `muonCSCDigis_MuonCSCDigiCollection__RECO`) that do not exist.
- **`runDisplayYumeng.sh` / README examples** run Phase-2 DY samples with `run3=true`, which C-4 makes inconsistent.
- **`CLAUDE.md` "Current state"** says `run_test.sh` runs MC. The working-tree `run_test.sh` runs data RAW run 403775 with `l1=true unpack=true unpackGEM=false runME11ILT=true`. The note about the `clct_emul_container` printout is also outdated (P-8).
- **Repository hygiene:** untracked `c1.pdf`, `c1.png` and `cscdisplay_edmout.root` in the repository root; there is no `.gitignore` entry for them.

## 6. Already fixed (working tree; not reported as open)

- `labelRawDataLikeMC=False` is now set only for B904 data (`CSCEventDisplay.py:115-116`).
- `CSCDetID::operator==` now compares `Ring` (`structForDisplay.h:24-30`).
- The empty SimHit wire pad for data was removed; pads are now named `padSimHitWire`/`padWire`/`padStrip`/`padComparator`/`padText` (`display.cc:154-161`).
- GEM pad and cluster handles are checked with `isValid()` (`GifDisplay.cc:758-763`, `:813-817`). Data without `l1GEM` uses `muonCSCDigis:MuonGEMPadDigiCluster` (`CSCEventDisplay.py:450-451`). GEM pad producers are scheduled only for MC or `l1GEM` (`:481-482`); see C-1 for the remaining hole.
- MC with `l1=false` uses `simCscTriggerPrimitiveDigis` from the file (`:439-442`).
- The data GT is applied only to data (`:140-141`). Run-3 MC uses `auto:phase1_2022_realistic` (`:128`).
- GE-minus file naming now checks `Endcap == -1` (`displaygem.cc:205`).
- Event-number overflow: `edm::RunNumber_t`/`edm::EventNumber_t` are used in the plugin and the display functions (`GifDisplay.cc:352-355`, `display.h:44`, `:50`, `:133`, `:136`). The event-list storage is still `double` (P-4).
- The container-size debug printout now prints the emulator container (`GifDisplay.cc:873-880`); it is unconditional (P-8).
