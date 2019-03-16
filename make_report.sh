#!/usr/bin/env bash

./diplom_cli compile_bench --compilers clang++ --bench_repeat 6 -o reports/compile_report.txt
./diplom_cli report_bench -i reports/compile_report.txt --aggregate_by Realisation,Bench \
    --heatmap user_time:Realisation*Bench  -o reports/compile_report_o2_o3_clang++.png
./diplom_cli report_bench -i reports/compile_report.txt --aggregate_by Realisation,Bench \
    --filter "df.Realisation != 'baseline_copy'" \
    --heatmap user_time:Realisation*Bench  -o reports/compile_report_o2_o3_clang++_prod.png

./diplom_cli bench --compilers clang++ --meta_iterations=1000  --bench_repeat 10 -o reports/report.txt
./diplom_cli report_bench -i reports/report.txt --aggregate_by Realisation,Bench \
    --heatmap UserProcessorTime:Realisation*Bench  -o reports/report_o2_o3_clang++.png
./diplom_cli report_bench -i reports/report.txt --aggregate_by Realisation,Bench \
    --filter "df.Realisation != 'baseline_copy'" \
    --heatmap UserProcessorTime:Realisation*Bench  -o reports/report_o2_o3_clang++_prod.png
