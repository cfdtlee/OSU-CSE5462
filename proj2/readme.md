## data generation
run 
```
waf --run scrach/myproj2
tcpdump -nn -tt -r tcp-large-transfer-0-0.pcap > rawdata
python get_seq.py
```
you will get: 
- rawdata: raw info of time and seq
- retransmission.data
- normal.data
- slowstart.cwnd
- congection_avoid.cwnd

## plot  
```
plot "normal.data" using 1:2 title 'normal' with linespoints, "retransmission.data" using 1:2 title 'retransmission' 
plot "congection_avoid.cwnd" using 1:2 title 'congection_avoid' with linespoints, "slowstart.cwnd" using 1:2 title 'slowstart'
```