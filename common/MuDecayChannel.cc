// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
//
// $Id: MuDecayChannel.cc,v 1.17 2006/06/29 19:25:34 gunter Exp $
// GEANT4 tag $Name: geant4-09-00 $
//
//
// ------------------------------------------------------------
//      GEANT 4 class header file
//
//      History: first implementation, based on object model of
//      30 May  1997 H.Kurashige
//
//      Fix bug in calcuration of electron energy in DecayIt 28 Feb. 01 H.Kurashige
//
//  2005
//      M. Melissas ( melissas AT cppm.in2p3.fr)
//      J. Brunner ( brunner AT cppm.in2p3.fr)
//      Adding V-A fluxes for neutrinos using a new algortithm :
//
//  2008-05
//      Modified for the muonium decay by Toni SHIROKA, Paul Scherrer Institut, PSI
//  2020-09
//      Modified to include the atomic electron by ursl, PSI
// ------------------------------------------------------------

#include "G4ParticleDefinition.hh"
#include "G4DecayProducts.hh"
#include "G4VDecayChannel.hh"
#include "MuDecayChannel.hh"
#include "Randomize.hh"
#include "G4LorentzVector.hh"
#include "G4LorentzRotation.hh"
#include "G4RotationMatrix.hh"

#include "TMath.h"

#define MMUON      0.1057
#define MELECTRON  0.0005

// ----------------------------------------------------------------------
// -- cf. ~/macros/psi-inHouse/muamu.C
double f_eAtomic(double *x, double *par) {
  double r = MELECTRON/MMUON;
  double rinf = 13.61;
  double denominator  = 1. + (1 + r)*(1 + r)*x[0]/rinf;
  double denominator4 = denominator*denominator*denominator*denominator;
  double numerator    = (16./TMath::Pi())*TMath::Sqrt((1+r)*(1+r)*x[0]/rinf) * (1 + r)*(1+r)/rinf;
  double result = numerator/denominator4; // this is in eV!!

  return result;
}


MuDecayChannel::MuDecayChannel(const G4String& theParentName, G4double theBR) : G4VDecayChannel("Muonium Decay",1) {
  feAtomic = new TF1("feAtomic", f_eAtomic, 0., 100., 1);
  // set names for daughter particles
  if (theParentName == "Muonium") {
    SetBR(theBR);
    SetParent("Muonium");
    SetNumberOfDaughters(4);
    SetDaughter(0, "e+");
    SetDaughter(1, "nu_tau"); // this is just a test whether this decay channel is active w/o explicit mentioning in PhysicsList
    SetDaughter(2, "anti_nu_mu");
    SetDaughter(3, "e-");
    G4cout << "MuDecayChannel:: constructor :";
    G4cout << " parent particle is ";
    G4cout << theParentName << G4endl;
  } else {
    G4cout << "MuDecayChannel:: constructor :";
    G4cout << " parent particle is not muon but ";
    G4cout << theParentName << G4endl;
  }
}

MuDecayChannel::~MuDecayChannel() {
  delete feAtomic;
}

