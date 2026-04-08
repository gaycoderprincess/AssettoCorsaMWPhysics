# Assetto Corsa Most Wanted Physics

Experiment for Assetto Corsa that converts the physics and handling model from NFS Most Wanted and Carbon

This mod was heavily based off of the work done in [Brawltendo's MW vehicles decompilation](https://github.com/Brawltendo/Most-Wanted-Vehicles-Decomp) and the cleanups and extensions of it done in [dbalatoni13's MW reverse engineering project](https://github.com/dbalatoni13/nfsmw).  
I have recently discovered that the latter project uses AI, so while this mod was based off of their code, I do not endorse dbalatoni13's project.

## Installation

- Make sure you have the latest Steam version of the game, as this is the only version this plugin is compatible with. (exe size of 22890776 bytes)
- Plop the files into your game folder.
- Enjoy, nya~ :3

## Features

- Full 1:1 port of Most Wanted's engine, gearbox, suspension and tire physics as well as the aerodynamics such as drag and air stabilization
- The mod is based on actual code rather than tweaks of random values, ensuring the handling is as accurate as possible
- Nitrous has been added, bound to the ALT key on the keyboard and B on an Xbox controller, the NOS gauge replaces the fuel gauge.
- Performance tuning has been added, available in the new Most Wanted section of the car setup menu
- Speedbreaker has been added, enabled by the `speedbreaker` option in the toml file, bound to the X key on the keyboard or X on an Xbox controller

## Useful info

- FOR DEVELOPERS: Almost all handling values have been replaced! For editing car handling, edit the files in `CarDataDump` instead, except for car mass and some aerodynamics-related values
- Only cars that have configs in the `CarDataDump` folder work in-game! Copy a car from the `orig_mw_full` or `orig_cb_full` subfolders and rename it to your desired car's name to make it work!
- This mod is only compatible with the vanilla game or CSP 0.2.11 (NO OTHER VERSION)! If you're using CSP, Tyres FX has to be disabled!

## Building

Building is done on an Arch Linux system with CLion and vcpkg being used for the build process. 

Before you begin, clone [nya-common](https://github.com/gaycoderprincess/nya-common) and [nya-common-ac](https://github.com/gaycoderprincess/nya-common-ac) to folders next to this one, so they can be found.

Required packages: `mingw-w64-gcc`

You should be able to build the project now in CLion.

## Handling List

- a3dr_viper_rt10 - viper (Most Wanted)
- adam_lz_240sx - 240sx (Carbon)
- ad_td_porsche_carrera_gt - carreragt (Most Wanted)
- ag_dodge_vipercc_08 - viper (Most Wanted)
- ag_mazda_rx8_madmike - rx8 (Most Wanted)
- ag_viper_gts_13 - viper (Most Wanted)
- audi_ttr_dtm - tt (Most Wanted)
- audi_tt_04 - tt (Most Wanted)
- bdp_mazda_rx8 - rx8 (Most Wanted)
- bg_mercedes_clk55_amg - clk500 (Most Wanted)
- bmw_m3_e92 - bmwm3gtr (Most Wanted)
- bmw_m3_e92_drift - bmwm3gtr (Most Wanted)
- bmw_m3_e92_s1 - bmwm3gtr (Most Wanted)
- bmw_m3_gt2 - bmwm3gtr (Most Wanted)
- bm_mazda_rx8_types - rx8 (Most Wanted)
- corvette_c6r - corvettec6r (Most Wanted)
- dodge_char_srt8 - charger06 (Carbon)
- dthwsh_nissan_240sx - 240sx (Carbon)
- fd_2000_gt2_murcielago_lp600_scream - murcielago (Most Wanted)
- ferrari_360_modena_1999 - gallardo (Most Wanted)
- ferrari_458 - carreragt (Most Wanted)
- ferrari_458_gt2 - carreragt (Most Wanted)
- ferrari_458_s3 - carreragt (Most Wanted)
- ferrari_599xxevo - corvettec6r (Most Wanted)
- ferrari_f40 - gallardo (Most Wanted)
- ferrari_f40_s3 - gallardo (Most Wanted)
- ferrari_laferrari - corvettec6r (Most Wanted)
- fiagt_bmw_m3_e46_gtr - bmwm3gtr (Most Wanted)
- fiagt_legion_corvette_c6r - corvettec6r (Most Wanted)
- golf_mk5_stock - gti (Most Wanted)
- golf_mk5_tuned - gti (Most Wanted)
- gravel_mitsubishi_evo9_r4 - lancerevo8 (Most Wanted)
- gra_subaru_555 - imprezawrx (Most Wanted)
- gue_lexus_is_300_jdm - is300 (Most Wanted)
- hd_ngt_350z - 350z (Carbon)
- hd_vipersrt10 - viper (Most Wanted)
- kas_vw_golf_gti_mkv_2006 - gti (Most Wanted)
- ks_audi_a1s1 - a3 (Most Wanted)
- ks_audi_r8_lms - darius (Carbon)
- ks_audi_r8_lms_2016 - darius (Carbon)
- ks_audi_r8_plus - darius (Carbon)
- ks_audi_sport_quattro - punto (Most Wanted)
- ks_audi_sport_quattro_rally - punto (Most Wanted)
- ks_audi_sport_quattro_s1 - punto (Most Wanted)
- ks_audi_tt_cup - tt (Most Wanted)
- ks_audi_tt_vln - tt (Most Wanted)
- ks_corvette_c7 - corvette (Most Wanted)
- ks_corvette_c7r - corvettec6r (Most Wanted)
- ks_corvette_c7_stingray - corvette (Most Wanted)
- ks_ferrari_288_gto - murcielago (Most Wanted)
- ks_ferrari_330_p4 - corvettec6r (Most Wanted)
- ks_ferrari_488_challenge_evo - carreragt (Most Wanted)
- ks_ferrari_488_gt3 - carreragt (Most Wanted)
- ks_ferrari_488_gt3_2020 - carreragt (Most Wanted)
- ks_ferrari_488_gtb - carreragt (Most Wanted)
- ks_ferrari_812_superfast - corvettec6r (Most Wanted)
- ks_ferrari_fxx_k - corvettec6r (Most Wanted)
- ks_ford_gt40 - fordgt (Most Wanted)
- ks_ford_mustang_2015 - mustanggt (Most Wanted)
- ks_lamborghini_aventador_sv - murcielago (Most Wanted)
- ks_lamborghini_countach - murcielago (Most Wanted)
- ks_lamborghini_countach_s1 - murcielago (Most Wanted)
- ks_lamborghini_gallardo_sl - gallardo (Most Wanted)
- ks_lamborghini_gallardo_sl_s3 - gallardo (Most Wanted)
- ks_lamborghini_huracan_gt3 - murcielago (Most Wanted)
- ks_lamborghini_huracan_performante - murcielago (Most Wanted)
- ks_lamborghini_huracan_st - murcielago (Most Wanted)
- ks_lamborghini_sesto_elemento - murcielago (Most Wanted)
- ks_maserati_mc12_gt1 - zonda (Carbon)
- ks_mazda_787b - zonda (Carbon)
- ks_mazda_miata - rx8 (Most Wanted)
- ks_mazda_mx5_cup - rx7 (Most Wanted)
- ks_mazda_mx5_nd - rx7 (Most Wanted)
- ks_mazda_rx7_spirit_r - rx7 (Most Wanted)
- ks_mazda_rx7_tuned - rx7 (Most Wanted)
- ks_mclaren_570s - carreragt (Most Wanted)
- ks_mclaren_650_gt3 - corvettec6r (Most Wanted)
- ks_mclaren_f1_gtr - corvettec6r (Most Wanted)
- ks_mclaren_p1 - carreragt (Most Wanted)
- ks_mclaren_p1_gtr - corvettec6r (Most Wanted)
- ks_mercedes_amg_gt3 - slr (Most Wanted)
- ks_mercedes_c9 - zonda (Carbon)
- ks_nissan_370z - 350z (Carbon)
- ks_nissan_gtr - 350z (Carbon)
- ks_nissan_gtr_gt3 - 350z (Carbon)
- ks_nissan_skyline_r34 - skyline (Carbon)
- ks_pagani_huayra_bc - zonda (Carbon)
- ks_porsche_718_boxster_s - caymans (Most Wanted)
- ks_porsche_718_boxster_s_pdk - caymans (Most Wanted)
- ks_porsche_718_cayman_s - caymans (Most Wanted)
- ks_porsche_911_carrera_rsr - 997tt (Carbon)
- ks_porsche_911_gt3_cup_2017 - 911gt2 (Most Wanted)
- ks_porsche_911_gt3_rs - 911gt2 (Most Wanted)
- ks_porsche_911_gt3_r_2016 - 911gt2 (Most Wanted)
- ks_porsche_911_r - 911gt2 (Most Wanted)
- ks_porsche_911_rsr_2017 - 911gt2 (Most Wanted)
- ks_porsche_991_carrera_s - 997s (Most Wanted)
- ks_porsche_991_turbo_s - 911turbo (Most Wanted)
- ks_porsche_cayman_gt4_clubsport - caymans (Most Wanted)
- ks_porsche_cayman_gt4_std - caymans (Most Wanted)
- ks_ruf_rt12r - 911gt2 (Most Wanted)
- ks_ruf_rt12r_awd - 911gt2 (Most Wanted)
- ks_toyota_ae86 - corolla (Carbon)
- ks_toyota_ae86_drift - corolla (Carbon)
- ks_toyota_ae86_tuned - corolla (Carbon)
- ks_toyota_supra_mkiv - supra (Most Wanted)
- ks_toyota_supra_mkiv_drift - supra (Most Wanted)
- ks_toyota_supra_mkiv_tuned - supra (Most Wanted)
- ks_toyota_ts040 - zonda (Carbon)
- lamborghini_diablo_sv - gallardo (Most Wanted)
- lotus_elise_sc - elise (Most Wanted)
- lotus_elise_sc_s1 - elise (Most Wanted)
- lotus_elise_sc_s2 - elise (Most Wanted)
- lotus_exige_240 - elise (Most Wanted)
- lotus_exige_240_s3 - elise (Most Wanted)
- lotus_exige_s - elise (Most Wanted)
- lotus_exige_scura - elise (Most Wanted)
- lotus_exige_s_roadster - elise (Most Wanted)
- mazda_mazda_speed_3_2009 - mazdaspeed3 (Carbon)
- mazda_rx8_track - rx8 (Most Wanted)
- mazda_rx8_types - rx8 (Most Wanted)
- mazda_rx8_type_s_2007 - rx8 (Most Wanted)
- mby_subaru_impreza_gdb - imprezawrx (Most Wanted)
- mby_viper_gts - viper (Most Wanted)
- mclaren_mp412c - carreragt (Most Wanted)
- mclaren_mp412c_gt3 - corvettec6r (Most Wanted)
- mercedes_c63_amg_black_series - sl500 (Most Wanted)
- mercedes_mclaren_slr - slr (Most Wanted)
- murcielago_lp640 - murcielago (Most Wanted)
- ng_is300 - is300 (Most Wanted)
- outlawvettegt1 - corvettec6r (Most Wanted)
- p4-5_2011 - zonda (Carbon)
- pagani_huayra - zonda (Carbon)
- pagani_zonda_r - zonda (Carbon)
- pb_cobalt_ss_sc - cobaltss (Most Wanted)
- pb_cobalt_ss_turbo - cobaltss (Most Wanted)
- pb_m3gtr_nfs - bmwm3gtr (Most Wanted)
- r3_abarth_punto - punto (Most Wanted)
- ruf_yellowbird - 997tt (Carbon)
- shelby_cobra_427sc - mustangshlbyo (Carbon)
- some1_corvette_c4_zr1_1990 - corvette (Most Wanted)
- some1_corvette_c4_zr1_1990_s1 - corvette (Most Wanted)
- tjl_mitsubishi_eclipse_gs-t - eclipsegt (Most Wanted)
- toyota_vitz1.5rs - punto (Most Wanted)
- toyota_vitz_rs_2007 - punto (Most Wanted)
- trafpizza - cs_clio_trafpizza (Most Wanted)
- vitz_rs_2007_track - punto (Most Wanted)
- vw_golf_v_gti - gti (Most Wanted)