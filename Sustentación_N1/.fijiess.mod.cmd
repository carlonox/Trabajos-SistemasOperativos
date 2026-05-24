savedcmd_fijiess.mod := printf '%s\n'   fijiess.o | awk '!x[$$0]++ { print("./"$$0) }' > fijiess.mod