G4DecayProducts *MuDecayChannel::DecayIt(G4double)
{
  // this version neglects muon polarization,and electron mass
  //              assumes the pure V-A coupling
  //              the Neutrinos are correctly V-A.
  //#ifdef G4VERBOSE
  if (0) G4cout << "MuDecayChannel::DecayIt! " <<  G4endl;
  //#endif

  //------------------------------modified----------xran----------------
  // if (G4MT_parent == 0) FillParent();
  // if (G4MT_daughters == 0) FillDaughters();
  //------------------------------modified----------xran----------------

  // parent mass
  //------------------------------modified----------xran----------------
  G4double parentmass = G4MT_parent->GetPDGMass();
  //------------------------------modified----------xran----------------


  //daughters'mass
  G4double daughtermass[4];
  G4double sumofdaughtermass = 0.0;
  for (G4int index=0; index<4; index++){
    daughtermass[index] = G4MT_daughters[index]->GetPDGMass();
    // -- exclude atomic electron from muon decay daughters:
    if (index < 3) sumofdaughtermass += daughtermass[index];
  }

  //create parent G4DynamicParticle at rest
  G4ThreeVector dummy;
  G4DynamicParticle * parentparticle = new G4DynamicParticle( G4MT_parent, dummy, 0.0);
  //create G4Decayproducts
  G4DecayProducts *products = new G4DecayProducts(*parentparticle);
  delete parentparticle;

  // calculate daughter momentum
  G4double daughtermomentum[4];
  // calcurate electron energy
  G4double xmax = (1.0 + daughtermass[0]*daughtermass[0]/parentmass/parentmass);
  G4double x, y;

  G4double Ee,Ene;

  G4double gam;
  G4double EMax=parentmass/2 - daughtermass[0];


  // -- Generate random energies for muonic decay products
  do {
    Ee = G4UniformRand();
    do{
      x = xmax*G4UniformRand();
      gam = G4UniformRand();
    } while (gam > x*(1.-x));
    Ene = x;
  } while (Ene < (1.-Ee));
  G4double Enm = (2. - Ee - Ene);

  // -- and for atomic electron
  G4double Eeatomic = 1.e-6*feAtomic->GetRandom(0., 100.);



  // -- initialisation of rotation parameters
  G4double costheta,sintheta,rphi,rtheta,rpsi;
  costheta= 1.-2./Ee-2./Ene+2./Ene/Ee;
  sintheta=std::sqrt(1.-costheta*costheta);


  rphi=CLHEP::twopi*G4UniformRand()*CLHEP::rad;
  rtheta=(std::acos(2.*G4UniformRand()-1.));
  rpsi=CLHEP::twopi*G4UniformRand()*CLHEP::rad;

  G4RotationMatrix rot;
  rot.set(rphi,rtheta,rpsi);

  // -- electron 0 (from muonic decay)
  daughtermomentum[0]=std::sqrt(Ee*Ee*EMax*EMax + 2.0*Ee*EMax * daughtermass[0]);
  G4ThreeVector direction0(0.0, 0.0, 1.0);
  direction0 *= rot;
  G4DynamicParticle * daughterparticle = new G4DynamicParticle(G4MT_daughters[0], direction0*daughtermomentum[0]);
  products->PushProducts(daughterparticle);

  // -- electronic neutrino 1
  daughtermomentum[1]=std::sqrt(Ene*Ene*EMax*EMax + 2.0*Ene*EMax * daughtermass[1]);
  G4ThreeVector direction1(sintheta, 0.0, costheta);
  direction1 *= rot;
  G4DynamicParticle * daughterparticle1 = new G4DynamicParticle(G4MT_daughters[1], direction1*daughtermomentum[1]);
  products->PushProducts(daughterparticle1);

  // -- muonic neutrino 2
  daughtermomentum[2]=std::sqrt(Enm*Enm*EMax*EMax + 2.0*Enm*EMax*daughtermass[2]);
  G4ThreeVector direction2(-Ene/Enm*sintheta, 0., -Ee/Enm-Ene/Enm*costheta);
  direction2 *= rot;
  G4DynamicParticle * daughterparticle2 = new G4DynamicParticle(G4MT_daughters[2], direction2*daughtermomentum[2]);
  products->PushProducts(daughterparticle2);


  // -- electron 1 (atomic electron)
  daughtermomentum[3] = std::sqrt(Eeatomic*Eeatomic + 2.*Eeatomic*daughtermass[3]);
  G4ThreeVector direction3(0., 0., -1.);
  direction3 *= rot;
  G4DynamicParticle * daughterparticle3 = new G4DynamicParticle(G4MT_daughters[3], direction3*daughtermomentum[3]);
  products->PushProducts(daughterparticle3);


  // output message
  if (0) {
    G4cout << "  created decay products in rest frame " <<G4endl;
    products->DumpInfo();
  }
  return products;
}
