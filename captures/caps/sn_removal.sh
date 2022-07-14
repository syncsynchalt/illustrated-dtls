#!/bin/bash -e

file=$2
key=$1
if [[ -z "$file" || -z "$key" ]]; then
    echo "Usage: $0 hexkey input > output" 1>&2
    exit 1
fi
sample=$(head -c 21 $file | tail -c 16 | xxd -p)

mask=$(echo $sample | xxd -r -p | openssl aes-128-ecb -K $key | head -c 2 | xxd -p)
mask1=$(echo $mask | sed -e 's/..$//')
mask2=$(echo $mask | sed -e 's/..//')
byte1=$(head -c2 $file | tail -c1 | xxd -p)
byte2=$(head -c3 $file | tail -c1 | xxd -p)
xor1=$(printf "%02x\n" $((0x${byte1} ^ 0x${mask1})))
xor2=$(printf "%02x\n" $((0x${byte2} ^ 0x${mask2})))

# output first byte as-is
head -c 1 $file

# xor the next two bytes
echo "$xor1 $xor2" | xxd -r -p

tail -c +4 $file
