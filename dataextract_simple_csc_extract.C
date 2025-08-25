// Simple ROOT macro to extract CSC information
// Run this in ROOT with: .x simple_csc_extract.C

void simple_csc_extract() {
    // Since you already have the file open, let's work with it
    TFile* file = gFile;
    if (!file) {
        cout << "No file open. Please open pickevents.root first." << endl;
        return;
    }
    
    cout << "Working with file: " << file->GetName() << endl;
    
    // Get the Events tree
    TTree* events = (TTree*)file->Get("Events");
    if (!events) {
        cout << "No Events tree found" << endl;
        return;
    }
    
    cout << "Events tree has " << events->GetEntries() << " entries" << endl;
    
    // List all branches
    cout << "\nAll branches:" << endl;
    TObjArray* branches = events->GetListOfBranches();
    for (int i = 0; i < branches->GetEntries(); i++) {
        TBranch* branch = (TBranch*)branches->At(i);
        cout << "  " << branch->GetName() << endl;
    }
    
    // Look for CSC branches
    cout << "\nCSC-related branches:" << endl;
    for (int i = 0; i < branches->GetEntries(); i++) {
        TBranch* branch = (TBranch*)branches->At(i);
        TString name = branch->GetName();
        if (name.Contains("CSC") || name.Contains("csc")) {
            cout << "  " << name << endl;
        }
    }
    
    // Look for event info branches
    cout << "\nEvent info branches:" << endl;
    for (int i = 0; i < branches->GetEntries(); i++) {
        TBranch* branch = (TBranch*)branches->At(i);
        TString name = branch->GetName();
        if (name.Contains("Event") || name.Contains("Run") || name.Contains("Lumi")) {
            cout << "  " << name << endl;
        }
    }
    
    cout << "\nDone listing branches." << endl;
    cout << "Now you can examine specific branches manually." << endl;
}
