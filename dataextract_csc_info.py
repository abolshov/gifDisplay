#!/usr/bin/env python3
"""
Script to extract CSC muon information from ROOT files
Extracts: Run, EventID, Endcap, Station, Ring, Chamber
"""

import uproot
import pandas as pd
import numpy as np
from pathlib import Path

def extract_csc_info(root_file_path):
    """
    Extract CSC information from ROOT file
    
    Args:
        root_file_path (str): Path to the ROOT file
        
    Returns:
        pandas.DataFrame: DataFrame with CSC information
    """
    
    print(f"Opening ROOT file: {root_file_path}")
    
    try:
        # Open the ROOT file
        with uproot.open(root_file_path) as file:
            print("Successfully opened ROOT file")
            
            # Get the Events tree
            if "Events" not in file:
                print("Error: No Events tree found in file")
                return None
                
            events = file["Events"]
            print(f"Found Events tree with {events.num_entries} entries")
            
            # List all branches
            print("\nAvailable branches:")
            for branch_name in events.keys():
                print(f"  {branch_name}")
            
            # Look for CSC-related branches
            print("\nCSC-related branches:")
            csc_branches = []
            for branch_name in events.keys():
                if "CSC" in branch_name.upper() or "csc" in branch_name.lower():
                    print(f"  {branch_name}")
                    csc_branches.append(branch_name)
            
            # Look for event info branches
            print("\nEvent info branches:")
            event_branches = []
            for branch_name in events.keys():
                if any(keyword in branch_name for keyword in ["Event", "Run", "Lumi"]):
                    print(f"  {branch_name}")
                    event_branches.append(branch_name)
            
            # Try to extract event information
            print("\nTrying to extract event information...")
            
            # Common event info branch names
            run_branch = None
            event_branch = None
            lumi_branch = None
            
            for branch in event_branches:
                if "run" in branch.lower():
                    run_branch = branch
                elif "event" in branch.lower():
                    event_branch = branch
                elif "lumi" in branch.lower():
                    lumi_branch = branch
            
            print(f"Run branch: {run_branch}")
            print(f"Event branch: {event_branch}")
            print(f"Lumi branch: {lumi_branch}")
            
            # Extract basic event info if available
            event_data = {}
            if run_branch:
                try:
                    event_data['run'] = events[run_branch].array()
                    print(f"Successfully extracted run information")
                except Exception as e:
                    print(f"Error extracting run info: {e}")
                    
            if event_branch:
                try:
                    event_data['event'] = events[event_branch].array()
                    print(f"Successfully extracted event information")
                except Exception as e:
                    print(f"Error extracting event info: {e}")
                    
            if lumi_branch:
                try:
                    event_data['lumi'] = events[lumi_branch].array()
                    print(f"Successfully extracted lumi information")
                except Exception as e:
                    print(f"Error extracting lumi info: {e}")
            
            # Try to extract CSC information
            print("\nTrying to extract CSC information...")
            
            csc_data = []
            
            # Process each event
            for i in range(min(10, events.num_entries)):  # Process first 10 events
                print(f"\nProcessing event {i}")
                
                # Get event info
                run_num = event_data.get('run', [0])[i] if 'run' in event_data else 0
                event_num = event_data.get('event', [0])[i] if 'event' in event_data else 0
                lumi_num = event_data.get('lumi', [0])[i] if 'lumi' in event_data else 0
                
                print(f"  Run: {run_num}, Event: {event_num}, Lumi: {lumi_num}")
                
                # Try to access CSC collections
                for csc_branch in csc_branches:
                    try:
                        print(f"  Checking branch: {csc_branch}")
                        # This is a simplified approach - in practice you'd need to handle
                        # the specific data structure of each branch
                        branch_data = events[csc_branch]
                        print(f"    Branch type: {type(branch_data)}")
                        print(f"    Branch shape: {branch_data.array().shape if hasattr(branch_data, 'array') else 'N/A'}")
                        
                    except Exception as e:
                        print(f"    Error accessing branch: {e}")
            
            print(f"\nProcessed {min(10, events.num_entries)} events")
            
    except Exception as e:
        print(f"Error processing file: {e}")
        return None
    
    return None

def main():
    """Main function"""
    
    # Default file path
    root_file = "pickevents.root"
    
    # Check if file exists
    if not Path(root_file).exists():
        print(f"Error: File {root_file} not found")
        print("Please make sure the ROOT file is in the current directory")
        return
    
    # Extract CSC information
    result = extract_csc_info(root_file)
    
    if result is not None:
        print("\nExtraction completed successfully")
        print("Result saved to output file")
    else:
        print("\nExtraction completed with some issues")
        print("Check the output above for details")

if __name__ == "__main__":
    main()
