clang -DBTB -DCALLS_CNT=16 tte_rsb.c -o tte_btb_rsb16 -no-pie

clang -DBTB -DCALLS_CNT=32 tte_rsb.c -o tte_btb_rsb32 -no-pie

clang -DRSB -DCALLS_CNT=16 tte_rsb.c -o tte_rsb_rsb16 -no-pie

clang -DRSB -DCALLS_CNT=32 tte_rsb.c -o tte_rsb_rsb32 -no-pie

clang -DPHT -DCALLS_CNT=16 tte_rsb.c -o tte_pht_rsb16 -no-pie

clang -DPHT -DCALLS_CNT=32 tte_rsb.c -o tte_pht_rsb32 -no-pie

