/usr/bin/vcgencmd measure_temp | mosquitto_pub -h localhost -t "/picterrari/temperatura" -l
/usr/bin/vcgencmd get_throttled | mosquitto_pub -h localhost -t "/picterrari/voltatge_error" -l
top -d 0.5 -b -n2 | grep "Cpu(s)"|tail -n 1 | awk '{print $2 + $4}' | mosquitto_pub -h localhost -t "/picterrari/cpu" -l
df -h | grep "/dev/root" | awk '{print $5}' | cut -c 1-2 | mosquitto_pub -h localhost -t "/picterrari/disc" -l
free | grep Mem | awk '{print 100*($7)/$2}' | mosquitto_pub -h localhost -t "/picterrari/ram" -l
