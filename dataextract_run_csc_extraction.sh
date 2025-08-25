#!/bin/bash

echo "CSC Muon Information Extraction Script"
echo "======================================"
echo ""

# Check if pickevents.root exists
if [ ! -f "pickevents.root" ]; then
    echo "Error: pickevents.root not found in current directory"
    echo "Please make sure you're in the correct directory"
    exit 1
fi

echo "Found pickevents.root file"
echo ""

# Option 1: Run Python script (if uproot is available)
echo "Option 1: Python extraction (requires uproot)"
if command -v python3 &> /dev/null; then
    echo "Python3 found. Checking for uproot..."
    if python3 -c "import uproot" &> /dev/null; then
        echo "uproot available. Running Python extraction..."
        python3 extract_csc_info.py
    else
        echo "uproot not available. Install with: pip install uproot"
    fi
else
    echo "Python3 not found"
fi

echo ""

# Option 2: ROOT macro
echo "Option 2: ROOT macro extraction"
echo "To use this option:"
echo "1. Open ROOT: root -l pickevents.root"
echo "2. Run the macro: .x simple_csc_extract.C"
echo "3. Or run the detailed extraction: .x extractCSCInfo.C"
echo ""

# Option 3: Manual ROOT commands
echo "Option 3: Manual ROOT commands"
echo "In ROOT, you can run these commands manually:"
echo ""
echo "// List all branches"
echo "Events->GetListOfBranches()->Print()"
echo ""
echo "// Look for CSC branches"
echo "Events->GetListOfBranches()->Print(\"*CSC*\")"
echo ""
echo "// Look for event info branches"
echo "Events->GetListOfBranches()->Print(\"*Event*\")"
echo "Events->GetListOfBranches()->Print(\"*Run*\")"
echo "Events->GetListOfBranches()->Print(\"*Lumi*\")"
echo ""
echo "// Check specific branch contents"
echo "Events->Scan(\"EventAuxiliary.run_:EventAuxiliary.event_\")"
echo ""

echo "Choose the option that works best for your environment."
echo "The Python script (Option 1) is recommended if uproot is available."
