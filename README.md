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
- ag_viper_gts_13 - viper (Most Wanted)
- audi_ttr_dtm - tt (Most Wanted)
- audi_tt_04 - tt (Most Wanted)
- bmw_m3_e92 - bmwm3gtr (Most Wanted)
- bmw_m3_e92_drift - bmwm3gtr (Most Wanted)
- bmw_m3_e92_s1 - bmwm3gtr (Most Wanted)
- bmw_m3_gt2 - bmwm3gtr (Most Wanted)
- corvette_c6r - corvettec6r (Most Wanted)
- dodge_char_srt8 - charger06 (Carbon)
- dthwsh_nissan_240sx - 240sx (Carbon)
- fd_2000_gt2_murcielago_lp600_scream - murcielago (Most Wanted)
- fiagt_bmw_m3_e46_gtr - bmwm3gtr (Most Wanted)
- fiagt_legion_corvette_c6r - corvettec6r (Most Wanted)
- golf_mk5_stock - gti (Most Wanted)
- golf_mk5_tuned - gti (Most Wanted)
- gravel_mitsubishi_evo9_r4 - lancerevo8 (Most Wanted)
- gra_subaru_555 - imprezawrx (Most Wanted)
- hd_ngt_350z - 350z (Carbon)
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
- ks_ford_gt40 - fordgt (Most Wanted)
- ks_ford_mustang_2015 - mustanggt (Most Wanted)
- ks_lamborghini_countach - murcielago (Most Wanted)
- ks_lamborghini_countach_s1 - murcielago (Most Wanted)
- ks_lamborghini_gallardo_sl - gallardo (Most Wanted)
- ks_lamborghini_gallardo_sl_s3 - gallardo (Most Wanted)
- ks_mazda_rx7_spirit_r - rx7 (Most Wanted)
- ks_mazda_rx7_tuned - rx7 (Most Wanted)
- ks_nissan_370z - 350z (Carbon)
- ks_nissan_gtr - 350z (Carbon)
- ks_nissan_gtr_gt3 - 350z (Carbon)
- ks_nissan_skyline_r34 - skyline (Carbon)
- ks_porsche_911_gt3_cup_2017 - 911gt2 (Most Wanted)
- ks_porsche_911_gt3_rs - 911gt2 (Most Wanted)
- ks_porsche_911_gt3_r_2016 - 911gt2 (Most Wanted)
- ks_porsche_911_r - 911gt2 (Most Wanted)
- ks_porsche_991_carrera_s - 997s (Most Wanted)
- ks_porsche_991_turbo_s - 911turbo (Most Wanted)
- ks_toyota_ae86 - corolla (Carbon)
- ks_toyota_ae86_drift - corolla (Carbon)
- ks_toyota_ae86_tuned - corolla (Carbon)
- ks_toyota_supra_mkiv - supra (Most Wanted)
- ks_toyota_supra_mkiv_drift - supra (Most Wanted)
- ks_toyota_supra_mkiv_tuned - supra (Most Wanted)
- lamborghini_diablo_sv - gallardo (Most Wanted)
- lotus_elise_sc - elise (Most Wanted)
- lotus_elise_sc_s1 - elise (Most Wanted)
- lotus_elise_sc_s2 - elise (Most Wanted)
- mby_subaru_impreza_gdb - imprezawrx (Most Wanted)
- mercedes_c63_amg_black_series - sl500 (Most Wanted)
- mercedes_mclaren_slr - slr (Most Wanted)
- murcielago_lp640 - murcielago (Most Wanted)
- outlawvettegt1 - corvettec6r (Most Wanted)
- pagani_zonda_r - zonda (Carbon)
- pb_m3gtr_nfs - bmwm3gtr (Most Wanted)
- r3_abarth_punto - punto (Most Wanted)
- tjl_mitsubishi_eclipse_gs-t - eclipsegt (Most Wanted)
- vw_golf_v_gti - gti (Most Wanted)