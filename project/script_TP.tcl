# Script to generate project for TP
#   vivado_hls -f script_TP.tcl
#   vivado_hls -p trackletProcessor
# WARNING: this will wipe out the original project by the same name

# create new project (deleting any existing one of same name)
open_project -reset trackletProcessor

# source files
set CFLAGS {-std=c++11 -I../TrackletAlgorithm}
set_top TrackletProcessor_L1L2D
add_files ../TrackletAlgorithm/TrackletProcessor.cc -cflags "$CFLAGS"
add_files -tb ../TestBenches/TrackletProcessor_L1L2D_test.cpp -cflags "$CFLAGS"

open_solution "solution1"

# Define FPGA, clock frequency & common HLS settings.
source settings_hls.tcl

# data files
add_files -tb ../emData/TP/tables/
add_files -tb ../emData/TP/TP_L1L2D/

csim_design

##csim_design -compiler gcc -mflags "-j8"
csynth_design
cosim_design 
##export_design -format ip_catalog

## Adding "-flow impl" runs full Vivado implementation, providing accurate resource use numbers (very slow).
#export_design -format ip_catalog -flow impl

exit
