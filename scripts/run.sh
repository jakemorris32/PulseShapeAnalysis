#!/bin/bash

# Run the bsl_adjust.c commands
echo "Starting bsl_adjust.c: executing bslAdjust()..."
root -l -b <<EOF
.L bsl_adjust.c
bslAdjust()
.q
EOF
echo "Completed execution of bsl_adjust.c."

# Run t0.c with argument 0.2
echo "Starting t0.c: executing t0(0.2)..."
root -l -b <<EOF
.L t0.c
t0(0.2)
.q
EOF
echo "Completed execution of t0(0.2)."

# Run t0.c with argument 0.1
echo "Starting t0.c: executing t0(0.1)..."
root -l -b <<EOF
.L t0.c
t0(0.1)
.q
EOF
echo "Completed execution of t0(0.1)."

# Run t0.c with argument 0.5
echo "Starting t0.c: executing t0(0.05)..."
root -l -b <<EOF
.L t0.c
t0(0.05)
.q
EOF
echo "Completed execution of t0(0.5)."

# Run t0.c with argument 0.3
echo "Starting t0.c: executing t0(0.03)..."
root -l -b <<EOF
.L t0.c
t0(0.03)
.q
EOF
echo "Completed execution of t0(0.3)."

echo "Now plotting"
root -l -b <<EOF
.L t0params.c
plot()
.q
EOF
echo "All commands executed successfully."
