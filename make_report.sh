#!/usr/bin/env bash

./diplom_cli compile_bench --compilers clang++ --bench_repeat 3 -o compile_report.txt
./diplom_cli report_bench -i compile_report.txt --aggregate_by Realisation,Bench --heatmap user_time:Realisation*Bench  -o compile_report_o2_o3_clang++.png

./diplom_cli bench --compilers clang++ --meta_iterations=1000  --bench_repeat 20 -o report.txt
./diplom_cli report_bench -i report.txt --aggregate_by Realisation,Bench --heatmap UserProcessorTime:Realisation*Bench  -o report_o2_o3_clang++.png
