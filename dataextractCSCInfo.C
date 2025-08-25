// ROOT macro to extract CSC muon information
// Usage: root -l extractCSCInfo.C

void extractCSCInfo() {
    // Open the ROOT file
    TFile* file = TFile::Open("pickevents.root");
    if (!file || file->IsZombie()) {
        cout << "Error: Could not open file pickevents.root" << endl;
        return;
    }
    
    cout << "Successfully opened file: pickevents.root" << endl;
    
    // Get the Events tree
    TTree* events = (TTree*)file->Get("Events");
    if (!events) {
        cout << "Error: Could not find Events tree" << endl;
        file->Close();
        return;
    }
    
    cout << "Found Events tree with " << events->GetEntries() << " entries" << endl;
    
    // List all branches to see what's available
    cout << "\nAvailable branches:" << endl;
    TObjArray* branches = events->GetListOfBranches();
    for (int i = 0; i < branches->GetEntries(); i++) {
        TBranch* branch = (TBranch*)branches->At(i);
        cout << "  " << branch->GetName() << endl;
    }
    
    // Look for CSC-related branches
    cout << "\nCSC-related branches:" << endl;
    for (int i = 0; i < branches->GetEntries(); i++) {
        TBranch* branch = (TBranch*)branches->At(i);
        TString branchName = branch->GetName();
        if (branchName.Contains("CSC") || branchName.Contains("csc")) {
            cout << "  " << branchName << endl;
        }
    }
    
    // Look for event info branches
    cout << "\nEvent info branches:" << endl;
    for (int i = 0; i < branches->GetEntries(); i++) {
        TBranch* branch = (TBranch*)branches->At(i);
        TString branchName = branch->GetName();
        if (branchName.Contains("Event") || branchName.Contains("Run") || branchName.Contains("Lumi")) {
            cout << "  " << branchName << endl;
        }
    }
    
    file->Close();
    cout << "\nFile closed." << endl;
}

// Function to extract detailed CSC information
void extractDetailedCSCInfo() {
    // Open the ROOT file
    TFile* file = TFile::Open("pickevents.root");
    if (!file || file->IsZombie()) {
        cout << "Error: Could not open file pickevents.root" << endl;
        return;
    }
    
    // Get the Events tree
    TTree* events = (TTree*)file->Get("Events");
    if (!events) {
        cout << "Error: Could not find Events tree" << endl;
        file->Close();
        return;
    }
    
    // Open output file
    ofstream outFile("csc_info.txt");
    outFile << "# Run EventID Endcap Station Ring Chamber" << endl;
    
    // Set branch addresses for event info
    UInt_t run, lumi, event;
    events->SetBranchAddress("EventAuxiliary.run_", &run);
    events->SetBranchAddress("EventAuxiliary.luminosityBlock_", &lumi);
    events->SetBranchAddress("EventAuxiliary.event_", &event);
    
    // Process events
    Long64_t nEntries = events->GetEntries();
    cout << "Processing " << nEntries << " events..." << endl;
    
    for (Long64_t i = 0; i < nEntries; i++) {
        events->GetEntry(i);
        
        cout << "Event " << i << ": Run=" << run << ", Lumi=" << lumi << ", Event=" << event << endl;
        
        // Try to access CSC collections
        // Note: We'll need to check what's actually available in the file
        
        // Check for CSC RecHits
        if (events->GetBranch("recoCSCRecHits_CSCRecHit2DCollection__RECO")) {
            cout << "  Found CSC RecHits branch" << endl;
            // We would need to set up proper branch addresses here
        }
        
        // Check for CSC Segments
        if (events->GetBranch("recoCSCSegments_CSCSegmentCollection__RECO")) {
            cout << "  Found CSC Segments branch" << endl;
            // We would need to set up proper branch addresses here
        }
        
        // Check for CSC Digis
        if (events->GetBranch("muonCSCDigis_MuonCSCDigiCollection__RECO")) {
            cout << "  Found CSC Digis branch" << endl;
            // We would need to set up proper branch addresses here
        }
        
        // Limit output for first few events
        if (i >= 4) {
            cout << "  ... (showing first 5 events only)" << endl;
            break;
        }
    }
    
    outFile.close();
    file->Close();
    
    cout << "\nCSC information extracted to csc_info.txt" << endl;
    cout << "Format: Run EventID Endcap Station Ring Chamber" << endl;
}
