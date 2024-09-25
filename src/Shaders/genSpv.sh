#!/bin/bash
for i in *.frag; do
	glslc $i -o $i.spv
done

for i in *.vert; do
	glslc $i -o $i.spv
done

for i in *.comp; do
	glslc $i -o $i.spv
done
