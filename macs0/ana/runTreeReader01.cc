#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdlib>
#include <math.h>

#include "TROOT.h"
#include "TRint.h"
#include "TChain.h"
#include "TFile.h"
#include "TDirectory.h"
#include "TString.h"
#include "TRandom.h"
#include "TUnixSystem.h"
#include "TSystem.h"
#include "TKey.h"

#include "util.hh"
#include "treeReader01.hh"

using namespace std;

// // ----------------------------------------------------------------------
// // -- temporary
// void replaceAll(string &str, const string &from, const string &to) {
//   if (from.empty()) return;
//   size_t start_pos = 0;
//   while((start_pos = str.find(from, start_pos)) != string::npos) {
//     str.replace(start_pos, from.length(), to);
//     start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
//   }
// }


// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// %% Usage: bin/runTreeReader01 -f test.root
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

int main(int argc, char *argv[]) {

  int processID = gSystem->GetPid();
  cout << "Running under process ID  " << processID << endl;

  string fileName;
  int file(0);
  int dirspec(0);
  int nevents(-1), start(-1);
  int verbose(-99);

  // Change the MaxTreeSize to 100 GB (default since root v5.26)
  TTree::SetMaxTreeSize(100000000000ll); // 100 GB

  // -- Some defaults
  string dirBase("./");               // this could point to "/home/ursl/data/root/."
  string dirName("."); dirspec = 0;   // and this to, e.g. "bmm", "bee", "bem", ...
  string cutFile("tree.default.cuts");

  string treeName("gen");
  string evtClassName("rEvent");

  string readerName("treeReader01");
  TString histfile("");

  // -- command line arguments
  for (int i = 0; i < argc; i++){
    if (!strcmp(argv[i],"-h")) {
	cout << "List of arguments:" << endl;
	cout << "-c filename   chain definition file" << endl;
	cout << "-C filename   file with cuts" << endl;
	cout << "-D path       where to put the output" << endl;
	cout << "-f filename   single file instead of chain" << endl;
	cout << "-n integer    number of events to run on" << endl;
	cout << "-s number     seed for random number generator" << endl;
	cout << "-S start      starting event number" << endl;
	cout << "-o filename   set output file" << endl;
	cout << "-v level      set verbosity level" << endl;
	cout << "-h            prints this message and exits" << endl;
	return 0;
    }
    if (!strcmp(argv[i],"-c"))  {fileName   = string(argv[++i]); file = 0; }     // file with chain definition
    if (!strcmp(argv[i],"-C"))  {cutFile    = string(argv[++i]);           }     // file with cuts
    if (!strcmp(argv[i],"-D"))  {dirName    = string(argv[++i]);  dirspec = 1; } // where to put the output
    if (!strcmp(argv[i],"-f"))  {fileName   = string(argv[++i]); file = 1; }     // single file instead of chain
    if (!strcmp(argv[i],"-n"))  {nevents    = atoi(argv[++i]); }                 // number of events to run
    if (!strcmp(argv[i],"-o"))  {histfile   = TString(argv[++i]); }              // set output file
    if (!strcmp(argv[i],"-S"))  {start = atoi(argv[++i]); }                      // set start event number
    if (!strcmp(argv[i],"-v"))  {verbose    = atoi(argv[++i]); }                 // set verbosity level
  }


  // -- Prepare histfilename variation with (part of) cut file name
  TString fn(cutFile);
  fn.ReplaceAll("cuts/", "");
  fn.ReplaceAll(".cuts", "");
  fn.ReplaceAll("tree", "");

  // -- Determine filename for output histograms and 'final' small/reduced tree
  TString meta = fileName;
  if(histfile == "") {
    TString  barefile(fileName), chainFile, meta;
    if (file == 0) {
      // -- input from chain
      if (barefile.Contains("chains/")) {
	barefile.ReplaceAll("chains/", "");
	histfile = barefile + "." + fn + ".root";
	if (dirspec) {
	  if (dirName[0] == '/') {
	    histfile = dirName + "/" + histfile;
	  } else {
	    histfile = dirBase + "/" + dirName + "/" + histfile;
	  }
	}
      } else {
	histfile =  barefile + "." + fn + ".root";
	if (dirspec) {
	  if (dirName[0] == '/') {
	    histfile = dirName + "/" + histfile;
	  } else {
	    histfile = dirBase + "/" + dirName + "/" + histfile;
	  }
	}
      }
      // -- The following lines strip everything from the string up to and including the last '/'
      int fl = barefile.Last('/');
      TString bla(barefile);
      bla.Replace(0, fl+1, ' '); bla.Strip(TString::kLeading, ' ');  bla.Remove(0,1);
      histfile =  bla + "." + fn + ".root";
      if (dirspec) {
	histfile = dirBase + "/" + dirName + "/" + histfile;
      }
    }  else if (file == 1) {
      // -- single file input
      // -- The following lines strip everything from the string up to and including the last '/'
      int fl = barefile.Last('/');
      TString bla(barefile);
      bla.Replace(0, fl+1, ' '); bla.Strip(TString::kLeading, ' ');  bla.Remove(0,1);
      histfile =  bla;
      histfile.ReplaceAll(".root", "");
      histfile +=  "." + fn + ".root";
      if (dirspec) {
	if (dirName[0] == '/') {
	  histfile = dirName + "/" + histfile;
	} else {
	  histfile = dirBase + "/" + dirName + "/" + histfile;
	}
      }
    }
  }
  string shistfile = histfile.Data();
  replaceAll(shistfile, "..", ".");
  histfile = shistfile.c_str();
  cout << "Opening " << histfile.Data() << " for output histograms" << endl;
  cout << "Opening " << fileName.c_str() << " for input" << endl;


  // -- Set up chain
  TChain *chain = new TChain(TString(treeName));
  cout << "Chaining ... " << treeName << endl;
  char pName[2000];
  int nentries;
  if (file == 0) {
    // -- non-trivial chain input
    ifstream is(meta);
    while(meta.ReadLine(is) && (!meta.IsNull())){
      nentries = -1;
      if (meta.Data()[0] == '#') continue;
      sscanf(meta.Data(), "%s %d", pName, &nentries);
      if (nentries > -1) {
        cout << pName << " -> " << nentries << " entries" << endl;
        chain->Add(pName, nentries);
      } else {
        cout << meta << endl;
        chain->Add(meta);
      }
    }
    is.close();
  } else if (file == 1) {
    // -- single file input
    cout << fileName << endl;
    chain->Add(TString(fileName));
  }

  // -- Now instantiate the tree-analysis class object, initialize, and run it ...
  treeReader01 *a = NULL;
  if ("treeReader01" == readerName) {
    cout << "instantiating treeReader01" << endl;
    a = new treeReader01(chain, TString(evtClassName));
  }

  if (a) {
    a->openHistFile(histfile);
    a->readCuts(cutFile.c_str(), 1);
    a->bookHist();

    a->startAnalysis();
    a->loop(nevents, start);
    a->endAnalysis();
    a->closeHistFile();
  }

  delete a; // so we can dump some information in the destructor

  return 0;
}
