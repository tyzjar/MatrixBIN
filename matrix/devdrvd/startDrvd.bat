ps -A | grep devdrvd | awk '$1 != "" {kill = "/bin/kill -s SIGUSR1 " $1; print kill; system(kill);}'

