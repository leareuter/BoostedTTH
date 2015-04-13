// -*- C++ -*-
//
// Package:    BoostedTTH/BoostedProducer/HEPTopJetMatcher
// Class:      SubjetFilterJetMatcher
// 
/**\class HEPTopJetMatcher HEPTopJetMatcher.cc BoostedTTH/BoostedProducer/plugins/HEPTopJetMatcher.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Shawn Williamson
//         Created:  Fri, 23 Jan 2015 08:17:01 GMT
//
//

#include "BoostedTTH/BoostedProducer/plugins/HEPTopJetMatcher.h"

//
// constants, enums and typedefs
//


//
// static data member definitions
//

//
// constructors and destructor
//
HEPTopJetMatcher::HEPTopJetMatcher(const edm::ParameterSet& iConfig):
  minJetPt(iConfig.getParameter<double>("minJetPt"))
{
  
  produces<boosted::HEPTopJetCollection>("heptopjets");
  
  recofatjetsTag_=iConfig.getParameter<edm::InputTag>("recofatjetsTag");
  patfatjetsTag_=iConfig.getParameter<edm::InputTag>("patfatjetsTag");
  patsubjetsTag_=iConfig.getParameter<edm::InputTag>("patsubjetsTag");
  toptagsTag_=iConfig.getParameter<edm::InputTag>("toptagsTag");
  subjettiness1Tag_=iConfig.getParameter<edm::InputTag>("subjettiness1Tag");
  subjettiness2Tag_=iConfig.getParameter<edm::InputTag>("subjettiness2Tag");
  subjettiness3Tag_=iConfig.getParameter<edm::InputTag>("subjettiness3Tag");
  subjettiness4Tag_=iConfig.getParameter<edm::InputTag>("subjettiness4Tag");
}


HEPTopJetMatcher::~HEPTopJetMatcher()
{
 
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//

// ------------ method called to produce the data  ------------
void
HEPTopJetMatcher::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  using namespace edm;
  
  edm::Handle< std::vector<reco::BasicJet> > recofatjetsHandle;
  iEvent.getByLabel(recofatjetsTag_, recofatjetsHandle);
  std::vector<reco::BasicJet> recofatjets = *recofatjetsHandle;
  
  edm::Handle<edm::View<pat::Jet> > patfatjetsHandle;
  iEvent.getByLabel(patfatjetsTag_, patfatjetsHandle);
  edm::View<pat::Jet> patfatjets = *patfatjetsHandle;
  
  edm::Handle<edm::View<pat::Jet> > patsubjetsHandle;
  iEvent.getByLabel(patsubjetsTag_, patsubjetsHandle);
  edm::View<pat::Jet> patsubjets = *patsubjetsHandle;
  
  edm::Handle<std::vector<bool> > toptagsHandle;
  iEvent.getByLabel(toptagsTag_, toptagsHandle);
  std::vector<bool> toptags = *toptagsHandle;
  
  edm::Handle<std::vector<double> > subjettiness1Handle;
  iEvent.getByLabel(subjettiness1Tag_, subjettiness1Handle);
  std::vector<double> subjettiness1 = *subjettiness1Handle;
  
  edm::Handle<std::vector<double> > subjettiness2Handle;
  iEvent.getByLabel(subjettiness2Tag_, subjettiness2Handle);
  std::vector<double> subjettiness2 = *subjettiness2Handle;
  
  edm::Handle<std::vector<double> > subjettiness3Handle;
  iEvent.getByLabel(subjettiness3Tag_, subjettiness3Handle);
  std::vector<double> subjettiness3 = *subjettiness3Handle;
  
  edm::Handle<std::vector<double> > subjettiness4Handle;
  iEvent.getByLabel(subjettiness4Tag_, subjettiness4Handle);
  std::vector<double> subjettiness4 = *subjettiness4Handle;
  
  std::auto_ptr<boosted::HEPTopJetCollection> HEPTopJets(new boosted::HEPTopJetCollection());

  std::multimap<double, int> patfatjetindex_by_eta;
  std::multimap<double, int> patsubjetindex_by_eta;
  
  for(size_t i=0; i<patfatjets.size(); ++i) patfatjetindex_by_eta.insert(std::pair<double,int>(patfatjets[i].eta(), i));
  for(size_t i=0; i<patsubjets.size(); ++i) patsubjetindex_by_eta.insert(std::pair<double,int>(patsubjets[i].eta(), i));
  
  for(typename std::vector<reco::BasicJet>::const_iterator it=recofatjets.begin();it!=recofatjets.end();++it){
    if(it->pt() < minJetPt) continue;
    
    HEPTopJets->push_back(boosted::HEPTopJet());
    HEPTopJets->back().fatjet 		= deltarJetMatching(patfatjets, patfatjetindex_by_eta, *it);
    HEPTopJets->back().toptag 		= toptags[it-recofatjets.begin()];
    HEPTopJets->back().subjettiness1 	= subjettiness1[it-recofatjets.begin()];
    HEPTopJets->back().subjettiness2 	= subjettiness2[it-recofatjets.begin()];
    HEPTopJets->back().subjettiness3 	= subjettiness3[it-recofatjets.begin()];
    HEPTopJets->back().subjettiness4 	= subjettiness4[it-recofatjets.begin()];
    
    std::vector<const reco::Candidate*> recosubjets = it->getJetConstituentsQuick();
    
    for(size_t i=0;i<recosubjets.size(); ++i){
      const pat::Jet & patsubjet = deltarJetMatching(patsubjets, patsubjetindex_by_eta, *(recosubjets.at(i)));
      
      if(i==0) HEPTopJets->back().topjet = patsubjet;
      if(i==1) HEPTopJets->back().nonW = patsubjet;
      if(i==2) HEPTopJets->back().W = patsubjet;
      if(i==3) HEPTopJets->back().W1 = patsubjet;
      if(i==4) HEPTopJets->back().W2 = patsubjet;
    }
  }
  
  iEvent.put(HEPTopJets,"heptopjets");
}


template<typename recojettype>
const pat::Jet & HEPTopJetMatcher::deltarJetMatching(const edm::View<pat::Jet> & patjets, const std::multimap<double, int> & patjetindex_by_eta, const recojettype & recojet){
	std::multimap<double, int>::const_iterator lower = patjetindex_by_eta.lower_bound(recojet.eta() - 0.01);
	std::multimap<double, int>::const_iterator upper = patjetindex_by_eta.upper_bound(recojet.eta() + 0.01);

	double delta_r = 9999.0;
	int best_match = -1;

	for(std::multimap<double, int>::const_iterator it=lower; it!=upper; ++it){
		if(best_match == -1 || reco::deltaR(patjets[it->second], recojet) < delta_r){
			best_match = it->second;
			delta_r = reco::deltaR(patjets[best_match], recojet);
		}
	}

	if(best_match >= 0) return patjets[best_match];
	throw std::string("deltarJetMatching: could not find matching jet.");
}


// ------------ method called once each job just before starting event loop  ------------
void 
HEPTopJetMatcher::beginJob()
{
}


// ------------ method called once each job just after ending the event loop  ------------
void 
HEPTopJetMatcher::endJob() {
}

 
// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
HEPTopJetMatcher::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(HEPTopJetMatcher);
