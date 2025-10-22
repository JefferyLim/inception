if [ "$#" -lt 3 ]; then
  echo "Usage: $0 CORE1 CORE2 OUTPUT_DIR [CLANG_ARGS]" >&2
  exit 1
fi

mkdir -p $3 

sudo bash -c "echo 0 > /sys/devices/system/cpu/cpu$2/online"

taskset -c $1 ./tte_btb_rsb16 > $3/btb_16_calls.txt $4

taskset -c $1 ./tte_btb_rsb32 > $3/btb_32_calls.txt $4

taskset -c $1 ./tte_rsb_rsb16 > $3/rsb_16_calls.txt $4

taskset -c $1 ./tte_rsb_rsb32 > $3/rsb_32_calls.txt $4

taskset -c $1 ./tte_pht_rsb16 > $3/pht_16_calls.txt $4

taskset -c $1 ./tte_pht_rsb32 > $3/pht_32_calls.txt $4

sudo bash -c "echo 1 > /sys/devices/system/cpu/cpu$2/online"
