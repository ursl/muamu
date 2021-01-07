#include "PrimaryGeneratorAction.hh"
#include "DetectorConstruction.hh"

#include "G4GenericMessenger.hh"
#include "G4Event.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "Randomize.hh"

#include "musrMuonium.hh"


// ----------------------------------------------------------------------
PrimaryGeneratorAction::PrimaryGeneratorAction(DetectorConstruction* myDC):
  G4VUserPrimaryGeneratorAction(), fParticleGun(0), fMyDetector(myDC),
  fBgNpart(1), fBgNpartSigma(0.),
  fBgKinEnergy(26.1), fBgKinEnergySigma(1.1) {
  fParticleGun = new G4ParticleGun();

  G4ParticleDefinition* particle  = musrMuonium::MuoniumDefinition();
  fParticleGun->SetParticleDefinition(particle);

  auto particleTable = G4ParticleTable::GetParticleTable();
  G4ParticleDefinition* backgroundParticle = particleTable->FindParticle("mu+");
  fBackgroundGun = new G4ParticleGun();
  fBackgroundGun->SetParticleDefinition(backgroundParticle);

  DefineCommands();

}

// ----------------------------------------------------------------------
PrimaryGeneratorAction::~PrimaryGeneratorAction() {
  delete fParticleGun;
  delete fBackgroundGun;
}

// ----------------------------------------------------------------------
void PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent) {
  // G4double position = -0.5*(fMyDetector->GetWorldFullLength());
  // fParticleGun->SetParticlePosition(G4ThreeVector(0.*cm, 0.*cm, position));
  // fParticleGun->GeneratePrimaryVertex(anEvent);
  // fParticleGun->SetParticleMomentumDirection(G4ThreeVector(0.,0.,-1.));
  // fParticleGun->SetParticleEnergy(1*keV);
  G4cout << "PrimaryGeneratorAction::GeneratePrimaries with particle = " << fParticleGun->GetParticleDefinition()->GetParticleName() << G4endl;
  fParticleGun->GeneratePrimaryVertex(anEvent);


  G4double z0  = -80.*cm;
  G4double x0  = 0*cm, y0  = 0*cm;
  G4double dx0 = 2*cm, dy0 = 2*cm;

  double sig =  G4RandGauss::shoot(0.,fBgNpartSigma);
  int nBg = round(fBgNpart + sig);

  G4cout << "PrimaryGeneratorAction::GeneratePrimaries background particles n = " << nBg
	 << " (sig = " << sig << ")"
	 << " with Ekin = " << fBgKinEnergy
	 << G4endl;
  for (int i = 0; i < nBg; ++i) {
    x0 = dx0*(G4UniformRand()-0.5);
    y0 = dy0*(G4UniformRand()-0.5);

    fBackgroundGun->SetParticlePosition(G4ThreeVector(x0, y0, z0));
    fBackgroundGun->SetParticleMomentumDirection(G4ThreeVector(0., 0., 1.));
    fBackgroundGun->SetParticleEnergy(fBgKinEnergy);

    fBackgroundGun->GeneratePrimaryVertex(anEvent);
  }
}


// ----------------------------------------------------------------------
void PrimaryGeneratorAction::DefineCommands() {
  fMessenger  = new G4GenericMessenger(this, "/macs0/generator/", "Primary generator control");

  // -- average number of background particles
  auto& bgNpartCmd = fMessenger->DeclareProperty("bgNpart", fBgNpart,
						 "Average number bg particles.");
  bgNpartCmd.SetParameterName("bgNpart", true);
  bgNpartCmd.SetRange("bgNpart>=0");
  bgNpartCmd.SetDefaultValue("2");

  // -- sigma of average number of background particles
  auto& bgNpartSigmaCmd = fMessenger->DeclareProperty("bgNpartSigma", fBgNpartSigma,
						 "Sigma of average number bg particles.");
  bgNpartSigmaCmd.SetParameterName("bgNpartSigma", true);
  bgNpartSigmaCmd.SetRange("bgNpartSigma>=0.");
  bgNpartSigmaCmd.SetDefaultValue("0.2");


  // -- kinetic energy of background particles
  auto& bgKinEnergyCmd = fMessenger->DeclarePropertyWithUnit("bgKinEnergy", "MeV", fBgKinEnergy,
							     "Mean kinetic energy of bg particles.");
  bgKinEnergyCmd.SetParameterName("bgEkin", true);
  bgKinEnergyCmd.SetRange("bgEkin>=0.");
  bgKinEnergyCmd.SetDefaultValue("26.");


  // -- sigma of kinetic energy of background particles
  auto& bgKinEnergySigmaCmd = fMessenger->DeclarePropertyWithUnit("bgKinEnergySigma", "MeV", fBgKinEnergySigma,
								  "Sigma of mean kinetic energy of bg particles.");
  bgKinEnergySigmaCmd.SetParameterName("bgEkinSigma", true);
  bgKinEnergySigmaCmd.SetRange("bgEkinSigma>=0.");
  bgKinEnergySigmaCmd.SetDefaultValue("1.");

}
