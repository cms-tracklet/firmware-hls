# Script to generate project for TB
#   vivado_hls -f script_TB.tcl
#   vivado_hls -p trackBuilder
# WARNING: this will wipe out the original project by the same name

# create new project (deleting any existing one of same name)
open_project -reset trackBuilder

# source files
set CFLAGS {-std=c++11 -I../TrackletAlgorithm}
set_top TrackBuilder_L1L2
add_files ../TrackletAlgorithm/TrackBuilderTop.cc -cflags "$CFLAGS"
add_files -tb ../TestBenches/TrackBuilder_test.cpp -cflags "$CFLAGS"

open_solution "solution1"

# Define FPGA, clock frequency & common HLS settings.
source settings_hls.tcl

# data files
add_files -tb ../emData/FT/FT_L1L2/

csim_design -compiler gcc -mflags "-j8"
csynth_design
cosim_design
export_design -format ip_catalog
# Adding "-flow impl" runs full Vivado implementation, providing accurate resource use numbers (very slow).
#export_design -format ip_catalog -flow impl

exit
