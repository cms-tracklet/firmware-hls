# Execute download.sh in the emData/ directory.
set cwd [pwd]
cd ../emData/
if { [catch { exec ./download.sh } msg] } {
    puts $msg
}
cd $cwd

# Allow HLS to use longer (easier to understand) names in resource/latency usage profiles.
config_compile -name_max_length 100

# Set clock frequency
create_clock -period 250MHz -name default

switch -glob -- $exe {
    *vitis* {
        # Set the FPGA part number
        set_part {xcvu7p-flvb2104-1-e}
    }
    default {
        # Set the FPGA part number
        set_part {xcvu7p-flvb2104-1-e} -tool vivado
        #set_part {xcku115-flvb2104-1-e} -tool vivado

        # Remove variables from IP interface if they are never used by HLS algo.
        config_interface -trim_dangling_port

        # Prevent HLS relaxing pipeline interval to meet timing.
        config_schedule -relax_ii_for_timing=0 -verbose
    }
}

# Encourage HLS to make more effort to find best solution.
# (Worth trying, but increases CPU use, so not enabled by default)
#config_bind -effort high
#config_schedule -effort high -relax_ii_for_timing=0 -verbose
