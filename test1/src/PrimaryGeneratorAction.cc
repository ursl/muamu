#include "PrimaryGeneratorAction.hh"
#include "DetectorConstruction.hh"

#include "G4Event.hh"
#include "G4GeneralParticleSource.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "globals.hh"
#include "G4SystemOfUnits.hh"

#include "musrMuonium.hh"


// ----------------------------------------------------------------------
PrimaryGeneratorAction::PrimaryGeneratorAction(DetectorConstruction* myDC):
  G4VUserPrimaryGeneratorAction(), fParticleGun(0), fMyDetector(myDC) {
  fParticleGun = new G4GeneralParticleSource();

  G4ParticleDefinition* particle  = musrMuonium::MuoniumDefinition();
  fParticleGun->SetParticleDefinition(particle);
}

// ----------------------------------------------------------------------
PrimaryGeneratorAction::~PrimaryGeneratorAction() {
  delete fParticleGun;
}

// ----------------------------------------------------------------------
void PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent) {
  G4double position = -0.5*(fMyDetector->GetWorldFullLength());
  fParticleGun->SetParticlePosition(G4ThreeVector(0.*cm,0.*cm,position));

  fParticleGun->GeneratePrimaryVertex(anEvent);
}
