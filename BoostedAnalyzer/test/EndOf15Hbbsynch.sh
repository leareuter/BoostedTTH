cmsenv

cmsRun /nfs/dust/cms/user/shwillia/EndOf2015HbbSync/CMSSW_7_4_14/src/BoostedTTH/BoostedAnalyzer/test/sync_tth_endof15Hbb_cfg.py

cmsRun /nfs/dust/cms/user/shwillia/EndOf2015HbbSync/CMSSW_7_4_14/src/BoostedTTH/BoostedAnalyzer/test/sync_ttbar_endof15Hbb_cfg.py

cmsRun /nfs/dust/cms/user/shwillia/EndOf2015HbbSync/CMSSW_7_4_14/src/BoostedTTH/BoostedAnalyzer/test/sync_ttbar_ext3_endof15Hbb_cfg.py

mkdir KIT_syncHBB
mkdir KIT_syncHBB/onlySL
mkdir KIT_syncHBB/onlyDL

cp sync_tth_endof15Hbb_raw.csv KIT_syncHBB/tth_raw.csv
cp sync_tth_endof15Hbb_JESdown.csv KIT_syncHBB/tth_JESDown.csv
cp sync_tth_endof15Hbb_JESup.csv KIT_syncHBB/tth_JESUp.csv
cp sync_tth_endof15Hbb.csv KIT_syncHBB/tth.csv

cp sync_ttbar_endof15Hbb_raw.csv KIT_syncHBB/ttjets_raw.csv
cp sync_ttbar_endof15Hbb_JESdown.csv KIT_syncHBB/ttjets_JESDown.csv
cp sync_ttbar_endof15Hbb_JESup.csv KIT_syncHBB/ttjets_JESUp.csv
cp sync_ttbar_endof15Hbb.csv KIT_syncHBB/ttjets.csv

cp sync_ttbar_ext3_endof15Hbb_raw.csv KIT_syncHBB/ttjets_ext3_raw.csv
cp sync_ttbar_ext3_endof15Hbb_JESdown.csv KIT_syncHBB/ttjets_ext3_JESDown.csv
cp sync_ttbar_ext3_endof15Hbb_JESup.csv KIT_syncHBB/ttjets_ext3_JESUp.csv
cp sync_ttbar_ext3_endof15Hbb.csv KIT_syncHBB/ttjets_ext3.csv


for i in KIT_syncHBB/*.csv; do echo "$i"; python /nfs/dust/cms/user/shwillia/EndOf2015HbbSync/CMSSW_7_4_14/src/BoostedTTH/BoostedAnalyzer/test/splitSLDL.py "$i"; done