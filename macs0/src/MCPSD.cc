#include "MCPSD.hh"
#include "G4HCofThisEvent.hh"
#include "G4Step.hh"
#include "G4ThreeVector.hh"
#include "G4SDManager.hh"
#include "G4ios.hh"

#include "RootIO.hh"

// ----------------------------------------------------------------------
MCPSD::MCPSD(const G4String& name, const G4String& hitsCollectionName, int verbose) :
  G4VSensitiveDetector(name),
  fHitsCollection(NULL),
  fVerbose(verbose) {
  collectionName.insert(hitsCollectionName);
}

// ----------------------------------------------------------------------
MCPSD::~MCPSD() {
  //  RootIO::GetInstance()->Close();
}

// ----------------------------------------------------------------------
void MCPSD::Initialize(G4HCofThisEvent* hce) {
  fHitsCollection = new MCPHitsCollection(GetName(), collectionName[0]);
  G4int hcID  = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
  hce->AddHitsCollection(hcID, fHitsCollection);
}

// ----------------------------------------------------------------------
G4bool MCPSD::ProcessHits(G4Step* aStep, G4TouchableHistory*) {
  G4double edep = aStep->GetTotalEnergyDeposit();

  if (edep==0.) return false;

  if (fVerbose > 9) G4cout << "==========> MCPSD::ProcessHits> new hit added aStep = " << aStep
			   << " at " << aStep->GetPostStepPoint()->GetPosition()
			   << G4endl;
  MCPHit *newHit = new MCPHit();
  newHit->SetTrackID  (aStep->GetTrack()->GetTrackID());
  newHit->SetChamberNb(aStep->GetPreStepPoint()->GetTouchable()->GetReplicaNumber());
  newHit->SetEdep     (edep);
  newHit->SetPos      (aStep->GetPostStepPoint()->GetPosition());
  newHit->SetEtrk     (aStep->GetTrack()->GetKineticEnergy());
  newHit->SetGblTime  (aStep->GetTrack()->GetGlobalTime());
  fHitsCollection->insert(newHit);

  return true;
}

// ----------------------------------------------------------------------
void MCPSD::EndOfEvent(G4HCofThisEvent*) {
  // storing the hits in ROOT file
  G4int NbHits = fHitsCollection->entries();
  std::vector<MCPHit*> hitsVector;

  if (fVerbose > 0) G4cout << "-------->Storing hits in the ROOT file: there are " << NbHits
			   << " hits in the MCP chambers: "
			   << G4endl;
  if (1) {
    for (G4int i=0;i<NbHits;i++) {
      if (0) (*fHitsCollection)[i]->Print();
      // -- write into event/tree
      THit *hit = RootIO::GetInstance()->getEvent()->addHit();
      hit->fNumber  = RootIO::GetInstance()->getEvent()->nHits() - 1;
      hit->fDetId   = 1;// MCP = 1
      hit->fChamber = (*fHitsCollection)[i]->GetChamberNb();
      hit->fTrack   = (*fHitsCollection)[i]->GetTrackID();
      hit->fGenCand = 0;
      hit->fEdep    = (*fHitsCollection)[i]->GetEdep();
      hit->fGblTime = (*fHitsCollection)[i]->GetGblTime();
      G4ThreeVector x = (*fHitsCollection)[i]->GetPos();
      hit->fPos     = TVector3(x.x(), x.y(), x.z());
    }
  }

  if (1) {
    for (G4int i=0;i<NbHits;i++) hitsVector.push_back((*fHitsCollection)[i]);
    RootIO::GetInstance()->WriteMCPHits(&hitsVector);
  }

}
