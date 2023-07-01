cat lab3benchs | sed -E -e 's/^Benchmark/},{Method:/' -e 's/input size/input_size/' -e 's/,128,128/,/' -e 's/output_channels/,output_channels/' -e 's/^Total Time \[ms\]/,TotalTime/' -e 's/Avg\. Time \[ms\]/,AvgTime/'

