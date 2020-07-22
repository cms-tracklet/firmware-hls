# Determine which executable is running
set exepath [info nameofexecutable]
set exe [file tail $exepath]
set version_string [exec $exe -version]
set result [regexp {([20][20][0-9][0-9]\.[1-4])} $version_string version]
set year [string range $version 0 3]