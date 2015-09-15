// -*- C++ -*-
//
// Package:    BoostedTTH/BoostedProducer
// Class:      SelectedLeptonProducer
// 
/**\class SelectedLeptonProducer SelectedLeptonProducer.cc BoostedTTH/BoostedProducer/plugins/SelectedLeptonProducer.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Matthias Schroeder
//         Created:  Tue, 15 Sep 2015 11:58:04 GMT
//
//


// system include files
#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDProducer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "MiniAOD/MiniAODHelper/interface/MiniAODHelper.h"

#include "DataFormats/PatCandidates/interface/Lepton.h"
#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/PatCandidates/interface/Muon.h"

//
// class declaration
//

class SelectedLeptonProducer : public edm::EDProducer {
public:
  explicit SelectedLeptonProducer(const edm::ParameterSet&);
  ~SelectedLeptonProducer();
  
  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);
  
private:
  enum LeptonType { Electron, Muon };

  virtual void beginJob() override;
  virtual void produce(edm::Event&, const edm::EventSetup&) override;
  virtual void endJob() override;
  bool setUpHelper(const edm::Event& iEvent);
  
  // ----------member data ---------------------------
  MiniAODHelper helper_;
  LeptonType leptonType_;  

  // lepton selection criteria
  double ptMin_;
  double etaMax_;

  // data access tokens
  edm::EDGetTokenT< double >                  EDMRhoToken; //  pileup density
  edm::EDGetTokenT< reco::VertexCollection >  EDMVertexToken; // vertex
  edm::EDGetTokenT< pat::MuonCollection >     EDMMuonsToken;  // muons
  edm::EDGetTokenT< pat::ElectronCollection > EDMElectronsToken;  // electrons
};


//
// constructors and destructor
//
SelectedLeptonProducer::SelectedLeptonProducer(const edm::ParameterSet& iConfig)
{
  const std::string leptonType = iConfig.getParameter<std::string>("leptonType");
  if(      leptonType == "electron" ) leptonType_ = Electron;
  else if( leptonType == "muon"     ) leptonType_ = Muon;
  else {
    std::cerr << "\n\nERROR: Unknown lepton type '" << leptonType << "'" << std::endl;
    std::cerr << "Please select 'electron' or 'muon'\n" << std::endl;
    throw std::exception();
  }

  const std::string era = iConfig.getParameter<std::string>("era");
  const std::string analysisType = iConfig.getParameter<std::string>("analysisType");
  analysisType::analysisType iAnalysisType = analysisType::LJ;
  if(      analysisType == "LJ"     ) iAnalysisType = analysisType::LJ;
  else if( analysisType == "DIL"    ) iAnalysisType = analysisType::DIL;
  else if( analysisType == "TauLJ"  ) iAnalysisType = analysisType::TauLJ;
  else if( analysisType == "TauDIL" ) iAnalysisType = analysisType::TauDIL;
  else {
    std::cerr << "\n\nERROR: No matching analysis type found for: " << analysisType << std::endl;
    throw std::exception();
  }
  const bool isData = iConfig.getParameter<bool>("isData");
  const int sampleID = iConfig.getParameter<int>("sampleID");

  EDMRhoToken    = consumes< double > (edm::InputTag(std::string("fixedGridRhoFastjetAll")));
  EDMVertexToken = consumes< reco::VertexCollection > (edm::InputTag("offlineSlimmedPrimaryVertices","",""));
  EDMMuonsToken      = consumes< pat::MuonCollection >     (iConfig.getParameter<edm::InputTag>("leptons"));
  EDMElectronsToken  = consumes< pat::ElectronCollection > (iConfig.getParameter<edm::InputTag>("leptons"));

  ptMin_ = iConfig.getParameter<double>("ptMin");
  etaMax_ = iConfig.getParameter<double>("etaMax");

  // Set up MiniAODHelper
  helper_.SetUp(era,sampleID,iAnalysisType,isData);

  // do we need to set up those?
  // helper_.SetJetCorrectorUncertainty();
  // helper_.SetJetCorrector(const JetCorrector*);
  // helper_.SetFactorizedJetCorrector();
  // helper_.SetPackedCandidates(const std::vector<pat::PackedCandidate> & all, int fromPV_thresh=1, float dz_thresh=9999., bool also_leptons=false);

  if(      leptonType_ == Electron ) produces<pat::ElectronCollection>();
  else if( leptonType_ == Muon     ) produces<pat::MuonCollection>();
}


SelectedLeptonProducer::~SelectedLeptonProducer() {}


//
// member functions
//

// ------------ method called to produce the data  ------------
void
SelectedLeptonProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  if( setUpHelper(iEvent) ) {
    
    if( leptonType_ == Electron ) {
      // select and produce electron collection

      edm::Handle<pat::ElectronCollection> hElectrons;
      iEvent.getByToken(EDMElectronsToken,hElectrons);
      
      // do we want to have these configurable?
      const electronID::electronID iElectronID = electronID::electronLoose;

      std::auto_ptr<pat::ElectronCollection> selectedLeptons( new pat::ElectronCollection(helper_.GetSelectedElectrons(*hElectrons,ptMin_,iElectronID,etaMax_)) );
      iEvent.put(selectedLeptons);

      
    } else if( leptonType_ == Muon ) {
      // select and produce muon collection

      edm::Handle<pat::MuonCollection> hMuons;
      iEvent.getByToken(EDMMuonsToken,hMuons);
      
      // do we want to have these configurable?
      const muonID::muonID iMuonID = muonID::muonLoose;
      const coneSize::coneSize iconeSize = coneSize::R04;
      const corrType::corrType icorrType = corrType::rhoEA;

      std::auto_ptr<pat::MuonCollection> selectedLeptons( new pat::MuonCollection(helper_.GetSelectedMuons(*hMuons,ptMin_,iMuonID,iconeSize,icorrType,etaMax_)) );
      std::cout << ">>>>> SELECTED MUONS: " << selectedLeptons->size() << " muons" << std::endl;
      for(pat::MuonCollection::const_iterator iObj = selectedLeptons->begin(); iObj != selectedLeptons->end(); ++iObj) {
	std::cout << "  pt = " << iObj->pt() << std::endl;
      }
      
      iEvent.put(selectedLeptons);
    }
  }
}

// Do event-wise setup of MiniAODHelper
// Return true if successful, false otherwise
bool
SelectedLeptonProducer::setUpHelper(const edm::Event& iEvent)
{
  // get RHO
  edm::Handle<double> hRho;
  iEvent.getByToken(EDMRhoToken,hRho);
  if( hRho.isValid() ) {
    helper_.SetRho(*hRho);
  } else {
    return false;
  }

  // get PRIMARY VERTICES
  edm::Handle<reco::VertexCollection> hVtxs;
  iEvent.getByToken(EDMVertexToken,hVtxs);
  if( hVtxs->size()>0 ) {
    helper_.SetVertex( hVtxs->at(0) );
  } else {
    return false;
  }

  return true;
}


// ------------ method called once each job just before starting event loop  ------------
void 
SelectedLeptonProducer::beginJob()
{
}

// ------------ method called once each job just after ending the event loop  ------------
void 
SelectedLeptonProducer::endJob() {
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
SelectedLeptonProducer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(SelectedLeptonProducer);
