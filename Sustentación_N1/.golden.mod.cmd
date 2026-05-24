savedcmd_golden.mod := printf '%s\n'   golden.o | awk '!x[$$0]++ { print("./"$$0) }' > golden.mod
