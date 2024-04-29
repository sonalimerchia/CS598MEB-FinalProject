for num_cells in 100 150 200 250 300 600
do 
    for s in 0 1 2 3 4 5 6
    do 
        for loci in 1000 2000 3000 4000
        do 
            echo $num_cells $loci $s
            if ! test -f output_data/n${num_cells}_l${loci}_s${s}-CN3-TIMES; then
                date
                time ./compare -f lazac-copy-number/data/simulations/n${num_cells}_l${loci}_s${s}_full_cn_profiles.csv -o output_data/n${num_cells}_l${loci}_s${s} -t 2>log${loci}-${s}.log
            fi
        done
    done
done

